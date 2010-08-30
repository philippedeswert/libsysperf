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
 * File: array.c
 * 
 * Author: Simo Piiroinen
 * 
 * -------------------------------------------------------------------------
 * 
 * History:
 * 
 * 28-Jun-2005 Simo Piiroinen
 * - imported from track2
 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <assert.h>

#include "array.h"

#ifndef F_all
# define F_all 1
#endif

/* ------------------------------------------------------------------------- *
 * array_swap  --  swap contents of two arrays
 * ------------------------------------------------------------------------- */

#if F_all || F_array_swap
void array_swap(array_t *arr1, array_t *arr2)
{
  array_t tmp;
  tmp = *arr1, *arr1 = *arr2, *arr2 = tmp;
}
#endif

/* ------------------------------------------------------------------------- *
 * array_minsize  --  allocate space for minimum of N items
 * ------------------------------------------------------------------------- */

#if F_all || F_array_minsize
void array_minsize(array_t *arr, size_t req)
{
  assert( arr != NULL );
  if( arr->alloc < req )
  {
    size_t n = 16;
    while( n < req )
    {
      n = n * 4 / 3;
    }
    arr->alloc = n;
    arr->data  = xrealloc(arr->data, arr->alloc * sizeof *arr->data);
  }
}
#endif

/* ------------------------------------------------------------------------- *
 * array_rem  --  remove (not delete) Nth item from array
 * ------------------------------------------------------------------------- */

#if F_all || F_array_rem
void *array_rem(array_t *arr, size_t index)
{
  void *elem = NULL;
  size_t i;
  if( index < arr->size )
  {
    elem = arr->data[index];
    arr->size--;
    for( i = index; i < arr->size; ++i )
    {
      arr->data[i] = arr->data[i+1];
    }
  }
  return elem;
}
#endif

/* ------------------------------------------------------------------------- *
 * array_del  --  remove and delete Nth item from array
 * ------------------------------------------------------------------------- */

#if F_all || F_array_del
void array_del(array_t *arr, size_t index)
{
  arr->del(array_rem(arr, index));
}
#endif

/* ------------------------------------------------------------------------- *
 * array_clear  --  remove and delete all items from array
 * ------------------------------------------------------------------------- */

#if F_all || F_array_clear
void array_clear(array_t *arr)
{
  size_t i;
  for( i = 0; i < arr->size; ++i )
  {
    arr->del(arr->data[i]);
  }
  arr->size = 0;
}
#endif

/* ------------------------------------------------------------------------- *
 * array_flush  --  as array_clear but also release item table
 * ------------------------------------------------------------------------- */


#if F_all || F_array_flush
void array_flush(array_t *arr)
{
  array_clear(arr);
  xfree(arr->data);
  arr->data  = 0;
  arr->alloc = 0;
}
#endif

/* ------------------------------------------------------------------------- *
 * array_add  --  add an item to end of array
 * ------------------------------------------------------------------------- */

#if F_all || F_array_add
size_t array_add(array_t *arr, void *elem)
{
  array_minsize(arr, arr->size + 1);
  arr->data[arr->size] = elem;
  return arr->size++;
}
#endif

/* ------------------------------------------------------------------------- *
 * array_sort  --  sort array
 * ------------------------------------------------------------------------- */

#if F_all || F_array_sort
void array_sort(array_t *arr, array_cmp_fn fn)
{
  if( arr->size > 1 )
  {
    qsort(arr->data, arr->size, sizeof *arr->data, fn);
  }
}
#endif

/* ------------------------------------------------------------------------- *
 * array_sort_range  --  sort array range
 * ------------------------------------------------------------------------- */

#if F_all || F_array_sort_range
void array_sort_range(array_t *arr, array_cmp_fn fn, size_t lo, size_t hi)
{
  if( lo > arr->size) lo = arr->size;
  if( hi > arr->size) hi = arr->size;

  if( lo < hi )
  {
    qsort(&arr->data[lo], hi-lo, sizeof *arr->data, fn);
  }
}
#endif

/* ------------------------------------------------------------------------- *
 * array_reverse  --  reverse the order of items in array
 * ------------------------------------------------------------------------- */

#if F_all || F_array_reverse
void array_reverse(array_t *arr)
{

  if( arr->size > 0 )
  {
    size_t i = 0;
    size_t k = arr->size - 1;

    while( i < k )
    {
      void *temp = arr->data[i];
      arr->data[i] = arr->data[k];
      arr->data[k] = temp;
      ++i, --k;
    }
  }
}
#endif
/* ------------------------------------------------------------------------- *
 * array_pop  --  remove (not delete) last item from array
 * ------------------------------------------------------------------------- */

#if F_all || F_array_pop
void *array_pop(array_t *arr)
{
  void *elem = NULL;
  if( arr->size > 0 )
  {
    elem = arr->data[--arr->size];
  }
  return elem;
}
#endif

/* ------------------------------------------------------------------------- *
 * array_final  --  resize to minimum needed size
 * ------------------------------------------------------------------------- */

#if F_all || F_array_final
void array_final(array_t *arr)
{
  assert( arr != NULL );

  if( arr->size < arr->alloc )
  {
    arr->alloc = arr->size;
    arr->data  = xrealloc(arr->data, arr->alloc * sizeof *arr->data);
  }
}
#endif

