#!/usr/opt/PERL5004/bin/perl -w

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


