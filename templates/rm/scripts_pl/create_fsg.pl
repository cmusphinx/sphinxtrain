#!/usr/bin/perl -w
use strict;

my $hdrdone;
my %bigrams;
my $word;
while (<>) {
    $hdrdone = 1 if s/\*\///;
    next unless $hdrdone;
    next if /^\s*$/;

    tr/+/\'/;
    if (/^>(\S+)/) {
	$word = $1;
	$word = '<s>' if $word eq 'SENTENCE-END';
    }
    else {
	s/^\s+//;
	s/\s+$//;
	$_ = '</s>' if $_ eq 'SENTENCE-END';
	push @{$bigrams{$word}}, $_;
    }
}

my ($nstate, $endstate);

# Each word plus <s> and </s> gets a state
$nstate = keys(%bigrams) + 1;
$endstate = $nstate-1;

print "FSG_BEGIN wordpair\n";
print "NUM_STATES $nstate\n";
print "START_STATE 0\n";
print "FINAL_STATE $endstate\n";

# Now output transitions
my %stateid;
$stateid{'<s>'} = 0;
$stateid{'</s>'} = $endstate;
my $lastid = 0;
my $dest = $bigrams{'<s>'};
my $tprob = 1/@$dest;
foreach my $word (@$dest) {
    $stateid{$word} = ++$lastid;
    print "TRANSITION 0 $stateid{$word} $tprob $word\n";
}

foreach my $word1 (keys %bigrams) {
    next if $word1 eq '<s>';
    $stateid{$word1} = ++$lastid unless exists $stateid{$word1};
    $dest = $bigrams{$word1};
    $tprob = 1/@$dest;
    foreach my $word2 (@$dest) {
	$stateid{$word2} = ++$lastid unless exists $stateid{$word2};
	if ($word2 eq '</s>') {
	    print "TRANSITION $stateid{$word1} $stateid{$word2} $tprob $word2\n";
	}
	else {
	    print "TRANSITION $stateid{$word1} $stateid{$word2} $tprob $word2\n";
	}
    }
}
print "FSG_END\n";

1;
__END__

=head1 NAME

create_fsg.pl - Build an FSG from RM1 word-pair grammar

=head1 SYNOPSIS

create_fsg.pl < wp_gram.txt > rm1.fsg

=head1 DESCRIPTION

This script reads the RM1 word-pair grammar format and generates a
finite state grammar suitable for decoding with
Sphinx2/PocketSphinx/Sphinx3.

=head1 AUTHOR

=cut
