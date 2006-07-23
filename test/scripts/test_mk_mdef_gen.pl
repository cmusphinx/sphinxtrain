#!/usr/local/bin/perl

use strict;
require './scripts/testlib.pl';

my $bindir="../bin.i686-pc-linux-gnu/";
my $resdir="res/";
my $exec_resdir="mk_mdef_gen";
my $bin="$bindir$exec_resdir";

test_help($bindir,$exec_resdir);
test_example($bindir,$exec_resdir);

