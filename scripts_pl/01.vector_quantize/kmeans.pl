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

#***************************************************************************
# Script to find the VQ codebooks for the training set.
# The codewords in the codebook will be used later as the means of
# clusters for the Discrete HMMs.
#***************************************************************************

#TODO - Finish usage
#print "Usage: $0\n";

my ($hmmdir,$outhmm,$segdmpdir,$dumpfile,$logfile);

&ST_Log ("\tKMeans ");

# Definitions
$hmmdir = "$CFG_BASE_DIR/model_parameters";
mkdir ($hmmdir,0777) unless -d $hmmdir;

$outhmm  = "$hmmdir/${CFG_EXPTNAME}.ci_semi_flatinitial";
mkdir ($outhmm,0777) unless -d $outhmm;

$segdmpdir = "$CFG_BASE_DIR/bwaccumdir/${CFG_EXPTNAME}_buff_1";
mkdir ($segdmpdir,0777) unless -d $segdmpdir;

$dumpfile = "$segdmpdir/${CFG_EXPTNAME}.dmp";

mkdir ($CFG_VQ_LOG_DIR,0777) unless -d $CFG_VQ_LOG_DIR;
$logfile = "$CFG_VQ_LOG_DIR/${CFG_EXPTNAME}.kmeans.log";

&ST_HTML_Print ("\t<A HREF=\"$logfile\">Log File</A>");

#set VQ = ~rsingh/09..sphinx3code/trainer/bin.alpha/kmeans_init
#$VQ = "$CFG_BIN_DIR/kmeans_init";
# -grandvar   yes   

$| = 1;
if (open PIPE,"${CFG_BIN_DIR}/kmeans_init -gthobj single -stride 1 -ntrial 1 -minratio 0.001 -ndensity 256 -meanfn $outhmm/means -varfn $outhmm/variances -reest no -segdmpdirs $segdmpdir -segdmpfn   $dumpfile -ceplen     ${CFG_VECTOR_LENGTH} -feat ${CFG_FEATURE} -agc $CFG_AGC -cmn ${CFG_CMN} 2>&1 |") {
    
    open LOG,">$logfile";
    while (<PIPE>) {
	# Should be filtering for errors/warnings
	print LOG "$_";
    }
    close LOG;
#    &ST_Log ("\t\tcompleted\n");
    &ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> completed </font>\n");
    $| = 0;
    exit (0);
} 

$| = 0;
# system ("rm -f $dumpfile"); # RAH 7.21.2000 - this was part of the earlier code, it was commented out there as well.
&ST_HTML_Print ("\t\t<font color=\"$CFG_ERROR_COLOR\"> FAILED </font>\n");
#&ST_Log ("\t\tFAILED\n");
exit (-1);

