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

if ($ST::CFG_HMM_TYPE eq ".cont.") {
  Log("    Skipped for continuous models\n");
  exit(0);
}

# this script runs deleted interpolation on a bunch of semi-cont
# hmm buffers. You need 2 or more buffers to run this!!

my $nsenones = "$ST::CFG_N_TIED_STATES";

my $cilambda = 0.9;

# up to 99 buffers
my $cd_hmmdir = "$ST::CFG_BASE_DIR/model_parameters/$ST::CFG_EXPTNAME.cd_${ST::CFG_DIRLABEL}_"."$ST::CFG_N_TIED_STATES";
my $bwaccumdir 	     = "${ST::CFG_BASE_DIR}/bwaccumdir";
opendir(ACCUMDIR, $bwaccumdir)
    or die "Could not open $bwaccumdir: $!";
my @bwaccumdirs = map catdir($bwaccumdir, $_),
    grep /^${ST::CFG_EXPTNAME}_buff_/, readdir(ACCUMDIR);
closedir(ACCUMDIR);

my $hmm_dir = "$ST::CFG_BASE_DIR/model_parameters/$ST::CFG_EXPTNAME.cd_${ST::CFG_DIRLABEL}_"
    . $ST::CFG_N_TIED_STATES . "_delinterp";
mkdir ($hmm_dir,0777);

copy "$cd_hmmdir/means", "$hmm_dir/means";
copy "$cd_hmmdir/variances", "$hmm_dir/variances";
copy "$cd_hmmdir/transition_matrices", "$hmm_dir/transition_matrices";
my $mixwfn = "$hmm_dir/mixture_weights";

my $moddeffn = "$ST::CFG_BASE_DIR/model_architecture/$ST::CFG_EXPTNAME.$ST::CFG_N_TIED_STATES.mdef";

my $logdir = "$ST::CFG_BASE_DIR/logdir/90.deleted_interpolation";
my $logfile = "$logdir/$ST::CFG_EXPTNAME.deletedintrep-${nsenones}.log";

$| = 1; # Turn on autoflushing
Log ("MODULE: 90 deleted interpolation\n");
Log ("    Cleaning up directories: logs...\n");
rmtree($logdir, 0, 1);
mkdir($logdir,0777);

Log ("    Doing interpolation...\n");
HTML_Print ("\t" . FormatURL("$logfile", "Log File") . " ");

exit RunTool('delint', $logfile, 0,
	     -accumdirs => @bwaccumdirs,
	     -moddeffn => $moddeffn,
	     -mixwfn => $mixwfn,
	     -cilambda => $cilambda,
	     -feat => $ST::CFG_FEATURE,
	     -ceplen => $ST::CFG_VECTOR_LENGTH,
	     -maxiter => 4000);
