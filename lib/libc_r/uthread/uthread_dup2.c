/*
 * Copyright (c) 1995-1998 John Birrell <jb@cimlogic.com.au>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by John Birrell.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JOHN BIRRELL AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libc_r/uthread/uthread_dup2.c,v 1.4.2.1 2000/01/04 10:03:30 tg Exp $
 */
#include <errno.h>
#include <unistd.h>
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"

int
dup2(int fd, int newfd)
{
	int             ret;
	int		newfd_opened;

	/* Check if the file descriptor is out of range: */
	if (newfd < 0 || newfd >= _thread_dtablesize) {
		/* Return a bad file descriptor error: */
		errno = EBADF;
		ret = -1;
	}

	/* Lock the file descriptor: */
	else if ((ret = _FD_LOCK(fd, FD_RDWR, NULL)) == 0) {
		/* Lock the file descriptor: */
		if (!(newfd_opened = (_thread_fd_table[newfd] != NULL)) || 
		    (ret = _FD_LOCK(newfd, FD_RDWR, NULL)) == 0) {
			/* Perform the 'dup2' syscall: */
			if ((ret = _thread_sys_dup2(fd, newfd)) < 0) {
			}
			/* Initialise the file descriptor table entry: */
			else if (_thread_fd_table_init(ret) != 0) {
				/* Quietly close the file: */
				_thread_sys_close(ret);

				/* Reset the file descriptor: */
				ret = -1;
			} else {
				/*
				 * Save the file open flags so that they can
				 * be checked     later: 
				 */
				_thread_fd_table[ret]->flags = _thread_fd_table[fd]->flags;
			}

			/* Unlock the file descriptor: */
			if (newfd_opened)
				_FD_UNLOCK(newfd, FD_RDWR);
		}
		/* Unlock the file descriptor: */
		_FD_UNLOCK(fd, FD_RDWR);
	}
	/* Return the completion status: */
	return (ret);
}
#endif
