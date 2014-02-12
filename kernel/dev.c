#include <minios/dev.h>

struct dev_s devices[MAX_DEVICES];

void dev_init(void)
{

}

/* assign file operations of device */
int dev_file_calls(struct file_s *flip, dev_t dev)
{
    if (dev >= MAX_DEVICES)
        return -1;
    flip->f_op = &devices[dev].d_op;
    return 0;
}

int dev_register(unsigned int major, struct file_operations_s *fops)
{
    if (major >= MAX_DEVICES)
        return -1;
    devices[major].d_op = *fops;
    return 0;
}
