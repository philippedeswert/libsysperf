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
 * File: proc_stat.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmalloc.h"
#include "cstring.h"
#include "proc_stat.h"

/* ------------------------------------------------------------------------- *
 * proc_stat_update
 * ------------------------------------------------------------------------- */

void
proc_stat_update(proc_stat_t *self, char *data)
{
  char *pos = data;

  self->pid         = strtol(pos,&pos,10);
  cstring_copy(self->comm, sizeof self->comm,
               cstring_split_quoted(pos,&pos));
  self->state       = *cstring_split_at_white(pos,&pos);
  self->ppid        = strtol(pos,&pos,10);
  self->pgrp        = strtol(pos,&pos,10);
  self->session     = strtol(pos,&pos,10);
  self->tty_nr      = strtol(pos,&pos,10);
  self->tpgid       = strtol(pos,&pos,10);
  self->flags       = strtoul(pos,&pos,10);
  self->minflt      = strtoul(pos,&pos,10);
  self->cminflt     = strtoul(pos,&pos,10);
  self->majflt      = strtoul(pos,&pos,10);
  self->cmajflt     = strtoul(pos,&pos,10);
  self->utime       = strtoul(pos,&pos,10);
  self->stime       = strtoul(pos,&pos,10);
  self->cutime      = strtol(pos,&pos,10);
  self->cstime      = strtol(pos,&pos,10);
  self->priority    = strtol(pos,&pos,10);
  self->nice        = strtol(pos,&pos,10);
  self->unused0     = strtol(pos,&pos,10);
  self->itrealvalue = strtol(pos,&pos,10);
  self->starttime   = strtoul(pos,&pos,10);
  self->vsize       = strtoul(pos,&pos,10);
  self->rss         = strtol(pos,&pos,10);
  self->rlim        = strtoul(pos,&pos,10);
  self->startcode   = strtoul(pos,&pos,10);
  self->endcode     = strtoul(pos,&pos,10);
  self->startstack  = strtoul(pos,&pos,10);
  self->kstkesp     = strtoul(pos,&pos,10);
  self->kstkeip     = strtoul(pos,&pos,10);
  self->signal      = strtoul(pos,&pos,10);
  self->blocked     = strtoul(pos,&pos,10);
  self->sigignore   = strtoul(pos,&pos,10);
  self->sigcatch    = strtoul(pos,&pos,10);
  self->wchan       = strtoul(pos,&pos,10);
  self->nswap       = strtoul(pos,&pos,10);
  self->cnswap      = strtoul(pos,&pos,10);
  self->exit_signal = strtol(pos,&pos,10);
  self->processor   = strtol(pos,&pos,10);
}

/* ------------------------------------------------------------------------- *
 * proc_stat_parse
 * ------------------------------------------------------------------------- */

int proc_stat_parse(proc_stat_t *self, const char *path)
{
  int xc = -1; // assume error
  char *data = cstring_from_file(path);
  if( data != 0 )
  {
    proc_stat_update(self, data);
    xfree(data);
    xc = 0;
  }
  return xc;
}

/* ------------------------------------------------------------------------- *
 * proc_stat_repr
 * ------------------------------------------------------------------------- */

void
proc_stat_repr(proc_stat_t *self, FILE *file)
{
  fprintf(file,
          "pid         = %d\n"
          "comm        = %s\n"
          "state       = %c\n"
          "ppid        = %d\n"
          "pgrp        = %d\n"
          "session     = %d\n"
          "tty_nr      = %d\n"
          "tpgid       = %d\n"
          "flags       = %lu\n"
          "minflt      = %lu\n"
          "cminflt     = %lu\n"
          "majflt      = %lu\n"
          "cmajflt     = %lu\n"
          "utime       = %lu\n"
          "stime       = %lu\n"
          "cutime      = %ld\n"
          "cstime      = %ld\n"
          "priority    = %ld\n"
          "nice        = %ld\n"
          "unused0     = %ld\n"
          "itrealvalue = %ld\n"
          "starttime   = %lu\n"
          "vsize       = %lu\n"
          "rss         = %ld\n"
          "rlim        = %lu\n"
          "startcode   = %lu\n"
          "endcode     = %lu\n"
          "startstack  = %lu\n"
          "kstkesp     = %lu\n"
          "kstkeip     = %lu\n"
          "signal      = %lu\n"
          "blocked     = %lu\n"
          "sigignore   = %lu\n"
          "sigcatch    = %lu\n"
          "wchan       = %lu\n"
          "nswap       = %lu\n"
          "cnswap      = %lu\n"
          "exit_signal = %d\n"
          "processor   = %d\n",
          self->pid,
          self->comm,
          self->state,
          self->ppid,
          self->pgrp,
          self->session,
          self->tty_nr,
          self->tpgid,
          self->flags,
          self->minflt,
          self->cminflt,
          self->majflt,
          self->cmajflt,
          self->utime,
          self->stime,
          self->cutime,
          self->cstime,
          self->priority,
          self->nice,
          self->unused0,
          self->itrealvalue,
          self->starttime,
          self->vsize,
          self->rss,
          self->rlim,
          self->startcode,
          self->endcode,
          self->startstack,
          self->kstkesp,
          self->kstkeip,
          self->signal,
          self->blocked,
          self->sigignore,
          self->sigcatch,
          self->wchan,
          self->nswap,
          self->cnswap,
          self->exit_signal,
          self->processor);
}
