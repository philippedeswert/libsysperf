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
 * File: argvec.c
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 21-Jun-2005 Simo Piiroinen
 * - initial version
 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "msg.h"
#include "argvec.h"

/* ========================================================================= *
 * argvec_t
 * ========================================================================= */

int argvec_done(argvec_t *self)
{
  return self->av_iarg == self->av_argc;
}

/* ------------------------------------------------------------------------- *
 * argvec_create
 * ------------------------------------------------------------------------- */

argvec_t *argvec_create(int argc, char **argv,
                        const option_t *opt,
                        const manual_t *man)
{
  argvec_t *self = calloc(1, sizeof *self);

  self->av_argc = argc;
  self->av_argv = argv;
  self->av_iarg = 1;
  self->av_ichr = 0;
  self->av_done = 0;

  self->av_opt  = opt;
  self->av_man  = man;

  return self;
}

/* ------------------------------------------------------------------------- *
 * argvec_delete
 * ------------------------------------------------------------------------- */

void argvec_delete(argvec_t *self)
{
  free(self);
}

/* ------------------------------------------------------------------------- *
 * argvec_options
 * ------------------------------------------------------------------------- */

void
argvec_options(argvec_t *self, int verbose)
{
  static const char sep[] = " | ";

  const char *s;

  const option_t *spec = self->av_opt;

  for( ; spec->op_tag >= 0; ++spec )
  {
    int i = 0;
    printf("  ");
    if( (s = spec->op_short) != 0 )
    {
      for( ; *s; ++s )
      {
        printf("%s-%c", i++ ? sep : "", *s);
      }
      if( (s = spec->op_param) != 0 )
      {
        // optional arguments for short switches
        // must be right after the switch character!
        printf("%s%s", (*s=='[') ? "" : " ", s);
      }
    }
    if( (s = spec->op_long) != 0 )
    {
      printf("%s--%s", i++ ? sep : "", s);

      if( (s = spec->op_param) != 0 )
      {
        printf("=%s", s);
      }
    }
    printf("\n");

    if( verbose && spec->op_usage )
    {
      const char *beg = spec->op_usage;
      const char *pos = spec->op_usage;
      int         ind = strspn(beg, " \t");

      while( *pos != 0 )
      {
        int len = strcspn(pos, "\n");

        if( len > 0 )
        {
          fputc('\t', stdout);
          fwrite(beg, ind, 1, stdout);
          fwrite(pos, len, 1, stdout);
        }
        printf("\n");
        pos += len;
        if( *pos ) ++pos;
      }
    }
  }
}

/* ------------------------------------------------------------------------- *
 * argvec_usage
 * ------------------------------------------------------------------------- */

void argvec_usage(argvec_t *self)
{
  const manual_t *man = self->av_man;

  if( man == 0 )
  {
    argvec_options(self, 1);
  }
  else for( ; man->man_sect; ++man )
  {
    printf("%s\n", man->man_sect);

    if( man->man_text )
    {
      const char *s = man->man_text;
      for( ;; )
      {
        int n = strcspn(s,"\n");
        printf("  %.*s\n", n, s);
        s += n;
        if( *s != 0 ) ++s;
        if( *s == 0 ) break;
      }
    }
    else
    {
      argvec_options(self, 1);
    }
    printf("\n");
  }
}

/* ------------------------------------------------------------------------- *
 * argvec_pull
 * ------------------------------------------------------------------------- */

char *
argvec_pull(argvec_t *self)
{
  if( self->av_iarg < self->av_argc )
  {
    return self->av_argv[self->av_iarg++];
  }
  return 0;
}

/* ------------------------------------------------------------------------- *
 * argvec_next
 * ------------------------------------------------------------------------- */

int
argvec_next(argvec_t *self, int *ptag, char **pstr)
{
  const option_t *spec = self->av_opt;
  char *pos = 0;
  char *val = 0;

  /* - - - - - - - - - - - - - - - - - - - *
   * all done
   * - - - - - - - - - - - - - - - - - - - */

  if( self->av_iarg >= self->av_argc )
  {
    self->av_done = 1;
    return 0;
  }

  pos = &self->av_argv[self->av_iarg][self->av_ichr];

  /* - - - - - - - - - - - - - - - - - - - *
   * at the start of new argument?
   * - - - - - - - - - - - - - - - - - - - */

  if( self->av_ichr == 0 )
  {
    /* - - - - - - - - - - - - - - - - - - - *
     * just "-" or non-switch argument ?
     * - - - - - - - - - - - - - - - - - - - */

    if( pos[0] != '-' || pos[1] == 0 )
    {
      *ptag = -1, *pstr = pos;
      self->av_iarg += 1, self->av_ichr = 0;
      return 1;
    }

    /* - - - - - - - - - - - - - - - - - - - *
     * "--<option>[=<value>] argument ?
     * - - - - - - - - - - - - - - - - - - - */

    if( pos[1] == '-' )
    {
      pos += 2; // skip the initial '--'

      if( (val = strchr(pos, '=')) != 0 )
      {
        *val++ = 0;
      }

      for( ; spec->op_tag >= 0; ++spec )
      {
        if( spec->op_long && !strcmp(pos, spec->op_long) )
        {
          *ptag = spec->op_tag;
          *pstr = val;

          if( spec->op_param && !val && *spec->op_param != '[' )
          {
            // ERR: required value for option is missing
            msg_error("--%s requires %s argument\n",
                    spec->op_long, spec->op_param);
            return 0;
          }

          if( !spec->op_param && val )
          {
            // ERR: unneeded value for option
            msg_error("--%s does not take an argument\n",
                    spec->op_long);
            return 0;
          }

          // OK: long option [with value]
          self->av_iarg += 1, self->av_ichr = 0;
          return 1;
        }
      }
      // ERR: unknown long switch
      msg_error("unknown switch --%s\n", pos);
      return 0;
    }

    /* - - - - - - - - - - - - - - - - - - - *
     * set of single char switches
     * - - - - - - - - - - - - - - - - - - - */

    pos += 1; // skip the initial '-'
  }

  // -<optionchar>
  // -<optionchar><optionvalue>
  // -<optionchar> <optionvalue>
  //
  for( ; spec->op_tag >= 0; ++spec )
  {
    if( spec->op_short && strchr(spec->op_short, *pos) )
    {
      // update charpos -> past this switch
      self->av_ichr = ++pos - self->av_argv[self->av_iarg];

      // assume switch without option
      *ptag = spec->op_tag;
      *pstr = 0;

      if( spec->op_param )
      {
        // next time start from next argv position
        self->av_iarg += 1, self->av_ichr = 0;

        if( *pos != 0 )
        {
          // rest of the argument used as option value
          *pstr = pos;
        }
        else if( *spec->op_param == '[' )
        {
          // optional arguments for short switches
          // must be right after the switch character!
        }
        else if( self->av_iarg >= self->av_argc )
        {
          // ERR: required value for option is missing
          msg_error("-%c requires %s argument\n",
                  *spec->op_short, spec->op_param);
          return 0;
        }
        else
        {
          // next argument used as option value
          *pstr = self->av_argv[self->av_iarg];
          self->av_iarg += 1, self->av_ichr = 0;
        }
      }
      else if( *pos == 0 )
      {
        // step to next argument
        self->av_iarg += 1, self->av_ichr = 0;
      }
      return 1;
    }
  }

  msg_error("unknown switch -%c (0x%02x)\n",
          *pos, (unsigned char)*pos);
  return 0;
}
