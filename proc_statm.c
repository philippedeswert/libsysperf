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
 * File: proc_statm.c
 * 
 * Author: Simo Piiroinen
 * 
 * -------------------------------------------------------------------------
 * 
 * History:
 * 
 * 21-Sep-2006 Simo Piiroinen
 * - fixed include paths
 * 
 * 31-May-2006 Simo Piiroinen
 * - modified to use libsysperf API
 * - copied from track2 source tree
 * ========================================================================= */

#include "xmalloc.h"
#include "cstring.h"
#include "proc_statm.h"


/* ------------------------------------------------------------------------- *
 * proc_statm_update
 * ------------------------------------------------------------------------- */

void
proc_statm_update(proc_statm_t *self, char *data)
{
  self->size     = strtoul(data,&data,10);
  self->resident = strtoul(data,&data,10);
  self->shared   = strtoul(data,&data,10);
  self->trs      = strtoul(data,&data,10);
  self->drs      = strtoul(data,&data,10);
  self->lrs      = strtoul(data,&data,10);
  self->dt       = strtoul(data,&data,10);
}

/* ------------------------------------------------------------------------- *
 * proc_statm_parse
 * ------------------------------------------------------------------------- */

void
proc_statm_parse(proc_statm_t *self, const char *path)
{
  char *data = cstring_from_file(path);
  if( data != 0 )
  {
    proc_statm_update(self, data);
    xfree(data);
  }
}

/* ------------------------------------------------------------------------- *
 * proc_statm_repr
 * ------------------------------------------------------------------------- */

void
proc_statm_repr(proc_statm_t *self, FILE *file)
{
  fprintf(file,
          "size=%u,resident=%u,shared=%u,trs=%u,drs=%u,lrs=%u,dt=%u\n",
          self->size,
          self->resident,
          self->shared,
          self->trs,
          self->drs,
          self->lrs,
          self->dt);
}
