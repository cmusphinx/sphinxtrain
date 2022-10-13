/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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

/** \file cb2mllr_io.h
 * \brief Implementation of code book to mllr regression class map. 
 */

#ifndef S3CB2MLLR_IO_H
#define S3CB2MLLR_IO_H

#include "s3types.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
/* Fool Emacs. */
}
#endif

/**
   Read codeword to MLLR class mapping file
*/
int
cb2mllr_read(const char *fn, /**< MLLR class mapping file.*/
	     int32 **out_cb2mllr,  /**< Output CB 2 MLLR */
	     uint32 *out_n_cb,      /**< Number of codebook */
	     uint32 *out_n_mllr     /**< Number of MLLR class */
    );

#ifdef __cplusplus
}
#endif


#endif /* S3CB2MLLR_IO_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.6  2006/02/22 15:02:47  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Follow Dave's change to make uint32 to int32. 2, Fixed some dox-doc issue.
 *
 *
 * Revision 1.5  2005/10/05 00:29:44  dhdfu
 * Silence some largely unimportant signedness warnings.
 *
 * Revision 1.4  2005/06/22 08:05:55  arthchan2003
 * Fixed spelling in doc
 *
 * Revision 1.3  2005/06/22 08:00:09  arthchan2003
 * Completed all doxygen documentation on file description for libs3decoder/libutil/libs3audio and programs.
 *
 * Revision 1.2  2005/06/22 03:29:35  arthchan2003
 * Makefile.am s  for all subdirectory of libs3decoder/
 *
 * Revision 1.1  2005/06/15 19:41:02  archan
 * Added cb2mllr_io into libam
 *
 * Revision 1.1  2005/04/15 14:16:51  dhdfu
 * Additional MLLR runtime support, multi-class (1 stream only) MLLR runtime support, regression and performance tests
 *
 * Revision 1.5  2005/01/18 19:10:05  dhdfu
 * Fix signedness issues in multi-class MLLR code, incorporate extra error checking from original Sam-Joo Doh code
 *
 * Revision 1.4  2004/07/21 17:46:10  egouvea
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
 * Revision 1.1  97/07/16  11:39:10  eht
 * Initial revision
 * 
 *
 */
