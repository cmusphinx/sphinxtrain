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

use File::Copy;
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

# this script runs deleted interpolation on a bunch of semi-cont
# hmm buffers. You need 2 or more buffers to run this!!

my $nsenones = "$CFG_N_TIED_STATES";

my $INTERP = "$CFG_BIN_DIR/delint";
my $cilambda = 0.9;

# up to 99 buffers
my $cd_hmmdir = "$CFG_BASE_DIR/model_parameters/$CFG_EXPTNAME.cd_semi_"."$CFG_N_TIED_STATES";
$bwaccumdir 	     = "";
for (<${CFG_BASE_DIR}/bwaccumdir/${CFG_EXPTNAME}_buff_*>) {
    $bwaccumdir .= " $_";
}

my $hmm_dir = "$CFG_BASE_DIR/model_parameters/$CFG_EXPTNAME.cd_semi_"."$CFG_N_TIED_STATES"."_delinterp";
mkdir ($hmm_dir,0777) unless -d $hmm_dir;

copy "$cd_hmmdir/means", "$hmm_dir/means";
copy "$cd_hmmdir/variances", "$hmm_dir/variances";
copy "$cd_hmmdir/transition_matrices", "$hmm_dir/transition_matrices";
my $mixwfn = "$hmm_dir/mixture_weights";

my $moddeffn = "$CFG_BASE_DIR/model_architecture/$CFG_EXPTNAME.$CFG_N_TIED_STATES.mdef";

my $logdir = "$CFG_BASE_DIR/logdir/08.deleted_interpolation";
my $logfile = "$logdir/$CFG_EXPTNAME.deletedintrep-${nsenones}.log";

$| = 1; # Turn on autoflushing
&ST_Log ("MODULE: 08 deleted interpolation\n");
&ST_Log ("    Cleaning up directories: logs...\n");
rmtree ($logdir) unless ! -d $logdir;
mkdir ($logdir,0777);

&ST_Log ("    Doing interpolation...\n");
&ST_HTML_Print ("\t<A HREF=\"$logfile\">Log File</A> ");

open LOG,"> $logfile";

if (open PIPE,"$INTERP -accumdirs $bwaccumdir -moddeffn $moddeffn -mixwfn $mixwfn -cilambda $cilambda -feat $CFG_FEATURE -ceplen $CFG_VECTOR_LENGTH -maxiter 4000 2>&1 2>&1 |") {
    while ($line = <PIPE>) {
       print LOG $line;
    }
    close PIPE;
    close LOG;
    &ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> completed </font>\n");
} else {
    &ST_HTML_Print ("\t\t<font color=\"$CFG_ERROR_COLOR\"> completed </font>\n");
}

exit 0;
