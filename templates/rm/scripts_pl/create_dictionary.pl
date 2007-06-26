#!/usr/bin/perl -w
use strict;
use Getopt::Long;
use Pod::Usage;
use File::Spec::Functions;
use File::Basename;
use File::Temp qw(tempfile);

use lib catdir(dirname($0), 'lib');
use SimpleConfig;

my %opts = (config => 'etc/rm1_files.cfg',
	    outfile => 'etc/rm.dic',
	    cmudict => catfile(dirname($0), updir, updir, updir,
			       'test', 'res', 'cmudict.0.6d'));
GetOptions(\%opts,
	   'config=s',
	   'outfile|o=s',
	   'cmudict|c=s',
	   'help|h|?')
    or pod2usage(2);
pod2usage(0) if $opts{help};

my %dirs = parse_config($opts{config});

sub try_opening {
    my ($dir, @paths) = @_;

    my $fh = IO::File->new(catfile($dir, @paths));
    unless (defined($fh)) {
	$fh = IO::File->new(catfile($dir, map(uc, @paths)));
    }
    unless (defined($fh)) {
	$fh = IO::File->new(catfile($dir, map(lc, @paths)));
    }
    unless (defined($fh)) {
	my $file = catfile($dir, @paths);
	die "Failed to open $file (case-insensitive): $!";
    }
    return $fh;
}

# Read vocabulary list
my $lexfh = try_opening($dirs{doc}, 'lexicon.snr');
my $outfile = catfile($opts{outdir}, 'rm.dic');
my ($tmpfh, $tmpfile) = tempfile();
while (<$lexfh>) {
    next if /^;/;
    chomp;
    s/\r$//;
    tr/+/\'/;
    print $tmpfh "$_\n";
}
close($tmpfh);
my $rv = system('ngram_pronounce',
		-v => $tmpfile,
		-i => $opts{cmudict},
		-o => $opts{outfile});
unlink($tmpfile);
die "ngram_pronounce failed: $rv $!" unless $rv == 0;

1;
__END__

=head1 NAME

=head1 SYNOPSIS

create_transcripts.pl
     S<[<I<--config> B<etc/rm1_files.cfg>]>
     S<[ I<-c> B</path/to/cmudict.0.6d> ]>
     S<[ I<-o> B</path/to/training/etc/rm.dic> ]>

=head1 DESCRIPTION

This script creates a pronunciation dictionary for the 991-word
Resource Management task.

In order for it to work you must have a copy of cmudict in Sphinx
format (this defaults to looking in ../../../test/res/cmudict.0.6d,
assuming that you are running this inside a SphinxTrain tree), and you
must also have the CMU language modeling toolkit version 3 in your
$PATH (this can be obtained from
L<https://cmusphinx.svn.sourceforge.net/svnroot/cmusphinx/trunk/cmuclmtk>)

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut
