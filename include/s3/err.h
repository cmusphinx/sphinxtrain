/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */

#ifndef _ERR_H
#define _ERR_H

/*
 * Package for checking and catching common errors, printing out
 * errors nicely, etc.
 *
 * 6/01/95  Paul Placeway  CMU speech group
 */

#include <stdarg.h>

void _E__pr_header( const char *file, long line, const char *msg );
void _E__pr_info_header( const char *file, long line, const char *msg );

void _E__pr_warn( char *fmt, ... );
void _E__pr_info( char *fmt, ... );
void _E__die_error( char *fmt, ... );
void _E__abort_error( char *fmt, ... );
void _E__sys_error( char *fmt, ... );
void _E__fatal_sys_error( char *fmt, ... );

/* These three all abort */

/* core dump after error message */
#if !defined(WIN32)
#define E_ABORT  _E__pr_header(__FILE__, __LINE__, "ERROR"),_E__abort_error
#endif

/* exit with non-zero status after error message */
#define E_FATAL  _E__pr_header(__FILE__, __LINE__, "FATAL_ERROR"),_E__die_error

/* Print error text; Call perror(""); exit(errno); */
#define E_FATAL_SYSTEM	_E__pr_header(__FILE__, __LINE__, "FATAL"),_E__fatal_sys_error

/* Print error text; Call perror(""); */
#define E_WARN_SYSTEM	_E__pr_header(__FILE__, __LINE__, "WARN"),_E__sys_error

/* Print error text; Call perror(""); */
#define E_ERROR_SYSTEM	_E__pr_header(__FILE__, __LINE__, "ERROR"),_E__sys_error

/*
 * Prints error text only.
 *
 * This allows a lower level routine to give information regarding an error condition,
 * but allows higher level routines to give addl information and make the
 * determination whether or not to abort.
 */
#define E_ERROR	  _E__pr_header(__FILE__, __LINE__, "ERROR"),_E__pr_warn

#define E_INFO	  _E__pr_info_header(__FILE__, __LINE__, "INFO"),_E__pr_info

#define E_INFOCONT  _E__pr_info

#define E_WARN	  _E__pr_header(__FILE__, __LINE__, "WARNING"),_E__pr_warn

#endif /* !_ERR_H */
