/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char const copyright[] =
"@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)mksyntax.c	8.2 (Berkeley) 5/4/95";
#endif
static const char rcsid[] =
  "$FreeBSD: src/bin/sh/mksyntax.c,v 1.12.2.2 2000/03/20 10:51:03 cracauer Exp $";
#endif /* not lint */

/*
 * This program creates syntax.h and syntax.c.
 */

#include <stdio.h>
#include <string.h>
#include "parser.h"


struct synclass {
	char *name;
	char *comment;
};

/* Syntax classes */
struct synclass synclass[] = {
	{ "CWORD",	"character is nothing special" },
	{ "CNL",	"newline character" },
	{ "CBACK",	"a backslash character" },
	{ "CSQUOTE",	"single quote" },
	{ "CDQUOTE",	"double quote" },
	{ "CENDQUOTE",	"a terminating quote" },
	{ "CBQUOTE",	"backwards single quote" },
	{ "CVAR",	"a dollar sign" },
	{ "CENDVAR",	"a '}' character" },
	{ "CLP",	"a left paren in arithmetic" },
	{ "CRP",	"a right paren in arithmetic" },
	{ "CEOF",	"end of file" },
	{ "CCTL",	"like CWORD, except it must be escaped" },
	{ "CSPCL",	"these terminate a word" },
	{ NULL,		NULL }
};


/*
 * Syntax classes for is_ functions.  Warning:  if you add new classes
 * you may have to change the definition of the is_in_name macro.
 */
struct synclass is_entry[] = {
	{ "ISDIGIT",	"a digit" },
	{ "ISUPPER",	"an upper case letter" },
	{ "ISLOWER",	"a lower case letter" },
	{ "ISUNDER",	"an underscore" },
	{ "ISSPECL",	"the name of a special parameter" },
	{ NULL, 	NULL }
};

static char writer[] = "\
/*\n\
 * This file was generated by the mksyntax program.\n\
 */\n\
\n";


static FILE *cfile;
static FILE *hfile;
static char *syntax[513];
static int base;
static int size;	/* number of values which a char variable can have */
static int nbits;	/* number of bits in a character */
static int digit_contig;/* true if digits are contiguous */

static void filltable __P((char *));
static void init __P((void));
static void add __P((char *, char *));
static void print __P((char *));
static void output_type_macros __P((void));
static void digit_convert __P((void));

int
main(argc, argv)
	int argc __unused;
	char **argv __unused;
{
	char c;
	char d;
	int sign;
	int i;
	char buf[80];
	int pos;
	static char digit[] = "0123456789";

	/* Create output files */
	if ((cfile = fopen("syntax.c", "w")) == NULL) {
		perror("syntax.c");
		exit(2);
	}
	if ((hfile = fopen("syntax.h", "w")) == NULL) {
		perror("syntax.h");
		exit(2);
	}
	fputs(writer, hfile);
	fputs(writer, cfile);

	/* Determine the characteristics of chars. */
	c = -1;
	if (c < 0)
		sign = 1;
	else
		sign = 0;
	for (nbits = 1 ; ; nbits++) {
		d = (1 << nbits) - 1;
		if (d == c)
			break;
	}
#if 0
	printf("%s %d bit chars\n", sign? "signed" : "unsigned", nbits);
#endif
	if (nbits > 9) {
		fputs("Characters can't have more than 9 bits\n", stderr);
		exit(2);
	}
	size = (1 << nbits) + 1;
	base = 1;
	if (sign)
		base += 1 << (nbits - 1);
	digit_contig = 1;
	for (i = 0 ; i < 10 ; i++) {
		if (digit[i] != '0' + i)
			digit_contig = 0;
	}

	fputs("#include <sys/cdefs.h>\n", hfile);
	fputs("#include <ctype.h>\n", hfile);

	/* Generate the #define statements in the header file */
	fputs("/* Syntax classes */\n", hfile);
	for (i = 0 ; synclass[i].name ; i++) {
		sprintf(buf, "#define %s %d", synclass[i].name, i);
		fputs(buf, hfile);
		for (pos = strlen(buf) ; pos < 32 ; pos = (pos + 8) & ~07)
			putc('\t', hfile);
		fprintf(hfile, "/* %s */\n", synclass[i].comment);
	}
	putc('\n', hfile);
	fputs("/* Syntax classes for is_ functions */\n", hfile);
	for (i = 0 ; is_entry[i].name ; i++) {
		sprintf(buf, "#define %s %#o", is_entry[i].name, 1 << i);
		fputs(buf, hfile);
		for (pos = strlen(buf) ; pos < 32 ; pos = (pos + 8) & ~07)
			putc('\t', hfile);
		fprintf(hfile, "/* %s */\n", is_entry[i].comment);
	}
	putc('\n', hfile);
	fprintf(hfile, "#define SYNBASE %d\n", base);
	fprintf(hfile, "#define PEOF %d\n\n", -base);
	putc('\n', hfile);
	fputs("#define BASESYNTAX (basesyntax + SYNBASE)\n", hfile);
	fputs("#define DQSYNTAX (dqsyntax + SYNBASE)\n", hfile);
	fputs("#define SQSYNTAX (sqsyntax + SYNBASE)\n", hfile);
	fputs("#define ARISYNTAX (arisyntax + SYNBASE)\n", hfile);
	putc('\n', hfile);
	output_type_macros();		/* is_digit, etc. */
	putc('\n', hfile);

	/* Generate the syntax tables. */
	fputs("#include \"shell.h\"\n", cfile);
	fputs("#include \"syntax.h\"\n\n", cfile);
	init();
	fputs("/* syntax table used when not in quotes */\n", cfile);
	add("\n", "CNL");
	add("\\", "CBACK");
	add("'", "CSQUOTE");
	add("\"", "CDQUOTE");
	add("`", "CBQUOTE");
	add("$", "CVAR");
	add("}", "CENDVAR");
	add("<>();&| \t", "CSPCL");
	print("basesyntax");
	init();
	fputs("\n/* syntax table used when in double quotes */\n", cfile);
	add("\n", "CNL");
	add("\\", "CBACK");
	add("\"", "CENDQUOTE");
	add("`", "CBQUOTE");
	add("$", "CVAR");
	add("}", "CENDVAR");
	/* ':/' for tilde expansion, '-' for [a\-x] pattern ranges */
	add("!*?[=~:/-", "CCTL");
	print("dqsyntax");
	init();
	fputs("\n/* syntax table used when in single quotes */\n", cfile);
	add("\n", "CNL");
	add("'", "CENDQUOTE");
	/* ':/' for tilde expansion, '-' for [a\-x] pattern ranges */
	add("!*?[=~:/-", "CCTL");
	print("sqsyntax");
	init();
	fputs("\n/* syntax table used when in arithmetic */\n", cfile);
	add("\n", "CNL");
	add("\\", "CBACK");
	add("`", "CBQUOTE");
	add("'", "CSQUOTE");
	add("\"", "CDQUOTE");
	add("$", "CVAR");
	add("}", "CENDVAR");
	add("(", "CLP");
	add(")", "CRP");
	print("arisyntax");
	filltable("0");
	fputs("\n/* character classification table */\n", cfile);
	add("0123456789", "ISDIGIT");
	add("abcdefghijklmnopqrstucvwxyz", "ISLOWER");
	add("ABCDEFGHIJKLMNOPQRSTUCVWXYZ", "ISUPPER");
	add("_", "ISUNDER");
	add("#?$!-*@", "ISSPECL");
	print("is_type");
	if (! digit_contig)
		digit_convert();
	exit(0);
}



