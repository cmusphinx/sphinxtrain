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

$TESTING = 0;
my $iter = 1;
if (($#ARGV == ($index))) {
   $iter= $ARGV[$index];
}

my $scriptdir = "${CFG_SCRIPT_DIR}/04.cd_schmm_untied";
my $logdir = "${CFG_LOG_DIR}/04.cd_schmm_untied";
mkdir ("$logdir",0777) unless -d $logdir;

#Read npart_untied from variables.def

if ($iter == 1) {
   # Clean up junk from earlier runs
   &ST_Log ("MODULE: 04 Training Context Dependent models\n");
   &ST_Log ("\tCleaning up accumulator directories...");

    system ("rm -f $CFG_BWACCUM_DIR/${CFG_EXPTNAME}_buff_?/* ${CFG_BWACCUM_DIR}/${CFG_EXPTNAME}_buff_??/*");

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
$part = 1;

system ("$scriptdir/baum_welch.pl -cfg $cfg_file $iter $part $n_parts");
system ("$scriptdir/norm_and_launchbw.pl -cfg $cfg_file $iter");

exit 0;

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

