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
## Modified: Long Qin
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

die "USAGE: $0 <iter> <part> <npart> <lat_ext>" if @ARGV != 4;
my ($iter, $part, $npart, $lat_ext) = @ARGV;

my $logdir   = "$ST::CFG_LOG_DIR/65.mmie_train";
my $logfile  = "$logdir/${ST::CFG_EXPTNAME}.$iter-$part.bw.$lat_ext.log";
mkdir ($logdir,0777);

my $MODEL_TYPE  = 'cd';
my $modelname   = "${ST::CFG_EXPTNAME}.${MODEL_TYPE}_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}_mmie";

my $buffer_dir = "$ST::CFG_BWACCUM_DIR/${ST::CFG_EXPTNAME}_buff_${part}";
mkdir ($buffer_dir,0777);

my $hmm_dir     = "$ST::CFG_BASE_DIR/model_parameters/$modelname/iter_$iter";
my $moddeffn    = "$hmm_dir/mdef";
my $statepdeffn = $ST::CFG_HMM_TYPE;
my $mixwfn      = "$hmm_dir/mixture_weights";
my $mwfloor     = 1e-5;
my $tpfloor     = 1e-5;
my $tmatfn      = "$hmm_dir/transition_matrices";
my $meanfn      = "$hmm_dir/means";
my $varfn       = "$hmm_dir/variances";
my $minvar      = 1e-4;

my $topn = $ST::CFG_FINAL_NUM_DENSITIES;
$topn = $ST::CFG_CD_TOPN if (defined($ST::CFG_CD_TOPN) and ($ST::CFG_FULLVAR eq 'no') and ($ST::CFG_CD_TOPN < $ST::CFG_FINAL_NUM_DENSITIES));

my $mmie_type   = defined($ST::CFG_MMIE_TYPE) ? $ST::CFG_MMIE_TYPE : "rand";
my $lw          = defined($ST::CFG_LANGUAGEWEIGHT) ? $ST::CFG_LANGUAGEWEIGHT : "11.5";

my $numlatdir = defined($ST::CFG_NUMLAT_DIR)
    ? $ST::CFG_NUMLAT_DIR
    : "$ST::CFG_BASE_DIR/numlat";
my $latdir = defined($ST::CFG_LATTICE_DIR)
            ? $ST::CFG_LATTICE_DIR
    : "$ST::CFG_BASE_DIR/lattice";

my ($listoffiles, $transcriptfile);
if (-d $numlatdir) {
    $listoffiles = "$numlatdir/${ST::CFG_EXPTNAME}.alignedfiles";
    $transcriptfile = "$numlatdir/${ST::CFG_EXPTNAME}.alignedtranscripts";
} elsif ($ST::CFG_FORCEDALIGN eq "yes") {
    $listoffiles   = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedfiles";
    $transcriptfile = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedtranscripts";
} elsif ($ST::CFG_VTLN eq "yes") {
    $listoffiles   = "$ST::CFG_BASE_DIR/vtlnout/${ST::CFG_EXPTNAME}.alignedfiles";
    $transcriptfile = "$ST::CFG_BASE_DIR/vtlnout/${ST::CFG_EXPTNAME}.alignedtranscripts";
} else {
    $listoffiles = $ST::CFG_LISTOFFILES;
    $transcriptfile = $ST::CFG_TRANSCRIPTFILE;
}

# count how many files in the control file
my $ctl_counter = 0;
open INPUT,"<$listoffiles" or die "Failed to open $listoffiles: $!";
while (<INPUT>) {
    $ctl_counter++;
}
close INPUT;
$ctl_counter = int ($ctl_counter / $npart) if $npart;
$ctl_counter = 1 unless ($ctl_counter);

Log ("Baum welch starting for iteration: $iter ($part of $npart) ", 'result');
Log ("$lat_ext density accumulation", 'result');


use vars qw($MLLT_FILE);
$MLLT_FILE = catfile($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.mllt");

my @extra_args;
if (-r $MLLT_FILE) {
    push(@extra_args,
	 -lda => $MLLT_FILE,
	 -ldadim => $ST::CFG_LDA_DIMENSION);
}

my $return_value = RunTool
    ('bw', $logfile, $ctl_counter,
     -moddeffn => $moddeffn,
     -ts2cbfn => $statepdeffn,
     -mixwfn => $mixwfn,
     -mwfloor => $mwfloor,
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
     -accumdir => $buffer_dir,
     -varfloor => $minvar,
     -topn => $topn,
     -abeam => 1e-90,
     -bbeam => 1e-10,
     -agc => $ST::CFG_AGC,
     -cmn => $ST::CFG_CMN,
     -varnorm => $ST::CFG_VARNORM,
     -meanreest => "yes",
     -varreest => "yes",
     '-2passvar' => "no",
     -mixwreest => "no",
     -tmatreest => "no",
     -fullvar => $ST::CFG_FULLVAR,
     -diagfull => $ST::CFG_DIAGFULL,
     -feat => $ST::CFG_FEATURE,
     -ceplen => $ST::CFG_VECTOR_LENGTH,
     @extra_args,
     -timing => "no",
     -mmie => "yes",
     -mmie_type => $mmie_type,
     -latext => $lat_ext,
     -latdir => $latdir,
     -lw => $lw);

if ($return_value) {
  LogError("Failed to run bw");
}
exit ($return_value);
