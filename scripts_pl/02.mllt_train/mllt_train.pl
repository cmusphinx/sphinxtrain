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

my $processname = "02.mllt_train";
my $logdir   = catdir($ST::CFG_LOG_DIR, $processname);
my $logfile  = catfile($logdir, "${ST::CFG_EXPTNAME}.mllt_train.log");
my $ldafile  = catfile($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.lda");
my $mlltfile = catfile($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.mllt");

opendir(ACCUMDIR, $ST::CFG_BWACCUM_DIR)
    or die "Could not open $ST::CFG_BWACCUM_DIR: $!";
my @bwaccumdirs = map catdir($ST::CFG_BWACCUM_DIR, $_),
    grep /^\Q${ST::CFG_EXPTNAME}_buff_/, readdir(ACCUMDIR);
closedir(ACCUMDIR);

# Add PYTHONPATH
$ENV{PYTHONPATH} .= ':' . File::Spec->catdir($ST::CFG_SPHINXTRAIN_DIR, 'python');
my $return_value = RunTool(catfile($ST::CFG_SPHINXTRAIN_DIR, 'python', 'cmusphinx', 'mllt.py'),
		 $logfile, 0,
		 -l => $ldafile, 
		 $mlltfile,
		 @bwaccumdirs);

if (! -s $mlltfile || $return_value != 0) {
    LogError("mllt.py failed to create MLLT transform with status $return_value");
} else {
    open LOG, ">>$logfile" or die "Failed to open $logfile: $!";
    print LOG "MLLT training complete\n";
    close LOG;
}
exit($return_value);
