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
## mc_check:  checks each machine named in the config file
##            checks directory is accessible, wriateable and the binaries
##            can be run

$mc_info_dir="mc";
$mc_job_info_dir="$mc_info_dir/job_info";
$mc_config="etc/mc_config";

die "config file $mc_config no found" unless -f $mc_config;

mkdir ($mc_info_dir,0777) unless -d $mc_info_dir;
mkdir ($mc_job_info_dir,0777) unless -d $mc_job_info_dir;

# check each machine in config list

open(CFGFD,"<".$mc_config);
while ( <CFGFD> ) {
    chomp;
    my @s=split /\s+/;

    $job_num=$$;
    $mc_job_id="$s[0].$job_num";
    $mc_job_file="$mc_job_info_dir/$mc_job_id";

    system("ssh -n $s[0] \"(cd $s[1] && wc bin/bw >$mc_job_file.log)\" 2>/dev/null");

    if ( -f "$mc_job_file.log")
    {
	$file_access = "OK";
	system("/bin/rm $mc_job_file.log");
    }
    else
    {
	$file_access = "not_OK";
    }
    $la = get_remote_loadav($s[0]);

    print "$s[0] $la access $file_access\n";
    
}

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
	open rrr,"ssh -n $remote_machine uptime 2>/dev/null |";
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
