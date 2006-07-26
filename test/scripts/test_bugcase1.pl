#!/usr/local/bin/perl

use strict;
require './scripts/testlib.pl';

my $bindir="../bin.i686-pc-linux-gnu/";
my $resdir="res/";
my $exec_resdir="bugcase1";
my $bin="${bindir}mk_mdef_gen";

my $filler="${exec_resdir}/time.filler";
my $phonelist="${exec_resdir}/time.phonelist";

test_this("$bin -phnlstfn $phonelist -oalltphnmdef all.mdef -fdictfn $filler -dictfn ./res/cmudict.0.6d","DRY RUN of hack itree max cell bug");
unlink("all.mdef");

