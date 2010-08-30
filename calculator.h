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
 * File: calculator.h  --  simple infix calculator with external symtab
 * 
 * Author: Simo Piiroinen
 * 
 * -------------------------------------------------------------------------
 * 
 * History:
 * 
 * 25-Sep-2006 Simo Piiroinen
 * - added '?' operator (COL=="")?(COL="XYZ")
 * - short-cut evaluation for '&&' and '||'
 * - uses csvcell_t for holding values
 * - code cleanup
 * 
 * 28-Jun-2005 Simo Piiroinen
 * - imported from track2
 * 
 * 10-Jan-2005 Simo Piiroinen
 * - supports string variables too
 *
 * 26-Aug-2004 Simo Piiroinen
 * - first version
 * ========================================================================= */

#ifndef CALCULATOR_H_
#define CALCULATOR_H_

#include "csv_table.h"

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif


/* ========================================================================= *
 * typedefs & constants
 * ========================================================================= */

typedef struct calcop_t  calcop_t;
typedef struct calctok_t calctok_t;
typedef struct calcstk_t calcstk_t;
typedef struct calc_t    calc_t;

/* ------------------------------------------------------------------------- *
 * calcop_t
 * ------------------------------------------------------------------------- */

struct calcop_t
{
  const char *op_text;  /*   "+",   "-", "<=", ... */
  const char *op_name;  /* "add", "sub", "le", ... */
  int         op_args;  /* 0: value,
                         * 1: unary,
                         * 2: operator
                         */
  int         op_ipri;  /* priority to go INTO opstack */
  int         op_opri;  /* prioruty to get OUT of opstack */
};

/* ------------------------------------------------------------------------- *
 * calcop_t data
 * ------------------------------------------------------------------------- */

enum
{
#define OP(str,name,args,ipri,opri) tc_##name,
#include "calculator.inc"
  tc_count
};

/* ------------------------------------------------------------------------- *
 * calctok_t
 * ------------------------------------------------------------------------- */

struct calctok_t
{
  int        tok_code; /* tc_xxx */
  calctok_t *tok_arg1; /* operator */         
  calctok_t *tok_arg2; /* operator or unary */
  csvcell_t  tok_val;  /* token value */
  csvcell_t  tok_sym;  /* token symbol */
  int        tok_col;  /* column for symbol */
  
};

/* ------------------------------------------------------------------------- *
 * calcstk_t
 * ------------------------------------------------------------------------- */

struct calcstk_t
{
  /* |---|------|-------|
   * 0   head   tail    size
   *
   *     next() push()  new()
   *     peek() pop()   push()
   *            new()
   *            top()
   */

  int stk_head;  /* next slot to read */
  int stk_tail;  /* last slot in use */
  int stk_size;  /* slots allocated */

  int stk_fifo;  /* fifo == 1 allows:
		  *   calcstk_next()
		  *   calcstk_peek()
		  *   calcstk_new()
		  *
		  *   and calcstk_clear() deletes tokens contained
		  *   as implicitly does calcstk_dtor() too...
		  *
		  * fifo == 0 allows:
		  *   calcstk_top()
		  *   calcstk_pop()
		  *   calcstk_push()
		  */

  calctok_t **stk_data;
};

/* ------------------------------------------------------------------------- *
 * calc_t
 * ------------------------------------------------------------------------- */

struct calc_t
{
  /* expression to evaluate */

  char     *calc_expr; /* original text */
  calcstk_t calc_fifo; /* tokenized text: all tokens are kept here in
			* parse order - which allows reconstruction of
			* error messages where offending token can
			* be identified
			*/

  /* parse state: after calc_syntax_tree() vstk should contain
   *              only root token from which to evaluate value
   *              the expression
   */

  calcstk_t calc_ostk; /* operator stack */
  calcstk_t calc_vstk; /* value stack */

  /* symbol lookup: symbol lookup is done only at evaluate state
   *                thus modifying symbol table will yield
   *                different result without expression
   *                recompilation */

  void   (*calc_getvar)(calc_t *self, void *user, calctok_t *tok);
  void   (*calc_setvar)(calc_t *self, void *user, calctok_t *tok);

  void    *calc_userdata; /* data to pass to above hooks */
};


/* ------------------------------------------------------------------------- *
 * api functions
 * ------------------------------------------------------------------------- */

/* calculator.c */
void calc_clear(calc_t *self);
void calc_delete(calc_t *self);
calc_t *calc_create(void);
int calc_compile(calc_t *self, const char *expr);
double calc_evaluate(calc_t *self);
double calc_compile_and_evaluate(calc_t *self, const char *expr);

const char *calctok_getsymbol(const calctok_t *self);


#ifdef __cplusplus
};
#endif

#endif /* CALCULATOR_H_ */
