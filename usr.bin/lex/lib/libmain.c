/* libmain - flex run-time support library "main" function */

/* $Header: /home/daffy/u0/vern/flex/RCS/libmain.c,v 1.4 95/09/27 12:47:55 vern Exp $
 * $FreeBSD: src/usr.bin/lex/lib/libmain.c,v 1.3.50.1 2008/10/02 02:57:24 kensmith Exp $ */

extern int yylex();

int main( argc, argv )
int argc;
char *argv[];
	{
	while ( yylex() != 0 )
		;

	return 0;
	}
