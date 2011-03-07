/* The <unistd.h> header contains a few miscellaneous manifest constants.
 *
 * Taken from minixv3 - stripped down
 *
 */

#ifndef _UNISTD_H
#define _UNISTD_H

#ifndef _TYPES_H
#include <sys/types.h>
#endif

#include <sys/ucred.h>

/* Values used by access().  POSIX Table 2-8. */
#define F_OK               0	/* test if file exists */
#define X_OK               1	/* test if file is executable */
#define W_OK               2	/* test if file is writable */
#define R_OK               4	/* test if file is readable */

/* Values used for whence in lseek(fd, offset, whence).  POSIX Table 2-9. */
#define SEEK_SET           0	/* offset is absolute  */
#define SEEK_CUR           1	/* offset is relative to current position */
#define SEEK_END           2	/* offset is relative to end of file */

/* This value is required by POSIX Table 2-10. */
#define _POSIX_VERSION 199009L	/* which standard is being conformed to */

/* These three definitions are required by POSIX Sec. 8.2.1.2. */
#define STDIN_FILENO       0	/* file descriptor for stdin */
#define STDOUT_FILENO      1	/* file descriptor for stdout */
#define STDERR_FILENO      2	/* file descriptor for stderr */

/* NULL must be defined in <unistd.h> according to POSIX Sec. 2.7.1. */
#include <sys/null.h>

/* Function Prototypes. */
extern void _exit(int _status);
extern int chdir(const char *_path);
extern int fchdir(int fd);
extern int chown(const char *_path, uid_t _owner, gid_t _group);
extern int fchown(int fd, uid_t _owner, gid_t _group);
extern int close(int _fd);
extern int execl(const char *_path, const char *_arg, ...);
extern int execle(const char *_path, const char *_arg, ...);
extern int execlp(const char *_file, const char *arg, ...);
extern int execv(const char *_path, char *const _argv[]);
extern int execve(const char *_path, char *const _argv[], 
						char *const _envp[]);
extern int execvp(const char *_file, char *const _argv[]);
extern pid_t fork(void);
extern gid_t getgid(void);
extern uid_t getuid(void);
extern int link(const char *_existing, const char *_new);
extern off_t lseek(int _fd, off_t _offset, int _whence);
extern int read(int _fd, void *_buf, size_t _n);
extern int rmdir(const char *_path);
extern int setgid(gid_t _gid);
extern int setuid(uid_t _uid);
extern unsigned int sleep(unsigned int _seconds);
extern int unlink(const char *_path);
extern int write(int _fd, const void *_buf, size_t _n);
extern int truncate(const char *_path, off_t _length);
extern int ftruncate(int _fd, off_t _length);

#endif /* _UNISTD_H */
