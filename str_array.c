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
 * File: str_array.c
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 28-Jun-2005 Simo Piiroinen
 * - imported from track2
 *
 * 10-Jan-2005 Simo Piiroinen
 * - added str_array_exclude
 * ========================================================================= */

#include "str_array.h"
#include "cstring.h"

/* ------------------------------------------------------------------------- *
 * str_array_create
 * ------------------------------------------------------------------------- */

str_array_t *
str_array_create(void)
{
  str_array_t *self = xcalloc(1, sizeof *self);
  str_array_ctor(self);
  return self;
}

/* ------------------------------------------------------------------------- *
 * str_array_delete
 * ------------------------------------------------------------------------- */

void
str_array_delete(str_array_t *self)
{
  if( self != 0 )
  {
    str_array_dtor(self);
    xfree(self);
  }
}

/* ------------------------------------------------------------------------- *
 * str_array_add
 * ------------------------------------------------------------------------- */

void
str_array_add(str_array_t *self, const char *str)
{
  array_add(self, xstrdup(str));
}

/* ------------------------------------------------------------------------- *
 * str_array_set
 * ------------------------------------------------------------------------- */

void
str_array_set(str_array_t *self, size_t index, const char *str)
{
  array_set(self, index, xstrdup(str));
}

/* ------------------------------------------------------------------------- *
 * str_array_index
 * ------------------------------------------------------------------------- */

int
str_array_index(const str_array_t *self, const char *str)
{
  size_t i;
  for( i = 0; i < self->size; ++i )
  {
    if( !cstring_compare((char *)self->data[i], str) ) return (int)i;
  }
  return -1;
}

/* ------------------------------------------------------------------------- *
 * str_array_exclude
 * ------------------------------------------------------------------------- */

size_t
str_array_exclude(str_array_t *self, const str_array_t *excl)
{
  size_t n = 0;
  for( size_t i = 0; i < self->size; ++i )
  {
    const char *s = self->data[i];
    if( str_array_index(excl, s) != -1 )
    {
      array_del_nocompact(self, i);
      n++;
    }
  }
  if( n != 0 )
  {
    array_compact(self);
  }
  return n;
}
