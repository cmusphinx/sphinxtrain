/* ====================================================================
 * Copyright (c) 1989-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
/* last edit by Joe Keane on October 10, 1989 */

#define BASE		1.0001
#define LOG_BASE	9.9995000333297321e-05
#define R_LOG_BASE	1.0000499991668185e+04
/* #define MIN_LOG		(-1 << 29) */
#define MIN_LOG		(-1 << 27)
#define ADDITION_TABLE_SIZE 99042
#define SUBTRACTION_TABLE_SIZE 99042

#define FIX(X)		((X) <= MIN_LOG ? MIN_LOG : (X))
#define EXP(X)		(exp ((X) * LOG_BASE))
#define LOG(X)		((X) <= 0 ? MIN_LOG : (int)  (log ((double) (X)) * R_LOG_BASE))
#define MULTIPLY(X, Y)	((X) + (Y) <= MIN_LOG ? MIN_LOG : (X) + (Y))
#define ADD(X, Y)	((X) > (Y) ? (Y) <= MIN_LOG || (unsigned) ((X) - (Y)) >= ADDITION_TABLE_SIZE ? (X) : (X) + Addition_Table[(X) - (Y)] : (X) <= MIN_LOG || (unsigned) ((Y) - (X)) >= ADDITION_TABLE_SIZE ? (Y) : (Y) + Addition_Table[(Y) - (X)])
#define SUBTRACT(X, Y)	((X) <= (Y) ? MIN_LOG : (unsigned) ((X) - (Y)) >= SUBTRACTION_TABLE_SIZE ? (X) : (X) - Subtraction_Table[(X) - (Y)])

/* ADD_ASSIGN (X, Y) is equivalent to X = ADD (X, Y) and MULTIPLY_ASSIGN (X, Y) is equivalent to X = MULTIPLY (X, Y) but they are faster */
#define MULTIPLY_ASSIGN(X, Y) ((X) + (Y) <= MIN_LOG ? ((X) = MIN_LOG) : ((X) += (Y)))
#define ADD_ASSIGN(X, Y) ((X) > (Y) ? (Y) <= MIN_LOG || (unsigned) ((X) - (Y)) >= ADDITION_TABLE_SIZE ? (X) : ((X) += Addition_Table[(X) - (Y)]) : (X) <= MIN_LOG || (unsigned) ((Y) - (X)) >= ADDITION_TABLE_SIZE ? ((X) = (Y)) : ((X) = (Y) + Addition_Table[(Y) - (X)]))
#define SUBTRACT_ASSIGN(X, Y) ((X) <= (Y) ? ((X) = MIN_LOG) : (unsigned) ((X) - (Y)) >= SUBTRACTION_TABLE_SIZE ? (X) : ((X) -= Subtraction_Table[(X) - (Y)]))

#include <math.h>

extern short Addition_Table[ADDITION_TABLE_SIZE];
extern int Subtraction_Table[SUBTRACTION_TABLE_SIZE];
