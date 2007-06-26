#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 2006 Carnegie Mellon University.  All rights
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
## Author: David Huggins-Daines
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;
use File::Temp;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

my $processname = "05.lda_train";
my $logdir   = "$ST::CFG_LOG_DIR/$processname";
my $logfile  = "$logdir/${ST::CFG_EXPTNAME}.lda_train.log";
my $moddeffn = defined($ST::CFG_FORCE_ALIGN_MDEF)
    ? $ST::CFG_FORCE_ALIGN_MDEF
    : "$ST::CFG_BASE_DIR/model_architecture/$ST::CFG_EXPTNAME.ci.mdef";
my $listoffiles = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedfiles";

Log("MODULE: 05 Train LDA transformation\n");
if ($ST::CFG_HMM_TYPE eq ".semi.") {
    Log("Skipped for semi-continuous models\n");
    exit 0;
}

unless (-r $listoffiles and -d $ST::CFG_STSEG_DIR) {
    Log("Skipped: No force-alignment was done\n");
    exit 0;
}

Log("Cleaning up old log files...\n");
rmtree($logdir, 0, 1);
mkdir($logdir,0777);

HTML_Print ("\t\tlda_train " . FormatURL("$logfile", "Log File") . " ");

exit RunTool('lda_train', $logfile, 0,
	     -moddeffn => $moddeffn,
	     -ctlfn => $listoffiles,
	     -segdir => $ST::CFG_STSEG_DIR,
	     -cepdir => $ST::CFG_FEATFILES_DIR,
	     -cepext => $ST::CFG_FEATFILE_EXTENSION,
	     -agc => $ST::CFG_AGC,
	     -cmn => $ST::CFG_CMN,
	     -varnorm => $ST::CFG_VARNORM,
	     -feat => $ST::CFG_FEATURE,
	     -ceplen => $ST::CFG_VECTOR_LENGTH,
	     -outfn => $ST::CFG_LDA_TRANSFORM);
