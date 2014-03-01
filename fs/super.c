#include <minios/misc.h>
#include <unistd.h>
#include <fcntl.h>
#include "fs.h"

struct superblock_s superblock;
struct superblock_s *sb;
u8_t imap[BLOCK_SIZE];
bitchunk_t *imap_origin;
u8_t zmap[BLOCK_SIZE];
bitchunk_t *zmap_origin;

/* load up the super block; most 'constants' depend on this */
int read_super()
{
    sb = &superblock;
    fs_dev->f_op->lseek(fs_dev, SUPER_OFFSET, SEEK_SET);
    fs_dev->f_op->read(fs_dev, (char *) sb, sizeof(struct superblock_s));

    fs_dev->f_op->lseek(fs_dev, IMAP_OFFSET, SEEK_SET);
    fs_dev->f_op->read(fs_dev, (char *) &imap, sizeof(imap));

    fs_dev->f_op->lseek(fs_dev, ZMAP_OFFSET, SEEK_SET);
    fs_dev->f_op->read(fs_dev, (char *) &zmap, sizeof(zmap));

    imap_origin = (bitchunk_t *) &imap;
    zmap_origin = (bitchunk_t *) &zmap;

    return 0;
}

/* allocate bit in map (algo. idea taken heavily from minix) */
ino_t alloc_bit(int map)
{
    ino_t map_bits, b;
    bitchunk_t *start, *wptr, *wlim;
    unsigned int i;

    if (map == IMAP) {
        start = (bitchunk_t *) (&imap);
        wptr = imap_origin;
        map_bits = INODE_MAX;
        wlim = (bitchunk_t *) (&imap + BLOCK_SIZE);
    } else {
        start = (bitchunk_t *) (&zmap);
        wptr = zmap_origin;
        map_bits = BLOCK_MAX;
        wlim = (bitchunk_t *) (&zmap + BLOCK_SIZE);
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

char zero[BLOCK_SIZE];

/* return an unused block (and mark it as used) */
block_t empty_block()
{
    block_t block = alloc_bit(ZMAP);

    mymemset(zero, 0, BLOCK_SIZE);
    fs_dev->f_op->lseek(fs_dev, block * BLOCK_SIZE, SEEK_SET);
    fs_dev->f_op->write(fs_dev, zero, BLOCK_SIZE);

    return block;
}

/* free bit in map (algo. idea taken heavily from minix) */
void free_bit(int map, unsigned int num)
{
    bitchunk_t *wptr, mask;
    unsigned int words, bits;

    if (map == IMAP)
        wptr = (bitchunk_t *) (&imap);
    else
        wptr = (bitchunk_t *) (&zmap);

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

/* mark inode as unused in bitmap and free its blocks */
void rm_inode(ino_t ino_num)
{
    int i;
    struct inode_s *ino;

    ino = get_inode(ino_num);
    for (i = 0; i < NR_ZONES; i++)
        if (ino->i_zone[i] != NO_BLOCK)
            rm_block(ino->i_zone[i]);
    ino->i_dirty = 1;
    release_inode(ino);
    free_bit(IMAP, ino_num);
}

/* mark block as unused in bitmap */
void rm_block(block_t block_num)
{
    free_bit(ZMAP, block_num);
}
