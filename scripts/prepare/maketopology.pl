#!/usr/bin/perl
# ====================================================================
# Copyright (c) 2000 Carnegie Mellon University.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer. 
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# This work was supported in part by funding from the Defense Advanced 
# Research Projects Agency and the National Science Foundation of the 
# United States of America, and the CMU Sphinx Speech Consortium.
#
# THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
# ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
# NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ====================================================================

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), '../lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

my ($outfile, $nstates, $allowskips) = @ARGV;

unless (defined $outfile) {
  LogError("topology file name is required");
  exit (1);
}

$nstates = (defined($ST::CFG_STATESPERHMM) ? $ST::CFG_STATESPERHMM : 3) unless defined $nstates;
$allowskips = (defined($ST::CFG_SKIPSTATE) ? $ST::CFG_SKIPSTATE : "yes") unless defined $allowskips;

open STDOUT, ">$outfile" or die "Failed to open $outfile: $!";
print "#\n";
print "# ${nstates}-state Bakis topology HMM with non-emitting last state\n";
print "# These values are normalized so that rows sum to one.\n";
print "#\n";
print "# NO COMMENTS BETWEEN # OF STATES AND TRANSITION MATRIX\n";
print "#\n";
print "#\n";
print "#Version number\n";
print "0.1\n";
print "# Number of states per model followed by transition matrix\n";

my $no = $nstates + 1;;
print "$no\n";

my $n = 1;
while ($n <= $nstates) {
    my $j = 1;
    my $nnn;
    if ( $allowskips eq "no" ) {
     $nnn = $n + 1;
    } else {
     $nnn = $n + 2;
    }

    while ($j <= $no) {
        if ($j < $n || $j > $nnn) {
            print "0.0	";
	} elsif ($j == $n) {
	    print "3.0	";
        } else {
            print "1.0	";
	}
	$j = $j + 1;
    }
    print "\n";
    $n = $n + 1;
}
print "# Last state has no outgoing arcs unless\n";
print "# embedded in a sentence hmm structure\n";
