#if defined(LIBC_SCCS) && !defined(lint)
static char *rcsid = "$FreeBSD: src/lib/libc/gen/shmctl.c,v 1.3.2.1 1999/08/29 14:46:19 peter Exp $";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#if __STDC__
int shmctl(int shmid, int cmd, struct shmid_ds *buf)
#else
int shmctl(shmid, cmd, buf)
	int shmid;
	int cmd;
	struct shmid_ds *buf;
#endif
{
	return (shmsys(4, shmid, cmd, buf));
}
