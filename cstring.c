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
 * File: cstring.c
 *
 * Author: Simo Piiroinen
 * ========================================================================= */

#include <string.h>
#include <assert.h>

#include "cstring.h"

#ifndef F_all
# define F_all 1
#endif

/* ------------------------------------------------------------------------- *
 * cstring_basename
 * ------------------------------------------------------------------------- */

#if F_all || F_cstring_basename
char *cstring_basename(const char *path)
{
  char *s = strrchr(path, '/');
  return (s != 0) ? (s+1) : (char *)path;
}
#endif

/* ------------------------------------------------------------------------- *
 * cstring_extension
 * ------------------------------------------------------------------------- */

#if F_all || F_cstring_extension
char *cstring_extension(const char *path)
{
  char *b = cstring_basename(path);
  char *s = strrchr(b, '.');
  return (s != 0) ? s : strchr(b,0);
}
#endif

/* ------------------------------------------------------------------------- *
 * cstring_lstrip
 * ------------------------------------------------------------------------- */

#if F_all || F_cstring_lstrip
char *cstring_lstrip(char *str)
{
  char *src = cstring_skip_white(str);
  char *dst = str;

  if( dst != src )
  {
    while( (*dst++ = *src++) != 0 ) {}
  }
  return str;
}
#endif

/* ------------------------------------------------------------------------- *
 * cstring_rstrip
 * ------------------------------------------------------------------------- */

#if F_all || F_cstring_rstrip
char *cstring_rstrip(char *str)
{
  char *dst = str;
  char *end = str;
  char *src = cstring_skip_white(str);

  while( *src != 0 )
  {
    if( cstring_at_black(src) ) { end = src + 1; }
    *dst++ = *src++;
  }
  *end = 0;

  return str;
}
#endif

/* ------------------------------------------------------------------------- *
 * cstring_strip
 * ------------------------------------------------------------------------- */

#if F_all || F_cstring_strip
char *cstring_strip(char *str)
{
  char *dst = str;
  char *src = cstring_skip_white(str);

  for( ;; )
  {
    while( cstring_at_black(src) )
    {
      *dst++ = *src++;
    }

    while( cstring_at_white(src) )
    {
      ++src;
    }
    if( *src == 0 ) break;
    *dst++ = ' ';
  }
  *dst = 0;
  return str;
}
#endif

/* ------------------------------------------------------------------------- *
 * cstring_split_at_white
 * ------------------------------------------------------------------------- */

#if F_all || F_cstring_split_at_white
char *cstring_split_at_white(char *str, char **next)
{
  char *res = cstring_skip_white(str);
  char *end = cstring_skip_black(res);

  if( *end != 0 )
  {
    *end++ = 0;
  }
  if( next != 0 )
  {
    *next = end;
  }
  return res;
}
#endif

/* ------------------------------------------------------------------------- *
 * cstring_split_at_char
 * ------------------------------------------------------------------------- */

#if F_all || F_cstring_split_at_char
char *cstring_split_at_char(char *str, char **next, int ch)
{
  char *end;

  if( (end = strchr(str, ch)) != 0 )
  {
    *end++ = 0;
  }
  else
  {
    end = strchr(str, 0);
  }

  if( next != 0 )
  {
    *next = end;
  }
  return str;
}
#endif

/* ------------------------------------------------------------------------- *
 * cstring_split_quoted
 * ------------------------------------------------------------------------- */

#if F_all || F_cstring_split_quoted
char *cstring_split_quoted(char *str, char **next)
{
  char *res = cstring_skip_white(str);
  int   sep = 0;

  switch( *res )
  {
  case '(':  sep = ')';  break;
  case '[':  sep = ']';  break;
  case '{':  sep = '}';  break;
  case '<':  sep = '>';  break;
  case '"':  sep = '"';  break;
  case '\'': sep = '\''; break;
  }

  if( sep != 0 )
  {
    cstring_split_at_char(++res, next, sep);
  }
  else
  {
    cstring_split_at_white(res, next);
  }
  return res;
}
#endif

/* ------------------------------------------------------------------------- *
 * cstring_copy
 * ------------------------------------------------------------------------- */

#if F_all || F_cstring_copy
char *cstring_copy(char *dest, size_t size, const char *srce)
{
  if( size > 0 )
  {
    size_t i, n = size - 1;

    for( i = 0; (i < n) && (srce[i] != 0); ++i )
    {
      dest[i] = srce[i];
    }
    dest[i] = 0;
  }
  return dest;
}
#endif

/* ------------------------------------------------------------------------- *
 * cstring_from_file
 * ------------------------------------------------------------------------- */

#if F_all || F_cstring_from_file

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "cstring.h"
#include "xmalloc.h"

char *cstring_from_file(const char *path)
{
  char *text = NULL;
  int   size = 255;
  int   used = 0;
  int   file = -1;

  int rc;

  if( path == 0 )
  {
    fprintf(stderr, "trying to read from NULL path\n");
    goto cleanup;
  }

  if( (file = open(path,O_RDONLY)) == -1 )
  {
    perror(path); goto cleanup;
  }

  text = xmalloc(size + 1);

  for( ;; )
  {
    if( used == size )
    {
      size *= 2;
      text = xrealloc(text, size);
    }

    rc = read(file, text+used, size-used);

    if( rc == 0 )
    {
      break;
    }

    if( rc == -1 )
    {
      perror(path); break;
    }

    used += rc;
  }
  text[used] = 0;

  cleanup:

  if( file != -1 )
  {
    if( close(file) != 0 ) perror(path);
  }

  return text;
}
#endif

/* ------------------------------------------------------------------------- *
 * cstring_from_stream
 * ------------------------------------------------------------------------- */

#if F_all || F_cstring_from_stream

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "cstring.h"
#include "xmalloc.h"

char *cstring_from_stream(FILE *file)
{
  char *text = NULL;
  int   size = 255;
  int   used = 0;

  int rc;

  assert( file != 0 );

  text = xmalloc(size + 1);

  for( ;; )
  {
    if( used == size )
    {
      size *= 2;
      text = xrealloc(text, size);
    }
    if( (rc = fread(text+used, 1, size-used, file)) <= 0 )
    {
      if( ferror(file) )
      {
        perror("fread");
      }
      break;
    }
    used += rc;
  }

  text[used] = 0;

  return text;
}
#endif
