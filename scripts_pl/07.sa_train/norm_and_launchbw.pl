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
## Modified: Rita Singh, 27 Nov 2000
## Author: Ricky Houghton 
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

#*******************************************************************
#*******************************************************************
$| = 1; # Turn on autoflushing

die "USAGE: $0 <iter>" if (@ARGV < 1);

my ($iter) = @ARGV;

my $processname="07.sa_train";

my $logdir ="$ST::CFG_LOG_DIR/$processname";
mkdir ($logdir,0777) unless -d $logdir;
my $log = "$logdir/${ST::CFG_EXPTNAME}.$iter.norm.log";

my @speakers;
# Read list of speakers
open SPEAKER, "<$ST::CFG_SPEAKERLIST" or die "Failed to open $ST::CFG_SPEAKERLIST: $!";
chomp(@speakers = <SPEAKER>);
close SPEAKER;
my @parts = map "$iter-$_", @speakers;

# Check the number and list of parts done. Compute avg likelihood per frame
my $num_done = 0; my $tot_lkhd = 0; my $tot_frms = 0;
my %done;
foreach (@parts) {
    my $input_log = "${logdir}/${ST::CFG_EXPTNAME}.$_.bw.log";
    next if (! -s $input_log);
    open LOG,$input_log;
    while (<LOG>) {
	if (/.*(Counts saved to).*/) {
	    $num_done++;
	    $done{$_} = 1;
	}
	if (/.*(overall>).*/) {
	    my (undef, undef, $nfrms, undef, undef, $lkhd) = split(/ /);
	    $tot_lkhd = $tot_lkhd + $lkhd;
	    $tot_frms = $tot_frms + $nfrms;
	}
    }
    close LOG;
}

my $n_parts = @parts;
if ($num_done != $n_parts) {
    open OUTPUT,">$log";
    print OUTPUT "Only $num_done parts of $n_parts of Baum Welch were successfully completed\n";
    print "Parts ";
    foreach (@parts) {
        print OUTPUT "$_ " unless $done{$_};
    }
    print OUTPUT "failed to run!\n";
    close OUTPUT;
    exit (0);
}

if ($tot_frms == 0) {
    open OUTPUT,">$log";
    print OUTPUT "Baum welch ran successfully for only 0 frames! Aborting..\n";
    close OUTPUT;
    exit (0);
}

my $hmmdir = catdir($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.sat_$ST::CFG_DIRLABEL");

# Inverse-transform Gaussian occupancy counts using previous iteration's MLLR
if ($iter > 1) {
    Log("Inverse transform of Gaussian counts... ");

    foreach my $speaker (@speakers) {
	Log(" $speaker");
	my $mllrfile = catfile($hmmdir, "$ST::CFG_EXPTNAME.$speaker.mllr");
	my $bwaccumdir = "$ST::CFG_BASE_DIR/bwaccumdir/${ST::CFG_EXPTNAME}_buff_${speaker}";
	my $gaucntfn = catfile($bwaccumdir, 'gauden_counts');
	my $logfile  = "$logdir/${ST::CFG_EXPTNAME}.$iter-$speaker.mllr_transform.log";
	my $rv = RunTool
	    ('mllr_transform', $logfile, 0,
	     -ingaucntfn => $gaucntfn,
	     -outgaucntfn => "$gaucntfn.new",
	     -inverse => 'yes',
	     -mllrmat => $mllrfile);
	# FIXME: Support multi-class MLLR
	exit $rv if $rv;
    }
    Log("\n");
}

# Estimate new MLLR transformations
Log("Estimating speaker MLLR transforms... ");
foreach my $speaker (@speakers) {
    # Run mllr_solve
    Log(" $speaker");
    my $inmodeldir = ($iter == 1) ?
	catfile($ST::CFG_MODEL_DIR,
		"${ST::CFG_EXPTNAME}.cd_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}")
	    : $hmmdir;
    my $mllrfile = catfile($hmmdir, "$ST::CFG_EXPTNAME.$speaker.mllr");
    my $logfile  = "$logdir/${ST::CFG_EXPTNAME}.$iter-$speaker.mllr_solve.log";
    # FIXME: Support multi-class MLLR
    my $rv = RunTool
	('mllr_solve', $logfile, 0,
	 -outmllrfn => $mllrfile,
	 -accumdir => catfile($ST::CFG_BASE_DIR,
			      'bwaccumdir', "${ST::CFG_EXPTNAME}_buff_${speaker}"),
	 -meanfn => catfile($inmodeldir, 'means'),
	 -varfn => catfile($inmodeldir, 'variances'));
    exit $rv if $rv;
}
Log("\n");

# Now overwrite the old Gaussian counts with the inverse-transform ones
if ($iter > 1) {
    Log("Updating Gaussian count files... ");
    foreach my $speaker (@speakers) {
	Log(" $speaker");
	my $mllrfile = catfile($hmmdir, "$ST::CFG_EXPTNAME.$speaker.mllr");
	my $bwaccumdir = "$ST::CFG_BASE_DIR/bwaccumdir/${ST::CFG_EXPTNAME}_buff_${speaker}";
	my $gaucntfn = catfile($bwaccumdir, 'gauden_counts');
	unlink($gaucntfn) or die "Failed to remove $gaucntfn: $!";
	rename("$gaucntfn.new", $gaucntfn)
	    or die "Failed to rename $gaucntfn.new to $gaucntfn: $!";
    }
    Log("\n");
}

my $lkhd_per_frame = $tot_lkhd/$tot_frms;
my $previter = $iter - 1;
my $prev_norm = "${logdir}/${ST::CFG_EXPTNAME}.${previter}.norm.log";
if (! -s $prev_norm) {
    # Either iter == 1 or we are starting from an intermediate iter value
    RunScript('norm.pl', $iter);
    open OUTPUT, ">> $log";
    print OUTPUT "Current Overall Likelihood Per Frame = $lkhd_per_frame\n";
    close OUTPUT;
    Launch_BW($iter);
    exit (0);
}

# Find the likelihood from the previous iteration
open LOG,$prev_norm; my $prevlkhd = -99999999;
while (<LOG>) {
   if (/.*(Current Overall Likelihood Per Frame).*/){
      (undef,undef,undef,undef,undef,undef,$prevlkhd) = split(/ /);
   }
}
close LOG;

if ($prevlkhd == -99999999) {
    # Some error with previous norm.log. Continue Baum Welch
    RunScript('norm.pl', $iter);
    open OUTPUT, ">> $log";
    print OUTPUT "Current Overall Likelihood Per Frame = $lkhd_per_frame\n";
    close OUTPUT;
    Launch_BW($iter);
    exit (0);
}

my ($convg_ratio, $absprev);
if ($prevlkhd == 0) {
    $convg_ratio = 0;
    $convg_ratio = 1 if ($lkhd_per_frame > 0);
    $convg_ratio = -1 if ($lkhd_per_frame < 0);
}
else {
    $absprev = $prevlkhd;
    $absprev = -$absprev if ($prevlkhd < 0);
    $convg_ratio = ($lkhd_per_frame - $prevlkhd)/$absprev;
}
RunScript('norm.pl', $iter);

open OUTPUT, ">> $log";
print OUTPUT "Current Overall Likelihood Per Frame = $lkhd_per_frame\n";
if (defined($convg_ratio)) {
    print OUTPUT "Convergence ratio = $convg_ratio\n";
}

if ($convg_ratio < 0) {
    print OUTPUT "*WARNING*: NEGATIVE CONVERGENCE RATIO! CHECK YOUR DATA AND TRASNCRIPTS\n";
    print "*WARNING*: NEGATIVE CONVERGENCE RATIO AT ITER ${iter}! CHECK BW AND NORM LOGFILES\n";
}

if ($convg_ratio > $ST::CFG_CONVERGENCE_RATIO && $iter >= $ST::CFG_MAX_ITERATIONS) {
    open OUTPUT, ">> $log";
    print OUTPUT "Maximum desired iterations $ST::CFG_MAX_ITERATIONS performed. Terminating CI training\n";
    print OUTPUT "******************************TRAINING COMPLETE*************************\n";
    my $date = localtime;
    print OUTPUT "$date\n";
    print "Maximum desired iterations $ST::CFG_MAX_ITERATIONS performed. Terminating CI training\n";
    close OUTPUT;
    exit (0);
}

if ($convg_ratio > $ST::CFG_CONVERGENCE_RATIO) {
    Launch_BW($iter);
    exit (0);
}
else {
    print "        Current Overall Likelihood Per Frame = $lkhd_per_frame\n";
    open OUTPUT, ">> $log";
    print OUTPUT "Likelihoods have converged! Baum Welch training completed\!\n";
    print OUTPUT "******************************TRAINING COMPLETE*************************\n";
    my $date = localtime;
    print OUTPUT "$date\n";
    close OUTPUT;
    exit (0);
}

sub Launch_BW {
    my $iter = shift;
    my $newiter = $iter + 1;
    print "        Current Overall Likelihood Per Frame = $lkhd_per_frame\n";
    print "        Convergence Ratio = $convg_ratio\n" if defined $convg_ratio;
    RunScript('slave_convg.pl', $newiter);
}

