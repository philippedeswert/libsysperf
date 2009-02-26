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
 * File: mem_pool.h
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

#ifndef MEM_POOL_H_
#define MEM_POOL_H_

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif

typedef struct mem_chunk_t mem_chunk_t;
typedef struct mem_pool_t mem_pool_t;

/* ========================================================================= *
 * mem_chunk_t
 * ========================================================================= */

struct mem_chunk_t
{
  mem_chunk_t *next;
  size_t       head;
  size_t       tail;
  char         data[];
};

mem_chunk_t *mem_chunk_create(size_t size);
void mem_chunk_delete(mem_chunk_t *self);
size_t mem_chunk_avail(mem_chunk_t *self);
void *mem_chunk_alloc(mem_chunk_t *self, size_t size);

/* ========================================================================= *
 * mem_pool_t
 * ========================================================================= */

struct mem_pool_t
{
  mem_chunk_t *chunk;
  size_t       alloc;
};

void mem_pool_ctor(mem_pool_t *self);
void mem_pool_dtor(mem_pool_t *self);

mem_pool_t *mem_pool_create(void);
void mem_pool_delete(mem_pool_t *self);

void *mem_pool_alloc(mem_pool_t *self, size_t size);
void *mem_pool_strdup(mem_pool_t *self, const char *str);

#ifdef __cplusplus
};
#endif

#endif /* MEM_POOL_H_ */

/* mem_pool.c */
