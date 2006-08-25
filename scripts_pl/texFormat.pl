#!/usr/bin/perl 
use strict;


if (@ARGV < 5) {
    printf "$0 <rawcmdline> <texcmdline> <toolname> <helpstr> <examplestr>\n";
    exit(-1);
}

my ($rawcmdline,$texcmdline, $toolname, $helpstr, $examplestr) = @ARGV;


open(RAW,"$rawcmdline") ||die "Cannot open file $rawcmdline for reading\n";
my @rawlines = <RAW>;
close(RAW);


open(HELP,"$helpstr") || die "Cannot open file $helpstr for reading\n";
my @helplines = <HELP>;
close(HELP);

open(EXAMPLE,"$examplestr") || die "Cannot open file $examplestr for reading\n";
my @examplelines = <EXAMPLE>;
close(EXAMPLE);

open(TEX,"> $texcmdline") || die "Cannot open file $texcmdline for writing\n";

my $toolname =~ s/_/\\_/g;

#Tool description page
print TEX "\\subsubsection\{Tool Description\}\n";

my $line;
foreach $line (@helplines) {
    chomp($line);
    $line =~ s/_/\\_/g;
    $line =~ s/>/\$>\$/g;
    $line =~ s/</\$<\$/g;
    $line =~ s/\#/\\\#/g;

    if ($line !~ m/^.*Compiled on.*$/) {
	printf TEX "$line\n";   
    }  

}
#Tool example page
print TEX "\\subsubsection\{Example\}\n";

foreach $line (@examplelines) {
    chomp($line);
    $line =~ s/_/\\_/g;
    $line =~ s/>/\$>\$/g;
    $line =~ s/</\$<\$/g;
    $line =~ s/\#/\\\#/g;

    if ($line !~ m/^.*Compiled on.*$/) {
      printf TEX "$line\n";   
    }
}

print TEX "\\newpage\n";









# Commandl line argument page 
print TEX "\\subsubsection\{Command-line Argument Description\}\n";
print TEX "\{\\it Usage: $toolname [options]\}\n";
print TEX "\\begin\{description\}\n";



my $titleline = shift(@rawlines);
$titleline = shift(@rawlines) if ($titleline =~ /^.*Compiled on.*$/);
chomp($titleline);

my @chars;
@chars = split(//,$titleline);

my $pos = 0;
my $char;

my $description;
my @desc_words;
my $i;
my $opt_def;
my $wanted_pos;

foreach $char (@chars) {
    if ($char eq "s") {
	$wanted_pos = $pos;
    }
    $pos++;
}

# print ("$pos $wanted_pos\n");
foreach $line (@rawlines) {

#    print $line;
    chomp($line);

    next if ($line =~ /^.*Compiled on.*$/);
    $line =~ s/_/\\_/g;
    $line =~ s/>/\$>\$/g;
    $line =~ s/</\$<\$/g;
    $line =~ s/\#/\\#/g;

    @chars = split(//,$line);

    $opt_def = "";
    for ($i = 0; $i < $wanted_pos-3; $i++) {
	$opt_def = sprintf("%s%s",$opt_def,$chars[$i]) ;

    }
    $opt_def = sprintf("%s%s",$opt_def,"\n");
    
    my ($option, $default) = split(/\s+/,$opt_def);

#    print "$opt_def, $option, $default, ";

    $description = "";
    for ($i = $wanted_pos-3; $i < length($line); $i++) {
	$description = sprintf("%s%s",$description,$chars[$i]);
    }
    @desc_words = split(/\s+/,$description);
    $description = join(" ",@desc_words);

#    print "$description\n";
    
    if (! defined $default || $default eq "") {
	$default = "NONE";
    }
    print TEX "\\item\[$option\] $description (Default Value : $default) \n\n";
}
print TEX "\\end\{description\}\n";
print TEX "\\newpage\n";

close(TEX);

