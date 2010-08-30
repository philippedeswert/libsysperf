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
 * File: str_pool.h
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 05-Oct-2005 Simo Piiroinen
 * - include file fixes
 * 
 * 29-Jun-2005 Simo Piiroinen
 * - cleanup & comments
 *
 * 21-Jun-2005 Simo Piiroinen
 * - added includes mem_pool.h & stdlib.h
 * - initial version
 * ========================================================================= */

#ifndef STR_POOL_H_
#define STR_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include "mem_pool.h"

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif

/* ========================================================================= *
 * Typedefs
 * ========================================================================= */

typedef struct str_pool_t   str_pool_t;
typedef struct pooled_str_t pooled_str_t;

/* ========================================================================= *
 * struct pooled_str_t  --  non-mutable interned string
 * ========================================================================= */

struct pooled_str_t
{
  pooled_str_t *next;	// chain
  unsigned      hash;	// hash value of text
  unsigned      refs;	// use count
  char          text[1];// string data
};

/* ========================================================================= *
 * struct str_pool_t  --  pool of interned strings
 * ========================================================================= */

struct str_pool_t
{
  unsigned       hcnt;	// number of interned strings
  
  unsigned       hmax;	// size of the hash table
  pooled_str_t **htab;	// hash table slots for interned strings
  
  mem_pool_t     pool;	// memory manager for interned strings
};

/* ------------------------------------------------------------------------- *
 * extern str_pool_t methods 
 * ------------------------------------------------------------------------- */

str_pool_t *str_pool_create(void);
void str_pool_delete(str_pool_t *self);
const char *str_pool_add(str_pool_t *self, const char *text);
void str_pool_emit(str_pool_t *self, FILE *file);

/* ------------------------------------------------------------------------- *
 * str_pool_ctor
 * ------------------------------------------------------------------------- */

static inline void str_pool_ctor(str_pool_t *self)
{
  self->hcnt = 0;
  self->hmax = 32<<10;
  self->htab = calloc(self->hmax, sizeof *self->htab);
  
  mem_pool_ctor(&self->pool);
}

/* ------------------------------------------------------------------------- *
 * str_pool_dtor
 * ------------------------------------------------------------------------- */

static inline void str_pool_dtor(str_pool_t *self)
{
  free(self->htab);  
  mem_pool_dtor(&self->pool);
}



#ifdef __cplusplus
};
#endif

#endif /* STR_POOL_H_ */
