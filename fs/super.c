#include "fs.h"

struct superblock_s *sb;

int read_super()
{
    /* the superblock starts just when the boot block finishes */
    sb = (struct superblock_s *) (fs_offset + 1024);

    return 0;
}
