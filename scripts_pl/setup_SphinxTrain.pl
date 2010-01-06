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
use File::Find;
use File::Spec::Functions;

if ($#ARGV == -1) {
  pod2usage(2);
}

my ($SPHINXTRAINDIR,
    $DBNAME,
    $TEMPLATE,
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
			'template=s' => \$TEMPLATE,
			'task=s' => \$DBNAME);

$DBNAME ||= $TEMPLATE;
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

# Copy in a template if it exists
if ($TEMPLATE) {
    print "Copying template $TEMPLATE\n";
    replace_tree("$SPHINXTRAINDIR/templates/$TEMPLATE", ".", $replace_mode);
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
}

# Copy all executables to the local bin directory. We verify which
# directory from a list has the most recent file, and assume this is
# the last time the user compiled something. Therefore, this must be
# the directory the user cares about. We add bin/Release and bin/Debug
# to the list (that's where MS Visual C compiles files to), as well as
# any existing bin.platform

my @dir_candidates = ();
push @dir_candidates, "$SPHINXTRAINDIR/bin/Release";
push @dir_candidates, "$SPHINXTRAINDIR/bin/Debug";
push @dir_candidates, "$SPHINXTRAINDIR/bin$PLATFORM" if ($PLATFORM ne "");

my $execdir = executable_dir(@dir_candidates);

die "Couldn't find executables. Did you compile SphinxTrain?\n" if ($execdir eq "");

print "Copying executables from $execdir\n";

opendir(DIR, "$execdir") or die "Can't open $execdir\n";
if ($^O eq 'MSWin32' or $^O eq 'msys' or $^O eq 'cygwin') {
    @dirlist = grep /^[^.].*\.(dll|exe)$/i, readdir DIR;
}
else {
    @dirlist = grep /^[^.].*/, readdir DIR;
}
closedir(DIR);
foreach my $executable (@dirlist) {
 replace_file("$execdir/$executable",
	      "bin/$executable",
	      $replace_mode);
}

print "Copying scripts from $SPHINXTRAINDIR/scripts_pl\n";

# Copy the scripts from the scripts_pl directory
replace_tree("$SPHINXTRAINDIR/scripts_pl",
	     "scripts_pl", $replace_mode, qr/\.p[lm]$/);

# Copy additional files
replace_file("$SPHINXTRAINDIR/scripts_pl/maketopology.pl",
	     "bin/maketopology.pl",
	     $replace_mode);
replace_file("$SPHINXTRAINDIR/scripts_pl/make_feats.pl",
	     "bin/make_feats.pl",
	     $replace_mode);
replace_file("$SPHINXTRAINDIR/scripts_pl/make_dict",
	     "bin/make_dict",
	     $replace_mode);
replace_file("$SPHINXTRAINDIR/etc/feat.params",
	     "etc/feat.params",
	     $replace_mode);

# Copy the Python modules
replace_tree("$SPHINXTRAINDIR/python/build/lib",
	     "python", $replace_mode);

# Set the permissions to executable;
opendir(DIR, "bin") or die "Can't open bin directory\n";
@dirlist = grep !/^\./, readdir DIR;
closedir(DIR);
@dirlist = map { "bin/$_" } @dirlist;
chmod 0755, @dirlist;

# Finally, we generate the config file for this specific task, backing
# up any existing one, unless we are in leave-alone mode and it exists
# already (that is the default)
unless (-e "etc/sphinx_train.cfg" and $replace_mode == $LEAVE_MODE) {
  print "Generating SphinxTrain configuration file in etc/sphinx_train.cfg\n";
  if (-e "etc/sphinx_train.cfg") {
    print "Backing up existing configuration file to etc/sphinx_train.cfg.orig\n";
    unlink "etc/sphinx_train.cfg.orig";
    rename "etc/sphinx_train.cfg", "etc/sphinx_train.cfg.orig";
  }
  # Look for a config template in the target directory
  unless (open (CFGIN, "<etc/sphinx_train.template")) {
    open (CFGIN, "<$SPHINXTRAINDIR/etc/sphinx_train.cfg") or
      die "Can't open etc/sphinx_train.template or $SPHINXTRAINDIR/etc/sphinx_train.cfg\n";
  }
  open (CFGOUT, ">etc/sphinx_train.cfg") or die "Can't open etc/sphinx_train.cfg\n";
  while (<CFGIN>)
    {
      chomp;
      s/___DB_NAME___/$DBNAME/g;
      my $currDir = cwd;
      s/___BASE_DIR___/$currDir/g;
      s/___SPHINXTRAIN_DIR___/$SPHINXTRAINDIR/g;
      print CFGOUT "$_\n";
    }
  close(CFGIN);
  close(CFGOUT);
}

print "Set up for acoustic training for $DBNAME complete\n";

sub executable_dir {
  my @dirs = @_;
  my $return_dir = "";
  my $most_recent = 0;
  for my $dir (@dirs) {
    my $this_date = get_most_recent_date($dir);
    if ($this_date > $most_recent) {
      $most_recent = $this_date;
      $return_dir = $dir;
    }
  }
  return $return_dir;
}

sub get_most_recent_date {
  my $dir = shift;
  my $return_date = 0;
  if (opendir(DIR, "$dir")) {
    @dirlist = grep !/^\./, readdir DIR;
    closedir(DIR);
    for my $file (@dirlist) {
      my $this_date = stat("$dir/$file");
      if (($this_date->mtime) > ($return_date)) {
	$return_date = $this_date->mtime;
      }
    }
  }
  return $return_date;
}

sub replace_file {
  my $source = shift;
  my $destination = shift;
  my $replace_mode = shift;

  if (($replace_mode == $FORCE_MODE) or (! -e $destination)) {
    if (-e $destination) {
      print "Replacing file $destination with $source\n";
    }
    else {
      print "Installing $source to $destination\n";
    }
    copy("$source", "$destination");
  } elsif ($replace_mode == $UPDATE_MODE) {
    my $source_time = stat($source);
    my $dest_time = stat($destination);
    if (($source_time->mtime) > ($dest_time->mtime)) {
      print "Replacing file $destination with $source\n";
      copy("$source", "$destination");
    }
  }
}

sub replace_tree {
    my ($src, $dest, $mode, $pattern) = @_;

    print "$src => $dest\n";
    find({ wanted => sub {
	       my $sf = $File::Find::name;
	       my $df = $sf;
	       substr($df, 0, length($src)) = "";
	       $df = catfile($dest, $df);
	       return if ($sf =~ /~$/); # Skip Emacs tempfiles
	       return if ($sf =~ /\/\.svn/); # Skip .svn directories
	       if (-d $sf) {
		   mkdir $df, 0777;
	       }
	       else {
		   if (defined($pattern)) {
		       return unless $sf =~ /$pattern/;
		   }
		   replace_file($sf, $df, $mode);
		   chmod 0755, $df;
	       }
	   },
	   no_chdir => 1,
	 }, $src);
}

__END__

=head1 NAME

setup_SphinxTrain.pl - setup the SphinxTrain environment for a new task

=head1 SYNOPSIS

=over 4

=item To setup a new SphinxTrain task

Create the new directory (e.g., mkdir RM1)

Go to the new directory (e.g., cd RM1)

Run this script (e.g., perl $SPHINXTRAIN/scripts_pl/setup_SphinxTrain.pl -task RM1)

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
