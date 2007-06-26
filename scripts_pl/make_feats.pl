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

use Getopt::Long;
use Pod::Usage;
use File::Basename;
use File::Spec::Functions;

use lib catdir(dirname($0), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;


if ($#ARGV == -1) {
  pod2usage(2);
}

Getopt::Long::Configure('no_auto_abbrev', 'pass_through');

GetOptions('help|h' => \$help,
	   'ctl=s' => \$ctl,
	   'cfg=s' => \$cfg_file);


if ($help) {
  pod2usage( -exitval => "NOEXIT",
             -verbose => 1 );
  system("bin/wave2feat");
  exit(-1);
}

if (!defined $cfg_file) {
  print ("-cfg not specified, using the default ./etc/sphinx_train.cfg\n");
  $cfg_file = "./etc/sphinx_train.cfg";
}

if (defined $ctl) {
  if (! -s "$cfg_file") {
    print ("unable to find default configuration file, use -cfg file.cfg or create etc/sphinx_train.cfg for default\n");
    exit -3;
  }

  require $cfg_file;

  $ST::WAVFILE_TYPE ||= 'nist';
  $ST::WAVFILES_DIR ||= 'wav';
  $ST::WAVFILE_EXTENSION ||= 'sph';
  $ST::FEATFILES_DIR ||= 'feat';
  $ST::FEATFILE_EXTENSION ||= 'mfc';

  # Read control file and create any necessary directories
  open CTL, "<$ctl" or die "Failed to open control file $ctl: $!";
  while (<CTL>) {
      chomp;
      my $dir = dirname($_);
      mkdir(catdir($ST::FEATFILES_DIR, $dir), 0777);
  }
  close CTL;

  # Now run wave2feat (should maybe be sphinx_fe eventually)
  system("bin/wave2feat -verbose yes -c \"$ctl\" -$ST::WAVFILE_TYPE yes " .
	 "-di \"$ST::WAVFILES_DIR\" -ei \"$ST::CFG_WAVFILE_EXTENSION\" ".
	 "-do \"$ST::CFG_FEATFILES_DIR\" " .
	 "-eo \"$ST::CFG_FEATFILE_EXTENSION\"");
} else {
  system("bin/wave2feat @ARGV");
}

__END__

=head1 NAME

make_feats.pl - creates feature files (cepstra) from wave files

=head1 SYNOPSIS

=over 4

=item make_feats.pl -help 

For full list of arguments

=item  make_feats.pl [-cfg <cfg file>] -ctl <control file>

For processing the files in the control file with default arguments

=item make_feats.pl <program arguments>

If specifying all arguments

=back

=head1 ARGUMENTS

Obtained by running "bin/wave2feat" without arguments.

The recommended parameters for wideband (default) and narrowband (in parentheses) are:

=over 4

=item B<-alpha>  0.97

=item B<-srate>  16000 (8000)

=item B<-frate>  100

=item B<-wlen>   0.0256

=item B<-nfft>   512

=item B<-nfilt>  40 (31)

=item B<-lowerf> 130

=item B<-upperf> 6800 (3500)

=item B<-ncep>   13

=back

=cut
