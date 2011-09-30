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

die "USAGE: $0 <iter> <n_parts>" if (@ARGV != 2);

my ($iter, $n_parts) = @ARGV;

my $processname="01.lda_train";

my $logdir ="$ST::CFG_LOG_DIR/$processname";
mkdir ($logdir,0777);
my $log = "$logdir/${ST::CFG_EXPTNAME}.$iter.norm.log";

# Check the number and list of parts done. Compute avg likelihood per frame
my $num_done = 0; my $tot_lkhd = 0; my $tot_frms = 0;
my @done;
for (my $i=1;$i<=$n_parts;$i++){
    $done[$i] = 0;
    my $input_log = "${logdir}/${ST::CFG_EXPTNAME}.${iter}-${i}.bw.log";
    next if (! -s $input_log);
    open LOG,$input_log;
    while (<LOG>) {
        if (/.*(Counts saved to).*/) {
            $num_done++;
            $done[$i] = 1;
        }
        if (/.*(overall>).*/){
            my (undef, undef, $nfrms, undef, undef, $lkhd) = split(/ /);
            $tot_lkhd = $tot_lkhd + $lkhd;
            $tot_frms = $tot_frms + $nfrms;
        }
    }
    close LOG;
}

if ($num_done != $n_parts) {
    print OUTPUT "Only $num_done parts of $n_parts of Baum Welch were successfully completed\n";
    my $errmsg = "Parts ";
    for (my $i=1;$i<=$n_parts;$i++) {
        $errmsg .= "$i " if ($done[$i] == 0);
    }
    $errmsg .= "failed to run!\n";
    open OUTPUT,">$log";
    print OUTPUT "Only $num_done parts of $n_parts of Baum Welch were successfully completed\n";
    print OUTPUT $errmsg;
    close OUTPUT;
    LogError("Only $num_done parts of $n_parts of Baum Welch were successfully completed");
    LogError($errmsg);
    exit (0);
}

if ($tot_frms == 0) {
    open OUTPUT,">$log";
    print OUTPUT "Baum welch ran successfully for only 0 frames! Aborting..\n";
    close OUTPUT;
    exit (0);
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
    Launch_BW($iter, $iter + 1);
    exit (0);
}

# Find the likelihood from the previous iteration
open LOG, $prev_norm; my $prevlkhd = -99999999;
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
    Launch_BW($iter, $iter + 1);
    exit (0);
}

my $convg_ratio;
if ($prevlkhd == 0) {
    $convg_ratio = 0;
    $convg_ratio = 1 if ($lkhd_per_frame > 0);
    $convg_ratio = -1 if ($lkhd_per_frame < 0);
}
else {
    $convg_ratio = $lkhd_per_frame - $prevlkhd;
}
RunScript('norm.pl', $iter);

open OUTPUT, ">> $log";
print OUTPUT "Current Overall Likelihood Per Frame = $lkhd_per_frame\n";
if (defined($convg_ratio)) {
    print OUTPUT "Convergence ratio = $convg_ratio\n";
}

if ($convg_ratio < 0) {
    print OUTPUT "*WARNING*: NEGATIVE CONVERGENCE RATIO! CHECK YOUR DATA AND TRASNCRIPTS\n";
    LogWarning("*WARNING*: NEGATIVE CONVERGENCE RATIO AT ITER ${iter}! CHECK BW AND NORM LOGFILES");
}

if ($convg_ratio > $ST::CFG_CONVERGENCE_RATIO && $iter >= $ST::CFG_MAX_ITERATIONS) {
    open OUTPUT, ">> $log";
    print OUTPUT "Maximum desired iterations $ST::CFG_MAX_ITERATIONS performed. Terminating training iteration\n";
    my $date = localtime;
    print OUTPUT "$date\n";
    Log("Maximum desired iterations $ST::CFG_MAX_ITERATIONS performed. Terminating CI training for LDA",
	'result');
    close OUTPUT;
    # Do the last iteration
    Launch_BW($iter, 'N');
    exit (0);
}

if ($convg_ratio > $ST::CFG_CONVERGENCE_RATIO or $iter < $ST::CFG_MIN_ITERATIONS) {
    Launch_BW($iter, $iter + 1);
    exit (0);
}
else {
    Log("Current Overall Likelihood Per Frame = $lkhd_per_frame", 'result');
    open OUTPUT, ">> $log";
    print OUTPUT "Likelihoods have converged! CI training for LDA completed\!\n";
    my $date = localtime;
    print OUTPUT "$date\n";
    close OUTPUT;
    # Do the last iteration
    Launch_BW($iter, 'N');
    exit (0);
}

sub Launch_BW {
    my ($iter, $newiter) = @_;
    Log("Current Overall Likelihood Per Frame = $lkhd_per_frame", 'result');
    Log("Convergence Ratio = $convg_ratio", 'result') if defined $convg_ratio;
    RunScript('slave_lda.pl', $newiter, $n_parts);
}
