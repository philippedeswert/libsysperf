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
 * File: csv_float.c
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 12-Sep-2007 Eero Tamminen
 * - use libc floating point functions ("#if 1"),
 *   the ones here don't seem to work with Glibc v2.5
 * 
 * 22-Nov-2006 Simo Piiroinen
 * - normalization bug fixed
 * 
 * 21-Jun-2005 Simo Piiroinen
 * - initial version
 * ========================================================================= */

#include <stddef.h>
#include <math.h>
#include <float.h>

#include "msg.h"
#include "csv_float.h"

#if 1
#include <stdlib.h>
#include <stdio.h>
double csv_float_parse(const char **ppos)
{
  return strtod((char *)*ppos, (char **)ppos);
}
char *csv_float_to_string(double num, char *buff, size_t size)
{
  //snprintf(buff, size, "%.11e", num);
  snprintf(buff, size, "%.11g", num);
  return buff;
}

/* With Glibc 2.5 in Maemo, this gives bogus numbers,
 * With Glibc 2.3.6 in Debian Etch or in Maemo it worked
 */
#else

/* ========================================================================= *
 * Internal Functions
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * digit_val_  --  numeric value of character at given position
 * ------------------------------------------------------------------------- */

static inline int digit_val_(const char *pos)
{
  switch( *pos )
  {
  case '0' ... '9': return *pos - '0';
  case 'A' ... 'Z': return *pos - 'A' + 10;
  case 'a' ... 'z': return *pos - 'a' + 10;
  }
  return 255;
}

/* ------------------------------------------------------------------------- *
 * parse_sign_  --  get +1/-1 from parse position
 * ------------------------------------------------------------------------- */

static inline double parse_sign_(const char **ppos)
{
  double sgn = 1.0;
  const char *pos = *ppos;
  switch( *pos++ )
  {
  case '-': sgn = -1.0;
  case '+': *ppos = pos; break;
  }
  return sgn;
}

/* ------------------------------------------------------------------------- *
 * parse_real_  --  get full number from parse position
 * ------------------------------------------------------------------------- */

static inline double parse_real_(const char **ppos, int radix)
{
  double sum = 0.0;
  const char *pos = *ppos;
  
  int digit;
  
  while( (digit = digit_val_(pos)) < radix )
  {
    pos += 1;
    sum *= radix;
    sum += digit;
  }
  
  *ppos = pos;
  return sum;
}

/* ------------------------------------------------------------------------- *
 * parse_frac_  --  get number fraction from parse position
 * ------------------------------------------------------------------------- */

static inline double parse_frac_(const char **ppos, int radix)
{
  double sum = 0.0;
  double exp = 1.0 / radix;
  
  const char *pos = *ppos;
  
  int digit;
  
  while( (digit = digit_val_(pos)) < radix )
  {
    pos += 1;
    sum += exp * digit;
    exp /= radix;
  }
  
  *ppos = pos;
  return sum;
}

/* ========================================================================= *
 * API Functions
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * csv_float_parse
 * ------------------------------------------------------------------------- */

