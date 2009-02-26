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
 * File: proc_meminfo.c
 *
 * Author: Simo Piiroinen
 *
 * History:
 *
 * 21-Sep-2006 Simo Piiroinen
 * - fixed include paths
 *
 * 30-May-2006 Simo Piiroinen
 * - moved from track2 source tree
 *
 * 13-Dec-2004 Simo Piiroinen
 * - proc_meminfo_parse now zeroes struct before parsing new values
 * ========================================================================= */

#include "cstring.h"
#include "xmalloc.h"
#include "proc_meminfo.h"

/* ------------------------------------------------------------------------- *
 * proc_meminfo_create
 * ------------------------------------------------------------------------- */

proc_meminfo_t *
proc_meminfo_create(void)
{
  proc_meminfo_t *self = xcalloc(1, sizeof *self);
  proc_meminfo_ctor(self);
  return self;
}

/* ------------------------------------------------------------------------- *
 * proc_meminfo_delete
 * ------------------------------------------------------------------------- */

void
proc_meminfo_delete(proc_meminfo_t *self)
{
  if( self != 0 )
  {
    proc_meminfo_dtor(self);
    xfree(self);
  }
}

/* ------------------------------------------------------------------------- *
 * proc_meminfo_update
 * ------------------------------------------------------------------------- */

void
proc_meminfo_update(proc_meminfo_t *self, char *data)
{
  char *key, *val;

  while( *data )
  {
    key = cstring_split_at_char(data, &data, '\n');
    cstring_split_at_char(key, &val, ':');

#define VAR(name)\
  if( !strcmp(key, #name) ) { \
    self->name = strtoul(val,0,0); \
  } else
#include "proc_meminfo.inc"
    {}
  }
}

/* ------------------------------------------------------------------------- *
 * proc_meminfo_parse
 * ------------------------------------------------------------------------- */

void
proc_meminfo_parse(proc_meminfo_t *self, const char *path)
{
  /* clear old values... just to make sure changes in /proc file contents
   * do not result in uninitialized data */
  memset(self, 0, sizeof *self);

  char *data = cstring_from_file(path);
  if( data != 0 )
  {
    proc_meminfo_update(self, data);
    xfree(data);
  }
}

/* ------------------------------------------------------------------------- *
 * proc_meminfo_repr
 * ------------------------------------------------------------------------- */

void
proc_meminfo_repr(proc_meminfo_t *self, FILE *file)
{
  fprintf(file,
#define VAR(name) #name "=%u "
#include "proc_meminfo.inc"
          "\n"
#define VAR(name) ,self->name
#include "proc_meminfo.inc"
          );
}
