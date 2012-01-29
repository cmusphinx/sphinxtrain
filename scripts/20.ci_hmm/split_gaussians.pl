#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 1996-2004 Carnegie Mellon University.  All rights 
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
## Author: Evandro Gouvea
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

#***************************************************************************
# This script splits the current set of gaussian models. The increase
# in number of Gaussian components is the argument used to call the
# script.
# ***************************************************************************

die "Usage: $0 [-cfg <config file>] <current n_gaussians> <increase>\n" if @ARGV != 2;

# With semi continuous models, we already start with the right number
# of components
if ($ST::CFG_HMM_TYPE ne ".cont.") {
  Log("Split Gaussian not performed for semi continuous or tied-mixture\n");
  exit 0;
}

if ($ST::CFG_CI_MGAU ne "yes") {
  Log("Split Gaussian not performed unless CFG_CI_MGAU = 'yes'\n");
  exit 0;
}

my ($n_current, $n_inc) = @ARGV;

# If $n_inc isn't at least 1, there's nothing else to do
if ($n_inc <= 0) {
  exit 0;
}

if ($n_current + $n_inc > $ST::CFG_FINAL_NUM_DENSITIES) {
  LogError("Increase in number of Gaussians beyond the desired total\n" .
	   "Current: $n_current, increase by: $n_inc, desired total: $ST::CFG_FINAL_NUM_DENSITIES\n");
  exit -3;
}

use vars qw($MLLT_FILE $MODEL_TYPE);
$MLLT_FILE = catfile($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.mllt");
$MODEL_TYPE = 'ci';

$| = 1; # Turn on autoflushing
my $logdir = "$ST::CFG_LOG_DIR/20.${MODEL_TYPE}_hmm";
mkdir ($logdir,0777);

my $modeldir  = "$ST::CFG_BASE_DIR/model_parameters";
mkdir ($modeldir,0777);

Log ("Split Gaussians, increase by $n_inc\n", 'result');

my $src_hmmdir = "$ST::CFG_BASE_DIR/model_parameters/${ST::CFG_EXPTNAME}.${MODEL_TYPE}_${ST::CFG_DIRLABEL}";
mkdir ($src_hmmdir,0777);

my $src_mixwfn = "$src_hmmdir/mixture_weights";
my $src_meanfn = "$src_hmmdir/means";
my $src_varfn = "$src_hmmdir/variances";
my $src_tmatfn = "$src_hmmdir/transition_matrices";

my $dest_hmmdir = "$ST::CFG_BASE_DIR/model_parameters/$ST::CFG_EXPTNAME.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_initial";
mkdir ($dest_hmmdir,0777);

my $dest_mixwfn = "$dest_hmmdir/mixture_weights";
my $dest_meanfn = "$dest_hmmdir/means";
my $dest_varfn = "$dest_hmmdir/variances";
my $dest_tmatfn = "$dest_hmmdir/transition_matrices";

my $backup_hmmdir =  "$ST::CFG_BASE_DIR/model_parameters/$ST::CFG_EXPTNAME.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_${n_current}";
mkdir ($backup_hmmdir,0777);

my $backup_mixwfn = "$backup_hmmdir/mixture_weights";
my $backup_meanfn = "$backup_hmmdir/means";
my $backup_varfn = "$backup_hmmdir/variances";
my $backup_tmatfn = "$backup_hmmdir/transition_matrices";

# Copy the current models to a safe location, to preserve them
copy "$src_mixwfn", "$backup_mixwfn";
copy "$src_meanfn", "$backup_meanfn";
copy "$src_varfn", "$backup_varfn";
copy "$src_tmatfn", "$backup_tmatfn";

# The transition file won't change, so just copy it
copy "$src_tmatfn", "$dest_tmatfn";

my $logfile = "$logdir/$ST::CFG_EXPTNAME.split_gaussians.$n_current.$n_inc.log";

my $rv = RunTool('inc_comp', $logfile, 0,
		 -ninc => $n_inc,
		 -dcountfn => $src_mixwfn,
		 -inmixwfn => $src_mixwfn,
		 -outmixwfn=> $dest_mixwfn,
		 -inmeanfn => $src_meanfn,
		 -outmeanfn=> $dest_meanfn,
		 -invarfn  => $src_varfn,
		 -outvarfn => $dest_varfn,
		 -fullvar => $ST::CFG_FULLVAR);
exit $rv;
