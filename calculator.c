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
 * File: calculator.c  --  simple infix calculator with external symtab
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 18-Jan-2007 Simo Piiroinen
 * - special case for division by zero
 *
 * 25-Sep-2006 Simo Piiroinen
 * - added c-style '?:' operators: L=(val<10)?"LO":"HI"
 * - added '#' operator (COL=="")#(COL="XYZ")
 * - short-cut evaluation for '&&' and '||'
 * - uses csvcell_t for holding values
 * - code cleanup
 *
 * 31-Aug-2006 Simo Piiroinen
 * - fix: some diagnostics went to stdout instead of stderr
 *
 * 14-Sep-2005 Simo Piiroinen
 * - commented out dead code
 *
 * 14-Jul-2005 Simo Piiroinen
 * - TESTMAIN supports string variables
 *
 * 28-Jun-2005 Simo Piiroinen
 * - imported from track2
 *
 * 10-Jan-2005 Simo Piiroinen
 * - supports string variables too
 * - debug messages now written to stderr
 *
 * 07-Jan-2005 Simo Piiroinen
 * - added '&&' and '||' operators
 * - string values allowed too
 * - string equality check supports 'str' == 'pat[,pat]...' lists
 *
 * 26-Nov-2004 Simo Piiroinen
 * - now allows underscores in symbols
 *
 * 26-Aug-2004 Simo Piiroinen
 * - first version
 * ========================================================================= */

/* ========================================================================= *
 * Include files
 * ========================================================================= */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <setjmp.h>

// QUARANTINE #define VERBOSE 1
// QUARANTINE #define TESTMAIN

#include "calculator.h"

// {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{
// {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{
// {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{
// {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{
// {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{

// }}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
// }}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
// }}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
// }}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
// }}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
// }}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}

/* ========================================================================= *
 * debug utils
 * ========================================================================= */

#if VERBOSE

static int tabout = 0;

static void tab(void)
{
  fprintf(stderr, "%*s", tabout, "");
}

static void emit(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

static void emit(const char *fmt, ...)
{
  char buf[4<<10];
  va_list va;
  va_start(va, fmt);
  vsnprintf(buf, sizeof buf, fmt, va);
  va_end(va);

  char *pos, *end;

  for( pos = buf; pos && *pos; pos = end )
  {
    if( (end = strchr(pos,'\n')) != 0 )
    {
      *end++ = 0;
    }
    fprintf(stderr, "%*s%s\n", tabout, "", pos);
  }
}

#define ENTER do{ \
  emit("@ %s() {\n", __FUNCTION__); \
  tabout+=4;\
  calc_show_state(self);\
  emit("---\n");\
} while(0);

#define LEAVE do{ \
  emit("---\n");\
  calc_show_state(self);\
  tabout-=4;\
  emit("} @ %s()\n", __FUNCTION__); \
} while(0);

#define HERE emit("%d\n", __LINE__);
#else
#define ENTER do{}while(0);
#define LEAVE do{}while(0);
#define HERE  do{}while(0);
#define tab() do{}while(0)
#endif

/* ========================================================================= *
 * module data
 * ========================================================================= */

#define EPSILON (1e-9) // floating point equality comparison

/* ========================================================================= *
 * calcop_t
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * calcop_t data
 * ------------------------------------------------------------------------- */

static const calcop_t opinfo[tc_count] =
{
#define OP(str,name,args,ipri,opri) {str,#name,args,ipri,opri},
#include "calculator.inc"
};

/* ------------------------------------------------------------------------- *
 * calcop_list
 * ------------------------------------------------------------------------- */

#ifdef TESTMAIN
void calcop_list(void)
{
  for( size_t i = 0; i < tc_count; ++i )
  {
    fprintf(stderr, "[%2d] text=%s name=%s, args=%d, ipri=%d, opri=%d\n", i,
           opinfo[i].op_text, opinfo[i].op_name, opinfo[i].op_args,
           opinfo[i].op_ipri, opinfo[i].op_opri);
  }
}
#endif

/* ------------------------------------------------------------------------- *
 * error handling: currently done wial longjmp, and thus is not thread safe
 * ------------------------------------------------------------------------- */

static jmp_buf syntax_error_return;
static calctok_t *syntax_error_token = 0;

/* ========================================================================= *
 * generic functions
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * match  --  return length of key if it matches start of txt
 * ------------------------------------------------------------------------- */

static int match(const char *key, const char *txt)
{
  for( int i = 0; ; ++i )
  {
    if( key[i] == 0 ) return i;
    if( key[i] != txt[i] ) break;
  }
  return 0;
}

/* ------------------------------------------------------------------------- *
 * arrow  --  print arrow of given length
 * ------------------------------------------------------------------------- */

static void arrow(int n)
{
  while( n-- > 0 ) fputc('-', stderr);
  fputc('^', stderr);
  fputc('\n', stderr);
}

