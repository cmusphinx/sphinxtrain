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

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

$| = 1; # Turn on autoflushing

die "USAGE: $0 <clustdir> <iter> <clust>" if @ARGV != 3;
my ($clustdir, $iter, $clust) = @ARGV;

my $modelname="${ST::CFG_EXPTNAME}.cd_${ST::CFG_DIRLABEL}_$ST::CFG_N_TIED_STATES";
my $processpart="55.sa_cluster";

opendir(ACCUMDIR, $clustdir) or die "Could not open $clustdir: $!";
my @bwaccumdirs = map catdir($clustdir, $_),
    grep /^${ST::CFG_EXPTNAME}\.buff_${clust}_/, readdir(ACCUMDIR);
closedir(ACCUMDIR);
my $hmmdir              = "${ST::CFG_BASE_DIR}/model_parameters/$modelname";
mkdir ($hmmdir,0777) unless -d $hmmdir;
my $means               = "$hmmdir/means";
my $variances           = "$hmmdir/variances";
my $mllr                = catfile($clustdir, "clust.$clust.$iter.mllr");

my $logfile = "$clustdir/${ST::CFG_EXPTNAME}.${clust}.${iter}.mllr.log";

copy "$ST::CFG_GIF_DIR/green-ball.gif", "$ST::CFG_BASE_DIR/.55.mllr.${clust}.$iter.state.gif";
HTML_Print ("\t" . ImgSrc("$ST::CFG_BASE_DIR/.55.mllr.${clust}.$iter.state.gif") . " ");
Log ("    MLLR estimation for cluster: $clust iteration: $iter ");
HTML_Print (FormatURL("$logfile", "Log File") . " ");

my $return_value = RunTool
    ('mllr_solve', $logfile, 0,
     -accumdir => @bwaccumdirs,
     -meanfn => $means,
     -varfn => $variances,
     -outmllrfn => $mllr
    );

if ($return_value) {
  copy "$ST::CFG_GIF_DIR/red-ball.gif", "$ST::CFG_BASE_DIR/.55.mllr.${clust}.$iter.state.gif";
  LogError ("\tFailed to start mllr_solve \n");
}
exit ($return_value);
