#!/usr/local/bin/perl

use strict;
require './scripts/testlib.pl';


my $bindir="../bin.i686-pc-linux-gnu/";
my $resdir="res/";
my $exec_resdir="dict2tri";
my $bin="$bindir$exec_resdir";

my $args = "-dictfn $resdir/communicator.dic.cmu ";
$args .= "-basephnfn $resdir/communicator.phone.cmu";
my $w_output = "dict2tri.triphones";
my $wo_output = "dict2tri.no_triphones";
my $w_triphones = "-btwtri yes";
my $wo_triphones = "-btwtri no";
my $match_w_output = "${exec_resdir}/test.dict2tri.internaltriphones";
my $match_wo_output = "${exec_resdir}/test.dict2tri.no_internaltriphones";

test_help($bindir,$exec_resdir);
test_example($bindir,$exec_resdir);

#Testing with triphone computed
test_this("$bin $args $w_triphones > $w_output",$exec_resdir,"DRY RUN w/int triphones TEST");
compare_these_two($w_output,$match_w_output,,$exec_resdir,"COMPUTE W INTERNAL TRIPHONES");

#Testing wo triphone computed
test_this("$bin $args $wo_triphones > $wo_output",$exec_resdir,"DRY RUN wo/int triphones TEST");
compare_these_two($wo_output,$match_wo_output,$exec_resdir,"COMPUTE WO INTERNAL TRIPHONES");







