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


# RAH Force passage of config file, or look for it in the current directory
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    die "-cfg specified, but unable to find file $ARGV[1]" unless (-s $cfg_file);
    require $cfg_file;
} else {
    $cfg_file = "./sphinx_train.cfg";
    die "Must specify -cfg or create default file ./sphinx_train.cfg" unless (-s  $cfg_file);
    require ("./sphinx_train.cfg");
    &ST_LogWarning ("-cfg not specified, using the default ./sphinx_train.cfg");
}


#*****************************************************************************
# The agg_seg script aggregates all the training feature vectors into a 
# single dump file and the kmeans script uses the contents of this dump
# file to compute the vq centroids in the vector space 
#*****************************************************************************

#Clean up from previous runs
system "rm -rf $CFG_VQ_LOG_DIR/*" if -d "$CFG_VQ_LOG_DIR";

# No error checking
&ST_Log ("MODULE: 01 Vector Quantization\n");

system ("$CFG_VQ_PERL_DIR/agg_seg.pl -cfg $cfg_file");
system ("$CFG_VQ_PERL_DIR/kmeans.pl -cfg $cfg_file");
