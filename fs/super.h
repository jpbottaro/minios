#ifndef __SUPER_H__
#define __SUPER_H__

/* Taken from minix v3 */

#include <sys/types.h>

struct superblock_s {
  u16_t s_ninodes;		    /* # usable inodes on the minor device */
  u16_t s_nzones;		    /* total device size, including bit maps etc */
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
/* FUNCIONES PARA MODIFICAR BITMAP */

#define BLOCK_SIZE     1024
#define IMAP_BLOCKS    (sb->s_imap_blocks)
#define ZMAP_BLOCKS    (sb->s_zmap_blocks)

#define INODE_SIZE     (sizeof(struct inode_s))
#define INODE_MAX      (sb->s_ninodes)
#define INODE_OFFSET   (2048 + (IMAP_BLOCKS + ZMAP_BLOCKS) * BLOCK_SIZE)

#define DIRENTRY_SIZE  (sizeof(struct dir_entry_s))
#define NR_DIR_ENTRIES (BLOCK_SIZE / DIRENTRY_SIZE)

#endif /* __SUPER_H__ */
