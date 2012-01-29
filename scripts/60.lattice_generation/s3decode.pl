#!/usr/bin/perl
# ====================================================================
#
# Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
# reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer. 
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# This work was supported in part by funding from the Defense Advanced 
# Research Projects Agency and the National Science Foundation of the 
# United States of America, and the CMU Sphinx Speech Consortium.
#
# THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
# ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
# NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ====================================================================
#
# Author: Evandro Gouvea
# Modified: Long Qin
#

use File::Path;
use File::Copy;
use File::Basename;
use File::Spec::Functions;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

#************************************************************************
# this script performs decoding.
# it needs as inputs a set of models in s3 format
# a mdef file and cepstra with transcription files.
#************************************************************************

$| = 1; # Turn on autoflushing

die "Usage: $0 <part> <nparts>\n" unless @ARGV == 2;
my ($part, $npart) = @ARGV;

my $log_dir     = "$ST::CFG_LOG_DIR/60.lattice_generation";
my $logfile     = "$log_dir/${ST::CFG_EXPTNAME}.${part}.decode.log";
my $statepdeffn = $ST::CFG_HMM_TYPE;
my $hmm_dir     = "$ST::CFG_MODEL_DIR/$ST::CFG_EXPTNAME.cd_${ST::CFG_DIRLABEL}_${ST::CFG_N_TIED_STATES}";

my $numlatdir = defined($ST::CFG_NUMLAT_DIR)
    ? $ST::CFG_NUMLAT_DIR
    : "$ST::CFG_BASE_DIR/numlat";
my $listoffiles;
if (-e "$numlatdir/${ST::CFG_EXPTNAME}.alignedfiles") {
    $listoffiles = "$numlatdir/${ST::CFG_EXPTNAME}.alignedfiles";
}elsif ($ST::CFG_FORCEDALIGN eq "yes") {
    $listoffiles   = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedfiles";
} elsif ($ST::CFG_VTLN eq "yes") {
    $listoffiles   = "$ST::CFG_BASE_DIR/vtlnout/${ST::CFG_EXPTNAME}.alignedfiles";
} else {
    $listoffiles = $ST::CFG_LISTOFFILES;
}

# Get the number of utterances
open INPUT,"$listoffiles" or die "Failed to open $listoffiles: $!";
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

my $denlatdir = defined($ST::CFG_DENLAT_DIR)
    ? $ST::CFG_DENLAT_DIR
    : "$ST::CFG_BASE_DIR/denlat";
my $outlatdir = "$denlatdir$ctlext";

my $nlines = 1;
while (<INPUT>) {
    $nlines++;
}
close INPUT;
my $ctloffset = int ( ( $nlines * ( $part - 1 ) ) / $npart );
my $ctlcount = int ( ( $nlines * $part ) / $npart ) - $ctloffset;

my $lw    = defined($ST::CFG_LANGUAGEWEIGHT) ? $ST::CFG_LANGUAGEWEIGHT : "11.5";
my $beam  = defined($ST::CFG_BEAMWIDTH) ? $ST::CFG_BEAMWIDTH : "1e-100";
my $wbeam = defined($ST::CFG_WORDBEAM) ? $ST::CFG_WORDBEAM : "1e-80";
my $wip   = defined($ST::CFG_WORDPENALTY) ? $ST::CFG_WORDPENALTY : "0.2";

Log("Decoding $ctlcount segments starting at $ctloffset (part $part of $npart) ", 'result');
my $rv = RunTool('sphinx3_decode', $logfile, $ctlcount,
		 -senmgau => $statepdeffn,
		 -hmm => $hmm_dir,
		 -lw => $lw,
		 -feat => $ST::CFG_FEATURE,
		 -beam => $beam,
		 -wbeam => $wbeam,
		 -dict => GetDict(),
		 -fdict => $ST::CFG_FILLERDICT,
		 -lm => $ST::CFG_LANGUAGEMODEL,
		 -wip => $wip,
		 -ctl => $listoffiles,
		 -ctloffset => $ctloffset,
		 -ctlcount => $ctlcount,
		 -cepdir => $ST::CFG_FEATFILES_DIR,
		 -cepext => ".$ST::CFG_FEATFILE_EXTENSION",
		 -agc => $ST::CFG_AGC,
		 -varnorm => $ST::CFG_VARNORM,
		 -cmn => $ST::CFG_CMN,
		 -ugonly => "yes",
		 -outlatdir => $outlatdir);

if ($rv) {
  LogError("Failed to run sphinx3_decode");
}
exit ($rv);
