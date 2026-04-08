#!/usr/bin/env perl
## After CI HMM training, run multipron force alignment so later stages can use
## pronunciation-disambiguated transcripts. Skipped when $CFG_MULTIPRON is no.

use strict;
use warnings;
use File::Basename qw(dirname);
use File::Spec::Functions qw(catfile catdir updir);

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

$| = 1;
Log("MODULE: 21 Multipron alignment (after CI)\n");

if (!defined($ST::CFG_MULTIPRON) || $ST::CFG_MULTIPRON eq "no") {
    Log("Skipped (\$CFG_MULTIPRON unset or no)\n");
    exit 0;
}

my $pl = catfile($ST::CFG_SPHINXTRAIN_DIR, qw(scripts 11.force_align multipron_align.pl));
my $etc = catfile($ST::CFG_BASE_DIR, "etc");
unless (-f $pl) {
    LogError("Missing $pl\n");
    exit 1;
}
unless (-d $etc) {
    LogError("Missing directory $etc\n");
    exit 1;
}

my $logdir = "$ST::CFG_LOG_DIR/21.multipron_align";
mkdir($logdir, 0777) unless -d $logdir;
my $logfile = "$logdir/$ST::CFG_EXPTNAME.multipron_align_wrap.log";

Log("Running multipron_align.pl (requires CI models under \$CFG_MODEL_DIR)\n");
# Do not pass $etc on the command line: multipron_align.pl resolves etc/ from
# sphinx_train.cfg; an extra path is forwarded to multipron_align.py as a bogus arg.
my $rv = RunTool($^X, $logfile, 0, $pl);
if ($rv != 0) {
    my $sphinx_log = catfile(
        $ST::CFG_BASE_DIR, "multipron_align",
        "$ST::CFG_EXPTNAME.multipron_align.log"
    );
    LogError(
        "multipron_align exit $rv. Stderr from Python is in $logfile; "
        . "if sphinx3_align started, full command/output is in $sphinx_log. "
        . "Typical causes: sphinx3_align missing under \$CFG_BIN_DIR, "
        . "missing CI HMM dir under \$CFG_MODEL_DIR, or sphinx3_align nonzero.\n"
    );
}
exit($rv);
