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
 * File: xmalloc.c
 * 
 * Author: Simo Piiroinen
 * ========================================================================= */

/* ========================================================================= *
 * "Safer" versions for stdlib memory allocation functions
 *
 * History:
 *
 * 12-Apr-2001 Simo Piiroinen
 * - initial version
 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "xmalloc.h"

#ifndef F_all
# define F_all 1
#endif

/* ------------------------------------------------------------------------- *
 * xmalloc
 * ------------------------------------------------------------------------- */

#if F_all || F_xmalloc
void *xmalloc(size_t size)
{
  void *addr = malloc(size);
  assert( addr != NULL );
  return addr;
}
#endif

/* ------------------------------------------------------------------------- *
 * xfree
 * ------------------------------------------------------------------------- */

#if F_all || F_xfree
void xfree(void *addr)
{
  if( addr != 0 ) { free(addr); }
}
#endif

/* ------------------------------------------------------------------------- *
 * xcalloc
 * ------------------------------------------------------------------------- */

#if F_all || F_xcalloc
void *xcalloc(size_t nelem, size_t size)
{
  void *addr = calloc(nelem, size);
  assert( addr != NULL );
  return addr;
}
#endif

/* ------------------------------------------------------------------------- *
 * xrealloc
 * ------------------------------------------------------------------------- */

#if F_all || F_xrealloc
void *xrealloc(void *old_addr, size_t new_size)
{
  void *new_addr = NULL;

  if( new_size == 0 )
  {
    free(old_addr);
  }
  else if( old_addr == NULL )
  {
    new_addr = malloc(new_size);
    assert( new_addr != NULL );
  }
  else
  {
    new_addr = realloc(old_addr, new_size);
    assert( new_addr != NULL );
  }
  return new_addr;
}
#endif

/* ------------------------------------------------------------------------- *
 * xstrdup
 * ------------------------------------------------------------------------- */

#if F_all || F_xstrdup
#include <string.h>
char *xstrdup(const char *str)
{
  char *res = 0;

  if( str != 0 )
  {
    size_t cnt = strlen(str) + 1;
    res = memcpy(xmalloc(cnt), str, cnt);
  }
  return res;
}
#endif

/* ------------------------------------------------------------------------- *
 * xstrset
 * ------------------------------------------------------------------------- */

#if F_all || F_xstrset
void xstrset(char **dst, const char *str)
{
  free(*dst);
  *dst = (str == 0) ? 0 : xstrdup(str);
}
#endif


/* ------------------------------------------------------------------------- *
 * xstrdupn
 * ------------------------------------------------------------------------- */

#if F_all || F_xstrdupn
#include <string.h>
char *xstrdupn(const char *str, size_t len)
{
  char *res = 0;

  if( str != 0 )
  {
    size_t cnt = strnlen(str, len);
    res = xmalloc(cnt + 1);
    memcpy(res, str, cnt);
    res[cnt] = 0;
  }
  return res;
}
#endif
