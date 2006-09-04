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

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

$|=1;

# This script implements the technique described in S.E. Johnson and
# P.C. Woodland, "Speaker clustering by direct maximization of the
# MLLR-adapted likelihood", Proceedings of ICSLP 1998.
#
# One convenient consequence of this method which is not mentioned in
# the above paper is that the final set of MLLR transforms which
# accompany the clusters are identical to the transforms that would be
# generated in the first stage of cluster-adaptive training.
# Therefore the output of this script serves as the input to the next
# stage.

my ($nclust, $clustdir, $parent) = @ARGV;
die "Usage: $0 NCLUST [CLUSTDIR] [PARENT]\n" unless @ARGV >= 1;

$clustdir = catdir($ST::CFG_BASE_DIR, 'cluster') unless defined($clustdir);
my $logfile = catfile($clustdir, "$ST::CFG_EXPTNAME.clust.log");
open LOG, ">>$logfile" or die "Failed to open $logfile: $!";

my ($parent_ctl, $parent_lsn);
# (This particular comparison is necessary since if it was undef in
# cluster.pl it will be an empty string here)
if ($parent ne "") {
    # Use parent's file listing
    $parent_ctl = "$parent.ctl";
    $parent_lsn = "$parent.lsn";
} else {
    # Use the top-level stuff
    ($parent_ctl, $parent_lsn) = GetLists();
}
print LOG "Splitting node in $clustdir from $parent_ctl\n";

sub utt_n_frames {
    my ($fileid, $sf, $ef, $uttid) = @_;
    my $nframes;
    if (defined($sf) and defined($ef)) {
	$nframes = $ef - $sf;
    } else {
	# Get the file length from the file (this will all get
	# moved into a module soonish)
	my $file = catfile($ST::CFG_FEATFILES_DIR, "$fileid.$ST::CFG_FEATFILE_EXTENSION");
	die "Feature file $file does not exist: $!" unless -e $file;
	$nframes = -s $file;
	$nframes -= 4;
	$nframes /= $ST::CFG_VECTOR_LENGTH;
	die "Feature file $file is oddly sized: $nframes"
	    unless $nframes == int($nframes);
    }
    return $nframes;
}

    # Partition the data into a maximum of N clusters
print LOG "    Partitioning data...";
# Do this by equal number of frames (this will go in the
# library soon since we actually want to do this for normal
# training)
open CTL, "<$parent_ctl" or die "Failed to open $parent_ctl: $!";
my $total_frames = 0;
while (<CTL>) {
    my $nframes = utt_n_frames(split);
    $total_frames += $nframes;
}

# Enforce a minimum number of frames per cluster (in config file)
my $min_frames = defined($ST::CFG_MIN_CLUSTER_FRAMES) ?
    $ST::CFG_MIN_CLUSTER_FRAMES : 200000; # A reasonable default
# First use it as a top condition for splitting
if ($total_frames < $min_frames) {
    print LOG " Node has fewer than $min_frames frames, terminating.\n";
    exit 0;
}
# Next determine the number of parts (max $nclust)
my $part_frames = $total_frames / $nclust;
if ($part_frames < $min_frames) {
    # Round up to the next whole number
    $nclust = int($total_frames / $min_frames) + 1;
    $part_frames = $total_frames / $nclust;
}
print LOG " Total frames: $total_frames Parts: $nclust Each part: $part_frames\n";

seek CTL, 0, 0;
open LSN, "<$parent_lsn" or die "Failed to open $parent_lsn: $!";
my $part = 1;
$total_frames = 0;
my $part_ctl = catfile($clustdir, "clust.1.1.ctl");
my $part_lsn = catfile($clustdir, "clust.1.1.lsn");
open OUTCTL, ">$part_ctl" or die "Failed to open $part_ctl: $!";
open OUTLSN, ">$part_lsn" or die "Failed to open $part_lsn: $!";
while (<CTL>) {
    my $nframes = utt_n_frames(split);
    $total_frames += $nframes;
    if ($total_frames > $part_frames * $part) {
	++$part;
	$part_ctl = catfile($clustdir, "clust.$part.1.ctl");
	$part_lsn = catfile($clustdir, "clust.$part.1.lsn");
	open OUTCTL, ">$part_ctl" or die "Failed to open $part_ctl: $!";
	open OUTLSN, ">$part_lsn" or die "Failed to open $part_lsn: $!";
    }
    my $trans = <LSN>;
    print OUTCTL $_;
    print OUTLSN $trans;
}
close CTL;
close LSN;


