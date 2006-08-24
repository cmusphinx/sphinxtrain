#!/usr/bin/perl
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
## This work was supported in part by funding from the Defense Advanced 
## Research Projects Agency and the National Science Foundation of the 
## United States of America, and the CMU Sphinx Speech Consortium.
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
## Author: Ricky Houghton 
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

my $mdeffn   = "${ST::CFG_BASE_DIR}/model_architecture/${ST::CFG_EXPTNAME}.ci.mdef";
my $hmm_dir  = "${ST::CFG_BASE_DIR}/model_parameters/${ST::CFG_EXPTNAME}.ci_${ST::CFG_DIRLABEL}";
my $meanfn   = "$hmm_dir/means";
my $varfn    = "$hmm_dir/variances";
my $mixwfn   = "$hmm_dir/mixture_weights";
my $tmp_str = time();
#my $tempfn   = "${ST::CFG_BASE_DIR}/tmp/questions.$tmp_str";
#my $questfn  = "${ST::CFG_BASE_DIR}/model_architecture/${ST::CFG_EXPTNAME}.tree_questions";
my $questfn = ${ST::CFG_QUESTION_SET};

my $logdir = "${ST::CFG_LOG_DIR}/40.buildtrees";
mkdir ($logdir,0777) unless -d $logdir;
my $logfile = "$logdir/${ST::CFG_EXPTNAME}.make_questions.log";

$| = 1; # Turn on autoflushing
Log ("    Make Questions\n");
HTML_Print ("\t" . FormatURL("$logfile", "Log File") . " ");

exit RunTool('make_quests', $logfile, 0,
	     -moddeffn => $mdeffn,
	     -meanfn => $meanfn,
	     -varfn => $varfn,
	     -mixwfn => $mixwfn,
	     -npermute => 8,
	     -niter => 1,
	     -qstperstt => 20,
	     -questfn => $questfn,
	     -type => $ST::CFG_HMM_TYPE);
