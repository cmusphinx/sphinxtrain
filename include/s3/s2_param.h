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
/*********************************************************************
 *
 * file: s2_param.h
 * 
 * traceability: 
 * 
 * description: 
 * 
 * author: 
 * 
 *********************************************************************/

#ifndef S2_PARAM_H
#define S2_PARAM_H

#define S2_N_CODEWORD	256	/* number of codewords */

#define S2_N_STATE	6	/* 5 state w/ dummy end state */

#define S2_TRANSITION	6	/* index of transition id in tying DAG */

#define S2_CEP_VECLEN	13	/* dimension of the cepstrum vector */

#define S2_CEP_FEATURE		0
#define S2_DCEP_FEATURE		1
#define S2_POW_FEATURE		2
#define S2_2DCEP_FEATURE	3

#define S2_CEPF_VECLEN		12	/* dimension of the cepstrum feature */
#define S2_D1CEPF_VECLEN	12	/* dimension of the +/- 2 frm diff cep feature */
#define S2_D2CEPF_VECLEN	12	/* dimension of the +/- 2 frm diff cep feature */
#define S2_DCEPF_VECLEN		24	/* concatenated +/- 2 and +/- 4 frame */

#define S2_POWF_VECLEN		3	/* dimension of the pow feature */
#define S2_2DCEPF_VECLEN	12	/* dimension of the 2nd ord. diff cep feature */

#define S2_N_FEATURE	4	/* number of independent feature streams */


#define S2_SHORT_DIFFW	2	/* +/- # frames for short duration dcep */
#define S2_LONG_DIFFW	4	/* +/- # frames for short duration dcep */
#define S2_2ND_ORD_DIFFW	3	/* effective +/- # frames for 2nd ord dcep */

#define MIN_FLOAT32		1e-36

#define S2_ALPHA_BETA_EPSILON	0.01

#endif /* S2_PARAM_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  17:46:09  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.3  2001/04/05 20:02:30  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:12  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:30  awb
 * *** empty log message ***
 *
 * Revision 1.1  1995/10/12  17:47:11  eht
 * Initial revision
 *
 *
 */
