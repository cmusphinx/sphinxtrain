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
## 4. Redistributions of any form whatsoever must retain the following
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
## Author: Ricky Houghton (Alan W Black)
##
## Pulled this out of slace.treebuilder to allow this to run on
## multiple machines

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

die "USAGE: $0 <phone> " if ($#ARGV != $index);

$phone = $ARGV[$index];

my $scriptdir = "{$CFG_SCRIPT_DIR}/05.buildtrees";
my $logdir = "${CFG_LOG_DIR}/05.buildtrees";
mkdir ($logdir,0777) unless -d $logdir;

$| = 1; # Turn on autoflushing

my $mdef_file       = "${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.untied.mdef";
my $mixture_wt_file = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.cd_semi_untied/mixture_weights";
my $tree_base_dir   = "${CFG_BASE_DIR}/trees";
my $unprunedtreedir = "$tree_base_dir/${CFG_EXPTNAME}.unpruned";
mkdir ($tree_base_dir,0777) unless -d $tree_base_dir;
mkdir ($unprunedtreedir,0777) unless -d $unprunedtreedir;

$state = 0;
while ( $state < $CFG_STATESPERHMM) {
    &BuildTree ($phone,$state);
    $state++;
}

exit 0;


# SubRoutine created from build_tree.pl
sub BuildTree ()
{
    my $phn = shift;
    my $stt = shift;

    my $logfile = "$logdir/${CFG_EXPTNAME}.buildtree.${phn}.${stt}.log";

    &ST_Log ("\t\t${phn} ${stt} ");
    &ST_HTML_Print ("<A HREF=\"$logfile\">Log File</A>\n");

    #$mach = `~/51..tools/machine_type.csh`;
    #$BUILDTREE = "/net/alf19/usr2/eht/s3/bin.$mach/bldtree";
    #$BUILDTREE = "~rsingh/09..sphinx3code/trees_cont/bin.$mach/bldtree";
    
    $BUILDTREE = "${CFG_BIN_DIR}/bldtree";

    # RAH 7.21.2000 - These were other possible values for these
    # variables, I'm not sure the circumstance that would dictate
    # either set of values
    #	-stwt     1.0 0.1 0.01 
    #	-stwt     1.0 0.3 0.2 
    #	-ssplitthr 8e-4 \
    #	-csplitthr 1e-5 \

    open LOG,">$logfile";    
    if (open PIPE, "$BUILDTREE -treefn $unprunedtreedir/$phn-$stt.dtree -moddeffn $mdef_file -mixwfn $mixture_wt_file -ts2cbfn ${CFG_HMM_TYPE} -mwfloor 1e-30 -psetfn ${CFG_QUESTION_SET} -phone $phn -state $stt -stwt 1.0 0.3 0.1 0.01 0.001 -ssplitmin 1 -ssplitmax 5 -ssplitthr 0 -csplitmin 1 -csplitmax 500 -csplitthr 0 2>&1 |") {
	
	while (<PIPE>) {
	    print LOG "$_";
	}
	close PIPE;
	close LOG;
    } else {
	&ST_LogError ("Unable to start $BUILDTREE\n");
	print LOG "Unable to start $BUILDTREE\n";
	close LOG;
    }
}
