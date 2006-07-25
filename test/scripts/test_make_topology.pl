#!/usr/local/bin/perl

use strict;
require './scripts/testlib.pl';

my $bindir="../scripts_pl/";
my $resdir="res/";
my $dir="make_topology";
my $bin ="perl ${bindir}maketopology.pl";
my $tmpfn="tmp.topo";

#These two have the same topology. 
test_this("$bin 1 yes > $tmpfn","Dry run make_topology.pl 1 state with skip");
compare_these_two("$tmpfn","${dir}/1_sk.topo","make_topology.pl 1 state with skip");
unlink("$tmpfn");
test_this("$bin 1  no > $tmpfn","Dry run make_topology.pl 1 state without skip");
compare_these_two("$tmpfn","${dir}/1_sk.topo","make_topology.pl 1 state wo skip");
unlink("$tmpfn");

my $i;
for($i=2;$i<=7;$i++){
    test_this("$bin $i yes > $tmpfn","Dry run make_topology.pl 1 state with skip");
    compare_these_two("$tmpfn","${dir}/${i}_sk.topo","make_topology.pl $i state with skip");
    unlink("$tmpfn");
    test_this("$bin $i no > $tmpfn","Dry run make_topology.pl 1 state without skip");
    compare_these_two("$tmpfn","${dir}/${i}_nosk.topo","make_topology.pl $i state wo skip");
    unlink("$tmpfn");
}
