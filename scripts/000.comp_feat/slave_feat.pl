#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 1996-201 Carnegie Mellon University.  All rights 
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
## Author: Jidong Tao <vjdtao@hotmail.com>
#
# ====================================================================
#
#  Script that launches the feature exaction script
#
# ====================================================================

use File::Copy;
use File::Basename;
use File::Spec::Functions;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

mkdir ($ST::CFG_LOG_DIR,0755);

#************************************************************************
# this script launches the feature exaction script
#************************************************************************
$n_parts = (defined($ST::CFG_NPART) ? $ST::CFG_NPART : 1) unless defined $n_parts;

$| = 1; # Turn on autoflushing

Log ("MODULE: 000 Computing feature from audio files\n");
my @jobs;
for (my $i = 1; $i <= $n_parts; $i++) {
    push @jobs, LaunchScript('000.comp_feat', ['make_feats.pl', $i, $n_parts, "$ST::CFG_EXPTNAME.train", $ST::CFG_LISTOFFILES]);
}
for (my $i = 1; $i <= $n_parts; $i++) {
    push @jobs, LaunchScript('000.comp_feat', ['make_feats.pl', $i, $n_parts, "$ST::CFG_EXPTNAME.test", $ST::DEC_CFG_LISTOFFILES]);
}
foreach my $job (@jobs) {
     WaitForScript($job);
}

if ($ST::CFG_VTLN eq "yes") {
    my $outdir = "$ST::CFG_BASE_DIR/feat";
    my $logdir = "$ST::CFG_LOG_DIR/000_comp_feat";

    mkdir($outdir,0777);
    my @vtln_jobs;
		
    $ST::CFG_VTLN_START = 0.80 unless defined($ST::CFG_VTLN_START);
    $ST::CFG_VTLN_END = 1.45 unless defined($ST::CFG_VTLN_END);
    $ST::CFG_VTLN_STEP = 0.05 unless defined($ST::CFG_VTLN_STEP);

    my $vtlnctl = catfile($outdir, "${ST::CFG_EXPTNAME}.vtlnctl");
    my $vtlnlsn = catfile($outdir, "${ST::CFG_EXPTNAME}.vtlnlsn");
    for (my $warp = $ST::CFG_VTLN_START; 
         $warp <= $ST::CFG_VTLN_END; $warp += $ST::CFG_VTLN_STEP) {
	 $warp = sprintf("%.2f", $warp);

	for (my $i = 1; $i <= $n_parts; $i++) {
	    push @vtln_jobs, LaunchScript('000.comp_feat', ['make_feats.pl', $i, $n_parts, "$ST::CFG_EXPTNAME.train", $ST::CFG_LISTOFFILES, $warp]);
	}

	foreach my $vtln_job (@vtln_jobs) {
	    WaitForScript($vtln_job);
	}
    }
}

Log ("Feature extraction is done\n");
