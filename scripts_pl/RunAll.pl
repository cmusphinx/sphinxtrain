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
##
## Author: Ricky Houghton
##

use strict;
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

# What pieces would you like to compute.

my@sample_steps =
    ("$ST::CFG_SCRIPT_DIR/00.verify/verify_all.pl",
     "$ST::CFG_SCRIPT_DIR/01.vector_quantize/slave.VQ.pl",
     "$ST::CFG_SCRIPT_DIR/02.ci_schmm/slave_convg.pl",
     "$ST::CFG_SCRIPT_DIR/03.makeuntiedmdef/make_untied_mdef.pl",
     "$ST::CFG_SCRIPT_DIR/04.cd_schmm_untied/slave_convg.pl",
     "$ST::CFG_SCRIPT_DIR/05.buildtrees/slave.treebuilder.pl",
     "$ST::CFG_SCRIPT_DIR/06.prunetree/slave.state-tie-er.pl",
     "$ST::CFG_SCRIPT_DIR/07.cd-schmm/slave_convg.pl",
     "$ST::CFG_SCRIPT_DIR/08.deleted-interpolation/deleted_interpolation.pl",
     "$ST::CFG_SCRIPT_DIR/09.make_s2_models/make_s2_models.pl",
    );


foreach my $step (@sample_steps) {
    my $ret_value = RunScript($step);
    die "Something failed: ($step)\n" if $ret_value;
}
