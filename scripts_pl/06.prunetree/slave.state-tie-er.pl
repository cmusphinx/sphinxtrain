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
# This script runs the prunetree and tiestate scripts 
#*************************************************************************
#
#  Author: Alan W Black (awb@cs.cmu.edu)
#

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

my $logdir = "$ST::CFG_LOG_DIR/06.prunetree";

$| = 1; # Turn on autoflushing
Log ("MODULE: 06 Prune Trees\n");
Log ("    Cleaning up old log files...\n");
rmtree ($logdir) unless ! -d $logdir;
mkdir ($logdir,0777);

# Build all triphone model
my $logfile = "$logdir/$ST::CFG_EXPTNAME.build.alltriphones.mdef.log";
my $modarchdir          = "$ST::CFG_BASE_DIR/model_architecture";
my $ALLTRIPHONESMDDEF = "$modarchdir/$ST::CFG_EXPTNAME.alltriphones.mdef";
my $phonefile           = "$modarchdir/$ST::CFG_EXPTNAME.phonelist";

HTML_Print ("\t\tmk_mdef_gen " . FormatURL("$logfile", "Log File") . " ");

my $status = RunTool('mk_mdef_gen', $logfile, 0,
		  -phnlstfn => $phonefile,
		  -oalltphnmdef => $ALLTRIPHONESMDDEF,
		  -dictfn => $ST::CFG_DICTIONARY,
		  -fdictfn => $ST::CFG_FILLERDICT,
		  -n_state_pm => $ST::CFG_STATESPERHMM);
exit $status if $status;

$status = RunScript('prunetree.pl', $ST::CFG_N_TIED_STATES)
    or RunScript('tiestate.pl', $ST::CFG_N_TIED_STATES);
exit $status;

