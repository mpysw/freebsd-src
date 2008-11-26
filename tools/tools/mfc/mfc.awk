#!/usr/bin/awk -f
#
# $FreeBSD: src/tools/tools/mfc/mfc.awk,v 1.1.12.1 2008/10/02 02:57:24 kensmith Exp $
#

BEGIN {
	CVSROOT="ncvs:/home/ncvs"
	UPDATEOPTS="-kk"
}

/^>/ {
	sub(">[ 	]*", "")
}

/^Revision/ || $1 == "" {
	next
}

{
	if (sub("1.", "") != 1)
		next
	if (!(match($2, "\\+[0-9]") && match($3, "-[0-9]")))
		next
	printf("cvs -d %s update %s -j 1.%d -j 1.%d %s\n", CVSROOT, UPDATEOPTS, $1 - 1, $1, $4)
	files = files " " $4
}

END {
	printf("cvs -d %s commit %s\n", CVSROOT, files);
}
