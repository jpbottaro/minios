#include "fs.h"

struct superblock_s *sb;
bitchunk_t *imap_origin;
bitchunk_t *zmap_origin;

/* load up the super block; most 'constants' depend on this */
int read_super()
{
    sb = (struct superblock_s *) (fs_offset + SUPER_OFFSET);
    imap_origin = (bitchunk_t *) (fs_offset + IMAP_OFFSET);
    zmap_origin = (bitchunk_t *) (fs_offset + ZMAP_OFFSET);

    return 0;
}

/* allocate bit in map (algo. idea taken heavily from minix) */
/* FIXME HIGHGLY UNTESTED AND PROBABLY DOES NOT WORK, CHECK */
ino_t alloc_bit(int map)
{
    ino_t map_bits, b;
    bitchunk_t *start, *wptr, *wlim;
    unsigned int i;

    if (map == IMAP) {
        start = (bitchunk_t *) (fs_offset + IMAP_OFFSET);
        wptr = imap_origin;
        map_bits = INODE_MAX;
        wlim = (bitchunk_t *) (fs_offset + ZMAP_OFFSET);
    } else {
        start = (bitchunk_t *) (fs_offset + ZMAP_OFFSET);
        wptr = zmap_origin;
        map_bits = BLOCK_MAX;
        wlim = (bitchunk_t *) (fs_offset + INODE_OFFSET);
    }

    for (; wptr < wlim; wptr++) {
        /* is there a free bit */
        if (*wptr == (bitchunk_t) ~0) continue;

        /* find it */
        for (i = 0; (*wptr & (1 << i)) != 0; ++i) {}

        /* bit number (that is, inode number) */
        b = (wptr - start) * BITCHUNK_BITS + i;

        /* do not allocate bits beyond the end of the map */
        if (b >= map_bits) break;

        /* allocate it */
        *wptr |= 1 << i;

        /* update origin */
        if (map == IMAP) imap_origin = wptr;
        else             zmap_origin = wptr;

        /* return it ( :o ) */
        return b;
    }

    return NO_INODE;
}

/* return an unused inode (and mark it as used) */
ino_t empty_inode()
{
    return alloc_bit(IMAP);
}

/* return an unused block (and mark it as used) */
block_t empty_block()
{
    return alloc_bit(ZMAP);
}

/* free bit in map (algo. idea taken heavily from minix) */
/* FIXME HIGHGLY UNTESTED AND PROBABLY DOES NOT WORK, CHECK */
void free_bit(int map, unsigned int num)
{
    bitchunk_t *wptr, mask;
    unsigned int words, bits;

    if (map == IMAP)
        wptr = (bitchunk_t *) (fs_offset + IMAP_OFFSET);
    else
        wptr = (bitchunk_t *) (fs_offset + ZMAP_OFFSET);

    words = num / BITCHUNK_BITS;
    bits = num % BITCHUNK_BITS;
    mask = 1 << bits;

    wptr += words;
    /* if (!(*wptr & mask)) panic(); ver si poner panic */
    *wptr &= ~mask;
    
    if (map == IMAP && imap_origin > wptr)
        imap_origin = wptr;
    else if (map == ZMAP && zmap_origin > wptr)
        zmap_origin = wptr;
}

/* mark inode as unused in bitmap */
void rm_inode(ino_t ino_num)
{
    free_bit(IMAP, ino_num);
}

/* mark block as unused in bitmap */
void rm_block(block_t block_num)
{
    free_bit(ZMAP, block_num);
}
