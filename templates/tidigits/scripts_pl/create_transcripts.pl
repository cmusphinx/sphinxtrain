#!/usr/bin/perl -w
use strict;
use Getopt::Long;
use Pod::Usage;
use File::Spec::Functions;
use File::Basename;
use File::Find qw(find);

my %opts = (featdir => 'feat');
GetOptions(\%opts,
	   'featdir|o=s',
	   'help|h|?')
    or pod2usage(2);
pod2usage(0) if $opts{help};

my %digits = (
	      z => 'zero',
	      o => 'oh',
	      1 => 'one',
	      2 => 'two',
	      3 => 'three',
	      4 => 'four',
	      5 => 'five',
	      6 => 'six',
	      7 => 'seven',
	      8 => 'eight',
	      9 => 'nine'
	     );

sub name2trans {
    my $name = shift;
    $name =~ s/[ab]$//;
    return join " ", map $digits{$_}, split "", $name;
}

foreach my $dir (qw(test train)) {
    my $indir = catdir($opts{featdir}, $dir);
    open OUTTRANS, ">etc/tidigits_$dir.transcription\n"
	or die "Failed to open etc/tidigits_$dir.transcription: $!\n";
    open OUTCTL, ">etc/tidigits_$dir.fileids\n"
	or die "Failed to open etc/tidigits_$dir.fileids: $!\n";
    find(sub {
	     if (-f and /\.mfc$/) {
		 my $relpath = $File::Find::name;
		 # Strip off everything up to $dir and the extension
		 $relpath =~ s/^.*\/+$dir\//$dir\//;
		 $relpath =~ s/\.mfc$//;
		 print OUTCTL "$relpath\n";
		 my $uttid = basename($_, '.mfc');
		 print OUTTRANS "<s> ", name2trans($uttid), " </s> ($uttid)\n";
	     }
	 }, $indir);
    close OUTTRANS
	or die "Failed to close etc/tidigits_$dir.transcription: $!\n";
    close OUTCTL
	or die "Failed to open etc/tidigits_$dir.fileids: $!\n";
}
