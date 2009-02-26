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
 * File: reader.c
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 22-Jun-2005 11:58:01
 * - line count was increased twice per row
 *
 * 21-Jun-2005 Simo Piiroinen
 * - added stream support
 * - initial version
 * ========================================================================= */

/* ========================================================================= *
 * Include files
 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <math.h>

#include <assert.h>

#include "msg.h"
#include "reader.h"

/* ========================================================================= *
 * Local Functions
 * ========================================================================= */

/* reader.c */
static void reader_dummy_detach(reader_t *self);
static int reader_dummy_read(reader_t *self, void *buff, int size);
static char *reader_dummy_readline(reader_t *self, char **pbuff, size_t *psize);
static int reader_dummy_attach(reader_t *self);
static void reader_stream_detach(reader_t *self);
static int reader_stream_read(reader_t *self, void *buff, int size);
static char *reader_stream_readline(reader_t *self, char **pbuff, size_t *psize);
static int reader_stream_attach(reader_t *self, FILE *file);
static void reader_mmap_detach(reader_t *self);
static int reader_mmap_read(reader_t *self, void *buff, int size);
static char *reader_mmap_readline(reader_t *self, char **pbuff, size_t *psize);
static int reader_mmap_attach(reader_t *self, int file, size_t size);

/* ========================================================================= *
 * dummy reader
 * ========================================================================= */

static void reader_dummy_detach(reader_t *self)
{
}

static int reader_dummy_read(reader_t *self, void *buff, int size)
{
  return -1;
}

static char *reader_dummy_readline(reader_t *self, char **pbuff, size_t *psize)
{
  return 0;
}

static reader_vtab_t vtab_dummy =
{
  .detach   = reader_dummy_detach,
  .read     = reader_dummy_read,
  .readline = reader_dummy_readline,
};

static int reader_dummy_attach(reader_t *self)
{
  msg_debug("@ %s() ...\n", __FUNCTION__);
  self->vtab = vtab_dummy;
  return 0;
}

/* ========================================================================= *
 * stream reader
 * ========================================================================= */

static void reader_stream_detach(reader_t *self)
{
  if( self->file != 0 )
  {
    fclose(self->file);
    self->file = 0;
  }
}

static int reader_stream_read(reader_t *self, void *buff, int size)
{
  return fread(buff, 1, size, self->file);
}

static char *reader_stream_readline(reader_t *self, char **pbuff, size_t *psize)
{
  char *line = 0;

  ssize_t n = getline(pbuff, psize, self->file);

  if( n > 0 )
  {
    line = *pbuff;

    if( line[n-1] == '\n' )
    {
      line[n-1] = 0;
    }
  }
  return line;
}

static reader_vtab_t vtab_stream =
{
  .detach   = reader_stream_detach,
  .read     = reader_stream_read,
  .readline = reader_stream_readline,
};

static int reader_stream_attach(reader_t *self, FILE *file)
{
  msg_debug("@ %s() ...\n", __FUNCTION__);

  self->vtab = vtab_stream;
  self->file = file;
  return 0;
}

/* ========================================================================= *
 * mmap reader
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * reader_mmap_eof
 * ------------------------------------------------------------------------- */

static inline int reader_mmap_eof(reader_t *self)
{
  return self->curr == self->tail;
}

static void reader_mmap_detach(reader_t *self)
{
  if( self->head != 0 )
  {
    munmap(self->head, self->tail - self->head);

    self->head = 0;
    self->tail = 0;
    self->curr = 0;
  }
}

static int reader_mmap_read(reader_t *self, void *buff, int size)
{
  return -1;
}

static char *reader_mmap_readline(reader_t *self, char **pbuff, size_t *psize)
{
  char *line = 0;

  if( !reader_mmap_eof(self) )
  {
    char   *buff = *pbuff;
    size_t  size = *psize;

    char  *beg = self->curr;
    char  *end = self->curr;

    while( end < self->tail && *end != '\n' ) ++end;

    size_t len = end-beg;

    if( len >= size )
    {
      size = len + 256;
      buff = realloc(buff, size);
    }

    memcpy(buff, beg, len);
    buff[len] = 0;

    self->curr = (end < self->tail) ? (end+1): end;

    //self->line += 1;

    *pbuff = line = buff;
    *psize = size;
  }
  return line;
}

