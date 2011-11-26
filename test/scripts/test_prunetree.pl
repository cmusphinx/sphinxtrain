#!/usr/local/bin/perl

use strict;
require './scripts/testlib.pl';

chomp(my $host=`../config.guess | xargs ../config.sub`);
my $bindir="../src/programs/prunetree/";
my $resdir="res/";
my $exec_resdir="prunetree";
my $bin="$bindir$exec_resdir";

test_help($bindir,$exec_resdir);
test_example($bindir,$exec_resdir);

my $unprunetreedir="./res/trees/CFS3.unpruned/";
my $prunetreedir2000="./res/trees/CFS3.2000/";
my $prunetreedir500="./res/trees/CFS3.500/";
my $prunetreedir100="./res/trees/CFS3.100/";
my $untiedmdef="./res/hmm/CFS3.untied.mdef";
my $questionset="./res/linguistic_questions";
my $testdir="./test_${exec_resdir}";
my $fn;
mkdir "$testdir" || printf("$testdir is already built\n");

my $testcmd="$bin ";
$testcmd .= "-itreedir $unprunetreedir "; 
$testcmd .= "-otreedir $testdir ";
$testcmd .= "-moddeffn $untiedmdef ";
$testcmd .= " -psetfn $questionset ";

test_this($testcmd . "-nseno 2000",$exec_resdir,"DRY RUN 2000 SENONE TEST");
test_this("diff -r --exclude=.svn ./test_${exec_resdir} $prunetreedir2000",$exec_resdir,"TREES MATCH 2000 SENONE TEST");

foreach $fn (glob("$testdir/*"))
{
    unlink $fn;
}

test_this($testcmd . "-nseno 500",$exec_resdir,"DRY RUN 500 SENONE TEST");
test_this("diff -r --exclude=.svn ./test_${exec_resdir} $prunetreedir500",$exec_resdir,"TREES MATCH 500 SENONE TEST");

foreach $fn (glob("$testdir/*"))
{
    unlink $fn;
}


test_this($testcmd . "-nseno 100",$exec_resdir,"DRY RUN 100 SENONE TEST");
test_this("diff -r --exclude=.svn ./test_${exec_resdir} $prunetreedir100",$exec_resdir,"TREES MATCH 100 SENONE TEST");

foreach $fn (glob("$testdir/*"))
{
    unlink $fn;
}


test_this($testcmd . "-nseno 10",$exec_resdir,"DRY RUN 10 SENONE TEST");
test_this("diff -r --exclude=.svn ./test_${exec_resdir} $prunetreedir100",$exec_resdir,"TREES MATCH 10 SENONE TEST");

foreach $fn (glob("$testdir/*"))
{
    unlink $fn;
}


test_this($testcmd . "-nseno 1",$exec_resdir,"DRY RUN 1 SENONE TEST");
test_this("diff -r --exclude=.svn ./test_${exec_resdir} $prunetreedir100",$exec_resdir,"TREES MATCH 1 SENONE TEST");

foreach $fn (glob("$testdir/*"))
{
    unlink $fn;
}

rmdir "$testdir/";
