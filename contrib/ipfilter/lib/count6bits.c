/*	$FreeBSD: src/contrib/ipfilter/lib/count6bits.c,v 1.2.2.2.4.1 2008/10/02 02:57:24 kensmith Exp $	*/

/*
 * Copyright (C) 2000-2001 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: count6bits.c,v 1.4.4.1 2006/06/16 17:20:57 darrenr Exp $
 */

#include "ipf.h"


int count6bits(msk)
u_32_t *msk;
{
	int i = 0, k;
	u_32_t j;

	for (k = 3; k >= 0; k--)
		if (msk[k] == 0xffffffff)
			i += 32;
		else {
			for (j = msk[k]; j; j <<= 1)
				if (j & 0x80000000)
					i++;
		}
	return i;
}
