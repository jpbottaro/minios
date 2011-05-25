#include <minios/i386.h>
#include "hdd.h"

size_t hdd_read(struct file_s *flip, char *buf, size_t n)
{
    return 0;
}

ssize_t hdd_write(struct file_s *flip, char *buf, size_t n)
{
    return 0;
}

int hdd_flush(struct file_s *flip)
{
    return 0;
}

void hdd_init()
{
	
}
