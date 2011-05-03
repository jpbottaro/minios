#ifndef _PSO_FILE_H
#define _PSO_FILE_H

#include <sys/types.h>

typedef int (func_main)(void);

typedef struct str_pso_file {
	sint_8 signature[4];
	uint_32 mem_start;
	uint_32 mem_end_disk;
	uint_32 mem_end;
	func_main* _main;
	uint_8 data[0]; 
} pso_file;

#endif /* _PSO_FILE_H */
