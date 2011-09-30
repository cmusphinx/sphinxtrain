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
my $logdir  = "$ST::CFG_LOG_DIR/60.lattice_generation";

Log("MODULE: 60 Lattice Generation\n");

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

# some global variables
my ($listoffiles, $transcriptfile);
if ($ST::CFG_FORCEDALIGN eq "yes") {
    $listoffiles   = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedfiles";
    $transcriptfile  = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedtranscripts";
} elsif ($ST::CFG_VTLN eq "yes") {
    $listoffiles   = "$ST::CFG_BASE_DIR/vtlnout/${ST::CFG_EXPTNAME}.alignedfiles";
    $transcriptfile  = "$ST::CFG_BASE_DIR/vtlnout/${ST::CFG_EXPTNAME}.alignedtranscripts";
} else {
    $listoffiles = $ST::CFG_LISTOFFILES;
    $transcriptfile = $ST::CFG_TRANSCRIPTFILE;
}

my $numlatdir = defined($ST::CFG_NUMLAT_DIR)
    ? $ST::CFG_NUMLAT_DIR
    : "$ST::CFG_BASE_DIR/numlat";
my $denlatdir = defined($ST::CFG_DENLAT_DIR)
    ? $ST::CFG_DENLAT_DIR
    : "$ST::CFG_BASE_DIR/denlat";

# clean up logs
Log("Phase 1: Cleaning up directories ...\n");
LogProgress("log...");
rmtree($logdir, 0, 1);
mkdir($logdir,0777);
LogProgress("qmanager...");
rmtree($ST::CFG_QMGR_DIR, 0, 1);
mkdir($ST::CFG_QMGR_DIR,0777);
LogProgress("numlat...");
rmtree($numlatdir, 0, 1);
mkdir($numlatdir, 0777);
LogProgress("denlat...");
rmtree($denlatdir, 0, 1);
mkdir($denlatdir, 0777);
LogProgress("\n");

# generate numerator lattices
gennumlat();

# generate denominator lattices
gendenlat();

# generate numerator lattices
sub gennumlat () {
    Log("Phase 2: Generating numerator lattices ...\n");

    unless (-x catfile($ST::CFG_BIN_DIR, "sphinx3_align")
	    or -x catfile($ST::CFG_BIN_DIR, "sphinx3_align.exe")) {
	Log("Skipped: No sphinx3_align(.exe) found in $ST::CFG_BIN_DIR\n");
	Log("If you wish to generate numerator lattices, please copy or link the\n");
	Log("sphinx3_align binary from Sphinx 3 to $ST::CFG_BIN_DIR\n");
	exit 1;
    }

    # build numerator lattice directory
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
	    mkpath(catdir($numlatdir, $basedir), 0, 0777);
	}
    }
    close INPUT;

    # Create a new dictionary unless one is given
    my %silences = ();
    unless (defined($ST::CFG_FORCE_ALIGN_DICTIONARY) or defined($ST::CFG_FORCE_ALIGN_FILLERDICT)) {
	my $dict = "$numlatdir/$ST::CFG_EXPTNAME.falign.dict";
	my $fdict = "$numlatdir/$ST::CFG_EXPTNAME.falign.fdict";
	
	open INFDICT, "<$ST::CFG_FILLERDICT" or die "Failed to open $ST::CFG_FILLERDICT: $!";
	open OUTFDICT, ">$fdict" or die "Failed to open $fdict: $!";
	my %fillers;
	# Strip out all fillers except silence
	while (<INFDICT>) {
	    my ($word, @phones) = split;
	    if (($#phones == 0) and ($phones[0] =~ m,^SIL[be]?$,i)) {
		$silences{$word} = "@phones";
	    print OUTFDICT;
	    }
	    else {
		$fillers{$word} = "@phones";
	    }
	}
	close INFDICT;
	close OUTFDICT;
	
	# Add the extra fillers to the main dictionary
	open INDICT, "<".GetDict() or die "Failed to open ", GetDict(), ": $!";
	open OUTDICT, ">$dict" or die "Failed to open $dict: $!";
	while (<INDICT>) {
	    print OUTDICT;
	} 
        for my $k (sort keys %fillers) {
	    print OUTDICT "$k\t$fillers{$k}\n";
	}
	close INDICT;
	close OUTDICT;
    }

    # Preprocess the transcript to remove extraneous <s> and </s> markers
    my $outtransfile = "$numlatdir/$ST::CFG_EXPTNAME.aligninput";
    open INPUT,"<$transcriptfile" or die "Failed to open $transcriptfile: $!";
    open OUTPUT,">$outtransfile" or die "Failed to open $outtransfile: $!";
    while (<INPUT>) {
	chomp;
	# Remove silences
	foreach my $sil (keys %silences) {
	    s,(\s|^)$sil(\s|$),$1$2,g;
	}
	# These are now done by the above removal of silence
	#s,</?s>,,g;
	#s,</?sil>,,g;
	# Also remove pronunciation variants
	s,\(\d+\),,g;
	# Also normalize whitespaces
	s,^\s+,,;
	s,\s+$,,;
	s,\s+, ,g;
	print OUTPUT $_, "\n";
    }
    close INPUT;
    close OUTPUT;
    
    # Run n_parts of force alignment to get the numerator lattices
    my @jobs;
    for (my $i = 1; $i <= $n_parts; ++$i) {
	push @jobs, LaunchScript("falign.$i", ['force_align.pl', $i, $n_parts]);
    }
    
    # Wait for them all to finish, and concatenate their output
    my $outlistoffiles = catfile($numlatdir, "${ST::CFG_EXPTNAME}.alignedfiles");
    my $outtranscriptfile = catfile($numlatdir, "${ST::CFG_EXPTNAME}.alignedtranscripts");
    open CTL, ">$outlistoffiles" or die "Failed to open $outlistoffiles: $!";
    open LSN, ">$outtranscriptfile" or die "Failed to open $outtranscriptfile: $!";
    
    my $i = 1;
    my $return_value = 0;
    foreach my $job (@jobs) {
	WaitForScript($job);
	my $logfile = catfile($logdir, "${ST::CFG_EXPTNAME}.$i.falign.log");
	open LOG, "<$logfile" or die "Failed to open $logfile: $!";
	my $success = 0;
	while (<LOG>) {
	    if (/TOTAL FRAMES/) {
		$success = 1;
		last;
	    }
	}
	unless ($success) {
	    Log("Failed in part $i");
	    $return_value = 1;
	    next;
	}
	close LOG;
	
	my $part = catfile($numlatdir, "${ST::CFG_EXPTNAME}.alignedtranscripts.$i");
	open INPUT, "<$part" or die "Failed to open $part: $!";
	open INCTL,"$listoffiles" or die "Failed to open $listoffiles: $!";
	while (<INPUT>) {
	    my ($uttid) = (/\(([^\(\)]+)\)$/);
	    print LSN;
	    # Now scan forward in the control past any utterances that were missed
	    while (defined(my $ctlline = <INCTL>)) {
		# For whatever reason, the full path isn't in the align output
		chomp(my $ctl_uttid = (split " ", $ctlline)[-1]);
		if ($ctl_uttid =~ /$uttid$/) {
		    print CTL $ctlline;
		    last;
		}
	    }
	}
	++$i;
    }
    if ($return_value) {
	exit $return_value;
    }
}

