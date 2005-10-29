#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
## Author: Evandro Gouvea
##
use File::Copy;
use Cwd;
use Getopt::Long;
use Pod::Usage;
use File::stat;

$| = 1;				# Turn on autoflushing

if ($#ARGV == -1) {
  pod2usage(2);
}

my ($cfg_file,
    $task_name,
    $help,
    $force,
    $update);

my $LEAVE_MODE = 0;
my $UPDATE_MODE = 1;
my $FORCE_MODE = 2;

my $replace_mode = $LEAVE_MODE;

my $result = GetOptions('help|h' => \$help,
			'force' => \$force,
			'update' => \$update,
			'cfg=s' => \$cfg_file,
			'task=s' => \$task_name);

if (($result == 0) or (defined($help)) or (!defined($task_name))) {
  pod2usage( -verbose => 1 );
  exit(-1);
}

if (!defined($cfg_file)) {
  if (-e "etc/sphinx_decode.cfg") {
    $cfg_file = "etc/sphinx_decode.cfg";
  } else {
    $cfg_file = "etc/sphinx_train.cfg";
  }
} else {
  pod2usage( -verbose => 1 );
  warn "unable to find default configuration file, use -cfg file.cfg or create etc/sphinx_decode.cfg for default\n";
  exit(-1);
}

require $cfg_file;

$replace_mode = $UPDATE_MODE if (defined $update);
$replace_mode = $FORCE_MODE if (defined $force);

$replace_option = "";

# Check if the directory where the setup will be installed is empty
if (-e "../$task_name") {
  opendir(DIR, "../$task_name") or die "Can't open current directory";
  my @dirlist = grep !/^\./, readdir DIR;
  closedir(DIR);

  if ($#dirlist > 0) {
    print "Current directory not empty.\n";
    if ($replace_mode == $FORCE_MODE) {
      print "Will overwrite existing files.\n";
      $replace_option = "-force";
    } elsif ($replace_mode == $UPDATE_MODE) {
      print "Will overwrite existing files if they're older.\n";
      $replace_option = "-update";
    } else {
      print "Will leave existing files as they are, and copy non-existing files.\n";
    }
  }
} else {
  mkdir "../$task_name";
}

chdir "../$task_name";

$ret_value = 0;

# Get the current task name from the directory name

# Setup the trainer
$SphinxTrain_dir = $CFG_SPHINXTRAIN_DIR;
system("perl \"$SphinxTrain_dir/scripts_pl/setup_SphinxTrain.pl\" " .
       "$replace_option -sphinxtraindir \"$SphinxTrain_dir\" " .
       "-task $task_name");

# Setup the decoder
$sphinx_decoder_dir = $DEC_CFG_SPHINXDECODER_DIR;
$sphinx_decoder_dir =~ m/.*[\\\/]([^\\\/]+)$/;
$sphinx_decoder_version = lc($1);

$langmodel = $DEC_CFG_LANGUAGEMODEL;
$langmodel =~ s/.*[\\\/]([^\\\/]+)$/$1/;
$langmodel =~ s/$CFG_DB_NAME/$task_name/;

system("perl " .
       "\"$sphinx_decoder_dir/scripts/setup_$sphinx_decoder_version.pl\" " .
       "$replace_option " .
       "-${sphinx_decoder_version}dir \"$sphinx_decoder_dir\" " .
       "-task $task_name " .
       "-langmod \"$langmodel\" " .
       "-langwt $DEC_CFG_LANGUAGEWEIGHT " .
       "-bw $DEC_CFG_BEAMWIDTH " .
       "-wbeam $DEC_CFG_WORDBEAM " .
       "-align \"$DEC_CFG_ALIGN\"");


# Copy the etc/* files, changing the task name appropriately
opendir(DIR, "$DEC_CFG_BASE_DIR/etc") 
  or die "Can't open directory \"$DEC_CFG_BASE_DIR/etc\"";
my @etc_dir_list = grep /^$CFG_DB_NAME/i, readdir DIR;
closedir(DIR);

foreach my $old_task_etc_file (@etc_dir_list) {
  my $new_task_etc_file = $old_task_etc_file;
  $new_task_etc_file =~ s/$CFG_DB_NAME/$task_name/gi;
  replace_file("$DEC_CFG_BASE_DIR/etc/$old_task_etc_file",
	       "etc/$new_task_etc_file", 
	       $replace_mode);
}

copy_dir("$DEC_CFG_BASE_DIR/wav", "wav", $replace_mode);

copy_dir("$DEC_CFG_BASE_DIR/feat", "feat", $replace_mode);

print "Set up for $task_name complete\n";

exit (0);

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

sub copy_dir() {
  my $source_dir = shift;
  my $dest_dir = shift;
  my $replace_mode = shift;
  mkdir $dest_dir unless (-e $dest_dir);
  opendir(DIR, "$source_dir") 
    or die "Can't open directory \"$source_dir\"";
  my @dir_list = grep !/^\./, readdir DIR;
  closedir(DIR);
  foreach my $dir (@dir_list) {
    $source ="$source_dir/$dir";
    if (-d $source) {
      copy_dir($source, "$dest_dir/$dir", $replace_mode);
    } else {
      replace_file($source, "$dest_dir/$dir", $replace_mode);
    }
  }
}


__END__

=head1 NAME

copy_setup.pl - copy the current directory setup to a new location, with new task name

=head1 SYNOPSIS

=over 4

=item To setup a new task

Go to the an existing task directory (e.g., cd RM1)

Run this script with the new task name (e.g., perl scripts_pl/copy_setup.pl -task myRM1)

=item ./scripts/copy_setup.pl -help

For full list of arguments

=item  ./scripts/copy_setup.pl [-force|-update] [-cfg <config file>] -task <task name>

For setting up the new task environment, from an existing task located in the current directory, into a parallel directory, naming the task <task name>

=back

=head1 ARGUMENTS

=over 4

=item B<-force>

Force the setup script to overwrite existing files. Optional.

=item B<-update>

Update existing files if they are older. Optional.

=item B<-cfg>

The location of the SphinxTrain configuration file. If not provided, the script uses "etc/sphinx_decode.cfg", if available, or "etc/sphinx_train.cfg". Optional.
=item B<-task>

The name of the new task. Required.

=item B<-help>

The help screen (this screen). Optional.

=back

=cut
