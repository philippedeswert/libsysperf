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
 * File: csv_table.h
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 12-Oct-2006 Simo Piiroinen
 * - added csvtext_global_replace_char_hack
 *
 * 27-Sep-2006 Simo Piiroinen
 * - added csv_delrow, csv_delrow_nocompact, csv_compactrows
 *
 * 21-Sep-2006 Simo Piiroinen
 * - fixed include paths
 *
 * 30-May-2006 Simo Piiroinen
 * - added csv_save_as_html()
 *
 * 17-May-2006 Simo Piiroinen
 * - new version
 *
 * 05-Oct-2005 Simo Piiroinen
 * - include file fixes
 *
 * 22-Sep-2005 Simo Piiroinen
 * - added csv_ncols, csv_nrows, ...
 *
 * 14-Sep-2005 Simo Piiroinen
 * - added csv_rowcalc()
 *
 * 22-Jul-2005 Simo Piiroinen
 * - csv API cleanup
 *
 * 14-Jul-2005 Simo Piiroinen
 * - added missing prototypes
 *
 * 28-Jun-2005 Simo Piiroinen
 * - added sort, unique, reverse and column shuffle functionality
 * - it is now possible to trim headers, labels & separator lines
 *   from csv output
 *
 * 22-Jun-2005 Simo Piiroinen
 * - added csv_getlabel_ex()
 *
 * 21-Jun-2005 Simo Piiroinen
 * - added include str_pool.h
 * - initial version
 * ========================================================================= */

#ifndef CSV_H_
#define CSV_H_

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <float.h>

#include "array.h"
#include "cstring.h"

#define CSV_EPSILON DBL_EPSILON

/* ========================================================================= *
 * Data Structures
 * ========================================================================= */

typedef struct csvvar_t     csvvar_t;   // key = value string pair
typedef struct csvtext_t    csvtext_t;  // cell strings are interned
typedef struct csvcell_t    csvcell_t;  // double/string container
typedef struct csvrow_t     csvrow_t;   // array of cells
typedef struct csv_t        csv_t;      // table of cells

typedef struct csvord_t csvord_t;

// QUARANTINE typedef struct csvshuffle_t csvshuffle_t; // column shuffle book keeping

/* ------------------------------------------------------------------------- *
 * csvvar_t  --  csv file header variables
 * ------------------------------------------------------------------------- */

struct csvvar_t
{
  char *cv_key;
  char *cv_val;
};

/* ------------------------------------------------------------------------- *
 * csvtext_t  --  interned strings
 * ------------------------------------------------------------------------- */

struct csvtext_t
{
  csvtext_t *ct_next;
  uint32_t   ct_hash;
  size_t     ct_size;
  char       ct_text[];
};

/* ------------------------------------------------------------------------- *
 * csvcell_t  --  numberic / textual value
 * ------------------------------------------------------------------------- */

struct csvcell_t
{
  double      cc_number;
  const char *cc_string;
  unsigned    cc_flags;
};

enum
{
  CF_USR1 = (1u<<0),
  CF_USR2 = (1u<<1),
  CF_USR3 = (1u<<2),
  CF_USR4 = (1u<<3),
};

#define CSVCELL_ZERO {0.0,  0, 0}
#define CSVCELL_NULL {0.0, "", 0}

/* ------------------------------------------------------------------------- *
 * csvrow_t  --  row of cells
 * ------------------------------------------------------------------------- */

struct csvrow_t
{
  int        cr_flags;
  int        cr_cols;

  csvcell_t  cr_celltab[];
};

enum
{
  RF_USR1 = (1u<<0),
  RF_USR2 = (1u<<1),
  RF_USR3 = (1u<<2),
  RF_USR4 = (1u<<3),

  RF_ACTIVE = (1u<<10),
  RF_MASKLO = (1u<<11),
  RF_MASKHI = (1u<<12),
};

enum
{
  CTF_NO_HEADER     = (1u<<0), // omit headers while saving
  CTF_NO_LABELS     = (1u<<1), // omit labels while saving
  CTF_NO_TERMINATOR = (1u<<2), // omit empty line after data
};

