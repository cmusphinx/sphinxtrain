#!/usr/bin/perl
# ====================================================================
# Copyright (c) 2000 Carnegie Mellon University.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer. 
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# This work was supported in part by funding from the Defense Advanced 
# Research Projects Agency and the National Science Foundation of the 
# United States of America, and the CMU Sphinx Speech Consortium.
#
# THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
# ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
# NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ====================================================================
#
#  TeX formatting script
#
# ====================================================================
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

