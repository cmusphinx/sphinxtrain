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
## Author: Ricky Houghton
##

use File::Path;

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

#*************************************************************************
# This script runs the build_tree script for each state of each basephone
#*************************************************************************

my ($phone,$state);
my $scriptdir = "$CFG_SCRIPT_DIR/05.buildtrees";
my $logdir = "${CFG_LOG_DIR}/05.buildtrees";
&ST_Log ("Module: 05 Build Trees\n");
&ST_Log ("    Cleaning up old log files...\n");
rmtree ("$logdir");
mkdir ($logdir,0777) unless -d $logdir;

$| = 1; # Turn on autoflushing
system("$scriptdir/make_questions.pl");
&ST_Log ("    Tree building\n");

my $mdef_file       = "${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.untied.mdef";
my $mixture_wt_file = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.cd_semi_untied/mixture_weights";
my $tree_base_dir   = "${CFG_BASE_DIR}/trees";
my $unprunedtreedir = "$tree_base_dir/${CFG_EXPTNAME}.unpruned";
mkdir ($tree_base_dir,0777) unless -d $tree_base_dir;
mkdir ($unprunedtreedir,0777) unless -d $unprunedtreedir;

# For every phone submit each possible state
&ST_Log ("\tProcessing each phone with each state\n");
open INPUT,"${CFG_RAWPHONEFILE}";
foreach $phone (<INPUT>) {
    chomp $phone;
    if (($phone =~ m/^(\+).*(\+)$/) || ($phone =~ m/^SIL$/)) {
	&ST_Log ("        Skipping $phone\n");
	next;
    }

    if ($MC)   # do multi-machine 
    {
        $job_command = "$scriptdir/buildtree.pl -cfg $cfg_file $phone";
	$job_name = "no_job";
	while ($job_name eq "no_job")
        {
	    open rrr,"scripts_pl/mc/mc_run.pl $job_command |";
	    while ($line = <rrr>)
	    {
		chomp($line);
#	    print "mc_run: ".$line."\n";
		@fff=split(/\s+/,$line);
		if ($fff[0] eq "MC")
		{
		    $job_name = $fff[1];
		    last;
		}
#            print "waiting for mc_run to say something\n";
		sleep 3;
	    }
	    close rrr;
	    if ($job_name eq "no_job")
	    {
		print "waiting for machine for job $phone\n";
		sleep 30;
	    }
	}
	print "running job $phone on $job_name \n";
	$phones{$phone} = $job_name;
    }
    else
    {
	system("$scriptdir/buildtree.pl $phone");
    }
    close INPUT;
}

if ($MC)       # wait for all the sub tasks to finish
{
    $jobs_still_todo = 1;
    while ($jobs_still_todo)
    {
	$jobs_still_todo = 0;
	for my $k (keys %phones)
        {
	    if ( -f $phones{$k} )
	    {
		print "waiting for $k on ".$phones{$k}."\n";
		$jobs_still_todo = 1;
	    }
	}
	sleep 30;
    }
}

exit 0;
