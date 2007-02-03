#!/usr/local/bin/perl

use strict;
use File::Copy;
require './scripts/testlib.pl';

chomp(my $host=`../config.guess | xargs ../config.sub`);
my $bindir="../bin.$host/";
my $resdir="res/";
my $exec_resdir="norm";
my $bin="$bindir$exec_resdir";
my $bin_printp="${bindir}printp";
my $out="globalmean.out";

test_help($bindir,$exec_resdir);
test_example($bindir,$exec_resdir);

my @params=(1,3,6,12,25,50,100) ;
foreach my $i (@params)
{
    copy("init_gau/gauden_counts.$i","./gauden_counts");
    test_this("$bin -accumdir . -meanfn ./globalmean",$exec_resdir,"Dry run accumulate the global mean from init_gau $i");
#    copy("./globalmean","./norm/globalmean.$i");
    test_this("${bin_printp} -gaufn ./globalmean > $out ",$exec_resdir,"Print output of norm");
    compare_these_two($out,"./norm/test_globalmean.$i.txt",$exec_resdir,"norm, generate global mean. ");
    unlink("./gauden_counts",$out,"./globalmean");
}
