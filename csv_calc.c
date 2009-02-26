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
 * File: csv_calc.c
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 25-Sep-2006 Simo Piiroinen
 * - sync with calculator changes
 *
 * 17-May-2006 Simo Piiroinen
 * - csv api updates
 *
 * 14-Sep-2005 Simo Piiroinen
 * - initial version
 * ========================================================================= */

/* ========================================================================= *
 * Include Files
 * ========================================================================= */

#include <stdio.h>
#include <assert.h>

#include "csv_calc.h"

#define EPSILON (1e-9)

/* ------------------------------------------------------------------------- *
 * csv_calc_getsym_fn  --  suitable callback for row operations
 * ------------------------------------------------------------------------- */

static void csv_calc_getsym_fn(calc_t *calc, void *user, calctok_t *tok)
{
  csv_calc_t *self = user;

  if( tok->tok_col < 0 )
  {
    tok->tok_col = csv_addcol(self->table, calctok_getsymbol(tok));
  }
  tok->tok_val = *csv_getcell(self->table, self->row, tok->tok_col);
}

/* ------------------------------------------------------------------------- *
 * csv_calc_setsym_fn  --  suitable callback for row operations
 * ------------------------------------------------------------------------- */

static void csv_calc_setsym_fn(calc_t *calc, void *user, calctok_t *tok)
{
  csv_calc_t *self = user;

  if( tok->tok_col < 0 )
  {
    tok->tok_col = csv_addcol(self->table, calctok_getsymbol(tok));
  }
  *csv_getcell(self->table, self->row, tok->tok_col) = tok->tok_val;
}

/* ------------------------------------------------------------------------- *
 * csv_calc_delete
 * ------------------------------------------------------------------------- */

void csv_calc_delete(csv_calc_t *self)
{
  if( self != 0 )
  {
    calc_delete(self->calc);
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * csv_calc_create
 * ------------------------------------------------------------------------- */

csv_calc_t *csv_calc_create(csv_t *table, const char *expr)
{
  csv_calc_t *self = calloc(1, sizeof *self);

  self->table = table;
  self->row   = 0;
  self->calc  = calc_create();

  self->calc->calc_getvar = csv_calc_getsym_fn;
  self->calc->calc_setvar = csv_calc_setsym_fn;

  if( !calc_compile(self->calc, expr) )
  {
    csv_calc_delete(self);
    self = 0;
  }

  return self;
}

/* ------------------------------------------------------------------------- *
 * csv_calc_row_value
 * ------------------------------------------------------------------------- */

double csv_calc_row_value(csv_calc_t *self, int row)
{
  self->row = row;
  return calc_evaluate(self->calc);
}

/* ------------------------------------------------------------------------- *
 * csv_calc_row_true
 * ------------------------------------------------------------------------- */

int csv_calc_row_true(csv_calc_t *self, int row)
{
  double v = csv_calc_row_value(self, row);
  return (v < -EPSILON) || (v > EPSILON);
}
/* ------------------------------------------------------------------------- *
 * csv_calc_all
 * ------------------------------------------------------------------------- */

void csv_calc_all_rows(csv_calc_t *self)
{
  int rows = csv_rows(self->table);

// QUARANTINE   for( self->row = 0; self->row < self->table->ct_rows; ++self->row )
  for( self->row = 0; self->row < rows; ++self->row )
  {
    calc_evaluate(self->calc);
  }
}
