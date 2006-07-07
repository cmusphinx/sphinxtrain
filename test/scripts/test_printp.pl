#!/usr/local/bin/perl

use strict;
my $bin="../bin.i686-pc-linux-gnu/printp";
my $resdir="res/";
my $exec_resdir="printp";

my $tmatarg="-tmatfn res/hmm/transition_matrices";
my $mixwarg="-mixwfn res/hmm/mixture_weights";
my $meanarg="-gaufn  res/hmm/means";
my $vararg="-gaufn  res/hmm/variances";
my $mdefarg="-moddeffn ./res/hmm/RM.1000.mdef";

my $tmatout="printp.tmat.txt";
my $mixwout="printp.mixw.txt";
my $meanout="printp.mean.txt";
my $varout ="printp.var.txt";
my $mdefout="printp.mdef.txt";

my $tmatmatch="printp/test_printp_tmat";
my $mixwmatch ="printp/test_printp_mixw";
my $meanmatch="printp/test_printp_means";
my $varmatch ="printp/test_printp_var";
my $mdefmatch ="printp/test_printp_mdef";

system("$bin $tmatarg > $tmatout");
if(-e "${tmatmatch}.gz"){
    system("gzip -d $tmatmatch.gz");
}
system("echo diff ${tmatmatch} ${tmatout}");
system("diff ${tmatmatch} ${tmatout}");
if($? == 0){
    printf("Test printp transition matrix printing PASSED\n");
}else{
    printf("Test printp transition matrix printing FAILED, signal $?, msgn $!\n");
}

system("$bin $mixwarg > $mixwout");
if(-e "${mixwmatch}.gz"){
    system("gzip -d $mixwmatch.gz");
}
system("diff ${mixwmatch} ${mixwout}");
if($?==0){
    printf("Test printp mixture weights printing PASSED\n");
}else{
    printf("Test printp mixture weights printing FAILED, signal $?, msgn $!\n");
}

system("$bin $meanarg > $meanout");
if(-e "${meanmatch}.gz"){
    system("gzip -d $meanmatch.gz");
}
system("diff ${meanmatch} ${meanout}");
if($?==0){
    printf("Test printp means printing PASSED\n");
}else{
    printf("Test printp means printing FAILED, signal $?, msgn $!\n");
}

system("$bin $vararg > $varout");
if(-e "${varmatch}.gz"){
    system("gzip -d $varmatch.gz");
}
system("diff ${varmatch} ${varout}");
if($?==0){
    printf("Test printp variances printing PASSED\n");
}else{
    printf("Test printp variances printing FAILED, signal $?, msgn $!\n");
}
