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

$| = 1; # Turn on autoflushing

# FIXME: Too many arguments?
die "USAGE: $0 <clustdir> <iter> <clust> <part> <npart> <ctl> <lsn> estimate|evaluate [<mllr>]" if (@ARGV < 8);
my ($clustdir, $iter, $clust, $part, $npart,
    $listoffiles, $transcriptfile, $action, $mllrfile) = @ARGV;

my $modelname="${ST::CFG_EXPTNAME}.cd_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}";
my $mdefname="${ST::CFG_EXPTNAME}.$ST::CFG_N_TIED_STATES.mdef";
my $processname = "55.sa_cluster";

my $hmm_dir     = "$ST::CFG_BASE_DIR/model_parameters/$modelname";
my $moddeffn    = "$ST::CFG_BASE_DIR/model_architecture/$mdefname";
my $statepdeffn = $ST::CFG_HMM_TYPE; # indicates the type of HMMs
my $mixwfn  = "$hmm_dir/mixture_weights";
my $mwfloor = 1e-8;
my $tmatfn  = "$hmm_dir/transition_matrices";
my $meanfn  = "$hmm_dir/means";
my $varfn   = "$hmm_dir/variances";
my $minvar  = 1e-4;

# We could either be estimating for an MLLR transform or evaluating
# the results of one.  Do different stuff if so
my @extra_args;
my $reest;
my $logfile = "$clustdir/${ST::CFG_EXPTNAME}.$action.$clust.$iter-$part.bw.log";
if ($action eq 'evaluate') {
    $reest = 'no';
}
elsif ($action eq 'estimate') {
    $reest = 'yes';
    my $output_buffer_dir = catdir($clustdir, "${ST::CFG_EXPTNAME}.buff_${clust}_${part}");
    mkdir ($output_buffer_dir,0777);
    push @extra_args,
	-accumdir => $output_buffer_dir;
}
else {
    die "Action must be 'estimate' or 'evaluate'";
}
# if there is an LDA transformation, use it
if (defined($ST::CFG_LDA_TRANSFORM) and -r $ST::CFG_LDA_TRANSFORM) {
    push(@extra_args,
	 -ldafn => $ST::CFG_LDA_TRANSFORM,
	 -ldadim => $ST::CFG_LDA_DIMENSION);
}
# if there is an MLLR transformation, use it
push @extra_args, -mllrmat => $mllrfile if defined($mllrfile);

my $topn     = 4;

my $ctl_counter = 0;
open INPUT,"<$listoffiles" or die "Failed to open $listoffiles: $!";
while (<INPUT>) {
    $ctl_counter++;
}
close INPUT;
$ctl_counter = int ($ctl_counter / $npart) if $npart;
$ctl_counter = 1 unless ($ctl_counter);

copy "$ST::CFG_GIF_DIR/green-ball.gif", "$ST::CFG_BASE_DIR/.55.bw.$clust.$iter.$part.state.gif";
HTML_Print ("\t" . ImgSrc("$ST::CFG_BASE_DIR/.55.bw.$clust.$iter.$part.state.gif") . " ");
Log ("    Baum welch starting for cluster $clust, iteration: $iter ($part of $npart) ");
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
     -ctlfn => $listoffiles,
     -part => $part,
     -npart => $npart,
     -cepdir => $ST::CFG_FEATFILES_DIR,
     -cepext => $ST::CFG_FEATFILE_EXTENSION,
     -lsnfn => $transcriptfile,
     -varfloor => $minvar,
     -topn => $topn,
     -abeam => 1e-90,
     -bbeam => 1e-40,
     -agc => $ST::CFG_AGC,
     -cmn => $ST::CFG_CMN,
     -varnorm => $ST::CFG_VARNORM,
     -meanreest => $reest,
     -varreest => $reest,
     -tmatreest => $reest,
     '-2passvar' => "yes",
     -feat => $ST::CFG_FEATURE,
     -ceplen => $ST::CFG_VECTOR_LENGTH,
     @extra_args,
     -outputfullpath => "yes",
     -timing => "no");


if ($return_value) {
  copy "$ST::CFG_GIF_DIR/red-ball.gif", "$ST::CFG_BASE_DIR/.55.bw.$clust.$iter.$part.state.gif";
  LogError ("\tFailed to start bw \n");
}
exit ($return_value);
