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

if (($#ARGV < 0) || ($#ARGV > 1)) {
  print "Usage: $0 <no of states per hmm> (allow skips yes/no)\n";
  print "\tDefault: allow skips yes\n";
  exit -1;
}

$nstates = $ARGV[0];
if ( $#ARGV == 0) {
  $allowskips = "yes";
} else {
  $allowskips = $ARGV[1];
}
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

$no = $nstates + 1;;
print "$no\n";

$n = 1;
while ($n <= $nstates) {
    $j = 1;
    if ( $allowskips eq "no" ) {
     $nnn = $n + 1;
    } else {
     $nnn = $n + 2;
    }

    while ($j <= $no) {
        if ($j < $n || $j > $nnn) {
            print "0.0	";
	    $j = $j + 1;
        } else {
            print "1.0	";
	    $j = $j + 1
	}
    }
    print "\n";
    $n = $n + 1;
}
print "# Last state has no outgoing arcs unless\n";
print "# embedded in a sentence hmm structure\n";
