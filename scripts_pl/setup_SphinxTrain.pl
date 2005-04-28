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
#  Set up script
#
# ====================================================================

use strict;
use File::Copy;
use Cwd;
use Getopt::Long;
use Pod::Usage;
use File::stat;

if ($#ARGV == -1) {
  pod2usage(2);
}

my ($SPHINXTRAINDIR,
    $DBNAME,
    $help,
    $force,
    $update);

my $LEAVE_MODE = 0;
my $UPDATE_MODE = 1;
my $FORCE_MODE = 2;

my $replace_mode = $LEAVE_MODE;

$SPHINXTRAINDIR = $0;

$SPHINXTRAINDIR =~ s/^(.*)[\/\\]scripts_pl[\\\/].*$/$1/i;

my $result = GetOptions('help|h' => \$help,
		     'force' => \$force,
		     'update' => \$update,
		     'sphinxtraindir|st=s' => \$SPHINXTRAINDIR,
		     'task=s' => \$DBNAME);

if (($result == 0) or (defined($help)) or (!defined($DBNAME))) {
  pod2usage( -verbose => 1 );
  exit(-1);
}

$replace_mode = $UPDATE_MODE if (defined $update);
$replace_mode = $FORCE_MODE if (defined $force);

# Check if the current directory - where the setup will be installed - is empty
opendir(DIR, ".") or die "Can't open current directory";
my @dirlist = grep !/^\./, readdir DIR;
closedir(DIR);

if ($#dirlist > 0) {
  print "Current directory not empty.\n";
  if ($replace_mode == $FORCE_MODE) {
    print "Will overwrite existing files.\n";
  } elsif ($replace_mode == $UPDATE_MODE) {
    print "Will overwrite existing files if they're older.\n";
  } else {
    print "Will leave existing files as they are, and copy non-existing files.\n";
  }
}

# Start building the directory structure
print "Making basic directory structure\n";
mkdir "bin" unless -d bin;
mkdir "etc" unless -d etc;
mkdir "feat" unless -e feat;
mkdir "wav" unless -e wav;

mkdir "logdir" unless -d logdir;
mkdir "bwaccumdir" unless -d bwaccumdir;
mkdir "model_parameters" unless -d model_parameters;
mkdir "model_architecture" unless -d model_architecture;

# have to find these somewhere
mkdir "gifs" unless -d gifs;
replace_file("$SPHINXTRAINDIR/etc/images/green-ball.gif",
	     "gifs/green-ball.gif",
	     $replace_mode);
replace_file("$SPHINXTRAINDIR/etc/images/red-ball.gif",
	     "gifs/red-ball.gif",
	     $replace_mode);

