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
## Author: John Salatas <jsalatas@users.sourceforge.net>
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

$| = 1; # Turn on autoflushing
Log ("MODULE: 0000 train grapheme-to-phoneme model\n");
if ($ST::CFG_G2P_MODEL ne "yes") {
    Log("Skipped (set \$CFG_G2P_MODEL = 'yes' to enable)\n");
    exit(0);
}
my $logdir = "$ST::CFG_LOG_DIR/0000.g2p_train";
my $logfile = "$logdir/$ST::CFG_EXPTNAME.g2p";

Log ("Phase 1: Cleaning up directories: logs...\n");
rmtree($logdir, 0, 1);
mkdir($logdir,0777);

my $g2p_dir = "$ST::CFG_BASE_DIR/g2p/";
my $g2p_prefix = "$ST::CFG_BASE_DIR/g2p/$ST::CFG_EXPTNAME";
my $dict = "$ST::CFG_DICTIONARY";
my $g2p_model = "$ST::CFG_BASE_DIR/g2p/$ST::CFG_EXPTNAME.fst";
my $test_file = "$ST::CFG_BASE_DIR/g2p/$ST::CFG_EXPTNAME.test";
my $decoder_path = "$ST::CFG_BIN_DIR";

mkdir ($g2p_dir,0777);

Log ("Phase 2: Training g2p model...\n");
my $rv = RunTool('g2p_train', $logfile . ".training.log", 0,
		 -ifile => $dict,
		 -prefix => $g2p_prefix);
return $rv if $rv;

Log ("Phase 3: Evaluating g2p model...\n");
my @script_args = (catfile($ST::CFG_SCRIPT_DIR, '0000.g2p_train', 'evaluate.py'),
                $decoder_path,
                $g2p_model,
                $test_file,
                $g2p_prefix);
my $rv = RunTool(shift @script_args, $logfile . ".evaluate.log", 0, @script_args);

Log ("Phase 4: Creating pronunciations for OOV words...\n");
my @wordsflag = (-words => 'yes');
my @isfileflag = (-isfile => 'yes');
my $rv = RunTool('phonetisaurus-g2p', $logfile . ".make_dict.log", 0,
		 -model => $g2p_model,
		 -nbest => 1,
		 @wordsflag,
		 -input => "$ST::CFG_TRANSCRIPTFILE.oov",
		 @isfileflag,
		 -output => "$ST::CFG_TRANSCRIPTFILE.oov.dic");

Log ("Phase 5: Merging primary and OOV dictionaries...\n");
open MERGED, ">", "$ST::CFG_DICTIONARY.full";
open DIC, "<", "$ST::CFG_DICTIONARY";
open OOV, "<", "$ST::CFG_TRANSCRIPTFILE.oov.dic";
while(<DIC>) {
    my($line) = $_;
    print MERGED $line;
}
while(<OOV>) {
    my($line) = $_;
    print MERGED $line;
}
close MERGED;
close DIC;
close OOV;

#Log("$rv\n");
#return $rv if $rv;
