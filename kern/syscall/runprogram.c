/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than runprogram() does.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>

#include<limits.h>
#include<file.h>

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
static int std_init() {
	//Initilize the fd stuff
	int result;
	//STDIN
	if (curproc->file_desc[0] == NULL) {
		struct vnode* v1;
		char c1[] = "con:";
		curproc->file_desc[0] = kmalloc(sizeof(struct file));
		if (curproc->file_desc[0] == NULL) {
			return ENOMEM;
		}
		result = vfs_open(c1, O_RDONLY, 0644, &v1);
		if (result) {
			free_file(&curproc->file_desc[0]);
			return result;
		}
		curproc->file_desc[0]->vnode = v1;
		curproc->file_desc[0]->flag = O_RDONLY;
		curproc->file_desc[0]->lock = lock_create("stdin");
		if (curproc->file_desc[0]->lock == NULL) {
			free_file(&curproc->file_desc[0]);
			return ENOMEM;
		}
		curproc->file_desc[0]->offset = 0;
		curproc->file_desc[0]->ref = 1;
	}
	//STDOUT
	if (curproc->file_desc[1] == NULL) {
		struct vnode* v2;
		char c2[] = "con:";
		curproc->file_desc[1] = kmalloc(sizeof(struct file));
		if (curproc->file_desc[1] == NULL) {
			return ENOMEM;
		}
		result = vfs_open(c2, O_WRONLY, 0644, &v2);
		if (result) {
			free_file(&curproc->file_desc[1]);
			return result;
		}
		curproc->file_desc[1]->vnode = v2;
		curproc->file_desc[1]->flag = O_WRONLY;
		curproc->file_desc[1]->lock = lock_create("stdout");
		if (curproc->file_desc[1]->lock == NULL) {
			free_file(&curproc->file_desc[1]);
			return ENOMEM;
		}
		curproc->file_desc[1]->offset = 0;
		curproc->file_desc[1]->ref = 1;
	}
	
	//STDERR
	if (curproc->file_desc[2] == NULL) {
		struct vnode* v3;
		char c3[] = "con:";
		curproc->file_desc[2] = kmalloc(sizeof(struct file));
		if (curproc->file_desc[2] == NULL) {
			return ENOMEM;
		}
		result = vfs_open(c3, O_WRONLY, 0644, &v3);
		if (result) {
			free_file(&curproc->file_desc[2]);
			return result;
		}
		curproc->file_desc[2]->vnode = v3;
		curproc->file_desc[2]->flag = O_WRONLY;
		curproc->file_desc[2]->lock = lock_create("stderr");
		if (curproc->file_desc[2]->lock == NULL) {
			free_file(&curproc->file_desc[2]);
			return ENOMEM;
		}
		curproc->file_desc[2]->offset = 0;
		curproc->file_desc[2]->ref = 1;
	}

	if (overLock == NULL) {
		overLock = lock_create("Over_Lock");
		if (overLock == NULL) {
			free_file(&curproc->file_desc[0]);
			free_file(&curproc->file_desc[1]);
			free_file(&curproc->file_desc[2]);
			return ENOMEM;
		}
	}
	return 0;
}
/*static int std_close() {
	int err;
	err = free_file(&curproc->file_desc[0]);
	if (err) { return err; }
	err = free_file(&curproc->file_desc[1]);
	if (err) { return err; }
	err =free_file(&curproc->file_desc[2]);
	if (err) { return err; }
	return 0;
}*/
int
runprogram(char *progname)
{
	int result = 0;
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	
	//init std;
	result = std_init();
	if (result) {
		return result;
	}

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}
	

	/* We should be a new process. */
	KASSERT(proc_getas() == NULL);



	/* Create a new address space. */
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	proc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);


	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}
	

	/* Warp to user mode. */
	enter_new_process(0 /*argc*/, NULL /*userspace addr of argv*/,
			  NULL /*userspace addr of environment*/,
			  stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	//std_close();
	return EINVAL;
}

