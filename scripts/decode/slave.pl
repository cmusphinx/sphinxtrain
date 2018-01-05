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
#
#  Script that launches the main decoder script
#
# ====================================================================

use File::Copy;
use File::Basename;
use File::Spec::Functions;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

#************************************************************************
# this script launches the decoder scripts.
#************************************************************************

$| = 1; # Turn on autoflushing

if ($ARGV[0] ne '-alignonly') {
    my @jobs;
    Log ("MODULE: DECODE Decoding using models previously trained\n");
    for (my $i = 1; $i <= $ST::DEC_CFG_NPART; $i++) {
	push @jobs, LaunchScript('decode', [$ST::DEC_CFG_SCRIPT,  $i, $ST::DEC_CFG_NPART]);
    }
    foreach my $job (@jobs) {
	WaitForScript($job);
    }
}

Log ("        Aligning results to find error rate\n");
compute_acc();

sub compute_acc {
  $result_dir = "$ST::DEC_CFG_RESULT_DIR";
  $match_file = "$result_dir/${ST::DEC_CFG_EXPTNAME}.match";

  concat_hyp($match_file);
  $tmp_hyp = condition_text($match_file);
  $tmp_ref = condition_text($ST::DEC_CFG_TRANSCRIPTFILE);
  align_hyp($tmp_ref, $tmp_hyp);
  unlink $tmp_ref, $tmp_hyp;
}

sub concat_hyp {
  my $match_file = shift;
  open (MATCH, ">$match_file") or die "Can't open file $match_file\n";
  for (my $i = 1; $i <= $ST::DEC_CFG_NPART; $i++) {

    $hypfile = "$result_dir/${ST::DEC_CFG_EXPTNAME}-${i}-${ST::DEC_CFG_NPART}.match";

    open (HYP, "< $hypfile") or ((warn "Can't open $hypfile\n" and next));
    while (<HYP>) {
	# Remove the acoustic score if any
	s/\((\S+)\s+[^\)]+\)$/($1)/;
	# Remove file paths as they make sclite upset
	s,\(.*/,\(,;
	print MATCH "$_";
    }
    close(HYP);
  }
  close(MATCH);
}

sub condition_text {
  my $fn = shift;
  my $tmpfn = $fn.$$;
  my $fileid = $ST::DEC_CFG_LISTOFFILES;

  open (IN, "< $fn") or die "Can't open $fn for reading\n";
  open (OUT, "> $tmpfn") or die "Can't open $tmpfn for writing\n";
  open (LIST, "< $fileid") or die "Can't open $fileid for reading\n";

  while (<IN>) {
    m/^(.*)\((\S+)\)\s*$/;
# Make them uppercase
    my $text = uc($1);
    my $id = uc($2);
# Removing leading spaces
    $text =~ s/^\s+//;
# Removing trailing spaces
    $text =~ s/\s+$//;
# Removing double spaces;
    $text =~ s/\s+/ /g;
# Removing some unwanted strings
    $text =~ s/_/ /g;
    $text =~ s/\./ /g;
    $text =~ s/\+\+[^\s\+]+\+\+/ /g;
    $text =~ s/\+\+[^\s\+]+\+\+/ /g;
    $text =~ s/<\/?s>/ /gi;
    $text =~ s/<sil>/ /gi;
    $text =~ s/\(\d+\)\b/ /g;
    my $file = <LIST>;
    @path = split /[\\\/]/, $file;
    my $user;
    if ($#path > 0) {
      $user = $path[$#path - 1];
    } else {
      $user = "user";
    }
    print OUT "$text ($user-$id)\n";
  }
  close(LIST);
  close(OUT);
  close(IN);
  return $tmpfn;
}

sub align_hyp {
  my $ref = shift;
  my $hyp = shift;
  my $align = $ST::DEC_CFG_ALIGN;
  my $use_cer = $ST::DEC_CFG_ALIGN_CER eq "yes" ? "--cer" : "";

  if ($align eq 'builtin') {
    my $outfile = "$ST::DEC_CFG_RESULT_DIR/${ST::DEC_CFG_EXPTNAME}.align";
    my $thisdir = dirname($0);
    my ($wer, $ser, $word_total, $sent_total, $sent_err);
    open (OUT, "> $outfile") or die "Can't open $outfile for writing\n";
    my $cmdln = "perl \"$thisdir/word_align.pl\" $use_cer -i \"$ref\" \"$hyp\"";
    $sent_total = 0;
    if (open (PIPE, "$cmdln 2>&1 |")) {
      while (<PIPE>) {
	print OUT "$_";
	if (/^TOTAL Words: (\d+)/) {
	    $word_total = $1;
	}
	elsif (/^TOTAL Percent.*Error = ([\d\.]+)%/) {
	    $wer = $1;
	}
	elsif (/^Words: (\d+).*Errors: (\d+)/) {
	    $sent_total++;
	    $sent_err++ if $2;
	}
      }
    }
    close(OUT);
    close(PIPE);
    if ($?) {
	die "word_align.pl failed with error code $?";
    }
    $ser = ($sent_err / $sent_total) * 100;
    Log(sprintf("SENTENCE ERROR: %.1f%% (%d/%d)   %s ERROR RATE: %.1f%% (%d/%d)",
		$ser, $sent_err, $sent_total,
		$use_cer ? "CHARACTER" : "WORD",
		$wer, $word_total * $wer / 100, $word_total), 'result');
    HTML_Print("<p class='result'>", FormatURL("$outfile", "Log File"), "</p>");
  } elsif ($align =~ m/sclite/i) {
    my $outfile = "$ST::DEC_CFG_RESULT_DIR/${ST::DEC_CFG_EXPTNAME}.align";
    my ($wer, $ser, $word_total, $sent_total);
    open (OUT, "> $outfile") or die "Can't open $outfile for writing\n";
    my $cmdln = "\"$align\" " .
	  "-i rm " .
	  "-o sum pralign dtl stdout " .
	  "-f 0 " .
	  "-r \"$ref\" " .
	  "-h \"$hyp\"";
    if (open (PIPE, "$cmdln 2>&1 |")) {
      while (<PIPE>) {
	print OUT "$_";
	if (m/\|\s*Sum\/Avg\s*\|\s*(\d+)\s*(\d+).*\s+(\d+\.\d+)\s+(\d+\.\d+)\s*\|/) {
	    $sent_total = $1;
	    $word_total = $2;
	    $wer = $3;
	    $ser = $4;
	}
      }
    }
    close(OUT);
    close(PIPE);
    if ($?) {
	die "sclite failed with error code $?";
    }
    Log(sprintf("SENTENCE ERROR: %.1f%% (%d/%d)   WORD ERROR RATE: %.1f%% (%d/%d)",
		$ser, $sent_total * $ser / 100, $sent_total,
		$wer, $word_total * $wer / 100, $word_total), 'result');
    HTML_Print("<p class='result'>", FormatURL("$outfile", "Log File"), "</p>");
  } else {
    Log("Accuracy not computed, please visually compare the decoder output with the reference file");
  }
}
