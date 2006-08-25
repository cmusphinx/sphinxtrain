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
#  Tutorial set up script
#
# ====================================================================
use strict;
use Cwd;
use File::Path;

my $SphinxTrain_dir = getcwd();

my $task = "an4";
if ($#ARGV > 0) {
  die "Usage: $0 <tutorial task name>\n";
} elsif ($#ARGV == 0) {
  $task = $ARGV[0];
} 

print "Building task $task\n";

my $task_dir = "../$task";
mkdir "$task_dir" unless (-e "$task_dir");
chdir "$task_dir";

system("perl \"$SphinxTrain_dir/scripts_pl/setup_SphinxTrain.pl\" " .
       "-force -sphinxtraindir \"$SphinxTrain_dir\" -task $task");

mkdir "feat" if (! -e "feat");
open (CTL, "etc/${task}_train.fileids") 
  or die "Could not open control file etc/${task}_train.fileids\n";
while (<CTL>) {
  s/[\/\\][^\/\\]+$//g;
  mkpath ("feat/$_", 0, 0755) if (! -e "feat/$_");
}
close(CTL);

print "\n\nNow, please do:\n";
print "\tcd $task_dir\n";
print "And then, in Unix/Linux:\n";
print "\tperl scripts_pl/make_feats.pl -ctl etc/${task}_train.fileids (if needed)\n";
print "\tperl scripts_pl/RunAll.pl\n";
print "Or in Windows:\n";
print "\tperl scripts_pl\\make_feats.pl -ctl etc\\${task}_train.fileids (if needed)\n";
print "\tperl scripts_pl\\RunAll.pl\n";
