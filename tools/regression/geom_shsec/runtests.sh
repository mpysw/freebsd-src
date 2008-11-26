#!/bin/sh
# $FreeBSD: src/tools/regression/geom_shsec/runtests.sh,v 1.1.14.1 2008/10/02 02:57:24 kensmith Exp $

dir=`dirname $0`

gshsec load >/dev/null 2>&1
for ts in `dirname $0`/test-*.sh; do
	sh $ts
done
gshsec unload
