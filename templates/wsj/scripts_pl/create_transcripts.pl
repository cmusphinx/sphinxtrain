#!/usr/bin/perl -w
use strict;
use Getopt::Long;
use Pod::Usage;
use File::Spec::Functions;
use File::Basename;
use File::Find;

use lib catdir(dirname($0), 'lib');
use SimpleConfig;

my %opts = (config => 'etc/wsj_files.cfg',
	    outdir => 'etc');
GetOptions(\%opts,
	   'config=s',
	   'outdir|o=s',
	   'help|h|?')
    or pod2usage(2);
pod2usage(0) if $opts{help};

my %dirs = parse_config($opts{config});

sub convert_dot {
    my ($file, $bdir, $index, $pattern) = @_;

    open DOT, "<$file" or die "Failed to open $file: $!";
    while (<DOT>) {
	chomp;
	s/\r$//; # Remove DOS newline residue
	s/\s*\(([^\)\(]+)\)$//; # Remove and save utterance ID
	my $uttid = $1;
	if (%$index) {
	    next unless exists $index->{$uttid};
	}
	elsif (defined($pattern)) {
	    # HACK for devel data, only use the closed vocab part
	    next unless $uttid =~ /$pattern/;
	}

	# Remove escaped things
	s/\\//g;

	# Convert filler words to Sphinx convention
	s/\[[<>\/]?([^<>\/\]]+)[<>\/]?\]/\+\+$1\+\+/g;

	# Remove useless characters
	tr/\*/ /;
	tr/<>/ /;
	tr/\`/\'/;

	# Upper case it
	$_ = uc $_;
	my @words;
	foreach (split) {
	    # Deal with partial words
	    s/^-\([^\)]+\)/-/;
	    s/\([^\)]+\)-/-/;
	    # Not sure where this came from
	    s/\+\+\/$/\+\+/;
	    # Sphinx3 (and ngram_pronounce) is stupid about
	    # underscores in filler words
	    if (/^\+\+.*\+\+$/) {
		tr/_-//d;
	    }
	    # Remove superfluous punctuation
	    next if $_ eq '.';
	    next if $_ eq '~';
	    next if $_ eq '~~';
	    next if $_ eq '-';
	    next if $_ eq "'";
	    next if $_ eq ",";

	    # Correct one silly little typo
	    $_ = "EXISTING" if $_ eq "EXISITING";

	    tr/://d unless $_ eq ':COLON';
	    tr/!//d unless $_ eq '!EXCLAMATION-POINT';
	    push @words, $_ unless $_ eq '.' or $_ eq '~';
	}

	print OUTLSN "<s> @words </s> ($uttid)\n";
	print OUTCTL "$bdir/$uttid\n";
    }
    close DOT;
}

sub convert_dots {
    my ($outbase, $basedir, $tag, $ndxbase, $ndxfile) = @_;

    my %index;
    my $pattern;
    if (defined($ndxbase) and defined($ndxfile)) {
	my $ndx = catdir($ndxbase, $ndxfile);
	if (! -e $ndx) {
	    $ndx = catdir($ndxbase, uc $ndxfile);
	    if (! -e $ndx) {
		die "Index file $ndxbase/$ndxfile not found!\n";
	    }
	}
	open INDEX, "<$ndx" or die "Failed to open $ndx: $!\n";
	while (<INDEX>) {
	    next if /^;/;
	    chomp;
	    s/\r$//;
	    my ($cd, $path) = split /:\s*/;
	    my $uttid = basename($path, '.wv1');
	    $index{$uttid}++;
	}
	close INDEX;
    }
    elsif (defined($ndxbase)) {
	$pattern = $ndxbase;
    }

    my @tags;
    if (ref($tag)) {
	@tags = @$tag;
    }
    else {
	@tags = $tag;
    }
    open OUTCTL, ">$outbase.fileids"
	or die "Failed to open $outbase.fileids: $!";
    open OUTLSN, ">$outbase.transcription"
	or die "Failed to open $outbase.transcription: $!";
    foreach my $tag (@tags) {
	my $dotdir = catdir($basedir, $tag);
	if (! -e $dotdir) {
	    $dotdir = catdir($basedir, uc $tag);
	    if (! -e $dotdir) {
		die "Transcript directory $basedir/$tag not found!\n";
	    }
	}
	find(sub {
		 return unless /\.dot/i;
		 my $bdir = $File::Find::dir;
		 substr($bdir, 0, length($dotdir) + 1) = "";
		 # The .dot file itself contains the file IDs
		 convert_dot($File::Find::name, "$tag/$bdir", \%index, $pattern);
	     }, $dotdir);
    }
    close OUTCTL;
    close OUTLSN;
}

convert_dots(catfile($opts{outdir}, "wsj_si84_train"),
	     $dirs{wsj0_transcrp}, 'si_tr_s',
	     $dirs{wsj0_doc}, catfile('indices', 'train', 'tr_s_wv1.ndx'));

convert_dots(catfile($opts{outdir}, "wsj_si200_train"),
	     $dirs{wsj1_trans}, 'si_tr_s',
	     $dirs{wsj1_doc}, catfile('indices', 'si_tr_s.ndx'));

convert_dots(catfile($opts{outdir}, "wsj_sd_train"),
	     $dirs{wsj0_transcrp}, 'sd_tr_s',
	     $dirs{wsj0_doc}, catfile('indices', 'train', 'tr_l_wv1.ndx'));

convert_dots(catfile($opts{outdir}, "wsj_sd_long_train"),
	     $dirs{wsj0_transcrp}, ['sd_tr_s', 'sd_tr_l'],
	     $dirs{wsj0_doc}, catfile('indices', 'train', 'tr_v_wv1.ndx'));

convert_dots(catfile($opts{outdir}, "wsj_1200_train"),
	     $dirs{wsj1_trans}, 'si_tr_l');

convert_dots(catfile($opts{outdir}, "wsj_j200_train"),
	     $dirs{wsj1_trans}, 'si_tr_j');

convert_dots(catfile($opts{outdir}, "wsj_jspon_train"),
	     $dirs{wsj1_trans}, 'si_tr_jd');

convert_dots(catfile($opts{outdir}, "wsj_5k_test"),
	     $dirs{wsj0}, 'si_et_05',
	     $dirs{wsj0_doc}, catfile('indices', 'test', 'nvp', 'si_et_05.ndx'));

convert_dots(catfile($opts{outdir}, "wsj_20k_test"),
	     $dirs{wsj0}, 'si_et_20',
	     $dirs{wsj0_doc}, catfile('indices', 'test', 'nvp', 'si_et_20.ndx'));

convert_dots(catfile($opts{outdir}, "wsj_5k_devel"),
	     $dirs{wsj0_transcrp}, 'si_dt_05', qr/...c..../);

convert_dots(catfile($opts{outdir}, "wsj_20k_devel"),
	     $dirs{wsj0_transcrp}, 'si_dt_20', qr/...c..../);

# Just concatenate SI-84 and SI-200 to get SI-284

1;
__END__

=head1 NAME

create_transcripts.pl - Create training and testing transcripts for WSJ

=head1 SYNOPSIS

create_transcripts.pl
     S<[<I<--config> B<etc/rm1_files.cfg>]>
     S<[ I<-o> B</path/to/training/etc> ]>

=head1 DESCRIPTION

This script creates transcripts and control files for:

=over 4

=item The 84-speaker SI-84 (speaker independent) training set (7236 utterances)

This will create the files etc/wsj_train.fileids and etc/wsj_train.transcription

=item The 200-speaker SI-200 training set (30278 utterances)

This will create the files etc/wsj_si200_train.fileids and etc/wsj_si200_train.transcription

=item The 8-speaker Speaker-Independent 5k Read NVP test set (330 utterances)

This will create the files etc/wsj_test.fileids and etc/wsj_test.transcription

=item The 8-speaker Speaker-Independent 20k Read NVP test set (333 utterances)

This will create the files etc/wsj_20k_test.fileids and etc/wsj_20k_test.transcription

=item The 10-speaker Speaker-Independent 5k Read NVP development set (410 utterances)

This will create the files etc/wsj_devel.fileids and etc/wsj_devel.transcription

=item The 10-speaker Speaker-Independent 20k Read NVP development set (403 utterances)

This will create the files etc/wsj_20k_devel.fileids and etc/wsj_20k_devel.transcription

=bak

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut

