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
 * File: argvec.h
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 21-Jun-2005 Simo Piiroinen
 * - initial version
 * ========================================================================= */

#ifndef ARGVEC_H_
#define ARGVEC_H_

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif

/* ------------------------------------------------------------------------- *
 * manual_t
 * ------------------------------------------------------------------------- */

typedef struct manual_t
{
  const char *man_sect;
  const char *man_text;

} manual_t;

#define MAN_ADD(s,c) { s, c },
#define MAN_END      { 0, 0 }

/* ------------------------------------------------------------------------- *
 * option_t
 * ------------------------------------------------------------------------- */

typedef struct option_t
{
  int         op_tag;
  const char *op_short;
  const char *op_long;
  const char *op_param;
  const char *op_usage;
} option_t;

#define OPT_ADD(tag,sn,ln,ar,us) { tag, sn, ln, ar, us }
#define OPT_END { -1, }

/* ------------------------------------------------------------------------- *
 * argvec_t
 * ------------------------------------------------------------------------- */

typedef struct argvec_t
{
  int       av_argc;  // argv from main()
  char    **av_argv;  //

  int       av_iarg; // parse pos: av_argv[av_iarg][av_ichr]
  int       av_ichr; //

  int       av_done; // all args handled succesully

  const
  option_t *av_opt; //

  const
  manual_t *av_man;
} argvec_t;

/* ------------------------------------------------------------------------- *
 * prototypes
 * ------------------------------------------------------------------------- */

int argvec_done(argvec_t *self);
argvec_t *argvec_create(int argc, char **argv, const option_t *opt, const manual_t *man);
void argvec_delete(argvec_t *self);
void argvec_options(argvec_t *self, int verbose);
void argvec_usage(argvec_t *self);
char *argvec_pull(argvec_t *self);
int argvec_next(argvec_t *self, int *ptag, char **pstr);

#ifdef __cplusplus
};
#endif

#endif /* ARGVEC_H_ */
