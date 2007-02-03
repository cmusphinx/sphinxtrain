#!/usr/local/bin/perl

use strict;
use File::Copy;
require './scripts/testlib.pl';

chomp(my $host=`../config.guess | xargs ../config.sub`);
my $bindir="../bin.$host/";
my $resdir="res/";
my $exec_resdir="mk_flat";
my $printp_resdir="printp";
my $bin="$bindir$exec_resdir";
my $bin_printp="${bindir}${printp_resdir}";
my $mdefopt="-moddeffn ";
my $topoopt="-topo ";
my $mixwopt="-mixwfn ";
my $tmatopt="-tmatfn ";
my $matchdir=$exec_resdir;
my $mkmgdir="mk_mdef_gen";
my $mktopodir="make_topology";
my $mixw_bin="mixw";
my $tmat_bin="tmat";
my $mixw_txt="mixw.txt";
my $tmat_txt="tmat.txt";
my $mixw_txt_match="mixw.txt.match";
my $tmat_txt_match="tmat.txt.match";

test_help($bindir,$exec_resdir);
test_example($bindir,$exec_resdir);

#Only 1,3,5, 1 is the special case, 3 and 5 is the common 
#used case.

my $tmp="$bin ";
$tmp .= "$mdefopt $mkmgdir/3st.ci.mdef ";
$tmp .= "$topoopt $mktopodir/3_sk.topo ";

test_this("$tmp  $mixwopt $mixw_bin -nstream 1 -ndensity 1",$exec_resdir,"dry run 3 state, stream 1, density 1 just -mixwfn ");
test_this("$tmp  $tmatopt $tmat_bin -nstream 1 -ndensity 1",$exec_resdir,"dry run 3 state, stream 1, density 1 just -tmatfn ");
$tmp="";

my $i;
for($i=1;$i<=5;$i+=2)
{
    #Test on nstream,n_density=(1,1) (4,256) and (4,1024).
    #
    #Note1: That represents how sphinx 3 cont, sphinx 2 
    #semi work. To save space. (4,256) (4,1024) just dry run. 
    #
    #Note2: This makes used of the match case prepared in mk_mdef_gen
    #and make_topology
    
    my $tmp="$bin ";
    $tmp .= "$mdefopt $mkmgdir/${i}st.ci.mdef ";
    $tmp .= "$topoopt $mktopodir/${i}_sk.topo ";
    $tmp .= "$mixwopt $mixw_bin ";
    $tmp .= "$tmatopt $tmat_bin ";
    
    my $cmd = $tmp ;
    $cmd .= "-nstream 1 -ndensity 1";
    test_this($cmd,$exec_resdir,"dry run $i state, stream 1, density 1 ");
    test_this("$bin_printp -tmatfn $tmat_bin >$tmat_txt",$exec_resdir,"Read $tmat_bin from output of $i state, stream 1, density 1");
    test_this("$bin_printp -mixwfn $mixw_bin >$mixw_txt",$exec_resdir,"Read $mixw_bin from output of $i state, stream 1, density 1");
    test_this("$bin_printp -tmatfn ${exec_resdir}/tmat_${i}st_1str_1den >$tmat_txt_match",$exec_resdir,"Read template $tmat_bin");
    test_this("$bin_printp -mixwfn ${exec_resdir}/mixw_${i}st_1str_1den >$mixw_txt_match",$exec_resdir,"Read template $mixw_bin");
    compare_these_two(${tmat_txt},${tmat_txt_match},$exec_resdir,"Transition matrices match $i state, stream 1, density 1");
    compare_these_two(${mixw_txt},${mixw_txt_match},$exec_resdir,"Mixture weight match $i state, stream 1, density 1");

    unlink($tmat_bin,$tmat_txt, $mixw_bin,$mixw_txt, $tmat_txt_match, $mixw_txt_match);

    $cmd = $tmp ;
    $cmd .= "-nstream 4 -ndensity 256";
    test_this($cmd,$exec_resdir,"mk_flat dry run $i state, stream 4, density 256");
    test_this("$bin_printp -tmatfn $tmat_bin >$tmat_txt",$exec_resdir,"Read $tmat_bin from output of $i state, stream 4, density 256");
    test_this("$bin_printp -mixwfn $mixw_bin >$mixw_txt",$exec_resdir,"Read $mixw_bin from output of $i state, stream 4, density 256");
    unlink($tmat_bin,$tmat_txt, $mixw_bin,$mixw_txt);

    $cmd = $tmp ;
    $cmd .= "-nstream 4 -ndensity 1024";
    test_this($cmd,$exec_resdir,"mk_flat dry run $i state, stream 4, density 1024");
    test_this("$bin_printp -tmatfn $tmat_bin >$tmat_txt",$exec_resdir,"Read $tmat_bin from output of $i state, stream 4, density 1024");
    test_this("$bin_printp -mixwfn $mixw_bin >$mixw_txt",$exec_resdir,"Read $mixw_bin from output of $i state, stream 4, density 1024");
    unlink($tmat_bin,$tmat_txt, $mixw_bin,$mixw_txt);

}

