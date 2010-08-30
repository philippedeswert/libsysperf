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
 * File: reader.h
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 * 
 * 05-Oct-2005 Simo Piiroinen
 * - include file fixes
 *
 * 21-Jun-2005 Simo Piiroinen
 * - added stream support
 * - initial version
 * ========================================================================= */

#ifndef READER_H_
#define READER_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif


typedef struct reader_t reader_t;

typedef struct reader_vtab_t
{
  void  (*detach)  (reader_t *);
  int   (*read)    (reader_t *, void *, int);
  char *(*readline)(reader_t *, char **, size_t *);
 
} reader_vtab_t;

struct reader_t
{
  reader_vtab_t vtab;

  char *path;
  int   line;
  
  /* - - - - - - - - - - - - - - - - - - - *
   * stream
   * - - - - - - - - - - - - - - - - - - - */

  FILE *file;
  
  /* - - - - - - - - - - - - - - - - - - - *
   * mmap
   * - - - - - - - - - - - - - - - - - - - */

  char *head;
  char *tail;
  char *curr;
};


/* reader.c */
reader_t *reader_create(void);
void reader_delete(reader_t *self);
char *reader_readline(reader_t *self, char **pbuff, size_t *psize);
int reader_read(reader_t *self, void *buff, size_t size);
void reader_detach(reader_t *self);
int reader_attach(reader_t *self, const char *path);


#ifdef __cplusplus
};
#endif

#endif /* READER_H_ */
