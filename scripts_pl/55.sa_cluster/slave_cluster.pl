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

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

my $clustdir = catdir($ST::CFG_BASE_DIR, 'cluster');
my $logdir = "$ST::CFG_LOG_DIR/55.sa_cluster";

# Clean everything up
Log ("MODULE: 55 Doing MLLR-based speaker clustering\n");
unless ($ST::CFG_CLUSTER eq 'yes') {
    Log("    Skipped (set \$CFG_CLUSTER to yes to enable)\n");
    exit 0;
}
Log ("    Cleaning up directories: accumulator...");
rmtree ($ST::CFG_BWACCUM_DIR, 0, 1);
mkdir ($ST::CFG_BWACCUM_DIR,0777);
Log ("cluster...");
rmtree ($clustdir, 0, 1);
mkdir ($clustdir,0777);
Log ("qmanager...\n");
rmtree ($ST::CFG_QMGR_DIR, 0, 1);
mkdir ($ST::CFG_QMGR_DIR,0777);

# Split the top-level node and recurse
Log("Splitting top-level node... ");
my $job = LaunchScript("split.top",
		       ['split_convg.pl',
			4, $clustdir]);
# Wait for it to complete
WaitForScript($job);
Log("done\n");

# Now collect all the terminal nodes
RunScript("collect.pl");

# And merge them (optionally)
