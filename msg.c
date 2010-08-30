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
 * File: msg.c
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 * 
 * 30-May-2006 Simo Piiroinen
 * - added msg_perror()
 * 
 * 09-Sep-2005 Simo Piiroinen
 * - defaults to verbosity >= warnings
 *
 * 29-Jun-2005 Simo Piiroinen
 * - added msg_setsilent()
 * 
 * 21-Jun-2005 Simo Piiroinen
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

#include "msg.h"

/* ========================================================================= *
 * Module data & functions
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * msg_progname  --  identification string emitted before messages
 * ------------------------------------------------------------------------- */

static char msg_progname[32];

/* ------------------------------------------------------------------------- *
 * msg_verbosity  --  current verbosity level
 * ------------------------------------------------------------------------- */

//static int  msg_verbosity = MSG_DEBUG;
static int  msg_verbosity = MSG_WARNING;

/* ------------------------------------------------------------------------- *
 * internal functions
 * ------------------------------------------------------------------------- */

static void msg_defprogname(void);
static void msg_emit_va(const char *tag, const char *fmt, va_list va);

/* ========================================================================= *
 * Program name handling API
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * msg_defprogname  --  obtain progname from /proc/self/cmdline
 * ------------------------------------------------------------------------- */

static void msg_defprogname(void)
{
  char temp[256];
  int  file = -1;

  *msg_progname = 0;
  
  if( (file = open("/proc/self/cmdline", O_RDONLY)) != -1 )
  {
    int n = read(file, temp, sizeof temp-1);
    char *s,*b;
    
    if( n > 0 )
    {
      temp[n] = 0;
    
      for( s = b = temp; *s; ++s )
      {
	if( *s == '/' ) b = s + 1;
      }
      
      strncat(msg_progname, b, sizeof msg_progname-1);
    }
    close(file);
  }
  
  if( *msg_progname == 0 )
  {
    strcpy(msg_progname, "<unknown>");
  }
}

/* ------------------------------------------------------------------------- *
 * msg_getprogname  --  get program name currently used for messages
 * ------------------------------------------------------------------------- */

const char *msg_getprogname(void)
{
  if( *msg_progname == 0 )
  {
    msg_defprogname();
  }
  return msg_progname; 
}

/* ------------------------------------------------------------------------- *
 * msg_setprogname  --  set program name currently used for messages
 * ------------------------------------------------------------------------- */

void msg_setprogname(const char *progname)
{
  if( progname == 0 )
  {
    msg_defprogname();
  }
  else
  {
    *msg_progname = 0;
    strncat(msg_progname, progname, sizeof msg_progname-1);
  }
}

/* ========================================================================= *
 * Message output "engine"
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * msg_emit_va  --  main message emitter function using va_list
 * ------------------------------------------------------------------------- */

static void msg_emit_va(const char *tag, const char *fmt, va_list va)
{
  char temp[1<<10];
  int  left = 0;
  char *pos, *end;
  
  left += fprintf(stderr, "%s: ", msg_getprogname());
  
  if( tag != 0 )
  {
    left += 
    fprintf(stderr, "%s: ", tag);
  }
  
  vsnprintf(temp, sizeof temp, fmt, va);
  for( pos = temp; ; )
  {
    end = pos + strcspn(pos, "\n");
    fprintf(stderr, "%.*s\n", (int)(end-pos), pos);
    
    pos = end + (*end != 0);
    if( *pos == 0 ) break;
    
    fprintf(stderr, "%-*s", left, "|");
  }
}

/* ------------------------------------------------------------------------- *
 * msg_emit  --  emit freeform messages
 * ------------------------------------------------------------------------- */

// QUARANTINE void msg_emit(const char *tag, const char *fmt, ...)
// QUARANTINE {
// QUARANTINE   if( msg_verbosity >= MSG_FATAL )
// QUARANTINE   {
// QUARANTINE     va_list va;
// QUARANTINE     va_start(va, fmt);
// QUARANTINE     msg_emit_va(tag,fmt,va);
// QUARANTINE     va_end(va);
// QUARANTINE   }
// QUARANTINE }

