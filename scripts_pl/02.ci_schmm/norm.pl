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
require "$CFG_SCRIPT_DIR/util/utils.pl";

#*****************************************************************************
#  Baum-welch is done in several parts. This script gathers the results of
#  all those parts and then computes the discrete probability distributions
#  associated with all the states. It also computes the transition matrices.
#****************************************************************************

$| = 1; # Turn on autoflushing

die "USAGE: $0 <iter>" if ($#ARGV != $index);
$iter = $ARGV[$index];

$modelname="${CFG_EXPTNAME}.ci_${CFG_DIRLABEL}";
$processpart="02.ci_schmm";

$bwaccumdir 	     = "";
opendir(ACCUMDIR, "${CFG_BASE_DIR}/bwaccumdir") 
    or die "Could not open ${CFG_BASE_DIR}/bwaccumdir\n";
@bwaccumdirs = grep /${CFG_EXPTNAME}_buff_/, readdir(ACCUMDIR);
closedir(ACCUMDIR);
for (@bwaccumdirs) {
    $bwaccumdir .= " \"${CFG_BASE_DIR}/bwaccumdir/$_\"";
}
$hmmdir 	     = "${CFG_BASE_DIR}/model_parameters/$modelname";
mkdir ($hmmdir,0777) unless -d $hmmdir;
$means               = "$hmmdir/means";
$variances           = "$hmmdir/variances";
$mixture_weights     = "$hmmdir/mixture_weights";
$transition_matrices = "$hmmdir/transition_matrices";

$logdir              = "${CFG_LOG_DIR}/$processpart";
mkdir ($logdir,0777) unless $logdir;
$logfile 	     = "$logdir/${CFG_EXPTNAME}.${iter}.norm.log";

copy "$CFG_GIF_DIR/green-ball.gif", "$CFG_BASE_DIR/.02.norm.$iter.state.gif";
&ST_HTML_Print ("\t" . &ST_ImgSrc("$CFG_BASE_DIR/.02.norm.$iter.state.gif") . " ");   
&ST_Log ("    Normalization for iteration: $iter ");
&ST_HTML_Print (&ST_FormatURL("$logfile", "Log File") . " ");

$NORM  = "$CFG_BIN_DIR/norm";

my $cmd = "\"$NORM\" -accumdir $bwaccumdir -mixwfn \"$mixture_weights\"  -tmatfn \"$transition_matrices\" -meanfn \"$means\" -varfn \"$variances\" -feat ${CFG_FEATURE} -ceplen ${CFG_VECTOR_LENGTH}";

$return_value = RunTool($cmd, $logfile, 0);

if ($return_value) {
  copy "$CFG_GIF_DIR/red-ball.gif", "$CFG_BASE_DIR/.02.norm.$iter.state.gif";
  &ST_LogError ("\t$NORM failed\n");
}
exit ($return_value);

