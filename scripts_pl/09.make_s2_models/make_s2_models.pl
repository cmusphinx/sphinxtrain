#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 2000 Carnegie Mellon University.  All rights 
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
#*************************************************************************
#  Build SphinxII models from Sphinx3 semicontinous models
#*************************************************************************
#
#  Author: Alan W Black (awb@cs.cmu.edu)
#

use File::Path;

my $index = 0;
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    $index = 2;
} else {
    $cfg_file = "etc/sphinx_train.cfg";
}

if (! -s "$cfg_file") {
    print ("unable to find default configuration file, use -cfg file.cfg or create etc/sphinx_train.cfg for default\n");
    exit -3;
}
require $cfg_file;

# If we created continuous models, we can't create S2 models from
# them. Sorry, we have to rerun.
if  ($CFG_HMM_TYPE eq ".cont.") {
  &ST_Log ("MODULE: 09 Convert to Sphinx2 format models\n");
  &ST_Log("    Can not create Sphinx-II models from continuous models.\n");
  &ST_Log("    Please rerun with CFG_HMM_TYPE = '.semi.'\n");
  exit(0);
}

my $scriptdir = "scripts_pl/09.make_s2_models";
my $logdir = "$CFG_LOG_DIR/09.make_s2_models";

$| = 1; # Turn on autoflushing
&ST_Log ("MODULE: 09 Convert to Sphinx2 format models\n");
&ST_Log ("    Cleaning up old log files...\n");
rmtree ($logdir) unless ! -d $logdir;
mkdir ($logdir,0777);

my $logfile_cb = "$logdir/${CFG_EXPTNAME}.mk_s2cb.log";
my $logfile_chmm = "$logdir/${CFG_EXPTNAME}.mk_s2chmm.log";
my $logfile_senone = "$logdir/${CFG_EXPTNAME}.mk_s2sendump.log";
my $logfile_s2phonemap = "$logdir/${CFG_EXPTNAME}.mk_s2phonemap.log";

$s3mdef = "$CFG_BASE_DIR/model_architecture/$CFG_EXPTNAME.$CFG_N_TIED_STATES.mdef";
$s2dir = "$CFG_BASE_DIR/model_parameters/$CFG_EXPTNAME.s2models";
mkdir ($s2dir,0777) unless -d $s2dir;

$s3hmmdir="$CFG_BASE_DIR/model_parameters/$CFG_EXPTNAME.cd_${CFG_DIRLABEL}_$CFG_N_TIED_STATES"."_delinterp";
$s3mixw = "$s3hmmdir/mixture_weights";
$s3mean = "$s3hmmdir/means";
$s3var = "$s3hmmdir/variances";
$s3tmat = "$s3hmmdir/transition_matrices";

&ST_Log ("    Make codebooks\n");
&ST_HTML_Print ("\t<A HREF=\"$logfile_cb\">Log File</A> ");
system("$CFG_BIN_DIR/mk_s2cb -meanfn $s3mean -varfn $s3var -cbdir $s2dir -varfloor 0.00001 >$logfile_cb 2>&1 ");
&ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> completed </font>\n");

&ST_Log ("    Make chmm files\n");
&ST_HTML_Print ("\t<A HREF=\"$logfile_chmm\">Log File</A> ");
system("$CFG_BIN_DIR/mk_s2hmm -moddeffn $s3mdef -mixwfn $s3mixw -tmatfn $s3tmat -hmmdir $s2dir >$logfile_chmm 2>&1");
&ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> completed </font>\n");

&ST_Log ("    Make senone file\n");
&ST_HTML_Print ("\t<A HREF=\"$logfile_senone\">Log File</A> ");
system( "$CFG_BIN_DIR/mk_s2sendump -moddeffn $s3mdef -mixwfn $s3mixw -tpfloor 0.0000001 -feattype s2_4x -sendumpfn $s2dir/sendump >$logfile_senone 2>&1");
&ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> completed </font>\n");

&ST_Log ("    Make phone and map files\n");
&ST_HTML_Print ("\t<A HREF=\"$logfile_s2phonemap\">Log File</A> ");
system("$CFG_BIN_DIR/mk_s2phonemap -moddeffn $s3mdef -phonefn $s2dir/phone -mapfn $s2dir/map >$logfile_s2phonemap 2>&1");
&ST_HTML_Print ("\t<font color=\"$CFG_OKAY_COLOR\"> completed </font>\n");

exit 0;