static reader_vtab_t vtab_mmap =
{
  .detach   = reader_mmap_detach,
  .read     = reader_mmap_read,
  .readline = reader_mmap_readline,
};

static int reader_mmap_attach(reader_t *self, int file, size_t size)
{
  msg_debug("@ %s() ...\n", __FUNCTION__);

  int   err  = -1;
  void *addr = MAP_FAILED;

  if( (addr = mmap(0,size,PROT_READ,MAP_SHARED,file,0)) != MAP_FAILED )
  {
    self->vtab  = vtab_mmap;

    self->head  = addr;
    self->tail  = addr;
    self->curr  = addr;

    self->tail += size;

    err = 0;
  }

  msg_debug("mmap: file=%d, size=%d, err=%d\n", file, size, err);

  return err;
}

/* ------------------------------------------------------------------------- *
 * reader_ctor
 * ------------------------------------------------------------------------- */

static inline void reader_ctor(reader_t *self)
{
  self->vtab = vtab_dummy;

  self->path = 0;
  self->line = 0;

  self->file = 0;

  self->head = 0;
  self->tail = 0;
  self->curr = 0;
}

/* ------------------------------------------------------------------------- *
 * reader_dtor
 * ------------------------------------------------------------------------- */

static inline void reader_dtor(reader_t *self)
{
  reader_detach(self);
}

/* ------------------------------------------------------------------------- *
 * reader_create
 * ------------------------------------------------------------------------- */

reader_t *reader_create(void)
{
  reader_t *self = calloc(1, sizeof *self);
  reader_ctor(self);
  return self;
}

/* ------------------------------------------------------------------------- *
 * reader_delete
 * ------------------------------------------------------------------------- */

void reader_delete(reader_t *self)
{
  if( self != 0 )
  {
    reader_dtor(self);
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * reader_readline
 * ------------------------------------------------------------------------- */

char *reader_readline(reader_t *self, char **pbuff, size_t *psize)
{
  char *line = self->vtab.readline(self, pbuff, psize);
  self->line += (line != 0);
  return line;
}

int reader_read(reader_t *self, void *buff, size_t size)
{
  return self->vtab.read(self, buff, size);
}

void reader_detach(reader_t *self)
{
  self->vtab.detach(self);

  free(self->path);
  self->path = 0;

  reader_dummy_attach(self);
}

int reader_attach(reader_t *self, const char *path)
{
  int   err    = -1;
  int   file   = -1;
  FILE *stream = 0;

  struct stat st;

  /* - - - - - - - - - - - - - - - - - - - *
   * detach from previous
   * - - - - - - - - - - - - - - - - - - - */

  reader_detach(self);

  /* - - - - - - - - - - - - - - - - - - - *
   * set up position
   * - - - - - - - - - - - - - - - - - - - */

  self->path = strdup(path ? path : "<stdin>");
  self->line = 0;

  /* - - - - - - - - - - - - - - - - - - - *
   * attach according to file type
   * - - - - - - - - - - - - - - - - - - - */

  if( path == 0 )
  {
    if( fstat(STDIN_FILENO, &st) == 0 &&
        S_ISREG(st.st_mode) )
    {
      if( (err = reader_mmap_attach(self, STDIN_FILENO, st.st_size)) != 0 )
      {
        msg_error("could not mmap '%s'\n", self->path);
        goto cleanup;
      }
    }
    else
    {
      err = reader_stream_attach(self, stdin);
    }
  }
  else
  {
    if( (file = open(path, O_RDONLY)) == -1 )
    {
      msg_error("could not open '%s'\n", path);
      goto cleanup;
    }

    if( fstat(file, &st) != 0 )
    {
      msg_error("could not stat '%s'\n", path);
      goto cleanup;
    }

    if( S_ISREG(st.st_mode) )
    {
      if( (err = reader_mmap_attach(self, file, st.st_size)) != 0 )
      {
        msg_error("could not mmap '%s'\n", path);
        goto cleanup;
      }
    }
    else
    {
      if( (stream = fdopen(file, "r")) == 0 )
      {
        msg_error("could not fdopen '%s'\n", path);
        goto cleanup;
      }
      file = -1;

      if( (err = reader_stream_attach(self, stream)) != 0 )
      {
        msg_error("could not attach '%s' stream\n", path);
      }
    }
  }

  cleanup:

  if( stream != 0 ) fclose(stream);
  if( file != -1 ) close(file);

  return err;
}
