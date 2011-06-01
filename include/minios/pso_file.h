#ifndef _PSO_FILE_H
#define _PSO_FILE_H

#include <sys/types.h>

typedef struct str_pso_file {
	i8_t signature[4];
	u32_t mem_start;
	u32_t mem_end_disk;
	u32_t mem_end;
	u32_t _main;
	u8_t data[0]; 
} pso_file;

#define PSO_SIZE sizeof(pso_file)

#endif /* _PSO_FILE_H */
