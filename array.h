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
 * File: array.h
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 18-Jan-2007 Simo Piiroinen
 * - NULL item destructor redirected to dummy
 *
 * 22-Sep-2005 Simo Piiroinen
 * - added array_size()
 *
 * 28-Jun-2005 Simo Piiroinen
 * - imported from track2
 *
 * 20-Aug-2004 Simo Piiroinen
 * - added array_empty() method
 * ========================================================================= */

#ifndef ARRAY_H_
#define ARRAY_H_

#include <stddef.h>
#include "xmalloc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*array_delete_fn)(void *);

typedef struct array_t
{
  array_delete_fn del;
  size_t alloc;
  size_t size;
  void **data;
} array_t;

static inline int array_size(const array_t *self)
  {
    return self->size;
  }

static inline int array_empty(const array_t *self)
  {
    return self->size == 0;
  }

static inline void *array_first(const array_t *self)
  {
    return (self->size > 0) ? self->data[0] : 0;
  }
static inline void *array_last(const array_t *self)
  {
    return (self->size > 0) ? self->data[self->size - 1] : 0;
  }

#define ARR_INITIALIZER(del) { (array_delete_fn)(del),0,0,0 }

typedef int (*array_cmp_fn)(const void *a, const void *b);

void array_swap(array_t *arr1, array_t *arr2);
void array_minsize(array_t *arr, size_t req);
void *array_rem(array_t *arr, size_t index);
void array_del(array_t *arr, size_t index);
void array_clear(array_t *arr);
void array_flush(array_t *arr);
size_t array_add(array_t *arr, void *elem);
void array_sort(array_t *arr, array_cmp_fn fn);
void array_sort_range(array_t *arr, array_cmp_fn fn, size_t lo, size_t hi);
void array_reverse(array_t *arr);
void *array_pop(array_t *arr);

void array_final(array_t *arr);

static inline void array_dummy_delete(void *ptr) { }

static inline void array_ctor_nc(array_t *self, array_delete_fn del)
{
  self->del   = del ? del : array_dummy_delete;
  self->alloc = 0;
  self->size  = 0;
  self->data  = 0;
}
#define array_ctor(self,del) array_ctor_nc(self, (array_delete_fn)(del))

static inline void array_dtor(array_t *self)
{
  array_flush(self);
}

static inline array_t *array_create_(array_delete_fn del)
{
  array_t *self = xcalloc(1, sizeof *self);
  array_ctor(self, del);
  return self;
}
#define array_create(del) array_create_((array_delete_fn)(del))

static inline void array_delete(array_t *self)
{
  if( self != 0 )
  {
    array_dtor(self);
    xfree(self);
  }
}

static inline void array_set(array_t *arr, size_t i, void *data)
{
  if( i < arr->size )
  {
    arr->del(arr->data[i]);
    arr->data[i] = data;
  }
  else
  {
    arr->del(data);
  }
}

static inline void array_del_nocompact(array_t *arr, size_t i)
{
  if( i < arr->size )
  {
    arr->del(arr->data[i]);
    arr->data[i] = 0;
  }
}

static inline void *array_rem_nocompact(array_t *arr, size_t i)
{
  void *item = 0;
  if( i < arr->size )
  {
    item = arr->data[i];
    arr->data[i] = 0;
  }
  return item;
}

static inline size_t array_compact(array_t *arr)
{
  size_t si,di;

  for( si = di = 0; si < arr->size; ++si )
  {
    if( arr->data[si] != 0 )
    {
      arr->data[di++] = arr->data[si];
    }
  }
  arr->size = di;
  return si-di;
}

static inline void *array_get(const array_t *self, size_t i)
{
  return (i < self->size) ? self->data[i] : 0;
}

static inline void *array_get_def(array_t *self, size_t i, const void *def)
{
  return (i < self->size) ? self->data[i] : (void *)def;
}

#ifdef __cplusplus
};
#endif

#endif // ARRAY_H_
