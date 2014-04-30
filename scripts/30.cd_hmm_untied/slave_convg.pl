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
## Author: David Huggins-Daines
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

if ($ST::CFG_CD_TRAIN eq 'no') {
    Log("Skipped (set \$CFG_CD_TRAIN = 'yes' to enable)\n");
    exit 0;
}

#***************************************************************************
# This script launches all the ci - continuous training jobs in the proper
# order. First it cleans up the directories, then launches the
# flat initialization, and the baum-welch and norm jobs for the required
# number of iterations. Within each iteration it launches as many baumwelch
# jobs as the number of parts we wish to split the training into.
#***************************************************************************

my ($iter, $n_parts) = @ARGV;
$iter = 1 unless defined $iter;
$n_parts = (defined($ST::CFG_NPART) ? $ST::CFG_NPART : 1) unless defined $n_parts;
# Otherwise we use all memory in norm and die
$n_parts = 15 if $n_parts > 15;

my $modeldir  = "$ST::CFG_BASE_DIR/model_parameters";
mkdir ($modeldir,0777);

use vars qw($MLLT_FILE $MODEL_TYPE $CI_MODEL_TYPE);
$MLLT_FILE = catfile($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.mllt");
$MODEL_TYPE = 'cd';
$CI_MODEL_TYPE = 'ci';

$| = 1; # Turn on autoflushing
my $logdir = "${ST::CFG_LOG_DIR}/30.${MODEL_TYPE}_hmm_untied";
mkdir ("$logdir",0777);

#Read npart_untied from variables.def
my $return_value = 0;
if ($iter == 1) {
    # Clean up junk from earlier runs
    Log ("MODULE: 30 Training Context Dependent models\n");
    Log("Phase 1: Cleaning up directories:");
    # Don't do this on a queue, because of NFS bugs
    unless ($ST::CFG_QUEUE_TYPE eq 'Queue::PBS') {
	LogProgress("\taccumulator...");
	rmtree ($ST::CFG_BWACCUM_DIR, 0, 1);
	mkdir ($ST::CFG_BWACCUM_DIR,0777);
    }
    LogProgress("logs...");
    rmtree($logdir, 0, 1);
    mkdir ($logdir,0777);
    LogProgress("qmanager...\n");
    rmtree ($ST::CFG_QMGR_DIR, 0, 1);
    mkdir ($ST::CFG_QMGR_DIR,0777);
    LogStatus('completed');

    # For the first iteration Flat initialize models.
    $return_value = Initialize();
    exit ($return_value) if ($return_value);
    Log("Phase 3: Forward-Backward");
}

# Call baum_welch with iter part and n_parts,
# once done call norm_and_lauchbw.pl
my @deps;
for (my $i=1; $i<=$n_parts; $i++)
{
    push @deps, LaunchScript("bw.$iter.$i", ['baum_welch.pl', $iter, $i, $n_parts])
}
LaunchScript("norm.$iter", ['norm_and_launchbw.pl', $iter, $n_parts], \@deps);
# For the first iteration (i.e. the one that was called from the
# command line or a parent script), wait until completion or error
if ($iter == 1) {
    $return_value = WaitForConvergence($logdir);
}
exit $return_value;

exit 0;

sub Initialize () {
  my $cihmmdir = "${ST::CFG_BASE_DIR}/model_parameters/${ST::CFG_EXPTNAME}.${CI_MODEL_TYPE}_${ST::CFG_DIRLABEL}";
  my $cdhmmdir = "${ST::CFG_BASE_DIR}/model_parameters/${ST::CFG_EXPTNAME}.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_untied";
  mkdir ($cdhmmdir,0777);

  my $logdir  =  "${ST::CFG_LOG_DIR}/30.${MODEL_TYPE}_hmm_untied";
  mkdir ($logdir,0777);

  Log ("Phase 2: Initialization");
  my $untiedmdef = "${ST::CFG_BASE_DIR}/model_architecture/${ST::CFG_EXPTNAME}.untied.mdef";
  my $logfile = "$logdir/${ST::CFG_EXPTNAME}.make_alltriphonelist.log";

  # aligned transcripts and the list of aligned files is obtained as a result
  # of (03.) forced alignment or (04.) VTLN
  # FIXME: This should go in SphinxTrain::Util
  my ($listoffiles, $transcriptfile);
  if ($ST::CFG_FORCEDALIGN eq "yes") {
      $listoffiles   = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedfiles";
      $transcriptfile  = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedtranscripts";
  } elsif ($ST::CFG_VTLN eq "yes") {
      $listoffiles   = "$ST::CFG_BASE_DIR/vtlnout/${ST::CFG_EXPTNAME}.alignedfiles";
      $transcriptfile  = "$ST::CFG_BASE_DIR/vtlnout/${ST::CFG_EXPTNAME}.alignedtranscripts";
  } else {
      $listoffiles = $ST::CFG_LISTOFFILES;
      $transcriptfile = $ST::CFG_TRANSCRIPTFILE;
  }

  my $rv = RunTool('mk_mdef_gen', $logfile, 0,
		   -phnlstfn => $ST::CFG_RAWPHONEFILE,
		   -dictfn => GetDict(),
		   -fdictfn => $ST::CFG_FILLERDICT,
		   -lsnfn => $transcriptfile,
		   -ountiedmdef => $untiedmdef,
		   -n_state_pm => $ST::CFG_STATESPERHMM);
  return $rv if $rv;

  $logfile = "$logdir/${ST::CFG_EXPTNAME}.copycitocd.log";

  my $cd_mdeffile = "${ST::CFG_BASE_DIR}/model_architecture/${ST::CFG_EXPTNAME}.untied.mdef";
  $rv = RunTool
      ('init_mixw', $logfile, 0,
       -src_moddeffn => "${ST::CFG_BASE_DIR}/model_architecture/${ST::CFG_EXPTNAME}.ci.mdef",
       -src_ts2cbfn => $ST::CFG_HMM_TYPE,
       -src_mixwfn => "$cihmmdir/mixture_weights",
       -src_meanfn => "$cihmmdir/means",
       -src_varfn => "$cihmmdir/variances",
       -src_tmatfn => "$cihmmdir/transition_matrices",
       -dest_moddeffn => $cd_mdeffile,
       -dest_ts2cbfn => $ST::CFG_HMM_TYPE,
       -dest_mixwfn => "$cdhmmdir/mixture_weights",
       -dest_meanfn => "$cdhmmdir/means",
       -dest_varfn => "$cdhmmdir/variances",
       -dest_tmatfn => "$cdhmmdir/transition_matrices",
       -fullvar => $ST::CFG_FULLVAR);
  return $rv;
}


