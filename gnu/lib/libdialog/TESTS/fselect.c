/*
 * small test-driver for new dialog functionality
 *
 * Copyright (c) 1995, Jordan Hubbard
 *
 * All rights reserved.
 *
 * This source code may be used, modified, copied, distributed, and
 * sold, in both source and binary form provided that the above
 * copyright and these terms are retained, verbatim, as the first
 * lines of this file.  Under no circumstances is the author
 * responsible for the proper functioning of the software nor does
 * the author assume any responsibility for damages incurred with
 * its use.
 *
 * $FreeBSD: src/gnu/lib/libdialog/TESTS/fselect.c,v 1.3.2.2 2000/04/24 12:05:59 phantom Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dialog.h>

/* Kick it off, James! */
int
main(int argc, char *argv[])
{
  char *retval;

  init_dialog();

  retval = dialog_fselect(".", "*.[ch]");
  dialog_clear();
  if (retval)
    fprintf(stderr, "returned value for dialog_fselect was %s\n", retval);
  else
    fprintf(stderr, "returned value for dialog_fselect was NULL\n");

  end_dialog();
  return 0;
}
