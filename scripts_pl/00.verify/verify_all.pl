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
## 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
##    endorse or promote products derived from this software without
##    prior written permission. To obtain permission, contact 
##    sphinx@cs.cmu.edu.
##
## 4. Products derived from this software may not be called "Sphinx"
##    nor may "Sphinx" appear in their names without prior written
##    permission of Carnegie Mellon University. To obtain permission,
##    contact sphinx@cs.cmu.edu.
##
## 5. Redistributions of any form whatsoever must retain the following
##    acknowledgment:
##    "This product includes software developed by Carnegie
##    Mellon University (http://www.speech.cs.cmu.edu/)."
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
##

my $index = 0;
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    $index = 2;
} else {
    $cfg_file = "etc/sphinx_train.cfg";
}

if (! -s "$cfg_file") {
    print ("unable to find default configuration file, use -cfg file.cfg or create etc/sphinx_train.cfg for default\n");
    exit -3;
}

require $cfg_file;

$ret_value = 0;

&ST_Log ("MODULE: 00 $0\n");

# PHASE 1: Check to see if the phones in the dictionary are listed in the phonelist file
# PHASE 2: Check to make sure there are not duplicate entries in the dictionary
{
    open DICT,"$CFG_DICTIONARY" or die "Can not open the dictionary ($CFG_DICTIONARY)";

    %dict_phone_hash = ();
    %dict_hash = ();

    &ST_Log ("\tPhase 1: DICT - Checking to see if the dict and filler dict agrees with the phonelist file\n");
    # This is rather ugly, but it's late and I'm just trying to get the pieces together
    # Clean it up later

    # Read the dictionary and stick phones into dict_phone_hash
    $counter =0;
    while (<DICT>) {
	if (/^(\S+)\s(.*)$/) {
	    $dict_hash{lc($1)}++;
	    $tmp = $2;
	    $tmp =~ s/\t/ /g;
	    $tmp =~ s/  / /g;
	    @phones = split / /,$tmp;
	    for $phone (@phones) {
		$phone =~ s/  / /g;
		$dict_phone_hash{uc($phone)}++ if $phone;
	    }
	}
	$counter++;
    }
    close DICT;

    open DICT,"$CFG_FILLERDICT" or die "Can not open filler dict ($CFG_FILLERDICT)\n";
    while (<DICT>) {
	if (/^(\S+)\s(.*)$/) {
	    $dict_hash{lc($1)}++;
	    $tmp = $2;
	    $tmp =~ s/\t/ /g;
	    $tmp =~ s/  / /g;
	    @phones = split / /,$tmp;
	    for $phone (@phones) {
		$phone =~ s/  / /g;
		$dict_phone_hash{uc($phone)}++ if $phone;
	    }
	}
	$counter++;
    }
    close DICT;

    
    # Read the phonelist and stick phones into phonelist_hash
    open PHONE,"$CFG_RAWPHONEFILE";
    while (<PHONE>) {
	chomp;
	$phonelist_hash{uc($_)}++;
    }
    close PHONE;
    
    @keys = keys %dict_phone_hash;
    &ST_Log ("\t\tFound $counter words using $#keys phones\n");
    
    $status = 'passed';
    for $key (sort (keys %dict_phone_hash)){
	if (! defined($phonelist_hash{$key})) {
	    $status = 'FAILED';
	    $ord = ord($key);
	    $ret_value = -1;
	    system ("cp $CFG_GIF_DIR/red-ball.gif $CFG_BASE_DIR/.00.1.state.gif");
	    &ST_LogWarning ("This phone ($key -> $ord) occurs in the dictionary ($CFG_DICTIONARY), but not in the phonelist ($CFG_RAWPHONEFILE\n");
	}
    }

    &ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> $status </font>\n") if ($status eq 'passed');
    &ST_HTML_Print ("\t\t<font color=\"$CFG_ERROR_COLOR\"> $status </font>\n") if ($status eq 'FAILED');
#    &ST_Log("\t\t$status\n");

    &ST_Log("\tPhase 2: DICT - Checking to make sure there are not duplicate entries in the dictionary\n");
    $duplicate_status = 'passed';
    for $key (keys %dict_hash) {
	if ($dict_hash{$key} > 1) {
	    $ret_value = -2;
	    $duplicate_status = 'FAILED';
	    system ("cp $CFG_GIF_DIR/red-ball.gif $CFG_BASE_DIR/.00.2.state.gif");
	    &ST_LogWarning("This word ($key) has duplicate entries in ($CFG_DICTIONARY)\n");
	}
    }
#    &ST_Log ("\t\t$duplicate_status\n");
    &ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> $duplicate_status </font>\n") if($duplicate_status eq 'passed');
    &ST_HTML_Print ("\t\t<font color=\"$CFG_ERROR_COLOR\"> $duplicate_status </font>\n") if($duplicate_status eq 'FAILED');
}


# Check to make sure .ctl file is roughly of correct format
# 3.) Check that each utterance specified in the .ctl file has a positive length
#     Verify that the files listed are available and are not of size 0
# 4.) Check number of lines in the transcript and in ctl - they should be the same\n";
{
    my ($status,@ctl_lines,$ctl_line,$file,$start,$end,$number_ctl_lines,$number_lines_transcript);
    
    open CTL,"$CFG_LISTOFFILES" or die "Can not open listoffiles ($CFG_LISTOFFILES)";
    @ctl_lines = <CTL>;		# We are going to iterate over this several times
    close CTL;

    # 3.) Check that each utterance specified in the .ctl file has a positive length
    #     Verify that the files listed are available and are not of size 0

    &ST_Log("\tPhase 3: CTL - Check general format; utterance length (must be positive); files exist\n");
    $status = 'passed';
    $estimated_training_data = 0;
    for $ctl_line (@ctl_lines) {
	# Accept: filename int int possible_comment
	if ($ctl_line =~ m/(.+)\s(\d+)\s(\d+).*/) {
	    $file = $1;
	    $start = $2;
	    $end = $3;
	    if ((defined $start)&& (defined $file)) {
		if ($end <= $start) {
		    warn "Utterance length is <= 0: $start -> $end ($ctl_line)";
		    $status = 'FAILED';
		    $ret_value = -3;
		}

		if (! -s "$CFG_FEATFILES_DIR/$file.$CFG_FEATFILE_EXTENSION") {
		    $ret_value = -4;
		    $status = 'FAILED';
		    &ST_LogWarning ("This file, $CFG_FEATFILES_DIR/$file.$CFG_FEATFILE_EXTENSION, does not exist\n");
		}
	    }
	} else {
	    # Accepts only the file name and possible comment on line by itself..no start/send markers
	    if ($ctl_line =~ m/([^\s]+)\s.*/) {
		$file = $1;
		$size = -s "$CFG_FEATFILES_DIR/$file.$CFG_FEATFILE_EXTENSION";
		# 1 frame = 13 floating point numbers = 13*4bytes = 52 bytes (only valid for MFC files)
		$estimated_training_data += ($size / 52) if (lc($CFG_FEATFILE_EXTENSION) eq 'mfc');
		if (! $size) {
		    $ret_value = -4;
		    $status = 'FAILED';
		    &ST_LogWarning ("CTL file, $CFG_FEATFILES_DIR/$file.$CFG_FEATFILE_EXTENSION, does not exist\n");
		}
	    } else {
		$status = 'FAILED';
		$ret_value = -5;
		&ST_LogWarning ("CTL line does not parse correctly:\n$ctl_line\n");
	    }
	}
    }

#    &ST_Log ("\t\t$status\n");
    &ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> $status </font>\n") if($status eq 'passed');
    &ST_HTML_Print ("\t\t<font color=\"$CFG_ERROR_COLOR\"> $status </font>\n") if($status eq 'FAILED');
    
    $number_ctl_lines = $#ctl_lines + 1;

    
    # 4) Check number of lines in the transcript and in ctl - they should be the same\n";
    &ST_Log ("\tPhase 4: CTL - Checking number of lines in the transcript should match lines in control file\n");
    open TRN,"$CFG_TRANSCRIPTFILE" or die "Can not open Transcript file ($CFG_TRANSCRIPTFILE)";
    $number_transcript_lines = 0;
    while (<TRN>) {
	$number_transcript_lines++;
    }
    close TRN;
    
    $status = ($number_ctl_lines == $number_transcript_lines) ? 'passed' : 'FAILED';
    &ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> $status </font>\n") if($status eq 'passed');
    &ST_HTML_Print ("\t\t<font color=\"$CFG_ERROR_COLOR\"> $status </font>\n") if($status eq 'FAILED');


    # 4a) Should already have estimates on the total training time, 

    &ST_Log ("\tPhase 4a: CTL - Determine amount of training data, see if n_tied_states seems reasonable.\n");
    $status = 'passed';
    $total_training_data = 0;
    for $ctl_line (@ctl_lines) {
	# Accept: filename int int possible_comment
	#($file,$start,$end) = map /(.+)\s(\d+)\s(\d+).*/,$ctl_line;
	# start and end time specify start and end frames
	if ($ctl_line =~ m/(.+)\s(\d+)\s(\d+).*/) {
	    $file = $1;
	    $start = $2;
	    $end = $3;
	    $total_training_data += ($end - $start) unless (($end - $start) < 0);
	} 
    }
    $total_training_data = $estimated_training_data if ($total_training_data == 0) ;

    if ($total_training_data) {
	$total_training_hours = ($total_training_data / 3600)/100;
	&ST_Log("\t\tTotal Hours Training: $total_training_hours\n");
	$estimated_n_tied_states = 1000;
	if ($total_training_hours < 10) {
	    $status = WARNING;
	    &ST_Log("\t\tThis is a small amount of data, no comment at this time\n");
	} else {
	    if ($total_training_hours < 100) {
		$status = WARNING;
		$estimated_n_tied_states = 3000 if ($CFG_HMM_TYPE eq '.cont.'); # Likely bogus 
		$estimated_n_tied_states = 4000 if ($CFG_HMM_TYPE eq '.semi.'); # 
		&ST_Log("\t\tRule of thumb suggests $estimated_n_tied_states, however there is no correct answer\n");
	    } else {
		$estimated_n_tied_states = 8000;
		$status = 'passed';
		&ST_Log("\t\t100+ hours of training data is goodly amount of data.\n");
		&ST_Log("\t\tRule of thumb suggests $estimated_n_tied_states for 100 hours, you can adjust accordingly.\n");
	    }
	}
    }
    &ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> $status </font>\n") if($status eq 'passed');
    &ST_HTML_Print ("\t\t<font color=\"$CFG_ERROR_COLOR\"> $status </font>\n") if($status eq 'FAILED');
    &ST_HTML_Print ("\t\t<font color=\"$CFG_WARNING_COLOR\"> $status </font>\n") if($status eq 'WARNING');
#    &ST_Log("\t\t$status\n");
    @ctl_lines = ();
}



