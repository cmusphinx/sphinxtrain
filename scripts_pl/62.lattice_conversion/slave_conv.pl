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

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

$| = 1; # Turn on autoflushing

my $n_parts = defined($ST::CFG_NPART) ? $ST::CFG_NPART : 1;
my $logdir  = "$ST::CFG_LOG_DIR/62.lattice_conversion";

Log("MODULE: 62 Lattice Format Conversion\n");

if ($ST::CFG_MMIE ne "yes") {
    Log("Skipped:  \$ST::CFG_MMIE set to \'$ST::CFG_MMIE\' in sphinx_train.cfg\n");
    exit(0);
}

# mmie training doesn't work for semi-continuous model
if ($ST::CFG_HMM_TYPE eq ".semi.") {
    Log("Skipped:  \$CFG_DIRLABEL = .semi. \n");
    Log("MMIE training only works on the continuous model\n");
    exit(0);
}

# some global variable
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

# make sure the numerator and denominator lattices have been generated
unless (-d $numlatdir) {
    Log("Skipped: No $numlatdir found \n");
    Log("Generate numerator lattices first\n");
    exit 1;
}
unless (-d $denlatdir || -d $pruned_denlatdir) {
    Log("Skipped: No $denlatdir found \n");
    Log("Generate denominator lattices first\n");
    exit 1;
}

# clean up directories
Log("Phase 1: Cleaning up log directory ...\n");
LogProgress("log...");
rmtree($logdir, 0, 1);
mkdir($logdir,0777);
LogProgress("qmanager...");
rmtree($ST::CFG_QMGR_DIR, 0, 1);
mkdir($ST::CFG_QMGR_DIR,0777);
LogProgress("lattice...");
rmtree($latdir, 0, 1);
mkdir($latdir, 0777);
LogProgress("\n");

Log("Phase 2: Converting lattice format ...\n");

# build lattice directory
my $listoffiles;
if (-e "$numlatdir/${ST::CFG_EXPTNAME}.alignedfiles") {
    $listoffiles = "$numlatdir/${ST::CFG_EXPTNAME}.alignedfiles";
}elsif ($ST::CFG_FORCEDALIGN eq "yes") {
    $listoffiles   = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedfiles";
} elsif ($ST::CFG_VTLN eq "yes") {
    $listoffiles   = "$ST::CFG_BASE_DIR/vtlnout/${ST::CFG_EXPTNAME}.alignedfiles";
} else {
    $listoffiles = $ST::CFG_LISTOFFILES;
}

open INPUT,"$listoffiles" or die "Failed to open $listoffiles: $!";
my %dirs;
while (<INPUT>) {
    chomp;
    my @fields = split;
    my $fileid = shift @fields;
    my $basedir = dirname($fileid);
    next if $basedir eq ".";
    unless ($dirs{$basedir}) {
	$dirs{$basedir}++;
	mkpath(catdir($latdir, $basedir), 0, 0777);
    }
}
close INPUT;

# convert lattice format in parts
my @jobs;
for (my $i=1; $i<=$n_parts; $i++) {
    push @jobs, LaunchScript("latconv.$i", ['lattice_conv.pl',  $i, $n_parts]);
}

# check all jobs are done
my $n_done = 0;
my @done;
for(my $i=1; $i<=$n_parts; $i++){

    my $job = shift @jobs;
    WaitForScript($job);
    
    $done[$i] = 0;
    my $input_log = "${logdir}/${ST::CFG_EXPTNAME}.$i.latconv.log";
    next if (! -s $input_log);
    open LOG,$input_log;
    while (<LOG>) {
	if (/.*(ALL DONE).*/) {
	    $n_done++;
	    $done[$i] = 1;
	}
    }
    close LOG;
}

if ($n_done != $n_parts) {
    Log("Only $n_done parts of $n_parts of lattices were successfully converted\n");
    Log("Parts ");
    for (my $i=1; $i<=$n_parts; $i++) {
	Log("$i ") if ($done[$i] == 0);
    }
    Log("failed to run!\n");
    exit 1;
}

exit 0;
