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
## Author: Alan W Black (awb@cs.cmu.edu)
##
## A simple set of scripts for machine clutering, assumes [r|s]sh access
## accross machines and that directory is accessible from remote machines
## Note this isn't design for small jobs, its not going to be worth using
## if the typical job time is less than a few minutes
##
## Get's information from etc/mc_config
## Stores jobs infor in .mc/job_info/
## Related scripts: mc_status, mc_run, mc_kill
##

$mc_info_dir="mc";
$mc_job_info_dir="$mc_info_dir/job_info";
$mc_config="etc/mc_config";

die "config file $mc_config no found\nMC no_job\n" unless -f $mc_config;

# run job on next unloaded machine

mkdir ($mc_info_dir,0777) unless -d $mc_info_dir;
mkdir ($mc_job_info_dir,0777) unless -d $mc_job_info_dir;

$mc_command=join ' ',@ARGV;

$best_machine="";
$least_load=0;
$max_jobs=0;
open(CFGFD,"<".$mc_config);
while ( <CFGFD> ) {
    chomp;
    my @s=split /\s+/;
#    print $s[0]."\n";

    # Don't run anything on a machine if its already got enough of my
    # jobs running running on it (should be s[2] of my jobs)
    $current_jobs=0;
    open rrr,"ls $mc_job_info_dir | grep -v log |";
    while ($line = <rrr>)
    {
	chomp($line);
	$line =~ s/\.[0-9]+$//;
	if ($line eq $s[0])
	{
	    $current_jobs = $current_jobs + 1;
	}
#	print "mok: ".$s[0]." ".$current_jobs." ".$s[2]."\n";
    }
    close rrr;

    if ($current_jobs < $s[2])
    {
	$la = get_remote_loadav($s[0]);
#	print "machine $s[0] la $la\n";
	if (($la < $s[2]) && ($s[2]-$la > $least_load))
	{
	    $best_machine = $s[0]; 
	    $best_dir     = $s[1];
	    $max_jobs     = $s[2];
	    $least_load   = $s[2]-$la;
	}
    }
}

#print "best_machine ".$best_machine." least_load ".$least_load."\n";
#print "gonna exec ".$best_machine." ".$mc_command."\n";

if ($best_machine eq "")
{
    print "MC no_job\n";
    exit 0;
}

# exit -1 unless $best_machine; # no free machine to run on

# run given job on remote machine
# should should probably be a local script on the machine rather than 
# a complex sh command
$job_num=$$;
$mc_job_id="$best_machine.$job_num";
$mc_job_file="$mc_job_info_dir/$mc_job_id";

# Due to NFS latency, we'll just mark that machine is used now,
system("/bin/touch $mc_job_file");
# Do the remote execution 
system("ssh -f -n $best_machine \"(cd $best_dir && /bin/touch $mc_job_file && $mc_command >$mc_job_file.log && /bin/rm $mc_job_file &)\" 2>/dev/null & sleep 3");
#}

print "MC ".$mc_job_file."\n";

exit 0;

sub get_remote_loadav
{
    my ($remote_machine) = @_;
    chomp($hostname=`hostname`);
    if ($remote_machine eq $hostname)
    {
	$uptime=`uptime`;
    }
    else
    {
	open rrr,"ssh -n $remote_machine uptime |";
	while ($line = <rrr>)
        {
	    chomp($line);
	    $uptime = $line;
#	    print "up: ".$line."\n";
        }
	close rrr;
    }
    @ups=split(/\s+/,$uptime);
    for ($ii=1; $ii<12; $ii=$ii+1)
    {
	if ($ups[$ii-1] eq "average:")
	{
	    $loadav = $ups[$ii];
	}
    }
    $loadav =~ s/,//;
#    print "la: ".$remote_machine." ".$ups[10]." >".$uptime."<\n";
    return $loadav;
}
