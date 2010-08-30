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
 * File: xmalloc.h
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

#ifndef XMALLOC_H_
#define XMALLOC_H_

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void *xmalloc(size_t size);
void  xfree(void *addr);
void *xcalloc(size_t nelem, size_t size);
void *xrealloc(void *old_addr, size_t new_size);

void  xstrset(char **dst, const char *str);
char *xstrdup(const char *str);
char *xstrdupn(const char *str, size_t len);

static inline void *xmemdup(void *addr, size_t size)
{
  return memcpy(xmalloc(size), addr, size);
}

#ifdef __cplusplus
};
#endif

#endif // XMALLOC_H_