# generate denominator lattices
sub gendenlat() {
    Log("Phase 3: Generating denominator lattices ...\n");

    unless (-x catfile($ST::CFG_BIN_DIR, "sphinx3_decode")
	    or -x catfile($ST::CFG_BIN_DIR, "sphinx3_decode.exe")) {
	Log("Skipped: No sphinx3_decode(.exe) found in $ST::CFG_BIN_DIR\n");
	Log("If you wish to generate denominator lattices, please copy or link the\n");
	Log("sphinx3_decode binary from Sphinx 3 to $ST::CFG_BIN_DIR\n");
	exit 1;
    }

    my $ctlfile;
    if (-e "$numlatdir/${ST::CFG_EXPTNAME}.alignedfiles") {
	$ctlfile = "$numlatdir/${ST::CFG_EXPTNAME}.alignedfiles";
    } else {
	$ctlfile = $listoffiles;
    }

    # build denominator directories
    open INPUT,"$ctlfile" or die "Failed to open $ctlfile: $!";
    my %dirs;
    while (<INPUT>) {
        chomp;
        my @fields = split;
        my $fileid = shift @fields;
        my $basedir = dirname($fileid);
        next if $basedir eq ".";
        unless ($dirs{$basedir}) {
            $dirs{$basedir}++;
            mkpath(catdir($denlatdir, $basedir), 0, 0777);
        }
    }
    close INPUT;

    # decode training data in parts to generate denominator lattices
    my @jobs;
    for (my $i=1; $i<=$n_parts; $i++) {
        push @jobs, LaunchScript("decode.$i", ['s3decode.pl',  $i, $n_parts]);
    }

    # check all jobs are done
    my $n_done = 0;
    my @done;
    for(my $i=1; $i<=$n_parts; $i++){

        my $job = shift @jobs;
        WaitForScript($job);

        $done[$i] = 0;
        my $input_log = "${logdir}/${ST::CFG_EXPTNAME}.$i.decode.log";
        next if (! -s $input_log);
        open LOG,$input_log;
        while (<LOG>) {
            if (/.*(SUMMARY:).*/) {
                $n_done++;
                $done[$i] = 1;
            }
        }
        close LOG;
    }

    if ($n_done != $n_parts) {
        Log("Only $n_done parts of $n_parts of denominator lattices were successfully generated\n");
        Log("Parts ");
        for (my $i=1; $i<=$n_parts; $i++) {
            Log("$i ") if ($done[$i] == 0);
        }
        Log("failed to run!\n");
        exit 1;
    }
}

exit 0;
