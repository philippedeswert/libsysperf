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
 * File: hash.c
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

#include <string.h>
#include <alloca.h>

#include <assert.h>

#include "hash.h"

typedef unsigned int  u32;
typedef unsigned char u8;
#include <linux/jhash.h>

unsigned hash_str(const char *str)
{
  int len = strlen(str);  
  len = (len + 3) & ~3;
  void *data = alloca(len);
  strncpy(data, str, len);
  
  assert( !(3 & (int)data) );
  
  return jhash2(data, len>>2, 0);
}

#if 01
# include <stdio.h>
int main(int ac, char **av)
{
  char buf[1024];
  
  while( fgets(buf, sizeof buf, stdin) )
  {
    buf[strcspn(buf,"\r\n")]=0;
    unsigned h = hash_str(buf);
    printf("%08x %s\n", h, buf);
  }
  return 0;
}
#endif
