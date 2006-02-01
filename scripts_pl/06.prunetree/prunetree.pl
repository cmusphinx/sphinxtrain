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
#*************************************************************************
# This script prunes the trees computed earlier to have the desired number
# of leaves. Each leaf corresponds to one tied state
#*************************************************************************
#
#   Author: Alan W Black (awb@cs.cmu.edu)
#

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

die "USAGE: $0 <iteration number>" if (($#ARGV != ($index)));

my $n_tied_states = $ARGV[$index];
my $occurance_threshold = 0;

my $mdef_file = "$CFG_BASE_DIR/model_architecture/$CFG_EXPTNAME.alltriphones.mdef";

my $unprunedtreedir = "$CFG_BASE_DIR/trees/$CFG_EXPTNAME.unpruned";
my $prunedtreedir  = "$CFG_BASE_DIR/trees/$CFG_EXPTNAME.$n_tied_states";
mkdir ($prunedtreedir,0777) unless -d $prunedtreedir;

my $PRUNETREE = "$CFG_BIN_DIR/prunetree";

my $logdir = "$CFG_LOG_DIR/06.prunetree";
mkdir ($logdir,0777) unless -d $logdir;
my $logfile = "$logdir/$CFG_EXPTNAME.prunetree.$n_tied_states.log";

$| = 1; # Turn on autoflushing
&ST_Log ("    Prune trees\n");
&ST_HTML_Print ("\t\t" . &ST_FormatURL("$logfile", "Log File") . " ");

my $cmd = "\"$PRUNETREE\" -itreedir \"$unprunedtreedir\" -nseno $n_tied_states -otreedir \"$prunedtreedir\" -moddeffn \"$mdef_file\" -psetfn \"$CFG_QUESTION_SET\" -minocc $occurance_threshold";

exit (RunTool($cmd, $logfile, 0));
