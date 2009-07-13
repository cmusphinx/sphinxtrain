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

my %opts = (config => 'etc/wsj_files.cfg',
	    outdir => 'feat',
	    outext => 'mfc',
	    wavext => 'wv1',
	    wavtype => 'nist',
	    params => 'etc/feat16k.params',
	    parts => 'si84,devtest');
GetOptions(\%opts,
	   'config=s',
	   'parts=s',
	   'params|p=s',
	   'outdir|o=s',
	   'outext|e=s',
	   'inctl|c=s',
	   'wavext|w=s',
	   'wavtype|t=s',
	   'keep-audio',
	   'help|h|?')
    or pod2usage(2);
pod2usage(0) if $opts{help};

my %dirs = parse_config($opts{config});
my @parts = split /,/, $opts{parts};
my @dirs;
my %dirmap = (si84 => ['wsj0_si_tr_s'],
	      si200 => ['wsj1_si_tr_s'],
	      extra => ['wsj0_sd_tr_l',
			'wsj0_sd_tr_s',
			'wsj1_si_tr_j',
			'wsj1_si_tr_jd',
			'wsj1_si_tr_l'],
	      devtest => ['wsj0_si_dt_05',
			  'wsj0_si_dt_20',
			  'wsj0_si_et_05',
			  'wsj0_si_et_20'],
	     );

foreach my $p (@parts) {
    push @dirs, @{$dirmap{$p}};
}

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

my $fileext = $opts{wv2} ? 'wv2' : 'wv1';
foreach my $dir (@dirs) {
    my $indir = $dirs{$dir};
    my ($od) = ($dir =~ /^wsj[01]_(.*)/);
    my $outdir = rel2abs(catdir($opts{outdir}, $od));
    mkdir($opts{outdir}, 0777);
    mkdir($outdir, 0777);
    my @files;
    find(sub {
	     my $relpath = $File::Find::name;
	     substr($relpath, 0, length($indir)) = "";
	     $relpath =~ s/^\/+//;

	     if (-f $File::Find::name and $File::Find::name =~ /\.$fileext$/i) {
		 my $o = lc($relpath);
		 $o =~ s/\.$fileext$//;
		 push @files, $relpath
		     unless %inctl and not exists $inctl{"$od/$o"};
	     }
	     elsif (-d $File::Find::name and !/^\.\.?$/) {
		 mkdir(catdir($outdir, lc($relpath)), 0777);
	     }
	 }, $indir);
    # Do them in batches of 50 to save disk space
    while (@files) {
	my @l = splice @files, 0, 50;
	my ($tmpfh, $tmpfile) = tempfile();
	foreach my $p (@l) {
	    my $o = lc($p);
	    $o =~ s/\.$fileext$//;
	    my $rv = system("sph2pipe",
			    catfile($indir, $p),
			    catfile($outdir, "$o.sph"));
	    die "sph2pipe failed: $rv" unless $rv == 0;
	    print $tmpfh "$o\n";
	}
	# Optimized feature parameters (similar to HTK ones)
	my $rv = system('sphinx_fe',
			-di => $outdir, -do => $outdir,
			-ei => 'sph', -eo => $opts{outext},
			@feat_params,
			-c => $tmpfile, "-$opts{wavtype}" => 'yes');
	foreach my $p (@l) {
	    my $o = lc($p);
	    $o =~ s/\.wv1$//;
	    unlink(catfile($outdir, "$o.sph"))
		unless $opts{'keep-audio'};
	}
	unlink $tmpfile;
    }
}

1;
__END__

=head1 NAME

extract_features.pl - Create acoustic feature files for WSJ

=head1 SYNOPSIS

create_features.pl
     S<[ I<--config> B<etc/rm1_files.cfg> ]>
     S<[ I<--params> B<etc/feat16k.params> ]>
     S<[ I<--parts> B<si84,devtest> ]>
     S<[ I<-o> B</path/to/training/feat> ]>

=head1 DESCRIPTION

By default, this script creates feature files for all waveforms in the
WSJ SI-84 training set and the 5k and 20k development and testing
sets.  Only the Sennheiser microphone (.wv1) files are used.
Optionally, it will extract the SI-200 and extra training data as
well.

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut

