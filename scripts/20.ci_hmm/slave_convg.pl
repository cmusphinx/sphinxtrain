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

my ($iter, $n_parts, $n_gau) = @ARGV;
$iter = 1 unless defined $iter;
$n_parts = (defined($ST::CFG_NPART) ? $ST::CFG_NPART : 1) unless defined $n_parts;
$n_gau = $ST::CFG_INITIAL_NUM_DENSITIES unless defined $n_gau;

my $modeldir  = "$ST::CFG_BASE_DIR/model_parameters";
mkdir ($modeldir,0777);

$| = 1; # Turn on autoflushing
my $logdir = "$ST::CFG_LOG_DIR/20.ci_hmm";
my $return_value = 0;

use vars qw($MLLT_FILE);
$MLLT_FILE = catfile($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.mllt");

# We have to clean up and run flat initialize if it is the first iteration
if ($iter == 1 and $n_gau == $ST::CFG_INITIAL_NUM_DENSITIES) {
    Log("MODULE: 20 Training Context Independent models\n");
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
    LogProgress("qmanager...");
    rmtree ($ST::CFG_QMGR_DIR, 0, 1);
    mkdir ($ST::CFG_QMGR_DIR,0777);
    LogProgress("models...\n");
    rmtree("$modeldir/${ST::CFG_EXPTNAME}.ci_$ST::CFG_DIRLABEL", 0, 1);
    LogStatus('completed');

    # If we previously force aligned with single-Gaussian models, use
    # them for initialization to save some time.
    if (($ST::CFG_FORCEDALIGN eq 'yes' or $ST::CFG_VTLN eq 'yes')
	and -e catfile($ST::CFG_FORCE_ALIGN_MODELDIR, 'means')
	and ($ST::CFG_FALIGN_CI_MGAU ne 'yes' or $ST::CFG_HMM_TYPE ne '.cont.')
	) {
	$return_value = CopyInitialize();
	exit ($return_value) if ($return_value);
    }
    else {
	# For the first iteration Flat initialize models.
	$return_value = FlatInitialize();
	exit ($return_value) if ($return_value);
    }
    Log("Phase 3: Forward-Backward");
}

if (defined($ST::CFG_PHSEG_DIR) and ! -d $ST::CFG_PHSEG_DIR) {
    # Build phone segmentation dirs
    open INPUT,"${ST::CFG_LISTOFFILES}" or die "Failed to open $ST::CFG_LISTOFFILES: $!";
    my %dirs;
    while (<INPUT>) {
	chomp;
	my @fields = split;
	my $uttid = pop @fields;
	my $basedir = dirname($uttid);
	next if $basedir eq ".";
	unless ($dirs{$basedir}) {
	    $dirs{$basedir}++;
	    mkpath(catdir($ST::CFG_PHSEG_DIR, $basedir), 0, 0777);
	}
    }
    close INPUT;
}


# Call baum_welch with iter part and n_parts,
# once done call norm_and_lauchbw.pl
my @deps;
for (my $i=1; $i<=$n_parts; $i++)
{
    push @deps, LaunchScript("bw.$iter.$i", ['baum_welch.pl', $iter, $i, $n_parts, $n_gau])
}
LaunchScript("norm.$iter", ['norm_and_launchbw.pl', $iter, $n_parts, $n_gau], \@deps);
# For the first iteration (i.e. the one that was called from the
# command line or a parent script), wait until completion or error
if ($iter == 1 && $n_gau == $ST::CFG_INITIAL_NUM_DENSITIES) {
    if ($ST::CFG_CI_MGAU eq 'yes') {
	$return_value = TiedWaitForConvergence($logdir);
    }
    else {
	$return_value = WaitForConvergence($logdir);
    }
}
exit $return_value;

sub CopyInitialize {
    Log("Phase 2: Copy initialize from falign model\n");
    my $hmmdir = catdir($ST::CFG_BASE_DIR, "model_parameters");
    my $outhmm = catdir($hmmdir, "${ST::CFG_EXPTNAME}.ci_${ST::CFG_DIRLABEL}_flatinitial");
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

    my $logdir              = "$ST::CFG_LOG_DIR/20.ci_hmm";
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
    mkdir ($outhmm,0777);

    my $FLAT = "$ST::CFG_BIN_DIR/mk_flat";

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.makeflat_cihmm.log";
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

    # For semi-continuous, PTM, or generalized tied mixtures,
    # duplicate the codebook in-place to produce the initial model
    # (this does nothing for SC but is necessary for the others)
    if ($ST::CFG_HMM_TYPE ne ".cont.") {
        $logfile = "$logdir/${ST::CFG_EXPTNAME}.duplicate_codebook.log";
	if ($return_value = RunTool('init_mixw', $logfile, 0,
				    # Flat K-means init always gives us one codebook
				    -src_ts2cbfn => '.semi.',
				    # There might be multiple codebooks here though
				    -dest_ts2cbfn => $ST::CFG_HMM_TYPE,
    				    -fullvar => $ST::CFG_FULLVAR,
				    -src_moddeffn => $ci_mdeffile,
				    -dest_moddeffn => $ci_mdeffile,
				    -src_mixwfn => catfile($outhmm, 'mixture_weights'),
				    -dest_mixwfn => catfile($outhmm, 'mixture_weights'),
				    -src_tmatfn => catfile($outhmm, 'transition_matrices'),
				    -dest_tmatfn => catfile($outhmm, 'transition_matrices'),
				    -src_meanfn=> catfile($outhmm, 'means'),
				    -dest_meanfn => catfile($outhmm, 'means'),
				    -src_varfn=> catfile($outhmm, 'variances'),
				    -dest_varfn => catfile($outhmm, 'variances'),
	    )) {
	    return $return_value;
	}
      return (0);
    }

    #-------------------------------------------------------------------------
    # Accumulate the means from the training data
    #------------------------------------------------------------------------

    $logfile = "$logdir/${ST::CFG_EXPTNAME}.initmean_cihmm.log";

    open LOG,">$logfile";

    my $output_buffer_dir = "$ST::CFG_BWACCUM_DIR/${ST::CFG_EXPTNAME}_buff_1";
    mkdir ($output_buffer_dir,0777);

    # if there is an MLLT transformation, use it
    my @feat_args;
    if (defined($ST::CFG_SVSPEC)) {
	push(@feat_args, -svspec =>$ST::CFG_SVSPEC);
    }
    if (-r $MLLT_FILE) {
	push(@feat_args,
	     -lda => $MLLT_FILE,
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
				@feat_args
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
				@feat_args
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
