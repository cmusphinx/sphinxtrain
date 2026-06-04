#!/usr/bin/env perl
## Write evaluated sphinx_train.cfg to sphinx_train.resolved.json (stdout or -out).
use strict;
use File::Basename;
use File::Spec::Functions qw(catdir updir);
use Getopt::Long;

my $cfg_file = "./etc/sphinx_train.cfg";
my $out_file = "";
GetOptions(
    "cfg=s" => \$cfg_file,
    "out=s" => \$out_file,
) or exit 2;

@ARGV = ("-cfg", $cfg_file);

use lib catdir(dirname($0), updir(), "lib");
use SphinxTrain::Config;
use SphinxTrain::Resolved;

if ($out_file ne "") {
    $SphinxTrain::Resolved::DOC =
        SphinxTrain::Resolved::write_file($out_file, $cfg_file);
} else {
    print SphinxTrain::Resolved::to_json($SphinxTrain::Resolved::DOC);
}
exit 0;
