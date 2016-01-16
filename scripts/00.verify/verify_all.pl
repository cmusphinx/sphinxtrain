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
## Author: Ricky Houghton (converted from scripts by Rita Singh)
## Author: David Huggins-Daines (converted from scripts by Rita Singh)
##
use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

$| = 1;				# Turn on autoflushing
my $ret_value = 0;

Log("MODULE: 00 verify training files");

# PHASE 1: Check to see if the phones in the dictionary are listed in the phonelist file
# PHASE 2: Check to make sure there are not duplicate entries in the dictionary
my %phonelist_hash;
{
    open DICT,"$ST::CFG_DICTIONARY" or die "Can not open the dictionary ($ST::CFG_DICTIONARY)";

    my %dict_phone_hash = ();
    my %dict_hash = ();

    Log("Phase 1: Checking to see if the dict and filler dict agrees with the phonelist file.");
    # This is rather ugly, but it's late and I'm just trying to get the pieces together
    # Clean it up later

    # Read the dictionary and stick phones into dict_phone_hash
    my $counter =0;
    while (<DICT>) {
	if (/^(\S+)\s(.*)$/) {
	    $dict_hash{$1}++;
	    my $phonetic = $2;
	    # Aggregate the non-space characters and store the results
	    # in @phone
	    my @phones = ($phonetic =~ m/(\S+)/g);
	    for my $phone (@phones) {
		$dict_phone_hash{$phone}++;
	    }
	}
	$counter++;
    }
    close DICT;

    open DICT,"$ST::CFG_FILLERDICT" or die "Can not open filler dict ($ST::CFG_FILLERDICT)\n";
    while (<DICT>) {
	if (/^(\S+)\s(.*)$/) {
	    $dict_hash{$1}++;
	    my $phonetic = $2;
	    my @phones = ($phonetic =~ m/(\S+)/g);
	    for my $phone (@phones) {
		$dict_phone_hash{uc($phone)}++;
	    }
	}
	$counter++;
    }
    close DICT;

    # Read the phonelist and stick phones into phonelist_hash
    open PHONE,"$ST::CFG_RAWPHONEFILE" or die "Can not open phone list ($ST::CFG_RAWPHONEFILE)\n";
    my $has_SIL = 0;
    my $has_DUP = 0;
    while (<PHONE>) {
	my $line = Trim($_);
	$has_SIL = 1 if ($line =~ m/^SIL$/);
	$has_DUP = 1 if (exists $phonelist_hash{$line});
	$phonelist_hash{$line} = 0;
    }
    close PHONE;

    unless ($has_SIL) {
	my $status = 'FAILED';
	$ret_value = -1;
	LogWarning("The phonelist ($ST::CFG_RAWPHONEFILE) does not define the phone SIL (required!)");
      }

    if ($has_DUP) {
	my $status = 'FAILED';
	$ret_value = -1;
	LogWarning("The phonelist ($ST::CFG_RAWPHONEFILE) has duplicated phones");
    }

    
    my $nkeys = keys %dict_phone_hash;
    Log("Found $counter words using $nkeys phones", 'result');
    
    my $status = 'passed';
    for my $key (sort (keys %dict_phone_hash)){
	if (defined($phonelist_hash{$key})) {
	    $phonelist_hash{$key} = 1;
	} else {
	    my $status = 'FAILED';
	    $ret_value = -1;
	    LogWarning ("This phone ($key) occurs in the dictionary ($ST::CFG_DICTIONARY), but not in the phonelist ($ST::CFG_RAWPHONEFILE)");
	}
    }

    for my $key (sort (keys %phonelist_hash)) {
      if ($phonelist_hash{$key} == 0) {
	    my $status = 'FAILED';
	    $ret_value = -1;
	    LogWarning ("This phone ($key) occurs in the phonelist ($ST::CFG_RAWPHONEFILE), but not in the dictionary ($ST::CFG_DICTIONARY)");
	}
    }

    LogStatus($status);
    Log("Phase 2: Checking to make sure there are not duplicate entries in the dictionary");
    my $duplicate_status = 'passed';
    for my $key (keys %dict_hash) {
	if ($dict_hash{$key} > 1) {
	    $ret_value = -2;
	    $duplicate_status = 'FAILED';
	    LogWarning("This word ($key) has duplicate entries in ($ST::CFG_DICTIONARY)");
	}
    }
    LogStatus($duplicate_status);
}