/* ------------------------------------------------------------------------- *
 * csv_t  --  table of cells with variables
 * ------------------------------------------------------------------------- */

struct csv_t
{
  array_t    csv_head; // array_t<csvvar_t *>
  int        csv_flags;

  int        csv_rowcnt;
  int        csv_rowmax;

  unsigned  *csv_colflags;

  csvrow_t  *csv_labtab;
  csvrow_t **csv_rowtab;

  char      *csv_sepstr;

  char      *csv_source;
};

/* ------------------------------------------------------------------------- *
 * csvshuffle_t  --  column reordering book keeping
 * ------------------------------------------------------------------------- */

// QUARANTINE struct csvshuffle_t
// QUARANTINE {
// QUARANTINE   int cs_src;      // number of cols before shuffle
// QUARANTINE   int cs_dst;    // number of cols after shuffle
// QUARANTINE   int cs_map[0]; // col[cs_map[i]] <- col[i]
// QUARANTINE
// QUARANTINE };

/* ------------------------------------------------------------------------- *
 * csvord_t
 * ------------------------------------------------------------------------- */

struct csvord_t
{
  int    co_cols;
  int   *co_forw;
  int   *co_back;
};

/* ========================================================================= *
 * csvvar_t  --  methods
 * ========================================================================= */

void      csvvar_ctor     (csvvar_t *self);
void      csvvar_dtor     (csvvar_t *self);
csvvar_t *csvvar_create   (const char *key, const char *val);
void      csvvar_delete   (csvvar_t *self);
void      csvvar_delete_cb(void *self);
void      csvvar_setval   (csvvar_t *self, const char *val);

/* ========================================================================= *
 * csvtext_t  --  methods
 * ========================================================================= */

extern const char csvtext_empty[];

const char *csvtext_intern (const char *text);
int         csvtext_compare(const char *s1, const char *s2);

void csvtext_global_replace_char_hack(int from, int to);

/* ========================================================================= *
 * csvcell_t  --  methods
 * ========================================================================= */

void        csvcell_ctor               (csvcell_t *self);
void        csvcell_dtor               (csvcell_t *self);
int         csvcell_isstring           (const csvcell_t *self);
int         csvcell_isempty            (const csvcell_t *self);
int         csvcell_isnumber           (const csvcell_t *self);
int         csvcell_iszero             (const csvcell_t *self);
double      csvcell_getnumber          (const csvcell_t *self);
const char *csvcell_getstring          (const csvcell_t *self, char *buff, size_t size);
void        csvcell_setstring          (csvcell_t *self, const char *text);
void        csvcell_setnumber          (csvcell_t *self, double numb);
void        csvcell_setauto            (csvcell_t *self, const char *text);
csvcell_t  *csvcell_create             (void);
void        csvcell_delete             (csvcell_t *self);
void        csvcell_delete_cb          (void *self);
int         csvcell_compare            (const csvcell_t *a, const csvcell_t *b);
double      csvcell_diff               (const csvcell_t *a, const csvcell_t *b);
int         csvcell_compare_cb         (const void *a, const void *b);
int         csvcell_compare_indirect_cb(const void *a, const void *b);

/* ========================================================================= *
 * csvrow_t  --  methods
 * ========================================================================= */

size_t           csvrow_sizeof             (int cols);
csvcell_t       *csvrow_getcell            (const csvrow_t *self, int col);
int              csvrow_isstring           (const csvrow_t *self, int col);
int              csvrow_isnumber           (const csvrow_t *self, int col);
double           csvrow_getnumber          (const csvrow_t *self, int col);
const char      *csvrow_getstring          (const csvrow_t *self, int col, char *buff, size_t size);
void             csvrow_setstring          (csvrow_t *self, int col, const char *text);
void             csvrow_setnumber          (csvrow_t *self, int col, double numb);
void             csvrow_setauto            (csvrow_t *self, int col, const char *text);
csvrow_t        *csvrow_create             (int cols);
void             csvrow_delete             (csvrow_t *self);
int              csvrow_addcol             (csvrow_t **pself);
int              csvrow_remcol             (csvrow_t **pself, int col);
void             csvrow_delete_cb          (void *self);
int              csvrow_compare            (const csvrow_t *row1, const csvrow_t *row2);
int              csvrow_compare_cb         (const void *row1, const void *row2);
int              csvrow_compare_indirect_cb(const void *row1p, const void *row2p);

