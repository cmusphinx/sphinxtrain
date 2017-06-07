#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 1996-2009 Carnegie Mellon University.  All rights 
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
## Script to test the consistency of the test set
##
## Author: Ricky Houghton (converted from scripts by Rita Singh)
## Author: David Huggins-Daines (converted from scripts by Rita Singh)
## Author: Nickolay V. Shmyrev (adapted from the verify_all.pl)
use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

$| = 1;				# Turn on autoflushing
my $ret_value = 0;

Log("MODULE: 00 verify testing files");

# Check to make sure .ctl file is roughly of correct format
# 1.) Check that each utterance specified in the .ctl file has a positive length
#     Verify that the files listed are available and are not of size 0
# 2.) Check number of lines in the transcript and in ctl - they should be the same";
{
    my ($status,@ctl_lines,$ctl_line,$file,$start,$end,$number_ctl_lines,$number_lines_transcript);
    
    open CTL,"$ST::DEC_CFG_LISTOFFILES" or die "Can not open listoffiles ($ST::DEC_CFG_LISTOFFILES)";
    @ctl_lines = <CTL>;		# We are going to iterate over this several times
    close CTL;

    # 3.) Check that each utterance specified in the .ctl file has a positive length
    #     Verify that the files listed are available and are not of size 0

    Log("Phase 1: CTL - Check general format; utterance length (must be positive); files exist");
    $status = 'passed';
    my $estimated_training_data = 0;
    for $ctl_line (@ctl_lines) {
        chomp($ctl_line);
	# Accept: filename int int possible_comment
	if ($ctl_line =~ m/^\s*(\S+)\s+(\d+)\s+(\d+).*/) {
	    $file = $1;
	    $start = $2;
	    $end = $3;
	    if ((defined $start) and (defined $file)) {
		if ($end <= $start) {
		    warn "Utterance length is <= 0: $start -> $end ($ctl_line)";
		    $status = 'FAILED';
		    $ret_value = -3;
		}

		if (! -s "$ST::CFG_FEATFILES_DIR/$file.$ST::CFG_FEATFILE_EXTENSION") {
		    $ret_value = -4;
		    $status = 'FAILED';
		    LogWarning ("This file, $ST::CFG_FEATFILES_DIR/$file.$ST::CFG_FEATFILE_EXTENSION, does not exist");
		}
	    }
	} else {
	    # Accepts only the file name and possible comment on line by itself..no start/send markers
	    if ($ctl_line =~ m/^(\S+)(\s.*)?$/) {
		$file = $1;
		my $size = -s "$ST::CFG_FEATFILES_DIR/$file.$ST::CFG_FEATFILE_EXTENSION";
		# 1 frame = 13 floating point numbers = 13*4bytes = 52 bytes (only valid for MFC files)
		$estimated_training_data += ($size / 52) if (lc($ST::CFG_FEATFILE_EXTENSION) eq 'mfc');
		if (! $size) {
		    $ret_value = -4;
		    $status = 'FAILED';
		    LogWarning ("CTL file, $ST::CFG_FEATFILES_DIR/$file.$ST::CFG_FEATFILE_EXTENSION, does not exist");
		}
	    } else {
		$status = 'FAILED';
		$ret_value = -5;
		LogWarning ("CTL line does not parse correctly:\n$ctl_line");
	    }
	}
    }
    LogStatus($status);
    
    $number_ctl_lines = $#ctl_lines + 1;

    
    # 2) Check number of lines in the transcript and in ctl - they should be the same";
    Log ("Phase 2: CTL - Checking number of lines in the transcript should match lines in control file");
    open TRN,"$ST::DEC_CFG_TRANSCRIPTFILE" or die "Can not open Transcript file ($ST::DEC_CFG_TRANSCRIPTFILE)";
    my $number_transcript_lines = 0;
    while (<TRN>) {
	$number_transcript_lines++;
    }
    close TRN;
    
    $status = ($number_ctl_lines == $number_transcript_lines) ? 'passed' : 'FAILED';
    LogStatus($status);

    @ctl_lines = ();
}

my %transcript_phonelist_hash = ();

# Verify that all transcription words are in the dictionary, and all
# phones are covered
{
    Log("Phase 3: TRANSCRIPT - Checking that all the words in the transcript are in the dictionary");
    open DICT,"$ST::CFG_DICTIONARY" or die "Can not open the dictionary ($ST::CFG_DICTIONARY)";
    my @dict = <DICT>;
    close DICT;
    my $ndict = @dict;
    Log("Words in dictionary: $ndict", 'result');

    my %d;
    for (@dict) {		# Create a hash of the dict entries
	/(\S+)\s+(.*)$/;
	$d{$1} = $2;
    }
    
    open DICT,"$ST::CFG_FILLERDICT" or die "Can not open filler dict ($ST::CFG_FILLERDICT)";
    my @fill_dict = <DICT>;
    close DICT;
    $ndict = @fill_dict;
    Log ("Words in filler dictionary: $ndict", 'result');
    
    for (@fill_dict) {		# Create a hash of the dict entries
	/(\S+)\s+(.*)$/;
	$d{$1} = $2;
    }
    
    @dict = undef;			# not needed
    @fill_dict = undef;		# not needed
    
    open TRN,"$ST::DEC_CFG_TRANSCRIPTFILE" or die "Can not open the transcript file ($ST::DEC_CFG_TRANSCRIPTFILE)"; 
    
    my $status = 'passed';
    while (<TRN>) {
	my ($text) = m/(.*)\s*\(.*\)$/;
	if ($text) {
	    my @words = split /\s+/,$text;
	    for my $word (@words) {
		if ($ST::CFG_G2P_MODEL ne 'yes' and !($d{$word}) and ($word =~ m/\S+/)) {
		    LogWarning ("This word: $word was in the transcript file, but is not in the dictionary ($text). Do cases match?");
		    $status = 'FAILED';
		    $ret_value = -5;
		} else {
		    my @phones = ($d{$word} =~ m/(\S+)/g);
		    for my $phone (@phones) {
		        $transcript_phonelist_hash{$phone} = 1;
		    }
		}
	    }
	}
    }
    close TRN;
    LogStatus($status);
}

mkdir ($ST::CFG_LOG_DIR,0755);
mkdir ($ST::CFG_BWACCUM_DIR,0755);

exit ($ret_value);

