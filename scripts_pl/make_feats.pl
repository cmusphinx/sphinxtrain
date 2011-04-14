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
use File::Path;

use lib catdir(dirname($0), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;


if ($#ARGV == -1) {
  pod2usage(2);
}

Getopt::Long::Configure('no_auto_abbrev', 'pass_through');

GetOptions('help|h' => \$help,
	   'ctl|c=s' => \$ctl,
	   'wavext|w=s' => \$wavext,
	   'wavtype|t=s' => \$wavtype,
	   'ext|e=s' => \$cepext,
	   'cfg=s' => \$cfg_file,
	   'param=s' => \$param_file);

if ($ARGV[0] == '--') {
    shift @ARGV;
}

if ($help) {
  pod2usage( -exitval => "NOEXIT",
             -verbose => 1 );
  system("sphinx_fe");
  exit(-1);
}

if (!defined $cfg_file) {
  print ("-cfg not specified, using the default ./etc/sphinx_train.cfg\n");
  $cfg_file = "./etc/sphinx_train.cfg";
}

if (!defined $param_file) {
  print ("-param not specified, using the default ./etc/feat.params\n");
  $param_file = "./etc/feat.params";
}

if (! -s $param_file) {

  $params = <<"EOP";
-alpha 0.97
-dither yes
-doublebw no
-nfilt 40
-ncep 13
-lowerf 133.33334
-upperf 6855.4976
-nfft 512
-wlen 0.0256
EOP

  open PARAM, ">$param_file" or die "Failed to open param file $param_file for writing: $!";
  print PARAM $params;
  print PARAM <<"EOP";
-transform legacy
-feat __CFG_FEATURE__
-agc __CFG_AGC__
-cmn __CFG_CMN__
-varnorm __CFG_VARNORM__
EOP
 close PARAM;    
 
 $params =~ s/\n/ /gs;
} else {

  open PARAM, "<$param_file" or die "Failed to open param file $param_file for reading: $!";     
  my @params;
  while(<PARAM>) {
     chomp;
     push @params, $_ unless ($_ =~ /_CFG_/ or $_ =~ /transform/);
  }

  $params = join (" ", @params);
}

if (defined $ctl) {
  if (! -s "$cfg_file") {
    print ("unable to find default configuration file, use -cfg file.cfg or create etc/sphinx_train.cfg for default\n");
    exit -3;
  }

  require $cfg_file;

  $ST::CFG_WAVFILE_TYPE ||= 'nist';
  $ST::CFG_WAVFILES_DIR ||= 'wav';
  $ST::CFG_WAVFILE_EXTENSION ||= 'sph';
  $ST::CFG_FEATFILES_DIR ||= 'feat';
  $ST::CFG_FEATFILE_EXTENSION ||= 'mfc';

  # Override things from the command line
  $ST::CFG_WAVFILE_EXTENSION = $wavext if defined($wavext);
  $ST::CFG_WAVFILE_TYPE = $wavtype if defined($wavtype);
  $ST::CFG_FEATFILE_EXTENSION = $cepext if defined($cepext);

  # Read control file and create any necessary directories
  open CTL, "<$ctl" or die "Failed to open control file $ctl: $!";
  while (<CTL>) {
      chomp;
      my $dir = dirname($_);
      mkpath(catdir($ST::CFG_FEATFILES_DIR, $dir));
  }
  close CTL;

  system("sphinx_fe -verbose yes $params -c \"$ctl\" -$ST::CFG_WAVFILE_TYPE yes " .
	 "-di \"$ST::CFG_WAVFILES_DIR\" -ei \"$ST::CFG_WAVFILE_EXTENSION\" ".
	 "-do \"$ST::CFG_FEATFILES_DIR\" " .
	 "-eo \"$ST::CFG_FEATFILE_EXTENSION\"".
	 " @ARGV");

} else {
  system("sphinx_fe @ARGV");
  open PARAM, ">$param_file" or die "Failed to open param file $param_file for writing: $!";
  while (@ARGV) {
      ($k, $v) = splice @ARGV, 0, 2;
      print PARAM "$k $v\n";
  }
  close PARAM;
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

Obtained by running "sphinx_fe" without arguments.

The recommended parameters for wideband (default) and narrowband (in parentheses) are:

=over 4

=item B<-alpha>  0.97

=item B<-samprate>  16000 (8000)

=item B<-frate>  100

=item B<-wlen>   0.0256

=item B<-nfft>   512

=item B<-nfilt>  40 (31)

=item B<-lowerf> 130

=item B<-upperf> 6800 (3500)

=item B<-ncep>   13

=back

=cut
