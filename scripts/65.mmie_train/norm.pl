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
## Modified: Long Qin
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

#*****************************************************************************
#  Baum-welch is done in several parts. This script gathers the results of
#  all those parts and then computes the discrete probability distributions
#  associated with all the states. It also computes the transition matrices.
#****************************************************************************

$| = 1; # Turn on autoflushing

die "USAGE: $0 <iter>" if @ARGV != 1;
my $iter = shift;

my $logdir  = "${ST::CFG_LOG_DIR}/65.mmie_train";
my $logfile = "$logdir/${ST::CFG_EXPTNAME}.${iter}.norm.log";
mkdir ($logdir,0777);

my $previter       = $iter - 1;
my $MODEL_TYPE     = 'cd';
my $modelname      = "${ST::CFG_EXPTNAME}.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}";
my $mmie_modelname = "${ST::CFG_EXPTNAME}.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}_mmie";

opendir(ACCUMDIR, $ST::CFG_BWACCUM_DIR)
    or die "Could not open $ST::CFG_BWACCUM_DIR: $!";
my @bwaccumdirs = map catdir($ST::CFG_BWACCUM_DIR, $_),
    grep /^\Q${ST::CFG_EXPTNAME}_buff_/, readdir(ACCUMDIR);
closedir(ACCUMDIR);

# input old mean and variance
my $in_hmm_dir;
if ($iter == 1) {
    $in_hmm_dir = "$ST::CFG_BASE_DIR/model_parameters/$modelname";
} else {
    $in_hmm_dir = "$ST::CFG_BASE_DIR/model_parameters/$mmie_modelname/iter_$previter";
}
my $in_means     = "$in_hmm_dir/means";
my $in_variances = "$in_hmm_dir/variances";

# output new mean and variance
my $out_hmm_dir   = "${ST::CFG_BASE_DIR}/model_parameters/$mmie_modelname/iter_$iter";
my $out_means     = "$out_hmm_dir/means";
my $out_variances = "$out_hmm_dir/variances";

my $constE  = defined($ST::CFG_MMIE_CONSTE) ? $ST::CFG_MMIE_CONSTE : "3.0";
my $fullvar = defined($ST::CFG_FULLVAR) ? $ST::CFG_FULLVAR : "no";

Log ("Normalization for iteration: $iter", 'result');

my $return_value = RunTool
    ('norm', $logfile, 0,
     -mmie => "yes",
     -accumdir => join(",", @bwaccumdirs),
     -inmeanfn => $in_means,
     -invarfn => $in_variances,
     -constE => $constE,
     -meanfn => $out_means,
#     -varfn => $out_variances,
     -fullvar => $fullvar
    );

if ($return_value) {
  LogError ("Failed to start norm");
}
exit ($return_value);
