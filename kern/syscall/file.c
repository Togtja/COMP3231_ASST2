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
#include<proc.h>

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

	//fd after stderr
	int fd;
	//Find first avalable file_desc
	for (fd = 3; fd < __OPEN_MAX; fd++) {
		if (curproc->file_desc[fd] == NULL) {
			break;
		}
	}
	//If the first "avalable" is outside the amout of possible open files
	if (fd == __OPEN_MAX) {
		*err = ENFILE;
		return -1;
	}
	curproc->file_desc[fd] = kmalloc(sizeof(struct file));
	if (curproc->file_desc[fd] == NULL){
		*err = ENOMEM;
		return -1;
	}
	struct vnode* vn;
	*err = vfs_open(saneFileName, flag, mode, &vn);
	if (err) {
		return -1;//Returns -1 if failed
	}
	//curproc->file_desc[fd]->fd = fd;
	curproc->file_desc[fd]->vnode = vn;
	curproc->file_desc[fd]->offset = 0;
	curproc->file_desc[fd]->lock = lock_create(saneFileName);
	curproc->file_desc[fd]->flag = flag;
	if (curproc->file_desc[fd]->lock == NULL) {
		*err = ENOMEM;
		return -1;
	}
	kfree(saneFileName);
	return fd;//Sucess!!!
}
int sys_read(int fd, userptr_t buffer, size_t bufsize, int * err) {
	(void)fd;
	(void)buffer;
	(void)bufsize;
	(void)err;
	return 0;
}
int sys_write(int fd, userptr_t buffer, size_t bytesize, int * err) {
	if (fd < 0 || fd >= __OPEN_MAX || curproc->file_desc[fd] == NULL) {
		*err = EBADF;
		return -1;
	}
	if (curproc->file_desc[fd]->flag == O_RDONLY) {
		*err = EINVAL;
		return -1;
	}
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