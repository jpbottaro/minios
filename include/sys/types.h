/* The <sys/types.h> header contains important data type definitions.
 * It is considered good programming practice to use these definitions,
 * instead of the underlying base type.  By convention, all type names end
 * with _t.
 *
 * Taken from Minix - slightly modified to fit our needs
 *
 */

#ifndef _TYPES_H
#define _TYPES_H

#include <sys/null.h>

typedef unsigned char   u8_t;	   /* 8 bit type */
typedef unsigned short u16_t;	   /* 16 bit type */
typedef signed char     i8_t;      /* 8 bit signed type */
typedef short          i16_t;      /* 16 bit signed type */

#if __SIZEOF_LONG__ > 4
/* compiling with gcc on some (e.g. x86-64) platforms */
typedef unsigned int   u32_t;	   /* 32 bit type */
typedef int            i32_t;      /* 32 bit signed type */
#else
/* default for ACK or gcc on 32 bit platforms */
typedef unsigned long  u32_t;	   /* 32 bit type */
typedef long           i32_t;      /* 32 bit signed type */
#endif

#if !defined(__LONG_LONG_SUPPORTED)
typedef struct {
	u32_t lo;
	u32_t hi;
} u64_t;
#else
#if __SIZEOF_LONG__ > 4
typedef unsigned long u64_t;
#else
typedef unsigned long long u64_t;
#endif
#endif

/* some Minix specific types that do not conflict with posix */
typedef u32_t zone_t;	   /* zone number */
typedef u32_t block_t;	   /* block number */
typedef u32_t bit_t;	   /* bit number in a bit map */
typedef u16_t zone1_t;	   /* zone number for V1 file systems */
typedef u32_t bitchunk_t; /* collection of bits in a bitmap */

/* ANSI C makes writing down the promotion of unsigned types very messy.  When
 * sizeof(short) == sizeof(int), there is no promotion, so the type stays
 * unsigned.  When the compiler is not ANSI, there is usually no loss of
 * unsignedness, and there are usually no prototypes so the promoted type
 * doesn't matter.  The use of types like Ino_t is an attempt to use ints
 * (which are not promoted) while providing information to the reader.
 */

typedef unsigned int  Ino_t;

/* The type size_t holds all results of the sizeof operator.  At first glance,
 * it seems obvious that it should be an unsigned int, but this is not always
 * the case. For example, MINIX-ST (68000) has 32-bit pointers and 16-bit
 * integers. When one asks for the size of a 70K struct or array, the result
 * requires 17 bits to express, so size_t must be a long type.  The type
 * ssize_t is the signed version of size_t.
 */
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef int ssize_t;
#endif

#ifndef _TIME_T
#define _TIME_T
typedef long time_t;		   /* time in sec since 1 Jan 1970 0000 GMT */
#endif

/* Types used in disk, inode, etc. data structures. */
typedef char           gid_t;	   /* group id */
typedef unsigned int   ino_t; 	   /* i-node number */
typedef unsigned short mode_t;	   /* file type and permissions bits */
typedef short        nlink_t;	   /* number of links to a file */
typedef long	       off_t;	   /* offset within a file */
typedef int            pid_t;	   /* process id (must be signed) */
typedef short          uid_t;	   /* user id */
typedef unsigned int   dev_t;


#endif /* _TYPES_H */