/* ========================================================================= *
 * error handling
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * syntax_error
 * ------------------------------------------------------------------------- */

static void syntax_error(calctok_t *tok)
{
  syntax_error_token = tok;
  longjmp(syntax_error_return, 1);
}

/* ========================================================================= *
 * struct calctok_t methods
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * calctok_zero
 * ------------------------------------------------------------------------- */

static inline void calctok_zero(calctok_t *self)
{
  self->tok_code = tc_lit;
  self->tok_arg1 = 0;
  self->tok_arg2 = 0;
  csvcell_setnumber(&self->tok_val, 0.0);
  csvcell_setnumber(&self->tok_sym, 0.0);
  self->tok_col  = -2;
}

/* ------------------------------------------------------------------------- *
 * calctok_create
 * ------------------------------------------------------------------------- */

static calctok_t *calctok_create(void)
{
  calctok_t *self = calloc(1, sizeof *self);
  calctok_zero(self);
  return self;
}

/* ------------------------------------------------------------------------- *
 * calctok_delete
 * ------------------------------------------------------------------------- */

static void calctok_delete(calctok_t *self)
{
  free(self);
}

/* ------------------------------------------------------------------------- *
 * calctok_isnumber
 * ------------------------------------------------------------------------- */

static int calctok_isnumber(const calctok_t *self)
{
  return csvcell_isnumber(&self->tok_val);
}

/* ------------------------------------------------------------------------- *
 * calctok_isstring
 * ------------------------------------------------------------------------- */

static int calctok_isstring(const calctok_t *self)
{
  return csvcell_isstring(&self->tok_val);
}

/* ------------------------------------------------------------------------- *
 * calctok_issymbol
 * ------------------------------------------------------------------------- */

static int calctok_issymbol(const calctok_t *self)
{
  return csvcell_isstring(&self->tok_sym);
}

/* ------------------------------------------------------------------------- *
 * calctok_getnumber
 * ------------------------------------------------------------------------- */

static double calctok_getnumber(const calctok_t *self)
{
  assert( calctok_isnumber(self) );
  return csvcell_getnumber(&self->tok_val);
}

/* ------------------------------------------------------------------------- *
 * calctok_getstring
 * ------------------------------------------------------------------------- */

static const char *calctok_getstring(const calctok_t *self)
{
  assert( calctok_isstring(self) );
  return csvcell_getstring(&self->tok_val,0,0);
}

/* ------------------------------------------------------------------------- *
 * calctok_getsymbol
 * ------------------------------------------------------------------------- */

const char *calctok_getsymbol(const calctok_t *self)
{
  assert( calctok_issymbol(self) );
  return csvcell_getstring(&self->tok_sym,0,0);
}

/* ------------------------------------------------------------------------- *
 * calctok_isvalue
 * ------------------------------------------------------------------------- */

static int calctok_isvalue(const calctok_t *self)
{
  // literal or variable
  return opinfo[self->tok_code].op_args == 0;
}

/* ------------------------------------------------------------------------- *
 * calctok_isunary
 * ------------------------------------------------------------------------- */

static int calctok_isunary(const calctok_t *self)
{
  return opinfo[self->tok_code].op_args == 1;
}

/* ------------------------------------------------------------------------- *
 * calctok_isoperator
 * ------------------------------------------------------------------------- */

static int calctok_isoperator(const calctok_t *self)
{
  //return opinfo[self->tok_code].op_args == 2;
  return opinfo[self->tok_code].op_args >= 2;
}

/* ------------------------------------------------------------------------- *
 * calctok_ipri
 * ------------------------------------------------------------------------- */

static int calctok_ipri(const calctok_t *self)
{
  return opinfo[self->tok_code].op_ipri;
}

/* ------------------------------------------------------------------------- *
 * calctok_opri
 * ------------------------------------------------------------------------- */

static int calctok_opri(const calctok_t *self)
{
  return opinfo[self->tok_code].op_opri;
}

/* ------------------------------------------------------------------------- *
 * calctok_repr
 * ------------------------------------------------------------------------- */

static char *calctok_repr(calctok_t *self, char *buff)
{
  if( self == 0 )
  {
    return strcpy(buff, "NULL");
  }
  switch( self->tok_code )
  {
  case tc_lit:
    if( calctok_isnumber(self) )
    {
      sprintf(buff, "#%g", calctok_getnumber(self));
    }
    else
    {
      sprintf(buff, "'%s'", calctok_getstring(self));
    }
    break;

  case tc_var:
    sprintf(buff, "$%s", calctok_getsymbol(self));
    break;

  default:
    sprintf(buff, "<%s>", opinfo[self->tok_code].op_name);
    break;
  }
  return buff;
}

