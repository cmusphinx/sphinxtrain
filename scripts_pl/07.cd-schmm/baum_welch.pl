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

use File::Copy;

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


#************************************************************************
# this script performs baum-welch training using s3 code for a 
# continuous mdef file.
# it needs as inputs an initial set of semicont models in s3 format
# a mdef file and cepstra with transcription files.
#************************************************************************
$| = 1; # Turn on autoflushing

die "USAGE: $0 <iter> <part> <npart>" if ($#ARGV != ($index + 2));

$iter   = $ARGV[$index];
$part   = $ARGV[$index+1];
$npart  = $ARGV[$index+2];

$modelinitialname="${CFG_EXPTNAME}.cd_semi_initial";
$modelname="${CFG_EXPTNAME}.cd_semi_${CFG_N_TIED_STATES}";
$mdefname="${CFG_EXPTNAME}.$CFG_N_TIED_STATES.mdef";
$processname = "07.cd-schmm";

$output_buffer_dir = "$CFG_BASE_DIR/bwaccumdir/${CFG_EXPTNAME}_buff_${part}";
mkdir ($output_buffer_dir,0777) unless -d $output_buffer_dir;

if ($iter == 1) {
    $hmm_dir  = "$CFG_BASE_DIR/model_parameters/$modelinitialname";
    $var2pass	 = "no";
} else {
    $hmm_dir      = "$CFG_BASE_DIR/model_parameters/$modelname";
    $var2pass	  = "yes";
}


$moddeffn    = "$CFG_BASE_DIR/model_architecture/$mdefname";
$statepdeffn = $CFG_HMM_TYPE; # indicates the type of HMMs
$mixwfn  = "$hmm_dir/mixture_weights";
$mwfloor = 1e-8;
$tmatfn  = "$hmm_dir/transition_matrices";
$meanfn  = "$hmm_dir/means";
$varfn   = "$hmm_dir/variances";
$minvar  = 1e-4;


# aligned transcripts and the list of aligned files is obtained as a result
# of (03.) forced alignment

if ( $CFG_FORCEDALIGN eq "no" ) {
    $listoffiles = $CFG_LISTOFFILES;
    $transcriptfile = $CFG_TRANSCRIPTFILE;
} else {
    $listoffiles   = "$CFG_BASE_DIR/generated/${CFG_EXPTNAME}.alignedfiles";
    $transcriptfile  = "$CFG_BASE_DIR/generated/${CFG_EXPTNAME}.alignedtranscripts";
}

$topn     = 4;
$logdir   = "$CFG_LOG_DIR/$processname";
$logfile  = "$logdir/${CFG_EXPTNAME}.$iter-$part.bw.log";
mkdir ($logdir,0777) unless -d $logdir;

$ctl_counter = 0;
open INPUT,"${CFG_LISTOFFILES}";
while (<INPUT>) {
    $ctl_counter++;
}
close INPUT;
$ctl_counter = int ($ctl_counter / $npart) if $npart;
$ctl_counter = 1 unless ($ctl_counter);

copy "$CFG_GIF_DIR/green-ball.gif", "$CFG_BASE_DIR/.02.bw.$iter.$part.state.gif";
&ST_HTML_Print ("\t<img src=$CFG_BASE_DIR/.02.bw.$iter.$part.state.gif> ");        
&ST_Log ("    Baum welch starting for iteration: $iter ($part of $npart) ");
&ST_HTML_Print ("<A HREF=\"$logfile\">Log File</A>\n");

open LOG,">$logfile";

$BW   = "$CFG_BIN_DIR/bw";
if (open PIPE, "$BW -moddeffn $moddeffn -ts2cbfn $statepdeffn -mixwfn	$mixwfn -mwfloor $mwfloor -tmatfn $tmatfn -meanfn $meanfn -varfn $varfn -dictfn $CFG_DICTIONARY -fdictfn $CFG_FILLERDICT -ctlfn $CFG_LISTOFFILES -part $part -npart $npart -cepdir $CFG_FEATFILES_DIR -cepext $CFG_FEATFILE_EXTENSION -lsnfn $CFG_TRANSCRIPTFILE -accumdir	$output_buffer_dir -varfloor $minvar -topn $topn -abeam 1e-90 -bbeam 1e-40 -agc $CFG_AGC -cmn $CFG_CMN -meanreest yes -varreest yes -2passvar $var2pass -tmatreest yes -feat $CFG_FEATURE -ceplen $CFG_VECTOR_LENGTH 2>&1 |") {

    $processed_counter = 0;
    &ST_Log ("\n        Using $ctl_counter files: ");
    $| = 1;				# Turn on autoflushing
    while (<PIPE>) {
	if (/(ERROR).*/) {
	    &ST_LogError ($_);
	}
	if (/(FATAL).*/) {
	    &ST_LogError ($_);
	    die "Received a fatal error";
	}
	print LOG "$_";
	# Keep track of progress being made.
	$processed_counter++  if (/.*(utt\>).*/);
	$percentage = int (($processed_counter / $ctl_counter) * 100);
	if (!($percentage % 10)) {
	    &ST_Log ("${percentage}% ") unless $printed;
	    $printed = 1;
	} else {
	    $printed = 0;
	}
    }
    close PIPE;
    $| = 0;
    $date = localtime;
    print LOG "$date\n";
    close LOG;
    &ST_Log ("Finished\n");
    exit (0);
}

copy "$CFG_GIF_DIR/red-ball.gif", "$CFG_BASE_DIR/.02.bw.$iter.$part.state.gif";
&ST_LogError ("\tFailed to start $BW \n");    
exit (-1);

