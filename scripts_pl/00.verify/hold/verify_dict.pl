#!/usr/opt/PERL5004/bin/perl -w
## ====================================================================
##
## Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
## reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
##
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer. 
##
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in
##    the documentation and/or other materials provided with the
##    distribution.
##
## 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
##    endorse or promote products derived from this software without
##    prior written permission. To obtain permission, contact 
##    sphinx@cs.cmu.edu.
##
## 4. Products derived from this software may not be called "Sphinx"
##    nor may "Sphinx" appear in their names without prior written
##    permission of Carnegie Mellon University. To obtain permission,
##    contact sphinx@cs.cmu.edu.
##
## 5. Redistributions of any form whatsoever must retain the following
##    acknowledgment:
##    "This product includes software developed by Carnegie
##    Mellon University (http://www.speech.cs.cmu.edu/)."
##
## THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
## ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
## THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
## PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
## NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
## ====================================================================
##
## Author: Ricky Houghton (converted from scripts by Rita Singh)
##


require "/sphx_train/swb1/scripts_pl/sphinx_train.cfg";

open DICT,"$CFG_DICTIONARY" or die "Can not open the dictionary ($CFG_DICTIONARY)";

%dict_phone_hash = ();


# PHASE 1: Check to see if the phones in the dictionary are listed in the phonelist file
# PHASE 2: Check to make sure there are not duplicate entries in the dictionary

print "Phase 1: Checking to see if the dictionary agrees with the phonelist file\n";

# This is rather ugly, but it's late and I'm just trying to get the pieces together
# Clean it up later

$dict_hash = ();

# Read the dictionary and stick phones into dict_phone_hash
$counter =0;
while (<DICT>) {
    if (/^(\S+)\s(.*)$/) {
	$dict_hash{lc($1)}++;
	$tmp = $2;
	$tmp =~ s/\t/ /g;
	$tmp =~ s/  / /g;
	@phones = split / /,$tmp;
	for $phone (@phones) {
	    $phone =~ s/  / /g;
	    $dict_phone_hash{uc($phone)}++ if $phone;
	}
    }
    $counter++;
}
close DICT;

# Read the phonelist and stick phones into phonelist_hash
open PHONE,"$CFG_RAWPHONEFILE";
while (<PHONE>) {
    chomp;
    $phonelist_hash{uc($_)}++;
}
close PHONE;

@keys = keys %dict_phone_hash;
print "\tFound $counter words using $#keys phones\n";

$status = 'passed';
for $key (sort (keys %dict_phone_hash)){
#    print "$key => $dict_phone_hash{$key}\n";
    if (! defined($phonelist_hash{$key})) {
	$status = 'FAILED';
	$ord = ord($key);
	warn "This phone ($key -> $ord) occurs in the dictionary ($CFG_DICTIONARY), but not in the phonelist ($CFG_RAWPHONEFILE";
    }
}

print "\t$status\n";



print "Phase 2: Checking to make sure there are not duplicate entries in the dictionary\n";
$duplicate_status = 'passed';
for $key (keys %dict_hash) {
    if ($dict_hash{$key} > 1) {
	$duplicate_status = 'FAILED';
	warn "This word ($key) has duplicate entries in ($CFG_DICTIONARY)";
    }
}

print "\t$duplicate_status\n";


exit (0) if (($status eq 'passed')|| ($duplicate_status eq 'passed'));
exit (-1);