#ifdef DEAD_CODE
static void calctok_emit(calctok_t *self)
{
  if( self )
  {
    char t[64];
    fprintf(stderr, "%s", calctok_repr(self, t));
    if( self->tok_arg1 || self->tok_arg2 )
    {
      fprintf(stderr, "{");
      calctok_emit(self->tok_arg1);
      fprintf(stderr, ", ");
      calctok_emit(self->tok_arg2);
      fprintf(stderr, "}");
    }
  }
  else
  {
    fprintf(stderr, "NULL");
  }
}

static void calctok_emit2(calctok_t *self, int tabout)
{
  if( self )
  {
    char t[64];
    tab();fprintf(stderr, "%*s%s\n", tabout, "", calctok_repr(self, t));

    if( self->tok_arg1 || self->tok_arg2 )
    {
      calctok_emit2(self->tok_arg1, tabout+8);
      calctok_emit2(self->tok_arg2, tabout+8);
    }
  }
  else
  {
    tab();fprintf(stderr, "%*s%s\n", tabout, "", "NULL");
  }
}
#endif

/* ========================================================================= *
 * struct calcstk_t methods
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * calcstk_empty
 * ------------------------------------------------------------------------- */

static int calcstk_empty(calcstk_t *self)
{
  return self->stk_head == self->stk_tail;
}

/* ------------------------------------------------------------------------- *
 * calcstk_clear
 * ------------------------------------------------------------------------- */

static void calcstk_clear(calcstk_t *self)
{
  if( self->stk_fifo )
  {
    for( int i = 0; i < self->stk_tail; ++i )
    {
      calctok_delete(self->stk_data[i]);
    }
  }
  self->stk_head = 0;
  self->stk_tail = 0;
}

/* ------------------------------------------------------------------------- *
 * calcstk_dtor
 * ------------------------------------------------------------------------- */

static void calcstk_dtor(calcstk_t *self)
{
  calcstk_clear(self);
  free(self->stk_data);
}

/* ------------------------------------------------------------------------- *
 * calcstk_ctor
 * ------------------------------------------------------------------------- */

static void calcstk_ctor(calcstk_t *self, int fifo)
{
  self->stk_head = 0;
  self->stk_tail = 0;
  self->stk_size = 64;
  self->stk_fifo = fifo;
  self->stk_data = malloc(self->stk_size * sizeof *self->stk_data);
}

/* ------------------------------------------------------------------------- *
 * calcstk_next
 * ------------------------------------------------------------------------- */

static calctok_t *calcstk_next(calcstk_t *self)
{
  assert( self->stk_fifo );

  calctok_t *tok = 0;

  if( !calcstk_empty(self) )
  {
    tok = self->stk_data[self->stk_head++];
  }
  return tok;
}

/* ------------------------------------------------------------------------- *
 * calcstk_new
 * ------------------------------------------------------------------------- */

static calctok_t *calcstk_new(calcstk_t *self)
{
  assert( self->stk_fifo );

  if( self->stk_tail == self->stk_size )
  {
    self->stk_size *= 2;
    self->stk_data = realloc(self->stk_data,
                             self->stk_size * sizeof *self->stk_data);
  }

  calctok_t *tok = calctok_create();
  self->stk_data[self->stk_tail++] = tok;
  return tok;
}

/* ------------------------------------------------------------------------- *
 * calcstk_top
 * ------------------------------------------------------------------------- */

static calctok_t *calcstk_top(calcstk_t *self)
{
  assert( !self->stk_fifo );

  calctok_t *tok = 0;

  if( !calcstk_empty(self) )
  {
    tok = self->stk_data[self->stk_tail - 1];
  }
  return tok;
}

/* ------------------------------------------------------------------------- *
 * calcstk_pop
 * ------------------------------------------------------------------------- */

static calctok_t *calcstk_pop(calcstk_t *self)
{
  assert( !self->stk_fifo );

  calctok_t *tok = 0;

  if( !calcstk_empty(self) )
  {
    tok = self->stk_data[--self->stk_tail];
  }
  return tok;
}

/* ------------------------------------------------------------------------- *
 * calcstk_push
 * ------------------------------------------------------------------------- */

static void calcstk_push(calcstk_t *self, calctok_t *tok)
{
  assert( !self->stk_fifo );

  if( self->stk_tail == self->stk_size )
  {
    self->stk_size *= 2;
    self->stk_data = realloc(self->stk_data,
                             self->stk_size * sizeof *self->stk_data);
  }
  self->stk_data[self->stk_tail++] = tok;
}

/* ========================================================================= *
 * struct calc_t methods
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * calc_root
 * ------------------------------------------------------------------------- */

static calctok_t *calc_root(calc_t *self)
{
  return calcstk_top(&self->calc_vstk);
}

/* ------------------------------------------------------------------------- *
 * calc_getenv
 * ------------------------------------------------------------------------- */

static void
calc_getenv(calc_t *self, void *user, calctok_t *tok)
{
  char *s;

  if( (s = getenv(tok->tok_sym.cc_string)) != 0 )
  {
    csvcell_setauto(&tok->tok_val, s);
    tok->tok_code = tc_lit;
  }
  else
  {
    csvcell_setnumber(&tok->tok_val, 0.0);
    tok->tok_code = tc_lit;
  }

  fprintf(stderr, "GETENV '%s' '%s'\n", tok->tok_sym.cc_string, s ? s : "<none>");
}

/* ------------------------------------------------------------------------- *
 * calc_setenv
 * ------------------------------------------------------------------------- */

static void
calc_setenv(calc_t *self, void *user, calctok_t *tok)
{
  char tmp[32];
  const char *val = csvcell_getstring(&tok->tok_val, tmp, sizeof tmp);
  setenv(tok->tok_sym.cc_string, val, 1);

  fprintf(stderr, "SETENV '%s' '%s'\n", tok->tok_sym.cc_string, val);
}

/* ------------------------------------------------------------------------- *
 * calc_get_token  --  next token from parse fifo
 * ------------------------------------------------------------------------- */

static calctok_t *calc_get_token(calc_t *self)
{
  calctok_t *tok = calcstk_next(&self->calc_fifo);
  //if( tok == 0 ) emit("epop: NULL\n");
  return tok;
}

/* ------------------------------------------------------------------------- *
 * calc_add_token  --  add new token to parse fifo
 * ------------------------------------------------------------------------- */

static calctok_t *calc_add_token(calc_t *self)
{
  return calcstk_new(&self->calc_fifo);
}

/* ------------------------------------------------------------------------- *
 * calc_vpush  --  push token to value stack
 * ------------------------------------------------------------------------- */

static void calc_vpush(calc_t *self, calctok_t *tok)
{
  calcstk_push(&self->calc_vstk, tok);
}

/* ------------------------------------------------------------------------- *
 * calc_vpop  --  pop token from value stack
 * ------------------------------------------------------------------------- */

static calctok_t *calc_vpop(calc_t *self)
{
  calctok_t * tok = calcstk_pop(&self->calc_vstk);
  //if( tok == 0 ) emit("vpop: NULL (fatal)\n");
  return tok;
}

/* ------------------------------------------------------------------------- *
 * calc_opush  --  push token to operator stack
 * ------------------------------------------------------------------------- */

static void calc_opush(calc_t *self, calctok_t *tok)
{
  calcstk_push(&self->calc_ostk, tok);
}

/* ------------------------------------------------------------------------- *
 * calc_opop  --  pop token from operator stack
 * ------------------------------------------------------------------------- */

static calctok_t *calc_opop(calc_t *self)
{
  calctok_t * tok = calcstk_pop(&self->calc_ostk);
  //if( tok == 0 ) emit("opop: NULL\n");
  return tok;
}

/* ------------------------------------------------------------------------- *
 * calc_otop  --  get token on top of operator stack
 * ------------------------------------------------------------------------- */

static calctok_t *calc_otop(calc_t *self)
{
  calctok_t * tok = calcstk_top(&self->calc_ostk);
  //if( tok == 0 ) emit("otop: NULL\n");
  return tok;
}

/* ------------------------------------------------------------------------- *
 * calc_show_state  --  debug: display contents of parse stacks
 * ------------------------------------------------------------------------- */

#if VERBOSE
static void calc_show_state(calc_t *self)
{
  char tmp[32];
  tab();fprintf(stderr, "fifo:");
  for( int i = self->calc_fifo.stk_head; i < self->calc_fifo.stk_tail; ++i )
  {
    calctok_t *tok = self->calc_fifo.stk_data[i];
    fprintf(stderr, " %s", calctok_repr(tok, tmp));
  }
  fprintf(stderr, "\n");

  tab();fprintf(stderr, "ostk:");
  for( int i = self->calc_ostk.stk_tail; i-- > 0; )
  {
    calctok_t *tok = self->calc_ostk.stk_data[i];
    fprintf(stderr, " %s", calctok_repr(tok, tmp));
  }
  fprintf(stderr, "\n");

  tab();fprintf(stderr, "vstk:");
  for( int i = self->calc_vstk.stk_tail; i-- > 0; )
  {
    calctok_t *tok = self->calc_vstk.stk_data[i];
    fprintf(stderr, " %s", calctok_repr(tok, tmp));
  }
  fprintf(stderr, "\n");
}
#endif

/* ------------------------------------------------------------------------- *
 * calc_consume_eval  --  handle operator on top of operator stack
 * ------------------------------------------------------------------------- */

