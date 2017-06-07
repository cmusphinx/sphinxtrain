#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 1996-201 Carnegie Mellon University.  All rights 
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
## Author: Jidong Tao <vjdtao@hotmail.com>
#
# ====================================================================
#
#  Script that launches the feature exaction script
#
# ====================================================================

use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

#************************************************************************
# this script performs feature extraction.
# it needs as inputs a control file, audio file extension, etc.
#************************************************************************

$| = 1; # Turn on autoflushing
die "USAGE: $0 <part> <npart> [<exptid> <control> <warp>]" if @ARGV < 2;
my ($part, $npart, $exptid, $ctlfile, $warp) = @ARGV;

$part = 1 unless defined($part);
$npart = 1 unless defined($npart);

$exptid = $ST::CFG_EXPTNAME unless defined($exptid);
$ctlfile = $ST::CFG_LISTOFFILES unless defined($ctlfile);
$outfolder = $ST::CFG_FEATFILES_DIR unless defined($warp);
$processname = "000.comp_feat";
$log_dir = "$ST::CFG_LOG_DIR/$processname";
mkdir ($log_dir,0777) unless -d $log_dir;
$logfile = "$log_dir/${exptid}-${part}-${npart}.log";

# Defines input audio format
$wavtype = $ST::CFG_WAVFILE_TYPE unless defined($wavtype);
$nist = "no"; $raw = "no"; $mswav = "no";
if ($wavtype eq 'nist') {$nist = "yes";}
if ($wavtype eq 'raw') {$raw = "yes";}
if ($wavtype eq 'mswav') {$mswav = "yes";}


my @warp_args;
if (defined($warp)) {
    $outfolder = catdir("$ST::CFG_BASE_DIR/feat", $warp);
    $logfile = "$log_dir/${exptid}-${part}-${npart}-${warp}.log";
    push(@warp_args, -warp_params => $warp);
}

open CTL, "<$ctlfile" or die "Failed to open control file $ctlfile: $!";
while (<CTL>) {
    chomp;
    my $dir = dirname($_);
    mkpath(catdir($ST::CFG_FEATFILES_DIR, $dir));
}
close CTL;

if (defined($warp)) {
    Log("Extracting features from $ctlcount segments starting at $ctloffset with warp factor $warp (part $part of $npart)\n", 'audio files');
} else {
    Log("Extracting features from $ctlcount segments starting at $ctloffset (part $part of $npart) ", 'audio files');
}

my $rv = RunTool('sphinx_fe', $logfile, $ctlcount,
		  -c => $ctlfile,
		  -part => $part,
		  -npart => $npart,
		  -di => $ST::CFG_WAVFILES_DIR,
		  -ei => $ST::CFG_WAVFILE_EXTENSION,
		  -do => $outfolder,
		  -eo => $ST::CFG_FEATFILE_EXTENSION,
		  -nist => $nist,
		  -raw => $raw,
		  -mswav => $mswav,
		  -samprate => $ST::CFG_WAVFILE_SRATE,
		  -lowerf => $ST::CFG_LO_FILT,
		  -upperf => $ST::CFG_HI_FILT,
		  -nfilt => $ST::CFG_NUM_FILT,
		  -transform => $ST::CFG_TRANSFORM,
		  -lifter => $ST::CFG_LIFTER,
		  -ncep => $ST::CFG_VECTOR_LENGTH,
	          @warp_args);

if ($rv) {
    LogError("Failed to start ${ST::CFG_BIN_DIR}/sphinx_fe");
}

exit ($rv);
