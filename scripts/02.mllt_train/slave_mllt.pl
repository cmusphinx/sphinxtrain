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

# LDA/MLLT doesn't really have sense with multistream
if ($ST::CFG_NUM_STREAMS != 1) {
    Log("MODULE: 02 Train MLLT transformation\n");
    Log("Skipped for multistream setup, see CFG_NUM_STREAMS configuration\n");
    Log("LDA/MLLT only has sense for single stream features");
    Log("Skipping MLLT training");
    exit 0;
}

if ($ST::CFG_LDA_MLLT ne 'yes') {
    Log("MODULE: 02 Train MLLT transformation\n");
    Log("Skipped (set \$CFG_LDA_MLLT = 'yes' to enable)\n");
    exit 0;
}

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

# This script gets run after LDA.  We will flat initialize and train a CI model.
$| = 1; # Turn on autoflushing
my $logdir = "$ST::CFG_LOG_DIR/02.mllt_train";

if ($iter == 1) {
    Log("MODULE: 02 Train MLLT transformation\n");
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

    # Now flat initialize
    my $return_value = FlatInitialize();
    exit $return_value if $return_value;
    Log("Phase 3: Forward-Backward");
}

if ($iter eq 'N') {
    Log("Phase 4: MLLT transform estimation");
    my @deps;
    for (my $i=1; $i<=$n_parts; $i++) {
	push @deps, LaunchScript("bw.mllt.$iter.$i", ['baum_welch.pl', $iter, $i, $n_parts, 'yes']);
    }
    LaunchScript("mllt", "mllt_train.pl", \@deps);
}
else {
    my @deps;
    for (my $i=1; $i<=$n_parts; $i++) {
	push @deps, LaunchScript("bw.mllt.$iter.$i", ['baum_welch.pl', $iter, $i, $n_parts, 'no']);
    }
    LaunchScript("norm.$iter", ['norm_and_launchbw.pl', $iter, $n_parts], \@deps);
    # On the first iteration, wait for the MLLT stuff to complete
    my $mllt_log = File::Spec->catfile($logdir, "$ST::CFG_EXPTNAME.mllt_train.log");
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
	    if (open LOG, "<$mllt_log") {
		while (<LOG>) {
		    if (/failed/) {
			LogError("MLLT Training failed");
			exit 1;
		    }
		    elsif (/complete/) {
			Log("MLLT Training completed", 'result');
			exit 0;
		    }
		}
	    }
	    sleep $interval;
	}
    }
}

