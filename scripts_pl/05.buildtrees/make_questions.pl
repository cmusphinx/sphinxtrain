#!/usr/local/bin/perl5

my $index = 0;

# RAH Force passage of config file, or look for it one directory up.
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    if (! -s $cfg_file) {
	print "-cfg specified, but unable to find file $ARGV[1]\n";
	exit -3;
    }
    $index = 2;
    require $cfg_file;
} else {
    $cfg_file = "../sphinx_train.cfg";
    require $cfg_file;
    &ST_LogWarning("-cfg not specified, using the default ../sphinx_train.cfg");
}


my $mdeffn   = "${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.ci.mdef";
my $hmm_dir  = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.ci_semi";
my $meanfn   = "$hmm_dir/means";
my $varfn    = "$hmm_dir/variances";
my $mixwfn   = "$hmm_dir/mixture_weights";
my $tmp_str = time();
my $tempfn   = "${CFG_BASE_DIR}/tmp/questions.$tmp_str";
#my $questfn  = "${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.tree_questions";
my $questfn = ${CFG_QUESTION_SET};

my $logdir = "${CFG_LOG_DIR}/05.buildtrees";
mkdir ($logdir,0777) unless -d $logdir;
my $logfile = "$logdir/${CFG_EXPTNAME}.make_questions.log";

#$MAKE_QUEST = "~rsingh/09..sphinx3code/trainer/bin.alpha/make_quests";
my $MAKE_QUEST = "${CFG_BIN_DIR}/make_quests";

&ST_Log ("MODULE: 05 Build Trees\n");
&ST_Log ("\tMake Questions ");
&ST_HTML_Print ("<A HREF=\"$logfile\">Log File</A>\n");

$|=1;
if (open PIPE, "$MAKE_QUEST -moddeffn $mdeffn -meanfn $meanfn -varfn $varfn -mixwfn $mixwfn -npermute 8 -niter 0 -qstperstt 30 -tempfn $tempfn -questfn $questfn -type ${CFG_HMM_TYPE} 2>&1 |") {

    open LOG,">$logfile";
while (<PIPE>) {
    print LOG "$_";
}
close PIPE;
close LOG;
&ST_Log ("\tFinished\n");
$| = 0;
exit 0;
}


&ST_Log ("\tFailure\n");
exit -1