/*
 * Clear the syntax table.
 */

static void
filltable(dftval)
	char *dftval;
{
	int i;

	for (i = 0 ; i < size ; i++)
		syntax[i] = dftval;
}


/*
 * Initialize the syntax table with default values.
 */

static void
init()
{
	filltable("CWORD");
	syntax[0] = "CEOF";
	syntax[base + CTLESC] = "CCTL";
	syntax[base + CTLVAR] = "CCTL";
	syntax[base + CTLENDVAR] = "CCTL";
	syntax[base + CTLBACKQ] = "CCTL";
	syntax[base + CTLBACKQ + CTLQUOTE] = "CCTL";
	syntax[base + CTLARI] = "CCTL";
	syntax[base + CTLENDARI] = "CCTL";
	syntax[base + CTLQUOTEMARK] = "CCTL";
}


/*
 * Add entries to the syntax table.
 */

static void
add(p, type)
	char *p, *type;
{
	while (*p)
		syntax[*p++ + base] = type;
}



/*
 * Output the syntax table.
 */

static void
print(name)
	char *name;
{
	int i;
	int col;

	fprintf(hfile, "extern const char %s[];\n", name);
	fprintf(cfile, "const char %s[%d] = {\n", name, size);
	col = 0;
	for (i = 0 ; i < size ; i++) {
		if (i == 0) {
			fputs("      ", cfile);
		} else if ((i & 03) == 0) {
			fputs(",\n      ", cfile);
			col = 0;
		} else {
			putc(',', cfile);
			while (++col < 9 * (i & 03))
				putc(' ', cfile);
		}
		fputs(syntax[i], cfile);
		col += strlen(syntax[i]);
	}
	fputs("\n};\n", cfile);
}



/*
 * Output character classification macros (e.g. is_digit).  If digits are
 * contiguous, we can test for them quickly.
 */

static char *macro[] = {
	"#define is_digit(c)\t((c >= 0 && is_type+SYNBASE)[c] & ISDIGIT)",
	"#define is_alpha(c)\t((c) != PEOF && ((c) < CTLESC || (c) > CTLENDARI) && isalpha((unsigned char) (c)))",
	"#define is_name(c)\t((c) != PEOF && ((c) < CTLESC || (c) > CTLENDARI) && ((c) == '_' || isalpha((unsigned char) (c))))",
	"#define is_in_name(c)\t((c) != PEOF && ((c) < CTLESC || (c) > CTLENDARI) && ((c) == '_' || isalnum((unsigned char) (c))))",
	"#define is_special(c)\t((is_type+SYNBASE)[c] & (ISSPECL|ISDIGIT))",
	NULL
};

static void
output_type_macros()
{
	char **pp;

	if (digit_contig)
		macro[0] = "#define is_digit(c)\t((unsigned)((c) - '0') <= 9)";
	for (pp = macro ; *pp ; pp++)
		fprintf(hfile, "%s\n", *pp);
	if (digit_contig)
		fputs("#define digit_val(c)\t((c) - '0')\n", hfile);
	else
		fputs("#define digit_val(c)\t(digit_value[c])\n", hfile);
}



/*
 * Output digit conversion table (if digits are not contiguous).
 */

static void
digit_convert()
{
	int maxdigit;
	static char digit[] = "0123456789";
	char *p;
	int i;

	maxdigit = 0;
	for (p = digit ; *p ; p++)
		if (*p > maxdigit)
			maxdigit = *p;
	fputs("extern const char digit_value[];\n", hfile);
	fputs("\n\nconst char digit_value[] = {\n", cfile);
	for (i = 0 ; i <= maxdigit ; i++) {
		for (p = digit ; *p && *p != i ; p++);
		if (*p == '\0')
			p = digit;
		fprintf(cfile, "      %d,\n", p - digit);
	}
	fputs("};\n", cfile);
}
