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
# this script performs baum-welch training using
# it needs as inputs an initial set of models in s3 format
# a mdef file and cepstra with transcription files.
#************************************************************************

$| = 1; # Turn on autoflushing

die "USAGE: $0 <ngau> <iter> <part> <npart>" if (@ARGV != 4);
my ($n_gau, $iter, $part, $npart) = @ARGV;

use vars qw($MLLT_FILE $MODEL_TYPE);
$MLLT_FILE = catfile($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.mllt");
$MODEL_TYPE = 'cd';

my $modelinitialname="${ST::CFG_EXPTNAME}.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_initial";
my $modelname="${ST::CFG_EXPTNAME}.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}";
my $mdefname="${ST::CFG_EXPTNAME}.$ST::CFG_N_TIED_STATES.mdef";
my $processname = "50.${MODEL_TYPE}_hmm_tied";

my $output_buffer_dir = "$ST::CFG_BWACCUM_DIR/${ST::CFG_EXPTNAME}_buff_${part}";
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
my $mwfloor = 1e-5;
my $tpfloor = 1e-5;
my $tmatfn  = "$hmm_dir/transition_matrices";
my $meanfn  = "$hmm_dir/means";
my $varfn   = "$hmm_dir/variances";
my $minvar  = 1e-4;

# Add the MLLT transform if it exists
my @extra_args;
if (defined($ST::CFG_SVSPEC)){
    push(@extra_args, -svspec =>$ST::CFG_SVSPEC);
}
if (-r $MLLT_FILE) {
    push(@extra_args,
	 -lda => $MLLT_FILE,
	 -ldadim => $ST::CFG_LDA_DIMENSION);
}

if ($ST::CFG_CD_VITERBI eq 'yes') {
    push(@extra_args, -viterbi => 'yes');
}

if (defined($ST::CFG_PHSEG_DIR)) {
    open INPUT,"${ST::CFG_LISTOFFILES}" or die "Failed to open $ST::CFG_LISTOFFILES: $!";
    # Check control file format (determines if we need -outputfullpath)
    my $line = <INPUT>;
    if (split(" ", $line) ==1) {
	# Use full file path
	push(@extra_args, -outputfullpath => 'yes');
    }
    close INPUT;
    push(@extra_args,
	 -phsegdir => $ST::CFG_PHSEG_DIR);
}

# aligned transcripts and the list of aligned files is obtained as a result
# of (03.) forced alignment or (04.) VTLN
my ($listoffiles, $transcriptfile) = GetLists();

my $topn = $n_gau;
$topn = $ST::CFG_CD_TOPN if (defined($ST::CFG_CD_TOPN) and 
			     ($iter > 1) and 
			     ($ST::CFG_FULLVAR eq 'no') and 
			     ($ST::CFG_CD_TOPN < $n_gau));

my $logdir   = "$ST::CFG_LOG_DIR/$processname";
my $logfile  = "$logdir/${ST::CFG_EXPTNAME}.${n_gau}.$iter-$part.bw.log";
mkdir ($logdir,0777);

my $ctl_counter = 0;
open INPUT,"<$listoffiles" or die "Failed to open $listoffiles: $!";
while (<INPUT>) {
    $ctl_counter++;
}
close INPUT;
$ctl_counter = int ($ctl_counter / $npart) if $npart;
$ctl_counter = 1 unless ($ctl_counter);

Log("Baum welch starting for $n_gau Gaussian(s), iteration: $iter ($part of $npart)",
    'result');

my $return_value = RunTool
    ('bw', $logfile, $ctl_counter,
     -moddeffn => $moddeffn,
     -ts2cbfn => $statepdeffn,
     -mixwfn => $mixwfn,
     -mwfloor => $mwfloor,
     -tpfloor => $tpfloor,
     -tmatfn => $tmatfn,
     -meanfn => $meanfn,
     -varfn => $varfn,
     -dictfn => GetDict(),
     -fdictfn => $ST::CFG_FILLERDICT,
     -ctlfn => $listoffiles,
     -part => $part,
     -npart => $npart,
     -cepdir => $ST::CFG_FEATFILES_DIR,
     -cepext => $ST::CFG_FEATFILE_EXTENSION,
     -lsnfn => $transcriptfile,
     -accumdir => $output_buffer_dir,
     -varfloor => $minvar,
     -topn => $topn,
     -abeam => 1e-90,
     -bbeam => 1e-10,
     -agc => $ST::CFG_AGC,
     -cmn => $ST::CFG_CMN,
     -varnorm => $ST::CFG_VARNORM,
     -meanreest => "yes",
     -varreest => "yes",
     '-2passvar' => $var2pass,
     -tmatreest => "yes",
     -fullvar => $ST::CFG_FULLVAR,
     -diagfull => $ST::CFG_DIAGFULL,
     -feat => $ST::CFG_FEATURE,
     -ceplen => $ST::CFG_VECTOR_LENGTH,
     @extra_args,
     -timing => "no");


if ($return_value) {
  LogError("Failed to start bw");
}
exit ($return_value);
