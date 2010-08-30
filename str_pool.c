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
 * File: str_pool.c
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 * 
 * 29-Jun-2005 Simo Piiroinen
 * - cleanup & comments
 *
 * 21-Jun-2005 Simo Piiroinen
 * - initial version
 * ========================================================================= */

/* ========================================================================= *
 * Includes
 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "msg.h"
#include "mem_pool.h"
#include "str_pool.h"

/* ========================================================================= *
 * str_pool_t methods
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * str_pool_create
 * ------------------------------------------------------------------------- */

str_pool_t *str_pool_create(void)
{
  str_pool_t *self = calloc(1, sizeof *self);
  str_pool_ctor(self);
  return self;
}

/* ------------------------------------------------------------------------- *
 * str_pool_delete
 * ------------------------------------------------------------------------- */

void str_pool_delete(str_pool_t *self)
{
  if( self != 0 )
  {
    str_pool_dtor(self);
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * str_pool_add  --  return pointer to interned version of string
 * ------------------------------------------------------------------------- */

typedef unsigned int  u32;
typedef unsigned char u8;
#include <linux/jhash.h>

const char *str_pool_add(str_pool_t *self, const char *text)
{
  /* - - - - - - - - - - - - - - - - - - - *
   * Make a word aligned & zero padded to
   * multiples of word version of the
   * string. This way we can use faster
   * version of the hash function.
   * - - - - - - - - - - - - - - - - - - - */
  
  size_t    len  = strlen(text);
  size_t    size = (len + 3) & ~3;
  void     *temp = strncpy(alloca(size), text, size);
  unsigned  hash = jhash2(temp, size>>2, 0);
  
  /* - - - - - - - - - - - - - - - - - - - *
   * locate/create a pooled string matching
   * the parameter
   * - - - - - - - - - - - - - - - - - - - */
  
  unsigned      h = hash & (self->hmax-1);
  pooled_str_t *p = self->htab[h];
  
  for( ;; p = p->next )
  {
    if( p == 0 )
    {
      /* - - - - - - - - - - - - - - - - - - - *
       * no match -> create a new entry
       * - - - - - - - - - - - - - - - - - - - */
      
      p = mem_pool_alloc(&self->pool, sizeof *p + len);
      p->refs = 0;
      p->next = self->htab[h];
      p->hash = hash;
      memcpy(p->text, text, len+1);
      
      self->htab[h] = p;
      break;
    }
    
    if( p->hash == hash && !strcmp(text, p->text) )
    {
      /* - - - - - - - - - - - - - - - - - - - *
       * found match, use it
       * - - - - - - - - - - - - - - - - - - - */
      
      break;
    }
  }
  
  p->refs += 1;
  return p->text;
}

/* ------------------------------------------------------------------------- *
 * str_pool_emit  --  debugging: dump string pool stats
 * ------------------------------------------------------------------------- */

void str_pool_emit(str_pool_t *self, FILE *file)
{
  pooled_str_t *s;
  
  int used = 0;
  int dmax = 0;
  
  for( unsigned h = 0; h < self->hmax; ++h )
  {
    int d = 0;
    
    for( s = self->htab[h]; s; s = s->next )
    {
      d += 1;
      fprintf(file, "%6d %8p %08x %s\n", s->refs, s, s->hash, s->text);
    }
    used += (d != 0);
    if( dmax < d ) dmax = d;
  }
  
  fprintf(file, "used: %d / %d = %g%%\n", used, self->hmax, 100.0*used/self->hmax);
  fprintf(file, "dmax: %d\n", dmax);
}

/* ========================================================================= *
 * entry point for testing
 * ========================================================================= */

#if 0
int main(int ac, char **av)
{
  str_pool_t *pool = str_pool_create();
  char buf[1024];

  //msg_setverbosity(MSG_WARNING);
  
  while( fgets(buf, sizeof buf, stdin) )
  {
    buf[strcspn(buf,"\r\n")]=0;
    
    str_pool_add(pool, buf);
    
    //unsigned h = hash_str(buf);
    //printf("%08x %s\n", h, buf);
  }
  str_pool_emit(pool);
  str_pool_delete(pool);
  return 0;
}
#endif
