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

use strict;
use File::Basename;
use File::Spec::Functions;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

$| = 1; # Turn on autoflushing
Log("MODULE: 00a Create vocabulary-restricted dictionary\n");

if ($ST::CFG_VOCAB_DICT ne "yes") {
    Log("Skipped (set \$CFG_VOCAB_DICT = 'yes' to enable)\n");
    exit(0);
}

my $logdir = "$ST::CFG_LOG_DIR/00a.vocab_dict";
mkdir($logdir, 0777) unless -d $logdir;
my $logfile = "$logdir/$ST::CFG_EXPTNAME.vocab_dict.log";

my $vocabfile = "$ST::CFG_LIST_DIR/$ST::CFG_EXPTNAME.vocab";
my $outdict = $ST::CFG_VOCAB_DICTIONARY;

unless (-f $vocabfile) {
    LogError("Vocabulary file not found: $vocabfile\n");
    LogError("Run 00.verify first to generate it\n");
    exit(1);
}

Log("Creating vocabulary-restricted dictionary\n");
Log("  Input dict:  $ST::CFG_DICTIONARY\n");
Log("  Vocabulary:  $vocabfile\n");
Log("  Output dict: $outdict\n");

$ENV{PYTHONPATH} .= ':' . catdir($ST::CFG_SPHINXTRAIN_DIR, 'python');
my $rv = RunTool("python", $logfile, 0,
		 "-m", "cmusphinx.vocab_dict",
		 $ST::CFG_DICTIONARY,
		 $vocabfile,
		 $outdict);

exit($rv);
