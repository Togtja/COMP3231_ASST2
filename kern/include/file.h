/*
 * Declarations for file handle and file table management.
 */

#ifndef _FILE_H_
#define _FILE_H_

/*
 * Contains some file-related maximum length constants
 */
#include <limits.h>


/*
 * Put your function declarations and data types here ...
 */

int sys_open(const char* filename, int flag, mode_t mode);
int sys_read(int fd, void* buffer, size_t bufsize);
int sys_write(int fd,const void* buffer, size_t bytesize);
uint64_t sys_lseek(int fd, uint64_t pos, int whence);
int sys_close(int fd);
int sys_dub2(int oldfd, int newfd);


#endif /* _FILE_H_ */
