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
 * File: str_array.h
 * 
 * Author: Simo Piiroinen
 * 
 * -------------------------------------------------------------------------
 * 
 * History:
 * 
 * 28-Jun-2005 Simo Piiroinen
 * - imported from track2
 *
 * 10-Jan-2005 Simo Piiroinen
 * - added str_array_exclude
 * ========================================================================= */

#ifndef STR_ARRAY_H_
#define STR_ARRAY_H_

#include "array.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef array_t str_array_t;

static inline void str_array_ctor(str_array_t *self)
{
  array_ctor(self, free);
}

static inline void str_array_dtor(str_array_t *self)
{
  array_dtor(self);
}

static inline char *str_array_get(const str_array_t *self, size_t index)
{
  return array_get(self, index);
}

str_array_t *str_array_create(void);
void str_array_delete(str_array_t *self);

void str_array_add(str_array_t *self, const char *str);
void str_array_set(str_array_t *self, size_t index, const char *str);
int str_array_index(const str_array_t *self, const char *str);
size_t str_array_exclude(str_array_t *self, const str_array_t *excl);

  

#ifdef __cplusplus
};
#endif

#endif // STR_ARRAY_H_
