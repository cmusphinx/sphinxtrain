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
	    extradict => 'etc/rm.acronyms.dic',
	    outdict => 'etc/rm.dic',
	    outvocab => 'etc/rm.vocab',
	    cmudict => catfile(dirname($0), updir, updir, updir,
			       'test', 'res', 'cmudict.0.6d'));
GetOptions(\%opts,
	   'config=s',
	   'outdict|o=s',
	   'outvocab|v=s',
	   'extradict|e=s',
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

# Read and process vocabulary list
my $lexfh = try_opening($dirs{doc}, 'lexicon.snr');
open VOCAB, ">$opts{outvocab}" or die "Failed to open $opts{outvocab}: $!";
while (<$lexfh>) {
    next if /^;/;
    chomp;
    s/\r$//;
    tr/+/\'/;
    print VOCAB "$_\n";
}
close VOCAB;

# Read and merge input dictionaries
sub read_dict {
    my ($dict, $file) = @_;
    local *DICT;
    open DICT, "<$file" or die "Failed to open $file: $!";
    while (<DICT>) {
	next if /^##/;
	chomp;
	s/\r$//;
	my ($word, $alt, $phones) = /^(\S+)(?:\((\d+)\))?\s+(.*)$/;
	if (defined($alt)) {
	    $dict->{$word} = [$dict->{$word}] if exists $dict->{$word};
	    push @{$dict->{$word}}, $phones;
	} else {
	    $dict->{$word} = $phones;
	}
    }
}

my %dict;
read_dict(\%dict, $opts{extradict});
read_dict(\%dict, $opts{cmudict});
my ($tmpfh, $tmpfile) = tempfile();
foreach my $word (sort keys %dict) {
    if (ref($dict{$word})) {
	my $i = 1;
	foreach my $pron (@{$dict{$word}}) {
	    if ($i == 1) {
		print $tmpfh "$word $dict{$word}\n";
	    }
	    else {
		print $tmpfh "$word($i) $dict{$word}\n";
	    }
	    ++$i;
	}
    }
    else {
	print $tmpfh "$word $dict{$word}\n";
    }
}
close($tmpfh);

my $outfile = catfile($opts{outdir}, 'rm.dic');
my $rv = system('ngram_pronounce',
		-v => $opts{outvocab},
		-i => $tmpfile,
		-o => $opts{outdict});
unlink($tmpfile);
die "ngram_pronounce failed: $rv $!" unless $rv == 0;

1;
__END__

=head1 NAME

=head1 SYNOPSIS

create_transcripts.pl
     S<[<I<--config> B<etc/rm1_files.cfg>]>
     S<[ I<-c> B</path/to/cmudict.0.6d> ]>
     S<[ I<-e> B</path/to/training/etc/rm.acronyms.dic> ]>
     S<[ I<-o> B</path/to/training/etc/rm.dic> ]>
     S<[ I<-v> B</path/to/training/etc/rm.vocab> ]>

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
