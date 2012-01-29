#!/usr/local/bin/perl

use strict;
require './scripts/testlib.pl';

my $bindir="../scripts/";
my $resdir="res/";
my $dir="make_topology";
my $bin ="perl ${bindir}maketopology.pl";
my $tmpfn="tmp.topo";
my $cfg="-cfg ../etc/sphinx_train.cfg";

#maketopology is a perl script so there is no example and help here. 
#These two have the same topology. 
test_this("$bin $cfg $tmpfn 1 yes","Dry run make_topology.pl 1 state with skip");
compare_these_two("$tmpfn","${dir}/1_sk.topo","make_topology.pl 1 state with skip");
unlink("$tmpfn");
test_this("$bin $cfg $tmpfn 1 no","Dry run make_topology.pl 1 state without skip");
compare_these_two("$tmpfn","${dir}/1_sk.topo","make_topology.pl 1 state wo skip");
unlink("$tmpfn");

my $i;
for($i=2;$i<=7;$i++){
    test_this("$bin $cfg $tmpfn $i yes","Dry run make_topology.pl 1 state with skip");
    compare_these_two("$tmpfn","${dir}/${i}_sk.topo","make_topology.pl $i state with skip");
    unlink("$tmpfn");
    test_this("$bin $cfg $tmpfn $i no","Dry run make_topology.pl 1 state without skip");
    compare_these_two("$tmpfn","${dir}/${i}_nosk.topo","make_topology.pl $i state wo skip");
    unlink("$tmpfn");
}
