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

#***************************************************************************
# This script generates an mdef file for all the triphones occuring in the
# training set. This is done in several steps
# First, a list of all triphones possible in the vocabulary is generated
# (from the dictionary)
# An mdef file is then created to include all these triphones.
# The transcriptions of the training set are then used to count the number 
# of occurances of all the triphones in this mdef file.
# The triphones that occur more than a threshold number of times are used
# to generate the final mdef file. The threshold is automatically chosen
# so as to give the required number of triphones.
# (Thresholding is done so as to reduce the number of triphones, in order
# that the resulting models will be small enough to fit in the computer's
# memory)
#***************************************************************************

$logdir = "$CFG_LOG_DIR/03.makeuntiedmdef";
mkdir ($logdir,0777) unless -d $logdir;

$tmpdir	= "$CFG_BASE_DIR/tmp";
mkdir ($tmpdir,0777) unless -d $tmpdir;

&ST_Log ("MODULE: 03 Make Untied mdef\n");

#--------------------------------------------------------------------------
# Get the complete list of triphones possible in the dictionary
#--------------------------------------------------------------------------
$triphonelist 	= "$tmpdir/$$.triphones.list";
$tempdictionary = "$tmpdir/$$.dictionary";
$tempphonelist 	= "$tmpdir/$$.phonelist";

system ("sed \'s+\$+ 0 0 0 0+g\' ${CFG_RAWPHONEFILE} > $tempphonelist");
system ("echo \"SIL       SIL\" > $tempdictionary"); # Make sure SIL exists
system ("grep -v \"++\" $CFG_DICTIONARY >> $tempdictionary"); # Remove fillers
system ("sort +0 $tempdictionary > $tmpdir/$$.sortdictionary");
system ("mv -f $tmpdir/$$.sortdictionary $tempdictionary");

#set QUICK_COUNT = /net/alf19/usr3/eht/sphinx_ii/src/count_3phone/QUICK_COUNT 
#set QUICK_COUNT = ~bhiksha/work/sphinx-3/sphinx_ii/src/count_3phone/QUICK_COUNT
$QUICK_COUNT = "$CFG_BIN_DIR/QUICK_COUNT";

my $logfile = "$logdir/$CFG_EXPTNAME.quick_count.log";

&ST_HTML_Print ("\tQUICK_COUNT <A HREF=\"$logfile\">Log File</A> ");

if (open PIPE,"$QUICK_COUNT -q -p $tempphonelist -b $tempdictionary -o $triphonelist  2>&1 |")  {
    open LOG,">$logfile"  or die "Unable to open $logfile";
    while (<PIPE>) {
	print LOG $_;
    }
    close PIPE;
    close LOG;
    &ST_HTML_Print ("<font color=\"$CFG_OKAY_COLOR\"> Finished </font>\n");
} else {
    &ST_HTML_Print ("<font color=\"$CFG_ERROR_COLOR\"> Failed </font>\n");
    &ST_LogError ("Unable to run $QUICK_COUNT\n");
    die "Unable to run $QUICK_COUNT";
}

    
unlink "$tempdictionary";
unlink "$tempphonelist";


#----------------------------------------------------------------------------
# We need to append the triphone list to the properly formatted base phone
# list to be able to create the mdef file
#----------------------------------------------------------------------------
#system ("sed -e \'s+(+ +g\' -e \'s+,+ +g\' -e \'s+)+ +g\' $triphonelist | awk \'{printf(\"%s %s %s %s\n\",$1,$2,$3,$4)}\' | sed \'s+1+i+g\' > $tmpdir/$$.awkedtriphonelist");

system ("sed -e \'s+(+ +g\' -e \'s+,+ +g\' -e \'s+)+ +g\' $triphonelist | awk \'{printf(\"%s %s %s %s\\n\",\$1,\$2,\$3,\$4)}\' | sed \'s+1+i+g\' > $tmpdir/$$.awkedtriphonelist");

system ("sed -e \'s+\$+ - - -+g\' ${CFG_RAWPHONEFILE} > $tmpdir/$$.newbasephonelist");
system ("cat $tmpdir/$$.newbasephonelist $tmpdir/$$.awkedtriphonelist > $triphonelist");

unlink "$tmpdir/$$.newbasephonelist";
unlink "$tmpdir/$$.awkedtriphonelist";



#----------------------------------------------------------------------------
# Convert the complete list of triphones into an mdef file
#----------------------------------------------------------------------------
$alltriphones_mdef = "$CFG_BASE_DIR/model_architecture/${CFG_EXPTNAME}.alltriphones.mdef";

#set MAKE_MDEF = /net/alf24/usr2/rsingh/09..sphinx3code/trainer/bin.alpha/mk_model_def
$MAKE_MDEF = "$CFG_BIN_DIR/mk_model_def";

$logfile = "$logdir/${CFG_EXPTNAME}.make_alltriphonelist.log";

&ST_HTML_Print ("\tmk_model_def <A HREF=\"$logfile\">Log File</A>");

