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

/*
 * Add your file-related functions here ...
 */

int sys_open(const char* filename, int flag, mode_t mode) {
	(void)filename;
	(void)flag;
	(void)mode;
	return 0;
}
int sys_read(int fd, void* buffer, size_t bufsize) {
	(void)fd;
	(void)buffer;
	(void)bufsize;
	return 0;
}
int sys_write(int fd, const void* buffer, size_t bytesize) {
	(void)fd;
	(void)buffer;
	(void)bytesize;
	return 0;
}
uint64_t sys_lseek(int fd, uint64_t pos, int whence) {
	(void)fd;
	(void)pos;
	(void)whence;
	return 0;
}
int sys_close(int fd) {
	(void)fd;
	return 0;
}
int sys_dub2(int oldfd, int newfd) {
	(void)oldfd;
	(void)newfd;
	return 0;
}