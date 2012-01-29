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
## Author: Long Qin
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
# This script launches all the mmie training jobs in the proper order.
# First it cleans up the directories, then launches the baum-welch and norm
# jobs for the required number of iterations. Within each iteration it
# launches as many baumwelch jobs as the number of parts we wish to split
# the training into.
#***************************************************************************

my ($iter, $n_parts) = @ARGV;
$iter = 1 unless defined $iter;
$n_parts = (defined($ST::CFG_NPART) ? $ST::CFG_NPART : 1) unless defined $n_parts;

my $MODEL_TYPE = 'cd';
my $modeldir   = "$ST::CFG_BASE_DIR/model_parameters";

$| = 1; # Turn on autoflushing

my $logdir         = "${ST::CFG_LOG_DIR}/65.mmie_train";
my $modelname      = "${ST::CFG_EXPTNAME}.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}";
my $mmie_modelname = "${ST::CFG_EXPTNAME}.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}_mmie";

my $return_value = 0;
if ($iter == 1) {
    # Clean up junk from earlier runs
    Log ("MODULE: 65 MMIE Training\n");

    if ($ST::CFG_MMIE ne "yes") {
        Log("Skipped:  \$ST::CFG_MMIE set to \'$ST::CFG_MMIE\' in sphinx_train.cfg\n");
        exit(0);
    }

    # mmie training doesn't work for semi-continuous model
    if ($ST::CFG_HMM_TYPE eq ".semi.") {
	Log("Skipped:  \$CFG_DIRLABEL = .semi. \n");
	Log("MMIE training only works on the continuous model\n");
	exit(0);
    }

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

    my $mmie_hmmdir = "$modeldir/$mmie_modelname";
    rmtree ($mmie_hmmdir, 0, 1);
    mkdir ($mmie_hmmdir,0777)
	or die "Could not make directory: $mmie_hmmdir";

    Log("Phase 2: Forward-Backward on Lattices");
}

# make new mmie direcotry and copy model files
my $init_hmm_dir;
my $previter = $iter - 1;
if ($iter == 1) {
    $init_hmm_dir  = "$modeldir/$modelname";
} else {
    $init_hmm_dir  = "$modeldir/$mmie_modelname/iter_$previter";
}
my $hmm_dir = "$modeldir/$mmie_modelname/iter_$iter";
rmtree ($hmm_dir, 0, 1);
mkdir ($hmm_dir, 0777)
    or die "Could not make directory: $hmm_dir";

my $mdeffile    = "$init_hmm_dir/mdef";
my $fillerdict  = "$init_hmm_dir/noisedict";
my $featpara    = "$init_hmm_dir/feat.params";
my $meanfn      = "$init_hmm_dir/means";
my $varfn       = "$init_hmm_dir/variances";
my $tmatfn      = "$init_hmm_dir/transition_matrices";
my $mixwfn      = "$init_hmm_dir/mixture_weights";
my $feattransfn = "$init_hmm_dir/feature_transform";

copy($mdeffile, catfile($hmm_dir, 'mdef'))
    or die "Failed to copy $mdeffile to $hmm_dir/mdef: $!";
copy($fillerdict, catfile($hmm_dir, 'noisedict'))
    or die "Failed to copy $fillerdict to $hmm_dir/noisedict: $!";
copy($featpara, catfile($hmm_dir, 'feat.params'))
    or die "Failed to copy $featpara to $hmm_dir/feat.params: $!";
copy($meanfn, catfile($hmm_dir, 'means'))
    or die "Failed to copy $meanfn to $hmm_dir/means: $!";
copy($varfn, catfile($hmm_dir, 'variances'))
    or die "Failed to copy $varfn to $hmm_dir/variances: $!";
copy($tmatfn, catfile($hmm_dir, 'transition_matrices'))
    or die "Failed to copy $tmatfn to $hmm_dir/transition_matrices: $!";
copy($mixwfn, catfile($hmm_dir, 'mixture_weights'))
    or die "Failed to copy $mixwfn to $hmm_dir/mixture_weights: $!";
if ($ST::CFG_LDA_MLLT eq 'yes') {
    copy($feattransfn, catfile($hmm_dir, 'feature_transform'))
	or die "Failed to copy $feattransfn to $hmm_dir/feature_transform: $!";
}

# Call baum_welch with iter part and n_parts,
# first run on the numerator lattices and then
# on the denominator lattices,
# once done call norm_and_lauchbw.pl

# run on the numerator lattices
my @deps;
for (my $i=1; $i<=$n_parts; $i++)
{
    push @deps, LaunchScript("bw.num.$iter.$i", ['baum_welch.pl', $iter, $i, $n_parts, "numlat"]);
}

# wait for the bw run on the numerator lattices done
waitnumlatdone();

# run on the denominator lattices
for (my $i=1; $i<=$n_parts; $i++)
{
    push @deps, LaunchScript("bw.den.$iter.$i", ['baum_welch.pl', $iter, $i, $n_parts, "denlat"]);
}

LaunchScript("norm.$iter", ['norm_and_launchbw.pl', $iter, $n_parts], \@deps);

# For the first iteration (i.e. the one that was called from the
# command line or a parent script), wait until completion or error
my $return_value = 0;
if ($iter == 1) {
    $return_value = WaitForMMIEConverge($logdir);
}
exit $return_value;

sub waitnumlatdone ()
{
    my $n_done = 0;
    my @num_done;

    for(my $i=1; $i<=$n_parts; $i++){

	my $job = shift @deps;
	WaitForScript($job);

	$num_done[$i] = 0;
	my $input_log = "${logdir}/${ST::CFG_EXPTNAME}.${iter}-${i}.bw.numlat.log";
	next if (! -s $input_log);
	open LOG,$input_log;
	while (<LOG>) {
	    if (/.*(Counts saved to).*/) {
		$n_done++;
		$num_done[$i] = 1;
	    }
	}
	close LOG;
    }
    
    if ($n_done != $n_parts) {
	Log("Only $n_done parts of $n_parts of Baum Welch on numerator lattices were successfully completed\n");
	Log("Parts ");
	for (my $i=1;$i<=$n_parts;$i++) {
	    Log("numlat:$i ") if ($num_done[$i] == 0);
	}
	Log("failed to run!\n");
	exit 1;
    }
}
