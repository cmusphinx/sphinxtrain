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


my $index = 0;
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    $index = 2;
} else {
    $cfg_file = "etc/sphinx_train.cfg";
}

if (! -s "$cfg_file") {
    print ("unable to find default configuration file, use -cfg file.cfg or create etc/sphinx_train.cfg for default\n");
    exit -3;
}
require $cfg_file;

my $mdeffn   = "${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.ci.mdef";
my $hmm_dir  = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.ci_${CFG_DIRLABEL}";
my $meanfn   = "$hmm_dir/means";
my $varfn    = "$hmm_dir/variances";
my $mixwfn   = "$hmm_dir/mixture_weights";
my $tmp_str = time();
my $tempfn   = "${CFG_BASE_DIR}/tmp/questions.$tmp_str";
#my $questfn  = "${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.tree_questions";
my $questfn = ${CFG_QUESTION_SET};

my $logdir = "${CFG_LOG_DIR}/05.buildtrees";
mkdir ($logdir,0777) unless -d $logdir;
my $logfile = "$logdir/${CFG_EXPTNAME}.make_questions.log";

#$MAKE_QUEST = "~rsingh/09..sphinx3code/trainer/bin.alpha/make_quests";
my $MAKE_QUEST = "${CFG_BIN_DIR}/make_quests";

$| = 1; # Turn on autoflushing
&ST_Log ("    Make Questions\n");
&ST_HTML_Print ("\t" . &ST_FormatURL("$logfile", "Log File") . " ");

$|=1;
if (open PIPE, "$MAKE_QUEST -moddeffn $mdeffn -meanfn $meanfn -varfn $varfn -mixwfn $mixwfn -npermute 8 -niter 1 -qstperstt 20 -tempfn $tempfn -questfn $questfn -type ${CFG_HMM_TYPE} 2>&1 |") {

    open LOG,">$logfile";
while (<PIPE>) {
    print LOG "$_";
}
close PIPE;
close LOG;
&ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> completed </font>\n");
$| = 0;
exit 0;
}


&ST_HTML_Print ("\t\t<font color=\"$CFG_ERROR_COLOR\"> completed </font>\n");
exit -1
