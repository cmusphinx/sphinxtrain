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


my $index;

# RAH Force passage of config file, or look for it one directory up.
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    $index = 2;
    if (! -s $cfg_file) {
	print "-cfg specified, but unable to find file $ARGV[1]\n";
	exit -3;
    }
require $cfg_file;
} else {
    $cfg_file = "../sphinx_train.cfg";
    require $cfg_file;
    &ST_LogWarning("-cfg not specified, using the default ../sphinx_train.cfg");
    $index = 0 ;
}

#****************************************************************************
# The training procedure is done in several parts and the results are 
# consolidated in the end. This script runs the Baum-Welch accumulation
# on any one of the parts at one time. After all parts are done the 
# normalization program has to be run to get the partial results together. 
# combine them to estimate the new model parameters
#****************************************************************************
die "USAGE: $0 <iter> <part> <n_part>" if ($#ARGV != $index + 2);

$iter   	= $ARGV[$index];
$part   	= $ARGV[$index+1];
$npart 		= $ARGV[$index+2];


#$listofalignedfiles is now the list of files that were successfully aligned
#$alignedtranscriptfile is the output of the aligner

if (lc($CFG_FORCEDALIGN) eq "yes" ) {
  $filelist       = "${CFG_BASE_DIR}/generated/${CFG_EXPTNAME}.alignedfiles";
  $transcripts    = "${CFG_BASE_DIR}/generated/${CFG_EXPTNAME}.alignedtranscripts";
} else{
  $filelist       = "${CFG_LISTOFFILES}";
  $transcripts    = "${CFG_TRANSCRIPTFILE}";
}

$untiedmdef = "${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.untied.mdef";
$hmmdir	    = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.cd_semi_untied";

$meanfn  = "$hmmdir/means";
$varfn   = "$hmmdir/variances";
$mixwfn  = "$hmmdir/mixture_weights";
$tmatfn  = "$hmmdir/transition_matrices";

$var2pass = ($iter == 1) ? 'no' : 'yes';

$bwaccumdir 	= "${CFG_BWACCUM_DIR}/${CFG_EXPTNAME}_buff_${part}";
mkdir ($bwaccumdir,0777) unless -d $bwaccumdir;

$logdir         =  "${CFG_LOG_DIR}/04.cd_schmm_untied";
mkdir ($logdir,0777) unless -d $logdir;
$logfile 	=  "$logdir/${CFG_EXPTNAME}.$iter-$part.bw.log";

#set mach = `~/51..tools/machine_type.csh`
#set BW  = /net/alf19/usr2/eht/s3/bin.$mach/bw
#set BW  = ~/09..sphinx3code/trainer/bin.$mach/bw
$BW  = "$CFG_BIN_DIR/bw";
$mwfloor = 1e-8;
$minvar  = 1e-4;

$ctl_counter = 0;
open INPUT,"${CFG_LISTOFFILES}";
while (<INPUT>) {
    $ctl_counter++;
}
close INPUT;
$ctl_counter = int ($ctl_counter / $CFG_NPART) if $CFG_NPART;
$ctl_counter = 1 unless ($ctl_counter);

&ST_Log ("\tBaum welch starting for iteration: $iter ($part of $npart) ");
&ST_HTML_Print ("<A HREF=\"$logfile\">Log File</A>\n");

if (open PIPE, "$BW -moddeffn $untiedmdef -ts2cbfn ${CFG_HMM_TYPE} -mixwfn	$mixwfn -mwfloor $mwfloor -tmatfn $tmatfn -meanfn $meanfn -varfn $varfn -dictfn ${CFG_DICTIONARY} -fdictfn ${CFG_FILLERDICT} -ctlfn ${CFG_LISTOFFILES} -part $part -npart $npart -cepdir ${CFG_FEATFILES_DIR} -cepext ${CFG_FEATFILE_EXTENSION} -lsnfn ${CFG_TRANSCRIPTFILE} -accumdir $bwaccumdir -varfloor $minvar -topn 4 -abeam 1e-90 -bbeam 1e-40 -agc ${CFG_AGC} -cmn ${CFG_CMN} -meanreest yes -varreest yes -2passvar $var2pass -tmatreest yes -feat ${CFG_FEATURE} -ceplen ${CFG_VECTOR_LENGTH} 2>&1 |") {

    open LOG,">$logfile";
    
    # RAH 7.20.2000 
    # Note this portion keeps track of the progress, however it doesn't
    # suport running only a fraction of the ctl file. (Note, the scripts
    # don't support this either, however it is something we'd probably
    # like to do in the future.)

    $processed_counter = 0;
    &ST_Log ("\t\tProcessing $ctl_counter files\n\t\t");
    $| = 1;				# Turn on autoflushing
    while ($line = <PIPE>) {
	print LOG $line;
	if ($line =~ m/.*ERROR.*/) {
	    &ST_LogError ($_);	
	    die "04 - BW-----$line" 
	    }
	$processed_counter++  if ($line =~ m/.*(utt\>).*/);
	$percentage = int (($processed_counter / $ctl_counter) * 100);
	if (!($percentage % 10)) {
	    &ST_Log ("${percentage}% ") unless $printed;
	    $printed = 1;
	} else {
	    $printed = 0;
	}
    }
    $| = 0;				# Turn it off
    close PIPE;
    close LOG;
    &ST_Log ("\tFinished\n");
    exit (0);
}
&ST_Log ("Failed to start $BW\n");
exit (-1);
