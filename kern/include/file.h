/*
 * Declarations for file handle and file table management.
 */

#ifndef _FILE_H_
#define _FILE_H_

/*
 * Contains some file-related maximum length constants
 */
#include <limits.h>
#include <vnode.h>
#include <synch.h>

/*
 * Put your function declarations and data types here ...
 */
struct lock* overLock;

struct file {
	struct vnode* vnode;
	off_t offset;
	struct lock* lock;
	int flag;
	int ref;
};

//A destructor for the file struct
int free_file(struct file** f);

//https://courses.cs.washington.edu/courses/cse451/15sp/documents/os161-man/syscall/open.html
int sys_open(userptr_t filename, int flag, mode_t mode, int * err);
//https://courses.cs.washington.edu/courses/cse451/15sp/documents/os161-man/syscall/read.html
int sys_read(int fd, userptr_t buffer, size_t bufsize, int * err);
//https://courses.cs.washington.edu/courses/cse451/15sp/documents/os161-man/syscall/write.html
int sys_write(int fd,const userptr_t buffer, size_t bytesize, int * err);
//https://courses.cs.washington.edu/courses/cse451/15sp/documents/os161-man/syscall/lseek.html
off_t sys_lseek(int fd, uint64_t pos, int whence, int * err);
//https://courses.cs.washington.edu/courses/cse451/15sp/documents/os161-man/syscall/close.html
int sys_close(int fd, int * err);
//https://courses.cs.washington.edu/courses/cse451/15sp/documents/os161-man/syscall/dup2.html
int sys_dub2(int oldfd, int newfd, int * err);


#endif /* _FILE_H_ */
