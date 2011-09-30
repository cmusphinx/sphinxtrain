/* ====================================================================
 * Copyright (c) 2000 Carnegie Mellon University.  All rights 
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
#ifndef MK_UNTIED_H
#define MK_UNTIED_H


#include <s3/s3.h>
#include "heap.h"
#include "hash.h"

int32 make_ci_list_frm_mdef(const char  *mdeffile,
                            char  ***CIlist,
                            int32 *cilistsize);

int32 make_ci_list_cd_hash_frm_phnlist(const char  *phnlist,
                                    char  ***CIlist,
                                    int32 *cilistsize,
                                    hashelement_t ***CDhash,
                                    int32 *NCDphones);

int32 make_ci_list_cd_hash_frm_mdef(const char  *mdeffile,
                                    char  ***CIlist,
                                    int32 *cilistsize,
                                    hashelement_t ***CDhash,
                                    int32 *NCDphones);

int32  read_dict(const char *dictfile, const char *fillerdictfile,
		 dicthashelement_t ***dicthash);

int32 make_mdef_from_list(const char *mdeffile,
			char **CIlist, 
                        int32 cilistsize,
                        heapelement_t **CDheap,
                        int32 cdheapsize,
                        char  *pgm);

int32 make_dict_triphone_list (dicthashelement_t **dicthash,
                          hashelement_t ***triphonehash,
                          int ignore_wpos);

int32 make_CD_heap(hashelement_t  **triphonehash,
                   int32  threshold,
                   heapelement_t ***CDheap,
                   int32  *cdheapsize);

int32 find_threshold(hashelement_t  **triphonehash);

int32  count_triphones (const char *transfile,
                        dicthashelement_t **dicthash,
                        hashelement_t **tphnhash,
			phnhashelement_t ***CIhash,
			int ignore_wpos);

int32   print_counts(const char *countfn, phnhashelement_t  **CIhash,
                     hashelement_t **CDhash);
#endif
