#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/stat.h>
#include <kern/seek.h>
#include <lib.h>
#include <uio.h>
#include <thread.h>
#include <current.h>
#include <synch.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h>

#include <syscall.h>
#include <copyinout.h>
//#include <unistd.h>
//#include <seek.h> //For lseek's set/cur/end
#include <vfs.h>

/*
 * Add your file-related functions here ...
 */

int sys_open(userptr_t filename, int flag, mode_t mode, int * err) {
	char* saneFileName = kmalloc(sizeof(char)*__PATH_MAX);
	if (saneFileName == NULL) {
		*err = ENOMEM;
		return -1;//Returns -1 if failed
	}
	size_t len;
	//This sanitize the user pointer to kernel space
	*err = copyinstr(filename, saneFileName, __PATH_MAX, &len); 
	if (err) {
		return -1;//Returns -1 if failed
	}
	//DO some stuff here
	/*
	DO STUFF WITH PROC HERE AND FD
	*/


		struct vnode* vn;
	*err = vfs_open(saneFileName,flag,mode,&vn);
	if (err) {
		return -1;//Returns -1 if failed
	}
	kfree(vn);
	return 0;//Sucess!!!
}
int sys_read(int fd, userptr_t buffer, size_t bufsize, int * err) {
	(void)fd;
	(void)buffer;
	(void)bufsize;
	(void)err;
	return 0;
}
int sys_write(int fd, userptr_t buffer, size_t bytesize, int * err) {
	(void)fd;
	(void)buffer;
	(void)bytesize;
	(void)err;
	return 0;
}
off_t sys_lseek(int fd, uint64_t pos, int whence, int * err) {
	(void)fd;
	(void)pos;
	(void)whence;
	(void)err;
	return 0;
}
int sys_close(int fd, int * err) {
	(void)fd;
	(void)err;
	return 0;
}
int sys_dub2(int oldfd, int newfd, int * err) {
	(void)oldfd;
	(void)newfd;
	(void)err;
	return 0;
}