{
    &ST_Log("\tPhase 5: TRANSCRIPT - Checking that all the words in the transcript are in the dictionary\n");
    open DICT,"$CFG_DICTIONARY" or die "Can not open the dictionary ($CFG_DICTIONARY)";
    @dict = <DICT>;
    close DICT;
    &ST_Log("\t\tWords in dictionary: $#dict\n");
    
    for (@dict) {		# Create a hash of the dict entries
	/(\S+)\s+(.*)$/;
	$d{lc($1)} = $2;
    }
    
    open DICT,"$CFG_FILLERDICT" or die "Can not open filler dict ($CFG_FILLERDICT)\n";
    @fill_dict = <DICT>;
    close DICT;
    &ST_Log ("\t\tWords in filler dictionary: $#fill_dict\n");
    
    for (@fill_dict) {		# Create a hash of the dict entries
	/(\S+)\s+(.*)$/;
	$d{lc($1)} = $2;
    }
    
    @dict = undef;			# not needed
    @fill_dict = undef;		# not needed
    
    open TRN,"$CFG_TRANSCRIPTFILE" or die "Can not open the transcript file ($CFG_TRANSCRIPTFILE)"; 
    
    $status = 'passed';
    while (<TRN>) {
	($text) = map /(.*)\s*\(.*\)$/,$_;
	if ($text) {
	    @words = split / /,$text;
	    for $word (@words) {
		if (! $d{lc($word)} && ($word =~ m/\S+/)) {
		    &ST_LogWarning ("This word: $_ was in the transcript file, but is not in the dictionary ($text)\n");
		    $status = 'FAILED';
		    $ret_value = -5;
		}
	    }
	}
    }
    close TRN;

    &ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> $status </font>\n") if($status eq 'passed');
    &ST_HTML_Print ("\t\t<font color=\"$CFG_ERROR_COLOR\"> $status </font>\n") if($status eq 'FAILED');
    &ST_HTML_Print ("\t\t<font color=\"$CFG_WARNING_COLOR\"> $status </font>\n") if($status eq 'WARNING');

}

    
exit ($ret_value);

# General idea for senone: 
#   10 hours = 3000 cont. 4000 semi. 
#  100 hours = 8000 (cont and semi) 
# Rate of increase between the two is very small.

