#!/usr/bin/perl -w
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


#use strict;

require "/sphx_train/swb1/scripts_pl/sphinx_train.cfg";


my ($status,@ctl_lines,$ctl_line,$file,$start,$end,$number_ctl_lines,$number_lines_transcript);


# Check to make sure .ctl file is roughly of correct format
# 1.) Check that each utterance specified in the .ctl file has a positive length
#     Verify that the files listed are available and are not of size 0
# 2.) Check number of lines in the transcript and in ctl - they should be the same\n";


open CTL,"$CFG_LISTOFFILES" or die "Can not open listoffiles ($CFG_LISTOFFILES)";
@ctl_lines = <CTL>;		# We are going to iterate over this several times
close CTL;

print "Phase 1: Check format (reasonable); utterance lengths are positive: file exist\n";
# 1.) Check that each utterance specified in the .ctl file has a positive length
#     Verify that the files listed are available and are not of size 0
$status = 'passed';
for $ctl_line (@ctl_lines) {
    # Accept: filename int int possible_comment
    ($file,$start,$end) = map /(.+)\s(\d+)\s(\d+).*/,$ctl_line;
    if ((defined $start)&& (defined $file)) {
	if ($end <= $start) {
	    warn "Utterance length is <= 0: $start -> $end ($ctl_line)";
	    $status = 'FAILED';
	}
	if (! -s "$CFG_FEATFILES_DIR/$file.$CFG_FEATFILE_EXTENSION") {
	    $status = 'FAILED';
	    warn "This file, $CFG_FEATFILES_DIR/$file.$CFG_FEATFILE_EXTENSION, does not exist";
	}
    } else {
	$status = 'FAILED';
	warn "This line does not parse correctly:\n$ctl_line";
    }
}


print "\t$status\n";

$number_ctl_lines = $#ctl_lines + 1;
@ctl_lines = ();

# 2) Check number of lines in the transcript and in ctl - they should be the same\n";
print "Phase 2: Checking number of lines in the transcript and in ctl - they should be the same\n";
open TRN,"$CFG_TRANSCRIPTFILE" or die "Can not open Transcript file ($CFG_TRANSCRIPTFILE)";
$number_transcript_lines = 0;
while (<TRN>) {
    $number_transcript_lines++;
}
close TRN;

$status = ($number_ctl_lines == $number_transcript_lines) ? 'passed' : 'FAILED';
print "\t$status\n";

exit (0) if ($status eq 'passed');
exit (-1);
