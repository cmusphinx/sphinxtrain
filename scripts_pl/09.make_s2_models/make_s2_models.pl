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
#*************************************************************************
#  Build SphinxII models from Sphinx3 semicontinous models
#*************************************************************************
#
#  Author: Alan W Black (awb@cs.cmu.edu)
#

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

my $scriptdir = "scripts_pl/09.make_s2_models";
my $logdir = "$CFG_LOG_DIR/09.make_s2_models";
mkdir ($logdir,0777) unless -d $logdir;

$| = 1; # Turn on autoflushing
&ST_Log ("MODULE: 09 Convert to Sphinx2 format models\n");
&ST_Log ("    Cleaning up old log files...\n");
system ("/bin/rm -f $logdir/*");

my $logfile_cb = "$logdir/${CFG_EXPTNAME}.mk_s2cb.log";
my $logfile_chmm = "$logdir/${CFG_EXPTNAME}.mk_s2chmm.log";
my $logfile_senone = "$logdir/${CFG_EXPTNAME}.mk_s2seno.log";
my $logfile_s2phone = "$logdir/${CFG_EXPTNAME}.mk_s2phone.log";

$s3mdef = "$CFG_BASE_DIR/model_architecture/$CFG_EXPTNAME.$CFG_N_TIED_STATES.mdef";
$s2dir = "$CFG_BASE_DIR/model_parameters/s2models";
mkdir ($s2dir,0777) unless -d $s2dir;

$s3hmmdir="$CFG_BASE_DIR/model_parameters/$CFG_EXPTNAME.cd_semi_$CFG_N_TIED_STATES"."_delinterp";
$s3mixw = "$s3hmmdir/mixture_weights";
$s3mean = "$s3hmmdir/means";
$s3var = "$s3hmmdir/variances";
$s3tmat = "$s3hmmdir/transition_matrices";

&ST_Log ("    Make codebooks\n");
system("$CFG_BIN_DIR/mk_s2cb -meanfn $s3mean -varfn $s3var -cbdir $s2dir -varfloor 0.00001 >$logfile_cb 2>&1 ");
&ST_Log ("    Make chmm files\n");
system("$CFG_BIN_DIR/mk_s2hmm -moddeffn $s3mdef -mixwfn $s3mixw -tmatfn $s3tmat -hmmdir $s2dir >$logfile_chmm 2>&1");
&ST_Log ("    Make senome file\n");
system( "$CFG_BIN_DIR/mk_s2seno -moddeffn $s3mdef -mixwfn $s3mixw -tpfloor 0.0000001 -hmmdir $s2dir >$logfile_senone 2>&1");
#system( "rm $s2dir/*.ccode $s2dir/*.d2code $s2dir/*.p3code $s2dir/*.xcode");
exit
&ST_Log ("    Make phone and map files\n");
$tmpf="tmp.phones";
system ( "grep -v \"^#\" $s3mdef | awk 'NF==12 {print \$1,\$2,\$3,\$4}' >$tmpf\n");
system("$CFG_BIN_DIR/mk_s2phone -s2phonefn $s2dir/phone -phonelstfn $tmpf >$logfile 2>&1");

system("rm -f $tmpf");

exit 0;
