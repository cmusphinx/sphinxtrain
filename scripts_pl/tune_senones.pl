#!/usr/bin/perl -w

use strict;
use Pod::Usage;
use File::Spec::Functions;
use File::Basename;
use lib catdir(dirname($0), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

pod2usage(1) unless @ARGV == 3;
my ($start, $end, $incr) = @ARGV;

my $orig_cfgfile = $ST::CFG_FILE;
my $new_cfgfile = catdir($ST::CFG_BASE_DIR, 'tune_senones.cfg');
my $orig_decode_cfgfile = catdir($ST::CFG_BASE_DIR, 'etc', 'sphinx_decode.cfg');
my $new_decode_cfgfile = catdir($ST::CFG_BASE_DIR, 'tune_senones_decode.cfg');

for (my $nsen = $start; $nsen <= $end; $nsen += $incr) {
    # Build a temporary decode configuration
    open CFGIN, "<$orig_decode_cfgfile" or die "Failed to open $orig_decode_cfgfile: $!";
    open CFGOUT, ">$new_decode_cfgfile" or die "Failed to open $new_decode_cfgfile: $!";
    while (<CFGIN>) {
	s/\$DEC_CFG_SPHINXTRAIN_CFG\s*=\s*[^;]+;/\$DEC_CFG_SPHINXTRAIN_CFG = \"$new_cfgfile\";/;
	print CFGOUT;
    }
    close CFGIN;
    close CFGOUT;
    # And a temporary training configuration
    open CFGIN, "<$orig_cfgfile" or die "Failed to open $orig_cfgfile: $!";
    open CFGOUT, ">$new_cfgfile" or die "Failed to open $new_cfgfile: $!";
    while (<CFGIN>) {
	s/\$CFG_N_TIED_STATES\s*=\s*[^;]+;/\$CFG_N_TIED_STATES = $nsen;/;
	print CFGOUT;
    }
    close CFGIN;
    close CFGOUT;

    my $rv;
    # We set this so that RunScript() will pass it on to sub-scripts
    $ST::CFG_FILE = $new_cfgfile;
    $rv = RunScript(catfile($ST::CFG_SCRIPT_DIR, '45.prunetree', 'slave.state-tying.pl'));
    die "prunetree and state tying failed: $rv" if $rv;
    $rv = RunScript(catfile($ST::CFG_SCRIPT_DIR, '50.cd_hmm_tied', 'slave_convg.pl'));
    die "cd_hmm_tied training failed: $rv" if $rv;

    $ST::CFG_FILE = $new_decode_cfgfile;
    $rv = RunScript(catfile($ST::CFG_SCRIPT_DIR, 'decode', 'slave.pl'));
    die "decode failed: $rv" if $rv;

    rename(catfile($ST::CFG_BASE_DIR, 'result', "$ST::CFG_EXPTNAME.match"),
	   catfile($ST::CFG_BASE_DIR, 'result', "$ST::CFG_EXPTNAME-$nsen.match"));
    rename(catfile($ST::CFG_BASE_DIR, 'result', "$ST::CFG_EXPTNAME.align"),
	   catfile($ST::CFG_BASE_DIR, 'result', "$ST::CFG_EXPTNAME-$nsen.align"));
}

unlink $new_cfgfile;
unlink $new_decode_cfgfile;

1;
__END__

=head1 NAME

tune_senones.pl

=head1 SYNOPSIS

tune_senones.pl I<start> I<end> I<increment>

=head1 DESCRIPTION

This script generates acoustic models with different numbers of tied
states in the range from I<start> to I<end> in steps of I<increment>,
and runs decoding tests on each of them.

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut
