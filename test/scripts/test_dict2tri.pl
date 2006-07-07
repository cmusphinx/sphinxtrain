#!/usr/local/bin/perl

use strict;
my $bin="../bin.i686-pc-linux-gnu/dict2tri";
my $resdir="res/";
my $exec_resdir="dict2tri";

my $args = "-dictfn $resdir/communicator.dic.cmu ";
$args .= "-basephnfn $resdir/communicator.phone.cmu";
my $w_output = "dict2tri.triphones";
my $wo_output = "dict2tri.no_triphones";
my $w_triphones = "-btwtri yes";
my $wo_triphones = "-btwtri no";
my $match_w_output = "${exec_resdir}/test.dict2tri.internaltriphones";
my $match_wo_output = "${exec_resdir}/test.dict2tri.no_internaltriphones";

#Testing with triphone computed
system("$bin $args $w_triphones > $w_output");
system("diff $w_output $match_w_output");
if($! == 0){
    printf("Test dict2tri compute internal triphones PASSED\n");
}else{
    printf("Test dict2tri compute internal triphones FAILED, exit at %d\n",$!);
}

system("$bin $args $wo_triphones > $wo_output");
system("diff $wo_output $match_wo_output");

if($! == 0){
    printf("Test dict2tri wo compute internal triphones PASSED\n");
}else{
    printf("Test dict2tri wo compute internal triphones FAILED, exit at %d\n",$!);
}





