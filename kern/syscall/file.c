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


//Debug comand __LINE__ (Gives line nr)
//				__func_ (gives which func we are inside) 
/*
 * Add your file-related functions here ...
 */
int free_file(struct file** f) {
	
	if (*f == NULL) {
		return EBADF;
	}
	if ((*f)->ref > 1) {
		(*f)->ref--;
	}
	else {
		if ((*f)->vnode != NULL) {
			vfs_close((*f)->vnode);
			(*f)->vnode = NULL;
		}
		if ((*f)->lock != NULL && (*f)->lock->lk_holder != NULL) {
			lock_destroy((*f)->lock);
		}
		kfree(*f);
	}
	*f = NULL;
	
	return 0;
}



int sys_open(userptr_t filename, int flag, mode_t mode, int * err) {
	
	if (filename == NULL) {
		*err = ENODEV;
		return -1;
	}
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
	//Find first avalable
	for (fd = 0; fd < __OPEN_MAX; fd++) {
		if (curproc->file_desc[fd] == NULL) {
			break;
		}
	}
	//If the first "avalable" is outside the amout of possible open files
	if (fd >= __OPEN_MAX) {
		kfree(saneFileName);
		*err = ENFILE;
		
		return -1;
	}
	curproc->file_desc[fd] = kmalloc(sizeof(struct file*));
	if (curproc->file_desc[fd] == NULL){
		*err = ENOMEM;
		kfree(saneFileName);
		return -1;
	}
	struct vnode* vn;
	*err = vfs_open(saneFileName, flag, mode, &vn);
	if (*err) {
		kfree(saneFileName);
		kfree(curproc->file_desc[fd]);
		return -1;//Returns -1 if failed
	}
	
	curproc->file_desc[fd]->vnode = vn;
	curproc->file_desc[fd]->offset = 0;
	curproc->file_desc[fd]->lock = lock_create(saneFileName);
	curproc->file_desc[fd]->flag = flag;
	curproc->file_desc[fd]->ref = 1;
	//Make sure we have enough memory for the lock
	if (curproc->file_desc[fd]->lock == NULL) {
		*err = ENOMEM;
		//vfs_close(vn);
		kfree(saneFileName);
		free_file(&curproc->file_desc[fd]);//If we can't create the lock we just free the whole file
		return -1;
	}
	kfree(saneFileName);
	//kprintf("fd is : %d\n", fd);
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
	//Start reading
	lock_acquire(curproc->file_desc[fd]->lock);
	struct uio ruio;
	struct iovec riovec;
	uio_kinit(&riovec, &ruio, saneBuffer, bufsize, curproc->file_desc[fd]->offset, UIO_READ);
	*err = VOP_READ(curproc->file_desc[fd]->vnode, &ruio);
	if (*err) {
		lock_release(curproc->file_desc[fd]->lock);
		kfree(saneBuffer);
		return -1;
	}
	int read = bufsize - ruio.uio_resid;

	*err = copyout(saneBuffer, buffer, bufsize);
	if (*err) {
		kfree(saneBuffer);
		lock_release(curproc->file_desc[fd]->lock);
		return -1;
	}
	curproc->file_desc[fd]->offset += read;
	kfree(saneBuffer);
	lock_release(curproc->file_desc[fd]->lock);
	//kprintf("\nread: %d bytes\n", read);
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
	//kprintf(saneBuffer);
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
	int writen = bytesize - wuio.uio_resid;
	kfree(saneBuffer);
	lock_release(curproc->file_desc[fd]->lock);
	
	return writen;
}


off_t sys_lseek(int fd, uint64_t pos, int whence, int * err) {
	if (fd < 0 || fd >= __OPEN_MAX || curproc->file_desc[fd] == NULL) {
		*err = EBADF;
		return -1;
	}
	off_t ret;
	lock_acquire(curproc->file_desc[fd]->lock);
	//Maybe make a switch??? hmmm??

	//The new position is pos
	if (SEEK_SET == whence) {
		ret = pos;
	}
	//The new position is current position/ofset + pos
	else if (SEEK_CUR == whence) {
		ret = curproc->file_desc[fd]->offset + pos;
	}
	//The new position is eof + pos
	else if (SEEK_END == whence) {
		
		struct stat stat;
		
		*err = VOP_STAT(curproc->file_desc[fd]->vnode, &stat);
		if (*err) {
			lock_release(curproc->file_desc[fd]->lock);
			return -1;
		}
		ret = stat.st_size + pos;

	}
	//everything else fail
	else {
		lock_release(curproc->file_desc[fd]->lock);
		*err = EINVAL;
		return -1;
	}
	if (ret < 0) {
		*err = EINVAL;
		return -1;
	}
	curproc->file_desc[fd]->offset = ret;
	lock_release(curproc->file_desc[fd]->lock);
	return ret;
}
int sys_close(int fd, int * err) {
	//CLOSE GOT TO FREE THE FILE AFTER IT ISDONE WITH FILE

	// WE MIGHT NEED A LOCK FOR CLOSE
	lock_acquire(overLock);
	if (fd < 0 || fd >= __OPEN_MAX || curproc->file_desc[fd] == NULL) {
		*err = EBADF;
		lock_release(overLock);
		return -1;
	}
	*err = free_file(&curproc->file_desc[fd]);
	if (*err) {
		lock_release(overLock);
		return -1;
	}
	lock_release(overLock);
	return 0;
}
int sys_dub2(int oldfd, int newfd, int * err) {
	if (oldfd < 0 || oldfd >= __OPEN_MAX || curproc->file_desc[oldfd] == NULL) {
		*err = EBADF;
		return -1;
	}
	if (newfd < 0 || newfd >= __OPEN_MAX) {
		*err = EBADF;
		return -1;
	}
	if (oldfd == newfd) {
		*err = EBADF;
		return -1;
	}
	//If newhandle already exisit, close it
	if (curproc->file_desc[newfd] != NULL) {
		*err = sys_close(newfd, err);
		if (err) {
			return -1;
		}
	}
	//Potential for deadlock
	lock_acquire(curproc->file_desc[oldfd]->lock);
	//Copy the info from old to new, or just point to new?
	//currently join point to the same 
	//(but then if i close any of them, this will remove both, should this be fixed?)
	
	curproc->file_desc[newfd] = curproc->file_desc[oldfd];

	curproc->file_desc[newfd]->ref++;
	lock_release(curproc->file_desc[oldfd]->lock);
	return newfd;
}