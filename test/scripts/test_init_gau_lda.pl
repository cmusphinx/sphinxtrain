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

my $lda="./res/hmm/RM.lda";
my $cepdir="./res/feat/rm";
my $gd_cnt="./gauden_counts";
my $gd_out="./gd_cnt.out";

my $cmd="$bin ";
$cmd .= "-part 1 -npart 1 ";
$cmd .= "-cepdir $cepdir ";
$cmd .= "-cepext mfc -accumdir ./ ";
$cmd .= "-agc none -cmn current ";
$cmd .= "-lda $lda -ldadim 29 ";
$cmd .= "-varnorm no -feat 1s_c_d_dd -ceplen 13 ";

test_help($bindir,$exec_resdir);
test_example($bindir,$exec_resdir);

my $ctlfn="./res/feat/rm/rm1_train.fileids.100";
my $matchfn="./$exec_resdir/test_gauden_counts_lda.100.out";
$cmd .= "-ctlfn $ctlfn ";

test_this("$cmd",$exec_resdir,"DRY RUN TEST for 100 utterances");
test_this("$bin_printp -gaucntfn $gd_cnt > $gd_out ",$exec_resdir,"printp gau count for 100 utterances");
compare_these_two($gd_out,$matchfn,$exec_resdir,"LDA Gau count generation for 100 utterances");
#copy($gd_cnt,"./$exec_resdir/$gd_cnt"."_lda.100");

unlink($gd_cnt,$gd_out);
