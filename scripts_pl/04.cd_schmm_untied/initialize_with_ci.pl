#!/usr/opt/PERL5004/bin/perl -w
## ====================================================================
##
## Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
## reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
##
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer. 
##
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in
##    the documentation and/or other materials provided with the
##    distribution.
##
## 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
##    endorse or promote products derived from this software without
##    prior written permission. To obtain permission, contact 
##    sphinx@cs.cmu.edu.
##
## 4. Products derived from this software may not be called "Sphinx"
##    nor may "Sphinx" appear in their names without prior written
##    permission of Carnegie Mellon University. To obtain permission,
##    contact sphinx@cs.cmu.edu.
##
## 5. Redistributions of any form whatsoever must retain the following
##    acknowledgment:
##    "This product includes software developed by Carnegie
##    Mellon University (http://www.speech.cs.cmu.edu/)."
##
## THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
## ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
## THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
## PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
## NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
## ====================================================================
##
## Author: Ricky Houghton (converted from scripts by Rita Singh)
##


require "/sphx_train/testing/scripts_pl/sphinx_train.cfg";


#**************************************************************************
# This script copies the ci model-parameters into cd-model parameters
# as initial model-parameters for the cd training.
# This is called "initializing cd models with ci models"
#**************************************************************************
die "USAGE: $0" if shift;

# Definitions

$cihmmdir = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.ci_semi";
$cdhmmdir = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.cd_semi_untied";

mkdir ($cdhmmdir,0777) unless -d $cdhmmdir;

$logdir  =  "${CFG_BASE_DIR}/logdir/04.cd_schmm_untied";
mkdir ($logdir,0777) unless -d $logdir;
$logfile = "$logdir/${CFG_EXPTNAME}.copycitocd.log";

#set mach = `~/51..tools/machine_type.csh`
#set COPY_CI_TO_CD = /net/alf19/usr2/eht/s3/bin.$mach/init_mixw
#set COPY_CI_TO_CD = ~/09..sphinx3code/trainer/bin.$mach/init_mixw
$COPY_CI_TO_CD = "${CFG_BIN_DIR}/init_mixw";

system ("$COPY_CI_TO_CD \
	-src_moddeffn ${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.ci.mdef \
	-src_ts2cbfn  ${CFG_TYPE} \
	-src_mixwfn   $cihmmdir/mixture_weights \
	-src_meanfn   $cihmmdir/means \
	-src_varfn    $cihmmdir/variances \
	-src_tmatfn   $cihmmdir/transition_matrices \
	-dest_moddeffn ${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.untied.mdef \
	-dest_ts2cbfn  ${CFG_TYPE} \
	-dest_mixwfn   $cdhmmdir/mixture_weights \
	-dest_meanfn   $cdhmmdir/means \
	-dest_varfn    $cdhmmdir/variances \
	-dest_tmatfn   $cdhmmdir/transition_matrices \
        -feat          ${CFG_FEATURE} \
	-ceplen        ${CFG_VECTOR_LENGTH} \
>&! $logfile");

exit 0
