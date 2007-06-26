#!/usr/bin/perl -w
use strict;
use Getopt::Long;
use Pod::Usage;
use File::Spec::Functions;
use File::Basename;

use lib catdir(dirname($0), 'lib');
use SimpleConfig;

my %opts = (config => 'etc/rm1_files.cfg', outdir => 'etc');
GetOptions(\%opts,
	   'config=s',
	   'outdir|o=s',
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

# Read basic set of sentences
my $sentfh = try_opening($dirs{doc}, 'al_sents.snr');
my %sents;
while (<$sentfh>) {
    next if /^;/;
    chomp;
    s/\r$//;
    my ($text, $uttid) = /^(.*)\s+\(([^\)]+)\)$/;
    $text =~ tr/+/\'/;
    $sents{$uttid} = $text;
}

# Create control files and transcripts
sub create_files {
    my ($outname, @fhs) = @_;

    local (*OUTLSN, *OUTCTL);
    open OUTLSN, ">$opts{outdir}/$outname.transcription"
	or die "Failed to open $opts{outdir}/$outname.transcription: $!";
    open OUTCTL, ">$opts{outdir}/$outname.fileids"
	or die "Failed to open $opts{outdir}/$outname.fileids: $!";

    local $_;
    foreach my $fh (@fhs) {
	while (<$fh>) {
	    chomp;
	    s/\r$//;
	    next if /^;/;
	    s,^/rm1/,,i;
	    s/\.wav$//;
	    my $uttid = basename($_);
	    print OUTLSN "<s> " . $sents{uc($uttid)} . " </s> ($uttid)\n";
	    print OUTCTL "$_\n";
	}
    }
}

create_files('rm_train',
	     try_opening($dirs{doc}, 'train', '72_indtr.ndx'));
create_files('rm_aug_si_train',
	     try_opening($dirs{doc}, 'train', '37_indtr.ndx'),
	     try_opening($dirs{doc}, 'train', '72_indtr.ndx'));
create_files('rm_sd_train',
	     try_opening($dirs{doc}, 'train', '6a_deptr.ndx'),
	     try_opening($dirs{doc}, 'train', '6b_deptr.ndx'));

create_files('rm_si_0387',
	     try_opening($dirs{doc}, 'tests', '1_mar87', '1_indtst.ndx'));
create_files('rm_sd_0387',
	     try_opening($dirs{doc}, 'tests', '1_mar87', '1_deptst.ndx'));

create_files('rm_si_1087',
	     try_opening($dirs{doc}, 'tests', '2_oct87', '2_indtst.ndx'));
create_files('rm_sd_1087',
	     try_opening($dirs{doc}, 'tests', '2_oct87', '2_deptst.ndx'));

create_files('rm_0688',
	     try_opening($dirs{doc}, 'tests', '3_jun88', '3_alltst.ndx'));

create_files('rm_si_0289',
	     try_opening($dirs{doc}, 'tests', '4_feb89', '4_indtst.ndx'));
create_files('rm_sd_0289',
	     try_opening($dirs{doc}, 'tests', '4_feb89', '4_deptst.ndx'));

create_files('rm_si_1089',
	     try_opening($dirs{doc}, 'tests', '5_oct89', '5_indtst.ndx'));
create_files('rm_sd_1089',
	     try_opening($dirs{doc}, 'tests', '5_oct89', '5_deptst.ndx'));

1;
__END__

=head1 NAME

create_transcripts.pl - Create training and testing transcripts for RM

=head1 SYNOPSIS

create_transcripts.pl
     S<[<I<--config> B<etc/rm1_files.cfg>]>
     S<[ I<-o> B</path/to/training/etc> ]>

=head1 DESCRIPTION

This script creates transcripts and control files for:

=over 4

=item The 72-speaker "standard" training set (2880 utterances)

This will create the files etc/rm_train.fileids and etc/rm_train.transcription

=item The 109-speaker "augmented" training set (3990 utterances)

This will create the files etc/rm_aug_si_train.fileids and etc/rm_aug_si_train.transcription

=item The 12-speaker speaker-dependent training set (7200 utterances)

This will create the files etc/rm_sd_train.fileids and etc/rm_sd_train.transcription

=item The March '87 speaker-independent (SI) and speaker-dependent (SD) test sets

This will create the files etc/rm_si_0387.fileids, etc/rm_si_0387.transcription,
etc/rm_sd_0387.fileids, and etc/rm_sd_0387.transcription

=item The October '87 SI and SD test sets

This will create the files etc/rm_si_1087.fileids, etc/rm_si_1087.transcription,
etc/rm_sd_1087.fileids, and etc/rm_sd_1087.transcription

=item The June '88 combined SI/SD test set

This will create the files etc/rm_0688.fileids and etc/rm_0688.transcription

=item The February '89 SI and SD test sets

This will create the files etc/rm_si_0289.fileids, etc/rm_si_0289.transcription,
etc/rm_sd_0289.fileids, and etc/rm_sd_0289.transcription

=item The October '89 SI and SD test sets

This will create the files etc/rm_si_1089.fileids, etc/rm_si_1089.transcription,
etc/rm_sd_1089.fileids, and etc/rm_sd_1089.transcription

=bak

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut

