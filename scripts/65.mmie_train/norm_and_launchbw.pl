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
## Modified: Long Qin
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

my $logdir = "$ST::CFG_LOG_DIR/65.mmie_train";
my $log    = "$logdir/${ST::CFG_EXPTNAME}.$iter.norm.log";
mkdir ($logdir,0777);

# Check the number and list of parts done
my $n_done = 0;
my $tot_utt = 0;
my $tot_pp = 0;
my @den_done;
for (my $i=1; $i<=$n_parts; $i++){
    $den_done[$i] = 0;
    my $input_log = "${logdir}/${ST::CFG_EXPTNAME}.${iter}-${i}.bw.denlat.log";
    next if (! -s $input_log);
    open LOG,$input_log;
    while (<LOG>) {
        if (/.*(Counts saved to).*/) {
            $n_done++;
            $den_done[$i] = 1;
        }
	if (/.*(overall>).*/){
	    my (undef, undef, $nutt, undef, undef, $pp) = split(/ /);
	    $tot_pp = $tot_pp + $pp;
	    $tot_utt = $tot_utt + $nutt;
	}
    }
    close LOG;
}

if ($n_done != $n_parts) {
    print OUTPUT "Only $n_done parts of $n_parts of Baum Welch on the denominator lattices were successfully completed\n";
    my $errmsg = "Parts ";
    for (my $i=1; $i<=$n_parts; $i++) {
        $errmsg .= "denlat:$i " if ($den_done[$i] == 0);
    }
    $errmsg .= "failed to run!\n";
    open OUTPUT,">$log";
    print OUTPUT "Only $n_done parts of $n_parts of Baum Welch on the denominator lattices were successfully completed\n"; 
    print OUTPUT $errmsg;
    close OUTPUT;
    LogError("Only $n_done parts of $n_parts of Baum Welch on the denominator lattices were successfully completed");
    LogError($errmsg);
    exit 1;
}

if ($tot_utt == 0) {
    open OUTPUT,">$log";
    print OUTPUT "Baum welch ran successfully for only 0 utterance! Aborting..\n";
    close OUTPUT;
    exit 1;
}

# run norm script
RunScript('norm.pl', $iter);
open LOG, $log;
while (<LOG>) {
    if (/.*(ERROR).*/) {
        close LOG;
        Log("norm.pl failed to run!!! Please check the log file for details\n");
        exit 1;
    }
}

# Compute average posterior probability per utt
my $pp_per_utt = $tot_pp / $tot_utt;
print OUTPUT "Current Overall Log Posterior Probability Per Utterance = $pp_per_utt\n";
open OUTPUT, ">> $log";
print OUTPUT "Current Overall Log Posterior Probability Per Utterance = $pp_per_utt\n";
close OUTPUT;

if ($iter < $ST::CFG_MMIE_MAX_ITERATIONS) {
    Launch_BW($iter);
    exit (0);
}
else {
    open OUTPUT, ">> $log";
    print OUTPUT "Maximum desired iterations $ST::CFG_MMIE_MAX_ITERATIONS performed. Terminating MMIE training\n";
    print OUTPUT "******************************TRAINING COMPLETE*************************\n";
    my $date = localtime;
    print OUTPUT "$date\n";
    Log("Maximum desired iterations $ST::CFG_MMIE_MAX_ITERATIONS performed. Terminating MMIE training",
	'result');
    close OUTPUT;
    exit (0);
}

sub Launch_BW {
    my $iter = shift;
    my $newiter = $iter + 1;
    RunScript('slave_convg.pl', $newiter, $n_parts);
}
