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
## 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
##    endorse or promote products derived from this software without
##    prior written permission. To obtain permission, contact 
##    sphinx@cs.cmu.edu.
##
## 4. Products derived from this software may not be called "Sphinx"
##    nor may "Sphinx" appear in their names without prior written
##    permission of Carnegie Mellon University. To obtain permission,
##    contact sphinx@cs.cmu.edu.
##
## 5. Redistributions of any form whatsoever must retain the following
##    acknowledgment:
##    "This product includes software developed by Carnegie
##    Mellon University (http://www.speech.cs.cmu.edu/)."
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
## Author: Ricky Houghton (converted from scripts by Rita Singh)
##

my $index = 0;
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    $index = 2;
} else {
    $cfg_file = "etc/sphinx_train.cfg";
}

if (! -s "$cfg_file") {
    print ("unable to find default configuration file, use -cfg file.cfg or create etc/sphinx_train.cfg for default\n");
    exit -3;
}
require $cfg_file;

#*******************************************************************
#*******************************************************************

die "USAGE: $0 <iter>" if ($#ARGV != $index);

$iter = $ARGV[$index];

mkdir ($CFG_CI_LOG_DIR,0777) unless -d $CFG_CI_LOG_DIR;

$log = "$CFG_CI_LOG_DIR/${CFG_EXPTNAME}.$iter.norm.log";

# Check the number and list of parts done. Compute avg likelihood per frame
$num_done = 0; $tot_lkhd = 0; $tot_frms = 0;
for ($i=1;$i<=$CFG_NPART;$i++){
    $done[$i] = 0;
    $input_log = "${CFG_CI_LOG_DIR}/${CFG_EXPTNAME}.${iter}-${i}.bw.log";
    next if (! -s $input_log);
    open LOG,$input_log;
    while (<LOG>) {
        if (/.*(Counts saved to).*/) {
            $num_done++;
            $done[$i] = 1;
        }
        if (/.*(overall>).*/){
            ($jnk,$jnk,$nfrms,$jnk,$jnk,$lkhd) = split(/ /);
            $tot_lkhd = $tot_lkhd + $lkhd;
            $tot_frms = $tot_frms + $nfrms;
        }
    }
    close LOG;
}

if ($num_done != $CFG_NPART) {
    open OUTPUT,">$log";
    print "Only $num_done parts of $CFG_NPART of Baum Welch were successfully completed\n";
    print "Parts ";
    for ($i=1;$i<=$CFG_NPART;$i++) {
        print "$i " if ($done[$i] == 0);
    }
    print "failed to run!\n";
    close OUTPUT;
    exit (0);
}

if ($tot_frms == 0) {
    open OUTPUT,">$log";
    print "Baum welch ran successfully for only 0 frames! Aborting..\n";
    close OUTPUT;
    exit (0);
}

$lkhd_per_frame = $tot_lkhd/$tot_frms;

$previter = $iter - 1;
$prev_norm = "${CFG_CI_LOG_DIR}/${CFG_EXPTNAME}.${previter}.norm.log";
if (! -s $prev_norm) {
    # Either iter == 1 or we are starting from an intermediate iter value
    system ("$CFG_CI_PERL_DIR/norm.pl $iter");
    system("echo \"Current Overall Likelihood Per Frame = $lkhd_per_frame\" >> $log");
    &Launch_BW();
    exit (0);
}

# Find the likelihood from the previous iteration
open LOG,$prev_norm; $prevlkhd = -99999999;
while (<LOG>) {
   if (/.*(Current Overall Likelihood Per Frame).*/){
      ($jnk,$jnk,$jnk,$jnk,$jnk,$jnk,$prevlkhd) = split(/ /);
   }
}
close LOG;

if ($prevlkhd == -99999999) {
    # Some error with previous norm.log. Continue Baum Welch
    system ("$CFG_CI_PERL_DIR/norm.pl $iter");
    system("echo \"Current Overall Likelihood Per Frame = $lkhd_per_frame\" >> $log");
    &Launch_BW();
    exit (0);
}

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
print "Current Overall Likelihood Per Frame = $lkhd_per_frame\n";

system ("$CFG_CI_PERL_DIR/norm.pl $iter");
system("echo \"Current Overall Likelihood Per Frame = $lkhd_per_frame\" >> $log");
system("echo \"Convergence ratio = $convg_ratio\" >> $log");

if ($convg_ratio < 0) {
    system("echo \"*WARNING*: NEGATIVE CONVERGENCE RATIO! CHECK YOUR DATA AND TRASNCRIPTS\" >> $log");
    print "*WARNING*: NEGATIVE CONVERGENCE RATIO AT ITER ${iter}! CHECK BW AND NORM LOGFILES\n";
}

if ($convg_ratio < $CFG_CONVERGENCE_RATIO && $iter < $CFG_MIN_ITERATIONS) {
    system("echo \"Minimum desired iterations $CFG_MIN_ITERATIONS not yet performed. Continuing BW\" >> $log");
    print "Minimum desired iterations $CFG_MIN_ITERATIONS not yet performed. Continuing BW after convergence\n";
    &Launch_BW();
    exit (0);
}

if ($convg_ratio > $CFG_CONVERGENCE_RATIO && $iter >= $CFG_MAX_ITERATIONS) {
    system("echo \"Maximum desired iterations $CFG_MAX_ITERATIONS performed. Terminating CI training\" >> $log");
    system("echo \"******************************TRAINING COMPLETE*************************\" >> $log");
    system("date >> $log");
    print "Maximum desired iterations $CFG_MAX_ITERATIONS performed. Terminating CI training\n";
    exit (0);
}

if ($convg_ratio > $CFG_CONVERGENCE_RATIO) {
    &Launch_BW();
    exit (0);
}
else {
    system("echo \"Likelihoods have converged! Baum Welch training completed\!\" >> $log");
    system("echo \"******************************TRAINING COMPLETE*************************\" >> $log");
    system("date >> $log");
    exit (0);
}


sub Launch_BW () {
    $newiter = $iter + 1;
    system ("$CFG_CI_PERL_DIR/slave_convg.pl $newiter");
}

