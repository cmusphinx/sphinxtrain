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
## Author: David Huggins-Daines
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

# Assume that decoding has already been run.  Now create
# speaker-specific transcripts from the decoding result.
my $outdir = catdir($ST::DEC_CFG_RESULT_DIR, "${ST::DEC_CFG_EXPTNAME}.spk");
my $hypfile = catfile($ST::DEC_CFG_RESULT_DIR, "${ST::DEC_CFG_EXPTNAME}.match");
my $logdir = "$ST::CFG_LOG_DIR/80.mllr_adapt";
my $logfile = "$logdir/${ST::DEC_CFG_EXPTNAME}.adapt.log";

Log("MODULE: 80 Creating MLLR transformations\n");
Log("Phase 1: Cleaning up directories:");
# Don't do this on a queue, because of NFS bugs
unless ($ST::CFG_QUEUE_TYPE eq 'Queue::PBS') {
    LogProgress("\taccumulator...");
    rmtree ($ST::CFG_BWACCUM_DIR, 0, 1);
    mkdir ($ST::CFG_BWACCUM_DIR,0777);
}
LogProgress("logs...");
rmtree($logdir, 0, 1);
mkdir ($logdir,0777);
LogProgress("qmanager...");
rmtree ($ST::CFG_QMGR_DIR, 0, 1);
mkdir ($ST::CFG_QMGR_DIR,0777);
LogProgress("transformations...\n");
rmtree ($outdir, 0, 1);
mkdir ($outdir,0777);
LogStatus('completed');

Log("Phase 2: Make Speaker Lists");
my $rv = RunTool("../scripts/make_speaker_lists.pl",
		 $logfile, 0,
		 $ST::DEC_CFG_LISTOFFILES,
		 $hypfile, $outdir);
exit $rv if $rv;

# Run Baum-Welch and MLLR for each speaker
Log("Phase 3: Accumulate Statistics and Compute Transforms");
opendir OUTDIR, "$outdir" or die "Failed to open directory $outdir: $!";
while (defined(my $file = readdir OUTDIR)) {
    if ($file =~ /^(.*)\.fileids$/) {
	my $spk = $1;
	my $base = catfile($outdir, $spk);
	my $job =
	    LaunchScript("bw.$spk", ['baum_welch.pl',
				     $spk, "$base.fileids", "$base.transcription"]);
	# Launch an adapt job which runs afterwards
	LaunchScript("mllr.$spk", ['mllr_solve.pl', $spk], [$job]);
    }
}