double csv_float_parse(const char **ppos)
{
  const char *pos = *ppos;

// QUARANTINE   msg_debug("%s %s\n", __FUNCTION__, pos);
  
  /* - - - - - - - - - - - - - - - - - - - *
   * plus / minus
   * - - - - - - - - - - - - - - - - - - - */

  double sgn = parse_sign_(&pos);

// QUARANTINE   msg_debug("\tsign %.10g\n", sgn);
  
  /* - - - - - - - - - - - - - - - - - - - *
   * radix
   * - - - - - - - - - - - - - - - - - - - */

  int rad = 10;
  
  if( *pos == '0' )
  {
    switch( *++pos )
    {
    case 'X': case 'x':
      rad = 16;
      ++pos;
      break;
    case 'B': case 'b':
      rad = 2;
      ++pos;
      break;
    case '.':
      break;
    default:
      rad = 8;
      break;
    }
  }
  
  /* - - - - - - - - - - - - - - - - - - - *
   * integral value
   * - - - - - - - - - - - - - - - - - - - */
  
// QUARANTINE   msg_debug("\tradix %d\n", rad);

  double sum = parse_real_(&pos, rad);

  /* - - - - - - - - - - - - - - - - - - - *
   * fractional value 
   * - - - - - - - - - - - - - - - - - - - */

// QUARANTINE   msg_debug("\treal %.10g\n", sum);
  
  if( *pos == '.' )
  {
    pos += 1;
    sum += parse_frac_(&pos, rad);
  }

// QUARANTINE   msg_debug("\tfrac %.10g\n", sum);
  
  /* - - - - - - - - - - - - - - - - - - - *
   * negate
   * - - - - - - - - - - - - - - - - - - - */

  sum *= sgn;

// QUARANTINE   msg_debug("\tbase %.10g\n", sum);
  
  /* - - - - - - - - - - - - - - - - - - - *
   * exponent
   * - - - - - - - - - - - - - - - - - - - */

  if( pos != *ppos  && (*pos == 'e' || *pos == 'e' ) )
  {
    pos += 1;
    sgn  = parse_sign_(&pos);
    sgn *= parse_real_(&pos, 10);

    msg_debug("\texp %.10g\n", sgn);
    sum *= pow(10, sgn);
  }
  
  /* - - - - - - - - - - - - - - - - - - - *
   * update parse postion and return
   * - - - - - - - - - - - - - - - - - - - */
  
  *ppos = pos;

// QUARANTINE   msg_debug("\tres %.10g\n", sum);
  
  return sum;
}

/* ------------------------------------------------------------------------- *
 * csv_float_to_string
 * ------------------------------------------------------------------------- */

char *csv_float_to_string(double num, char *buff, size_t size)
{
  /* - - - - - - - - - - - - - - - - - - - *
   * BIG FAT NOTE: the 'size' parameter is
   * ignored  ... caveat caller
   * - - - - - - - - - - - - - - - - - - - */

#define CHECK_OVERFLOW 0  
  
  /* - - - - - - - - - - - - - - - - - - - *
   * precision must be > 10 to preserve
   * 32 bit integers intact
   * - - - - - - - - - - - - - - - - - - - */

#define PREC 10
  
  /* - - - - - - - - - - - - - - - - - - - *
   * digit area, prefixed with 4 x '0' to
   * allow generating ".00123" numbers
   * - - - - - - - - - - - - - - - - - - - */

  char dig_[4+PREC];
#define dig (dig_+4)
  
  char *pos = buff;
  
#if CHECK_OVERFLOW  
  char *end = buff + size - 1;
  void emit(int ch) { if( pos<end ) *pos++ = ch; }
#else
  void emit(int ch) { *pos++ = ch; }
#endif
  
  *(unsigned *)dig_ = 0x30303030;

  /* - - - - - - - - - - - - - - - - - - - *
   * emit sign if negative
   * - - - - - - - - - - - - - - - - - - - */
  
  if( num < 0 ) 
  { 
    emit('-');
    num = -num; 
  }

  if( num < DBL_MIN )
  {
    /* - - - - - - - - - - - - - - - - - - - *
     * zero handled as special case
     * - - - - - - - - - - - - - - - - - - - */

    emit('0');
  }
  else
  {
    int i;

    /* - - - - - - - - - - - - - - - - - - - *
     * normalize to "1.234" range
     * - - - - - - - - - - - - - - - - - - - */

    int xp = (int)floor(log10(num));
    num = num / pow(10, xp) + 0.5e-9;
    if( num >= 10.0 )
    {
      num /= 10.0, xp += 1;
    }

    /* - - - - - - - - - - - - - - - - - - - *
     * scan digits, mark last nonzero
     * - - - - - - - - - - - - - - - - - - - */

    int nz = 1;
    for( i = 0; i < PREC; ++i )
    {
      double now = floor(num);
      num -= now;
      num *= 10;
      
      if( (dig[i] = '0' + (int)now) != '0' )
      {
	nz = i+1;
      }
    }
    
    /* - - - - - - - - - - - - - - - - - - - *
     * determine decimal point position and
     * exponent scale (in hops of three)
     * - - - - - - - - - - - - - - - - - - - */

    int dp = xp + 1;
    int sc = 0;
    
    if( dp > 10 )
    {
      i = (dp - 4 + 3); i /= 3; i *= 3;
      dp -= i;
      sc += i;
    }
    else if( dp < 1-3 )
    {
      i = (dp - 0 - 3); i /= 3; i *= 3;
      dp -= i;
      sc += i;
    }
    
    /* - - - - - - - - - - - - - - - - - - - *
     * trailing zeroes for integral part
     * are significant
     * - - - - - - - - - - - - - - - - - - - */
    
    if( nz < dp ) nz = dp;
    
    /* - - - - - - - - - - - - - - - - - - - *
     * leading zeroes
     * - - - - - - - - - - - - - - - - - - - */

// QUARANTINE     if( dp < 0 )
// QUARANTINE     {
// QUARANTINE       emit('.');
// QUARANTINE       while(dp++)emit('0');
// QUARANTINE       dp=-1;
// QUARANTINE     }
    
    /* - - - - - - - - - - - - - - - - - - - *
     * emit digits & decimal point
     * - - - - - - - - - - - - - - - - - - - */

    
// QUARANTINE     for( i = 0; i < nz; ++i )
    for( i = (dp<0)?dp:0; i < nz; ++i )
    {
      if( i == dp ) emit('.');
      emit(dig[i]);
    }

    /* - - - - - - - - - - - - - - - - - - - *
     * emit exponent
     * - - - - - - - - - - - - - - - - - - - */

    if( sc != 0 )
    {
      emit('e');
      
      if( sc < 0 ) { emit('-'); sc = -sc; }
      
      i = 0;
      do { dig[i++] = '0' + sc % 10; } while( (sc /= 10) != 0 );
      while( i-- ) emit(dig[i]);
    }
  }
  *pos = 0;
  
// QUARANTINE   printf(">> %s <<\n", buff);
  
#undef PREC
  return buff;
}
#endif


