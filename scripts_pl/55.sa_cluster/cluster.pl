#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 2006 Carnegie Mellon University.  All rights
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
## Author: David Huggins-Daines
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;
use IO::File;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

# This script implements the technique described in S.E. Johnson and
# P.C. Woodland, "Speaker clustering by direct maximization of the
# MLLR-adapted likelihood", Proceedings of ICSLP 1998.

die "Usage: $0 <iter> <nclust> <clustdir> [<parent>]\n" unless @ARGV >= 3;
my ($iter, $nclust, $clustdir, $parent) = @ARGV;

my ($parent_ctl, $parent_lsn);
# (This particular comparison is necessary since if it was undef in
# slave_convg.pl it will be an empty string here)
if ($parent ne "") {
    # Use parent's file listing
    $parent_ctl = "$parent.ctl";
    $parent_lsn = "$parent.lsn";
} else {
    # Use the top-level stuff
    ($parent_ctl, $parent_lsn) = GetLists();
}

# Generate new clusters for the following iteration.
# Iterate over all parts of Forward-Backward evaluation
my (%ll, %idx); # Map uttid to best likelihood and cluster index (good
                # thing we have lots of RAM as there could be hundreds
                # of thousands of these)
foreach my $part (1..$ST::CFG_NPART) {
    foreach my $clust (1..$nclust) {
	my $logfile = catfile($clustdir, "${ST::CFG_EXPTNAME}.evaluate.$clust.$iter-$part.bw.log");
	# Skip empty clusters
	open LOG, "<$logfile" or next;
	while (<LOG>) {
	    if (/^utt>/) {
		next if /ERROR/;
		my (undef, $seq, $uttid, $nfr, $del, $shmm,
		    $avg_alpha, $avg_beta, $avg_reest,
		    $avg_post_prune, $frame_ll, $utt_ll) = split;
		if (!defined($ll{$uttid})
		    or $ll{$uttid} < $utt_ll) {
		    $ll{$uttid} = $utt_ll;
		    $idx{$uttid} = $clust;
		}
	    }
	}
    }
}

# Output the new control and transcription files
my $newiter = $iter+1;
my (@ctlfh, @lsnfh);
foreach my $clust (1..$nclust) {
    my $ctl = catfile($clustdir, "clust.$clust.$newiter.ctl");
    my $lsn = catfile($clustdir, "clust.$clust.$newiter.lsn");
    $ctlfh[$clust] = IO::File->new(">$ctl") or die "Failed to open $ctl: $!";
    $lsnfh[$clust] = IO::File->new(">$lsn") or die "Failed to open $lsn: $!";
}
open CTL, "<$parent_ctl" or die "Failed to open $parent_ctl: $!";
open LSN, "<$parent_lsn" or die "Failed to open $parent_lsn: $!";
while (<CTL>) {
    my ($fileid, $sf, $ef, $uttid) = split;
    $uttid = $fileid unless defined $uttid;
    unless (defined($idx{$uttid})) {
	# Discard the transcription for this utterance which failed in training
	# FIXME: This might mean that we could converge to zero utterances
	<LSN>;
	next;
    }
    my $idx = $idx{$uttid};
    $ctlfh[$idx]->print($_);
    my $trans = <LSN>;
    $lsnfh[$idx]->print($trans);
}
close CTL;
close LSN;
foreach (@ctlfh, @lsnfh) {
    close $_ if defined $_;
}
