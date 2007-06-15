#!/usr/bin/perl -w
use strict;

# USAGE: make_phoneset.pl etc/EXPT_NAME.dic etc/EXPT_NAME.filler > etc/EXPT_NAME.phone

my %phonetab;
while (<>) {
    next if /^##/;
    next if /^$/;

    my ($word, @phones) = split;
    $phonetab{$_}++ foreach @phones;
}

foreach my $p (sort keys %phonetab) {
    print "$p\n";
}
