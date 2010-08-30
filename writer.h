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
 * File: writer.h
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 22-Jun-2005 Simo Piiroinen
 * - initial version
 * ========================================================================= */

#ifndef WRITER_H_
#define WRITER_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif

/* ========================================================================= *
 * struct writer_t
 * ========================================================================= */

typedef struct writer_t writer_t;

struct writer_t
{
  char *wr_curr;
  char *wr_tail;
  char *wr_head;
  int   wr_file;
  char *wr_path;
};

/* ------------------------------------------------------------------------- *
 * extern methods
 * ------------------------------------------------------------------------- */

void writer_flush(writer_t *self);
void writer_detach(writer_t *self);
int writer_attach(writer_t *self, const char *path);
writer_t *writer_create(void);
void writer_delete(writer_t *self);


/* ------------------------------------------------------------------------- *
 * writer_ctor
 * ------------------------------------------------------------------------- */

static inline void writer_ctor(writer_t *self)
{
  size_t size = 256 << 10;
  char  *addr = malloc(size);
  
  self->wr_head = addr;
  self->wr_curr = addr;
  self->wr_tail = addr + size;
  self->wr_file = -1;
  self->wr_path = 0;
}

/* ------------------------------------------------------------------------- *
 * writer_dtor
 * ------------------------------------------------------------------------- */

static inline void writer_dtor(writer_t *self)
{
  writer_detach(self);
  free(self->wr_head);
}

/* ------------------------------------------------------------------------- *
 * writer_space
 * ------------------------------------------------------------------------- */

static inline size_t writer_space(writer_t *self)
{
  return self->wr_tail - self->wr_curr;
}

/* ------------------------------------------------------------------------- *
 * writer_putc
 * ------------------------------------------------------------------------- */

static inline void writer_putc(writer_t *self, int ch)
{
  if( self->wr_curr == self->wr_tail )
  {
    writer_flush(self);
  }
  *self->wr_curr++ = ch;
}

/* ------------------------------------------------------------------------- *
 * writer_puts
 * ------------------------------------------------------------------------- */

static inline void writer_puts(writer_t *self, const char *str)
{
  for( ;; )
  {
    size_t todo = writer_space(self);
    while( todo-- )
    {
      char ch = *str++;
      if( ch == 0 ) return;
      *self->wr_curr++ = ch;
    }
    writer_flush(self);
  }
  
  //while( *str ) writer_putc(self, *str++);
}

// QUARANTINE static inline void writer_putc_nc(writer_t *self, int ch)
// QUARANTINE {
// QUARANTINE   *self->wr_curr++ = ch;
// QUARANTINE }
// QUARANTINE static inline void writer_puts_nc(writer_t *self, const char *str)
// QUARANTINE {
// QUARANTINE   while( *str )  *self->wr_curr++ = *str++;
// QUARANTINE }
// QUARANTINE static inline void writer_flush_if(writer_t *self, size_t space)
// QUARANTINE {
// QUARANTINE   if( writer_space(self) < space ) { writer_flush(self); }
// QUARANTINE }

#ifdef __cplusplus
};
#endif

#endif /* WRITER_H_ */
