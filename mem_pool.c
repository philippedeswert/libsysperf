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
 * File: mem_pool.c
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "msg.h"
#include "mem_pool.h"

/* ========================================================================= *
 * mem_chunk_t
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * mem_chunk_create
 * ------------------------------------------------------------------------- */

mem_chunk_t *mem_chunk_create(size_t size)
{
  msg_debug("%s: %d kB\n", __FUNCTION__, size >> 10);

  mem_chunk_t *self = malloc(sizeof *self + size);
  self->head = 0;
  self->tail = size;
  return self;
}

/* ------------------------------------------------------------------------- *
 * mem_chunk_delete
 * ------------------------------------------------------------------------- */

void mem_chunk_delete(mem_chunk_t *self)
{
  msg_debug("%s: %d kB\n", __FUNCTION__, self->head >> 10);
  free(self);
}

/* ------------------------------------------------------------------------- *
 * mem_chunk_avail
 * ------------------------------------------------------------------------- */

size_t mem_chunk_avail(mem_chunk_t *self)
{
  return (self != 0) ? (self->tail - self->head) : 0;
}

/* ------------------------------------------------------------------------- *
 * mem_chunk_alloc
 * ------------------------------------------------------------------------- */

void *mem_chunk_alloc(mem_chunk_t *self, size_t size)
{
  void *addr = &self->data[self->head];
  self->head += size;

  assert( self->head <= self->tail );

  return addr;
}

/* ------------------------------------------------------------------------- *
 * mem_pool_ctor
 * ------------------------------------------------------------------------- */

void mem_pool_ctor(mem_pool_t *self)
{
  //self->alloc = 256 << 10;
  self->alloc = 512 << 10;
  self->chunk = 0;
}

/* ------------------------------------------------------------------------- *
 * mem_pool_dtor
 * ------------------------------------------------------------------------- */

void mem_pool_dtor(mem_pool_t *self)
{
  mem_chunk_t *chunk;

  while( (chunk = self->chunk) != 0 )
  {
    self->chunk = chunk->next;
    mem_chunk_delete(chunk);
  }
}

/* ------------------------------------------------------------------------- *
 * mem_pool_create
 * ------------------------------------------------------------------------- */

mem_pool_t *mem_pool_create(void)
{
  mem_pool_t *self = calloc(1, sizeof *self);
  mem_pool_ctor(self);
  return self;
}

/* ------------------------------------------------------------------------- *
 * mem_pool_delete
 * ------------------------------------------------------------------------- */

void mem_pool_delete(mem_pool_t *self)
{
  if( self != 0 )
  {
    mem_pool_dtor(self);
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * mem_pool_alloc
 * ------------------------------------------------------------------------- */

void *mem_pool_alloc(mem_pool_t *self, size_t size)
{
  size = (size + 3) & ~3;

  if( mem_chunk_avail(self->chunk) < size )
  {
    mem_chunk_t *chunk;

    chunk = mem_chunk_create(self->alloc);
    chunk->next = self->chunk;
    self->chunk = chunk;
  }

  return mem_chunk_alloc(self->chunk, size);
}

/* ------------------------------------------------------------------------- *
 * mem_pool_strdup
 * ------------------------------------------------------------------------- */

void *mem_pool_strdup(mem_pool_t *self, const char *str)
{
  size_t len = strlen(str) + 1;
  return memcpy(mem_pool_alloc(self, len), str, len);
}
