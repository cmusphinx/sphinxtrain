#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 2000 Carnegie Mellon University.  All rights 
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
#*************************************************************************
#  Build SphinxII models from Sphinx3 semicontinous models
#*************************************************************************
#
#  Author: Alan W Black (awb@cs.cmu.edu)
#

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

# If we want to use sphinx2, either we created semi continuous models,
# or we created continuous models, but with some restrictions, which
# we have to check.
unless  (($ST::CFG_HMM_TYPE eq ".semi.") or
     (($ST::CFG_HMM_TYPE eq ".cont.") and
      ($ST::CFG_STATESPERHMM == 5) and
      (($ST::CFG_FEATURE eq "1s_12c_12d_3p_12dd") or
       ($ST::CFG_FEATURE eq "s3_1x39")))) {
  Log ("MODULE: 99 Convert to Sphinx2 format models\n");
  Log("    Can not create models used by Sphinx-II.\n");
  Log("    If you intend to create models to use with Sphinx-II models, please rerun with:\n" .
	  "\$ST::CFG_HMM_TYPE = '.semi.' or\n" .
	  "\$ST::CFG_HMM_TYPE = '.cont' and \$ST::CFG_FEATURE = '1s_12c_12d_3p_12dd' and \$ST::CFG_STATESPERHMM = '5'\n");
  exit(0);
}

my $scriptdir = "scripts_pl/99.make_s2_models";
my $logdir = "$ST::CFG_LOG_DIR/99.make_s2_models";

$| = 1; # Turn on autoflushing
Log ("MODULE: 99 Convert to Sphinx2 format models\n");
Log ("Phase 1: Cleaning up old log files...\n");
rmtree ($logdir) unless ! -d $logdir;
mkdir ($logdir,0777);

my $return_value;

my $logfile_cb = "$logdir/${ST::CFG_EXPTNAME}.mk_s2cb.log";
my $logfile_chmm = "$logdir/${ST::CFG_EXPTNAME}.mk_s2chmm.log";
my $logfile_senone = "$logdir/${ST::CFG_EXPTNAME}.mk_s2sendump.log";
my $logfile_s2phonemap = "$logdir/${ST::CFG_EXPTNAME}.mk_s2phonemap.log";

my $s3mdef = "$ST::CFG_BASE_DIR/model_architecture/$ST::CFG_EXPTNAME.$ST::CFG_N_TIED_STATES.mdef";

my ($s3hmmdir, $s2dir);
if ($ST::CFG_HMM_TYPE eq ".semi.") {
  $s3hmmdir="$ST::CFG_BASE_DIR/model_parameters/$ST::CFG_EXPTNAME.cd_${ST::CFG_DIRLABEL}_$ST::CFG_N_TIED_STATES"."_delinterp";
  $s2dir = "$ST::CFG_BASE_DIR/model_parameters/$ST::CFG_EXPTNAME.cd_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}.s2models";
} else {
  $s3hmmdir="$ST::CFG_BASE_DIR/model_parameters/$ST::CFG_EXPTNAME.cd_${ST::CFG_DIRLABEL}_$ST::CFG_N_TIED_STATES"."_$ST::CFG_FINAL_NUM_DENSITIES";
  $s2dir = "$ST::CFG_BASE_DIR/model_parameters/$ST::CFG_EXPTNAME.cd_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}_$ST::CFG_FINAL_NUM_DENSITIES.s2models";
}
mkdir ($s2dir,0777);
my $s3mixw = "$s3hmmdir/mixture_weights";
my $s3mean = "$s3hmmdir/means";
my $s3var = "$s3hmmdir/variances";
my $s3tmat = "$s3hmmdir/transition_matrices";

Log("Phase 2: Copy noise dictionary\n");
open IN, "<$ST::CFG_FILLERDICT" or die "Failed to open $ST::CFG_FILLERDICT: $!";
open OUT, ">".catfile($s2dir, 'noisedict') or die "Failed to open $s2dir/noisedict: $!";
while (<IN>) {
    # Sphinx-II creates these dictionary entries (<s>, </s>) automatically
    next if /<\/?s>/i;
    print OUT;
}
close IN;
close OUT;

if ($ST::CFG_HMM_TYPE eq ".semi.") {
  Log ("Phase 3: Make codebooks\n");
  HTML_Print ("\t" . FormatURL("$logfile_cb", "Log File") . " ");
  $return_value = RunTool('mk_s2cb', $logfile_cb, 0,
			  -meanfn => $s3mean,
			  -varfn => $s3var,
			  -cbdir => $s2dir,
			  -varfloor => 0.00001);
warn "$return_value\n";
  #HTML_Print ("\t\t<font color=\"$ST::CFG_OKAY_COLOR\"> completed </font>\n");
  exit ($return_value != 0) if ($return_value);
} else {
  Log ("Phase 3: Copying mdef, means, vars, mix_weights\n");
  my $s2mdef = "$s2dir/mdef";
  copy($s3mdef, $s2mdef);
  my $s2mixw = "$s2dir/mixture_weights";
  copy($s3mixw, $s2mixw);
  my $s2mean = "$s2dir/means";
  copy($s3mean, $s2mean);
  my $s2var = "$s2dir/variances";
  copy($s3var, $s2var);
}

Log ("Phase 4: Make chmm files\n");
my $return_value;
if ($ST::CFG_HMM_TYPE eq ".semi.") {
    $return_value = RunTool('mk_s2hmm', $logfile_chmm, 0,
			    -moddeffn => $s3mdef,
			    -mixwfn => $s3mixw,
			    -tmatfn => $s3tmat,
			    -hmmdir => $s2dir)
} else {
    $return_value = RunTool('mk_s2hmm', $logfile_chmm, 0,
			    -moddeffn => $s3mdef,
			    -tmatfn => $s3tmat,
			    -hmmdir => $s2dir,
			    -mtype => 'fchmm');
}
exit $return_value if ($return_value);

if ($ST::CFG_HMM_TYPE eq ".semi.") {
  Log ("Phase 5: Make senone file\n");
  HTML_Print ("\t" . FormatURL("$logfile_senone", "Log File") . " ");
  $return_value = RunTool('mk_s2sendump', $logfile_senone, 0,
			  -moddeffn => $s3mdef,
			  -mixwfn => $s3mixw,
			  -mwfloor => 0.0000001,
			  -sendumpfn => catfile($s2dir, 'sendump'));
  exit $return_value if ($return_value);
}

Log ("Phase 6: Make phone and map files\n");
HTML_Print ("\t" . FormatURL("$logfile_s2phonemap", "Log File") . " ");

$return_value = RunTool('mk_s2phonemap', $logfile_s2phonemap, 0,
			-moddeffn => $s3mdef,
			-phonefn => catfile($s2dir, 'phone'),
			-mapfn => catfile($s2dir, 'map'));
exit $return_value;

