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
 * File: csv_table.c
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
 * 08-Oct-2006 Simo Piiroinen
 * - fix: csv_load sets input source name
 *
 * 27-Sep-2006 Simo Piiroinen
 * - added csv_delrow, csv_delrow_nocompact, csv_compactrows
 *
 * 25-Sep-2006 Simo Piiroinen
 * - sync with calculator changes
 *
 * 21-Sep-2006 Simo Piiroinen
 * - optimized csv_load & csv_save functions
 *
 * 29-Aug-2006 Simo Piiroinen
 * - fixed also csvtext_compare to treat numbers < strings
 *
 * 01-Jun-2006 Simo Piiroinen
 * - fixed incorrect return value for csv_save() and csv_save_as_html
 *
 * 30-May-2006 Simo Piiroinen
 * - added csv_save_as_html()
 *
 * 29-May-2006 Simo Piiroinen
 * - fixed: csv_save wrote some data to stdout
 *
 * 23-May-2006 Simo Piiroinen
 * - filter operators imported from sp_csv_filter application
 *
 * 17-May-2006 Simo Piiroinen
 * - new version
 *
 * 14-Sep-2005 Simo Piiroinen
 * - added csv_rowcalc()
 *
 * 22-Jul-2005 Simo Piiroinen
 * - csv API cleanup
 * - csv_usecols -> csv_usecols & csv_ordercols
 * - remcols shuffling removed from csv_newshuffle
 *
 * 14-Jul-2005 Simo Piiroinen
 * - csv_newshuffle supports 'remcols' shuffling
 *
 * 28-Jun-2005 Simo Piiroinen
 * - added sort, unique, reverse and column shuffle functionality
 * - it is now possible to trim headers, labels & separator lines
 *   from csv output
 * - csv_load() now ignores comment lines
 *
 * 22-Jun-2005 Simo Piiroinen
 * - added csv_getlabel_ex()
 * - rewrote csv_save()
 *
 * 21-Jun-2005 Simo Piiroinen
 * - added stream support
 * - removed test main()
 * - initial version
 * ========================================================================= */

#include "msg.h"
#include "calculator.h"
#include "csv_table.h"
#include "csv_float.h"

// QUARANTINE #include <assert.h>
#include <float.h>
#include <math.h>

#define DEBUG_CSVTEXT 0

#define CELL_FMT "%.10g"

/* ========================================================================= *
 * inline jhash implementation
 * ========================================================================= */

#define u8  uint8_t
#define u32 uint32_t
#include "jhash.h"
#undef  u8
#undef  u32

/* ========================================================================= *
 * csvvar_t  --  methods
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * csvvar_ctor
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvvar_ctor(csvvar_t *self)
{
  self->cv_key = 0;
  self->cv_val = 0;
}

/* ------------------------------------------------------------------------- *
 * csvvar_dtor
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvvar_dtor(csvvar_t *self)
{
  free(self->cv_key);
  free(self->cv_val);
}

/* ------------------------------------------------------------------------- *
 * csvvar_create
 * ------------------------------------------------------------------------- */

/*static inline*/ csvvar_t *
csvvar_create(const char *key, const char *val)
{
  csvvar_t *self = calloc(1, sizeof *self);
  csvvar_ctor(self);
  self->cv_key = strdup(key ? key : "");
  self->cv_val = strdup(val ? val : "");
  return self;
}

/* ------------------------------------------------------------------------- *
 * csvvar_delete
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvvar_delete(csvvar_t *self)
{
  if( self != 0 )
  {
    csvvar_dtor(self);
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * csvvar_delete_cb
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvvar_delete_cb(void *self)
{
  csvvar_delete(self);
}

/* ------------------------------------------------------------------------- *
 * csvvar_setval
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvvar_setval(csvvar_t *self, const char *val)
{
  free(self->cv_val);
  self->cv_val = strdup(val ? val : "");
}

/* ========================================================================= *
 * csvtext_t  --  methods
 * ========================================================================= */

enum { HSIZE = 16 << 10, HMASK = HSIZE - 1 };
static csvtext_t *csvtext_slot[HSIZE];
static size_t     csvtext_uniq;
static size_t     csvtext_adds;

const char csvtext_empty[] = "";

/* ------------------------------------------------------------------------- *
 * csvtext_statistics  --  string intern hash table statistics
 * ------------------------------------------------------------------------- */

#if DEBUG_CSVTEXT
static void csvtext_statistics(void) __attribute__((destructor));

static void
csvtext_statistics(void)
{
  fprintf(stderr, "--\n");
  fprintf(stderr, "%s: %Zd unique strings\n", __FUNCTION__, csvtext_uniq);
  fprintf(stderr, "%s: %Zd added  strings\n", __FUNCTION__, csvtext_adds);
  fprintf(stderr, "--\n");
}
#endif

/* ------------------------------------------------------------------------- *
 * csvtext_release  --  release all interned strings
 * ------------------------------------------------------------------------- */

static void csvtext_release(void) __attribute__((destructor));

static void
csvtext_release(void)
{
  for( int h = 0; h < HSIZE; ++h )
  {
    csvtext_t *t;

    while( (t = csvtext_slot[h]) != 0 )
    {
      csvtext_slot[h] = t->ct_next;
      free(t);
    }
  }
}

/* ------------------------------------------------------------------------- *
 * csvtext_global_replace_char_hack  --  ploticus & categories ....
 * ------------------------------------------------------------------------- */

void
csvtext_global_replace_char_hack(int from, int to)
{
  for( int h = 0; h < HSIZE; ++h )
  {
    for( csvtext_t *t = csvtext_slot[h]; t; t = t->ct_next )
    {
      for( char *s = t->ct_text; *s; ++ s)
      {
        if( *s == from ) *s = to;
      }
    }
  }
}

/* ------------------------------------------------------------------------- *
 * csvtext_intern  --  intern a string
 * ------------------------------------------------------------------------- */

