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

use File::Path;

my $index = 0;
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    $index = 2;
} else {
    $cfg_file = "etc/sphinx_train.cfg";
}

if (! -s "$cfg_file") {
    print ("unable to find default configuration file, use -cfg file.cfg or create etc/sphinx_train.cfg for default\n");
    exit -3;
}
require $cfg_file;

#***************************************************************************
# This script launches all the ci - continuous training jobs in the proper
# order. First it cleans up the directories, then launches the 
# flat initialization, and the baum-welch and norm jobs for the required
# number of iterations. Within each iteration it launches as many baumwelch
# jobs as the number of parts we wish to split the training into.
#***************************************************************************

my $iter = 1;
if (($#ARGV >= ($index))) {
   $iter= $ARGV[$index];
}

my $n_parts = ($CFG_NPART) ? $CFG_NPART : 1;

# If the number of parts is given as command line argument, overwrite
# the number coming from the config file
if (($#ARGV >= ($index+1))) {
   $n_parts= $ARGV[$index+1];
}
# as the next stage (deleted interpolation) requires at least 2 parts
# we set the default number of parts to be 2, but only if we're using
# semi continuous models
if (($n_parts < 2) and ($CFG_HMM_TYPE eq ".semi.")) {
    $n_parts = 2;
}

$| = 1; # Turn on autoflushing
my $scriptdir = "$CFG_SCRIPT_DIR/07.cd-schmm";
my $logdir = "$CFG_LOG_DIR/07.cd-schmm";
mkdir ("$logdir",0777) unless -d $logdir;

my $modeldir  = "$CFG_BASE_DIR/model_parameters";
mkdir ($modeldir,0777) unless -d $modeldir;

# We have to clean up and run flat initialize if it is the first iteration
if ($iter == 1) {
    
    &ST_Log ("MODULE: 07 Training Context dependent models\n");
    &ST_Log ("    Cleaning up directories: accumulator...");
    rmtree ($CFG_BWACCUM_DIR) unless ! -d $CFG_BWACCUM_DIR;
    mkdir ($CFG_BWACCUM_DIR,0777);
    &ST_Log ("logs...\n");
    rmtree ($logdir) unless ! -d $logdir;
    mkdir ($logdir,0777);
    &copyci2cd2initialize();
}

if ($MC && $n_parts > 1)
{
    # multi-processor version -- assumes ssh machine works
    for ($i=1; $i<=$n_parts; $i++)
    {
        $job_command = "$scriptdir/baum_welch.pl -cfg $cfg_file $iter $i $n_parts";
	open rrr,"scripts_pl/mc/mc_run.pl $job_command |";
#	print $job_command."\n";
	while ($line = <rrr>)
	{
	    chomp($line);
#	    print "mc_run: ".$line."\n";
	    @fff=split(/\s+/,$line);
            if ($fff[0] eq "MC")
            {
  	       $job_name = $fff[1];
	       last;
	    }
#            print "waiting for mc_run to say something\n";
	    sleep 3;
	}
        close rrr;
	if ($job_name eq "no_job")
	{
	    print "waiting for machine for part $i (of $n_parts) of iteration $iter\n";
	    sleep 30;
	    $i = $i-1;
	}
	else
	{
	    print "running part $i (of $n_parts) of iteration $iter on $job_name \n";
	    $parts[$i] = $job_name;
	}
    }
    # Wait for them all to finish
    $jobs_still_todo = 1;
    while ($jobs_still_todo)
    {
	$jobs_still_todo = 0;
	for ($i=1; $i<=$n_parts; $i++)
	{
	    if ( -f $parts[$i] )
	    {
		print "waiting for part $i (of $n_parts) of iteration $iter on ".$parts[$i]."\n";
		$jobs_still_todo = 1;
	    }
	}
	sleep 30;
    }
    print "iteration $iter: $n_parts completed\n";
}
else
{
    # Call baum_welch with iter part and n_part, 
    # once done call norm_and_lauchbw.pl
    for ($i=1; $i<=$n_parts; $i++)
    {
	system ("$scriptdir/baum_welch.pl -cfg $cfg_file $iter $i $n_parts");
    }
}
system ("$scriptdir/norm_and_launchbw.pl -cfg $cfg_file $iter $n_parts");

exit 0;

sub copyci2cd2initialize ()
{
    &ST_Log ("    Copy CI to CD initialize\n");
    
    #**************************************************************************
    # this script copies the mixw/mean/var/tmat from a ci (continuous) HMM
    # to a cd (continuous) HMM. We need two mdef files, one without triphones
    # for the ci hmm definition and another one with triphones for the cd
    # triphone definition.
    # our final goal is to train in stages, first the ci models and then
    # with cd models. This should minimize initialization problems..
     #*************************************************************************

    my $ci_hmmdir = "$CFG_BASE_DIR/model_parameters/${CFG_EXPTNAME}.ci_${CFG_DIRLABEL}";
    my $src_moddeffn = "$CFG_BASE_DIR/model_architecture/${CFG_EXPTNAME}.ci.mdef";
    my $src_mixwfn = "$ci_hmmdir/mixture_weights";
    my $src_meanfn = "$ci_hmmdir/means";
    my $src_varfn = "$ci_hmmdir/variances";
    my $src_tmatfn = "$ci_hmmdir/transition_matrices";

    my $cd_hmmdir = "$CFG_BASE_DIR/model_parameters/${CFG_EXPTNAME}.cd_${CFG_DIRLABEL}_initial";
    mkdir ($cd_hmmdir,0777) unless -d $cd_hmmdir;

    my $dest_moddeffn = "$CFG_BASE_DIR/model_architecture/${CFG_EXPTNAME}.$CFG_N_TIED_STATES.mdef";
    my $dest_mixwfn = "$cd_hmmdir/mixture_weights";
    my $dest_meanfn = "$cd_hmmdir/means";
    my $dest_varfn = "$cd_hmmdir/variances";
    my $dest_tmatfn = "$cd_hmmdir/transition_matrices";

    my $logdir = "$CFG_BASE_DIR/logdir/07.cd-schmm";
    mkdir ($logdir,0777) unless -d $logdir;
    my $logfile = "$logdir/$CFG_EXPTNAME.copy.ci.2.cd.log";

    my $COPY = "$CFG_BIN_DIR/init_mixw";

    open LOG,"> $logfile";

    if (open PIPE,"$COPY -src_moddeffn $src_moddeffn -src_ts2cbfn  ${CFG_HMM_TYPE} -src_mixwfn   $src_mixwfn -src_meanfn $src_meanfn -src_varfn $src_varfn -src_tmatfn $src_tmatfn -dest_moddeffn $dest_moddeffn -dest_ts2cbfn ${CFG_HMM_TYPE} -dest_mixwfn $dest_mixwfn -dest_meanfn  $dest_meanfn -dest_varfn $dest_varfn -dest_tmatfn $dest_tmatfn -feat $CFG_FEATURE -ceplen $CFG_VECTOR_LENGTH 2>&1 |") {
	while ($line = <PIPE>) {
	    print LOG $line;
	}
	
	close PIPE;
	close LOG;
    } else {
	print LOG "Unable to execute $COPY\n";
	&ST_Log ("Unable to execute $COPY\n");
    }

}

