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
    $cfg_file = "../sphinx_train.cfg";
    require $cfg_file;
    &ST_LogWarning("-cfg not specified, using the default ../sphinx_train.cfg");
}

#***************************************************************************
# This script launches all the ci - continuous training jobs in the proper
# order. First it cleans up the directories, then launches the 
# flat initialization, and the baum-welch and norm jobs for the required
# number of iterations. Within each iteration it launches as many baumwelch
# jobs as the number of parts we wish to split the training into.
#***************************************************************************

$TESTING = 0;

die "USAGE: $0 <iteration number>" if ($#ARGV != $index);

my $iter = $ARGV[$index];

my $scriptdir = "${CFG_SCRIPT_DIR}/04.cd_schmm_untied";
my $logdir = "${CFG_LOG_DIR}/04.cd_schmm_untied";
mkdir ("$logdir",0777) unless -d $logdir;


#Read npart_untied from variables.def

# Clean up junk from earlier runs
&ST_Log ("MODULE: 04 Training Context Dependent models\n");
&ST_Log ("\tCleaning up accumulator directories...");

system ("rm -f $CFG_BWACCUM_DIR/${CFG_EXPTNAME}_buff_?/* ${CFG_BWACCUM_DIR}/${CFG_EXPTNAME}_buff_??/*");

if ($iter == 1) {
    &ST_Log ("log directories...");
    system ("rm -rf $logdir") unless $TESTING;
    mkdir ($logdir,0777) unless -d $logdir;

    &ST_Log ("\n");
    # For the first iteration Flat initialize models.
    # To start off queue trap job id
    &Initialize () unless $TESTING;
} else {
    &ST_Log ("\n");
}


my $n_parts = ($CFG_NPART) ? $CFG_NPART : 1;

$converged = 0;
while ((! $converged)) {
    # baum_welch is configured to work on  multiple parts, we are ignoring this by using 1 1
#    system "$scriptdir/baum_welch.pl -cfg $cfg_file $iter 1 1";
    for ($part=1;$part<=$n_parts;$part++) {
	system ("$scriptdir/baum_welch.pl -cfg $cfg_file $iter $part $n_parts");
    }

    # Run normalization - Ignoring check that above may have failed.
    &Norm ($iter);

    # Don't even bother checking convergence until we've reached a minimum number of loops
    $converged = &ST_Converged ($iter,$logdir);
    $iter++;
}

$iter--;			# Bring it back to the correct value
&ST_Log("\tBaum Welch has converged after $iter iterations\n") if ($converged == 1);
&ST_Log("\tBaum Welch has exited after reaching MAX_ITERATIONS ($CFG_MAX_ITERATIONS)\n") if ($converged == 2);
&ST_LogWarning("\tBaum Welch has exited, however no one has updated the exit code correctly: $converged") if ($converged > 2);

exit 0;



sub Converged ()
{
  my $iter = shift;
  my $logdir = shift;
  my ($l,$p,$ratio);

  # See what happened at this iteration
  $l = `grep \"overall>\" $logdir/${CFG_EXPTNAME}.${iter}-*.bw.log | awk '{X += \$3;Y += \$6} END {print Y/X}'`;

  if ($iter-- > 1) {
      # See what happened previous iteration
      $p = `grep \"overall>\" $logdir/${CFG_EXPTNAME}.${iter}-*.bw.log | awk '{X += \$3;Y += \$6} END {print Y/X}'`;
      # Compute it's ratio
      $ratio = ($l-$p)/abs($p);
      &ST_Log ("\t\tRatio: $ratio\n");
  }
  return 0 if ($iter < $CFG_MIN_ITERATIONS); # Don't check for convergence unless we've gone enough times
  return 1 if ($ratio < $CFG_CONVERGENCE_RATIO);
  return 2 if ($iter > $CFG_MAX_ITERATIONS);
}


sub Norm()
{
    my $iter = shift;
    my $n_parts = ($CFG_NPART > 0) ? $CFG_NPART : 1; # Need at least two parts for deleted interpolation
    my $part;
    # buffer_dir should point to each possible bwaccumdir, based on the n_parts (CFG_PART)
    my $buffer_dir = "";
    for ($part=1;$part<=$n_parts;$part++) {
	$buffer_dir .= "${CFG_BASE_DIR}/bwaccumdir/${CFG_EXPTNAME}_buff_$part ";
    }
#    my $buffer_dir 	    = "${CFG_BWACCUM_DIR}/${CFG_EXPTNAME}_buff_1";
    my $hmmdir 	            = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.cd_semi_untied";
    my $meanfn              = "$hmmdir/means";
    my $varfn               = "$hmmdir/variances";
    my $mixwfn              = "$hmmdir/mixture_weights";
    my $tmatfn              = "$hmmdir/transition_matrices";
    
    my $logdir              = "${CFG_LOG_DIR}/04.cd_schmm_untied";
    mkdir ($logdir,0777) unless $logdir;
    $logfile 	            = "$logdir/${CFG_EXPTNAME}.${iter}.norm.log";

    &ST_Log ("\t\tNormalization ");
    &ST_HTML_Print ("<A HREF=\"$logfile\">Log File</A>\n");

    mkdir ($hmmdir,0777) unless -d $hmmdir;

    #set mach = `~/51..tools/machine_type.csh`
    #set NORM  = ~/09..sphinx3code/trainer/bin.$mach/norm
    my $NORM  = "$CFG_BIN_DIR/norm";

    open (PIPE, "$NORM -accumdir $buffer_dir -mixwfn $mixwfn -tmatfn $tmatfn -meanfn $meanfn -varfn $varfn -feat ${CFG_FEATURE} -ceplen  ${CFG_VECTOR_LENGTH} 2>&1 |");

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
}


sub Initialize ()
  {
    my $cihmmdir = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.ci_semi";
    my $cdhmmdir = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.cd_semi_untied";
    mkdir ($cdhmmdir,0777) unless -d $cdhmmdir;

    my $logdir  =  "${CFG_LOG_DIR}/04.cd_schmm_untied";
    mkdir ($logdir,0777) unless -d $logdir;
    my $logfile = "$logdir/${CFG_EXPTNAME}.copycitocd.log";

    &ST_Log ("\tInitalization <A HREF=\"$logfile\">Log File</A>\n");


    #set mach = `~/51..tools/machine_type.csh`
    #set COPY_CI_TO_CD = /net/alf19/usr2/eht/s3/bin.$mach/init_mixw
    #set COPY_CI_TO_CD = ~/09..sphinx3code/trainer/bin.$mach/init_mixw
    my $COPY_CI_TO_CD = "${CFG_BIN_DIR}/init_mixw";

    open LOG,"> $logfile";

    if (open PIPE,"$COPY_CI_TO_CD -src_moddeffn ${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.ci.mdef -src_ts2cbfn  ${CFG_HMM_TYPE} -src_mixwfn   $cihmmdir/mixture_weights -src_meanfn   $cihmmdir/means -src_varfn    $cihmmdir/variances -src_tmatfn  $cihmmdir/transition_matrices -dest_moddeffn ${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.untied.mdef -dest_ts2cbfn ${CFG_HMM_TYPE} -dest_mixwfn $cdhmmdir/mixture_weights -dest_meanfn $cdhmmdir/means -dest_varfn $cdhmmdir/variances -dest_tmatfn $cdhmmdir/transition_matrices -feat ${CFG_FEATURE} -ceplen ${CFG_VECTOR_LENGTH} 2>&1 |") {
	
	while ($line = <PIPE>) {
	    print LOG $line;
	}
	
	close PIPE;
	close LOG;
    } else {
	print LOG "Unable to execute $COPY_CI_TO_CD\n";
	&ST_Log ("Unable to execute $COPY_CI_TO_CD\n");
    }
}