sub FlatInitialize
{
    Log("Phase 2: Flat initialize\n");

    #**************************************************************************
    # this script given an mdef file and a  codebook (means/vars in S3 format)
    # produces flat mixture weights in a semicontinuos setting. From the models
    # produced by this script we can restart normal baum-welch training
    # Flat init might not be the best choice, specially if we have access to
    # segmentation files for the whole training database.
    #**************************************************************************

    my $logdir              = "$ST::CFG_LOG_DIR/02.mllt_train";
    my $modarchdir          = "$ST::CFG_BASE_DIR/model_architecture";
    my $hmmdir              = "$ST::CFG_BASE_DIR/model_parameters";
    mkdir ($logdir,0777);
    mkdir ($modarchdir,0777);
    mkdir ($hmmdir,0777);

    my $phonefile = "$modarchdir/$ST::CFG_EXPTNAME.phonelist";
    open PHONEFILE, "<".$phonefile;
    my $NUM_PHONES = 0;
    while (defined(my $line = <PHONEFILE>)) {
	$NUM_PHONES++;
    }
    close PHONEFILE;

    #-------------------------------------------------------------------------
    # make the flat models using the above topology file and the mdef file
    #------------------------------------------------------------------------

    my $outhmm = "$hmmdir/${ST::CFG_EXPTNAME}.ci_mllt_flatinitial";
    mkdir ($outhmm,0777);

    my $topologyfile = "$modarchdir/$ST::CFG_EXPTNAME.topology";
    my $mdeffile = "$ST::CFG_BASE_DIR/model_architecture/${ST::CFG_EXPTNAME}.ci.mdef";
    my $logfile = "$logdir/${ST::CFG_EXPTNAME}.makeflat_cihmm.log";
    my $return_value = 0;
    if ($return_value = RunTool('mk_flat', $logfile, 0,
				-moddeffn => $mdeffile,
				-topo => $topologyfile,
				-mixwfn => catfile($outhmm, 'mixture_weights'),
				-tmatfn => catfile($outhmm, 'transition_matrices'),
				-nstream => 1, # Always one stream!
				-ndensity => 1 # Always one density!
			       )) {
	return $return_value;
    }

    #-------------------------------------------------------------------------
    # Accumulate the means from the training data
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.initmean_cihmm.log";

    open LOG,">$logfile";

    my $output_buffer_dir = "$ST::CFG_BWACCUM_DIR/${ST::CFG_EXPTNAME}_buff_1";
    mkdir ($output_buffer_dir,0777);
    my $ldafile = catfile($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.lda");
    if ($return_value = RunTool('init_gau', $logfile, 0,
				-ctlfn => $ST::CFG_LISTOFFILES,
				-part => 1, -npart => 1,
				-cepdir => $ST::CFG_FEATFILES_DIR,
				-cepext => $ST::CFG_FEATFILE_EXTENSION,
				-accumdir => $output_buffer_dir,
				-agc => $ST::CFG_AGC,
				-cmn => $ST::CFG_CMN,
				-varnorm => $ST::CFG_VARNORM,
				-feat => $ST::CFG_FEATURE,
				-ceplen => $ST::CFG_VECTOR_LENGTH,
				-lda => $ldafile,
				-ldadim => $ST::CFG_LDA_DIMENSION,
			       )) {
	return $return_value;
    }

    #-------------------------------------------------------------------------
    # Normalize the means
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.normmean_cihmm.log";
    if ($return_value = RunTool('norm', $logfile, 0,
				-accumdir => $output_buffer_dir,
				-meanfn => catfile($outhmm, "globalmean"),
			       )) {
	return $return_value;
    }

    #-------------------------------------------------------------------------
    # Accumulate the variances from the training data
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.initvar_cihmm.log";
    if ($return_value = RunTool('init_gau', $logfile, 0,
				-meanfn => catfile($outhmm, "globalmean"),
				-ctlfn => $ST::CFG_LISTOFFILES,
				-part => 1,  -npart => 1,
				-cepdir => $ST::CFG_FEATFILES_DIR,
				-cepext => $ST::CFG_FEATFILE_EXTENSION,
				-accumdir => $output_buffer_dir,
				-agc => $ST::CFG_AGC,
				-cmn => $ST::CFG_CMN,
				-varnorm => $ST::CFG_VARNORM,
				-feat => $ST::CFG_FEATURE,
				-ceplen => $ST::CFG_VECTOR_LENGTH,
				-lda => $ldafile,
				-ldadim => $ST::CFG_LDA_DIMENSION,
			       )) {
	return $return_value;
    }

    #-------------------------------------------------------------------------
    # Normalize the variances
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.normvar_cihmm.log";
    if ($return_value = RunTool('norm', $logfile, 0,
				-accumdir => $output_buffer_dir,
				-varfn => catfile($outhmm, "globalvar"),
			       )) {
	return $return_value;
    }


    #-------------------------------------------------------------------------
    # Create the copy operation file, simply a map between states
    #------------------------------------------------------------------------

    my $NUM_STATES = $NUM_PHONES * $ST::CFG_STATESPERHMM;
    if (open CPFILE, ">$ST::CFG_CP_OPERATION") {
      for (my $STATE = 0; $STATE < $NUM_STATES; $STATE++) {
	print CPFILE "$STATE\t0\n";
      }
      close(CPFILE);
    } else {
      warn "Can't open $ST::CFG_CP_OPERATION\n";
    }

    #-------------------------------------------------------------------------
    # Copy the means to all other states
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.cpmean_cihmm.log";

    if ($return_value = RunTool('cp_parm', $logfile, 0,
				-cpopsfn => $ST::CFG_CP_OPERATION,
				-igaufn => catfile($outhmm, 'globalmean'),
				-ncbout => $NUM_STATES,
				-ogaufn => catfile($outhmm, 'means')
			       )) {
      return $return_value;
    }

    #-------------------------------------------------------------------------
    # Copy the variances to all other states
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.cpvar_cihmm.log";
    if ($return_value = RunTool('cp_parm', $logfile, 0,
				-cpopsfn => $ST::CFG_CP_OPERATION,
				-ncbout => $NUM_STATES,
				-igaufn => catfile($outhmm, 'globalvar'),
				-ogaufn => catfile($outhmm, 'variances'),
			       )) {
	return $return_value;
    }

    unlink $ST::CFG_CP_OPERATION;

    return $return_value;
}
