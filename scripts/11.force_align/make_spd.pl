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

my $logdir = "$ST::CFG_LOG_DIR/11.force_align";
my $outdir = "$ST::CFG_BASE_DIR/falignout";

my $transcriptfile = catfile($outdir, "${ST::CFG_EXPTNAME}.alignedtranscripts");
# Create a single pronunciation dictionary and transcript if requested

if ($ST::CFG_FORCE_ALIGN_SPD eq "yes") {
    my $spdict = catfile($outdir, "${ST::CFG_EXPTNAME}.spdict");
    my $sptrans = catfile($outdir, "${ST::CFG_EXPTNAME}.sptranscripts");
    my $logfile = catfile($logdir, "${ST::CFG_EXPTNAME}.spd.log");
    $ENV{PYTHONPATH} .= ':' . File::Spec->catdir($ST::CFG_BASE_DIR, 'python');
    my $rv = RunTool(catfile($ST::CFG_BASE_DIR, 'python', 'cmusphinx', 'dict_spd.py'),
		     $logfile, 0, GetDict(), $transcriptfile, $spdict);
    if ($rv != 0) {
	die "Failed to create single pronunciation dictionary: $rv";
    }
    open LSN, "<$transcriptfile" or die "Failed to open $transcriptfile: $!";
    open OUTLSN, ">$sptrans" or die "Failed to open $sptrans: $!";
    while (<LSN>) {
	s,\(\d\),,g;
	print OUTLSN;
    }
    close OUTLSN;
    close LSN;

    copy($spdict, catfile($ST::CFG_LIST_DIR, "${ST::CFG_EXPTNAME}.spdict"));
    copy($sptrans, catfile($ST::CFG_LIST_DIR, "${ST::CFG_EXPTNAME}.sptranscripts"));
}

exit 0;
