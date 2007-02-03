#!/usr/local/bin/perl

use strict;
require './scripts/testlib.pl';

chomp(my $host=`../config.guess | xargs ../config.sub`);
my $bindir="../bin.$host/";
my $resdir="res/";
my $exec_resdir="printp";
my $bin="$bindir$exec_resdir";


my $tmatarg="-tmatfn res/hmm/transition_matrices";
my $mixwarg="-mixwfn res/hmm/mixture_weights";
my $meanarg="-gaufn  res/hmm/means";
my $vararg="-gaufn  res/hmm/variances";
my $gaucntarg="-gaucntfn  ./printp/gauden_counts";
my $mdefarg="-moddeffn ./res/hmm/RM.1000.mdef";

my $tmatout="${exec_resdir}.tmat.txt";
my $mixwout="${exec_resdir}.mixw.txt";
my $meanout="${exec_resdir}.mean.txt";
my $varout ="${exec_resdir}.var.txt";
my $gaucntout="${exec_resdir}.gaucnt.txt";
my $mdefout="${exec_resdir}.mdef.txt";

my $tmatmatch="${exec_resdir}/test_${exec_resdir}_tmat";
my $mixwmatch ="${exec_resdir}/test_${exec_resdir}_mixw";
my $meanmatch="${exec_resdir}/test_${exec_resdir}_means";
my $varmatch ="${exec_resdir}/test_${exec_resdir}_var";
my $gaucntmatch="${exec_resdir}/test_${exec_resdir}_gaucnt.1gau";
my $mdefmatch ="${exec_resdir}/test_${exec_resdir}_mdef";

test_help($bindir,$exec_resdir);
test_example($bindir,$exec_resdir);

test_this("$bin $tmatarg > $tmatout",$exec_resdir,"DRY RUN w/-tmat TEST");
compare_these_two($tmatmatch,$tmatout,,$exec_resdir,"TRANSITION MATRIX PRINTING");
unlink($tmatout);

test_this("$bin $mixwarg > $mixwout",$exec_resdir,"DRY RUN w/-mixw TEST");
compare_these_two(${mixwmatch},${mixwout},$exec_resdir,"MIXTURE WEIGHTS PRINTING");
unlink($mixwout);

test_this("$bin $meanarg > $meanout",$exec_resdir,"DRY RUN w/-means TEST");
compare_these_two(${meanmatch},${meanout},$exec_resdir,"MEANS PRINTING");
unlink($meanout);

test_this("$bin $vararg > $varout",$exec_resdir,"DRY RUN w/-vars TEST");
compare_these_two(${varmatch},${varout},$exec_resdir,"VARS PRINTING");
unlink($varout);

test_this("$bin $gaucntarg > $gaucntout",$exec_resdir,"DRY RUN w/-gaucnts gaussian 1 TEST");
compare_these_two(${gaucntmatch},${gaucntout},$exec_resdir,"GAUCNTS PRINTING");
unlink($gaucntout);

