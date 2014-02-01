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

my ($iter, $n_parts) = @ARGV;
$iter = 1 unless defined $iter;
$n_parts = (defined($ST::CFG_NPART) ? $ST::CFG_NPART : 1) unless defined $n_parts;

$| = 1; # Turn on autoflushing

my $logdir = "$ST::CFG_LOG_DIR/12.vtln_align";
my $outdir = "$ST::CFG_BASE_DIR/vtlnout";

Log("MODULE: 12 Force-aligning data for VTLN\n");

if ($ST::CFG_VTLN ne "yes") {
    Log("Skipped:  \$ST::CFG_VTLN set to \'$ST::CFG_VTLN\' in sphinx_train.cfg\n");
    exit(0);
}

unless (-x catfile($ST::CFG_BIN_DIR, "sphinx3_align")
	or -x catfile($ST::CFG_BIN_DIR, "sphinx3_align.exe")) {
    Log("Skipped: No sphinx3_align(.exe) found in $ST::CFG_BIN_DIR\n");
    Log("If you wish to do force-alignment, please copy or link the\n");
    Log("sphinx3_align binary from Sphinx 3 to $ST::CFG_BIN_DIR\n");
    Log("and either define \$CFG_MODEL_DIR in sphinx_train.cfg or\n");
    Log("run context-independent training first.\n");
    exit 1;
}

unless (defined($ST::CFG_FORCE_ALIGN_MODELDIR)
	or -f "$ST::CFG_MODEL_DIR/$ST::CFG_EXPTNAME.falign_ci_$ST::CFG_DIRLABEL/means") {
    Log("Skipped: No acoustic models available for force alignment\n");
    Log("If you wish to do force-alignment, please define \$CFG_MODEL_DIR\n");
    Log("in sphinx_train.cfg or run context-independent training first.\n");
    exit 0;
}

Log("Phase 1: Cleaning up directories:");
LogProgress("logs...");
rmtree($logdir, 0, 1);
mkdir($logdir,0777);
LogProgress("qmanager...");
rmtree($ST::CFG_QMGR_DIR, 0, 1);
mkdir($ST::CFG_QMGR_DIR,0777);
LogProgress("output...");
rmtree($outdir, 0, 1);
mkdir($outdir,0777);

LogProgress("\n");

my %silences = ();
# Create a new dictionary unless one is given
unless (defined($ST::CFG_FORCE_ALIGN_DICTIONARY) or defined($ST::CFG_FORCE_ALIGN_FILLERDICT)) {
    my $dict = "$outdir/$ST::CFG_EXPTNAME.falign.dict";
    my $fdict = "$outdir/$ST::CFG_EXPTNAME.falign.fdict";

    Log("Phase 2: Creating dictionary for alignment...");
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
my $dictfn = GetDict();

    # Add the extra fillers to the main dictionary
    open INDICT, "<$dictfn" or die "Failed to open $dictfn: $!";
    open OUTDICT, ">$dict" or die "Failed to open $dict: $!";
    while (<INDICT>) {
	print OUTDICT;
    }
    while (my ($k, $v) = each %fillers) {
	print OUTDICT "$k\t$v\n";
    }
    close INDICT;
    close OUTDICT;
}

