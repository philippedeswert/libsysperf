/*
 * This file is part of libsysperf
 *
 * Copyright (C) 2001, 2004-2007 by Nokia Corporation.
 *
 * Contact: Eero Tamminen <eero.tamminen@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/* ========================================================================= *
 * File: fake_csv_pass.c
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 21-Jun-2005 Simo Piiroinen
 * - added command line parsing & runtime help
 * - initial version
 * ========================================================================= */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include "msg.h"
#include "argvec.h"
#include "csv_table.h"

/* ========================================================================= *
 * Configuration
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * Tool Version
 * ------------------------------------------------------------------------- */

#define TOOL_NAME "fake_csv_pass"
#define TOOL_VERS "0.4.2"

/* ------------------------------------------------------------------------- *
 * Runtime Manual
 * ------------------------------------------------------------------------- */

static const manual_t app_man[]=
{
  MAN_ADD("NAME",
          TOOL_NAME"\n"
          )
  MAN_ADD("SYNOPSIS",
          ""TOOL_NAME" <options>\n"
          )
  MAN_ADD("DESCRIPTION",
          "This tool reads and write CSV format file for library\n"
          "testing purposes.\n"
          )
  MAN_ADD("OPTIONS", 0)

  MAN_ADD("EXAMPLES",
          "% "TOOL_NAME" -ifoo.csv -obar.csv\n"
          )
  MAN_ADD("COPYRIGHT",
          "Copyright (C) 2001, 2004-2007 Nokia Corporation.\n\n"
          "This is free software.  You may redistribute copies of it under the\n"
          "terms of the GNU General Public License v2 included with the software.\n"
          "There is NO WARRANTY, to the extent permitted by law.\n"
          )
  MAN_ADD("SEE ALSO",
          "\n"
          )
  MAN_END
};

/* ------------------------------------------------------------------------- *
 * Commandline Arguments
 * ------------------------------------------------------------------------- */

enum
{
  opt_noswitch = -1,
  opt_help,
  opt_vers,

  opt_input,
  opt_output,
};

static const option_t app_opt[] =
{
  OPT_ADD(opt_help,
          "h", "help", 0,
          "This help text\n"),

  OPT_ADD(opt_vers,
          "V", "version", 0,
          "Tool version\n"),

  OPT_ADD(opt_input,
          "i", "input", "<source path>",
          "Input file to use instead of stdin.\n" ),

  OPT_ADD(opt_output,
          "o", "output", "<destination path>",
          "Output file to use instead of stdout.\n" ),

  OPT_END
};

/* ========================================================================= *
 * Main Entry Point
 * ========================================================================= */

int main(int ac, char **av)
{
  argvec_t *args = argvec_create(ac, av, app_opt, app_man);
  int       tag  = 0;
  char     *par  = 0;

  char     *input  = 0;
  char     *output = 0;

  while( !argvec_done(args) )
  {
    if( !argvec_next(args, &tag, &par) )
    {
      msg_error("(use --help for usage)\n");
      exit(1);
    }

    switch( tag )
    {
    case opt_help:
      argvec_usage(args);
      exit(0);

    case opt_vers:
      printf("%s\n", TOOL_VERS);
      exit(0);

    case opt_input:
      input = par;
      break;

    case opt_output:
      output = par;
      break;

    default:
      printf("tag=%d, par='%s'\n", tag, par);
      break;
    }
  }

  if( input == 0 && output == 0 && isatty(STDIN_FILENO) )
  {
    msg_fatal("refusing to filter stdin -> stdout interactively\n"
              "(use --help for usage)\n");
  }

  clock_t t;

  msg_debug("%s: create\n", __FUNCTION__);

  csv_tab_t *tab = csv_tab_create();

  msg_debug("%s: load\n", __FUNCTION__);
  t = clock();
  csv_tab_load(tab, input);
  t = clock() - t;
  fprintf(stderr, "load\t%.3f s\n", t * 1e-6);

#if 01
  msg_debug("%s: save\n", __FUNCTION__);
  t = clock();
  csv_tab_save(tab, output);
  t = clock() - t;
  fprintf(stderr, "save\t%.3f s\n", t * 1e-6);
#endif

#if 01
  msg_debug("%s: delete\n", __FUNCTION__);
  t = clock();
  csv_tab_delete(tab);
  t = clock() - t;
  fprintf(stderr, "del\t%.3f s\n", t * 1e-6);
#endif

  msg_debug("%s: done\n", __FUNCTION__);

  argvec_delete(args);

  return 0;
}
