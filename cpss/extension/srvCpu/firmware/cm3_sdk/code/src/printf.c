/*
 * Copyright (c) 2004,2012 Kustaa Nyholm / SpareTimeLabs
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * Neither the name of the Kustaa Nyholm or SpareTimeLabs nor the names of its
 * contributors may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#include "printf.h"
#include <cli.h>

typedef void ( *putcf ) ( void *, signed char );

#define PRINTF_LONG_SUPPORT
#ifdef PRINTF_LONG_SUPPORT

static void uli2a( unsigned long long num, unsigned long base, long uc, signed char * bf )
{
    long n = 0;
    unsigned long d = 1;
    while ( num / d >= base )
        d *= base;
    while ( d != 0 ) {
        long dgt = num / d;
        num %= d;
        d /= base;
        if ( n || dgt > 0 || d == 0 ) {
            *bf++ = dgt + ( dgt < 10 ? '0' : ( uc ? 'A' : 'a' ) - 10 );
            ++n;
            }
        }
    *bf = 0;
}

static void li2a ( long num, signed char * bf )
{
    if ( num < 0 ) {
        num =- num;
        *bf++ = '-';
        }
    uli2a( num, 10, 0, bf );
}

#endif

static void ui2a( unsigned long num, unsigned long base, long uc, signed char * bf )
{
    long n = 0;
    unsigned long d = 1;
    while ( num / d >= base )
        d *= base;
    while ( d != 0 ) {
        long dgt = num / d;
        num %= d;
        d /= base;
        if ( n || dgt > 0 || d == 0 ) {
            *bf++ = dgt + ( dgt < 10 ? '0' : ( uc ? 'A' : 'a' ) - 10 );
            ++n;
            }
        }
    *bf = 0;
}

static void i2a ( long num, signed char * bf )
{
    if ( num < 0 ) {
        num = -num;
        *bf++ = '-';
        }
    ui2a( num, 10, 0, bf );
}

static long a2d( signed char ch )
{
    if ( ch >= '0' && ch <= '9' )
        return ch - '0';
    else if ( ch >= 'a' && ch <= 'f' )
        return ch - 'a' + 10;
    else if ( ch >= 'A' && ch <= 'F' )
        return ch - 'A' + 10;
    else return -1;
}

static signed char a2i( signed char ch, signed char ** src, long base, long * nump )
{
    signed char * p = *src;
    long num = 0;
    long digit;
    while (( digit = a2d( ch )) >= 0 ) {
        if ( digit > base ) break;
        num = num * base + digit;
        ch = *p++;
        }
    *src = p;
    *nump = num;
    return ch;
}

static void putchw( void * putp, putcf putf, long n, signed char z, signed char * bf )
{
    signed char fc = z ? '0' : ' ';
    signed char ch;
    signed char * p = bf;
    while ( *p++ && n > 0 )
        n--;
    while ( n-- > 0 )
        putf( putp, fc );
    while (( ch = *bf++ ))
        putf( putp, ch );
}

void tfp_format( void * putp, putcf putf, signed char *fmt, va_list va )
{
    signed char bf[ 12 ];

    signed char ch;


    while (( ch = *( fmt++ ))) {
        if ( ch != '%' )
            putf( putp, ch );
        else {
            signed char lz = 0;
#ifdef  PRINTF_LONG_SUPPORT
            signed char lng = 0;
#endif
            long w = 0;
            ch = *( fmt++ );
            if ( ch == '0' ) {
                ch = *( fmt++ );
                lz = 1;
                }
            if ( ch >= '0' && ch <= '9' ) {
                ch = a2i( ch, &fmt, 10, &w );
                }
#ifdef  PRINTF_LONG_SUPPORT
            if ( ch == 'l' ) {
                ch = *( fmt++ );
                lng = 1;
            }
#endif
            switch ( ch ) {
                case 0 :
                    goto abort;
                case 'u' : {
#ifdef  PRINTF_LONG_SUPPORT
                    if ( lng )
                        uli2a( va_arg( va, unsigned long long ), 10, 0, bf );
                    else
#endif
                    ui2a( va_arg( va, unsigned long ), 10, 0, bf );
                    putchw( putp, putf, w, lz, bf );
                    break;
                    }
                case 'd' :  {
#ifdef  PRINTF_LONG_SUPPORT
                    if ( lng )
                        li2a( va_arg( va, unsigned long long ), bf );
                    else
#endif
                    i2a( va_arg( va, long ), bf );
                    putchw( putp, putf, w, lz, bf );
                    break;
                    }
                case 'p' :
                    putf( putp, '0' );
                    putf( putp, 'x' );
                case 'x' : case 'X' :
#ifdef  PRINTF_LONG_SUPPORT
                    if ( lng )
                        uli2a( va_arg( va, unsigned long long ), 16,( ch == 'X' ), bf );
                    else
#endif
                    ui2a( va_arg( va, unsigned long ), 16,( ch == 'X' ), bf );
                    putchw( putp, putf, w, lz, bf );
                    break;
                case 'c' :
                    putf( putp,( signed char )( va_arg( va, long )));
                    break;
                case 's' :
                    putchw( putp, putf, w, 0, va_arg( va, signed char * ));
                    break;
                case '%' :
                    putf( putp, ch );
                default :
                    break;
                }
            }
        }
    abort:;
}


void putcn( void * p, signed char c )
{
    mvputc( c );
    if ( c == '\n' )
        mvputc( '\r' );
}

void tfp_printf( char * fmt, ... )
{
    va_list va;
    va_start( va, fmt );
    tfp_format( NULL, putcn, ( signed char * )fmt, va );
    va_end( va );
}

static void putcp( void * p, signed char c )
{
    *( *(( signed char ** )p ))++ = c;
}

void tfp_sprintf( char * s, char * fmt, ... )
{
    va_list va;
    va_start( va, fmt );
    tfp_format( &s, putcp, ( signed char * )fmt, va );
    putcp( &s, 0 );
    va_end( va );
}

