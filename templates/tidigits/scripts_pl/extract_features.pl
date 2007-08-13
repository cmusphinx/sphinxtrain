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

my %opts = (config => 'etc/tidigits_files.cfg',
	    params => 'etc/feat16k.params',
	    outext => 'mfc',
	    outdir => 'feat');
GetOptions(\%opts,
	   'config=s',
	   'params|p=s',
	   'outdir|o=s',
	   'outext|e=s',
	   'help|h|?')
    or pod2usage(2);
pod2usage(0) if $opts{help};

my %dirs = parse_config($opts{config});

my @feat_params;
if (open PARAMS, "<$opts{params}") {
    while (<PARAMS>) {
	chomp;
	# Skip the templated parameters which are all dynamic feature related
	next if /__\w+__/;
	push @feat_params, split;
    }
}
close PARAMS;

foreach my $dir (qw(test train)) {
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
    foreach (@lcfiles) { print $fh "$_\n" }
    close($fh);
    my $rv = system('sphinx_fe',
		    -di => $indir, -do => $outdir,
		    -ei => 'wav', -eo => $opts{outext},
		    @feat_params,
		    -c => $filename, -nist => 'yes');
    unlink($filename);
    die "sphinx_fe failed: $rv" unless $rv == 0;
    foreach (@ucfiles) {
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

extract_features.pl - Create acoustic feature files for TIDIGITS

=head1 SYNOPSIS

create_features.pl
     S<[<I<--config> B<etc/rm1_files.cfg>]>
     S<[ I<-o> B</path/to/training/feat> ]>

=head1 DESCRIPTION

This script creates feature files for all waveforms in the TIDIGITS
training and test sets.  Filenames are all converted to lowercase.

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut

