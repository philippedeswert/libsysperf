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
 * File: proc_statm.h
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 31-May-2006 Simo Piiroinen
 * - copied from track2 source tree
 * ========================================================================= */

#ifndef PROC_STATM_H_
#define PROC_STATM_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct proc_statm_t
{
  unsigned size;     // total program size
  unsigned resident; // size of memory portions
  unsigned shared;   // number of pages that are shared
  unsigned trs;      // number of pages that are 'code'
  unsigned drs;      // number of pages of data/stack
  unsigned lrs;      // number of pages of library
  unsigned dt;       // number of dirty pages

} proc_statm_t;

static inline void proc_statm_dtor(proc_statm_t *self) { }
static inline void proc_statm_ctor(proc_statm_t *self) { }

void proc_statm_update(proc_statm_t *self, char *data);
void proc_statm_parse(proc_statm_t *self, const char *path);
void proc_statm_repr(proc_statm_t *self, FILE *file);

#ifdef __cplusplus
};
#endif

#endif // PROC_STATM_H_
