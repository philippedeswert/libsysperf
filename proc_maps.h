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
 * File: proc_maps.h
 * 
 * Author: Simo Piiroinen
 * 
 * History:
 * 
 * 21-Sep-2006 Simo Piiroinen
 * - fixed include paths
 * 
 * 29-May-2006 Simo Piiroinen
 * - modified to use libsysperf API
 * - moved from track2 source tree
 * ========================================================================= */

#ifndef PROC_MAPS_H_
#define PROC_MAPS_H_

#include <stdio.h>

#include "array.h"

typedef struct proc_maps_t
{
  // data
  unsigned begin;
  unsigned end;
  char     *prot;
  char     *path;
} proc_maps_t;

/* proc_maps_t.c */
void proc_maps_ctor(proc_maps_t *self, unsigned begin, unsigned end, char *prot, char *path);
void proc_maps_dtor(proc_maps_t *self);
proc_maps_t *proc_maps_create(unsigned begin, unsigned end, char *prot, char *path);
void proc_maps_delete(proc_maps_t *self);
int proc_maps_cmp_begin(const void *a, const void *b);
int proc_maps_cmp_begin_indirect(const void *a, const void *b);
void proc_maps_parse(array_t *arr, const char *path);
void proc_maps_repr(const proc_maps_t *obj, FILE *out);

#endif // PROC_MAPS_H_
