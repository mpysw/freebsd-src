/****************************************************************************
 *
 * Simple diff mode test.
 *
 * $FreeBSD: src/lib/libpthread/test/hello_d.c,v 1.1.28.1 2008/10/02 02:57:24 kensmith Exp $
 *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <pthread.h>

void *
entry(void * a_arg)
{
	fprintf(stderr, "Hello world\n");

	return NULL;
}

int
main()
{
	pthread_t thread;
	int error;

	error = pthread_create(&thread, NULL, entry, NULL);
	if (error)
		fprintf(stderr, "Error in pthread_create(): %s\n",
			strerror(error));

	error = pthread_join(thread, NULL);
	if (error)
		fprintf(stderr, "Error in pthread_join(): %s\n",
			strerror(error));

	return 0;
}
