#!/usr/local/bin/perl

use strict;
require './scripts/testlib.pl';

my $bindir="../bin.i686-pc-linux-gnu/";
my $resdir="res/";
my $exec_resdir="printp";
my $bin="$bindir$exec_resdir";


my $tmatarg="-tmatfn res/hmm/transition_matrices";
my $mixwarg="-mixwfn res/hmm/mixture_weights";
my $meanarg="-gaufn  res/hmm/means";
my $vararg="-gaufn  res/hmm/variances";
my $mdefarg="-moddeffn ./res/hmm/RM.1000.mdef";

my $tmatout="${exec_resdir}.tmat.txt";
my $mixwout="${exec_resdir}.mixw.txt";
my $meanout="${exec_resdir}.mean.txt";
my $varout ="${exec_resdir}.var.txt";
my $mdefout="${exec_resdir}.mdef.txt";

my $tmatmatch="${exec_resdir}/test_${exec_resdir}_tmat";
my $mixwmatch ="${exec_resdir}/test_${exec_resdir}_mixw";
my $meanmatch="${exec_resdir}/test_${exec_resdir}_means";
my $varmatch ="${exec_resdir}/test_${exec_resdir}_var";
my $mdefmatch ="${exec_resdir}/test_${exec_resdir}_mdef";

test_help($bindir,$exec_resdir);
test_example($bindir,$exec_resdir);

test_this("$bin $tmatarg > $tmatout",$exec_resdir,"DRY RUN w/-tmat TEST");
system("gzip -f -d $tmatmatch.gz") unless -e "${tmatmatch}";
compare_these_two($tmatmatch,$tmatout,,$exec_resdir,"TRANSITION MATRIX PRINTING");
system("gzip -f  $tmatmatch.gz") unless -e "${tmatmatch}";

test_this("$bin $mixwarg > $mixwout",$exec_resdir,"DRY RUN w/-mixw TEST");
system("gzip -f -d $mixwmatch.gz") unless  -e "${mixwmatch}";
compare_these_two(${mixwmatch},${mixwout},$exec_resdir,"MIXTURE WEIGHTS PRINTING");
system("gzip -f  $tmatmatch.gz") unless -e "${tmatmatch}";

test_this("$bin $meanarg > $meanout",$exec_resdir,"DRY RUN w/-means TEST");
system("gzip -f -d $meanmatch.gz") unless -e "${meanmatch}";
compare_these_two(${meanmatch},${meanout},$exec_resdir,"MEANS PRINTING");
system("gzip -f  $tmatmatch.gz") unless -e "${tmatmatch}";

test_this("$bin $vararg > $varout",$exec_resdir,"DRY RUN w/-vars TEST");
system("gzip -f -d $varmatch.gz") unless -e "${varmatch}";
compare_these_two(${varmatch},${varout},$exec_resdir,"VARS PRINTING");
system("gzip -f  $tmatmatch.gz") unless -e "${tmatmatch}";

