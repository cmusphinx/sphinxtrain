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

my $iter = 1;
if (@ARGV) {
    $iter = shift;
}

my $n_parts = ($ST::CFG_NPART) ? $ST::CFG_NPART : 1;
# If the number of parts is given as command line argument, overwrite
# the number coming from the config file
if (@ARGV) {
   $n_parts = shift;
}

# This script is meant to be run right after state tying.  It runs the
# first stage of CD-tied training (1 Gaussian), collects full
# covariances, and completes by generating an LDA transform
$| = 1; # Turn on autoflushing
my $logdir = "$ST::CFG_LOG_DIR/46.lda_train";

if ($iter == 1) {
    Log("MODULE: 46 Train LDA transformation\n");
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
    copyci2cd2initialize();
}

if ($iter eq 'N') {
    Log("Phase 4: LDA transform estimation");
    my @deps;
    for (my $i=1; $i<=$n_parts; $i++) {
	push @deps, LaunchScript("bw.lda.$iter.$i", ['baum_welch.pl', $iter, $i, $n_parts, 'yes']);
    }
    LaunchScript("lda", "lda_train.pl", \@deps);
}
else {
    Log("Phase 3: Forward-Backward");
    my @deps;
    for (my $i=1; $i<=$n_parts; $i++) {
	push @deps, LaunchScript("bw.lda.$iter.$i", ['baum_welch.pl', $iter, $i, $n_parts, 'no']);
    }
    LaunchScript("norm.$iter", ['norm_and_launchbw.pl', $iter, $n_parts], \@deps);
    # On the first iteration, wait for the LDA stuff to complete
    my $lda_log = File::Spec->catfile($logdir, "$ST::CFG_EXPTNAME.lda_train.log");
    if ($iter == 1) {
	# This is kind of a lousy way to do it, but oh well...
	local $SIG{CHLD} = sub { wait; };
	my $interval = 5;
	while (1) {
	    # Look for an error
	    for ($iter = 1; $iter <= $ST::CFG_MAX_ITERATIONS; ++$iter) {
		my $norm_log = File::Spec->catfile($logdir,
						   "$ST::CFG_EXPTNAME.$iter.norm.log");
		if (open LOG, "<$norm_log") {
		    if (/failed/ or /Aborting/) {
			LogError("Training failed in iteration $iter");
			exit 1;
		    }
		}
	    }
	    if (open LOG, "<$lda_log") {
		while (<LOG>) {
		    if (/failed/) {
			LogError("LDA Training failed");
			exit 1;
		    }
		    elsif (/complete/) {
			Log("LDA Training completed", 'result');
			exit 0;
		    }
		}
	    }
	    sleep $interval;
	}
    }
}

sub copyci2cd2initialize ()
{
    Log("Phase 2: Copy CI to CD initialize\n");

    #**************************************************************************
    # this script copies the mixw/mean/var/tmat from a ci (continuous) HMM
    # to a cd (continuous) HMM. We need two mdef files, one without triphones
    # for the ci hmm definition and another one with triphones for the cd
    # triphone definition.
    # our final goal is to train in stages, first the ci models and then
    # with cd models. This should minimize initialization problems..
    #**************************************************************************
    my $ci_hmmdir = "$ST::CFG_BASE_DIR/model_parameters/${ST::CFG_EXPTNAME}.ci_${ST::CFG_DIRLABEL}";
    my $src_moddeffn = "$ST::CFG_BASE_DIR/model_architecture/${ST::CFG_EXPTNAME}.ci.mdef";
    my $src_mixwfn = "$ci_hmmdir/mixture_weights";
    my $src_meanfn = "$ci_hmmdir/means";
    my $src_varfn = "$ci_hmmdir/variances";
    my $src_tmatfn = "$ci_hmmdir/transition_matrices";

    my $cd_hmmdir = "$ST::CFG_BASE_DIR/model_parameters/${ST::CFG_EXPTNAME}.cd_${ST::CFG_DIRLABEL}_initial";
    mkdir ($cd_hmmdir,0777);

    my $dest_moddeffn = "$ST::CFG_BASE_DIR/model_architecture/${ST::CFG_EXPTNAME}.$ST::CFG_N_TIED_STATES.mdef";
    my $dest_mixwfn = "$cd_hmmdir/mixture_weights";
    my $dest_meanfn = "$cd_hmmdir/means";
    my $dest_varfn = "$cd_hmmdir/variances";
    my $dest_tmatfn = "$cd_hmmdir/transition_matrices";

    my $logdir = "$ST::CFG_LOG_DIR/46.lda_train";
    mkdir ($logdir,0777);
    my $logfile = "$logdir/$ST::CFG_EXPTNAME.copy.ci.2.cd.log";

    my $rv = RunTool('init_mixw', $logfile, 0,
		   -src_moddeffn => $src_moddeffn,
		   -src_ts2cbfn => $ST::CFG_HMM_TYPE,
		   -src_mixwfn => $src_mixwfn,
		   -src_meanfn => $src_meanfn,
		   -src_varfn => $src_varfn,
		   -src_tmatfn => $src_tmatfn,
		   -dest_moddeffn => $dest_moddeffn,
		   -dest_ts2cbfn => $ST::CFG_HMM_TYPE,
		   -dest_mixwfn => $dest_mixwfn,
		   -dest_meanfn => $dest_meanfn,
		   -dest_varfn => $dest_varfn,
		   -dest_tmatfn => $dest_tmatfn,
		   -fullvar => $ST::CFG_FULLVAR);
    return $rv;
}