static void calc_consume_eval(calc_t *self)
{
  ENTER
  calctok_t *o = calc_opop(self);

// QUARANTINE   tab();fprintf(stderr, "E: ");calctok_emit(o);fprintf(stderr, "\n");

  assert( o != 0 );

  if( o == 0 )
  {
    // SHOULD NEVER HAPPEN: consume empty opstack
    syntax_error(0);
  }

  if( calctok_isunary(o) )
  {
    o->tok_arg2 = calc_vpop(self);
    o->tok_arg1 = 0;
    calc_vpush(self, o);

    if( o->tok_arg2 == 0 )
    {
      syntax_error(o);
    }
  }
  else if( o->tok_code == tc_op2 )
  {

#if 0
    /* - - - - - - - - - - - - - - - - - - - *
     * kludge to handle: e1 ? e2 : e3
     * - - - - - - - - - - - - - - - - - - - */
    calctok_t *s[256]; int n=0;
    calctok_t *t;
    for( ;; )
    {
      assert( o != 0 );
      assert( o->tok_code == tc_op1 || o->tok_code == tc_op2 );

      if( o->tok_code == tc_op1 )
      {
        fprintf(stderr, "POP\n");
        assert( n > 0 );
        t = s[--n];

// QUARANTINE   tab();fprintf(stderr, "T: ");calctok_emit(t);fprintf(stderr, "\n");

        assert( t->tok_arg1 == 0);
        assert( t->tok_arg2 != 0);
        t->tok_arg1 = calc_vpop(self);

        assert( o->tok_arg1 == 0);
        assert( o->tok_arg2 == 0);
        o->tok_arg1 = calc_vpop(self);
        o->tok_arg2 = t;
        calc_vpush(self, o);
        if( n == 0 ){
// QUARANTINE     calctok_emit2(o, 0);
          break;
        }
      }
      else
      {
        fprintf(stderr, "PUSH\n");
        assert( o->tok_arg1 == 0);
        assert( o->tok_arg2 == 0);
        o->tok_arg2 = calc_vpop(self);
        s[n++] = o;
      }
      o = calc_opop(self);
      assert( o != 0 );
    }
#else
    /* - - - - - - - - - - - - - - - - - - - *
     * kludge to handle: e1 ? e2 : e3
     * - - - - - - - - - - - - - - - - - - - */
    calctok_t *s = 0;
    calctok_t *t;
    for( ;; )
    {
      assert( o != 0 );
      assert( o->tok_code == tc_op1 || o->tok_code == tc_op2 );

      if( o->tok_code == tc_op1 )
      {
        // POP
        assert( s != 0 );
        t = s, s = t->tok_arg1, t->tok_arg1 = 0;

        //tab();fprintf(stderr, "T: ");calctok_emit(t);fprintf(stderr, "\n");

        assert( t->tok_arg1 == 0);
        assert( t->tok_arg2 != 0);
        t->tok_arg1 = calc_vpop(self);

        assert( o->tok_arg1 == 0);
        assert( o->tok_arg2 == 0);
        o->tok_arg1 = calc_vpop(self);
        o->tok_arg2 = t;
        calc_vpush(self, o);
        if( s == 0 )
        {
          //calctok_emit2(o, 0);
          break;
        }
      }
      else
      {
        assert( o->tok_arg1 == 0);
        assert( o->tok_arg2 == 0);
        o->tok_arg2 = calc_vpop(self);

        // PUSH
        o->tok_arg1 = s, s = o;
      }
      o = calc_opop(self);
      assert( o != 0 );
    }
#endif
  }
  else if( calctok_isoperator(o) )
  {
    assert( o->tok_arg2 == 0 );
    assert( o->tok_arg1 == 0 );

    o->tok_arg2 = calc_vpop(self);
    o->tok_arg1 = calc_vpop(self);
    calc_vpush(self, o);

    if( o->tok_arg2 == 0 || o->tok_arg1 == 0)
    {
      syntax_error(o);
    }
  }
  else
  {
    // SHOULD NEVER HAPPEN: value in opstack
    syntax_error(o);
  }
  LEAVE
}

/* ------------------------------------------------------------------------- *
 * calc_expect_num  --  next infix token should be a number
 * ------------------------------------------------------------------------- */

static int calc_expect_num(calc_t *self)
{
  int ok = 0;
  ENTER

  calctok_t *t = calc_get_token(self);

  if( t != 0 )
  {
    ok = 1;

    if( calctok_isvalue(t) )
    {
      calc_vpush(self, t);
    }
    else if( calctok_isunary(t) )
    {
      calc_opush(self, t);
      ok = calc_expect_num(self);
    }
    else if( t->tok_code == tc_sub )
    {
      calc_opush(self, t);
      t->tok_code = tc_neg;
      ok = calc_expect_num(self);
    }
    else
    {
      ok = 0;
      syntax_error(t);
      assert(0);
    }
  }
  LEAVE
  return ok;
}

