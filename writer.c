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
 * File: writer.c
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 22-Jun-2005 Simo Piiroinen
 * - structures & prototypes moved to writer.h
 * - initial version
 * ========================================================================= */

#include <unistd.h>
#include <string.h>
#include <fcntl.h>

// QUARANTINE #define MSG_DISABLE_DEBUG    0

#include "msg.h"
#include "writer.h"

/* ------------------------------------------------------------------------- *
 * writer_flush
 * ------------------------------------------------------------------------- */

void writer_flush(writer_t *self)
{
// QUARANTINE   msg_debug("%s: %s\n", __FUNCTION__, self->wr_path);

  if( self->wr_curr != self->wr_head )
  {
    size_t size = self->wr_curr - self->wr_head;

    msg_debug("%s: %s -> %d bytes\n", __FUNCTION__, self->wr_path, size);

    if( write(self->wr_file, self->wr_head, size) != size )
    {
      msg_fatal("write error %s\n", self->wr_path);
    }
    self->wr_curr = self->wr_head;
  }
}

/* ------------------------------------------------------------------------- *
 * writer_detach
 * ------------------------------------------------------------------------- */

void writer_detach(writer_t *self)
{
// QUARANTINE   msg_debug("%s: %s\n", __FUNCTION__, self->wr_path);

  if( self->wr_file != -1 )
  {
    writer_flush(self);
    close(self->wr_file);
    self->wr_file = -1;
  }
  
  if( self->wr_path != 0 )
  {
    free(self->wr_path);
    self->wr_path = 0;
  }
}

/* ------------------------------------------------------------------------- *
 * writer_attach
 * ------------------------------------------------------------------------- */

int writer_attach(writer_t *self, const char *path)
{
// QUARANTINE   msg_debug("%s: %s\n", __FUNCTION__, self->wr_path);

  int err = -1;
  
  writer_detach(self);
  self->wr_path = strdup(path ? path : "<stdout>");
  
  if( path == 0 )
  {
    self->wr_file = STDOUT_FILENO;
  }
  else
  {
    if( (self->wr_file = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0666)) == -1 )
    {
      msg_error("can't open '%s' for writing\n", path);
      goto cleanup;
    }
  }
  
  err = 0;
  
  cleanup:
  
// QUARANTINE   msg_debug("%s: %s -> %d\n", __FUNCTION__, self->wr_path, err);
 
  return err;
}

/* ------------------------------------------------------------------------- *
 * writer_create
 * ------------------------------------------------------------------------- */

writer_t *writer_create(void)
{
// QUARANTINE   msg_debug("%s\n", __FUNCTION__);

  writer_t *self = calloc(1, sizeof *self);
  writer_ctor(self);
  return self;
}

/* ------------------------------------------------------------------------- *
 * writer_delete
 * ------------------------------------------------------------------------- */

void writer_delete(writer_t *self)
{
// QUARANTINE   msg_debug("%s: %s\n", __FUNCTION__, self->wr_path);

  if( self != 0 )
  {
    writer_dtor(self);
    free(self);
  }
}
