#!/usr/local/bin/perl

use strict;
use File::Copy;
require './scripts/testlib.pl';

chomp(my $host=`../config.guess | xargs ../config.sub`);
my $bindir="../bin.$host/";
my $resdir="res/";
my $exec_resdir="cp_parm";
my $bin="$bindir$exec_resdir";
my $cpop="-cpopsfn ./cp_parm/rm1.cpmeanvar";
my $bin_printp="${bindir}printp";
my $out="mean.out";

test_help($bindir,$exec_resdir);
test_example($bindir,$exec_resdir);

test_this("$bin $cpop -igaufn ./norm/globalmean.1 -ogaufn means.1 -ncbout 134",$exec_resdir," wrong ncb_out expected", 65280);

my @params=(1,3,6,12,25,50,100) ;
foreach my $i (@params)
{
    my $matchcase="./cp_parm/test_means.$i.txt";
    test_this("$bin $cpop -igaufn ./norm/globalmean.$i -ogaufn means.$i -ncbout 135",$exec_resdir,"Copy parameters trained by $i samples", 0);
#    copy("./means.$i","./cp_parm/means.$i");
    test_this("${bin_printp} -gaufn ./means.$i > $out ",$exec_resdir,"Print output of cp_parm");
    compare_these_two($out,$matchcase,$exec_resdir,"cp_parm, copying ");
    unlink("./means.$i");
    unlink("$out");
}

