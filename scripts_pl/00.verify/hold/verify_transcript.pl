#!/usr/opt/PERL5004/bin/perl -w

require "/sphx_train/swb1/scripts_pl/sphinx_train.cfg";


print "Phase 1: Checking to see if all the words in the transcript file occur in the dictionary\n";

open DICT,"$CFG_DICTIONARY" or die "Can not open the dictionary ($CFG_DICTIONARY)";
@dict = <DICT>;
close DICT;
print "\tWords in dictionary: $#dict\n";

for (@dict) {		# Create a hash of the dict entries
    /(\S+)\s+(.*)$/;
    $d{lc($1)} = $2;
}

open DICT,"$CFG_FILLERDICT" or die "Can not open filler dict ($CFG_FILLERDICT)\n";
@fill_dict = <DICT>;
close DICT;
print "\tWords in filler dictionary: $#fill_dict\n";


for (@fill_dict) {		# Create a hash of the dict entries
    /(\S+)\s+(.*)$/;
    $d{lc($1)} = $2;
}


@dict = undef;			# not needed
@fill_dict = undef;		# not needed


open TRN,"$CFG_TRANSCRIPTFILE" or die "Can not open the transcript file ($CFG_TRANSCRIPTFILE)"; 

$status = 'passed';
while (<TRN>) {
    ($text,$comment) = map /(.*)\s*\((.*)\)$/,$_;
    if ($text) {
	@words = split / /,$text;
	for $word (@words) {
	    if (! $d{lc($word)} && ($word =~ m/\S+/)) {
		$missing{$word} = $text; # Keep track of which ones are missing
		warn "This word: $_ is missing ($text)";
		$status = 'FAILED';
	    }
	}
    }
}
close TRN;

print "\t$status\n";

exit (0) if (($status eq 'passed'));
exit (-1);




