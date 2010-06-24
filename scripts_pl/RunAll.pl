#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
## reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
##
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer. 
##
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in
##    the documentation and/or other materials provided with the
##    distribution.
##
## This work was supported in part by funding from the Defense Advanced 
## Research Projects Agency and the National Science Foundation of the 
## United States of America, and the CMU Sphinx Speech Consortium.
##
## THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
## ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
## THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
## PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
## NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
## ====================================================================
##
## Author: Ricky Houghton
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

# Start and end numbers, useful if things get interrupted
my ($start, $end) = @ARGV;
$start = 0 unless defined($start);
$end = 100 unless defined($end);

# What pieces would you like to compute.
my @steps =
    ("$ST::CFG_SCRIPT_DIR/00.verify/verify_all.pl",
     "$ST::CFG_SCRIPT_DIR/01.lda_train/slave_lda.pl",
     "$ST::CFG_SCRIPT_DIR/02.mllt_train/slave_mllt.pl",
     "$ST::CFG_SCRIPT_DIR/05.vector_quantize/slave.VQ.pl",
     "$ST::CFG_SCRIPT_DIR/10.falign_ci_hmm/slave_convg.pl",
     "$ST::CFG_SCRIPT_DIR/11.force_align/slave_align.pl",
     "$ST::CFG_SCRIPT_DIR/12.vtln_align/slave_align.pl",
     "$ST::CFG_SCRIPT_DIR/20.ci_hmm/slave_convg.pl",
     "$ST::CFG_SCRIPT_DIR/30.cd_hmm_untied/slave_convg.pl",
     "$ST::CFG_SCRIPT_DIR/40.buildtrees/slave.treebuilder.pl",
     "$ST::CFG_SCRIPT_DIR/45.prunetree/slave.state-tying.pl",
     "$ST::CFG_SCRIPT_DIR/50.cd_hmm_tied/slave_convg.pl",
     "$ST::CFG_SCRIPT_DIR/60.lattice_generation/slave_genlat.pl",
     "$ST::CFG_SCRIPT_DIR/61.lattice_pruning/slave_prune.pl",
     "$ST::CFG_SCRIPT_DIR/62.lattice_conversion/slave_conv.pl",
     "$ST::CFG_SCRIPT_DIR/65.mmie_train/slave_convg.pl",
     "$ST::CFG_SCRIPT_DIR/90.deleted_interpolation/deleted_interpolation.pl",
    );

# Do the common initialization and state tying steps
foreach my $step (@steps) {
    my ($index) = ($step =~ m,.*/(\d\d)\.,);
    next if $index < $start;
    last if $index > $end;
    my $ret_value = RunScript($step);
    die "Something failed: ($step)\n" if $ret_value;
}
