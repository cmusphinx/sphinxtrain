#!/usr/local/bin/perl5

my $index = 0;

# RAH Force passage of config file, or look for it one directory up.
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    if (! -s $cfg_file) {
	print "-cfg specified, but unable to find file $ARGV[1]\n";
	exit -3;
    }
    $index = 2;
    require $cfg_file;
} else {
    $cfg_file = "./sphinx_train.cfg";
    require $cfg_file;
    &ST_LogWarning("-cfg not specified, using the default ./sphinx_train.cfg");
}


#***************************************************************************
# This script launches all the ci - continuous training jobs in the proper
# order. First it cleans up the directories, then launches the 
# flat initialization, and the baum-welch and norm jobs for the required
# number of iterations. Within each iteration it launches as many baumwelch
# jobs as the number of parts we wish to split the training into.
#***************************************************************************

die "USAGE: $0 <iteration number>" if (($#ARGV != ($index)));

my $iter = $ARGV[$index];


my $scriptdir = "$CFG_SCRIPT_DIR/02.ci_schmm";
my $bwaccumdir = "$CFG_BASE_DIR/bwaccumdir";
mkdir ($bwaccumdir,0777) unless -d $bwaccumdir;

my $modeldir  = "$CFG_BASE_DIR/model_parameters";
mkdir ($modeldir,0777) unless -d $modeldir;

&ST_Log ("MODULE: 02 Training Context Independent models\n");
&ST_Log ("\tCleaning up accumulator directories...");
system ("rm  -rf $bwaccumdir/${CFG_EXPTNAME}_buff_*");

$logdir              = "$CFG_LOG_DIR/02.ci_schmm";

# We have to clean up and run flat initialize if it is the first iteration
if ($iter == 1) {
    &ST_Log ("log directories...");
    system ("rm -f $logdir/*");
    &ST_Log ("model directories..\n");
    system ("rm -f $modeldir/${CFG_EXPTNAME}.ci_semi/*");
    
    # For the first iteration Flat initialize models.
    &FlatInitialize();
}


my $n_parts = ($CFG_NPART) ? $CFG_NPART : 1;

$converged = 0;			
while ((! $converged)) {
    # baum_welch is configured to work on  multiple parts, we are ignoring this by using 1 1
    for ($part=1;$part<=$n_parts;$part++) {
	print ("$scriptdir/baum_welch.pl -cfg $cfg_file $iter $part $n_parts\n");
	system ("$scriptdir/baum_welch.pl -cfg $cfg_file $iter $part $n_parts");
    }

#    system ("$scriptdir/baum_welch.pl -cfg $cfg_file $iter 1 1");

    # Note, should be checking for error messages in the baum_welch logs.

    # Run normalization - Ignoring check that above may have failed.
    &Norm ($iter);
    $converged = &ST_Converged ($iter,$logdir);
    $iter++;
}
$iter--;			# Bring it back to the correct value
&ST_Log ("\tBaum Welch has converged after $iter iterations\n") if ($converged == 1);
&ST_Log ("\tBaum Welch has exited after reaching MAX_ITERATIONS ($CFG_MAX_ITERATIONS)\n") if ($converged == 2);
&ST_LogWarning ("\tBaum Welch has exited, however no one has updated the exit code correctly: $converged\n") if ($converged > 2);

exit 0;


# This is the normalization step. Some of the variables here are shared with the above script
sub Norm ()
{
    my $iter = shift;
    my $n_parts = ($CFG_NPART > 0) ? $CFG_NPART : 1; # Need at least two parts for deleted interpolation
    my $part;
    # buffer_dir should point to each possible bwaccumdir, based on the n_parts (CFG_PART)
    my $buffer_dir = "";
    for ($part=1;$part<=$n_parts;$part++) {
	$buffer_dir .= "${CFG_BASE_DIR}/bwaccumdir/${CFG_EXPTNAME}_buff_$part ";
    }
#    my $buffer_dir = "$CFG_BASE_DIR/bwaccumdir/${CFG_EXPTNAME}_buff_1";
    
    my $hmm_dir           = "$CFG_BASE_DIR/model_parameters/${CFG_EXPTNAME}.ci_semi";
    mkdir ($hmm_dir,0777) unless -d $hmm_dir;

    #new models to be produced after normalization
    my $mixwfn         = "$hmm_dir/mixture_weights";
    my $tmatfn         = "$hmm_dir/transition_matrices";
    my $meanfn         = "$hmm_dir/means";
    my $varfn          = "$hmm_dir/variances";

    mkdir ($CFG_CI_LOG_DIR,0777) unless -d $CFG_CI_LOG_DIR;
    my $logfile   = "$CFG_CI_LOG_DIR/${CFG_EXPTNAME}.$iter.norm.log";

    # HTML Logging, not so well done
    system ("cp $CFG_GIF_DIR/green-ball.gif $CFG_BASE_DIR/.02.norm.$iter.$part.state.gif");
    &ST_HTML_Print ("\t\t<img src=$CFG_BASE_DIR/.02.norm.$iter.$part.state.gif> ");        
    &ST_Log ("Normalization ");
    &ST_HTML_Print ("<A HREF=\"$logfile\">Log File</A>\n");

    #set mach = `~rsingh/51..tools/machine_type.csh`
    #set NORM   = ~rsingh/09..sphinx3code/trainer/bin.$mach/norm
    my $NORM   = "$CFG_BIN_DIR/norm";

    if (open (PIPE, "$NORM -accumdir $buffer_dir -mixwfn $mixwfn -tmatfn $tmatfn -meanfn $meanfn -varfn $varfn -feat $CFG_FEATURE -ceplen  $CFG_VECTOR_LENGTH 2>&1 |")) {
	
	open LOG,">$logfile";
	while (<PIPE>) {
	    print LOG "$_";
	}
	close PIPE;
	print LOG "Current Overall Likelihood per Frame = ";
	
	system("grep \"overall>\" $logdir/${CFG_EXPTNAME}.${iter}-*.bw.log | awk '{X += \$3;Y += \$6} END {print Y/X}' >>$logfile");
	
	print LOG "\n";
	$date = &ST_DateStr();
	print LOG "$date\n";
	close LOG;
    } else {
	system ("cp $CFG_GIF_DIR/red-ball.gif $CFG_BASE_DIR/.02.norm.$iter.$part.state.gif");
    }
}


sub Converged ()
{
  my $iter = shift;

  # See what happened at this iteration
  $l = `grep \"overall>\" $logdir/${CFG_EXPTNAME}.${iter}-*.bw.log | awk '{X += \$3;Y += \$6} END {print Y/X}'`;

  if ($iter > 0) {
      my $tmp_iter = $iter - 1;
      # See what happened last iteration
      $p = `grep \"overall>\" $logdir/${CFG_EXPTNAME}.${tmp_iter}-*.bw.log | awk '{X += \$3;Y += \$6} END {print Y/X}'`;
      # Compute it's ratio
      $ratio = ($l-$p)/abs($p);
      &ST_Log ("\t\tRatio: $ratio\n");
  }

  # Don't even bother checking convergence until we've reached a minimum number of loops
  return 0 if ($iter < $CFG_MIN_ITERATIONS);
  return 1 if ($ratio < $CFG_CONVERGENCE_RATIO);
  return 2 if ($iter > $CFG_MAX_ITERATIONS);
}


sub FlatInitialize ()
{
    &ST_Log ("\tFlat initialize\n");
    
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
    
    system "sed 's+\$+ - - - +g' $CFG_RAWPHONEFILE > $phonefile";


    my $logfile = "$logdir/${CFG_EXPTNAME}.make_ci_mdef_fromphonelist.log";
    &ST_HTML_Print ("\t\tmk_model_def <A HREF=\"$logfile\">Log File</A>\n");    

    #set mach = `~rsingh/51..tools/machine_type.csh`
    #set MAKE_MDEF = ~rsingh/09..sphinx3code/trainer/bin.$mach/mk_model_def
    $MAKE_MDEF = "$CFG_BIN_DIR/mk_model_def";
    system ("$MAKE_MDEF -phonelstfn $phonefile -moddeffn $ci_mdeffile -n_state_pm $CFG_STATESPERHMM 2>$logfile");
    
    #-------------------------------------------------------------------------
    # Decide on what topology to use for the hmms: 3 state, 5 state, blah state
    # or what, give it to the variable "statesperhmm" and use it to create
    # the topology matrix in the topology file
    #-------------------------------------------------------------------------
    
    #$statesperhmm obtained from variables.def
    $topologyfile             = "$modarchdir/$CFG_EXPTNAME.topology";
    
    #$base_dir/training/bin/maketopology.csh $statesperhmm $skipstate >! $topologyfile
    # Note, here we don't want STDERR going to topologyfile, just the STDOUT
    system ("/sphx_train/csh/maketopology.csh $CFG_STATESPERHMM $CFG_SKIPSTATE >$topologyfile");
    
    #-------------------------------------------------------------------------
    # make the flat models using the above topology file and the mdef file
    #------------------------------------------------------------------------
    $outhmm               = "$hmmdir/${CFG_EXPTNAME}.ci_semi_flatinitial";
    mkdir ($outhmm,0777) unless -d $outhmm;
    
    #set FLAT = ~rsingh/09..sphinx3code/trainer/bin.$mach/mk_flat
    $FLAT = "$CFG_BIN_DIR/mk_flat";

    $logfile = "$logdir/${CFG_EXPTNAME}.makeflat_cischmm.log";
    &ST_HTML_Print ("\t\tmk_flat<A HREF=\"$logfile\">Log File</A>\n");    

    open LOG,">$logfile";

    if (open PIPE, "$FLAT -moddeffn $ci_mdeffile -topo $topologyfile -mixwfn  $outhmm/mixture_weights -tmatfn $outhmm/transition_matrices -nstream $CFG_NUM_STREAMS -ndensity  256 2>&1 |") {
    
	while ($line = <PIPE>) {
	    print LOG $line;
	}
	
	close PIPE;
	close LOG;
    }
}
