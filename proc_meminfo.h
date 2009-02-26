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
 * File: proc_meminfo.h
 *
 * Author: Simo Piiroinen
 *
 * History:
 *
 * 30-May-2006 Simo Piiroinen
 * - moved from track2 source tree
 * ========================================================================= */

#ifndef PROC_MEMINFO_H_
#define PROC_MEMINFO_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef struct proc_meminfo_t
{
#define VAR(name) unsigned name;
#include "proc_meminfo.inc"
} proc_meminfo_t;

static inline void proc_meminfo_ctor(proc_meminfo_t *self)
{
#define VAR(name) self->name = 0;
#include "proc_meminfo.inc"
}

static inline void proc_meminfo_dtor(proc_meminfo_t *self)
{
}

proc_meminfo_t *proc_meminfo_create(void);
void proc_meminfo_delete(proc_meminfo_t *self);
void proc_meminfo_update(proc_meminfo_t *self, char *data);
void proc_meminfo_parse(proc_meminfo_t *self, const char *path);
void proc_meminfo_repr(proc_meminfo_t *self, FILE *file);

#ifdef __cplusplus
};
#endif

#endif // PROC_MEMINFO_H_
