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

use File::Copy;
use Cwd;

# Print help message if number of arguments is not right (2 or more)
if ($#ARGV < 1) {
  print "Usage: $0 [-force] <SphinxTrain location> <task>\n";
  print "where\n";
  print "\t'-force' forces the script to overwrite the current setup\n";
  print "\t<SphinxTrain location> is your local SphinxTrain directory\n";
  print "\t<task> is a short name for this training set\n";
  print "\nThe setup is built on the current working directory.\n";
  print "\n";
  print "e.g.:\n";
  print "\t $0 /usr0/robust/SphinxTrain mytask\n";
  print "\t $0 .. debug\n";
  exit;
}

$FORCE = shift;
if ($FORCE eq "-force") {
  $SPHINXTRAINDIR = shift;
} else {
  $SPHINXTRAINDIR = $FORCE;
  $FORCE = "";
}
$DBNAME = shift;

# Check if the current directory - where the setup will be installed - is empty
opendir(DIR, ".") or die "Can't open current directory";
@dirlist = grep !/^\./, readdir DIR;
closedir(DIR);

if ($#dirlist > 0) {
  print "Current directory not empty\n";
  if ($FORCE) {
    print "Will overwrite current setup\n";
  } else {
    die "Use the option '-force' if you wish to overwrite the current setup\n";
  }
}

# Start building the directory structure
print "Making basic directory structure\n";
mkdir bin unless -d bin;
mkdir etc unless -d etc;
mkdir feat unless -d feat;
mkdir wav unless -d wav;

mkdir logdir unless -d logdir;
mkdir bwaccumdir unless -d bwaccumdir;
mkdir model_parameters unless -d model_parameters;
mkdir model_architecture unless -d model_architecture;

# have to find these somewhere
mkdir gifs unless -d gifs;
copy("$SPHINXTRAINDIR/etc/images/green-ball.gif", "gifs/green-ball.gif");
copy("$SPHINXTRAINDIR/etc/images/red-ball.gif", "gifs/red-ball.gif");

# Figure out the platform string definition
if (open (SYSDESC, "$SPHINXTRAINDIR/config/system.mak")) {
  while (<SYSDESC>) {
    next unless m/PLATFORM/;
    chomp;
    @words = split;
    $PLATFORM = "." . $words[$#words];
  }
  close(SYSDESC);
  print "Platform: $PLATFORM\n";
} else {
  print "Platform: Win32\m";
  $PLATFORM = "";
}

# Copy all executables to the local bin directory. We first try the
# directory bin/Release, that is, release build in windows. Then we
# back off to the Debug build. If it still fails, then we're probably
# in linux/unix, and we use the platform info to find the executables
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
foreach $executable (@dirlist) {
 copy("$execdir/$executable", "bin/$executable");
}

# Likewise, we try to open the scripts dir under bin. If not present,
# we're backoff to copying from the main scripts dir. We just need to
# copy directories from scripts_pl that start with '0' or 'm'
mkdir scripts_pl unless -d scripts_pl;
if (opendir(DIR, "$SPHINXTRAINDIR/bin$PLATFORM/scripts_pl")) {
  $scriptdir = "$SPHINXTRAINDIR/bin$PLATFORM/scripts_pl";
} elsif (opendir(DIR, "$SPHINXTRAINDIR/scripts_pl")) {
  $scriptdir = "$SPHINXTRAINDIR/scripts_pl";
} else {
  die "Can't open $SPHINXTRAINDIR/bin$PLATFORM/scripts_pl";
}
print "Copying scripts from $scriptdir\n";
@dirlist = grep /^[0m]/, readdir DIR;
closedir(DIR);

# Copy the directory tree. We do so by creating each directory, and
# the copying it to the correct location here. We also set the permissions.
foreach $directory (@dirlist) {
  mkdir "scripts_pl/$directory" unless -d "scripts_pl/$directory";
  opendir(SUBDIR, "$scriptdir/$directory") or 
    die "Can't open subdir $directory\n";
  @subdirlist = grep !/^\./, readdir SUBDIR;
  foreach $executable (@subdirlist) {
    copy("$scriptdir/$directory/$executable", 
	 "scripts_pl/$directory/$executable");
    chmod 0755, scripts_pl/$directory/$executable;
  }
}

# We now copy additional files
copy("$SPHINXTRAINDIR/scripts_pl/maketopology.pl", "bin/maketopology.pl");
copy("$SPHINXTRAINDIR/scripts_pl/make_feats", "bin/make_feats");
copy("$SPHINXTRAINDIR/scripts_pl/make_dict", "bin/make_dict");

# Set the permissions to executable;
opendir(DIR, bin) or die "Can't open bin directory\n";
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
  $currDir = cwd;
  s/___BASE_DIR___/$currDir/g;
  s/___SPHINXTRAIN_DIR___/$SPHINXTRAINDIR/g;
  print CFGOUT "$_\n";
}
close(CFGIN);
close(CFGOUT);

print "Set up for acoustic training for $DBNAME complete";






