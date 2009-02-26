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
 * File: sp_csv_filter.c
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 21-Sep-2006 Simo Piiroinen
 * - fixed include paths
 *
 * 23-May-2006 Simo Piiroinen
 * - actual filtering code moved to libsysperf
 *
 * 05-Jan-2006 Simo Piiroinen
 * - added 'origin' operator -> subtracts value on first row from column
 *
 * 22-Jul-2005 Simo Piiroinen
 * - csv API cleanup
 *
 * 14-Jul-2005 Simo Piiroinen
 * - string values can be used in calculations
 *
 * 05-Jul-2005 Simo Piiroinen
 * - default operation: one past last '_' in progname instead of skipping
 *   initial "csv_" if present
 * - renamed: app_template.c -> sp_csv_filter.c
 *
 * 29-Jun-2005 Simo Piiroinen
 * - added opearions: header, labels
 * - added options: --verbose, --quiet, --silent
 * - EXIT_FAILURE if load or save fails
 *
 * 28-Jun-2005 Simo Piiroinen
 * - added opearions: calc, select, unique, usecols, order and reverse
 * - added options: --no-header, --no-labels, --data-only
 * - initial version
 * ========================================================================= */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include "msg.h"
#include "argvec.h"
#include "csv_table.h"
#include "str_array.h"

/* ========================================================================= *
 * Configuration
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * Tool Version
 * ------------------------------------------------------------------------- */

#define TOOL_NAME "sp_csv_filter"
#include "release.h"

/* ------------------------------------------------------------------------- *
 * Runtime Manual
 * ------------------------------------------------------------------------- */

