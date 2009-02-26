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
 * File: proc_status.h
 *
 * Author: Simo Piiroinen
 *
 * History:
 *
 * 30-May-2006 Simo Piiroinen
 * - moved from track2 source tree
 *
 * 15-Dec-2004 Simo Piiroinen
 * - added Name, Pid and PPid parsing
 * ========================================================================= */

#ifndef PROC_STATUS_H_
#define PROC_STATUS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct proc_status_t
{
  char     Name[32];
  unsigned Pid;
  unsigned PPid;

  unsigned VmSize;
  unsigned VmLck;
  unsigned VmRSS;
  unsigned VmData;
  unsigned VmStk;
  unsigned VmExe;
  unsigned VmLib;

} proc_status_t;

static inline void proc_status_ctor(proc_status_t *self)
{
  self->Name[0] = 0;
  self->Pid     = 0;
  self->PPid    = 0;

  self->VmSize = 0;
  self->VmLck  = 0;
  self->VmRSS  = 0;
  self->VmData = 0;
  self->VmStk  = 0;
  self->VmExe  = 0;
  self->VmLib  = 0;
}

static inline void proc_status_dtor(proc_status_t *self)
{
}

proc_status_t *proc_status_create(void);
void proc_status_delete(proc_status_t *self);
void proc_status_update(proc_status_t *self, char *data);
void proc_status_parse(proc_status_t *self, const char *path);
void proc_status_repr(proc_status_t *self, FILE *file);

#ifdef __cplusplus
};
#endif

#endif // PROC_STATUS_H_
