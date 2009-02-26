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
 * File: msg.h
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 * 25-Feb-2009 Simo Piiroinen
 * - added format attributes for logging functions so that gcc
 *   can do type checking at call points
 *
 * 30-May-2006 Simo Piiroinen
 * - added msg_perror()
 *
 * 29-Jun-2005 Simo Piiroinen
 * - added msg_setsilent()
 *
 * 22-Jun-2005 Simo Piiroinen
 * - now allows local override of MSG_DISABLE_XXX
 *
 * 21-Jun-2005 Simo Piiroinen
 * - initial version
 * ========================================================================= */

#ifndef MSG_H_
#define MSG_H_

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif

#ifndef  MSG_DISABLE_WARNING
# define MSG_DISABLE_WARNING  0
#endif

#ifndef  MSG_DISABLE_PROGRESS
# define MSG_DISABLE_PROGRESS 0
#endif

#ifndef  MSG_DISABLE_DEBUG
# define MSG_DISABLE_DEBUG    01
#endif

/* ------------------------------------------------------------------------- *
 * verbosity levels
 * ------------------------------------------------------------------------- */

enum
{
  MSG_SILENT,
  MSG_FATAL,
  MSG_ERROR,
  MSG_WARNING,
  MSG_PROGRESS,
  MSG_DEBUG,
};

/* ------------------------------------------------------------------------- *
 * API functions
 * ------------------------------------------------------------------------- */

const char *msg_getprogname(void);
void msg_setprogname(const char *progname);

void msg_fatal(const char *fmt, ...)     __attribute__((format(printf,1,2),noreturn));
void msg_error(const char *fmt, ...)     __attribute__((format(printf,1,2)));
void msg_perror(const char *fmt, ...)    __attribute__((format(printf,1,2)));

void msg_warning_(const char *fmt, ...)  __attribute__((format(printf,1,2)));
void msg_progress_(const char *fmt, ...) __attribute__((format(printf,1,2)));
void msg_debug_(const char *fmt, ...)    __attribute__((format(printf,1,2)));

#if MSG_DISABLE_WARNING
# define msg_warning(fmt, ...) do{}while(0)
#else
# define msg_warning(fmt, ...) msg_warning_(fmt, ## __VA_ARGS__)
#endif

#if MSG_DISABLE_PROGRESS
# define msg_progress(fmt, ...) do{}while(0)
#else
# define msg_progress(fmt, ...) msg_progress_(fmt, ## __VA_ARGS__)
#endif

#if MSG_DISABLE_DEBUG
# define msg_debug(fmt, ...) do{}while(0)
#else
# define msg_debug(fmt, ...) msg_debug_(fmt, ## __VA_ARGS__)
#endif

void msg_setverbosity(int level);
int  msg_getverbosity(void);
void msg_incverbosity(void);
void msg_decverbosity(void);
void msg_setsilent(void);

#ifdef __cplusplus
};
#endif

#endif /* MSG_H_ */
