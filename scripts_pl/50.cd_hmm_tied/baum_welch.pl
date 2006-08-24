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
## Author: David Huggins-Daines
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

#************************************************************************
# this script performs baum-welch training using s3 code for a
# continuous mdef file.
# it needs as inputs an initial set of semicont models in s3 format
# a mdef file and cepstra with transcription files.
#************************************************************************

$| = 1; # Turn on autoflushing

die "USAGE: $0 <ngau> <iter> <part> <npart>" if (@ARGV != 4);
my ($n_gau, $iter, $part, $npart) = @ARGV;

my $modelinitialname="${ST::CFG_EXPTNAME}.cd_${ST::CFG_DIRLABEL}_initial";
my $modelname="${ST::CFG_EXPTNAME}.cd_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}";
my $mdefname="${ST::CFG_EXPTNAME}.$ST::CFG_N_TIED_STATES.mdef";
my $processname = "50.cd_hmm_tied";

my $output_buffer_dir = "$ST::CFG_BASE_DIR/bwaccumdir/${ST::CFG_EXPTNAME}_buff_${part}";
mkdir ($output_buffer_dir,0777);

my ($hmm_dir, $var2pass);
if ($iter == 1) {
    $hmm_dir  = "$ST::CFG_BASE_DIR/model_parameters/$modelinitialname";
    $var2pass	 = "no";
} else {
    $hmm_dir      = "$ST::CFG_BASE_DIR/model_parameters/$modelname";
    $var2pass	  = "yes";
}


my $moddeffn    = "$ST::CFG_BASE_DIR/model_architecture/$mdefname";
my $statepdeffn = $ST::CFG_HMM_TYPE; # indicates the type of HMMs
my $mixwfn  = "$hmm_dir/mixture_weights";
my $mwfloor = 1e-8;
my $tmatfn  = "$hmm_dir/transition_matrices";
my $meanfn  = "$hmm_dir/means";
my $varfn   = "$hmm_dir/variances";
my $minvar  = 1e-4;

# if there is an LDA transformation, use it
my @lda_args;
if (defined($ST::CFG_LDA_TRANSFORM) and -r $ST::CFG_LDA_TRANSFORM) {
    push(@lda_args,
	 -ldafn => $ST::CFG_LDA_TRANSFORM,
	 -ldadim => $ST::CFG_LDA_DIMENSION);
}

# aligned transcripts and the list of aligned files is obtained as a result
# of (03.) forced alignment
my ($listoffiles, $transcriptfile);
if ( $ST::CFG_FORCEDALIGN eq "no" ) {
    $listoffiles = $ST::CFG_LISTOFFILES;
    $transcriptfile = $ST::CFG_TRANSCRIPTFILE;
} else {
    $listoffiles   = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedfiles";
    $transcriptfile  = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedtranscripts";
}

my $topn     = 4;
my $logdir   = "$ST::CFG_LOG_DIR/$processname";
my $logfile  = "$logdir/${ST::CFG_EXPTNAME}.${n_gau}.$iter-$part.bw.log";
mkdir ($logdir,0777);

my $ctl_counter = 0;
open INPUT,"${ST::CFG_LISTOFFILES}";
while (<INPUT>) {
    $ctl_counter++;
}
close INPUT;
$ctl_counter = int ($ctl_counter / $npart) if $npart;
$ctl_counter = 1 unless ($ctl_counter);

copy "$ST::CFG_GIF_DIR/green-ball.gif", "$ST::CFG_BASE_DIR/.50.bw.$n_gau.$iter.$part.state.gif";
HTML_Print ("\t" . ImgSrc("$ST::CFG_BASE_DIR/.50.bw.$n_gau.$iter.$part.state.gif") . " ");        
Log ("    Baum welch starting for $n_gau Gaussian(s), iteration: $iter ($part of $npart) ");
HTML_Print (FormatURL("$logfile", "Log File") . "\n");

my $return_value = RunTool
    ('bw', $logfile, $ctl_counter,
     -moddeffn => $moddeffn,
     -ts2cbfn => $statepdeffn,
     -mixwfn => $mixwfn,
     -mwfloor => $mwfloor,
     -tmatfn => $tmatfn,
     -meanfn => $meanfn,
     -varfn => $varfn,
     -ltsoov => $ST::CFG_LTSOOV,
     -dictfn => $ST::CFG_DICTIONARY,
     -fdictfn => $ST::CFG_FILLERDICT,
     -ctlfn => $ST::CFG_LISTOFFILES,
     -part => $part,
     -npart => $npart,
     -cepdir => $ST::CFG_FEATFILES_DIR,
     -cepext => $ST::CFG_FEATFILE_EXTENSION,
     -lsnfn => $ST::CFG_TRANSCRIPTFILE,
     -accumdir => $output_buffer_dir,
     -varfloor => $minvar,
     -topn => $topn,
     -abeam => 1e-90,
     -bbeam => 1e-40,
     -agc => $ST::CFG_AGC,
     -cmn => $ST::CFG_CMN,
     -varnorm => $ST::CFG_VARNORM,
     -meanreest => "yes",
     -varreest => "yes",
     '-2passvar' => $var2pass,
     -tmatreest => "yes",
     -feat => $ST::CFG_FEATURE,
     -ceplen => $ST::CFG_VECTOR_LENGTH,
     @lda_args,
     -timing => "no");


if ($return_value) {
  copy "$ST::CFG_GIF_DIR/red-ball.gif", "$ST::CFG_BASE_DIR/.50.bw.$n_gau.$iter.$part.state.gif";
  LogError ("\tFailed to start bw \n");
}
exit ($return_value);
