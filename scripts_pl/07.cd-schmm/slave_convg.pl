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
## 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
##    endorse or promote products derived from this software without
##    prior written permission. To obtain permission, contact 
##    sphinx@cs.cmu.edu.
##
## 4. Products derived from this software may not be called "Sphinx"
##    nor may "Sphinx" appear in their names without prior written
##    permission of Carnegie Mellon University. To obtain permission,
##    contact sphinx@cs.cmu.edu.
##
## 5. Redistributions of any form whatsoever must retain the following
##    acknowledgment:
##    "This product includes software developed by Carnegie
##    Mellon University (http://www.speech.cs.cmu.edu/)."
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
## Author: Ricky Houghton (converted from scripts by Rita Singh)
##


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


my $scriptdir = "$CFG_SCRIPT_DIR/07.cd-schmm";
my $logdir = "$CFG_LOG_DIR/07.cd_schmm_tied";
mkdir ($logdir,0777) unless -d $logdir;

my $bwaccumdir = "$CFG_BASE_DIR/bwaccumdir";
mkdir ($bwaccumdir,0777) unless -d $bwaccumdir;

my $modeldir  = "$CFG_BASE_DIR/model_parameters";
mkdir ($modeldir,0777) unless -d $modeldir;

&ST_Log ("MODULE: 07 Training Context dependent models\n");
&ST_Log ("\tCleaning up accumulator directories...");
system ("rm  -rf $bwaccumdir/${CFG_EXPTNAME}_buff_*");

$logdir = "$CFG_LOG_DIR/07.cd-schmm";

# We have to clean up and run flat initialize if it is the first iteration
if ($iter == 1) {
    
    &copyci2cd2initialize();
}


my $n_parts = ($CFG_NPART) ? $CFG_NPART : 1;

$converged = 0;			
while ((! $converged)) {
    # baum_welch is configured to work on  multiple parts, we are ignoring this by using 1 1
    for ($part=1;$part<=$n_parts;$part++) {
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
    
    my $hmm_dir = "$CFG_BASE_DIR/model_parameters/${CFG_EXPTNAME}.cd_semi_$CFG_N_TIED_STATES";
    mkdir ($hmm_dir,0777) unless -d $hmm_dir;

    #new models to be produced after normalization
    my $mixwfn         = "$hmm_dir/mixture_weights";
    my $tmatfn         = "$hmm_dir/transition_matrices";
    my $meanfn         = "$hmm_dir/means";
    my $varfn          = "$hmm_dir/variances";

    my $logfile   = "$logdir/${CFG_EXPTNAME}.$iter.norm.log";

    # HTML Logging, not so well done
    system ("cp $CFG_GIF_DIR/green-ball.gif $CFG_BASE_DIR/.07.norm.$iter.$part.state.gif");
    &ST_HTML_Print ("\t\t<img src=$CFG_BASE_DIR/.07.norm.$iter.$part.state.gif> ");        
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
	system ("cp $CFG_GIF_DIR/red-ball.gif $CFG_BASE_DIR/.07.norm.$iter.$part.state.gif");
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


sub copyci2cd2initialize ()
{
    &ST_Log ("\tCopy CI 2 CD 2 initialize\n");
    
    #**************************************************************************
    # this script copies the mixw/mean/var/tmat from a ci (continuous) HMM
    # to a cd (continuous) HMM. We need two mdef files, one without triphones
    # for the ci hmm definition and another one with triphones for the cd
    # triphone definition.
    # our final goal is to train in stages, first the ci models and then
    # with cd models. This should minimize initialization problems..
     #*************************************************************************

    my $ci_hmmdir = "$CFG_BASE_DIR/model_parameters/${CFG_EXPTNAME}.ci_semi";
    my $src_moddeffn = "$CFG_BASE_DIR/model_architecture/${CFG_EXPTNAME}.ci.mdef";
    my $src_mixwfn = "$ci_hmmdir/mixture_weights";
    my $src_meanfn = "$ci_hmmdir/means";
    my $src_varfn = "$ci_hmmdir/variances";
    my $src_tmatfn = "$ci_hmmdir/transition_matrices";

    my $cd_hmmdir = "$CFG_BASE_DIR/model_parameters/$CFG_EXPTNAME.cd_semi_initial";
    mkdir ($cd_hmmdir,0777) unless -d $hmmdir;

    my $dest_moddeffn = "$CFG_BASE_DIR/model_architecture/$CFG_EXPTNAME.$CFG_N_TIED_STATES.mdef";
    my $dest_mixwfn = "$cd_hmmdir/mixture_weights";
    my $dest_meanfn = "$cd_hmmdir/means";
    my $dest_varfn = "$cd_hmmdir/variances";
    my $dest_tmatfn = "$cd_hmmdir/transition_matrices";

    my $logdir = "$CFG_BASE_DIR/logdir/07.cd_schmm_tied";
    mkdir ($logdir,0777) unless -d $logdir;
    my $logfile = "$logdir/$CFG_EXPTNAME.copy.ci.2.cd.log";

    my $COPY = "$CFG_BIN_DIR/init_mixw";

    system("$COPY -src_moddeffn $src_moddeffn -src_ts2cbfn  .semi. -src_mixwfn   $src_mixwfn -src_meanfn $src_meanfn -src_varfn $src_varfn -src_tmatfn $src_tmatfn -dest_moddeffn $dest_moddeffn -dest_ts2cbfn .semi. -dest_mixwfn $dest_mixwfn -dest_meanfn  $dest_meanfn -dest_varfn $dest_varfn -dest_tmatfn $dest_tmatfn -feat $CFG_FEATURE -ceplen $CFG_VECTOR_LENGTH 2>&1 >>$logfile");

}

