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

my $iter = 1;
if (($#ARGV == ($index))) {
   $iter= $ARGV[$index];
}

my $scriptdir = "$CFG_SCRIPT_DIR/07.cd-schmm";
my $logdir = "$CFG_LOG_DIR/07.cd_schmm";
mkdir ($logdir,0777) unless -d $logdir;

my $bwaccumdir = "$CFG_BASE_DIR/bwaccumdir";
mkdir ($bwaccumdir,0777) unless -d $bwaccumdir;

my $modeldir  = "$CFG_BASE_DIR/model_parameters";
mkdir ($modeldir,0777) unless -d $modeldir;

# We have to clean up and run flat initialize if it is the first iteration
if ($iter == 1) {
    
    &ST_Log ("MODULE: 07 Training Context dependent models\n");
    &ST_Log ("\tCleaning up accumulator directories...");
    system ("rm  -rf $bwaccumdir/${CFG_EXPTNAME}_buff_*");
    &copyci2cd2initialize();
}


my $n_parts = ($CFG_NPART) ? $CFG_NPART : 1;
$part = 1;

system ("$scriptdir/baum_welch.pl -cfg $cfg_file $iter $part $n_parts");
system ("$scriptdir/norm_and_launchbw.pl -cfg $cfg_file $iter");

exit 0;

sub copyci2cd2initialize ()
{
    &ST_Log ("\nCopy CI 2 CD 2 initialize\n");
    
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

    my $logdir = "$CFG_BASE_DIR/logdir/07.cd_schmm";
    mkdir ($logdir,0777) unless -d $logdir;
    my $logfile = "$logdir/$CFG_EXPTNAME.copy.ci.2.cd.log";

    my $COPY = "$CFG_BIN_DIR/init_mixw";

    open LOG,"> $logfile";

    if (open PIPE,"$COPY -src_moddeffn $src_moddeffn -src_ts2cbfn  .semi. -src_mixwfn   $src_mixwfn -src_meanfn $src_meanfn -src_varfn $src_varfn -src_tmatfn $src_tmatfn -dest_moddeffn $dest_moddeffn -dest_ts2cbfn .semi. -dest_mixwfn $dest_mixwfn -dest_meanfn  $dest_meanfn -dest_varfn $dest_varfn -dest_tmatfn $dest_tmatfn -feat $CFG_FEATURE -ceplen $CFG_VECTOR_LENGTH 2>&1 |") {
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

