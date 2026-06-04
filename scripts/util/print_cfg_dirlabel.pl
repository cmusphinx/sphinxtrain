#!/usr/bin/env perl
## Print $ST::CFG_DIRLABEL after Config + resolved sync (for tests).
use strict;
use File::Basename;
use File::Spec::Functions qw(catdir updir);

BEGIN {
    @ARGV = ("-cfg", $ARGV[0]) if @ARGV == 1 && $ARGV[0] !~ /^-cfg$/;
}

use lib catdir(dirname($0), updir(), "lib");
use SphinxTrain::Config;

print "$SphinxTrain::Resolved::DOC->{variables}{CFG_DIRLABEL}\n";
