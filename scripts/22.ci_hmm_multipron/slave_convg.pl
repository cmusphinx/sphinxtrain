#!/usr/bin/env perl
## Optional second CI training pass after multipron (stage 21). When enabled, runs the
## same driver as stage 20; Baum-Welch uses GetLists(), which selects the multipron
## transcript once $CFG_BASE_DIR/multipron_align/$CFG_EXPTNAME.multipron.transcription
## exists. This re-runs flat initialization and replaces the CI model directory (same as
## a fresh stage 20). Set $CFG_CI_REESTIMATE_AFTER_MULTIPRON to 'no' (default) to skip.

use strict;
use warnings;
use File::Basename qw(dirname);
use File::Spec::Functions qw(catfile updir);

use lib catfile(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

$| = 1;
Log("MODULE: 22 Optional second CI pass (multipron supervision)\n");

if (!defined $ST::CFG_CI_REESTIMATE_AFTER_MULTIPRON
    || $ST::CFG_CI_REESTIMATE_AFTER_MULTIPRON ne 'yes') {
    Log("Skipped (set \$CFG_CI_REESTIMATE_AFTER_MULTIPRON to 'yes' to enable)\n");
    exit 0;
}
if (!ShouldUseMultipronTranscript()) {
    LogError(
        "CFG_CI_REESTIMATE_AFTER_MULTIPRON is yes but multipron transcript is not available. "
            . "Keep \$CFG_MULTIPRON enabled, complete stage 21, or set "
            . "CFG_CI_REESTIMATE_AFTER_MULTIPRON to no.\n"
    );
    exit 1;
}

my $target = catfile(dirname($0), updir(), '20.ci_hmm', 'slave_convg.pl');
Log("Invoking $target (same as stage 20; supervision = multipron transcript via GetLists)\n");
exec $^X, $target, @ARGV or die "exec $target: $!\n";
