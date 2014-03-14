#ifndef __SUPER_H__
#define __SUPER_H__

/* superblock struct taken from minix v3 */

#include <minios/fs.h>
#include <sys/types.h>

struct superblock_s {
  u16_t s_ninodes;		    /* # usable inodes */
  u16_t s_nzones;		    /* # usable zones */
  u16_t s_imap_blocks;		/* # of blocks used by inode bit map */
  u16_t s_zmap_blocks;		/* # of blocks used by zone bit map */
  u16_t s_firstdatazone;	/* number of first data zone (small) */
  u16_t s_log_zone_size;	/* log2 of blocks/zone (we suppose its 0) */
  u32_t s_max_size;		    /* maximum file size on this device */
  u16_t s_magic;		    /* magic number to recognize super-blocks */
  u16_t s_state;
  u32_t s_zones;		    /* number of zones */
};

/* since fs already in memory, no need to allocate space, just point to super */
extern struct superblock_s *sb;

int read_super();

ino_t   empty_inode();
block_t empty_block();

void rm_inode(ino_t ino_num);
void rm_block(block_t block_num);

#define IMAP_BLOCKS    (sb->s_imap_blocks)
#define ZMAP_BLOCKS    (sb->s_zmap_blocks)

#define SUPER_OFFSET   (1024)
#define IMAP_OFFSET    (2048)
#define ZMAP_OFFSET    (IMAP_OFFSET + IMAP_BLOCKS * BLOCK_SIZE)
#define INODE_OFFSET   (ZMAP_OFFSET + ZMAP_BLOCKS * BLOCK_SIZE)

#define INODE_SIZE     (sizeof(struct real_inode_s))
#define INODE_MAX      (sb->s_ninodes)
#define BLOCK_MAX      (sb->s_nzones)

#define TOTAL_SIZE     (BLOCK_MAX * BLOCK_SIZE)

#define DIRECT_ZONES   7

#define DIRENTRY_SIZE  (sizeof(struct dir_entry_s))
#define NR_DIR_ENTRIES (BLOCK_SIZE / DIRENTRY_SIZE)
#define DIRSIZE_MAX    (BLOCK_SIZE * DIRECT_ZONES)

#define IMAP 0
#define ZMAP 1

#define BITCHUNK_BITS  (sizeof(bitchunk_t) * 8)

#endif /* __SUPER_H__ */
