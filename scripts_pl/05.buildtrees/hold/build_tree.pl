#!/usr/bin/perl -w
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


require "../sphinx_train.cfg";

#*************************************************************************
# The script builds a tree for ONE state of ONE basephone. We therefore
# need to call it in a loop to build trees for all the states of all the
# basephones
#*************************************************************************

die "usage: $0 <phone> <state>" if ($#ARGV != 1);

$phone = $ARGV[0];
$state = $ARGV[1];

$mdef_file       = "${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.untied.mdef";
$mixture_wt_file = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.cd_semi_untied/mixture_weights";
$unprunedtreedir = "${CFG_BASE_DIR}/trees/${CFG_EXPTNAME}.unpruned";
mkdir ($unprunedtreedir,0777) unless -d $unprunedtreedir;

$logdir = "${CFG_LOG_DIR}/05.buildtrees";
mkdir ($logdir,0777) unless -d $logdir;

$logfile = "$logdir/${CFG_EXPTNAME}.buildtree.${phone}.${state}.log";

#$mach = `~/51..tools/machine_type.csh`;
#$BUILDTREE = "/net/alf19/usr2/eht/s3/bin.$mach/bldtree";
#$BUILDTREE = "~rsingh/09..sphinx3code/trees_cont/bin.$mach/bldtree";

$BUILDTREE = "${CFG_BIN_DIR}/bldtree";

#	-stwt     1.0 0.1 0.01 
#	-stwt     1.0 0.3 0.2 
#	-ssplitthr 8e-4 \
#	-csplitthr 1e-5 \

system ("$BUILDTREE -treefn   $unprunedtreedir/$phone-$state.dtree -moddeffn $mdef_file -mixwfn   $mixture_wt_file -ts2cbfn  .semi. -mwfloor  1e-30 -psetfn ${CFG_QUESTION_SET} -phone    $phone -state    $state -stwt     1.0 0.3 0.1 0.01 0.001 -ssplitmin 1 -ssplitmax 5 -ssplitthr 0 -csplitmin 1 -csplitmax 500 -csplitthr 0 >&! $logfile");

exit 0