/* ========================================================================= *
 * Message emitting API
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * msg_fatal  --  report fatal error and exit
 * ------------------------------------------------------------------------- */

void msg_fatal(const char *fmt, ...)
{
  if( msg_verbosity >= MSG_FATAL )
  {
    va_list va;
    va_start(va, fmt);
    msg_emit_va("FATAL",fmt,va);
    va_end(va);
  }
  exit(EXIT_FAILURE);
}

/* ------------------------------------------------------------------------- *
 * msg_error  --  report survivable error condition
 * ------------------------------------------------------------------------- */

void msg_error(const char *fmt, ...)
{
  if( msg_verbosity >= MSG_ERROR )
  {
    va_list va;
    va_start(va, fmt);
    msg_emit_va("ERROR",fmt,va);
    va_end(va);
  }
}

/* ------------------------------------------------------------------------- *
 * msg_perror  --  report survivable error condition + errno as string
 * ------------------------------------------------------------------------- */

void msg_perror(const char *fmt, ...)
{
  if( msg_verbosity >= MSG_ERROR )
  {
    char *err = strerror(errno);
    char  tmp[1024];
    va_list va;
    va_start(va, fmt);
    vsnprintf(tmp, sizeof tmp, fmt, va);
    va_end(va);
    
    msg_error("%s: %s\n", tmp, err);
  }
}

/* ------------------------------------------------------------------------- *
 * msg_warning  --  report minor problem
 * ------------------------------------------------------------------------- */

void msg_warning_(const char *fmt, ...)
{
  if( msg_verbosity >= MSG_WARNING )
  {
    va_list va;
    va_start(va, fmt);
    msg_emit_va("Warning",fmt,va);
    va_end(va);
  }
}

/* ------------------------------------------------------------------------- *
 * msg_progress  --  emit progress message
 * ------------------------------------------------------------------------- */

void msg_progress_(const char *fmt, ...)
{
  if( msg_verbosity >= MSG_PROGRESS )
  {
    va_list va;
    va_start(va, fmt);
    msg_emit_va(0,fmt,va);
    va_end(va);
  }
}

/* ------------------------------------------------------------------------- *
 * msg_debug  --  emit debugging message
 * ------------------------------------------------------------------------- */

void msg_debug_(const char *fmt, ...)
{
  if( msg_verbosity >= MSG_DEBUG )
  {
    va_list va;
    va_start(va, fmt);
    msg_emit_va("debug",fmt,va);
    va_end(va);
  }
}

/* ========================================================================= *
 * Verbosity control API
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * msg_setverbosity  --  explicitly set verbosity level
 * ------------------------------------------------------------------------- */

void msg_setverbosity(int level)
{
  msg_verbosity = level;
}

/* ------------------------------------------------------------------------- *
 * msg_getverbosity  -- get current verbosity level
 * ------------------------------------------------------------------------- */

int msg_getverbosity(void)
{
  return msg_verbosity;
}

/* ------------------------------------------------------------------------- *
 * msg_incverbosity  --  more verbose
 * ------------------------------------------------------------------------- */

void msg_incverbosity(void)
{
  ++msg_verbosity;
}

/* ------------------------------------------------------------------------- *
 * msg_decverbosity  --  less verbose
 * ------------------------------------------------------------------------- */

void msg_decverbosity(void)
{
  --msg_verbosity;
}
/* ------------------------------------------------------------------------- *
 * msg_setsilent  --  only fatal errors emitted
 * ------------------------------------------------------------------------- */

void msg_setsilent(void)
{
  msg_verbosity = MSG_SILENT;
}

/* ========================================================================= *
 * TEST MAIN
 * ========================================================================= */


#if 0
int main(int ac, char **av)
{
  while( ++av, --ac )
  {
// QUARANTINE     msg_emit("emit", "goo in\nfile %s detected\n", *av);

    msg_debug("goo in\nfile %s detected\n", *av);
    msg_progress("goo in\nfile %s detected\n", *av);
    msg_warning("goo in\nfile %s detected\n", *av);
    msg_error("goo in\nfile %s detected\n", *av);
    msg_fatal("goo\nin\nfile\n%s\ndetected\n", *av);
  }

  return 0;
}
#endif
