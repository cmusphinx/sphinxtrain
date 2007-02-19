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

$| = 1; # Turn on autoflushing

die "USAGE: $0 <iter> <speaker>" if @ARGV != 2;
my ($iter, $speaker) = @ARGV;

my $modelinitialname="${ST::CFG_EXPTNAME}.cd_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}";
my $modelname="${ST::CFG_EXPTNAME}.sat_${ST::CFG_DIRLABEL}";
my $mdefname="${ST::CFG_EXPTNAME}.$ST::CFG_N_TIED_STATES.mdef";
my $processname ="60.sa_train";

my $output_buffer_dir = "$ST::CFG_BWACCUM_DIR/${ST::CFG_EXPTNAME}_buff_${speaker}";
mkdir ($output_buffer_dir,0777);

my ($hmm_dir);
if ($iter == 1) {
    $hmm_dir  = "$ST::CFG_MODEL_DIR/$modelinitialname";
} else {
    $hmm_dir  = "$ST::CFG_MODEL_DIR/$modelname";
}

my $moddeffn    = "$ST::CFG_BASE_DIR/model_architecture/$mdefname";
my $statepdeffn = $ST::CFG_HMM_TYPE; # indicates the type of HMMs
my $mixwfn  = "$hmm_dir/mixture_weights";
my $mwfloor = 1e-8;
my $tmatfn  = "$hmm_dir/transition_matrices";
my $meanfn  = "$hmm_dir/means";
my $varfn   = "$hmm_dir/variances";
my $minvar  = 1e-4;

my $topn     = 4;
my $logdir   = "$ST::CFG_LOG_DIR/$processname";
mkdir ($logdir,0777);

# If there is an LDA transformation, use it
my @lda_args;
if (defined($ST::CFG_LDA_TRANSFORM) and -r $ST::CFG_LDA_TRANSFORM) {
    push(@lda_args,
	 -ldafn => $ST::CFG_LDA_TRANSFORM,
	 -ldadim => $ST::CFG_LDA_DIMENSION);
}

my ($listoffiles, $transcriptfile, $logfile);
$listoffiles = catfile($ST::CFG_LIST_DIR, "$speaker.ctl");
$transcriptfile = catfile($ST::CFG_LIST_DIR, "$speaker.lsn");
my @mllr_args;
# If we have an MLLR transform, apply it in Baum-Welch
if ($iter > 1) {
    # FIXME: Support multi-class MLLR
    @mllr_args = (-mllrmat => catfile($ST::CFG_MODEL_DIR, $modelname,
				      "$ST::CFG_EXPTNAME.$speaker.mllr"));
}
$logfile  = "$logdir/${ST::CFG_EXPTNAME}.$iter-$speaker.bw.log";

my $ctl_counter = 0;
open INPUT,"<$listoffiles" or die "Failed to open $listoffiles: $!";
while (<INPUT>) {
    $ctl_counter++;
}
close INPUT;
$ctl_counter = 1 unless ($ctl_counter);

copy "$ST::CFG_GIF_DIR/green-ball.gif", "$ST::CFG_BASE_DIR/.60.sa.bw.$iter.$speaker.state.gif";
HTML_Print ("\t" . ImgSrc("$ST::CFG_BASE_DIR/.60.sa.bw.$iter.$speaker.state.gif") . " ");
Log ("    Baum welch starting for iteration: $iter speaker: $speaker ");
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
     -cepdir => $ST::CFG_FEATFILES_DIR,
     -cepext => $ST::CFG_FEATFILE_EXTENSION,
     -ctlfn => $listoffiles,
     -lsnfn => $transcriptfile,
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
     '-2passvar' => 'yes',
     -tmatreest => "yes",
     -fullvar => $ST::CFG_FULLVAR,
     -diagfull => $ST::CFG_DIAGFULL,
     -feat => $ST::CFG_FEATURE,
     -ceplen => $ST::CFG_VECTOR_LENGTH,
     @lda_args,
     @mllr_args,
     -timing => "no");


if ($return_value) {
  copy "$ST::CFG_GIF_DIR/red-ball.gif", "$ST::CFG_BASE_DIR/.60.sa.bw.$iter.$speaker.state.gif";
  LogError ("\tbw failed\n");
}
exit ($return_value);
