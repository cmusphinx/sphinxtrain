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

$logdir = "$CFG_LOG_DIR/03.makeuntiedmdef";
mkdir ($logdir,0777) unless -d $logdir;
$logfile = "$logdir/${CFG_EXPTNAME}.make_alltriphonelist.log";

&ST_Log ("MODULE: 03 Make Untied mdef\n");

$untiedmdef = "${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.untied.mdef";

## awb: replace with
##  mk_mdef_gen -phnlstfn $phonelist TRANSCRIPTFILE DICTIONARY -ountiedmdef ..  n_states .. 
# -minocc       1         Min occurances of a triphone must occur for inclusion in mdef file

$MAKE_MDEF = "$CFG_BIN_DIR/mk_mdef_gen";
system ("$MAKE_MDEF -phnlstfn $CFG_RAWPHONEFILE -dictfn $CFG_DICTIONARY -fdictfn $CFG_FILLERDICT -lsnfn $CFG_TRANSCRIPTFILE -ountiedmdef  $untiedmdef -n_state_pm  $CFG_STATESPERHMM 2>$logfile");

exit 0;
