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

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

#*************************************************************************
# This script runs the build_tree script for each state of each basephone
#*************************************************************************

my ($phone,$state);
my $return_value = 0;
my $scriptdir = "$ST::CFG_SCRIPT_DIR/40.buildtrees";
my $logdir = "${ST::CFG_LOG_DIR}/40.buildtrees";
Log ("MODULE: 40 Build Trees\n");
Log ("    Cleaning up old log files...\n");
rmtree ("$logdir");
mkdir ($logdir,0777);

$| = 1; # Turn on autoflushing
if ($ST::CFG_MAKE_QUESTS eq 'no') {
    die "Question set $ST::CFG_QUESTION_SET not found, but CFG_MAKE_QUESTS is no"
	unless -e $ST::CFG_QUESTION_SET;
}
else {
    if (RunScript('make_questions.pl')) {
	$return_value = 1;
	exit ($return_value);
    }
}

Log ("    Tree building\n");

my $mdef_file       = "${ST::CFG_BASE_DIR}/model_architecture/${ST::CFG_EXPTNAME}.untied.mdef";
my $mixture_wt_file = "${ST::CFG_BASE_DIR}/model_parameters/${ST::CFG_EXPTNAME}.cd_${ST::CFG_DIRLABEL}_untied/mixture_weights";
my $tree_base_dir   = "${ST::CFG_BASE_DIR}/trees";
my $unprunedtreedir = "$tree_base_dir/${ST::CFG_EXPTNAME}.unpruned";
mkdir ($tree_base_dir,0777);
mkdir ($unprunedtreedir,0777);

# For every phone submit each possible state
my @jobs;
if ($ST::CFG_CROSS_PHONE_TREES eq 'yes') {
    Log ("\tProcessing all phones with each state\n");
    push @jobs, ['ALLPHONES' => LaunchScript("tree.all", ['buildtree.pl', 'ALLPHONES'])];
} else {
    Log ("\tProcessing each phone with each state\n");
    open INPUT,"${ST::CFG_RAWPHONEFILE}";
    foreach $phone (<INPUT>) {
	# Use @jobs as a queue (better than nothing for Queue::POSIX)
	if (@jobs > $ST::CFG_NPART) {
	    my ($p, $j) = @{shift @jobs};
	    WaitForScript($j);
	    print "$p ";
	}
	chomp $phone;
	if (($phone =~ m/^(\+).*(\+)$/) || ($phone =~ m/^SIL$/)) {
	    Log ("        Skipping $phone\n");
	    next;
	}

	my $job = LaunchScript("tree.$phone", ['buildtree.pl', $phone]);
	push @jobs, [$phone => $job];
    }
    close INPUT;
}

# Wait for all the remaining phones to finish
# It doesn't really matter what order we do this in
foreach (@jobs) {
    my ($phone,$job) = @$_;
    WaitForScript($job);
    print "$phone ";
}
print "\n";

# Build all triphone model
my $logfile = "$logdir/$ST::CFG_EXPTNAME.build.alltriphones.mdef.log";
my $modarchdir          = "$ST::CFG_BASE_DIR/model_architecture";
my $ALLTRIPHONESMDDEF = "$modarchdir/$ST::CFG_EXPTNAME.alltriphones.mdef";
my $phonefile           = "$modarchdir/$ST::CFG_EXPTNAME.phonelist";

HTML_Print ("\t\tmk_mdef_gen " . FormatURL("$logfile", "Log File") . " ");

my $status = RunTool('mk_mdef_gen', $logfile, 0,
		  -phnlstfn => $phonefile,
		  -oalltphnmdef => $ALLTRIPHONESMDDEF,
		  -dictfn => $ST::CFG_DICTIONARY,
		  -fdictfn => $ST::CFG_FILLERDICT,
		  -n_state_pm => $ST::CFG_STATESPERHMM);
exit $status if $status;

$status = RunScript('prunetree.pl', $ST::CFG_N_TIED_STATES)
    or RunScript('tiestate.pl', $ST::CFG_N_TIED_STATES);
exit $status;
