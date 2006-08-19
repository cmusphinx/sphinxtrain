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

my $modeldir  = "$ST::CFG_BASE_DIR/model_parameters";
mkdir ($modeldir,0777) unless -d $modeldir;

$| = 1; # Turn on autoflushing
my $logdir = "$ST::CFG_LOG_DIR/02.ci_schmm";
my $return_value = 0;

# We have to clean up and run flat initialize if it is the first iteration
if ($iter == 1) {
    Log ("MODULE: 02 Training Context Independent models\n");
    Log ("    Cleaning up directories: accumulator...");
    rmtree($ST::CFG_BWACCUM_DIR, 0, 1);
    mkdir ($ST::CFG_BWACCUM_DIR,0777);
    Log ("logs...");
    rmtree($logdir, 0, 1);
    mkdir ($logdir,0777);
    Log ("qmanager...");
    rmtree ($ST::CFG_QMGR_DIR, 0, 1);
    mkdir ($ST::CFG_QMGR_DIR,0777);
    Log ("models...\n");
    rmtree("$modeldir/${ST::CFG_EXPTNAME}.ci_$ST::CFG_DIRLABEL", 0, 1);

    # For the first iteration Flat initialize models.
    $return_value = FlatInitialize();
    exit ($return_value) if ($return_value);
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

sub FlatInitialize ()
{
    Log("    Flat initialize\n");

    #**************************************************************************
    # this script given an mdef file and a  codebook (means/vars in S3 format)
    # produces flat mixture weights in a semicontinuos setting. From the models
    # produced by this script we can restart normal baum-welch training
    # Flat init might not be the best choice, specially if we have access to
    # segmentation files for the whole training database.
    #**************************************************************************

    my $logdir              = "$ST::CFG_LOG_DIR/02.ci_schmm";
    my $modarchdir          = "$ST::CFG_BASE_DIR/model_architecture";
    my $hmmdir              = "$ST::CFG_BASE_DIR/model_parameters";
    mkdir ($logdir,0777) unless -d $logdir;
    mkdir ($modarchdir,0777) unless -d $modarchdir;
    mkdir ($hmmdir,0777) unless -d $hmmdir;
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
    while (defined(my $line = <PHONELIST>)) {
      chomp($line);
      next if $line =~ m/^\s*$/;
      $line =~ s/$/ - - - /;
      print PHONEFILE $line . "\n";
      $NUM_PHONES++;
    }

#    system "sed 's+\$+ - - - +g' $ST::CFG_RAWPHONEFILE > $phonefile";


    my $logfile = "$logdir/${ST::CFG_EXPTNAME}.make_ci_mdef_fromphonelist.log";
    HTML_Print ("\t\tmk_mdef_gen " . FormatURL("$logfile", "Log File") . " ");
    #-------------------------------------------------------------------------
    # Decide on what topology to use for the hmms: 3 state, 5 state, blah state
    # or what, give it to the variable "statesperhmm" and use it to create
    # the topology matrix in the topology file
    #-------------------------------------------------------------------------

    #$statesperhmm obtained from variables.def
    my $topologyfile             = "$modarchdir/$ST::CFG_EXPTNAME.topology";
    RunTool('maketopology.pl', $logfile, 0,
	    $ST::CFG_STATESPERHMM, $ST::CFG_SKIPSTATE, $topologyfile);

    if ($return_value = RunTool('mk_mdef_gen', $logfile, 0,
				-phnlstfn => $phonefile,
				-ocimdef => $ci_mdeffile,
				-n_state_pm => $ST::CFG_STATESPERHMM)) {
      return $return_value;
    }

    #-------------------------------------------------------------------------
    # make the flat models using the above topology file and the mdef file
    #------------------------------------------------------------------------
    my $outhmm               = "$hmmdir/${ST::CFG_EXPTNAME}.ci_${ST::CFG_DIRLABEL}_flatinitial";
    mkdir ($outhmm,0777) unless -d $outhmm;

    my $FLAT = "$ST::CFG_BIN_DIR/mk_flat";

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.makeflat_cihmm.log";
    HTML_Print ("\t\tmk_flat " . FormatURL("$logfile", "Log File") . " ");

    if ($return_value = RunTool('mk_flat', $logfile, 0,
				-moddeffn => $ci_mdeffile,
				-topo => $topologyfile,
				-mixwfn => catfile($outhmm, 'mixture_weights'),
				-tmatfn => catfile($outhmm, 'transition_matrices'),
				-nstream => $ST::CFG_NUM_STREAMS,
				-ndensity => $ST::CFG_INITIAL_NUM_DENSITIES,
			       )) {
      return $return_value;
    }

    if ($ST::CFG_HMM_TYPE eq ".semi.") {
      return (0);
    }

    #-------------------------------------------------------------------------
    # Accumulate the means from the training data
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.initmean_cihmm.log";
    HTML_Print ("\t\taccum_mean " . FormatURL("$logfile", "Log File") . " ");

    open LOG,">$logfile";

    my $output_buffer_dir = "$ST::CFG_BASE_DIR/bwaccumdir/${ST::CFG_EXPTNAME}_buff_1";
    mkdir ($output_buffer_dir,0777) unless -d $output_buffer_dir;

    # if there is an LDA transformation, use it
    my @lda_args;
    if (defined($ST::CFG_LDA_TRANSFORM) and -r $ST::CFG_LDA_TRANSFORM) {
	push(@lda_args,
	     -ldafn => $ST::CFG_LDA_TRANSFORM,
	     -ldadim => $ST::CFG_LDA_DIMENSION);
    }
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
				@lda_args
			       )) {
      return $return_value;
    }

    #-------------------------------------------------------------------------
    # Normalize the means
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.normmean_cihmm.log";
    HTML_Print ("\t\tnorm_mean " . FormatURL("$logfile", "Log File") . " ");

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
    HTML_Print ("\t\taccum_var " . FormatURL("$logfile", "Log File") . " ");

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
				@lda_args
			       )) {
      return $return_value;
    }

    #-------------------------------------------------------------------------
    # Normalize the variances
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.normvar_cihmm.log";
    HTML_Print ("\t\tnorm_var " . FormatURL("$logfile", "Log File") . " ");

    if ($return_value = RunTool('norm', $logfile, 0,
				-accumdir => $output_buffer_dir,
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
    HTML_Print ("\t\tcp_mean " . FormatURL("$logfile", "Log File") . " ");

    my @feat;
    if (@lda_args) {
	@feat = (-feat => '1s_c', -ceplen => $ST::CFG_LDA_DIMENSION);
    }
    else {
	@feat = (-feat => $ST::CFG_FEATURE, -ceplen => $ST::CFG_VECTOR_LENGTH);
    }
    if ($return_value = RunTool('cp_parm', $logfile, 0,
				-cpopsfn => $ST::CFG_CP_OPERATION,
				-igaufn => catfile($outhmm, 'globalmean'),
				-ncbout => $NUM_CI_STATES,
				-ogaufn => catfile($outhmm, 'means'),
				@feat
			       )) {
      return $return_value;
    }

    #-------------------------------------------------------------------------
    # Copy the variances to all other states
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.cpvar_cihmm.log";
    HTML_Print ("\t\tcp_var " . FormatURL("$logfile", "Log File") . " ");

    if ($return_value = RunTool('cp_parm', $logfile, 0,
				-cpopsfn => $ST::CFG_CP_OPERATION,
				-igaufn => catfile($outhmm, 'globalvar'),
				-ncbout => $NUM_CI_STATES,
				-ogaufn => catfile($outhmm, 'variances'),
				@feat
			       )) {
      return $return_value;
    }

    unlink $ST::CFG_CP_OPERATION;
    return $return_value;
}