if (open PIPE,"$MAKE_MDEF -phonelstfn $triphonelist -moddeffn $alltriphones_mdef -n_state_pm $CFG_STATESPERHMM  2>&1|") {
    open LOG,">$logfile" or die "Unable to open $logfile";
    while (<PIPE>) {
	print LOG $_;
    }
    close PIPE;
    close LOG;
    &ST_HTML_Print ("<font color=\"$CFG_OKAY_COLOR\"> Finished </font>\n");
} else {
    &ST_LogError ("Unable to run $mk_model_def\n");
    &ST_HTML_Print ("<font color=\"$CFG_ERROR_COLOR\"> Failed </font>\n");
    die "Unable to run $mk_model_def";
}

unlink "$triphonelist";

#----------------------------------------------------------------------------
# Find the number of times each of the triphones listed in the 
# alltriphones_mdef occured in the corpus
#---------------------------------------------------------------------------

$listofalignedfiles      = $CFG_LISTOFFILES;
$alignedtranscriptfile   = $CFG_TRANSCRIPTFILE;

$allphones_count_file    = "$tmpdir/$$.allphones.counted";
$logfile 		 = "$logdir/${CFG_EXPTNAME}.triphonecount.log";

#set COUNTPROGRAM = /net/alf24/usr2/rsingh/09..sphinx3code/trainer/bin.alpha/param_cnt
$COUNTPROGRAM = "$CFG_BIN_DIR/param_cnt";

# RAH, don't know how to capture STDERR when we are redirecting STDOUT, so pipe it to a seperate file and filter later
&ST_HTML_Print ("\tparam_cnt <A HREF=\"$logfile\">Log File</A>");

if (open PIPE,"$COUNTPROGRAM -moddeffn $alltriphones_mdef -ts2cbfn ${CFG_HMM_TYPE} -ctlfn $listofalignedfiles -lsnfn $alignedtranscriptfile -dictfn ${CFG_DICTIONARY} -fdictfn ${CFG_FILLERDICT} -segdir dummy -paramtype  phone > $allphones_count_file 2>$logfile|") {
    # Add filtering code here
    close PIPE;
    &ST_HTML_Print ("<font color=\"$CFG_OKAY_COLOR\"> Finished </font>\n");
} else {
    &ST_LogError ("Unable to run $param_cnt\n");
    &ST_HTML_Print ("<font color=\"$CFG_ERROR_COLOR\"> Failed </font>\n");
    die "Unable to run $param_cnt";
}




#----------------------------------------------------------------------------
# From the list of triphones and counts obtained by the previous program
# pull out all the triphones above a particular threshold.
# Vary the threshold until we get the desired number of triphones in the
# untied triphone list. The number we use is 10000
# This will require about 200 MB per buffer
#----------------------------------------------------------------------------

$maxdesired = 10000;
$threshold = 1;
$numtriph = `awk -v thr=$threshold \'\$5 >= thr && \$2 != \"-\" {print \$1}\' $allphones_count_file | wc -l | awk \'{print \$1}\'`;

while ($numtriph > $maxdesired) {
    print "\t\t$numtriph\n";
    $threshold++;
    $numtriph = `awk -v thr=$threshold \'\$5 >= thr && \$2 != \"-\" {print \$1}\' $allphones_count_file | wc -l | awk \'{print \$1}\'`;
}

system "awk -v thr=$threshold \'\$5 >= thr && \$2 != \"-\" {print \$1,\$2,\$3,\$4;}\' $allphones_count_file > $tmpdir/$$.triphonelist";


system ("sed -e \'s+\$+ - - -+g\' ${CFG_RAWPHONEFILE} > $tmpdir/$$.newbasephonelist");
system ("cat $tmpdir/$$.newbasephonelist $tmpdir/$$.triphonelist > $triphonelist");
#unlink "$tmpdir/$$.newbasephonelist";
#unlink "$allphones_count_file";

#----------------------------------------------------------------------------
# We finally have a triphone list of the desired size from which we can 
# create the untied-state mdef file for cd training
#----------------------------------------------------------------------------
$untiedmdef = "${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.untied.mdef";

$logfile = "$logdir/${CFG_EXPTNAME}.make_untiedmdef.log";
open LOG,">$logfile" or die "Unable to open $logvile";

print LOG "Threshold = $threshold\n";


&ST_HTML_Print ("\tmk_model_def <A HREF=\"$logfile\">Log File</A>");

if (open PIPE,"$MAKE_MDEF -phonelstfn $triphonelist -moddeffn $untiedmdef -n_state_pm  ${CFG_STATESPERHMM} 2>&1 |") {
    while (<PIPE>) {
	print LOG $_;
    }
    close PIPE;
    close LOG;
    &ST_HTML_Print ("<font color=\"$CFG_OKAY_COLOR\"> Finished </font>\n");
} else {
    &ST_LogError ("Unable to run $MAKE_MDEF\n");
    &ST_HTML_Print ("<font color=\"$CFG_ERROR_COLOR\"> Failed </font>\n");
    unlink "$triphonelist";
    die "Unable to run $MAKE_MDEF";
}
    
unlink "$triphonelist";
exit 0
