#!/usr/bin/perl
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
## 4. Redistributions of any form whatsoever must retain the following
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
## Author: Ricky Houghton 
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec;
use File::Path;

use lib File::Spec->catdir(dirname($0), File::Spec->updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

#*****************************************************************************
# The agg_seg script aggregates all the training feature vectors into a 
# single dump file and the kmeans script uses the contents of this dump
# file to compute the vq centroids in the vector space 
#*****************************************************************************

#Clean up from previous runs
my $logdir = "$ST::CFG_LOG_DIR/05.vector_quantize";

rmtree($logdir) unless ! -d $logdir;
mkdir ($logdir,0777);

$| = 1; # Turn on autoflushing
# No error checking
Log ("MODULE: 05 Vector Quantization\n");
my $return_value = 0;
if ($ST::CFG_HMM_TYPE eq ".cont.") {
  Log("Skipped for continuous models\n");
} else {
  $return_value = (RunScript('agg_seg.pl')
		   or RunScript('kmeans.pl'));
}
exit ($return_value != 0);

