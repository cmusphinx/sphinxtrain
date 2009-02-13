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
#**************************************************************************
# This script reads off the leaves of the pruned trees and creates an mdef
# file
#**************************************************************************
#
# Author: Alan W Black (awb@cs.cmu.edu)
#
use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

die "USAGE: $0 <number of tied states>" if @ARGV != 1;

my $n_tied_states = shift;

# If this is being run with an MLLT transformation keep the models and logs separate.
use vars qw($MLLT_FILE);
$MLLT_FILE = catfile($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.mllt");

my ($untied_mdef_file, $prunedtreedir, $tied_mdef_file, $logdir);
$untied_mdef_file = "$ST::CFG_BASE_DIR/model_architecture/$ST::CFG_EXPTNAME.alltriphones.mdef";
$prunedtreedir = "$ST::CFG_BASE_DIR/trees/$ST::CFG_EXPTNAME.$n_tied_states";
$tied_mdef_file = "$ST::CFG_BASE_DIR/model_architecture/$ST::CFG_EXPTNAME.$n_tied_states.mdef";
$logdir = "$ST::CFG_LOG_DIR/45.prunetree";

mkdir ($logdir,0777);
my $logfile = "$logdir/$ST::CFG_EXPTNAME.tiestate.$n_tied_states.log";

$| = 1; # Turn on autoflushing

my @phnarg;
if ($ST::CFG_CROSS_PHONE_TREES eq 'yes') {
    @phnarg = (-allphones => 'yes');
}
exit RunTool('tiestate', $logfile, 0,
	     -imoddeffn => $untied_mdef_file,
	     -omoddeffn => $tied_mdef_file,
	     -treedir => $prunedtreedir,
	     @phnarg,
	     -psetfn => $ST::CFG_QUESTION_SET);