# Check to make sure .ctl file is roughly of correct format
# 3.) Check that each utterance specified in the .ctl file has a positive length
#     Verify that the files listed are available and are not of size 0
# 4.) Check number of lines in the transcript and in ctl - they should be the same";
#
# Collect utterance IDs for transcript verification
my @uttids;
{
    my ($status,@ctl_lines,$ctl_line,$file,$start,$end,$number_ctl_lines,$number_lines_transcript);
    
    open CTL,"$ST::CFG_LISTOFFILES" or die "Can not open listoffiles ($ST::CFG_LISTOFFILES)";
    @ctl_lines = <CTL>;		# We are going to iterate over this several times
    close CTL;

    # 3.) Check that each utterance specified in the .ctl file has a positive length
    #     Verify that the files listed are available and are not of size 0

    Log("Phase 3: Check general format for the fileids file; utterance length (must be positive); files exist");
    $status = 'passed';
    my $estimated_training_data = 0;
    # 3.5) Check that there is a newline at the end of the file (yes,
    # this causes problems if it's not there, sorry...)
    if ($ctl_lines[-1] !~ /\n$/) {
	$status = 'FAILED';
	$ret_value = -6;
	LogWarning("Error in '$ST::CFG_LISTOFFILES': missing a newline at end of file");
    }
    for $ctl_line (@ctl_lines) {
        chomp($ctl_line);
	# Accept: filename int int possible_uttid
	if ($ctl_line =~ m/^\s*(\S+)\s+(\d+)\s+(\d+)(\s+(\S+))?/) {
	    $file = $1;
	    $start = $2;
	    $end = $3;
	    if (defined($5)) {
		push @uttids, $5;
	    }
	    else {
		push @uttids, $file;
	    }
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
		$estimated_training_data += ($end - $start);
	    }
	} else {
	    # Accepts only the file name and possible comment on line by itself..no start/send markers
	    if ($ctl_line =~ m/^(\S+)(\s+.*)?$/) {
		$file = $1;
		my $size = -s "$ST::CFG_FEATFILES_DIR/$file.$ST::CFG_FEATFILE_EXTENSION";
		# 1 frame = 13 floating point numbers = 13*4bytes = 52 bytes (only valid for MFC files)
		# also 4 bytes for header
		$estimated_training_data += (($size - 4) / 52);
		if (! $size) {
		    $ret_value = -4;
		    $status = 'FAILED';
		    LogWarning ("Error in '$ST::CFG_LISTOFFILES', the feature file '$ST::CFG_FEATFILES_DIR/$file.$ST::CFG_FEATFILE_EXTENSION' does not exist, or is empty");
		}
		push @uttids, $file;
	    } else {
		$status = 'FAILED';
		$ret_value = -5;
		LogWarning ("Error in '$ST::CFG_LISTOFFILES'. Can not parse the line '$ctl_line'\n");
	    }
	}
    }
    LogStatus($status);
    
    $number_ctl_lines = $#ctl_lines + 1;

    
    # 4) Check number of lines in the transcript and in ctl - they should be the same";
    Log ("Phase 4: Checking number of lines in the transcript file should match lines in fileids file");
    open TRN,"$ST::CFG_TRANSCRIPTFILE" or die "Can not open transcript file ($ST::CFG_TRANSCRIPTFILE)";
    my $number_transcript_lines = 0;
    my $trnline;
    while (<TRN>) {
	$number_transcript_lines++;
	$trnline = $_;
    }
    close TRN;
    
    # 4.5) Check that there is a newline at the end of the file (yes,
    # this causes problems if it's not there, sorry...)
    if ($trnline !~ /\n$/) {
	$status = 'FAILED';
	LogWarning("Transcript file missing a newline at end of file");
	LogStatus($status);
    }
    else {
	$status = ($number_ctl_lines == $number_transcript_lines) ? 'passed' : 'FAILED';
	LogStatus($status);
    }

    # 5) Should already have estimates on the total training time, 
    Log ("Phase 5: Determine amount of training data, see if n_tied_states seems reasonable.");
    $status = 'passed';
    if ($estimated_training_data) {
	my $total_training_hours = ($estimated_training_data / 3600)/100;
	Log("Estimated Total Hours Training: $total_training_hours", 'result');
	my $estimated_n_tied_states = 1000;
	
	if ($total_training_hours < 0.5 and $ST::CFG_CD_TRAIN eq 'yes') {
	    $status = 'FAILED';
	    $ret_value = -5;
	    LogError("Not enough data for the training, we can only train CI models (set CFG_CD_TRAIN to \"no\")");
	} else {
	    if ($total_training_hours < 10) {
		$status = 'WARNING';
		Log("This is a small amount of data, no comment at this time", 'result');
	    } else {
		if ($total_training_hours < 100) {
		    $status = 'WARNING';
		    $estimated_n_tied_states = 3000 if ($ST::CFG_HMM_TYPE eq '.cont.'); # Likely bogus 
		    $estimated_n_tied_states = 4000 if ($ST::CFG_HMM_TYPE eq '.semi.'); # 
		    Log("Rule of thumb suggests $estimated_n_tied_states, however there is no correct answer", 'result');
		} else {
		    $estimated_n_tied_states = 8000;
		    $status = 'passed';
		    Log("100+ hours of training data is goodly amount of data.", 'result');
		    Log("Rule of thumb suggests $estimated_n_tied_states for 100 hours, you can adjust accordingly.", 'result');
		}
	    }
	}
    }
    LogStatus($status);
    @ctl_lines = ();
}

