#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 2006 Carnegie Mellon University.  All rights
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
use File::Temp;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

die "Usage: $0 <warp> <part> <nparts>\n" unless @ARGV == 3;
my ($warp, $part, $npart) = @ARGV;

my $featdir = catdir("$ST::CFG_FEATFILES_DIR", $warp);

my $hmm_dir = defined($ST::CFG_FORCE_ALIGN_MODELDIR)
    ? $ST::CFG_FORCE_ALIGN_MODELDIR
    : "$ST::CFG_MODEL_DIR/$ST::CFG_EXPTNAME.ci_$ST::CFG_DIRLABEL";
my $logdir = "$ST::CFG_LOG_DIR/12.vtln_align";
my $indir = "$ST::CFG_BASE_DIR/vtlnout";
my $outdir = "$ST::CFG_BASE_DIR/vtlnout/$warp";
mkdir($outdir,0777);
my $outfile = "$outdir/$ST::CFG_EXPTNAME.alignedtranscripts.$part";

my $statepdeffn = $ST::CFG_HMM_TYPE; # indicates the type of HMMs
my $mwfloor = 1e-8;
my $minvar  = 1e-4;
my $listoffiles = $ST::CFG_LISTOFFILES;
my $transcriptfile = "$indir/$ST::CFG_EXPTNAME.aligninput";
my $dict = defined($ST::CFG_FORCE_ALIGN_DICTIONARY)
    ? $ST::CFG_FORCE_ALIGN_DICTIONARY
    : "$indir/$ST::CFG_EXPTNAME.falign.dict";
my $fdict = defined($ST::CFG_FORCE_ALIGN_FILLERDICT)
    ? $ST::CFG_FORCE_ALIGN_FILLERDICT
    : "$indir/$ST::CFG_EXPTNAME.falign.fdict";
my $beam = defined($ST::CFG_FORCE_ALIGN_BEAM) ? $ST::CFG_FORCE_ALIGN_BEAM : 1e-100;
my $logfile  = "$logdir/${ST::CFG_EXPTNAME}.$warp.$part.vtln.log";

# if there is an MLLT transformation, use it
my $mllt_file = catfile($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.mllt");
my @feat_args;
if (defined($ST::CFG_SVSPEC)){
    # This is not actually guaranteed to work with Sphinx3 :(
    push(@feat_args, -svspec => $ST::CFG_SVSPEC);
}
if (-r $mllt_file) {
    push(@feat_args,
     -lda => $mllt_file,
     -ldadim => $ST::CFG_LDA_DIMENSION);
}


# Get the number of utterances
open INPUT,"${ST::CFG_LISTOFFILES}" or die "Failed to open $ST::CFG_LISTOFFILES: $!";
# Check control file format (determines if we add ,CTL below)
my $line = <INPUT>;
my $ctlext;
if (split(" ", $line) ==1) {
    # Use full file path
    $ctlext = ",CTL";
}
else {
    # Use utterance ID
    $ctlext = "";
}
my $ctl_counter = 1;
while (<INPUT>) {
    $ctl_counter++;
}
close INPUT;
$ctl_counter = int ($ctl_counter / $npart) if $npart;
$ctl_counter = 1 unless ($ctl_counter);

Log("Force alignment starting: ($part of $npart) ", 'result');

my $return_value = RunTool
    ('sphinx3_align', $logfile, $ctl_counter,
     -senmgau => $statepdeffn,
     -mixwfloor => $mwfloor,
     -hmm => $hmm_dir,
     -varfloor => $minvar,
     -dict => $dict,
     -fdict => $fdict,
     -ctl => $ST::CFG_LISTOFFILES,
     -ctloffset => $ctl_counter * ($part-1),
     -ctlcount => $ctl_counter,
     -cepdir => $featdir,
     -cepext => ".$ST::CFG_FEATFILE_EXTENSION",
     -insent => $transcriptfile,
     -outsent => $outfile,
     -wdsegdir => "$outdir$ctlext",
     -beam => $beam,
     -agc => $ST::CFG_AGC,
     -cmn => $ST::CFG_CMN,
     -varnorm => $ST::CFG_VARNORM,
     -feat => $ST::CFG_FEATURE,
     -ceplen => $ST::CFG_VECTOR_LENGTH,
     @feat_args
     );

if ($return_value) {
  LogError("Failed to run sphinx3_align");
}
exit ($return_value);
