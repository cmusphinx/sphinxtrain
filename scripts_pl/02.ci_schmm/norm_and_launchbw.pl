#!/usr/bin/perl -w
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
## Author: Ricky Houghton (converted from scripts by Rita Singh)
##


require "../sphinx_train.cfg";


#*******************************************************************
#*******************************************************************

die "USAGE: $0 <iter>" if ($#ARGV != 0);

$iter = $ARGV[0];

mkdir ($CFG_CI_LOG_DIR,0777) unless -d $CFG_CI_LOG_DIR;

$log      = "$CFG_CI_LOG_DIR/${CFG_EXPTNAME}.$iter.norm.log";

#$num_done = `grep "MLLR regmat" $CFG_CI_LOG_DIR/${CFG_EXPTNAME}.${iter}-*.bw.log | wc -l | awk '{print $1}'`;

# Attempt to replace above:
$num_done = 0;
for $input_log (<${CFG_CI_LOG_DIR}/${CFG_EXPTNAME}.${iter}-*.bw.log>) {
 open LOG,$input_log;
 while (<LOG>) {
     $num_done++ if /.*(MLLR regmat).*/;
 }
 close LOG;
}

print "$num_done parts of $npart of Baum Welch were successfully completed\n";

if ($num_done != $npart) {
  open OUTPUT,">$log";
  print OUTPUT "Some of the baum-welch jobs seem to have bombed out. Expected $npart, got $num_done.\nAborting\!\!\!\\n";
  close OUTPUT;
  exit 0;
}

system ("$CFG_CI_PERL_DIR/norm.pl $iter");

# RAH, this needs to be cleaned up
$like = `grep "overall>" $logdir/${$CFG_EXPTNAME}.${iter}-*.bw.log | awk '{X += $3;Y += $6} END {print Y/X}'`;
system ("echo \"Current Overall Likelihood Per Frame = $like\" >> $log");

if ($iter == 1) { # Always perform a second iteration
    &Launch_BW();
    exit (0);
}

$previter = $iter - 1;
$prevlike = `grep "overall>" $logdir/${exptname}.${previter}-*.bw.log | awk '{ X += $3; Y += $6} END {print Y/X}'`;


# we seem to be starting from an intermediate iter value
if  ($prevlike eq "") {
  &Launch_BW();
  exit (0);
}


#hack to handle sign
$absprev = `echo "$prevlike"|awk '$1 < 0 {print -$1} $1 > 0 {print $1}'`;
system ("echo \"$prevlike $like $absprev\"|awk '{printf(\"Convergence Ratio = %f\n\",($2-$1)/$3)}' >> $log");
$testval = `echo "$prevlike $like $absprev"|awk -v th=$CFG_CONVERGENCE_RATIO '($2-$1)/$3 > th {print 1} ($2-$1)/$3 < th {print 0}'`;


if ($testval == 1) {
  &Launch_BW();
  exit (0);
}

# if testval != 1, the likelihoods have converged. No further jobs needed
system ("echo \"The likelihoods have converged. Baum Welch training completed\!\" >> $log");
system ("echo \"*********************************TRAINING COMPLETE***************************\" >> $log");
`date >> $log`;

exit (0);


sub Launch_BW () {
    $newiter = $iter + 1;
    system ("$CFG_CI_PERL_DIR/slave_convg.pl $newiter");
}


