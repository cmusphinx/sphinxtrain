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
## Author: Long Qin
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

die "Usage: $0 <part> <nparts>\n" unless @ARGV == 2;
my ($part, $npart) = @ARGV;

my $logdir   = "$ST::CFG_LOG_DIR/62.lattice_conversion";
my $logfile  = "$logdir/${ST::CFG_EXPTNAME}.$part.latconv.log";

my $numlatdir = defined($ST::CFG_NUMLAT_DIR)
    ? $ST::CFG_NUMLAT_DIR
    : "$ST::CFG_BASE_DIR/numlat";
my $denlatdir = defined($ST::CFG_DENLAT_DIR)
    ? $ST::CFG_DENLAT_DIR
    : "$ST::CFG_BASE_DIR/denlat";
my $pruned_denlatdir = defined($ST::CFG_PRUNED_DENLAT_DIR)
    ? $ST::CFG_PRUNED_DENLAT_DIR
    : "$ST::CFG_BASE_DIR/pruned_denlat";
my $latdir = defined($ST::CFG_LATTICE_DIR)
    ? $ST::CFG_LATTICE_DIR
    : "$ST::CFG_BASE_DIR/lattice";

my $lmfile     = $ST::CFG_LANGUAGEMODEL;
my $fillerfile = $ST::CFG_FILLERDICT;

my $in_denlatdir;
if (-d $pruned_denlatdir) {
    $in_denlatdir = $pruned_denlatdir;
} else {
    $in_denlatdir = $denlatdir;
}

my $filelst;
if (-e "$numlatdir/${ST::CFG_EXPTNAME}.alignedfiles") {
    $filelst = "$numlatdir/${ST::CFG_EXPTNAME}.alignedfiles";
}elsif ($ST::CFG_FORCEDALIGN eq "yes") {
    $filelst = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedfiles";
} elsif ($ST::CFG_VTLN eq "yes") {
    $filelst = "$ST::CFG_BASE_DIR/vtlnout/${ST::CFG_EXPTNAME}.alignedfiles";
} else {
    $filelst = $ST::CFG_LISTOFFILES;
}

# Get the number of utterances
open INPUT,"$filelst" or die "Failed to open $filelst: $!";
my $filecount = 0;
while (<INPUT>) {
    $filecount++;
}
close INPUT;
$filecount = int ($filecount / $npart) if $npart;
$filecount = 1 unless ($filecount);
my $fileoffset = $filecount * ($part-1);


# Add PYTHONPATH
$ENV{PYTHONPATH} .= ':' . File::Spec->catdir($ST::CFG_SPHINXTRAIN_DIR, 'python');
my $rv = RunTool(catfile($ST::CFG_SPHINXTRAIN_DIR, 'python', 'cmusphinx', 'lattice_conv.py'),
		 $logfile, $filecount,
		 $lmfile, $fillerfile, $filelst, $filecount, $fileoffset,
		 $in_denlatdir, $numlatdir, $latdir);

if ($rv) {
    LogError("Failed to run lattice_conv.py");
}
exit $rv;