/* ========================================================================= *
 * csvord_t  --  methods
 * ========================================================================= */

csvord_t *csvord_create (csv_t *csv, const char *labels, int remove_rest);
void      csvord_delete (csvord_t *self);
void      csvord_apply  (csvord_t *self, csv_t *csv);
void      csvord_unapply(csvord_t *self, csv_t *csv);

/* ========================================================================= *
 * csv_t  --  methods
 * ========================================================================= */

void        csv_setseparator(csv_t *self, const char *sep);
int         csv_cols        (const csv_t *self);
int         csv_rows        (const csv_t *self);
int         csv_colcheck    (const csv_t *self, int col);
int         csv_rowcheck    (const csv_t *self, int row);
unsigned    csv_getcolflags (const csv_t *self, int col);
unsigned    csv_tstcolflags (const csv_t *self, int col, unsigned mask);
void        csv_setcolflags (const csv_t *self, int col, unsigned mask);
void        csv_addcolflags (const csv_t *self, int col, unsigned mask);
void        csv_clrcolflags (const csv_t *self, int col, unsigned mask);
unsigned    csv_getrowflags (const csv_t *self, int row);
void        csv_setrowflags (csv_t *self, int row, unsigned mask);
void        csv_addrowflags (csv_t *self, int row, unsigned mask);
void        csv_clrrowflags (csv_t *self, int row, unsigned mask);
unsigned    csv_tstrowflags (const csv_t *self, int row, unsigned mask);
csvrow_t   *csv_getrow      (const csv_t *self, int row);
csvcell_t  *csv_getcell     (const csv_t *self, int row, int col);
int         csv_isstring    (const csv_t *self, int row, int col);
int         csv_isnumber    (const csv_t *self, int row, int col);
double      csv_getnumber   (const csv_t *self, int row, int col);
const char *csv_getstring   (const csv_t *self, int row, int col, char *buff, size_t size);
void        csv_setstring   (csv_t *self, int row, int col, const char *text);
void        csv_setnumber   (csv_t *self, int row, int col, double numb);
void        csv_setauto     (csv_t *self, int row, int col, const char *text);
void        csv_setsource   (csv_t *self, const char *path);
const char *csv_getsource   (const csv_t *self);
void        csv_ctor        (csv_t *self);
void        csv_dtor        (csv_t *self);
csv_t      *csv_create      (void);
void        csv_delete      (csv_t *self);
void        csv_delete_cb   (void *self);
void        csv_addvar      (csv_t *self, const char *key, const char *val);

csvrow_t   *csv_newrow      (csv_t *self);
void        csv_delrow      (csv_t *self, int row);
void        csv_delrow_nocompact(csv_t *self, int row);
void        csv_compactrows (csv_t *self);

int         csv_addcol      (csv_t *self, const char *lab);
void        csv_remcol      (csv_t *self, int col);
int         csv_getcol      (csv_t *self, const char *lab);
int         csv_index       (csv_t *self, const char *lab);
const char *csv_label       (const csv_t *self, int col);
int         csv_load        (csv_t *self, const char *path);
int         csv_save        (csv_t *self, const char *path);
int         csv_save_as_html(csv_t *self, const char *path);
void        csv_sortrows    (csv_t *self);
int         csv_op_calc     (csv_t *self, const char *expr);
void        csv_op_sort     (csv_t *self, const char *labels);
void        csv_op_uniq     (csv_t *self, const char *labels);
void        csv_op_usecols  (csv_t *self, const char *labels);
void        csv_op_remcols  (csv_t *self, const char *labels);
void        csv_op_origin   (csv_t *self, const char *labels);
void        csv_op_order    (csv_t *self, const char *labels);
void        csv_op_reverse  (csv_t *self);
int         csv_op_select   (csv_t *self, const char *expr);
int         csv_filter      (csv_t *self, const char *expression, const char *defop);

#ifdef __cplusplus
};
#endif

#endif /* CSV_H_ */