#ifdef TESTMAIN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
int main(int ac, char **av)
{
  enum { N = 1024*4, LO = -32, HI = 32 };
  double *data = calloc(N, sizeof *data);
  
  for( int i = 0; i < N; ++i )
  {
    data[i] = (1.0 + 9.0 * rand() / (RAND_MAX + 1.0)) / 10.0;
    assert( 0.1 <= data[i] );
    assert( data[i] < 1.0  );
  }
  
  for( int e = LO; e <= HI; ++e )
  {
    double m = pow(10.0,e);
    for( int i = 0; i < N; ++i )
    {
      double val = data[i] * m;
      char   str[256],txt[256];
      
      snprintf(str,sizeof str, "%.9e", val);
      
      double ref = strtod(str,0);
      const char  *pos = str;
      double dbl = csv_float_parse(&pos);

      if( fabs(dbl - val)/val > 1e-6 )
      {
	printf("1: %g -> '%s' -> %g [%e]\n", val, str, dbl, val-dbl);
	exit(1);
      }
      
      csv_float_to_string(val, txt, sizeof txt);
      dbl = strtod(txt, 0);

      if( fabs(dbl - val)/val > 1e-6 )
      {
	printf("2: %g -> '%s' -> %g [%e]\n", val, txt, dbl, val-dbl);
	exit(1);
      }
      
// QUARANTINE       if( strcmp(str, txt) )
// QUARANTINE       {
// QUARANTINE 	printf("'%s' -> \n'%s'\n", str, txt);
// QUARANTINE 	//exit(1);
// QUARANTINE       }
    }
  }
  printf("OK\n");
  
  
// QUARANTINE   for( int i = 1; i < ac; ++i )
// QUARANTINE   {
// QUARANTINE     const char *srce = av[i];
// QUARANTINE     char temp[512];
// QUARANTINE     double val = csv_float_parse(&srce);
// QUARANTINE     
// QUARANTINE     csv_float_to_string(val, temp, sizeof temp);
// QUARANTINE     
// QUARANTINE     printf("'%s' -> %g = %e -> '%s'\n", av[i], val, val, temp);
// QUARANTINE   }
  return 0;
}
#endif
