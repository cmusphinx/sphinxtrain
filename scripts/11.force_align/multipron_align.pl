#!/usr/bin/env perl
## Run multipron_align.py using paths from sphinx_train.cfg (project base = $CFG_BASE_DIR).
##
## Usage (from training project directory, same as sphinxtrain run):
##   perl path/to/scripts/11.force_align/multipron_align.pl [--dry-run] [--first-n N] [--binary PATH]
##
## See $CFG_MULTIPRON in etc/sphinx_train.cfg (no disables multipron; stage 21 runs this after CI by default).

use strict;
use warnings;
use File::Basename qw(dirname);
use File::Spec::Functions qw(catfile updir);

use lib catfile(dirname($0), updir(), 'lib');
use SphinxTrain::Config;

my @args = @ARGV;
my $dry = (@args && $args[0] eq '--dry-run') ? shift @args : 0;

my $etc = catfile($ST::CFG_BASE_DIR, 'etc');
if (!-d $etc) {
    die "Missing directory $etc (run from project after setup, or fix \$CFG_BASE_DIR)\n";
}

my $py = catfile($ST::CFG_SPHINXTRAIN_DIR, 'scripts', '11.force_align', 'multipron_align.py');
if (!-f $py) {
    die "Missing $py\n";
}

# Prefer $PYTHON; else python3 on Unix (many images have no `python` symlink); else python.
my $pyexe = $ENV{PYTHON};
if (!defined($pyexe) || $pyexe eq '') {
    $pyexe = ($^O eq 'MSWin32') ? 'python' : 'python3';
}
my @py = ($pyexe, $py);
push @py, '--dry-run' if $dry;
push @py, $etc, @args;
exec @py;
die "exec failed: $!\n";
