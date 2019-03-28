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
//https://courses.cs.washington.edu/courses/cse451/15sp/documents/os161-man/syscall/open.html
int sys_open(userptr_t filename, int flag, mode_t mode);
//https://courses.cs.washington.edu/courses/cse451/15sp/documents/os161-man/syscall/read.html
int sys_read(int fd, userptr_t buffer, size_t bufsize);
//https://courses.cs.washington.edu/courses/cse451/15sp/documents/os161-man/syscall/write.html
int sys_write(int fd,const userptr_t buffer, size_t bytesize);
//https://courses.cs.washington.edu/courses/cse451/15sp/documents/os161-man/syscall/lseek.html
uint64_t sys_lseek(int fd, uint64_t pos, int whence);
//https://courses.cs.washington.edu/courses/cse451/15sp/documents/os161-man/syscall/close.html
int sys_close(int fd);
//https://courses.cs.washington.edu/courses/cse451/15sp/documents/os161-man/syscall/dup2.html
int sys_dub2(int oldfd, int newfd);


#endif /* _FILE_H_ */
