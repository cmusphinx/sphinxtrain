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
    Log("MODULE: 01 Train LDA transformation\n");
    Log("Skipped for multistream setup, see CFG_NUM_STREAMS configuration\n");
    Log("LDA/MLLT only has sense for single stream features");
    Log("Skipping LDA training");
    exit 0;
}

if ($ST::CFG_LDA_MLLT ne 'yes') {
    Log("MODULE: 01 Train LDA transformation\n");
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

$| = 1; # Turn on autoflushing
my $logdir = "$ST::CFG_LOG_DIR/01.lda_train";

if ($iter == 1) {
    Log("MODULE: 01 Train LDA transformation\n");
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
    my $return_value = FlatInitialize();
    exit ($return_value) if ($return_value);

    Log("Phase 3: Forward-Backward");
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

sub CopyInitialize {
    Log("Phase 2: Copy initialize from falign model\n");
    my $hmmdir = catdir($ST::CFG_BASE_DIR, "model_parameters");
    my $outhmm = catdir($hmmdir, "${ST::CFG_EXPTNAME}.ci_lda_flatinitial");
    my $modarchdir = catdir($ST::CFG_BASE_DIR, "model_architecture");
    mkdir ($outhmm,0777);
    foreach (qw(means variances mixture_weights transition_matrices)) {
	copy(catfile($ST::CFG_FORCE_ALIGN_MODELDIR, $_), catfile($outhmm, $_))
	    or return -1;
    }
    my $ci_mdeffile = catfile($modarchdir, "$ST::CFG_EXPTNAME.falign_ci.mdef");
    my $out_mdeffile = catfile($modarchdir, "$ST::CFG_EXPTNAME.ci.mdef");
    copy($ci_mdeffile, $out_mdeffile);
    return 0;
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

    my $logdir              = "$ST::CFG_LOG_DIR/01.lda_train";
    my $modarchdir          = "$ST::CFG_BASE_DIR/model_architecture";
    my $hmmdir              = "$ST::CFG_BASE_DIR/model_parameters";
    mkdir ($logdir,0777);
    mkdir ($modarchdir,0777);
    mkdir ($hmmdir,0777);
    my $cmd ="";

    #-------------------------------------------------------------------------
    # Take the phone list. Put three hyphens after every phone. That is the
    # required format. Then make a ci model definition file from it using the
    # following program.
    #-------------------------------------------------------------------------

    #$rawphonefile obtained from variables.def
    my $phonefile           = "$modarchdir/$ST::CFG_EXPTNAME.phonelist";
    my $ci_mdeffile         = "$modarchdir/$ST::CFG_EXPTNAME.ci.mdef";

    open PHONELIST, "<".$ST::CFG_RAWPHONEFILE;
    open PHONEFILE, ">".$phonefile;
    my $NUM_PHONES = 0;
    my @phonelist;
    while (defined(my $line = <PHONELIST>)) {
      $line = Trim($line);
      next if $line =~ m/^\s*$/;
      $line =~ s/$/ - - - /;
      push @phonelist, $line;
      $NUM_PHONES++;
    }

    # Make sure the CI phones are sorted, as PocketSphinx requires them to be
    foreach (sort @phonelist) {
	print PHONEFILE $_, "\n";
    }
#    system "sed 's+\$+ - - - +g' $ST::CFG_RAWPHONEFILE > $phonefile";


    my $logfile = "$logdir/${ST::CFG_EXPTNAME}.make_ci_mdef_fromphonelist.log";
    #-------------------------------------------------------------------------
    # Decide on what topology to use for the hmms: 3 state, 5 state, blah state
    # or what, give it to the variable "statesperhmm" and use it to create
    # the topology matrix in the topology file
    #-------------------------------------------------------------------------

    #$statesperhmm obtained from variables.def
    my $topologyfile             = "$modarchdir/$ST::CFG_EXPTNAME.topology";
    RunScript("../prepare/maketopology.pl", $topologyfile);

    my $return_value;
    if ($return_value = RunTool('mk_mdef_gen', $logfile, 0,
				-phnlstfn => $phonefile,
				-ocimdef => $ci_mdeffile,
				-n_state_pm => $ST::CFG_STATESPERHMM)) {
      return $return_value;
    }

    #-------------------------------------------------------------------------
    # make the flat models using the above topology file and the mdef file
    #------------------------------------------------------------------------
    my $outhmm               = "$hmmdir/${ST::CFG_EXPTNAME}.ci_lda_flatinitial";
    mkdir ($outhmm,0777);

    my $FLAT = "$ST::CFG_BIN_DIR/mk_flat";

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.makeflat_cihmm.log";
    if ($return_value = RunTool('mk_flat', $logfile, 0,
				-moddeffn => $ci_mdeffile,
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
				-fullvar => $ST::CFG_FULLVAR,
			       )) {
      return $return_value;
    }

    #-------------------------------------------------------------------------
    # Normalize the variances
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.normvar_cihmm.log";

    if ($return_value = RunTool('norm', $logfile, 0,
				-accumdir => $output_buffer_dir,
				-fullvar => $ST::CFG_FULLVAR,
				-varfn => catfile($outhmm, "globalvar"),
			       )) {
      return $return_value;
    }


    #-------------------------------------------------------------------------
    # Create the copy operation file, simply a map between states
    #------------------------------------------------------------------------

    my $NUM_CI_STATES = $NUM_PHONES * $ST::CFG_STATESPERHMM;
    if (open CPFILE, ">$ST::CFG_CP_OPERATION") {
      for (my $CI_STATE = 0; $CI_STATE < $NUM_CI_STATES; $CI_STATE++) {
	print CPFILE "$CI_STATE\t0\n";
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
				-ncbout => $NUM_CI_STATES,
				-ogaufn => catfile($outhmm, 'means')
			       )) {
      return $return_value;
    }

    #-------------------------------------------------------------------------
    # Copy the variances to all other states
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.cpvar_cihmm.log";

    my @varcpy;
    if ($ST::CFG_FULLVAR eq 'yes') {
	@varcpy = (-ifullgaufn => catfile($outhmm, 'globalvar'),
		   -ofullgaufn => catfile($outhmm, 'variances'));
    }
    else {
	@varcpy = (-igaufn => catfile($outhmm, 'globalvar'),
		   -ogaufn => catfile($outhmm, 'variances'));
    }
    if ($return_value = RunTool('cp_parm', $logfile, 0,
				-cpopsfn => $ST::CFG_CP_OPERATION,
				-ncbout => $NUM_CI_STATES,
				@varcpy
			       )) {
	return $return_value;
    }

    unlink $ST::CFG_CP_OPERATION;

    return $return_value;
}
