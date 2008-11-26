/*	$NetBSD: fpgetround.c,v 1.2 2002/01/13 21:45:50 thorpej Exp $	*/

/*
 * Written by J.T. Conklin, Apr 10, 1995
 * Public domain.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/sparc64/gen/fpgetround.c,v 1.1.30.1 2008/10/02 02:57:24 kensmith Exp $");

#include <machine/fsr.h>
#include <ieeefp.h>

fp_rnd_t
fpgetround()
{
	unsigned int x;

	__asm__("st %%fsr,%0" : "=m" (x));
	return ((fp_rnd_t)FSR_GET_RD(x));
}