# Iterate until convergence
my $conv_ratio = 1;
my $last_ll;
my $iter;
ITERATION:
for ($iter = 1; $iter < $ST::CFG_MAX_ITERATIONS; ++$iter) {
    my @parts;
    # Collect MLLR-adapted statistics for each cluster
    foreach my $clust (1..$nclust) {
	my $ctl = catfile($clustdir, "clust.$clust.$iter.ctl");
	my $lsn = catfile($clustdir, "clust.$clust.$iter.lsn");
	# Skip empty clusters
	next if -s $ctl == 0;
	my $npart = int($ST::CFG_NPART / $nclust + 0.5);
	$npart = 1 if $npart == 0;
	foreach my $part (1..$npart) {
	    # Use the last iteration's MLLR for Forward-Backward if
	    # available (this will allow us to check for convergence among
	    # other things)
	    if ($iter == 1) {
		push @parts, LaunchScript("est.$clust.$iter.$part",
					  ['baum_welch.pl',
					   $clustdir, $iter, $clust, $part, $npart,
					   $ctl, $lsn, 'estimate']);
	    } else {
		my $previter = $iter-1;
		my $mllr = catfile($clustdir, "clust.$clust.$previter.mllr");
		push @parts, LaunchScript("est.$clust.$iter.$part",
					  ['baum_welch.pl',
					   $clustdir, $iter, $clust, $part, $npart,
					   $ctl, $lsn, 'estimate', $mllr]);
	    }
	}
    }

    # Check for convergence of the MLLR-adapted likelihood
    WaitForScript($_) foreach @parts;
    my ($total_ll, $total_nfr);
    foreach my $clust (1..$nclust) {
	my $npart = int($ST::CFG_NPART / $nclust + 0.5);
	$npart = 1 if $npart == 0;
	foreach my $part (1..$npart) {
	    my $logfile = catfile($clustdir,
				  "${ST::CFG_EXPTNAME}.estimate.$clust.$iter-$part.bw.log");
	    # Skip empty clusters
	    open BWLOG, "<$logfile" or next;
	    while (<BWLOG>) {
		if (/overall>/) {
		    my (undef, undef, $nfr, undef, undef, $ll) = split;
		    $total_ll += $ll;
		    $total_nfr += $nfr;
		}
	    }
	}
    }
    close BWLOG;
    my $avg_ll = $total_ll/$total_nfr;
    printf LOG "Iteration %d Average Adapted Log-Likelihood Per Frame = %f\n", $iter, $avg_ll;

    if (defined($last_ll)) {
	my $convg = ($avg_ll-$last_ll)/abs($last_ll);
	printf LOG "Convergence ratio %f\n", $convg;

	if ($convg < 1e-3) {
	    print LOG "Likelihoods have converged! Speaker clustering completed!\n";
	    last ITERATION;
	} elsif ($iter >= $ST::CFG_MAX_ITERATIONS) {
	    print LOG "Maximum desired iterations $ST::CFG_MAX_ITERATIONS performed. Terminating clustering iteration\n";
	    last ITERATION;
	}
    }
    $last_ll = $avg_ll;

    # Generate a new MLLR matrix and evaluate all the data with each cluster
    foreach my $clust (1..$nclust) {
	# Skip empty clusters
	my $ctl = catfile($clustdir, "clust.$clust.$iter.ctl");
	next if -s $ctl == 0;

	my $norm = LaunchScript("mllr.$clust.$iter",
				['mllr_solve.pl', $clustdir, $iter, $clust]);
	WaitForScript($norm);
	# Evaluation for each cluster depends only on the MLLR matrix
	@parts = ();
	foreach my $part (1..$ST::CFG_NPART) {
	    my $mllr = catfile($clustdir, "clust.$clust.$iter.mllr");
	    push @parts, LaunchScript("eval.$clust.$iter.$part",
				      ['baum_welch.pl', $clustdir,
				       $iter, $clust, $part, $ST::CFG_NPART,
				       $parent_ctl, $parent_lsn,
				       'evaluate', $mllr]);
	}
    }

    # Reassign utterances to each cluster based on likelihood, check for
    # convergence, and iterate.  This depends on all the evaluation being
    # complete.
    WaitForScript($_) foreach @parts;
    my $job = LaunchScript("clust.$iter",
			   ['cluster.pl', $iter, $nclust, $clustdir, $parent]);
    WaitForScript($job);
}

# Copy the final clusters to a canonical output name so they are easy
# to find.  Also count them and terminate if there is only one
# non-empty cluster (otherwise we will end up recursing endlessly!)
# Because the final iteration may have diverged, and doesn't have an
# MLLR transform, we take the N-1 iteration.
--$iter;
die "Final iteration == 1, shouldn't happen!" if $iter == 0;
my $active = 0;
my $list = catfile($clustdir, "clusters.txt");
open LIST, ">$list" or die "Failed to open $list: $!";
foreach my $clust (1..$nclust) {
    my $final = catfile($clustdir, "clust.$clust.$iter");
    my $out = catfile($clustdir, "clust.$clust");
    my $size = -s "$final.ctl";
    if ($size > 0) {
	++$active;
	print LIST "clust.$clust\n";
	copy("$final.ctl", "$out.ctl");
	copy("$final.lsn", "$out.lsn");
	copy("$final.mllr", "$out.mllr");
    }
}
close LIST;
die "No active child nodes (shouldn't happen)" if $active == 0;
if ($active == 1) { # Should never be zero!
    print LOG "1 child node remains, terminating.\n";
    exit 0;
}

# Now split all the child nodes
print LOG "Splitting child nodes recursively... ";
my @parts;
foreach my $clust (1..$nclust) {
    print LOG "$clust ";
    my $dir = catdir($clustdir, "clust.$clust");
    rmtree($dir, 0, 1);
    mkdir($dir, 0777);
    push @parts, LaunchScript("split.$clust",
			      ['split_convg.pl',
			       4, $dir, $dir]);
}
WaitForScript($_) foreach @parts;
print LOG "complete\n";
close LOG;