/* ------------------------------------------------------------------------- *
 * calc_expect_op  --  next infix token should be a operator
 * ------------------------------------------------------------------------- */

static int calc_expect_op(calc_t *self)
{
  int ok = 0;
  ENTER

  calctok_t *t = calc_get_token(self);
  calctok_t *o;

  if( t != 0 )
  {
    ok = 1;

    HERE
    if( t->tok_code == tc_ens )
    {
      HERE
      for( ;; )
      {
        if( (o = calc_otop(self)) == 0 )
        {
          syntax_error(t);
        }
        if( o->tok_code == tc_par )
        {
          calc_opop(self);
          break;
        }
        calc_consume_eval(self);
      }

      ok = calc_expect_op(self);

// QUARANTINE       while( (o = calc_otop(self)) != 0 && o->tok_code != tc_par )
// QUARANTINE       {
// QUARANTINE         HERE
// QUARANTINE         calc_consume_eval(self);
// QUARANTINE       }
// QUARANTINE
// QUARANTINE       if( (o = calc_otop(self)) != 0 && o->tok_code == tc_par )
// QUARANTINE       {
// QUARANTINE         calc_opop(self);
// QUARANTINE       }
// QUARANTINE       HERE
// QUARANTINE       ok = calc_expect_op(self);
// QUARANTINE       HERE
    }
    else if( calctok_isoperator(t) )
    {
      HERE
      while( (o = calc_otop(self)) != 0 && calctok_opri(o) >= calctok_ipri(t) )
      {
        HERE
        calc_consume_eval(self);
      }
      calc_opush(self, t);
    }
    else
    {
      ok = 0;
      syntax_error(t);
      assert(0);
    }
  }
  HERE
  LEAVE
  return ok;
}

/* ------------------------------------------------------------------------- *
 * calc_tokenize_expression  --  expression tokenizer
 * ------------------------------------------------------------------------- */

static inline int issymbeg(int c) { return isalpha(c) || (c == '_'); }
static inline int issymbol(int c) { return isalnum(c) || (c == '_'); }

static int calc_tokenize_expression(calc_t *self, const char *text)
{
  int   ok   = 1;
  char *work = strdup(text);
  char *end  = work;
  char *beg  = work;

  calc_clear(self);

  while( *end != 0 )
  {
    beg = end;
    int c = *end++;
    if( isspace(c) )
    {
      continue;
    }

    calctok_t *t = calc_add_token(self);

    if( c == '"' || c == '\'' )
    {
      /* - - - - - - - - - - - - - - - - - - - *
       * literal string in quotes
       * - - - - - - - - - - - - - - - - - - - */

      // skip initial quote char
      beg = end;

      // find end of quote
      if( (end = strchr(end, c)) == 0 )
      {
        ok = 0; break;
      }
      *end++ = 0;

      t->tok_code = tc_lit;
      csvcell_setstring(&t->tok_val, beg);
// QUARANTINE       fprintf(stderr, "TOK <- str '%s'\n", beg);
    }
    else if( isdigit(c) || (c == '.') )
    {
      /* - - - - - - - - - - - - - - - - - - - *
       * literal number
       * - - - - - - - - - - - - - - - - - - - */

      t->tok_code = tc_lit;
      csvcell_setnumber(&t->tok_val, strtod(beg,&end));

// QUARANTINE       fprintf(stderr, "TOK <- num '%.*s' %g\n",
// QUARANTINE         (int)(end-beg), beg, t->tok_val.cc_number);
    }
    else if( issymbeg(c) )
    {
      /* - - - - - - - - - - - - - - - - - - - *
       * symbol name
       * - - - - - - - - - - - - - - - - - - - */

      while( issymbol(*end) ) { ++end; }

      t->tok_code = tc_var;
      c = *end, *end = 0;
      csvcell_setstring(&t->tok_sym, beg);

// QUARANTINE       fprintf(stderr, "TOK <- var '%.*s'\n",
// QUARANTINE         (int)(end-beg), beg);
      *end = c;
    }
    else
    {
      /* - - - - - - - - - - - - - - - - - - - *
       * operator
       * - - - - - - - - - - - - - - - - - - - */

      int best = 0;

      t->tok_code = -1;

      for( int i = 0; i < tc_lit; ++i )
      {
        int curr = match(opinfo[i].op_text, beg);
        if( best < curr ) best = curr, t->tok_code = i;
      }

      if( t->tok_code == -1 )
      {
        ok = 0;
        break;
      }
      end = beg + best;

// QUARANTINE       fprintf(stderr, "TOK <- op '%.*s'\n",
// QUARANTINE         (int)(end-beg), beg);
    }
  }

  if( !ok )
  {
    fprintf(stderr, "TOKENIZATION ERROR:\n");
    fprintf(stderr, "%s\n", text);
    arrow((int)(beg - work));
  }

  free(work);

  return ok;
}

