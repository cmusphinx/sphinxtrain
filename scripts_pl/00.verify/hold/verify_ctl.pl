#!/usr/opt/PERL5004/bin/perl -w

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
