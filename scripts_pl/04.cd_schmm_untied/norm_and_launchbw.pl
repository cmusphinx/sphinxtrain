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
## Author: Ricky Houghton (converted from scripts by Rita Singh)
##


require "/sphx_train/testing/scripts_pl/sphinx_train.cfg";

#*******************************************************************
#*******************************************************************

die "USAGE: $0 <iter>" if ($#ARGV != 0);
$iter   		= $ARGV[0];

$logdir              = "${CFG_LOG_DIR}/04.cd_schmm_untied";
mkdir ($logdir,0777) unless $logdir;
$log 	     = "${CFG_LOG_DIR}/${CFG_EXPTNAME}.${iter}.norm.log";

$num_done = `grep "MLLR regmat" $logdir/${CFG_EXPTNAME}.${iter}-*.bw.log | wc -l | awk '{print $1}'`;

print "$num_done parts of $npart_untied of Baum Welch successfully completed" >! $log;

if ($num_done != $npart_untied) {
    open LOG,">>$log";
    print LOG "Some of the baum-welch jobs seem to have bombed out.";
    print LOG "Here is a list of jobs that died..";

    $p_id = 1;
    system  ("grep \"Counts saved\" $logdir/${CFG_EXPTNAME}.$iter-*.bw.log | awk -F\"_\" '{print $NF}' |sort -n > ! /tmp/xx.$p_id");
#    system ("~rsingh/51..tools/interval 1 $npart_untied >! /tmp/xy.$$
    system ("$CFG_BIN_DIR/interval 1 $npart_untied >! /tmp/xy.$p_id");
    system ("diff /tmp/xx.$p_id /tmp/xy.$p_id|grep \">\" >> $log");
    print LOG "Aborting\!\!\!";
    close LOG;
    system ("rm -f /tmp/xx.$$ /tmp/xy.$$");
    exit 0;
}

$script_dir = "$CFG_SCRIPT_DIR/04.cd_schmm_untied";
system ("$script_dir/norm.pl $iter");

$like = `grep "overall>" $logdir/${CFG_EXPTNAME}.${iter}-*.bw.log | awk '{X += $3;Y += $6} END {print Y/X}'`;
open LOG,">>$log";
print LOG "Current Overall Likelihood Per Frame = $like";

&launch_bw if ($iter == 1); # Always perform a second iteration

$previter = $iter - 1;

$prevlike = `grep "Current Overall Li" $logdir/${exptname}.${previter}.norm.log | awk '{print $NF}'`;

&launch_bw if ($prevlike eq ""); # we seem to be starting from an intermediate iter value

#hack to handle sign
$absprev = `echo "$prevlike"|awk '$1 < 0 {print -$1} $1 > 0 {print $1}'`;
print LOG "$prevlike $like $absprev"|awk '{printf("Convergence Ratio = %f\n",($2-$1)/$3)}';
$testval = `echo "$prevlike $like $absprev"|awk -v th=$convergence_ratio '($2-$1)/$3 > th {print 1} ($2-$1)/$3 < th {print 0}'`;

&launch_bw if ($testval == 1);


# if testval != 1, the likelihoods have converged. No further jobs needed
print LOG "The likelihoods have converged. Baum Welch training completed\!";
print LOG "*********************************TRAINING COMPLETE***************************";
print LOG 'date';
system ("$script_dir/makeuntiedmixw.pl");
close LOG;
exit 0;

sub launch_bw ()
{
    $newiter = $iter + 1;
    system ("$script_dir/slave_convg.pl $newiter");
    close LOG;
    exit 0;
}


