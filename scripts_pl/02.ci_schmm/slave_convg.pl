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
if (($#ARGV >= ($index+1))) {
   $n_part= $ARGV[$index+1];
}

my $scriptdir = "$CFG_SCRIPT_DIR/02.ci_schmm";

my $modeldir  = "$CFG_BASE_DIR/model_parameters";
mkdir ($modeldir,0777) unless -d $modeldir;

$| = 1; # Turn on autoflushing
$logdir = "$CFG_LOG_DIR/02.ci_hmm";

# We have to clean up and run flat initialize if it is the first iteration
if ($iter == 1) {
    &ST_Log ("MODULE: 02 Training Context Independent models\n");
    &ST_Log ("    Cleaning up directories: accumulator...");
    rmtree ($CFG_BWACCUM_DIR) unless ! -d $CFG_BWACCUM_DIR;
    mkdir ($CFG_BWACCUM_DIR,0777);
    &ST_Log ("logs...");
    rmtree ($logdir) unless ! -d $logdir;
    &ST_Log ("models...\n");
    rmtree ("$modeldir/${CFG_EXPTNAME}.ci_$CFG_DIRLABEL") unless ! -d $modeldir;
    
    # For the first iteration Flat initialize models.
    &FlatInitialize();

}

if ($MC && $n_parts > 1)
{
    # multi-processor version -- assumes ssh machine works
    for ($i=1; $i<=$n_parts; $i++)
    {
        $job_command = "$scriptdir/baum_welch.pl -cfg $cfg_file $iter $i $n_parts";
#	print $job_command."\n";
	open rrr,"scripts_pl/mc/mc_run.pl $job_command |";
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

sub FlatInitialize ()
{
    &ST_Log ("    Flat initialize\n");
    
    #**************************************************************************
    # this script given an mdef file and a  codebook (means/vars in S3 format)
    # produces flat mixture weights in a semicontinuos setting. From the models
    # produced by this script we can restart normal baum-welch training
    # Flat init might not be the best choice, specially if we have access to
    # segmentation files for the whole training database.
    #**************************************************************************
    
    $gender              = $CFG_GENDER;
    $logdir              = "$CFG_LOG_DIR/02.ci_schmm";
    $modarchdir          = "$CFG_BASE_DIR/model_architecture";
    $hmmdir              = "$CFG_BASE_DIR/model_parameters";
    mkdir ($logdir,0777) unless -d $logdir;
    mkdir ($modarchdir,0777) unless -d $modarchdir;
    mkdir ($hmmdir,0777) unless -d $hmmdir;
    
    #-------------------------------------------------------------------------
    # Take the phone list. Put three hyphens after every phone. That is the
    # required format. Then make a ci model definition file from it using the
    # following program.
    #-------------------------------------------------------------------------
    
    #$rawphonefile obtained from variables.def
    $phonefile           = "$modarchdir/$CFG_EXPTNAME.phonelist";
    $ci_mdeffile         = "$modarchdir/$CFG_EXPTNAME.ci.mdef";
    
    open PHONELIST, "<".$CFG_RAWPHONEFILE;
    open PHONEFILE, ">".$phonefile;
    $NUM_PHONES = 0;
    while ( $line = <PHONELIST> ) {
      chomp($line);
      $line =~ s/$/ - - - /;
      print PHONEFILE $line . "\n";
      $NUM_PHONES++;
    }

#    system "sed 's+\$+ - - - +g' $CFG_RAWPHONEFILE > $phonefile";


    my $logfile = "$logdir/${CFG_EXPTNAME}.make_ci_mdef_fromphonelist.log";
    &ST_HTML_Print ("\t\tmk_mdef_gen <A HREF=\"$logfile\">Log File</A>\n");    
    #-------------------------------------------------------------------------
    # Decide on what topology to use for the hmms: 3 state, 5 state, blah state
    # or what, give it to the variable "statesperhmm" and use it to create
    # the topology matrix in the topology file
    #-------------------------------------------------------------------------
    
    #$statesperhmm obtained from variables.def
    $topologyfile             = "$modarchdir/$CFG_EXPTNAME.topology";
    
    #$base_dir/training/bin/maketopology.csh $statesperhmm $skipstate >! $topologyfile
    # Note, here we don't want STDERR going to topologyfile, just the STDOUT
    system ("$CFG_BIN_DIR/maketopology.pl $CFG_STATESPERHMM $CFG_SKIPSTATE >$topologyfile");

    $MAKE_MDEF = "$CFG_BIN_DIR/mk_mdef_gen";
    system ("$MAKE_MDEF -phnlstfn $phonefile -ocimdef $ci_mdeffile -n_state_pm $CFG_STATESPERHMM 2>$logfile");
    
    #-------------------------------------------------------------------------
    # make the flat models using the above topology file and the mdef file
    #------------------------------------------------------------------------
    $outhmm               = "$hmmdir/${CFG_EXPTNAME}.ci_${CFG_DIRLABEL}_flatinitial";
    mkdir ($outhmm,0777) unless -d $outhmm;
    
    $FLAT = "$CFG_BIN_DIR/mk_flat";

    $logfile = "$logdir/${CFG_EXPTNAME}.makeflat_cihmm.log";
    &ST_HTML_Print ("\t\tmk_flat <A HREF=\"$logfile\">Log File</A>\n");    

    open LOG,">$logfile";

    if (open PIPE, "$FLAT -moddeffn $ci_mdeffile -topo $topologyfile -mixwfn  $outhmm/mixture_weights -tmatfn $outhmm/transition_matrices -nstream $CFG_NUM_STREAMS -ndensity  $CFG_INITIAL_NUM_DENSITIES 2>&1 |") {
    
	while ($line = <PIPE>) {
	    print LOG $line;
	}
	
	close PIPE;
	close LOG;
    }

    if ($CFG_HMM_TYPE eq ".semi.") {
      return (0);
    }

    #-------------------------------------------------------------------------
    # Accumulate the means from the training data
    #------------------------------------------------------------------------
    
    $ACCUM = "$CFG_BIN_DIR/init_gau";

    $logfile = "$logdir/${CFG_EXPTNAME}.initmean_cihmm.log";
    &ST_HTML_Print ("\t\taccum_mean <A HREF=\"$logfile\">Log File</A>\n");    

    open LOG,">$logfile";

    $output_buffer_dir = "$CFG_BASE_DIR/bwaccumdir/${CFG_EXPTNAME}_buff_1";
    mkdir ($output_buffer_dir,0777) unless -d $output_buffer_dir;

    if (open PIPE, "$ACCUM -ctlfn $CFG_LISTOFFILES -part 1 -npart 1 -cepdir $CFG_FEATFILES_DIR -cepext $CFG_FEATFILE_EXTENSION -accumdir $output_buffer_dir -agc $CFG_AGC -cmn $CFG_CMN -varnorm $CFG_VARNORM -feat $CFG_FEATURE -ceplen $CFG_VECTOR_LENGTH 2>&1 |") {
    
	while ($line = <PIPE>) {
	    print LOG $line;
	}
	
	close PIPE;
	close LOG;
    }

    #-------------------------------------------------------------------------
    # Normalize the means
    #------------------------------------------------------------------------
    
    $NORM = "$CFG_BIN_DIR/norm";

    $logfile = "$logdir/${CFG_EXPTNAME}.normmean_cihmm.log";
    &ST_HTML_Print ("\t\tnorm_mean <A HREF=\"$logfile\">Log File</A>\n");    

    open LOG,">$logfile";

    if (open PIPE, "$NORM -accumdir $output_buffer_dir -meanfn $outhmm/globalmean 2>&1 |") {
    
	while ($line = <PIPE>) {
	    print LOG $line;
	}
	
	close PIPE;
	close LOG;
    }

    #-------------------------------------------------------------------------
    # Accumulate the variances from the training data
    #------------------------------------------------------------------------
    
    $logfile = "$logdir/${CFG_EXPTNAME}.initvar_cihmm.log";
    &ST_HTML_Print ("\t\taccum_var <A HREF=\"$logfile\">Log File</A>\n");    

    open LOG,">$logfile";

    if (open PIPE, "$ACCUM -meanfn $outhmm/globalmean -ctlfn $CFG_LISTOFFILES -part 1 -npart 1 -cepdir $CFG_FEATFILES_DIR -cepext $CFG_FEATFILE_EXTENSION -accumdir $output_buffer_dir -agc $CFG_AGC -cmn $CFG_CMN -varnorm yes -feat $CFG_FEATURE -ceplen $CFG_VECTOR_LENGTH 2>&1 |") {
    
	while ($line = <PIPE>) {
	    print LOG $line;
	}
	
	close PIPE;
	close LOG;
    }

    #-------------------------------------------------------------------------
    # Normalize the variances
    #------------------------------------------------------------------------
    
    $logfile = "$logdir/${CFG_EXPTNAME}.normvar_cihmm.log";
    &ST_HTML_Print ("\t\tnorm_var <A HREF=\"$logfile\">Log File</A>\n");    

    open LOG,">$logfile";

    if (open PIPE, "$NORM -accumdir $output_buffer_dir -varfn $outhmm/globalvar 2>&1 |") {
    
	while ($line = <PIPE>) {
	    print LOG $line;
	}
	
	close PIPE;
	close LOG;
    }

    #-------------------------------------------------------------------------
    # Create the copy operation file, simply a map between states
    #------------------------------------------------------------------------

    $NUM_CI_STATES = $NUM_PHONES * $CFG_STATESPERHMM;
    if (open CPFILE, ">$CFG_CP_OPERATION") {
      for ($CI_STATE = 0; $CI_STATE < $NUM_CI_STATES; $CI_STATE++) {
	print CPFILE "$CI_STATE\t0\n";
      }
      close(CPFILE);
    } else {
      warn "Can't open $CFG_CP_OPERATION\n"; 
    }

    #-------------------------------------------------------------------------
    # Copy the means to all other states
    #------------------------------------------------------------------------
    
    $CPPARM = "$CFG_BIN_DIR/cp_parm";

    $logfile = "$logdir/${CFG_EXPTNAME}.cpmean_cihmm.log";
    &ST_HTML_Print ("\t\tcp_mean <A HREF=\"$logfile\">Log File</A>\n");    

    open LOG,">$logfile";

    if (open PIPE, "$CPPARM -cpopsfn $CFG_CP_OPERATION -igaufn $outhmm/globalmean -ncbout $NUM_CI_STATES -ogaufn $outhmm/means -feat $CFG_FEATURE 2>&1 |") {
    
	while ($line = <PIPE>) {
	    print LOG $line;
	}
	
	close PIPE;
	close LOG;
    }

    #-------------------------------------------------------------------------
    # Copy the variances to all other states
    #------------------------------------------------------------------------
    
    $logfile = "$logdir/${CFG_EXPTNAME}.cpvar_cihmm.log";
    &ST_HTML_Print ("\t\tcp_var <A HREF=\"$logfile\">Log File</A>\n");    

    open LOG,">$logfile";

    if (open PIPE, "$CPPARM -cpopsfn $CFG_CP_OPERATION -igaufn $outhmm/globalvar -ncbout $NUM_CI_STATES -ogaufn $outhmm/variances -feat $CFG_FEATURE 2>&1 |") {
    
	while ($line = <PIPE>) {
	    print LOG $line;
	}
	
	close PIPE;
	close LOG;
    }
    unlink $CFG_CP_OPERATION;
}