/* ------------------------------------------------------------------------- *
 * calc_syntax_tree  --  form syntax tree from token list
 * ------------------------------------------------------------------------- */

static int calc_syntax_tree(calc_t *self)
{
  int ok = 1;
  ENTER

  if( setjmp(syntax_error_return) != 0 )
  {
    ok = 0;
    fprintf(stderr, "\n");
    fprintf(stderr, "SYNTAX ERROR:\n");

    int x = 0, t = 0;

    for( int i = 0; i < self->calc_fifo.stk_tail; ++i )
    {
      calctok_t *tok = self->calc_fifo.stk_data[i];
      char tmp[64];
      if( tok == syntax_error_token ) { t = x; }
      x += fprintf(stderr, "%s ", calctok_repr(tok, tmp));
    }
    fprintf(stderr, "\n");
    arrow(t);
    //fprintf(stderr, "%*s^\n", t, "");
  }
  else
  {
    if( calc_expect_num(self) )
    {
      while( calc_expect_op(self) && calc_expect_num(self) ) { }
    }
    assert( calcstk_empty(&self->calc_fifo) );

    while( calc_otop(self) != 0 )
    {
      calc_consume_eval(self);
    }
  }
  LEAVE
  return ok;
}

/* ------------------------------------------------------------------------- *
 * calc_evalsub  --  evaluate numerical value of token tree
 * ------------------------------------------------------------------------- */

static csvcell_t *calc_evalsub(calc_t *self, calctok_t *root)
{
  auto double value(const csvcell_t *cell)
  {
    return cell->cc_number;
  }

  auto int istrue(const csvcell_t *cell)
  {
    return fabs(cell->cc_number) > EPSILON;
  }

  auto csvcell_t *a1(void) { return calc_evalsub(self, root->tok_arg1); }
  auto csvcell_t *a2(void) { return calc_evalsub(self, root->tok_arg2); }
  auto int        b1(void) { return istrue(a1()); }
  auto int        b2(void) { return istrue(a2()); }
  auto double     v1(void) { return value(a1()); }
  auto double     v2(void) { return value(a2()); }

  csvcell_t *res = &root->tok_val;

  ENTER

  switch( root->tok_code )
  {
  case tc_and:
    csvcell_setnumber(res, b1() && b2());
    break;
  case tc_or:
    csvcell_setnumber(res, b1() || b2());
    break;

  case tc_opt:
    if( !b1() )
    {
      csvcell_setnumber(res, 0);
    }
    else
    {
      *res = *a2();
    }
    break;

  case tc_op2:
    abort();

  case tc_op1:
    assert( root->tok_arg2->tok_code == tc_op2 );
    if( b1() )
    {
      *res = *calc_evalsub(self, root->tok_arg2->tok_arg1);
    }
    else
    {
      *res = *calc_evalsub(self, root->tok_arg2->tok_arg2);
    }
    break;

  case tc_add:
    csvcell_setnumber(res, v1() + v2());
    break;
  case tc_sub:
    csvcell_setnumber(res, v1() - v2());
    break;
  case tc_mul:
    csvcell_setnumber(res, v1() * v2());
    break;
  case tc_div:
    //csvcell_setnumber(res, v1() / v2());
    {
      double d = v2();
      if( fabs(d) > DBL_MIN )
      {
        csvcell_setnumber(res, v1() / d);
      }
      else
      {
        csvcell_setstring(res, "DIV0");
      }
    }
    break;

  case tc_eq:
    csvcell_setnumber(res, csvcell_compare(a1(), a2()) == 0);
    break;
  case tc_ne:
    csvcell_setnumber(res, csvcell_compare(a1(), a2()) != 0);
    break;
  case tc_lt:
    csvcell_setnumber(res, csvcell_compare(a1(), a2()) <  0);
    break;
  case tc_gt:
    csvcell_setnumber(res, csvcell_compare(a1(), a2()) >  0);
    break;
  case tc_le:
    csvcell_setnumber(res, csvcell_compare(a1(), a2()) <= 0);
    break;
  case tc_ge:
    csvcell_setnumber(res, csvcell_compare(a1(), a2()) >= 0);
    break;

  case tc_mod:
    csvcell_setnumber(res, fmod(v1(),v2()));
    break;
  case tc_pow:
    csvcell_setnumber(res, pow(v1(),v2()));
    break;

  case tc_not:
    csvcell_setnumber(res, !b2());
    break;

  case tc_neg:
    csvcell_setnumber(res, -v2());
    break;

  case tc_lit:
    break;

  case tc_var:
    if( root->tok_sym.cc_string == 0 )
    {
      fprintf(stderr, "get target not variable!\n");
    }
    else
    {
      self->calc_getvar(self, self->calc_userdata, root);
    }
    break;

  case tc_set:
    if( root->tok_arg1->tok_sym.cc_string == 0 )
    {
      fprintf(stderr, "set target not variable!\n");
    }
    else
    {
      root->tok_arg1->tok_val = *res = *a2();
      self->calc_setvar(self, self->calc_userdata, root->tok_arg1);
    }
    break;

  default:
    assert(0);
    abort();
  }

  LEAVE
  return &root->tok_val;
}

