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
## Author: Alan W Black (awb@cs.cmu.edu)
##
##

use strict;
use File::Basename;
use File::Spec::Functions;
use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

#***************************************************************************
# This script generates an mdef file for all the triphones occuring in the
# training set. This is done in several steps
# First, a list of all triphones possible in the vocabulary is generated
# (from the dictionary)
# An mdef file is then created to include all these triphones.
# The transcriptions of the training set are then used to count the number 
# of occurances of all the triphones in this mdef file.
# The triphones that occur more than a threshold number of times are used
# to generate the final mdef file. The threshold is automatically chosen
# so as to give the required number of triphones.
# (Thresholding is done so as to reduce the number of triphones, in order
# that the resulting models will be small enough to fit in the computer's
# memory)
#***************************************************************************
use File::Path;

my $logdir = "$ST::CFG_LOG_DIR/03.makeuntiedmdef";
my $logfile = "$logdir/${ST::CFG_EXPTNAME}.make_alltriphonelist.log";

$| = 1; # Turn on autoflushing
Log ("MODULE: 03 Make Untied mdef\n");
Log ("    Cleaning up old log files...\n");
rmtree($logdir) unless ! -d $logdir;
mkdir ($logdir,0777);

my $untiedmdef = "${ST::CFG_BASE_DIR}/model_architecture/${ST::CFG_EXPTNAME}.untied.mdef";

## awb: replace with
##  mk_mdef_gen -phnlstfn $phonelist TRANSCRIPTFILE DICTIONARY -ountiedmdef ..  n_states .. 
# -minocc       1         Min occurances of a triphone must occur for inclusion in mdef file

HTML_Print ("\t\tmk_untied_mdef " . FormatURL("$logfile", "Log File") . " ");

exit RunTool('mk_mdef_gen', $logfile, 0,
	     -phnlstfn => $ST::CFG_RAWPHONEFILE,
	     -dictfn => $ST::CFG_DICTIONARY,
	     -fdictfn => $ST::CFG_FILLERDICT,
	     -lsnfn => $ST::CFG_TRANSCRIPTFILE,
	     -ountiedmdef => $untiedmdef,
	     -n_state_pm => $ST::CFG_STATESPERHMM,
	     -maxtriphones => 10000);

