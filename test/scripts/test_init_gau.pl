#!/usr/local/bin/perl

use strict;
use File::Copy;
require './scripts/testlib.pl';

chomp(my $host=`../config.guess | xargs ../config.sub`);
my $bindir="../bin.$host/";
my $resdir="res/";
my $exec_resdir="init_gau";
my $bin="$bindir$exec_resdir";
my $bin_printp="${bindir}printp";

my $cepdir="./res/feat/rm";
my $gd_cnt="./gauden_counts";
my $gd_out="./gd_cnt.out";
my $gd_match="./printp/test_printp_gaucnt.1gau";

my $cmd="$bin ";
$cmd .= "-part 1 -npart 1 ";
$cmd .= "-cepdir $cepdir ";
$cmd .= "-cepext mfc -accumdir ./ ";
$cmd .= "-agc none -cmn current ";
$cmd .= "-varnorm no -feat 1s_c_d_dd -ceplen 13 ";

test_help($bindir,$exec_resdir);
test_example($bindir,$exec_resdir);

my @params=(1,3,6,12,25,50,100) ;
foreach my $i (@params)
{
    my $ctlfn="./res/feat/rm/rm1_train.fileids.$i";
    my $matchfn="./$exec_resdir/test_gauden_counts.$i.out";
    my $tmpcmd=$cmd;
    $tmpcmd .= "-ctlfn $ctlfn ";

    test_this("$tmpcmd",$exec_resdir,"DRY RUN TEST for $i utterances");
    test_this("$bin_printp -gaucntfn $gd_cnt > $gd_out ",$exec_resdir,"printp gau count for $i utterances");
    if($i==100){
	compare_these_two($gd_out,$gd_match,$exec_resdir,"Gau count generation for 100 utterance. Check duplicates with printp");
    }
    compare_these_two($gd_out,$matchfn,$exec_resdir,"Gau count generation for $i utterances");
    copy($gd_out,"./$exec_resdir/$gd_cnt.$i.out11");
}

unlink($gd_cnt,$gd_out);
