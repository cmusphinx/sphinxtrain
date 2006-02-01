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
require "$CFG_SCRIPT_DIR/util/utils.pl";

my ($AGG_SEG,$len,$stride,$segdmpdir,$dumpfile,$logfile);

$| = 1; # Turn on autoflushing
&ST_Log ("    AggSeg ");

$AGG_SEG  = "$CFG_BIN_DIR/agg_seg";

#unlimit
#limit core 0k

#Compute VQ codebooks on no more than 1 million vectors for sanity
#That should be about 2500 files assuming about 10 seconds of speech per file
# 1000*10*100 = 1 mil

#Instead of calling wc let's open the file. (Note on WIN32, wc may not exist)
open CTL,"$CFG_LISTOFFILES";
$len =0;
while (<CTL>) {
    $len++;
}
close CTL;

$stride = 1 unless int($stride = $len/2500);

$logdir = "$CFG_LOG_DIR/01.vector_quantize";
mkdir ($logdir,0777) unless -d $logdir;

$segdmpdir = "$CFG_BASE_DIR/bwaccumdir/${CFG_EXPTNAME}_buff_1";
mkdir ($segdmpdir,0777) unless -d $segdmpdir;

$dumpfile = "$segdmpdir/${CFG_EXPTNAME}.dmp";
$logfile = "$logdir/${CFG_EXPTNAME}.vq.agg_seg.log";
&ST_HTML_Print ("\t" . &ST_FormatURL("$logfile", "Log File") . " ");

# run it here 
#system ("$AGG_SEG -segdmpdirs $segdmpdir -segdmpfn $dumpfile  -segtype all -ctlfn $CFG_LISTOFFILES -cepdir $CFG_FEATFILES_DIR -cepext $CFG_FEATFILE_EXTENSION -ceplen $CFG_VECTOR_LENGTH -agc $CFG_AGC -cmn $CFG_CMN -feat $CFG_FEATURE -stride $stride");

my $cmd = "\"$AGG_SEG\" -segdmpdirs \"$segdmpdir\" -segdmpfn \"$dumpfile\"  -segtype all -ctlfn \"$CFG_LISTOFFILES\" -cepdir \"$CFG_FEATFILES_DIR\" -cepext $CFG_FEATFILE_EXTENSION -ceplen $CFG_VECTOR_LENGTH -agc $CFG_AGC -cmn $CFG_CMN -feat $CFG_FEATURE -stride $stride";

exit (RunTool($cmd, $logfile, 0));
