
# -*- cperl -*-
## ====================================================================
##
## Copyright (c) 1996-2004 Carnegie Mellon University.  All rights 
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
## This work was supported in part by funding from the Defense Advanced 
## Research Projects Agency and the National Science Foundation of the 
## United States of America, and the CMU Sphinx Speech Consortium.
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

use strict;

sub test_help
{
    my ($path, $exec)=@_;    
    test_this("$path/$exec -help yes\n",$exec,"HELP TEST");
}

sub test_example
{
    my ($path, $exec)=@_;
    test_this("$path/$exec -example yes\n",$exec,"EXAMPLE TEST");
    return 0;
}

sub test_this
{
  my ($command,$exec,$testname,$signal)=@_;
  system("$command");
  my $sig;
  if(! defined $signal){
    $sig=0;
  }else{
    $sig=$signal;
  }
  if($?==$sig){
    printf("Test ${exec} ${testname} PASSED\n");
  }else{
    printf("Test ${exec} ${testname} FAILED, signal $? (not equal to $signal) , msg $!\n");
  }

}

sub compare_these_two
{
  my ($fn1,$fn2,$exec,${testname},$tolerance,$absval)=@_;

  if(! defined $tolerance){
    $tolerance = 0.002;
  }

  my $comparison = 0;

  my $line1 = "";
  my $line2 = "";
  if ((open (FN1, "<$fn1")) and (open (FN2, "<$fn2"))) {
    $comparison = 1;
    while (($line1 = <FN1>) . ($line2 = <FN2>)) {
      chomp($line1);
      chomp($line2);
      next if ($line1 eq $line2);
      my @field1 = split /[,\s]+/, $line1;
      my @field2 = split /[,\s]+/, $line2;
      # If the number of tokens in each line is different, the lines,
      # and therefore the files, don't match.
      if ($#field1 != $#field2) {
	$comparison = 0;
	last;
      }
      for (my $i = 0; $i <= $#field1; $i++) {
	if (($field1[$i] !~ m/^([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?$/) or
	    ($field2[$i] !~ m/^([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?$/)) {
	  # Check if any of the tokens in the line is a string rather
	  # than a number, and compare the strings
	  if ($field1[$i] ne $field2[$i]) {
	    $comparison = 0;
	    last;
	  }
	} else {
	  if ($absval) {
	    # Only consider the absolute values if $absval is true
	    $field1[$i] = abs($field1[$i]);
	    $field2[$i] = abs($field2[$i]);
	  }
	  if (abs($field1[$i] - $field2[$i]) > $tolerance) {
	    # If the tokens are both numbers, check if they match within
	    # a tolerance
	    $comparison = 0;
	    last;
	  }
	}
      }
      # If there was a mismatch, we can skip to the end of the loop
      last if ($comparison == 0);
    }
    # If the files don't have the same number of lines, one of the
    # lines will be EOF, and the other won't.
    $comparison = 0 if ($line1 != $line2);
  }
  
  close(FN1);
  close(FN2);

  if($comparison){
    printf("Test ${exec} ${testname} PASSED (comparing $fn1 and $fn2)\n");
  }else{
    printf("Test ${exec} ${testname} FAILED (comparing $fn1 and $fn2)\n");
  }
}
1;
