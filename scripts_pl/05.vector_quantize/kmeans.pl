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
use File::Basename;
use File::Spec::Functions;
use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

#***************************************************************************
# Script to find the VQ codebooks for the training set.
# The codewords in the codebook will be used later as the means of
# clusters for the Discrete HMMs.
#***************************************************************************

#TODO - Finish usage
#print "Usage: $0\n";

my ($hmmdir,$outhmm,$segdmpdir,$dumpfile,$logfile);

$| = 1; # Turn on autoflushing

my $mllt_file = catfile($ST::CFG_MODEL_DIR, "${ST::CFG_EXPTNAME}.mllt");
my $model_type = 'ci';

# Definitions
$hmmdir = "$ST::CFG_BASE_DIR/model_parameters";
mkdir ($hmmdir,0777);

$outhmm  = "$hmmdir/${ST::CFG_EXPTNAME}.${model_type}_${ST::CFG_DIRLABEL}_flatinitial";
mkdir ($outhmm,0777);

$segdmpdir = "$ST::CFG_BWACCUM_DIR/${ST::CFG_EXPTNAME}_buff_1";
mkdir ($segdmpdir,0777);

$dumpfile = "$segdmpdir/${ST::CFG_EXPTNAME}.dmp";

my $logdir = "$ST::CFG_LOG_DIR/05.vector_quantize";
mkdir ($logdir,0777);
$logfile = "$logdir/${ST::CFG_EXPTNAME}.kmeans.log";

$| = 1;

my @feat_args;
if (defined($ST::CFG_SVSPEC)){
    @feat_args = (-svspec =>$ST::CFG_SVSPEC);
}
if (-r $mllt_file) {
    push(@feat_args,
	 -lda => $mllt_file,
	 -ldadim => $ST::CFG_LDA_DIMENSION);
}

exit RunTool('kmeans_init', $logfile, 0,
	     -gthobj => 'single',
	     -stride => 1,
	     -ntrial => 1,
	     -minratio => 0.001,
	     -ndensity => $ST::CFG_INITIAL_NUM_DENSITIES,
	     -meanfn => "$outhmm/means",
	     -varfn => "$outhmm/variances",
	     -fullvar => $ST::CFG_FULLVAR,
	     -reest => 'no',
	     -segdmpdirs => $segdmpdir,
	     -segdmpfn => $dumpfile,
	     -ceplen => $ST::CFG_VECTOR_LENGTH,
	     -feat => $ST::CFG_FEATURE,
	     @feat_args,
	     -agc => $ST::CFG_AGC,
	     -cmn => $ST::CFG_CMN,
	     -varnorm => $ST::CFG_VARNORM);
