#!/usr/opt/PERL5004/bin/perl -w

#$_ = `bsub -q priority ls`;

$job_id = $1 if (`bsub -q priority ls` =~ m/^Job \<(\d+)\> is submitted to queue .*$/);

print "-> ($job_id)\n";

