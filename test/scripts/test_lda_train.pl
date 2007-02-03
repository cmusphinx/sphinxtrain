#!/usr/local/bin/perl

use strict;
use File::Copy;
require './scripts/testlib.pl';

chomp(my $host=`../config.guess | xargs ../config.sub`);
my $bindir="../bin.$host/";
my $resdir="res/";
my $exec_resdir="lda_train";
my $bin="$bindir$exec_resdir";
my $bin_printp="${bindir}printp";

my $cepdir="./res/feat/rm";
my $segdir="./res/stseg/rm";
my $mdef="./res/hmm/RM.ci.mdef";
my $lda="./lda.mat";
my $lda_out="./lda.out";
my $lda_match="./printp/test_printp_lda";

my $cmd="$bin ";
$cmd .= "-moddeffn $mdef ";
$cmd .= "-cepdir $cepdir ";
$cmd .= "-segdir $segdir ";
$cmd .= "-outfn $lda ";
$cmd .= "-agc none -cmn current ";
$cmd .= "-varnorm no -feat 1s_c_d_dd -ceplen 13 ";

test_help($bindir,$exec_resdir);
test_example($bindir,$exec_resdir);

my @params=(50,100) ;
foreach my $i (@params)
{
    my $ctlfn="./res/feat/rm/rm1_train.fileids.$i";
    my $matchfn="./$exec_resdir/test_lda.$i.out";
    my $tmpcmd=$cmd;
    $tmpcmd .= "-ctlfn $ctlfn ";

    test_this("$tmpcmd",$exec_resdir,"DRY RUN TEST for $i utterances");
    test_this("$bin_printp -ldafn $lda > $lda_out ",
	      $exec_resdir,"printp LDA for $i utterances");
    if($i==100){
	compare_these_two($lda_out,$lda_match,$exec_resdir,
			  "LDA training for 100 utterance. Check duplicates with printp",0.002,1);
    }
    compare_these_two($lda_out,$matchfn,$exec_resdir,
		      "LDA training for $i utterances", 0.002, 1);
    copy("./lda.mat","./lda_train/lda.mat.$i");
}

unlink($lda,$lda_out);
