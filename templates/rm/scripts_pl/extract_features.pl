#!/usr/bin/perl -w
use strict;
use Getopt::Long;
use Pod::Usage;
use File::Spec::Functions qw(catdir catfile rel2abs);
use File::Basename;
use File::Temp qw(tempfile);
use File::Find;

use lib catdir(dirname($0), 'lib');
use SimpleConfig;

my %opts = (config => 'etc/rm1_files.cfg',
	    outdir => 'feat',
	    params => 'etc/feat.params',
	    outext => 'mfc');
GetOptions(\%opts,
	   'config=s',
	   'params|p=s',
	   'outdir|o=s',
	   'outext|e=s',
	   'inctl|c=s',
	   'help|h|?')
    or pod2usage(2);
pod2usage(0) if $opts{help};

my %dirs = parse_config($opts{config});

my %inctl;
if (defined($opts{inctl}) and open INCTL, "<$opts{inctl}") {
    while (<INCTL>) {
	chomp;
	my @fields = split;
	my $fileid = shift @fields;
	$fileid =~ tr,\\,/,;
	$inctl{$fileid}++;
    }
}

my @feat_params;
if (open PARAMS, "<$opts{params}") {
    while (<PARAMS>) {
	chomp;
	# Skip the templated parameters which are all dynamic feature related
	next if /__\w+__/;
	push @feat_params, split;
    }
}
if (@ARGV) {
    push @feat_params, @ARGV;
}

my @parts = qw(dep ind dep_trn ind_trn);
foreach my $dir (@parts) {
    my $indir = $dirs{$dir};
    my $outdir = rel2abs(catdir($opts{outdir}, $dir));
    mkdir($opts{outdir}, 0777);
    mkdir($outdir, 0777);
    my (@ucfiles, @lcfiles);
    find(sub {
	     my $relpath = $File::Find::name;
	     substr($relpath, 0, length($indir)) = "";
	     $relpath =~ s/^\/+//;

	     if (-f $File::Find::name and $File::Find::name =~ /\.wav/) {
		 $relpath =~ s/\.wav//;
		 push @lcfiles, $relpath;
	     }
	     elsif (-f $File::Find::name and $File::Find::name =~ /\.WAV/) {
		 $relpath =~ s/\.WAV//;
		 push @ucfiles, $relpath;
	     }
	     elsif (-d $File::Find::name and !/^\.\.?$/) {
		 mkdir(catdir($outdir, lc($relpath)), 0777);
	     }
	 }, $indir);

    my ($fh, $filename) = tempfile();
    foreach (@lcfiles) {
	print $fh "$_\n" unless %inctl and not exists $inctl{"$dir/$_"};
    }
    close($fh);
    my $rv = system('sphinx_fe',
		    -di => $indir, -do => $outdir,
		    -ei => 'wav', -eo => $opts{outext},
		    @feat_params,
		    -c => $filename, -nist => 'yes');
    unlink($filename);
    die "sphinx_fe failed: $rv" unless $rv == 0;
    foreach (@ucfiles) {
	next if %inctl and not exists $inctl{"$dir/" . lc($_)};
	my $infile = catfile($indir, $_) . '.WAV';
	my $outfile = catfile($outdir, lc($_)) . '.' . $opts{outext};
	$rv = system('sphinx_fe',
		     -i => $infile, -o => $outfile,
		     @feat_params,
		     -nist => 'yes');
	die "sphinx_fe failed: $rv" unless $rv == 0;
    }
}

1;
__END__

=head1 NAME

extract_features.pl - Create acoustic feature files for RM1

=head1 SYNOPSIS

extract_features.pl
     S<[ I<--config> B<etc/rm1_files.cfg> ]>
     S<[ I<--params> B<etc/feat.params> ]>
     S<[ I<-o> B</path/to/training/feat> ]>
     S<[ I<-e> I<EXTENSION> ]>
     S<[ I<-c> I<CONTROL-FILE> ]>

=head1 DESCRIPTION

This script creates feature files for all waveforms in the RM1 SI and
SD training and test sets.  Filenames are all converted to lowercase.

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut

