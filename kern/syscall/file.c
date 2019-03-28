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
void free_file(struct file* f) {
	if (f == NULL) {
		return;
	}
//	if (f->vnode != NULL) {
//		vfs_close(f->vnode);
//	}
	if (f->lock != NULL) {
		lock_destroy(f->lock);
	}
	kfree(f);
}

int sys_open(userptr_t filename, int flag, mode_t mode, int * err) {
	char* saneFileName = kmalloc(sizeof(char)*__PATH_MAX);
	if (saneFileName == NULL) {
		*err = ENOMEM;
		return -1;//Returns -1 if failed
	}
	size_t len;
	//This sanitize the user pointer to kernel space
	*err = copyinstr(filename, saneFileName, __PATH_MAX, &len);
	if (*err) {
		kfree(saneFileName);
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
		kfree(saneFileName);
		return -1;
	}
	curproc->file_desc[fd] = kmalloc(sizeof(struct file));
	if (curproc->file_desc[fd] == NULL){
		*err = ENOMEM;
		kfree(saneFileName);
		return -1;
	}
	struct vnode* vn;
	*err = vfs_open(saneFileName, flag, mode, &vn);
	if (*err) {
		vfs_close(vn);
		kfree(saneFileName);
		//free_file(curproc->file_desc[fd]);
		return -1;//Returns -1 if failed
	}
	//curproc->file_desc[fd]->fd = fd;
	curproc->file_desc[fd]->vnode = vn;
	curproc->file_desc[fd]->offset = 0;
	curproc->file_desc[fd]->lock = lock_create(saneFileName);
	curproc->file_desc[fd]->flag = flag;
	//Make sure we have enough memory for the lock
	if (curproc->file_desc[fd]->lock == NULL) {
		*err = ENOMEM;
		vfs_close(vn);
		kfree(saneFileName);
		free_file(curproc->file_desc[fd]);//If we can't create the lock we just free the whole file
		return -1;
	}
	kfree(saneFileName);
	return fd;//Sucess!!!
}
int sys_read(int fd, userptr_t buffer, size_t bufsize, int * err) {
	if (fd < 0 || fd >= __OPEN_MAX || curproc->file_desc[fd] == NULL) {
		*err = EBADF;
		return -1;
	}
	if (curproc->file_desc[fd]->flag == O_WRONLY) {
		*err = EACCES; //Maybe make EOWFS (Write-Only File System error)
		return -1;
	}
	void* saneBuffer = kmalloc(sizeof(void *) * bufsize);
	if (saneBuffer == NULL) {
		*err = ENOMEM;
		return -1;
	}
	*err = copyin(buffer, saneBuffer, bufsize);
	if (*err) {
		return -1;
	}
	struct uio ruio;
	struct iovec riovec;

	//Start reading
	lock_acquire(curproc->file_desc[fd]->lock);
	uio_kinit(&riovec, &ruio, saneBuffer, bufsize, curproc->file_desc[fd]->offset, UIO_READ);
	*err = VOP_READ(curproc->file_desc[fd]->vnode, &ruio);
	if (*err) {
		lock_release(curproc->file_desc[fd]->lock);
		kfree(saneBuffer);
		return -1;
	}
	int read = bufsize - ruio.uio_resid;

	*err = copyout(saneBuffer, buffer, read);
	if (*err) {
		kfree(saneBuffer);
		lock_release(curproc->file_desc[fd]->lock);
		return -1;
	}
	curproc->file_desc[fd]->offset += read;
	lock_release(curproc->file_desc[fd]->lock);
	kfree(saneBuffer);
	return read;
}
int sys_write(int fd, userptr_t buffer, size_t bytesize, int * err) {
	if (fd < 0 || fd >= __OPEN_MAX || curproc->file_desc[fd] == NULL) {
		*err = EBADF;
		return -1;
	}
	if (curproc->file_desc[fd]->flag == O_RDONLY) {
		*err = EROFS;
		return -1;
	}
	void* saneBuffer = kmalloc(sizeof(void *) * bytesize);
	if (saneBuffer == NULL) {
		*err = ENOMEM;
		return -1;
	}
	*err = copyin(buffer, saneBuffer, bytesize);
	if (*err) {
		return -1;
	}
	struct uio wuio;
	struct iovec wiovec;
	
	//Start writing to the file
	//Remember to release lock if error
	lock_acquire(curproc->file_desc[fd]->lock);
	uio_kinit(&wiovec, &wuio, saneBuffer, bytesize, curproc->file_desc[fd]->offset, UIO_WRITE);
	*err = VOP_WRITE(curproc->file_desc[fd]->vnode, &wuio);
	if (*err) {
		lock_release(curproc->file_desc[fd]->lock);
		kfree(saneBuffer);
		return -1;
	}

	
	curproc->file_desc[fd]->offset = wuio.uio_offset;
	lock_release(curproc->file_desc[fd]->lock);
	kfree(saneBuffer);
	int writen = bytesize - wuio.uio_resid;
	return writen;
}
off_t sys_lseek(int fd, uint64_t pos, int whence, int * err) {
	(void)fd;
	(void)pos;
	(void)whence;
	(void)err;
	return 0;
}
int sys_close(int fd, int * err) {
	//CLOSE GOT TO FREE THE FILE AFTER IT ISDONE WITH FILE
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