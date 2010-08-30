/*
 * This file is part of libsysperf
 *
 * Copyright (C) 2001, 2004-2007 Nokia Corporation. 
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
 * File: csv_calc.h
 * 
 * Author: Simo Piiroinen
 * 
 * -------------------------------------------------------------------------
 * 
 * History:
 * 
 * 14-Sep-2005 Simo Piiroinen
 * - initial version
 * ========================================================================= */

#ifndef CSV_CALC_H_
#define CSV_CALC_H_

#include "csv_table.h"
#include "calculator.h"

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif

typedef struct csv_calc_t csv_calc_t;

struct csv_calc_t
{
  csv_t  *table;
  calc_t *calc;
  int     row;
};

void csv_calc_delete(csv_calc_t *self);
csv_calc_t *csv_calc_create(csv_t *table, const char *expr);
double csv_calc_row_value(csv_calc_t *self, int row);
int csv_calc_row_true(csv_calc_t *self, int row);
void csv_calc_all_rows(csv_calc_t *self);


#ifdef __cplusplus
};
#endif

#endif /* CSV_CALC_H_ */