static const manual_t app_man[]=
{
  MAN_ADD("NAME",
          TOOL_NAME"\n"
          )
  MAN_ADD("SYNOPSIS",
          ""TOOL_NAME" [options] [operations] \n"
          )
  MAN_ADD("DESCRIPTION",
          "This tool allows making various operations on CSV format data.\n"
          )
  MAN_ADD("OPTIONS", 0)

  MAN_ADD("EXAMPLES",
          "% "TOOL_NAME" -ifoo.csv -obar.csv --data-only \\\n"
          "          :select:pid>1000 :uniq:pid,app\n"
          "\n"
          "  Selects rows with <pid> columns larger than 1000 and writes out\n"
          "  unique combinations of <pid> and <app> column values omitting\n"
          "  headers, labels and empty line separators.\n"
          )

  MAN_ADD("OPERATIONS",
          ":calc:<expr>\n"
          ":select:<expr>\n"
          ":sort:[label,...]]\n"
          ":uniq:[label,...]]\n"
          ":usecols:<label,...>\n"
          ":remcols:<label,...>\n"
          ":order:<label,...>\n"
          ":origin:<label,...>\n"
          ":reverse:\n"
          ":header:\n"
          ":labels:\n"
          "\n"
          "Default operation is derived from binary name, thus calling\n"
          "this tool via link named 'csv_select' makes\n"
          "  % csv_select 'pid<10'\n"
          "equal to\n"
          "  % "TOOL_NAME" :select:'pid<10'\n"
          "\n"
          "The operations are executed after the data has been read in the\n"
          "same order as specified on command line\n"
          "\n"
          "Note that you should escape of quote chars that have special\n"
          "meaning for shell.\n"
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

  opt_verbose,
  opt_quiet,
  opt_silent,

  opt_input,
  opt_output,

  opt_no_header,
  opt_no_labels,
  opt_data_only,
};

static const option_t app_opt[] =
{
  /* - - - - - - - - - - - - - - - - - - - *
   * usage, version & verbosity
   * - - - - - - - - - - - - - - - - - - - */

  OPT_ADD(opt_help,
          "h", "help", 0,
          "This help text\n"),

  OPT_ADD(opt_vers,
          "V", "version", 0,
          "Tool version\n"),

  OPT_ADD(opt_verbose,
          "v", "verbose", 0,
          "Enable diagnostic messages\n"),

  OPT_ADD(opt_quiet,
          "q", "quiet", 0,
          "Disable warning messages\n"),

  OPT_ADD(opt_silent,
          "s", "silent", 0,
          "Disable all messages\n"),

  /* - - - - - - - - - - - - - - - - - - - *
   * application options
   * - - - - - - - - - - - - - - - - - - - */

  OPT_ADD(opt_input,
          "f", "input", "<source path>",
          "Input file to use instead of stdin.\n" ),

  OPT_ADD(opt_output,
          "o", "output", "<destination path>",
          "Output file to use instead of stdout.\n" ),

  OPT_ADD(opt_no_header,
          0, "no-header", 0,
          "Omit header rows from Output.\n" ),

  OPT_ADD(opt_no_labels,
          0, "no-labels", 0,
          "Omit label row from Output.\n" ),

  OPT_ADD(opt_data_only,
          0, "data-only", 0,
          "Output only data rows.\n" ),

  OPT_END
};

/* ========================================================================= *
 * sp_csv_filter_t
 * ========================================================================= */

typedef struct sp_csv_filter_t sp_csv_filter_t;

struct sp_csv_filter_t
{
  char         *input;
  char         *output;
  csv_t        *table;
  str_array_t   expressions;
};

/* ------------------------------------------------------------------------- *
 * sp_csv_filter_ctor
 * ------------------------------------------------------------------------- */

static inline void sp_csv_filter_ctor(sp_csv_filter_t *self)
{
  self->input  = 0;
  self->output = 0;
  self->table  = csv_create();

  str_array_ctor(&self->expressions);
}

/* ------------------------------------------------------------------------- *
 * sp_csv_filter_dtor
 * ------------------------------------------------------------------------- */

static inline void sp_csv_filter_dtor(sp_csv_filter_t *self)
{
  free(self->input);
  free(self->output);
  csv_delete(self->table);

  array_dtor(&self->expressions);
}

/* ------------------------------------------------------------------------- *
 * sp_csv_filter_create
 * ------------------------------------------------------------------------- */

sp_csv_filter_t *sp_csv_filter_create(void)
{
  sp_csv_filter_t *self = calloc(1, sizeof *self);
  sp_csv_filter_ctor(self);
  return self;
}

/* ------------------------------------------------------------------------- *
 * sp_csv_filter_delete
 * ------------------------------------------------------------------------- */

void sp_csv_filter_delete(sp_csv_filter_t *self)
{
  if( self != 0 )
  {
    sp_csv_filter_dtor(self);
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * sp_csv_filter_sanity
 * ------------------------------------------------------------------------- */

void sp_csv_filter_sanity(sp_csv_filter_t *self)
{
  if( self->input == 0 && self->output == 0 &&
      isatty(STDIN_FILENO) && isatty(STDOUT_FILENO) )
  {
    msg_fatal("refusing to filter stdin -> stdout interactively\n"
              "(use --help for usage)\n");
  }
}

/* ------------------------------------------------------------------------- *
 * sp_csv_filter_load_table
 * ------------------------------------------------------------------------- */

int sp_csv_filter_load_table(sp_csv_filter_t *self)
{
  return csv_load(self->table, self->input);
}

/* ------------------------------------------------------------------------- *
 * sp_csv_filter_save_table
 * ------------------------------------------------------------------------- */

int sp_csv_filter_save_table(sp_csv_filter_t *self)
{
  return csv_save(self->table, self->output);
}

/* ------------------------------------------------------------------------- *
 * sp_csv_filter_add_expression
 * ------------------------------------------------------------------------- */

void sp_csv_filter_add_expression(sp_csv_filter_t *self, char *expr)
{
  str_array_add(&self->expressions, expr);
}

/* ------------------------------------------------------------------------- *
 * sp_csv_filter_handle_expressions
 * ------------------------------------------------------------------------- */

void sp_csv_filter_handle_expressions(sp_csv_filter_t *self)
{
  /* - - - - - - - - - - - - - - - - - - - *
   * default operation derived from
   * executable name
   * - - - - - - - - - - - - - - - - - - - */

  const char *prog = msg_getprogname();
  const char *base = strrchr(prog, '_');
  const char *oper = base ? (base + 1) : prog;

  /* - - - - - - - - - - - - - - - - - - - *
   * execute filters in original order
   * - - - - - - - - - - - - - - - - - - - */

  for( size_t i = 0; i < self->expressions.size; ++i )
  {
    char *expr = str_array_get(&self->expressions, i);
    csv_filter(self->table, expr, oper);
  }
}

/* ------------------------------------------------------------------------- *
 * sp_csv_filter_handle_arguments
 * ------------------------------------------------------------------------- */

void sp_csv_filter_handle_arguments(sp_csv_filter_t *self, int ac, char **av)
{
  argvec_t *args = argvec_create(ac, av, app_opt, app_man);

#define SET_STRING(var,str) do { free(var); (var) = strdup(str); } while(0)

  while( !argvec_done(args) )
  {
    int       tag  = 0;
    char     *par  = 0;

    if( !argvec_next(args, &tag, &par) )
    {
      msg_error("(use --help for usage)\n");
      exit(1);
    }

    switch( tag )
    {
    case opt_help:
      argvec_usage(args);
      exit(EXIT_SUCCESS);

    case opt_vers:
      printf("%s\n", TOOL_VERS);
      exit(EXIT_SUCCESS);

    case opt_verbose:
      msg_incverbosity();
      break;

    case opt_quiet:
      msg_decverbosity();
      break;

    case opt_silent:
      msg_setsilent();
      break;

    case opt_input:
      SET_STRING(self->input, par);
      break;

    case opt_output:
      SET_STRING(self->output, par);
      break;

    case opt_noswitch:
      sp_csv_filter_add_expression(self, par);
      break;

    case opt_no_header:
      self->table->csv_flags |= CTF_NO_HEADER;
      break;

    case opt_no_labels:
      self->table->csv_flags |= CTF_NO_LABELS;
      break;

    case opt_data_only:
      self->table->csv_flags |= (CTF_NO_HEADER | CTF_NO_LABELS |
                              CTF_NO_TERMINATOR);
      break;
    }
  }

#undef SET_STRING

  argvec_delete(args);
}

/* ========================================================================= *
 * Main Entry Point
 * ========================================================================= */

int main(int ac, char **av)
{
  sp_csv_filter_t *app = sp_csv_filter_create();

  sp_csv_filter_handle_arguments(app, ac, av);

  sp_csv_filter_sanity(app);

  if( sp_csv_filter_load_table(app) != 0 )
  {
    exit(EXIT_FAILURE);
  }

  csv_addvar(app->table, "filter", TOOL_NAME" "TOOL_VERS);

  sp_csv_filter_handle_expressions(app);

  if( sp_csv_filter_save_table(app) != 0 )
  {
    exit(EXIT_FAILURE);
  }

  sp_csv_filter_delete(app);

  return 0;
}
