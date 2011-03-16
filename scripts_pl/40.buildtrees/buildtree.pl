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
## Author: Ricky Houghton (Alan W Black)
##
## Pulled this out of slace.treebuilder to allow this to run on
## multiple machines

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

#*************************************************************************
# This script runs the build_tree script for each state of each basephone
#*************************************************************************

die "USAGE: $0 <phone> " if @ARGV != 1;

my $phone = shift;

use vars qw($MLLT_FILE $MODEL_TYPE);
$MLLT_FILE = catfile($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.mllt");
$MODEL_TYPE = 'cd';

my $logdir = "${ST::CFG_LOG_DIR}/40.buildtrees";
mkdir ($logdir,0777);

$| = 1; # Turn on autoflushing

my $mdef_file       = "${ST::CFG_BASE_DIR}/model_architecture/${ST::CFG_EXPTNAME}.untied.mdef";
my $mixture_wt_file = "${ST::CFG_BASE_DIR}/model_parameters/${ST::CFG_EXPTNAME}.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_untied/mixture_weights";
my $means_file = "${ST::CFG_BASE_DIR}/model_parameters/${ST::CFG_EXPTNAME}.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_untied/means";
my $variances_file = "${ST::CFG_BASE_DIR}/model_parameters/${ST::CFG_EXPTNAME}.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_untied/variances";
my $tree_base_dir = "${ST::CFG_BASE_DIR}/trees";
my $unprunedtreedir = "$tree_base_dir/${ST::CFG_EXPTNAME}.unpruned";
mkdir ($tree_base_dir,0777);
mkdir ($unprunedtreedir,0777);

my $state = 0;
my $return_value = 0;
while ( $state < $ST::CFG_STATESPERHMM) {
    if ($return_value = BuildTree ($phone,$state)) {
        last;
    }
    $state++;
}

exit $return_value;


# SubRoutine created from build_tree.pl
sub BuildTree
{
    my $phn = shift;
    my $stt = shift;

    my $logfile = "$logdir/${ST::CFG_EXPTNAME}.buildtree.${phn}.${stt}.log";

    Log("${phn} ${stt} ", 'result');

    # RAH 7.21.2000 - These were other possible values for these
    # variables, I'm not sure the circumstance that would dictate
    # either set of values
    #	-stwt     1.0 0.1 0.01 
    #	-stwt     1.0 0.3 0.2 
    #   -stwt     1.0 0.3 0.1 0.01 0.001
    #	-ssplitthr 8e-4 \
    #	-csplitthr 1e-5 \

    my @stwt;
    if ($ST::CFG_STATESPERHMM == 5) {
      @stwt = qw(1.0 0.3 0.1 0.01 0.001);
    } elsif  ($ST::CFG_STATESPERHMM == 4) {
      @stwt = qw(1.0 0.1 0.0 0.0);
    } elsif  ($ST::CFG_STATESPERHMM == 3) {
      @stwt = qw(1.0 0.05 0.0);
    } elsif  ($ST::CFG_STATESPERHMM == 2) {
      @stwt = qw(1.0 0.025);
    } elsif  ($ST::CFG_STATESPERHMM == 1) {
      @stwt = qw(1);      
    } else {
      LogError("Failed to guess weights for training the tree for large number $ST::CFG_STATESPERHMM of states per HMM");
      exit (1);
    }

    my @gauflag;
    if ($ST::CFG_HMM_TYPE eq ".cont.") {
	@gauflag = (-meanfn => $means_file,
		    -varfn => $variances_file,
		    -fullvar => $ST::CFG_FULLVAR);
    }
    else {
	# Treat PTM as semi-continuous
	$ST::CFG_HMM_TYPE = ".semi.";
    }

    my @phnflag;
    if ($ST::CFG_CROSS_PHONE_TREES eq 'yes') {
	@phnflag = (-allphones => 'yes');
    }
    else {
	@phnflag = (-phone => $phn);
    }
    return RunTool('bldtree', $logfile, 0,
		   -treefn => "$unprunedtreedir/$phn-$stt.dtree",
		   -moddeffn => "$mdef_file",
		   -mixwfn => "$mixture_wt_file",
		   -ts2cbfn => $ST::CFG_HMM_TYPE,
		   -mwfloor => 1e-8,
		   -psetfn => $ST::CFG_QUESTION_SET,
		   @phnflag,
		   -state => $stt,
		   -stwt => join(",", @stwt),
		   @gauflag,
		   -ssplitmin => 1,
		   -ssplitmax => 7,
		   -ssplitthr => 0,
		   -csplitmin => 1,
		   -csplitmax => 2000,
		   -csplitthr => 0);
}