# Figure out the platform string definition
my $PLATFORM = "";
if (open (SYSDESC, "$SPHINXTRAINDIR/config/system.mak")) {
  while (<SYSDESC>) {
    next unless m/PLATFORM/;
    chomp;
    my @words = split;
    $PLATFORM = "." . $words[$#words];
  }
  close(SYSDESC);
  print "Platform: $PLATFORM\n";
} else {
  print "Platform: Win32\n";
  $PLATFORM = "";
}

# Copy all executables to the local bin directory. We first try the
# directory bin/Release, that is, release build in windows. Then we
# back off to the Debug build. If it still fails, then we're probably
# in linux/unix, and we use the platform info to find the executables
my $execdir;
if (opendir(DIR, "$SPHINXTRAINDIR/bin/Release")) {
  $execdir = "$SPHINXTRAINDIR/bin/Release";
} elsif (opendir(DIR, "$SPHINXTRAINDIR/bin/Debug")) {
  $execdir = "$SPHINXTRAINDIR/bin/Debug";
} elsif (opendir(DIR, "$SPHINXTRAINDIR/bin$PLATFORM")) {
  $execdir = "$SPHINXTRAINDIR/bin$PLATFORM";
} else {
  die "Can't open $SPHINXTRAINDIR/bin$PLATFORM";
}
print "Copying executables from $execdir\n";
@dirlist = grep !/^\./, readdir DIR;
closedir(DIR);
foreach my $executable (@dirlist) {
 replace_file("$execdir/$executable",
	      "bin/$executable",
	      $replace_mode);
}

# We copy the scripts from the scripts_pl directory directly.
mkdir "scripts_pl" unless -d scripts_pl;
my $scriptdir = "$SPHINXTRAINDIR/scripts_pl";
print "Copying scripts from $scriptdir\n";
@dirlist = grep /^(0.*|mc)$/, readdir DIR;
closedir(DIR);

push @dirlist, ".";

# Copy the directory tree. We do so by creating each directory, and
# then copying it to the correct location here. We also set the permissions.
foreach my $directory (@dirlist) {
  mkdir "scripts_pl/$directory" unless -d "scripts_pl/$directory";
  opendir(SUBDIR, "$scriptdir/$directory") or 
    die "Can't open subdir $directory\n";
  my @subdirlist = grep /\.pl$/, readdir SUBDIR;
  closedir(SUBDIR);
  foreach my $executable (@subdirlist) {
    replace_file("$scriptdir/$directory/$executable",
		 "scripts_pl/$directory/$executable",
		 $replace_mode);
    chmod 0755, "scripts_pl/$directory/$executable";
  }
}

# We now copy additional files
replace_file("$SPHINXTRAINDIR/scripts_pl/maketopology.pl",
	     "bin/maketopology.pl",
	     $replace_mode);
replace_file("$SPHINXTRAINDIR/scripts_pl/make_feats.pl",
	     "bin/make_feats.pl",
	     $replace_mode);
replace_file("$SPHINXTRAINDIR/scripts_pl/make_dict",
	     "bin/make_dict",
	     $replace_mode);

# Set the permissions to executable;
opendir(DIR, "bin") or die "Can't open bin directory\n";
@dirlist = grep !/^\./, readdir DIR;
closedir(DIR);
@dirlist = map { "bin/$_" } @dirlist;
chmod 0755, @dirlist;

# Finally, we generate the config file for this specific task
print "Generating SphinxTrain specific scripts and config file\n";
open (CFGIN, "$SPHINXTRAINDIR/etc/sphinx_train.cfg") or 
  die "Can't open $SPHINXTRAINDIR/etc/sphinx_train.cfg\n";
open (CFGOUT, ">etc/sphinx_train.cfg") or die "Can't open etc/sphinx_train.cfg\n";
while (<CFGIN>) {
  chomp;
  s/___DB_NAME___/$DBNAME/g;
  my $currDir = cwd;
  s/___BASE_DIR___/$currDir/g;
  s/___SPHINXTRAIN_DIR___/$SPHINXTRAINDIR/g;
  print CFGOUT "$_\n";
}
close(CFGIN);
close(CFGOUT);

print "Set up for acoustic training for $DBNAME complete\n";

sub replace_file {
  my $source = shift;
  my $destination = shift;
  my $replace_mode = shift;

  if (($replace_mode == $FORCE_MODE) or (! -s $destination)) {
#    print "Replacing file $destination with $source\n";
    copy("$source", "$destination");
  } elsif ($replace_mode == $UPDATE_MODE) {
    my $source_time = stat($source);
    my $dest_time = stat($destination);
    if (($source_time->mtime) > ($dest_time->mtime)) {
      copy("$source", "$destination");
    }
  }
}

__END__

=head1 NAME

setup_SphinxTrain.pl - setup the SphinxTrain environment for a new task

=head1 SYNOPSIS

=over 4

=item To setup a new SphinxTrain task

Create the new directory (e.g., mkdir RM1)

Go to the new directory (e.g., cd RM1)

Run this script (e.g., perl $SPHINXTRAIN/scripts_pl/setup_SphinxTrain.pl RM1)

=item ./scripts_pl/setup_SphinxTrain.pl -help

For full list of arguments

=item  ./scripts_pl/setup_SphinxTrain.pl [-force] [-sphinxtraindir <SphinxTrain directory>] -task <task name>

For setting up the SphinxTrain environment, located at <SphinxTrain directory>, into current directory, naming the task <task name>

=back

=head1 ARGUMENTS

=over 4

=item B<-force>

Force the setup script to overwrite existing files. Optional.

=item B<-update>

Update existing files if they are older than in SphinxTrain. Optional.

=item B<-sphinxtraindir>

The location of the SphinxTrain suite. If not provided, same location as this script is assumed. Optional.

=item B<-task>

The name of the new task. Required.

=item B<-help>

The help screen (this screen). Optional.

=back

=cut
