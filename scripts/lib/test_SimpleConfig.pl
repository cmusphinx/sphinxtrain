#!/usr/bin/perl -w
use strict;
use SimpleConfig;
use Data::Dumper;

my %conf = parse_config('test.txt');
print Dumper(\%conf);
