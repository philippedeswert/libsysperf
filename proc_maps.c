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
 * File: proc_maps.c
 *
 * Author: Simo Piiroinen
 *
 * History:
 *
 * 21-Sep-2006 Simo Piiroinen
 * - fixed include paths
 *
 * 29-May-2006 Simo Piiroinen
 * - modified to use libsysperf API
 * - moved from track2 source tree
 * ========================================================================= */

#include <stdio.h>

#include "xmalloc.h"
#include "cstring.h"
#include "proc_maps.h"

/* ------------------------------------------------------------------------- *
 * proc_maps_ctor
 * ------------------------------------------------------------------------- */

void
proc_maps_ctor(proc_maps_t *self,
               unsigned begin, unsigned end,
               char *prot, char *path)
{
  self->begin = begin;
  self->end   = end;
  self->prot  = xstrdup(prot);
  self->path  = xstrdup(path);
}

/* ------------------------------------------------------------------------- *
 * proc_maps_dtor
 * ------------------------------------------------------------------------- */

void
proc_maps_dtor(proc_maps_t *self)
{
  xstrset(&self->prot, 0);
  xstrset(&self->path, 0);
}

/* ------------------------------------------------------------------------- *
 * proc_maps_create
 * ------------------------------------------------------------------------- */

proc_maps_t *
proc_maps_create(unsigned begin, unsigned end,
                 char *prot, char *path)
{
  proc_maps_t *self = xcalloc(1, sizeof *self);

  proc_maps_ctor(self, begin, end, prot, path);
  return self;
}

/* ------------------------------------------------------------------------- *
 * proc_maps_delete
 * ------------------------------------------------------------------------- */

void
proc_maps_delete(proc_maps_t *self)
{
  if( self != 0 )
  {
    proc_maps_dtor(self);
    xfree(self);
  }
}

/* ------------------------------------------------------------------------- *
 * proc_maps_cmp_begin
 * ------------------------------------------------------------------------- */

int
proc_maps_cmp_begin(const void *a, const void *b)
{
  const proc_maps_t *A = (const proc_maps_t *)a;
  const proc_maps_t *B = (const proc_maps_t *)b;
  return (int)(A->begin - B->begin);
}

/* ------------------------------------------------------------------------- *
 * proc_maps_cmp_begin_indirect
 * ------------------------------------------------------------------------- */

int
proc_maps_cmp_begin_indirect(const void *a, const void *b)
{
  const proc_maps_t *A = *(const proc_maps_t **)a;
  const proc_maps_t *B = *(const proc_maps_t **)b;
  return (int)(A->begin - B->begin);
}

/* ------------------------------------------------------------------------- *
 * proc_maps_parse
 * ------------------------------------------------------------------------- */

void
proc_maps_parse(array_t *arr, const char *path)
{
  char *data = cstring_from_file(path);

  if( data != 0 )
  {
    char *line = data;

    while( *line != 0 )
    {
      char *pos = cstring_split_at_char(line,&line, '\n');

      char *b = cstring_split_at_char(pos, &pos, '-');
      char *e = cstring_split_at_white(pos, &pos);
      char *p = cstring_split_at_white(pos, &pos);

      char *f;

      cstring_split_at_white(pos, &pos);
      cstring_split_at_white(pos, &pos);
      cstring_split_at_white(pos, &pos);

      f = cstring_split_at_white(pos, &pos);

      array_add(arr, proc_maps_create(strtoul(b,0,16), strtoul(e,0,16), p, f));
    }
    xfree(data);
  }
}

/* ------------------------------------------------------------------------- *
 * proc_maps_repr
 * ------------------------------------------------------------------------- */

void
proc_maps_repr(const proc_maps_t *obj, FILE *out)
{
  fprintf(out, "%08x-%08x %s %s\n",
          obj->begin, obj->end,
          obj->prot, obj->path);
}
