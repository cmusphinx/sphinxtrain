/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.1  1995/10/12  17:47:11  eht
 * Initial revision
 *
 *
 */