# Preprocess the transcript to remove extraneous <s> and </s> markers (argh)
my $transcriptfile = "$outdir/$ST::CFG_EXPTNAME.aligninput";
Log("Phase 3: Creating transcript for alignment...");
open INPUT,"<$ST::CFG_TRANSCRIPTFILE" or die "Failed to open $ST::CFG_TRANSCRIPTFILE: $!";
open OUTPUT,">$transcriptfile" or die "Failed to open $transcriptfile: $!";
while (<INPUT>) {
    chomp;
    # Remove silences
    foreach my $sil (keys %silences) {
        s/$sil//g;
    }
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

# Now do force-alignment for each warping factor
$ST::CFG_VTLN_START = 0.80 unless defined($ST::CFG_VTLN_START);
$ST::CFG_VTLN_END = 1.45 unless defined($ST::CFG_VTLN_END);
$ST::CFG_VTLN_STEP = 0.05 unless defined($ST::CFG_VTLN_STEP);
# Update a global list of files with warping factors and likelihoods
my $vtlnctl = catfile($outdir, "${ST::CFG_EXPTNAME}.vtlnctl");
my $vtlnlsn = catfile($outdir, "${ST::CFG_EXPTNAME}.vtlnlsn");
for (my $warp = $ST::CFG_VTLN_START;
     $warp <= $ST::CFG_VTLN_END; $warp += $ST::CFG_VTLN_STEP) {
    $warp = sprintf("%.2f", $warp);

    Log("Phase 4: Running force alignment in  $n_parts parts\n");
    my @jobs;
    for (my $i = 1; $i <= $n_parts; ++$i) {

	push @jobs, LaunchScript("falign.$warp.$i", ['vtln_align.pl', $warp, $i, $n_parts]);
    }

    # Wait for them all to finish, and collect their likelihoods
    my $newctl = "$vtlnctl.$warp";
    open CTL, ">$newctl" or die "Failed to open $newctl: $!";
    my $newlsn = "$vtlnlsn.$warp";
    open LSN, ">$newlsn" or die "Failed to open $newlsn: $!";

    my $i = 1;
    my $return_value = 0;
    foreach my $job (@jobs) {
	WaitForScript($job);

	# Look in the logfile to make sure it succeeded, and to find the
	# offset and length of this part in the original control file.
	my $logfile = catfile($logdir, "${ST::CFG_EXPTNAME}.$warp.$i.vtln.log");
	open LOG, "<$logfile" or die "Failed to open $logfile: $!";
	my $success = 0;
	my ($ctlcount, $ctloffset);
	while (<LOG>) {
	    if (/-ctlcount\s+\d+\s+(\d+)/) {
		$ctlcount = $1;
	    }
	    if (/-ctloffset\s+\d+\s+(\d+)/) {
		$ctloffset = $1;
	    }
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

	# Now append to the control file and transcript from this part of the output
	open INCTL, "<${ST::CFG_LISTOFFILES}" or die "Failed to open $ST::CFG_LISTOFFILES: $!";
	my $part = catfile($outdir, $warp, "${ST::CFG_EXPTNAME}.alignedtranscripts.$i");
	open INLSN, "<$part" or die "Failed to open $part: $!";
	my $j = 0;
	while (<INCTL>) {
	    chomp(my $fileid = $_);
	    next if $j < $ctloffset;
	    last if $j - $ctloffset >= $ctlcount;

	    my $wf = catfile($outdir, $warp, "$fileid.wdseg");
	    my $found = 0;
	    if (open WDSEG, "<$wf") {
		while (<WDSEG>) {
		    if (/Total score:\s+(\S+)/) {
			my $transcript = <INLSN>;
			print CTL "$fileid $warp $1\n";
			print LSN $transcript;
			$found = 1;
			last;
		    }
		}
		close WDSEG;
	    }
	    unless ($found) {
		# Put some placeholders in if alignment failed
		print CTL "$fileid $warp FAIL\n";
		print LSN "**FAIL** ($fileid)\n";
	    }
	}
	continue {
	    ++$j;
	}
	close INCTL;
	close INLSN;
	++$i;
    }
    close LSN;
    close CTL;
    # Fail if failure
    exit $return_value if $return_value;

    # Now update the master control file and transcript
    Log("Phase 5: Updating master control file and transcript\n");
    my $newvtlnctl = "$vtlnctl.new";
    my $newvtlnlsn = "$vtlnlsn.new";
    # A lot of filehandles here!
    open CTL, "<$newctl" or die "Failed to open $newctl: $!";
    open LSN, "<$newlsn" or die "Failed to open $newlsn: $!";
    open NEWCTL, ">$newvtlnctl" or die "Failed to open $newvtlnctl: $!";
    open NEWLSN, ">$newvtlnlsn" or die "Failed to open $newvtlnlsn: $!";
    if (open VTLNCTL, "<$vtlnctl") {
	open VTLNLSN, "<$vtlnlsn" or die "Failed to open $vtlnlsn: $!";
	while (<VTLNCTL>) {
	    chomp(my $prevline = $_);
	    chomp(my $newline = <CTL>);
	    chomp(my $prevtrans = <VTLNLSN>);
	    chomp(my $newtrans = <LSN>);
	    my ($pu, $pw, $pl) = split " ", $prevline;
	    my ($nu, $nw, $nl) = split " ", $newline;
	    die "Utterance ID mismatch: $pu != $nu, shouldn't happen!"
		unless $pu eq $nu;
	    if ($pl eq 'FAIL' or ($nl ne 'FAIL' and $nl > $pl)) {
		print NEWCTL $newline, "\n";
		print NEWLSN $newtrans, "\n";
		# Remove old feature file to save space
		my $feat = catfile($ST::CFG_FEATFILES_DIR,
				   "$pu.$pw.$ST::CFG_FEATFILE_EXTENSION");
		unlink $feat;
	    }
	    else {
		print NEWCTL $prevline, "\n";
		print NEWLSN $prevtrans, "\n";
		# Remove new feature file to save space
		my $feat = catfile($ST::CFG_FEATFILES_DIR,
				   "$nu.$nw.$ST::CFG_FEATFILE_EXTENSION");
		unlink $feat;
	    }
	}
	close VTLNLSN;
    }
    else {
	# Otherwise just copy the initial control file
	while (<CTL>) {
	    print NEWCTL;
	    my $trans = <LSN>;
	    print NEWLSN $trans;
	}
    }
    close CTL;
    close LSN;
    close NEWCTL;
    close NEWLSN;
    rename $newvtlnctl, $vtlnctl or die "Failed to update $vtlnctl: $!";
    rename $newvtlnlsn, $vtlnlsn or die "Failed to update $vtlnlsn: $!";
}

# Now convert the VTLN control to a normal one
open INCTL, "${ST::CFG_LISTOFFILES}" or die "Failed to open $ST::CFG_LISTOFFILES: $!";
open VTLNCTL, "<$vtlnctl" or die "Failed to open $vtlnctl: $!";
open VTLNLSN, "<$vtlnlsn" or die "Failed to open $vtlnlsn: $!";
my $outctl = catfile($outdir, "${ST::CFG_EXPTNAME}.alignedfiles");
open OUTCTL, ">$outctl" or die "Failed to open $outctl: $!";
my $outlsn = catfile($outdir, "${ST::CFG_EXPTNAME}.alignedtranscripts");
open OUTLSN, ">$outlsn" or die "Failed to open $outlsn: $!";
while (<VTLNCTL>) {
    chomp(my $vtlnline = $_);
    chomp(my $vtlntrans = <VTLNLSN>);
    chomp(my $inline = <INCTL>);
    my ($fileid, $warp, $likelihood) = split " ", $vtlnline;
    next if $likelihood eq 'FAIL';

    my @fields = split " ", $inline;
    die "Utterance ID mismatch: $fileid != $fields[0], shouldn't happen!"
	unless $fileid eq $fields[0];
    $fields[0] = catdir("$warp","$fileid");
    print OUTCTL "@fields\n";
    print OUTLSN "$vtlntrans\n";
}
close OUTCTL;
close OUTLSN;
close VTLNCTL;
close VTLNLSN;
close INCTL;

exit 0;
