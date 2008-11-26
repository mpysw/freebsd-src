/* $NetBSD: gtdf2.c,v 1.1 2000/06/06 08:15:05 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/softfloat/gtdf2.c,v 1.1.16.1 2008/10/02 02:57:24 kensmith Exp $");

flag __gtdf2(float64, float64);

flag
__gtdf2(float64 a, float64 b)
{

	/* libgcc1.c says a > b */
	return float64_lt(b, a);
}