/* ------------------------------------------------------------------------- *
 * calc_clear
 * ------------------------------------------------------------------------- */

void calc_clear(calc_t *self)
{
  free(self->calc_expr);
  self->calc_expr = 0;

  calcstk_clear(&self->calc_vstk);
  calcstk_clear(&self->calc_ostk);
  calcstk_clear(&self->calc_fifo);
}

/* ------------------------------------------------------------------------- *
 * calc_create
 * ------------------------------------------------------------------------- */

calc_t *calc_create(void)
{
  calc_t *self = calloc(1, sizeof *self);

  self->calc_expr = 0;

  calcstk_ctor(&self->calc_fifo,1);
  calcstk_ctor(&self->calc_ostk,0);
  calcstk_ctor(&self->calc_vstk,0);

  self->calc_userdata = 0;
  self->calc_getvar = calc_getenv;
  self->calc_setvar = calc_setenv;

  return self;
}

/* ------------------------------------------------------------------------- *
 * calc_delete
 * ------------------------------------------------------------------------- */

void calc_delete(calc_t *self)
{
  if( self != 0 )
  {
    calc_clear(self);

    calcstk_dtor(&self->calc_vstk);
    calcstk_dtor(&self->calc_ostk);
    calcstk_dtor(&self->calc_fifo);

    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * calc_compile  --  tokenize & parse expression text
 * ------------------------------------------------------------------------- */

int calc_compile(calc_t *self, const char *expr)
{
  calctok_t *root = 0;

  if( calc_tokenize_expression(self, expr) != 0 )
  {
    if( calc_syntax_tree(self) )
    {
      root = calc_root(self);
    }
  }
  return root != 0;
}

/* ------------------------------------------------------------------------- *
 * calc_evaluate  --  evaluate compiled expression
 * ------------------------------------------------------------------------- */

double calc_evaluate(calc_t *self)
{
  calctok_t *root = calc_root(self);
  if( root != 0 )
  {
    calc_evalsub(self, root);
    return root->tok_val.cc_number;
  }
  return 0;
}

/* ------------------------------------------------------------------------- *
 * calc_compile_and_evaluate -- just gimme the result ...
 * ------------------------------------------------------------------------- */

double calc_compile_and_evaluate(calc_t *self, const char *expr)
{
  if( calc_compile(self, expr) != 0 )
  {
    return calc_evaluate(self);
  }
  return 0.0;
}

/* ========================================================================= *
 * test main
 * ========================================================================= */

#ifdef TESTMAIN

void infix(calctok_t *t)
{
  if( t != 0 )
  {
    char tmp[32];
    if( t->tok_arg1 || t->tok_arg2 )
    {
      fprintf(stderr, "(");
      infix(t->tok_arg1);
      fprintf(stderr, " %s ", calctok_repr(t,tmp));
      infix(t->tok_arg2);
      fprintf(stderr, ")");
    }
    else
    {
      fprintf(stderr, "%s", calctok_repr(t,tmp));
    }
  }
}

void prefix(calctok_t *t)
{
  if( t != 0 )
  {
    char tmp[32];
    fprintf(stderr, " %s", calctok_repr(t,tmp));
    prefix(t->tok_arg1);
    prefix(t->tok_arg2);
  }
}

void postfix(calctok_t *t)
{
  if( t != 0 )
  {
    char tmp[32];
    postfix(t->tok_arg1);
    postfix(t->tok_arg2);
    fprintf(stderr, " %s", calctok_repr(t,tmp));
  }
}

int main(int ac, char **av)
{
  if( ac < 2 ) { calcop_list(); }

  calc_t *c = calc_create();

  for( int i = 1; i < ac; ++i )
  {
    const char *e = av[i];

    if( calc_compile(c, e) != 0)
    {
      calctok_t *root = calc_root(c);
// QUARANTINE       if( root != 0 )
// QUARANTINE       {
// QUARANTINE   fprintf(stderr, "infix:  "); infix(root); fprintf(stderr, "\n");
// QUARANTINE   fprintf(stderr, "prefix: "); prefix(root); fprintf(stderr, "\n");
// QUARANTINE   fprintf(stderr, "postfix:"); postfix(root); fprintf(stderr, "\n");
// QUARANTINE       }
      calctok_emit2(root, 0);

      double r = calc_evaluate(c);
      fprintf(stderr, "%s = %g\n", e, r);
    }

  }

  calc_delete(c);
  return 0;
}
#endif
