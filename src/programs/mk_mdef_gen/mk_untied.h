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
#ifndef MK_UNTIED_H
#define MK_UNTIED_H


#include <s3/s3.h>
#include "heap.h"
#include "hash.h"

int32 make_ci_list_frm_mdef(char  *mdeffile,
                            char  ***CIlist,
                            int32 *cilistsize);

int32 make_ci_list_cd_hash_frm_phnlist(char  *phnlist,
                                    char  ***CIlist,
                                    int32 *cilistsize,
                                    hashelement_t ***CDhash,
                                    int32 *NCDphones);

int32 make_ci_list_cd_hash_frm_mdef(char  *mdeffile,
                                    char  ***CIlist,
                                    int32 *cilistsize,
                                    hashelement_t ***CDhash,
                                    int32 *NCDphones);

int32  read_dict(char *dictfile, char *fillerdictfile,
		 dicthashelement_t ***dicthash);

int32 make_mdef_from_list(char *mdeffile,
			char **CIlist, 
                        int32 cilistsize,
                        heapelement_t **CDheap,
                        int32 cdheapsize,
                        char  *pgm);

int32 make_dict_triphone_list (dicthashelement_t **dicthash,
                          hashelement_t ***triphonehash);

int32 make_CD_heap(hashelement_t  **triphonehash,
                   int32  threshold,
                   heapelement_t ***CDheap,
                   int32  *cdheapsize);

int32 find_threshold(hashelement_t  **triphonehash);

int32  count_triphones (char *transfile,
                        dicthashelement_t **dicthash,
                        hashelement_t **tphnhash,
			phnhashelement_t ***CIhash);

int32   print_counts(char *countfn, phnhashelement_t  **CIhash,
                     hashelement_t **CDhash);
#endif
