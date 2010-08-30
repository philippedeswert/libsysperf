// -*- mode: c -*-

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
 * File: calculator.inc
 * 
 * Author: Simo Piiroinen
 * 
 * -------------------------------------------------------------------------
 * 
 * History:
 * 
 * 25-Sep-2006 Simo Piiroinen
 * - added '?' operator (COL=="")?(COL="XYZ")
 * - 'num' & 'str' merged to 'lit'
 *
 * 28-Jun-2005 Simo Piiroinen
 * - '||' vs. '(' out priority fixed
 * - imported from track2
 * 
 * 15-Jun-2005 Simo Piiroinen
 * - can use OP_HIDE_SPECIAL to limit visibility of macros only
 *   to normal operators
 * 
 * 07-Jan-2005 Simo Piiroinen
 * - added '||' and '&&' operators
 * - fixed '+' & '-' priorities higher than equality tests
 * 
 * 26-Aug-2004 Simo Piiroinen
 * - first version
 * ========================================================================= */

#ifdef OP
/*         A I O  A=0->number, A=1->unary, A=2->operator
 *                I=priority to go IN to stack
 *                O=priority to get OUT of stack
 */

//----------------
// precedence order
//----------------

OP("||", or,  2, 10,10)
OP("&&", and, 2, 20,20)

OP("==", eq,  2, 30,30) // EPSILON used for floating point equality tests
OP("!=", ne,  2, 30,30)
OP("<=", le,  2, 30,30)
OP(">=", ge,  2, 30,30)
OP("<",  lt,  2, 30,30)
OP(">",  gt,  2, 30,30)

OP("+",  add, 2, 40,40)
OP("-",  sub, 2, 40,40)

OP("*",  mul, 2, 50,50)
OP("/",  div, 2, 50,50)
OP("%",  mod, 2, 50,50)

OP("^",  pow, 2, 60,60)

OP("!",  not, 1, 70,70)

//----------------
// special
//----------------

OP("-",  neg, 1, 70,70) // 'sub' -> 'neg' if expecting number

#ifndef OP_HIDE_SPECIAL
OP("(",  par, 1, 80,00)
OP(")",  ens, 2,  0, 0)
OP("=",  set, 2, 90, 0)
OP("#",  opt, 2,  5, 5)

OP("?",  op1, 2,  5, 0)
OP(":",  op2, 2,  5, 0)

//----------------
// values start here
//----------------
OP("#",  lit, 0, 0,0)
OP("$",  var, 0, 0,0)
#else
# undef OP_HIDE_SPECIAL
#endif

# undef OP
#endif