const char *
csvtext_intern(const char *text)
{
  char    *r = 0;
  char    *w = 0;
  char    *s = 0;
  size_t   n = 0;

  /* - - - - - - - - - - - - - - - - - - - *
   * zero pad text to multiple of four size
   * work space for short strings taken from
   * stack, larger ones from heap
   * - - - - - - - - - - - - - - - - - - - */

  if( (n = (strlen(text) + 1u + 3u) & ~3u) < 256 )
  {
    s = alloca(n);
  }
  else
  {
    s = w = malloc(n);
  }

  strncpy(s, text, n);

  /* - - - - - - - - - - - - - - - - - - - *
   * hash table management
   * - - - - - - - - - - - - - - - - - - - */

  uint32_t h = jhash2((uint32_t*)s, n/4, 0);

  for( csvtext_t *p = csvtext_slot[h & HMASK]; ; p = p->ct_next )
  {
    if( p == 0 )
    {
      /* - - - - - - - - - - - - - - - - - - - *
       * add new entry
       * - - - - - - - - - - - - - - - - - - - */

      p = malloc(sizeof *p + n);
      p->ct_next = csvtext_slot[h & HMASK];
      p->ct_hash = h;
      p->ct_size = n;
      memcpy(p->ct_text, s, n);
      csvtext_slot[h & HMASK] = p;
      r = p->ct_text;

      csvtext_uniq += 1;
      break;
    }

    if( p->ct_hash == h && p->ct_size == n && !memcmp(p->ct_text, s, n) )
    {
      /* - - - - - - - - - - - - - - - - - - - *
       * use existing entry
       * - - - - - - - - - - - - - - - - - - - */

      r = p->ct_text;
      break;
    }
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * free temp stack space if used
   * - - - - - - - - - - - - - - - - - - - */

  if( w != 0 )
  {
    free(w);
  }

  csvtext_adds += 1;

  return r;
}

/* ------------------------------------------------------------------------- *
 * csvtext_compare  --  compare two strings
 * ------------------------------------------------------------------------- */

int
csvtext_compare(const char *s1, const char *s2)
{
  int r;
  for( ;; )
  {
    int c1 = (unsigned char)*s1++;
    int c2 = (unsigned char)*s2++;

    if( c1 == 0 || c2 == 0 )
    {
      // early test for end of string, also
      // makes sure "foo3" -> "foo"
      return c1 - c2;
    }

    int d1 = ('0' <= c1) && (c1 <= '9');
    int d2 = ('0' <= c2) && (c2 <= '9');

    if( (r = d2-d1) != 0 )
    {
      // string > number
      return r;
    }

    if( d1 != 0 )
    {
      // number vs number
      unsigned u1 = strtoul(s1-1,(char **)&s1,10);
      unsigned u2 = strtoul(s2-1,(char **)&s2,10);

      if( (r = (u1>u2)-(u1<u2)) != 0 )
      {
        return r;
      }
    }
    else
    {
      // character vs character
      if( (r = c1-c2) != 0 )
      {
        return r;
      }
    }
  }

  //return strcmp(s1, s2);
}

/* ========================================================================= *
 * csvcell_t  --  methods
 * ========================================================================= */

// QUARANTINE static const csvcell_empty = { 0.0, csvtext_empty };
// QUARANTINE static const csvcell_zero  = { 0.0, NULL };

/* ------------------------------------------------------------------------- *
 * csvcell_ctor
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvcell_ctor(csvcell_t *self)
{
  self->cc_number = 0.0;
  self->cc_string = csvtext_empty;
}

/* ------------------------------------------------------------------------- *
 * csvcell_dtor
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvcell_dtor(csvcell_t *self)
{
}

/* ------------------------------------------------------------------------- *
 * csvcell_isstring  --  check if cell is a string
 * ------------------------------------------------------------------------- */

/*static inline*/ int
csvcell_isstring(const csvcell_t *self)
{
  return self->cc_string != NULL;
}

/* ------------------------------------------------------------------------- *
 * csvcell_isempty  --  check if cell is a empty string
 * ------------------------------------------------------------------------- */

/*static inline*/ int
csvcell_isempty(const csvcell_t *self)
{
  return self->cc_string != NULL && *self->cc_string == 0;
}

/* ------------------------------------------------------------------------- *
 * csvcell_isnumber  --  check if cell is a number
 * ------------------------------------------------------------------------- */

/*static inline*/ int
csvcell_isnumber(const csvcell_t *self)
{
  return self->cc_string == NULL;
}

/* ------------------------------------------------------------------------- *
 * csvcell_iszero  --  check if cell is zero number
 * ------------------------------------------------------------------------- */

/*static inline*/ int
csvcell_iszero(const csvcell_t *self)
{
  return self->cc_string == NULL && self->cc_number == 0.0;
}

/* ------------------------------------------------------------------------- *
 * csvcell_getnumber  --  obtain numerical cell value
 * ------------------------------------------------------------------------- */

/*static inline*/ double
csvcell_getnumber(const csvcell_t *self)
{
  // guranteed: zero if non-numerical
  return self->cc_number;
}

/* ------------------------------------------------------------------------- *
 * csvcell_getstring  --  obtain textual cell value
 * ------------------------------------------------------------------------- */

/*static inline*/ const char *
csvcell_getstring(const csvcell_t *self, char *buff, size_t size)
{
  if( self->cc_string != NULL )
  {
    return self->cc_string;
  }
  if( buff != 0 )
  {
    snprintf(buff, size, ""CELL_FMT"", self->cc_number);
  }
  return buff;
}

/* ------------------------------------------------------------------------- *
 * csvcell_setstring  --  set cell to textual value
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvcell_setstring(csvcell_t *self, const char *text)
{
  self->cc_number = 0.0;
  self->cc_string = text ? csvtext_intern(text) : csvtext_empty;
}

/* ------------------------------------------------------------------------- *
 * csvcell_setnumber  --  set cell to numerical value
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvcell_setnumber(csvcell_t *self, double numb)
{
  self->cc_number = numb;
  self->cc_string = NULL;
}

/* ------------------------------------------------------------------------- *
 * csvcell_setauto  --  parse cell value from string -> numeric or textual
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvcell_setauto(csvcell_t *self, const char *text)
{
  char  *end;
  double val = strtod(text, &end);

  if( end != text && *end == 0 )
  {
    csvcell_setnumber(self, val);
  }
  else
  {
    csvcell_setstring(self, text);
  }
}

/* ------------------------------------------------------------------------- *
 * csvcell_create
 * ------------------------------------------------------------------------- */

csvcell_t *
csvcell_create(void)
{
  csvcell_t *self = calloc(1, sizeof *self);
  csvcell_ctor(self);
  return self;
}

/* ------------------------------------------------------------------------- *
 * csvcell_delete
 * ------------------------------------------------------------------------- */

void
csvcell_delete(csvcell_t *self)
{
  if( self != 0 )
  {
    csvcell_dtor(self);
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * csvcell_delete_cb
 * ------------------------------------------------------------------------- */

void
csvcell_delete_cb(void *self)
{
  csvcell_delete(self);
}

/* ------------------------------------------------------------------------- *
 * csvcell_compare  --  compare two cells
 * ------------------------------------------------------------------------- */

int
csvcell_compare(const csvcell_t *a, const csvcell_t *b)
{
  if( a->cc_string != 0 )
  {
    if( b->cc_string != 0 )
    {
      // A = string, B = string -> mixed alpha-numerical comparison
      return csvtext_compare(a->cc_string, b->cc_string);
    }
    // A = string > B = number
    return 1;
  }

  if( b->cc_string != 0 )
  {
    // A = number < B = string
    return -1;
  }

  // A = number, B = number -> numerical comparison
  return (a->cc_number > b->cc_number) - (a->cc_number < b->cc_number);
}

/* ------------------------------------------------------------------------- *
 * csvcell_diff  --  numerical difference of two cells
 * ------------------------------------------------------------------------- */

double
csvcell_diff(const csvcell_t *a, const csvcell_t *b)
{
  /* - - - - - - - - - - - - - - - - - - - *
   * For double vs. double comparison the
   * result is double difference.
   *
   * For other comparisons the result is
   * similar to csvcell_compare except
   * for return type {r <= -1.0, 0.0, >= 1.0}.
   * - - - - - - - - - - - - - - - - - - - */

  if( a->cc_string != 0 )
  {
    if( b->cc_string != 0 )
    {
      // A = string, B = string -> mixed alpha-numerical comparison
      return csvtext_compare(a->cc_string, b->cc_string);
    }
    // A = string > B = number
    return 1;
  }

  if( b->cc_string != 0 )
  {
    // A = number < B = string
    return -1;
  }

  // A = number, B = number -> numerical comparison
  return a->cc_number - b->cc_number;
}

/* ------------------------------------------------------------------------- *
 * csvcell_compare_cb  --  qsort compatible callback for cell arrays
 * ------------------------------------------------------------------------- */

int
csvcell_compare_cb(const void *a, const void *b)
{
  return csvcell_compare(a, b);
}

/* ------------------------------------------------------------------------- *
 * csvcell_compare_indirect_cb  --  qsort compatible callback for cell ptrs
 * ------------------------------------------------------------------------- */

int
csvcell_compare_indirect_cb(const void *a, const void *b)
{
  return csvcell_compare(*(const csvcell_t**)a, *(const csvcell_t**)b);
}

/* ========================================================================= *
 * csvrow_t  --  methods
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * csvrow_sizeof  --  calculate size needed for row of given width
 * ------------------------------------------------------------------------- */

/*static inline*/ size_t
csvrow_sizeof(int cols)
{
  return sizeof(csvrow_t) + cols * sizeof(csvcell_t);
}

/* ------------------------------------------------------------------------- *
 * csvrow_getcell
 * ------------------------------------------------------------------------- */

/*static inline*/ csvcell_t *
csvrow_getcell(const csvrow_t *self, int col)
{
  if( col < 0 || col >= self->cr_cols )
  {
    msg_fatal("csvrow_t: cell access out of bounds [%d/%d]\n",
              col, self->cr_cols);
  }
  return (csvcell_t *)&self->cr_celltab[col];
}

/* ------------------------------------------------------------------------- *
 * csvrow_isstring
 * ------------------------------------------------------------------------- */

/*static inline*/ int
csvrow_isstring(const csvrow_t *self, int col)
{
  return csvcell_isstring(csvrow_getcell(self, col));
}

/* ------------------------------------------------------------------------- *
 * csvrow_isnumber
 * ------------------------------------------------------------------------- */

/*static inline*/ int
csvrow_isnumber(const csvrow_t *self, int col)
{
  return csvcell_isnumber(csvrow_getcell(self, col));
}

/* ------------------------------------------------------------------------- *
 * csvrow_getnumber
 * ------------------------------------------------------------------------- */

/*static inline*/ double
csvrow_getnumber(const csvrow_t *self, int col)
{
  return csvcell_getnumber(csvrow_getcell(self, col));
}

/* ------------------------------------------------------------------------- *
 * csvrow_getstring
 * ------------------------------------------------------------------------- */

/*static inline*/ const char *
csvrow_getstring(const csvrow_t *self, int col, char *buff, size_t size)
{
  return csvcell_getstring(csvrow_getcell(self, col), buff, size);
}

/* ------------------------------------------------------------------------- *
 * csvrow_setstring
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvrow_setstring(csvrow_t *self, int col, const char *text)
{
  csvcell_setstring(csvrow_getcell(self, col), text);
}

/* ------------------------------------------------------------------------- *
 * csvrow_setnumber
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvrow_setnumber(csvrow_t *self, int col, double numb)
{
  csvcell_setnumber(csvrow_getcell(self, col), numb);
}

/* ------------------------------------------------------------------------- *
 * csvrow_setauto
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csvrow_setauto(csvrow_t *self, int col, const char *text)
{
  csvcell_setauto(csvrow_getcell(self, col), text);
}

/* ------------------------------------------------------------------------- *
 * csvrow_create
 * ------------------------------------------------------------------------- */

csvrow_t *
csvrow_create(int cols)
{
  csvrow_t *self = calloc(1, csvrow_sizeof(cols));

  self->cr_flags = 0;
  self->cr_cols  = cols;

  for( int c = 0; c < self->cr_cols; ++c )
  {
    csvcell_ctor(&self->cr_celltab[c]);
  }

  return self;
}

/* ------------------------------------------------------------------------- *
 * csvrow_delete
 * ------------------------------------------------------------------------- */

void
csvrow_delete(csvrow_t *self)
{
  if( self != 0 )
  {
    for( int c = 0; c < self->cr_cols; ++c )
    {
      csvcell_dtor(&self->cr_celltab[c]);
    }
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * csvrow_addcol
 * ------------------------------------------------------------------------- */

int
csvrow_addcol(csvrow_t **pself)
{
  csvrow_t *self = *pself;
  int       col  = self->cr_cols++;

  self = realloc(self, csvrow_sizeof(self->cr_cols));
  csvcell_ctor(&self->cr_celltab[col]);

  *pself = self;
  return col;
}

/* ------------------------------------------------------------------------- *
 * csvrow_remcol
 * ------------------------------------------------------------------------- */

int
csvrow_remcol(csvrow_t **pself, int col)
{
  int       err  = -1;
  csvrow_t *self = *pself;

  if( 0 <= col && col < self->cr_cols )
  {
    csvcell_dtor(&self->cr_celltab[col]);

    csvcell_t *d = &self->cr_celltab[col+0];
    csvcell_t *s = &self->cr_celltab[col+1];
    csvcell_t *e = &self->cr_celltab[self->cr_cols];
    while( s < e )
    {
      *d++ = *s++;
    }

    self->cr_cols -= 1;

    self = realloc(self, csvrow_sizeof(self->cr_cols));
    *pself = self;
    err = 0;
  }
  return err;
}

/* ------------------------------------------------------------------------- *
 * csvrow_delete_cb
 * ------------------------------------------------------------------------- */

void
csvrow_delete_cb(void *self)
{
  csvrow_delete(self);
}

/* ------------------------------------------------------------------------- *
 * csvrow_compare  --  compare two rows in table
 * ------------------------------------------------------------------------- */

int
csvrow_compare(const csvrow_t *row1, const csvrow_t *row2)
{
  int res = 0;

  for( int c = 0; c < row1->cr_cols; ++c )
  {
    const csvcell_t *cell1 = &row1->cr_celltab[c];
    const csvcell_t *cell2 = &row2->cr_celltab[c];
    if( (res = csvcell_compare(cell1, cell2)) != 0 )
    {
      break;
    }
  }
  return res;
}

/* ------------------------------------------------------------------------- *
 * csvrow_compare_cb  --  qsort callback for row arrays
 * ------------------------------------------------------------------------- */

int
csvrow_compare_cb(const void *row1, const void *row2)
{
  return csvrow_compare(row1, row2);
}

/* ------------------------------------------------------------------------- *
 * csvrow_compare_indirect_cb  --  qsort callback for row ptr arrays
 * ------------------------------------------------------------------------- */

int
csvrow_compare_indirect_cb(const void *row1p, const void *row2p)
{
  return csvrow_compare(*(csvrow_t **)row1p, *(csvrow_t **)row2p);
}

/* ========================================================================= *
 * csv_t  --  methods
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * csv_setseparator
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csv_setseparator(csv_t *self, const char *sep)
{
  free(self->csv_sepstr);
  self->csv_sepstr = sep ? strdup(sep) : 0;
}

/* ------------------------------------------------------------------------- *
 * csv_cols
 * ------------------------------------------------------------------------- */

/*static inline*/ int
csv_cols(const csv_t *self)
{
  return self->csv_labtab->cr_cols;
}

/* ------------------------------------------------------------------------- *
 * csv_rows
 * ------------------------------------------------------------------------- */

/*static inline*/ int
csv_rows(const csv_t *self)
{
  return self->csv_rowcnt;
}

/* ------------------------------------------------------------------------- *
 * csv_colcheck
 * ------------------------------------------------------------------------- */

/*static inline*/ int
csv_colcheck(const csv_t *self, int col)
{
  return 0 <= col && col < csv_cols(self);
}

/* ------------------------------------------------------------------------- *
 * csv_rowcheck
 * ------------------------------------------------------------------------- */

/*static inline*/ int
csv_rowcheck(const csv_t *self, int row)
{
  return 0 <= row && row < csv_rows(self);
}

/* ------------------------------------------------------------------------- *
 * csv_getcolflags
 * ------------------------------------------------------------------------- */

/*static inline*/ unsigned
csv_getcolflags(const csv_t *self, int col)
{
  if( csv_colcheck(self, col) )
  {
    return self->csv_colflags[col];
  }
  return 0;
}

/* ------------------------------------------------------------------------- *
 * csv_tstcolflags
 * ------------------------------------------------------------------------- */

/*static inline*/ unsigned
csv_tstcolflags(const csv_t *self, int col, unsigned mask)
{
  if( csv_colcheck(self, col) )
  {
    return self->csv_colflags[col] & mask;
  }
  return 0;
}

/* ------------------------------------------------------------------------- *
 * csv_setcolflags
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csv_setcolflags(const csv_t *self, int col, unsigned mask)
{
  if( csv_colcheck(self, col) )
  {
    self->csv_colflags[col] = mask;
  }
}

/* ------------------------------------------------------------------------- *
 * csv_addcolflags
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csv_addcolflags(const csv_t *self, int col, unsigned mask)
{
  if( csv_colcheck(self, col) )
  {
    self->csv_colflags[col] |= mask;
  }
}

/* ------------------------------------------------------------------------- *
 * csv_clrcolflags
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csv_clrcolflags(const csv_t *self, int col, unsigned mask)
{
  if( csv_colcheck(self, col) )
  {
    self->csv_colflags[col] &= ~mask;
  }
}

/* ------------------------------------------------------------------------- *
 * csv_getrowflags
 * ------------------------------------------------------------------------- */

/*static inline*/ unsigned
csv_getrowflags(const csv_t *self, int row)
{
  if( csv_rowcheck(self, row) )
  {
    return self->csv_rowtab[row]->cr_flags;
  }
  return 0;
}

/* ------------------------------------------------------------------------- *
 * csv_setrowflags
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csv_setrowflags(csv_t *self, int row, unsigned mask)
{
  if( csv_rowcheck(self, row) )
  {
    self->csv_rowtab[row]->cr_flags = mask;
  }
}

/* ------------------------------------------------------------------------- *
 * csv_addrowflags
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csv_addrowflags(csv_t *self, int row, unsigned mask)
{
  if( csv_rowcheck(self, row) )
  {
    self->csv_rowtab[row]->cr_flags |= mask;
  }
}

/* ------------------------------------------------------------------------- *
 * csv_clrrowflags
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csv_clrrowflags(csv_t *self, int row, unsigned mask)
{
  if( csv_rowcheck(self, row) )
  {
    self->csv_rowtab[row]->cr_flags &= ~mask;
  }
}

/* ------------------------------------------------------------------------- *
 * csv_tstrowflags
 * ------------------------------------------------------------------------- */

/*static inline*/ unsigned
csv_tstrowflags(const csv_t *self, int row, unsigned mask)
{
  if( csv_rowcheck(self, row) )
  {
    return self->csv_rowtab[row]->cr_flags & mask;
  }
  return 0;
}

/* ------------------------------------------------------------------------- *
 * csv_getrow
 * ------------------------------------------------------------------------- */

/*static inline*/ csvrow_t *
csv_getrow(const csv_t *self, int row)
{
  if( !csv_rowcheck(self, row) )
  {
    msg_fatal("csv_t: row access out of bounds [%d/%d]\n",
              row, self->csv_rowcnt);
  }
  // get rid of const status
  return (csvrow_t *)self->csv_rowtab[row];
}

/* ------------------------------------------------------------------------- *
 * csv_getcell
 * ------------------------------------------------------------------------- */

/*static inline*/ csvcell_t *
csv_getcell(const csv_t *self, int row, int col)
{
  return csvrow_getcell(csv_getrow(self, row), col);
}

/* ------------------------------------------------------------------------- *
 * csv_isstring
 * ------------------------------------------------------------------------- */

/*static inline*/ int
csv_isstring(const csv_t *self, int row, int col)
{
  return csvcell_isstring(csv_getcell(self, row, col));
}

/* ------------------------------------------------------------------------- *
 * csv_isnumber
 * ------------------------------------------------------------------------- */

/*static inline*/ int
csv_isnumber(const csv_t *self, int row, int col)
{
  return csvcell_isnumber(csv_getcell(self, row, col));
}

/* ------------------------------------------------------------------------- *
 * csv_getnumber
 * ------------------------------------------------------------------------- */

/*static inline*/ double
csv_getnumber(const csv_t *self, int row, int col)
{
  return csvcell_getnumber(csv_getcell(self, row, col));
}

/* ------------------------------------------------------------------------- *
 * csv_getstring
 * ------------------------------------------------------------------------- */

/*static inline*/ const char *
csv_getstring(const csv_t *self, int row, int col, char *buff, size_t size)
{
  return csvcell_getstring(csv_getcell(self, row, col), buff, size);
}

/* ------------------------------------------------------------------------- *
 * csv_setstring
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csv_setstring(csv_t *self, int row, int col, const char *text)
{
  csvcell_setstring(csv_getcell(self, row, col), text);
}

/* ------------------------------------------------------------------------- *
 * csv_setnumber
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csv_setnumber(csv_t *self, int row, int col, double numb)
{
  csvcell_setnumber(csv_getcell(self, row, col), numb);
}

/* ------------------------------------------------------------------------- *
 * csv_setauto
 * ------------------------------------------------------------------------- */

/*static inline*/ void
csv_setauto(csv_t *self, int row, int col, const char *text)
{
  csvcell_setauto(csv_getcell(self, row, col), text);
}

/* ------------------------------------------------------------------------- *
 * csv_setsource
 * ------------------------------------------------------------------------- */

void
csv_setsource(csv_t *self, const char *path)
{
  free(self->csv_source);
  self->csv_source = path ? strdup(path) : NULL;
}

/* ------------------------------------------------------------------------- *
 * csv_getsource
 * ------------------------------------------------------------------------- */

const char *
csv_getsource(const csv_t *self)
{
  return self->csv_source ? self->csv_source : "<unset>";
}

/* ------------------------------------------------------------------------- *
 * csv_ctor
 * ------------------------------------------------------------------------- */

void
csv_ctor(csv_t *self)
{
  array_ctor(&self->csv_head, csvvar_delete_cb);

  self->csv_rowcnt = 0;
  self->csv_rowmax = 256;

  self->csv_labtab = csvrow_create(0);
  self->csv_rowtab = malloc(self->csv_rowmax * sizeof *self->csv_rowtab);

  self->csv_sepstr = 0;
  self->csv_colflags = 0;
  self->csv_source = 0;

  self->csv_flags  = 0;
}

/* ------------------------------------------------------------------------- *
 * csv_dtor
 * ------------------------------------------------------------------------- */

void
csv_dtor(csv_t *self)
{
  array_dtor(&self->csv_head);
  csvrow_delete(self->csv_labtab);
  for( int r = 0; r < self->csv_rowcnt; ++r )
  {
    csvrow_delete(self->csv_rowtab[r]);
  }
  free(self->csv_rowtab);
  free(self->csv_sepstr);
  free(self->csv_colflags);
  free(self->csv_source);
}

/* ------------------------------------------------------------------------- *
 * csv_create
 * ------------------------------------------------------------------------- */

csv_t *
csv_create(void)
{
  csv_t *self = calloc(1, sizeof *self);
  csv_ctor(self);
  return self;
}

/* ------------------------------------------------------------------------- *
 * csv_delete
 * ------------------------------------------------------------------------- */

void
csv_delete(csv_t *self)
{
  if( self != 0 )
  {
    csv_dtor(self);
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * csv_delete_cb
 * ------------------------------------------------------------------------- */

void
csv_delete_cb(void *self)
{
  csv_delete(self);
}

/* ------------------------------------------------------------------------- *
 * csv_addvar
 * ------------------------------------------------------------------------- */

void
csv_addvar(csv_t *self, const char *key, const char *val)
{
  array_add(&self->csv_head, csvvar_create(key, val));
}

/* ------------------------------------------------------------------------- *
 * csv_delrow
 * ------------------------------------------------------------------------- */

void
csv_delrow(csv_t *self, int row)
{
  if( csv_rowcheck(self, row) )
  {
    csvrow_delete(self->csv_rowtab[row]);
    for( ; row < self->csv_rowcnt; ++row )
    {
      self->csv_rowtab[row+0] = self->csv_rowtab[row+1];
    }
    self->csv_rowcnt -= 1;
  }
}

/* ------------------------------------------------------------------------- *
 * csv_delrow_nocompact
 * ------------------------------------------------------------------------- */

void
csv_delrow_nocompact(csv_t *self, int row)
{
  if( csv_rowcheck(self, row) )
  {
    csvrow_delete(self->csv_rowtab[row]);
    self->csv_rowtab[row] = 0;
  }
}

/* ------------------------------------------------------------------------- *
 * csv_compactrows
 * ------------------------------------------------------------------------- */

void
csv_compactrows(csv_t *self)
{
  int di = 0, si = 0;

  while( si < self->csv_rowcnt )
  {
    csvrow_t *r = self->csv_rowtab[si++];
    if( r != 0 )
    {
      self->csv_rowtab[di++] = r;
    }
  }
  self->csv_rowcnt = di;
}

/* ------------------------------------------------------------------------- *
 * csv_newrow
 * ------------------------------------------------------------------------- */

csvrow_t *
csv_newrow(csv_t *self)
{
  if( self->csv_rowcnt == self->csv_rowmax )
  {
    self->csv_rowmax = self->csv_rowmax * 4 / 3;
    self->csv_rowtab = realloc(self->csv_rowtab,
                               self->csv_rowmax * sizeof *self->csv_rowtab);
  }

  csvrow_t *r = csvrow_create(csv_cols(self));
  self->csv_rowtab[self->csv_rowcnt++] = r;
  return r;
}

/* ------------------------------------------------------------------------- *
 * csv_addcol  --  find/add column by label name
 * ------------------------------------------------------------------------- */

int
csv_addcol(csv_t *self, const char *lab)
{
  for( int c=0, n = csv_cols(self); c < n; ++c )
  {
    char t[32];
    const char *s = csvrow_getstring(self->csv_labtab, c, t, sizeof t);

    if( !strcmp(s, lab) )
    {
      return c;
    }
  }

  for( int r = 0; r < self->csv_rowcnt; ++r )
  {
    csvrow_addcol(&self->csv_rowtab[r]);
  }

  int c = csvrow_addcol(&self->csv_labtab);
  csvrow_setstring(self->csv_labtab, c, lab);

  self->csv_colflags = realloc(self->csv_colflags,
                               csv_cols(self) * sizeof *self->csv_colflags);
  self->csv_colflags[c] = 0;

  return c;
}

/* ------------------------------------------------------------------------- *
 * csv_remcol
 * ------------------------------------------------------------------------- */

void
csv_remcol(csv_t *self, int col)
{
  if( csv_colcheck(self, col) )
  {
    csvrow_remcol(&self->csv_labtab, col);
    for( int r = 0; r < self->csv_rowcnt; ++r )
    {
      csvrow_remcol(&self->csv_rowtab[r], col);
    }
  }
}

/* ------------------------------------------------------------------------- *
 * csv_getcol  --  find column by label name
 * ------------------------------------------------------------------------- */

int
csv_getcol(csv_t *self, const char *lab)
{
  for( int c=0, n = csv_cols(self); c < n; ++c )
  {
    char t[32];
    const char *s = csvrow_getstring(self->csv_labtab, c, t, sizeof t);

    if( !strcmp(s, lab) )
    {
      return c;
    }
  }

  char *end = 0;
  int   col = strtol(lab,&end,0);

  if( end != lab && *end == 0 && csv_colcheck(self, col) )
  {
    return col;
  }

  return -1;
}

/* ------------------------------------------------------------------------- *
 * csv_index  --  find column by label name, abort if not existing
 * ------------------------------------------------------------------------- */

int
csv_index(csv_t *self, const char *lab)
{
  int c = csv_getcol(self, lab);
  if( c == -1 )
  {
    msg_fatal("csv_t: access to nonexisting column '%s'\n", lab);
  }
  return c;
}

/* ------------------------------------------------------------------------- *
 * csv_label  --  column name lookup, abort if not existing
 * ------------------------------------------------------------------------- */

const char *
csv_label(const csv_t *self, int col)
{
  if( col < 0 || col >= csv_cols(self) )
  {
    msg_fatal("csv_t: column access out of boulds [%d / %d]\n",
              col, csv_cols(self));
  }
  return csvrow_getstring(self->csv_labtab, col, 0,0);
}

/* ------------------------------------------------------------------------- *
 * csv_load
 * ------------------------------------------------------------------------- */

int
csv_load(csv_t *self, const char *path)
{
  /* - - - - - - - - - - - - - - - - - - - *
   * variables
   * - - - - - - - - - - - - - - - - - - - */

  int     error = -1;
  size_t  size  = 0x1000;
  char   *data  = malloc(size);
  FILE   *file  = 0;
  FILE   *fnew  = 0;
  int     sep   = ',';
  int     cols  = 1;
  char  **col   = 0;
  int    *idx   = 0;
  int     cnt;

  /* - - - - - - - - - - - - - - - - - - - *
   * fetch line of input
   * - - - - - - - - - - - - - - - - - - - */

  auto int input(void)
  {
    for( ;; )
    {
      int n = getline(&data,&size,file);
      if( n <= 0 )
      {
        return 1;
      }

      if( *data == '#' ) continue;

      if( n > 0 && data[n-1] == '\n' ) data[--n] = 0;
      if( n > 0 && data[n-1] == '\r' ) data[--n] = 0;

      //printf(">>%s<<\n", data);
      return 0;
    }
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * slice line of input
   * - - - - - - - - - - - - - - - - - - - */

  auto void slice(void)
  {
    char *pos = data;
    cnt = 0;
    col[cnt++] = pos;

    for( ;; )
    {
      switch( *pos )
      {
      case '\t': case ',': case ';':
        if( *pos == sep )
        {
          *pos++ = 0, col[cnt] = pos;
          if( cnt++ == cols )
          {
            goto at_eol;
          }
          break;
        }
      default:
        ++pos;
        break;

      case 0x00: case '\r': case '\n':
        goto at_eol;
      }
    }
    at_eol:
    ;
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * open file
   * - - - - - - - - - - - - - - - - - - - */

  if( !path || !strcmp(path, "-") )
  {
    file = stdin;
    path = "<stdin>";
  }
  else if( (file = fnew = fopen(path, "r")) == 0 )
  {
    perror(path); goto cleanup;
  }

  csv_setsource(self, path);

  /* - - - - - - - - - - - - - - - - - - - *
   * parse header
   * - - - - - - - - - - - - - - - - - - - */

  for( ;; )
  {
    if( input() )
    {
      msg_warning("%s: EOF while reading CSV header\n", path);
      goto at_eof;
    }

    if( *data == 0 )
    {
      if( input() )
      {
        msg_warning("%s: EOF after reading CSV header\n", path);
        goto at_eof;
      }
      break;
    }

    char *value = strchr(data, '=');

    if( value == 0 )
    {
      msg_warning("%s: missing CSV header terminator\n", path);
      break;
    }

    *value++ = 0;
    csv_addvar(self, cstring_strip(data), cstring_strip(value));
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * separator autodetect
   * - - - - - - - - - - - - - - - - - - - */

  if( *data == 0 )
  {
    msg_warning("%s: empty CSV label row\n", path);
    goto at_eof;
  }

  {
    int sem = 1;
    int tab = 1;
    for( const char *s = data; *s; ++s )
    {
      switch( *s )
      {
      case ',': ++cols;  break;
      case ';': ++sem;  break;
      case '\t': ++tab; break;
      }
    }
    if( cols < sem ) cols = sem, sep = ';';
    if( cols < tab ) cols = tab, sep = '\t';
  }

  col = calloc(cols+1, sizeof *col);
  idx = calloc(cols+0, sizeof *idx);

  /* - - - - - - - - - - - - - - - - - - - *
   * handle label row
   * - - - - - - - - - - - - - - - - - - - */

  slice();

  for( int i = 0; i < cnt; ++i )
  {
    idx[i] = csv_addcol(self, col[i]);
    //printf("[%3d] '%s'\n", idx[i], col[i]);
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * handle data rows
   * - - - - - - - - - - - - - - - - - - - */

  for( ;; )
  {
    if( input() )
    {
      msg_warning("%s: missing CSV table terminator\n", path);
      goto at_eof;
    }

    if( *data == 0 )
    {
      break;
    }

    slice();

    csvrow_t *row = csv_newrow(self);

    for( int i = 0; i < cnt; ++i )
    {
      double val;
      const char *pos = col[i];

      switch( *pos )
      {
      case '+':  case '-':  case '.':  case '0' ... '9':
        val = csv_float_parse(&pos);
        if( *pos == 0 )
        {
          csvrow_setnumber(row, idx[i], val);
          break;
        }
        // fall through

      default:
        csvrow_setstring(row, idx[i], col[i]);
        break;
      }

      //cellcnt += 1;
      //numeric += csvrow_isnumber(row, idx[i]);
    }

    if( cnt != cols )
    {
      msg_warning("%s: too %s columns\n", path,
                  (cnt<cols) ? "few" : "much");
    }
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * file parsed
   * - - - - - - - - - - - - - - - - - - - */

  at_eof:

  error = 0;

  /* - - - - - - - - - - - - - - - - - - - *
   * cleanup
   * - - - - - - - - - - - - - - - - - - - */

  cleanup:

  if( fnew != 0 ) fclose(fnew);

  free(data);
  free(col);
  free(idx);

  return error;
}

/* ------------------------------------------------------------------------- *
 * csv_save
 * ------------------------------------------------------------------------- */

int
csv_save(csv_t *self, const char *path)
{
  enum { SIZE = 0x8000 };

  int     error = -1;
  FILE   *file  = NULL;
  FILE   *newf  = NULL;

  char   *data  = malloc(SIZE);
  int     used  = 0;

  auto void flush(void)
  {
    fwrite(data, 1, used, file);
    used = 0;
  }

  auto void emit(const char *str)
  {
    while( *str )
    {
      if( used == SIZE ) flush();
      data[used++] = *str++;
    }
  }

// QUARANTINE   auto void emitf(const char *fmt, ...)
// QUARANTINE   {
// QUARANTINE     char tmp[1024];
// QUARANTINE     va_list va;
// QUARANTINE     va_start(va, fmt);
// QUARANTINE     vsnprintf(tmp, sizeof tmp, fmt, va);
// QUARANTINE     va_end(va);
// QUARANTINE     emit(tmp);
// QUARANTINE   }

  /* - - - - - - - - - - - - - - - - - - - *
   * open output
   * - - - - - - - - - - - - - - - - - - - */

  if( path == NULL || !strcmp(path, "-") )
  {
    file = stdout;
    path = "<stdout>";
  }
  else
  {
    file = newf = fopen(path, "w");
  }

  csv_setsource(self, path);

  if( file == NULL )
  {
    perror(path); goto cleanup;
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * output header
   * - - - - - - - - - - - - - - - - - - - */

  if( !(self->csv_flags & CTF_NO_HEADER) )
  {
    for( int i=0, n=array_size(&self->csv_head); i < n; ++i )
    {
      csvvar_t *var = array_get(&self->csv_head, i);
      emit(var->cv_key);
      emit("=");
      emit(var->cv_val);
      emit("\n");
    }
    emit("\n");
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * output table labels
   * - - - - - - - - - - - - - - - - - - - */

  if( !(self->csv_flags & CTF_NO_LABELS) )
  {
    csvrow_t *row = self->csv_labtab;

    for( int c=0, n=csv_cols(self); c < n; ++c )
    {
      csvcell_t *cell = csvrow_getcell(row, c);

      if( c != 0 )
      {
        emit(",");
      }
      if( cell->cc_string )
      {
        emit(cell->cc_string);
      }
      else
      {
        char t[32];
        emit(csv_float_to_string(cell->cc_number, t, sizeof t));
      }
    }
    emit("\n");
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * output table body
   * - - - - - - - - - - - - - - - - - - - */

  for( int r = 0; r < self->csv_rowcnt; ++r )
  {
    const csvrow_t *row = self->csv_rowtab[r];

    for( int c = 0; c < row->cr_cols; ++c )
    {
      csvcell_t *cell = csvrow_getcell(row, c);

      if( c != 0 )
      {
        emit(",");
      }
      if( cell->cc_string )
      {
        emit(cell->cc_string);
      }
      else
      {
        char t[32];
        emit(csv_float_to_string(cell->cc_number, t, sizeof t));
      }
    }
    emit("\n");
  }

  if( !(self->csv_flags & CTF_NO_TERMINATOR) )
  {
    emit("\n");
  }

  flush();

  if( ferror(file) )
  {
    perror(path); goto cleanup;
  }

  error = 0;

  /* - - - - - - - - - - - - - - - - - - - *
   * cleanup
   * - - - - - - - - - - - - - - - - - - - */

  cleanup:

  if( newf != NULL )
  {
    fclose(newf);
  }
  free(data);

  return error;
}

/* ------------------------------------------------------------------------- *
 * csv_load
 * ------------------------------------------------------------------------- */

// QUARANTINE static int count(const char *s, int c)
// QUARANTINE {
// QUARANTINE   int n = 0;
// QUARANTINE   while( *s )
// QUARANTINE   {
// QUARANTINE     if( *s++ == c ) ++n;
// QUARANTINE   }
// QUARANTINE   return n;
// QUARANTINE }
// QUARANTINE
// QUARANTINE static int slice(char ***pv, int *pn, char *s, int c)
// QUARANTINE {
// QUARANTINE   int i = 0;
// QUARANTINE   int n = *pn;
// QUARANTINE   char **v = *pv;
// QUARANTINE
// QUARANTINE   for( ;; )
// QUARANTINE   {
// QUARANTINE     if( i == n )
// QUARANTINE     {
// QUARANTINE       v = realloc(v, ++n * sizeof *v);
// QUARANTINE     }
// QUARANTINE     v[i++] = s;
// QUARANTINE
// QUARANTINE     if( (s = strchr(s,c)) == 0 )
// QUARANTINE     {
// QUARANTINE       break;
// QUARANTINE     }
// QUARANTINE     *s++ = 0;
// QUARANTINE   }
// QUARANTINE   *pv = v;
// QUARANTINE   *pn = n;
// QUARANTINE
// QUARANTINE   return i;
// QUARANTINE }
// QUARANTINE
// QUARANTINE int
// QUARANTINE csv_load(csv_t *self, const char *path)
// QUARANTINE {
// QUARANTINE   int     error = -1;
// QUARANTINE   FILE   *file  = NULL;
// QUARANTINE   FILE   *newf  = NULL;
// QUARANTINE
// QUARANTINE   char   *buff  = NULL;
// QUARANTINE   size_t  size  = 0;
// QUARANTINE   char   *curr  = NULL;
// QUARANTINE   int     line  = 0;
// QUARANTINE
// QUARANTINE   int    sepchr = ',';
// QUARANTINE   char **labstr = 0;
// QUARANTINE   int    labcnt = 0;
// QUARANTINE
// QUARANTINE   char **colstr = 0;
// QUARANTINE   int    colcnt = 0;
// QUARANTINE   int   *index  = 0;
// QUARANTINE
// QUARANTINE   int hdrrows = 0;
// QUARANTINE   int labrows = 0;
// QUARANTINE   int dtarows = 0;
// QUARANTINE
// QUARANTINE   /* - - - - - - - - - - - - - - - - - - - *
// QUARANTINE    * input helper function
// QUARANTINE    * - - - - - - - - - - - - - - - - - - - */
// QUARANTINE
// QUARANTINE   auto int next(void);
// QUARANTINE
// QUARANTINE   auto int next(void)
// QUARANTINE   {
// QUARANTINE     if( curr == 0 )
// QUARANTINE     {
// QUARANTINE       do
// QUARANTINE       {
// QUARANTINE   if( getline(&buff,&size,file) == -1 )
// QUARANTINE   {
// QUARANTINE     return 0;
// QUARANTINE   }
// QUARANTINE   line += 1;
// QUARANTINE   curr = buff;
// QUARANTINE   curr[strcspn(curr,"\r\n")] = 0;
// QUARANTINE       } while( *curr == '#' );
// QUARANTINE     }
// QUARANTINE     return 1;
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   /* - - - - - - - - - - - - - - - - - - - *
// QUARANTINE    * open input
// QUARANTINE    * - - - - - - - - - - - - - - - - - - - */
// QUARANTINE
// QUARANTINE   if( path == NULL || !strcmp(path, "-") )
// QUARANTINE   {
// QUARANTINE     file = stdin;
// QUARANTINE     path = "<stdin>";
// QUARANTINE   }
// QUARANTINE   else
// QUARANTINE   {
// QUARANTINE     file = newf = fopen(path, "r");
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   csv_setsource(self, path);
// QUARANTINE
// QUARANTINE   if( file == NULL )
// QUARANTINE   {
// QUARANTINE     perror(path); goto cleanup;
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   /* - - - - - - - - - - - - - - - - - - - *
// QUARANTINE    * parse header
// QUARANTINE    * - - - - - - - - - - - - - - - - - - - */
// QUARANTINE
// QUARANTINE   for( ; ; curr = 0 )
// QUARANTINE   {
// QUARANTINE     if( ! next() )
// QUARANTINE     {
// QUARANTINE       msg_warning("%s:%d: unexpected EOF while reading %s\n", path, line,
// QUARANTINE         "header");
// QUARANTINE       goto bailout;
// QUARANTINE     }
// QUARANTINE
// QUARANTINE     if( *curr == 0 )
// QUARANTINE     {
// QUARANTINE       curr = 0;
// QUARANTINE       if( hdrrows == 0 )
// QUARANTINE       {
// QUARANTINE   msg_warning("%s:%d: %s header?\n", path, line, "empty");
// QUARANTINE       }
// QUARANTINE       break;
// QUARANTINE     }
// QUARANTINE
// QUARANTINE     char *val = strchr(curr, '=');
// QUARANTINE
// QUARANTINE     if( val == NULL )
// QUARANTINE     {
// QUARANTINE       msg_warning("%s:%d: %s header?\n", path, line,
// QUARANTINE         hdrrows ? "broken" : "missing");
// QUARANTINE       break;
// QUARANTINE     }
// QUARANTINE
// QUARANTINE     *val++ = 0;
// QUARANTINE     csv_addvar(self, cstring_strip(curr), cstring_strip(val));
// QUARANTINE     hdrrows += 1;
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   /* - - - - - - - - - - - - - - - - - - - *
// QUARANTINE    * parse table labels
// QUARANTINE    * - - - - - - - - - - - - - - - - - - - */
// QUARANTINE
// QUARANTINE   if( !next() )
// QUARANTINE   {
// QUARANTINE     msg_warning("%s:%d: unexpected EOF while reading %s\n", path, line,
// QUARANTINE       "label row");
// QUARANTINE     goto bailout;
// QUARANTINE   }
// QUARANTINE   else
// QUARANTINE   {
// QUARANTINE     int m = 0;
// QUARANTINE     for( const char *septry = ",;\t"; *septry; septry++ )
// QUARANTINE     {
// QUARANTINE       int c = *septry;
// QUARANTINE       int n = count(curr, c);
// QUARANTINE       if( m < n )
// QUARANTINE       {
// QUARANTINE   m = n, sepchr = c;
// QUARANTINE       }
// QUARANTINE     }
// QUARANTINE
// QUARANTINE     slice(&labstr, &labcnt, curr, sepchr);
// QUARANTINE     index = malloc(labcnt * sizeof *index);
// QUARANTINE
// QUARANTINE     for( int c = 0; c < labcnt; ++c )
// QUARANTINE     {
// QUARANTINE       index[c] = csv_addcol(self, labstr[c]);
// QUARANTINE     }
// QUARANTINE     curr = 0;
// QUARANTINE     labrows += 1;
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   /* - - - - - - - - - - - - - - - - - - - *
// QUARANTINE    * parse table body
// QUARANTINE    * - - - - - - - - - - - - - - - - - - - */
// QUARANTINE
// QUARANTINE   for( ; ; curr = 0 )
// QUARANTINE   {
// QUARANTINE     if( ! next() )
// QUARANTINE     {
// QUARANTINE       if( dtarows == 0 )
// QUARANTINE       {
// QUARANTINE   msg_warning("%s:%d: unexpected EOF while reading %s\n", path, line,
// QUARANTINE           "data rows");
// QUARANTINE       }
// QUARANTINE       else
// QUARANTINE       {
// QUARANTINE   msg_warning("%s:%d: missing end of data separator line?\n", path, line);
// QUARANTINE       }
// QUARANTINE       goto bailout;
// QUARANTINE     }
// QUARANTINE
// QUARANTINE     if( *curr == 0 )
// QUARANTINE     {
// QUARANTINE       if( dtarows == 0 )
// QUARANTINE       {
// QUARANTINE   msg_warning("%s:%d: empty table body?\n", path, line);
// QUARANTINE       }
// QUARANTINE       curr = 0;
// QUARANTINE       break;
// QUARANTINE     }
// QUARANTINE
// QUARANTINE     int n = slice(&colstr, &colcnt, curr, sepchr);
// QUARANTINE     int m = (n < labcnt) ? n : labcnt;
// QUARANTINE
// QUARANTINE     if( n != labcnt )
// QUARANTINE     {
// QUARANTINE       msg_warning("%s:%d: found %d cols instead of %d\n",
// QUARANTINE         path, line, n, labcnt);
// QUARANTINE     }
// QUARANTINE
// QUARANTINE     csvrow_t *r = csv_newrow(self);
// QUARANTINE
// QUARANTINE     for( int c = 0; c < m; ++c )
// QUARANTINE     {
// QUARANTINE       csvrow_setauto(r, index[c], colstr[c]);
// QUARANTINE     }
// QUARANTINE     dtarows += 1;
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   /* - - - - - - - - - - - - - - - - - - - *
// QUARANTINE    * cleanup
// QUARANTINE    * - - - - - - - - - - - - - - - - - - - */
// QUARANTINE
// QUARANTINE   bailout:
// QUARANTINE   error = 0;
// QUARANTINE
// QUARANTINE   cleanup:
// QUARANTINE
// QUARANTINE   if( newf != NULL )
// QUARANTINE   {
// QUARANTINE     fclose(newf);
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   free(index);
// QUARANTINE   free(labstr);
// QUARANTINE   free(colstr);
// QUARANTINE   free(buff);
// QUARANTINE
// QUARANTINE   return error;
// QUARANTINE }
// QUARANTINE

/* ------------------------------------------------------------------------- *
 * csv_save
 * ------------------------------------------------------------------------- */

// QUARANTINE int
// QUARANTINE csv_save(csv_t *self, const char *path)
// QUARANTINE {
// QUARANTINE   int     error = -1;
// QUARANTINE   FILE   *file  = NULL;
// QUARANTINE   FILE   *newf  = NULL;
// QUARANTINE
// QUARANTINE   /* - - - - - - - - - - - - - - - - - - - *
// QUARANTINE    * open output
// QUARANTINE    * - - - - - - - - - - - - - - - - - - - */
// QUARANTINE
// QUARANTINE   if( path == NULL || !strcmp(path, "-") )
// QUARANTINE   {
// QUARANTINE     file = stdout;
// QUARANTINE     path = "<stdout>";
// QUARANTINE   }
// QUARANTINE   else
// QUARANTINE   {
// QUARANTINE     file = newf = fopen(path, "w");
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   csv_setsource(self, path);
// QUARANTINE
// QUARANTINE   if( file == NULL )
// QUARANTINE   {
// QUARANTINE     perror(path); goto cleanup;
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   /* - - - - - - - - - - - - - - - - - - - *
// QUARANTINE    * output header
// QUARANTINE    * - - - - - - - - - - - - - - - - - - - */
// QUARANTINE
// QUARANTINE
// QUARANTINE   if( !(self->csv_flags & CTF_NO_HEADER) )
// QUARANTINE   {
// QUARANTINE     for( int i=0, n=array_size(&self->csv_head); i < n; ++i )
// QUARANTINE     {
// QUARANTINE       csvvar_t *var = array_get(&self->csv_head, i);
// QUARANTINE       fprintf(file, "%s=%s\n", var->cv_key, var->cv_val);
// QUARANTINE     }
// QUARANTINE     fprintf(file, "\n");
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   /* - - - - - - - - - - - - - - - - - - - *
// QUARANTINE    * output table labels
// QUARANTINE    * - - - - - - - - - - - - - - - - - - - */
// QUARANTINE
// QUARANTINE
// QUARANTINE   if( !(self->csv_flags & CTF_NO_LABELS) )
// QUARANTINE   {
// QUARANTINE     for( int i=0, n=csv_cols(self); i < n; ++i )
// QUARANTINE     {
// QUARANTINE       char t[32];
// QUARANTINE       const char *s = csvrow_getstring(self->csv_labtab, i, t, sizeof t);
// QUARANTINE       fprintf(file, "%s%s", i ? "," : "", s);
// QUARANTINE     }
// QUARANTINE     fprintf(file, "\n");
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   /* - - - - - - - - - - - - - - - - - - - *
// QUARANTINE    * output table body
// QUARANTINE    * - - - - - - - - - - - - - - - - - - - */
// QUARANTINE
// QUARANTINE   for( int r = 0; r < self->csv_rowcnt; ++r )
// QUARANTINE   {
// QUARANTINE     const csvrow_t *row = self->csv_rowtab[r];
// QUARANTINE
// QUARANTINE     for( int c = 0; c < row->cr_cols; ++c )
// QUARANTINE     {
// QUARANTINE       char t[32];
// QUARANTINE       const char *s = csvrow_getstring(row, c, t, sizeof t);
// QUARANTINE       fprintf(file, "%s%s", c ? "," : "", s);
// QUARANTINE     }
// QUARANTINE     fprintf(file, "\n");
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   if( !(self->csv_flags & CTF_NO_TERMINATOR) )
// QUARANTINE   {
// QUARANTINE     fprintf(file, "\n");
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   if( ferror(file) )
// QUARANTINE   {
// QUARANTINE     perror(path); goto cleanup;
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   error = 0;
// QUARANTINE
// QUARANTINE   /* - - - - - - - - - - - - - - - - - - - *
// QUARANTINE    * cleanup
// QUARANTINE    * - - - - - - - - - - - - - - - - - - - */
// QUARANTINE
// QUARANTINE   cleanup:
// QUARANTINE
// QUARANTINE   if( newf != NULL )
// QUARANTINE   {
// QUARANTINE     fclose(newf);
// QUARANTINE   }
// QUARANTINE
// QUARANTINE   return error;
// QUARANTINE }
// QUARANTINE
/* ------------------------------------------------------------------------- *
 * csv_save_as_html
 * ------------------------------------------------------------------------- */

int
csv_save_as_html(csv_t *self, const char *path)
{
  int     error = -1;
  FILE   *file  = NULL;
  FILE   *newf  = NULL;

  /* - - - - - - - - - - - - - - - - - - - *
   * emit functions
   * - - - - - - - - - - - - - - - - - - - */

  auto void escape(const char *text)
  {
    while( text && *text )
    {
      int c = (unsigned char)*text++;
      switch( c )
      {
      case '<': fputs("&lt;",  file); break;
      case '>': fputs("&gt;",  file); break;
      case '&': fputs("&gamp", file); break;
      default: putc(c, file); break;
      }
    }
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * open output
   * - - - - - - - - - - - - - - - - - - - */

  if( path == NULL || !strcmp(path, "-") )
  {
    file = stdout;
    path = "<stdout>";
  }
  else
  {
    file = newf = fopen(path, "w");
  }

  //csv_setsource(self, path);

  if( file == NULL )
  {
    perror(path); goto cleanup;
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * output header
   * - - - - - - - - - - - - - - - - - - - */

  fprintf(file, "<html>\n<head><title>");
  escape(self->csv_source ?
         cstring_basename(self->csv_source) :
         "<unnamed csv data>");
  fprintf(file, "</title></head>\n<body>\n");

  if( !(self->csv_flags & CTF_NO_HEADER) )
  {
    int n = array_size(&self->csv_head);

    if( n > 0 )
    {
      fprintf(file, "<table border=1>\n");
      for( int i=0; i < n; ++i )
      {
        csvvar_t *var = array_get(&self->csv_head, i);
        fprintf(file, "<tr><th>");
        escape(var->cv_key);
        fprintf(file, "<td>");
        escape(var->cv_val);
        fprintf(file, "\n");
      }
      fprintf(file, "</table>\n<hr>\n");
    }
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * output table labels
   * - - - - - - - - - - - - - - - - - - - */

  fprintf(file, "<table border=1>\n");

  if( !(self->csv_flags & CTF_NO_LABELS) )
  {
    const csvrow_t *row = self->csv_labtab;
    fprintf(file, "<tr>\n");
    for( int c = 0; c < row->cr_cols; ++c )
    {
      const csvcell_t *cell = csvrow_getcell(row, c);
      if( csvcell_isnumber(cell) )
      {
        fprintf(file, "<th align=right>"CELL_FMT"\n", csvcell_getnumber(cell));
      }
      else
      {
        fprintf(file, "<th>");
        escape(csvcell_getstring(cell,0,0));
        fprintf(file, "\n");
      }
    }
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * output table body
   * - - - - - - - - - - - - - - - - - - - */

  for( int r = 0; r < self->csv_rowcnt; ++r )
  {
    const csvrow_t *row = self->csv_rowtab[r];

    fprintf(file, "<tr>\n");
    for( int c = 0; c < row->cr_cols; ++c )
    {
      const csvcell_t *cell = csvrow_getcell(row, c);
      if( csvcell_isnumber(cell) )
      {
        fprintf(file, "<td align=right>"CELL_FMT"\n", csvcell_getnumber(cell));
      }
      else
      {
        fprintf(file, "<td>");
        escape(csvcell_getstring(cell,0,0));
        fprintf(file, "\n");
      }
    }
  }

  fprintf(file, "</table>\n</body>\n</html>");

  if( ferror(file) )
  {
    perror(path); goto cleanup;
  }

  error = 0;

  /* - - - - - - - - - - - - - - - - - - - *
   * cleanup
   * - - - - - - - - - - - - - - - - - - - */

  cleanup:

  if( newf != NULL )
  {
    fclose(newf);
  }

  return error;
}

/* ------------------------------------------------------------------------- *
 * csv_sortrows  --  sort rows in table
 * ------------------------------------------------------------------------- */

void
csv_sortrows(csv_t *self)
{
  if( self->csv_rowcnt > 1 )
  {
    qsort(self->csv_rowtab, self->csv_rowcnt, sizeof *self->csv_rowtab,
          csvrow_compare_indirect_cb);
  }
}

/* ------------------------------------------------------------------------- *
 * csv_op_calc  --  evaluate expression row by row
 * ------------------------------------------------------------------------- */

int
csv_op_calc(csv_t *self, const char *expr)
{
  int row = 0;

  void getsym(calc_t *calc, void *user, calctok_t *tok)
  {
    if( tok->tok_col < 0 )
    {
      tok->tok_col = csv_addcol(self, calctok_getsymbol(tok));
    }

    tok->tok_val = *csv_getcell(self, row, tok->tok_col);
  }

  void setsym(calc_t *calc, void *user, calctok_t *tok)
  {
    if( tok->tok_col < 0 )
    {
      tok->tok_col = csv_addcol(self, calctok_getsymbol(tok));
    }
    *csv_getcell(self, row, tok->tok_col) = tok->tok_val;
  }

  int     err  = -1;
  calc_t *calc = 0;

  calc = calc_create();
  calc->calc_getvar = getsym;
  calc->calc_setvar = setsym;

  if( calc_compile(calc, expr) == 0 )
  {
    goto cleanup;
  }

  for( row = 0; row < self->csv_rowcnt; ++row )
  {
    calc_evaluate(calc);
  }

  err = 0;
  cleanup:

  calc_delete(calc);

  return err;
}

/* ------------------------------------------------------------------------- *
 * csv_op_sort
 * ------------------------------------------------------------------------- */

void
csv_op_sort(csv_t *self, const char *labels)
{
  csvord_t *ord = csvord_create(self, labels, 0);
  csvord_apply(ord, self);
  csv_sortrows(self);
  csvord_unapply(ord, self);
  csvord_delete(ord);
}

/* ------------------------------------------------------------------------- *
 * csv_op_uniq
 * ------------------------------------------------------------------------- */

void
csv_op_uniq(csv_t *self, const char *labels)
{
  if( labels != 0 && *labels != 0 )
  {
    csvord_t *ord = csvord_create(self, labels, 1);
    csvord_apply(ord, self);
    csvord_delete(ord);
  }

  csv_sortrows(self);

  int di = 0;
  csvrow_t *prev = 0;

  for( int si = 0; si < self->csv_rowcnt; ++si )
  {
    csvrow_t *curr = self->csv_rowtab[si];

    if( prev != 0 && csvrow_compare(prev, curr) == 0 )
    {
      csvrow_delete(curr);
    }
    else
    {
      self->csv_rowtab[di++] = prev = curr;
    }
  }
  self->csv_rowcnt = di;
}

/* ------------------------------------------------------------------------- *
 * csv_op_usecols
 * ------------------------------------------------------------------------- */

void
csv_op_usecols(csv_t *self, const char *labels)
{
  csvord_t *ord = csvord_create(self, labels, 1);
  csvord_apply(ord, self);
  csvord_delete(ord);
}

/* ------------------------------------------------------------------------- *
 * csv_op_remcols
 * ------------------------------------------------------------------------- */

void
csv_op_remcols(csv_t *self, const char *labels)
{
  char *work = strdup(labels);
  char *lab;
  int   col;

  for( char *pos = work; *pos; )
  {
    if( *(lab = cstring_split_at_char(pos,&pos,',')) != 0 )
    {
      if( (col = csv_getcol(self, lab)) != -1 )
      {
        csv_remcol(self, col);
      }
    }
  }

  free(work);
}

/* ------------------------------------------------------------------------- *
 * csv_op_origin
 * ------------------------------------------------------------------------- */

void
csv_op_origin(csv_t *self, const char *labels)
{
  char *work = strdup(labels);
  char *lab;
  int   col;

  for( char *pos = work; *pos; )
  {
    if( *(lab = cstring_split_at_char(pos,&pos,',')) != 0 )
    {
      if( (col = csv_getcol(self, lab)) != -1 )
      {
        int       cnt = 0;
        csvcell_t val = CSVCELL_ZERO;

        for( int row = 0; row < self->csv_rowcnt; ++row )
        {
          csvcell_t *cell = csv_getcell(self, row, col);

          if( csvcell_isnumber(cell) )
          {
            if( (cnt++ == 0) || (csvcell_compare(&val, cell) > 0) )
            {
              val = *cell;
            }
          }
        }

        for( int row = 0; row < self->csv_rowcnt; ++row )
        {
          csvcell_t *cell = csv_getcell(self, row, col);

          if( csvcell_isnumber(cell) )
          {
            cell->cc_number -= val.cc_number;
          }
        }
      }
    }
  }

  free(work);
}

/* ------------------------------------------------------------------------- *
 * csv_op_order
 * ------------------------------------------------------------------------- */

void
csv_op_order(csv_t *self, const char *labels)
{
  csvord_t *ord = csvord_create(self, labels, 0);
  csvord_apply(ord, self);
  csvord_delete(ord);
}

/* ------------------------------------------------------------------------- *
 * csv_op_reverse
 * ------------------------------------------------------------------------- */

void
csv_op_reverse(csv_t *self)
{
  for( int lo=0, hi=self->csv_rowcnt; lo < --hi; ++lo )
  {
    csvrow_t *a = self->csv_rowtab[lo];
    csvrow_t *b = self->csv_rowtab[hi];
    self->csv_rowtab[lo] = b;
    self->csv_rowtab[hi] = a;
  }
}

/* ------------------------------------------------------------------------- *
 * csv_op_select  --  evaluate expression row by row
 * ------------------------------------------------------------------------- */

int
csv_op_select(csv_t *self, const char *expr)
{
  int row = 0;

  void getsym(calc_t *calc, void *user, calctok_t *tok)
  {
    if( tok->tok_col < 0 )
    {
      tok->tok_col = csv_addcol(self, calctok_getsymbol(tok));
    }

    tok->tok_val = *csv_getcell(self, row, tok->tok_col);
  }

  void setsym(calc_t *calc, void *user, calctok_t *tok)
  {
    if( tok->tok_col < 0 )
    {
      tok->tok_col = csv_addcol(self, calctok_getsymbol(tok));
    }
    *csv_getcell(self, row, tok->tok_col) = tok->tok_val;
  }

  int     err  = -1;
  calc_t *calc = 0;

  calc = calc_create();
  calc->calc_getvar = getsym;
  calc->calc_setvar = setsym;

  if( calc_compile(calc, expr) == 0 )
  {
    goto cleanup;
  }

  int cnt = 0;

  for( row = 0; row < self->csv_rowcnt; ++row )
  {
    if( fabs(calc_evaluate(calc)) < CSV_EPSILON )
    {
      csvrow_delete(self->csv_rowtab[row]);
    }
    else
    {
      self->csv_rowtab[cnt++] = self->csv_rowtab[row];
    }
  }

  self->csv_rowcnt = cnt;

  err = 0;
  cleanup:

  calc_delete(calc);

  return err;
}

/* ------------------------------------------------------------------------- *
 * csv_filter
 * ------------------------------------------------------------------------- */

int
csv_filter(csv_t *self, const char *expression, const char *defop)
{
  int   err  = -1;
  char *work = strdup(expression);
  const char *expr = work;
  const char *oper = defop ? defop : "calc";

  if( *work == ':' )
  {
    oper = cstring_split_at_char(work+1, (char **)&expr, ':');
  }

  if( !strcmp(oper, "calc") )
  {
    csv_op_calc(self, expr);
  }
  else if( !strcmp(oper, "select") )
  {
    csv_op_select(self, expr);
  }
  else if( !strcmp(oper, "sort") )
  {
    csv_op_sort(self, expr);
  }
  else if( !strcmp(oper, "uniq") || !strcmp(oper, "unique") )
  {
    csv_op_uniq(self, expr);
  }
  else if( !strcmp(oper, "usecols") )
  {
    csv_op_usecols(self, expr);
  }
  else if( !strcmp(oper, "order") )
  {
    csv_op_order(self, expr);
  }
  else if( !strcmp(oper, "remcols") )
  {
    csv_op_remcols(self, expr);
  }
  else if( !strcmp(oper, "reverse") )
  {
    csv_op_reverse(self);
  }
  else if( !strcmp(oper, "origin") )
  {
    csv_op_origin(self, expr);
  }
  else if( !strcmp(oper, "header") )
  {
    for( int i = 0; i < self->csv_head.size; ++i )
    {
      csvvar_t *var = array_get(&self->csv_head, i);
      printf("%s=%s\n", var->cv_key, var->cv_val);
    }
    exit(EXIT_SUCCESS);
  }
  else if( !strcmp(oper, "labels") )
  {
    csvrow_t *row = self->csv_labtab;
    for( int c = 0; c < row->cr_cols; ++c )
    {
      char t[32];
      printf("%s\n", csvcell_getstring(&row->cr_celltab[c], t, sizeof t));
    }
    exit(EXIT_SUCCESS);
  }
  else
  {
    msg_fatal("unknown CSV operation '%s:%s'\n", oper, expr);
    goto cleanup;
  }

  {
    char tmp[256];
    int  len = snprintf(tmp, sizeof tmp, ":%s:%s", oper, expr);
    if( len < sizeof tmp )
    {
      csv_addvar(self, "operation", tmp);
    }
    else
    {
      char *tmp = malloc(len + 1);
      snprintf(tmp, len+1, ":%s:%s", oper, expr);
      csv_addvar(self, "opearation", tmp);
      free(tmp);
    }
  }

  err = 0;

  cleanup:

  free(work);

  return err;
}

// XoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoX
// oXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXo
// XoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoX
// oXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXo
// XoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoX
// oXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXo
// XoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoX
// oXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXo
// XoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoX
// oXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXo
// XoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoX
// oXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXo
// XoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoX
// oXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXo
// XoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoX
// oXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXo

/* ========================================================================= *
 * csvord_t  --  methods
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * csvord_create
 * ------------------------------------------------------------------------- */

csvord_t *
csvord_create(csv_t *csv, const char *labels, int remove_rest)
{
  int   cols = csv_cols(csv);
  int  *flag = malloc(cols * sizeof *flag);
  int  *forw = malloc(cols * sizeof *forw);
  int  *back = malloc(cols * sizeof *back);
  char *work = strdup(labels);
  int   indx = 0;
  int   size = 0;

  /* - - - - - - - - - - - - - - - - - - - *
   * init tables
   * - - - - - - - - - - - - - - - - - - - */

  for( int col = 0; col < cols; ++col )
  {
    flag[col] = -1;
    forw[col] = -1;
    back[col] = -1;
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * add requested labels
   * - - - - - - - - - - - - - - - - - - - */

  for( char *pos = work; *pos; )
  {
    char *lab = cstring_split_at_char(pos,&pos,',');
    if( *lab != 0 )
    {
      int col = csv_getcol(csv, lab);
      if( col != -1 && flag[col] == -1 )
      {
        flag[col] = indx++;
      }
    }
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * add rest of cols
   * - - - - - - - - - - - - - - - - - - - */

  if( remove_rest == 0 )
  {
    for( int col = 0; col < cols; ++col )
    {
      if( flag[col] == -1 )
      {
        flag[col] = indx++;
      }
    }
  }

  /* - - - - - - - - - - - - - - - - - - - *
   * assign back & forth indices
   * - - - - - - - - - - - - - - - - - - - */

  for( int col = 0; col < cols; ++col )
  {
    if( (indx = flag[col]) != -1 )
    {
      forw[indx] = col;
      back[size++] = indx;
    }
  }

// QUARANTINE   printf("FLAG:");
// QUARANTINE   for( int col = 0; col < cols; ++col ) printf(" %d", flag[col]);
// QUARANTINE   printf("\n");
// QUARANTINE   printf("FORW:");
// QUARANTINE   for( int col = 0; col < cols; ++col ) printf(" %d", forw[col]);
// QUARANTINE   printf("\n");
// QUARANTINE   printf("BACK:");
// QUARANTINE   for( int col = 0; col < cols; ++col ) printf(" %d", back[col]);
// QUARANTINE   printf("\n");

  /* - - - - - - - - - - - - - - - - - - - *
   * cleanup & create object
   * - - - - - - - - - - - - - - - - - - - */

  free(flag);
  free(work);

  csvord_t *self = calloc(1, sizeof *self);
  self->co_cols = size;
  self->co_forw = forw;
  self->co_back = back;
  return self;
}

/* ------------------------------------------------------------------------- *
 * csvord_delete
 * ------------------------------------------------------------------------- */

void
csvord_delete(csvord_t *self)
{
  if( self != 0 )
  {
    free(self->co_forw);
    free(self->co_back);
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * csvord_apply
 * ------------------------------------------------------------------------- */

void
csvord_apply(csvord_t *self, csv_t *csv)
{
  csvcell_t tmp[self->co_cols];

  auto void dorow(csvrow_t *row)
  {
    for( int c = 0; c < self->co_cols; ++c )
    {
      tmp[c] = row->cr_celltab[self->co_forw[c]];
    }
    for( int c = 0; c < self->co_cols; ++c )
    {
      row->cr_celltab[c] = tmp[c];
    }
    row->cr_cols = self->co_cols;
  }

  dorow(csv->csv_labtab);
  for( int r = 0; r < csv->csv_rowcnt; ++r )
  {
    dorow(csv->csv_rowtab[r]);
  }
}

/* ------------------------------------------------------------------------- *
 * csvord_unapply
 * ------------------------------------------------------------------------- */

void
csvord_unapply(csvord_t *self, csv_t *csv)
{
  csvcell_t tmp[self->co_cols];

  auto void dorow(csvrow_t *row)
  {
    for( int c = 0; c < self->co_cols; ++c )
    {
      tmp[c] = row->cr_celltab[self->co_back[c]];
    }
    for( int c = 0; c < self->co_cols; ++c )
    {
      row->cr_celltab[c] = tmp[c];
    }
    row->cr_cols = self->co_cols;
  }

  dorow(csv->csv_labtab);
  for( int r = 0; r < csv->csv_rowcnt; ++r )
  {
    dorow(csv->csv_rowtab[r]);
  }
}
