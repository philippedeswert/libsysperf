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
 * File: proc_status.c
 * 
 * Author: Simo Piiroinen
 * 
 * History:
 * 
 * 21-Sep-2006 Simo Piiroinen
 * - fixed include paths
 *
 * 30-May-2006 Simo Piiroinen
 * - moved from track2 source tree
 * 
 * 15-Dec-2004 Simo Piiroinen
 * - added Name, Pid and PPid parsing
 * ========================================================================= */

#include "cstring.h"
#include "xmalloc.h"
#include "proc_status.h"

/* ------------------------------------------------------------------------- *
 * proc_status_create
 * ------------------------------------------------------------------------- */

proc_status_t *
proc_status_create(void)
{
  proc_status_t *self = xcalloc(1, sizeof *self);
  proc_status_ctor(self);
  return self;
}

/* ------------------------------------------------------------------------- *
 * proc_status_delete
 * ------------------------------------------------------------------------- */

void
proc_status_delete(proc_status_t *self)
{
  if( self != 0 )
  {
    proc_status_dtor(self);
    xfree(self);
  }
}

/* ------------------------------------------------------------------------- *
 * proc_status_update
 * ------------------------------------------------------------------------- */

void
proc_status_update(proc_status_t *self, char *data)
{
  char *key, *val;

  while( *data )
  {
    key = cstring_split_at_char(data, &data, '\n');
    
    if( strchr("VNP", *key) == 0 ) continue;
    
    //if( key[0] != 'V' || key[1] != 'm' ) continue;

    cstring_split_at_char(key, &val, ':');

#define Xu(v) if( !strcmp(key, #v) )\
  { self->v = strtoul(val,0,0); } else
    
#define Xs(v) if( !strcmp(key, #v) ) \
  { snprintf(self->v,sizeof self->v, "%s", cstring_strip(val)); } else
    
    Xs(Name);
    Xu(Pid);
    Xu(PPid);
    Xu(VmSize)
    Xu(VmLck)
    Xu(VmRSS)
    Xu(VmData)
    Xu(VmStk)
    Xu(VmExe)
    Xu(VmLib)
    { }
    
#undef Xu
#undef Xs
  }
  
  for( char *s = self->Name; *s; ++s )
  {
    switch( *s )
    {
    case '/': 
    case '[': case ']':
    case '(': case ')':
    case '{': case '}':
      *s = '_';
      break;
    }
  }
}

/* ------------------------------------------------------------------------- *
 * proc_status_parse
 * ------------------------------------------------------------------------- */

void
proc_status_parse(proc_status_t *self, const char *path)
{
  char *data = cstring_from_file(path);
  if( data != 0 )
  {
    proc_status_update(self, data);
    xfree(data);
  }
}

/* ------------------------------------------------------------------------- *
 * proc_status_repr
 * ------------------------------------------------------------------------- */

void
proc_status_repr(proc_status_t *self, FILE *file)
{
  fprintf(file,
          "VmSize=%u,VmLck=%u,VmRSS=%u,VmData=%u,"
          "VmStk=%u,VmExe=%u,VmLib=%u\n",
          self->VmSize,
          self->VmLck,
          self->VmRSS,
          self->VmData,
          self->VmStk,
          self->VmExe,
          self->VmLib);
}
