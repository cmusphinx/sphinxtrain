#!/usr/local/bin/perl

use strict;
require './scripts/testlib.pl';

chomp(my $host=`../config.guess | xargs ../config.sub`);
my $bindir="../bin.$host/";
my $resdir="res/";
my $exec_resdir="bugcase2";
my $bin="${bindir}mk_flat";

my $topo="${exec_resdir}/bug.topo";
my $mdef="mk_mdef_gen/4st.ci.mdef";

test_this("$bin -moddeffn $mdef -topo $topo -tmatfn tm -mixwfn mw","mk_flat","mk_flat format bug (Tracker no: 1504066) expected", 256);

unlink("tm","mw");