my %transcript_phonelist_hash = ();

# Verify that all transcription words are in the dictionary, and all
# phones are covered
{
    Log("Phase 6: Checking that all the words in the transcript are in the dictionary");
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
    
    open TRN,"$ST::CFG_TRANSCRIPTFILE" or die "Can not open the transcript file ($ST::CFG_TRANSCRIPTFILE)"; 
    
    my $status = 'passed';
    my $i = 0;
    my %seen = ();
    if ($ST::CFG_G2P_MODEL eq 'yes') {
	open TRAIN_OOV, ">", "$ST::CFG_TRANSCRIPTFILE.oov";
    }
    
    while (<TRN>) {
	# Some sanity checking:
	#
	# - uttids should not contain spaces
	# - uttid should be separated from text by a space
	# - uttid should match
	chomp;
	my ($text, $uttid) = m/^(.*)\s*\(([^()]+)\)\s*$/;
	unless (defined($text) and defined($uttid)) {
	    LogWarning("Bad line in transcript:\n$_");
	    $status = 'FAILED';
	    $ret_value = -6;
	}
	if ($uttid =~ m/\s/) {
	    LogWarning("Utterance ID '$uttid' on line $. contains whitespace");
	    $status = 'FAILED';
	    $ret_value = -6;
	}
	if (/\S+\([^()]+\)$/) {
	    LogWarning("No whitespace between text and utterance ID on line $.");
	    $status = 'FAILED';
	    $ret_value = -6;
	}
	if ($uttid ne $uttids[$i] and $uttid ne basename($uttids[$i])) {
	    LogWarning("Utterance ID mismatch on line $.: $uttids[$i] vs $uttid");
	    $status = 'FAILED';
	    $ret_value = -6;
	}
	if ($text) {
	    my @words = split /\s+/,$text;
	    for my $word (@words) {
		if ($ST::CFG_G2P_MODEL ne 'yes' and !($d{$word}) and ($word =~ m/\S+/)) {
		    LogWarning ("This word: $word was in the transcript file, but is not in the dictionary ($text). Do cases match?");
		    $status = 'FAILED';
		    $ret_value = -5;
		} elsif ($ST::CFG_G2P_MODEL eq 'yes'  and !($d{$word}) and ($word =~ m/\S+/)) {
		    $seen{$word}++;
		    if ($seen{$word} == 1) {
    			print TRAIN_OOV "$word\n";
    		    }
		} else {
		    my @phones = ($d{$word} =~ m/(\S+)/g);
		    for my $phone (@phones) {
		        $transcript_phonelist_hash{$phone} = 1;
		    }
		}
	    }
	}
    }
    continue {
	++$i;
    }
    
    close TRN;

    if ($ST::CFG_G2P_MODEL eq 'yes') {
	close TRAIN_OOV;
    }

    LogStatus($status);
}

{
    Log("Phase 7: Checking that all the phones in the transcript are in the phonelist, and all phones in the phonelist appear at least once");
    my $status = 'passed';

    for my $phone (sort keys %phonelist_hash) {
      if (!defined $transcript_phonelist_hash{$phone}) {
	    LogWarning ("This phone ($phone) occurs in the phonelist ($ST::CFG_RAWPHONEFILE), but not in any word in the transcription ($ST::CFG_TRANSCRIPTFILE)");
	    $status = 'FAILED';
      }
    }
    LogStatus($status);
  }

mkdir ($ST::CFG_LOG_DIR,0755);
mkdir ($ST::CFG_BWACCUM_DIR,0755);

exit ($ret_value);

# General idea for senone: 
#   10 hours = 3000 cont. 4000 semi. 
#  100 hours = 8000 (cont and semi) 
# Rate of increase between the two is very small.

