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
 * File: cstring.h
 *
 * Author: Simo Piiroinen
 * ========================================================================= */

#ifndef CSTRING_H_
#define CSTRING_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------------------------------------------------------- *
 * cstring utilities
 * ------------------------------------------------------------------------- */

static inline int cstring_at_white(const char *str)
{
  int c = (unsigned char)*str;
  return (c <= 0x20) && (c > 0x00);
}

static inline int cstring_at_black(const char *str)
{
  int c = (unsigned char)*str;
  return (c > 0x20);
}

static inline char *cstring_skip_white(const char *str)
{
  while( cstring_at_white(str) ) { ++str; }
  return (char *)str;
}

static inline char *cstring_skip_black(char *str)
{
  while( cstring_at_black(str) ) { ++str; }
  return (char *)str;
}

char *cstring_extension(const char *path);
char *cstring_basename(const char *path);

char *cstring_lstrip(char *str);
char *cstring_rstrip(char *str);
char *cstring_strip(char *str);

char *cstring_split_at_white(char *str, char **next);
char *cstring_split_at_char(char *str, char **next, int ch);

char *cstring_split_quoted(char *str, char **next);

char *cstring_from_file(const char *path);
char *cstring_from_stream(FILE *file);

void cstring_set(char **dest, char *srce);
char *cstring_copy(char *dest, size_t size, const char *srce);

static inline int cstring_compare(const char *s1, const char *s2)
{
  if( s1 != 0 && s2 != 0 )
  {
    return strcmp(s1, s2);
  }
  return (s2 == 0) - (s1 == 0);
}

static inline int cstring_to_int(const char *s, int base)
{
  return (s != 0) ? (int)strtol(s,0,base) : 0;
}
static inline long cstring_to_long(const char *s, int base)
{
  return (s != 0) ? strtol(s,0,base) : 0;
}
static inline unsigned cstring_to_uint(const char *s, int base)
{
  return (s != 0) ? strtoul(s,0,base) : 0;
}

static inline double cstring_to_double(const char *s)
{
  return (s != 0) ? strtod(s,0) : 0.0;
}

static inline float cstring_to_float(const char *s)
{
  return (s != 0) ? strtof(s,0) : 0.0;
}

static inline int cstring_compare_as_double(const char *s1, const char *s2)
{
  if( s1 != 0 && s2 != 0 )
  {
    double v1 = strtod(s1, 0);
    double v2 = strtod(s2, 0);
    return (v1 > v2) - (v1 < v2);
 }
  return (s2 == 0) - (s1 == 0);
}

#ifdef __cplusplus
};
#endif

#endif // CSTRING_H_
