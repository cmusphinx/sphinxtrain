#!/usr/opt/PERL5004/bin/perl -w

require "../sphinx_train.cfg";

#*************************************************************************
# The script builds a tree for ONE state of ONE basephone. We therefore
# need to call it in a loop to build trees for all the states of all the
# basephones
#*************************************************************************

die "usage: $0 <phone> <state>" if ($#ARGV != 1);

$phone = $ARGV[0];
$state = $ARGV[1];

$mdef_file       = "${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.untied.mdef";
$mixture_wt_file = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.cd_semi_untied/mixture_weights";
$unprunedtreedir = "${CFG_BASE_DIR}/trees/${CFG_EXPTNAME}.unpruned";
mkdir ($unprunedtreedir,0777) unless -d $unprunedtreedir;

$logdir = "${CFG_LOG_DIR}/05.buildtrees";
mkdir ($logdir,0777) unless -d $logdir;

$logfile = "$logdir/${CFG_EXPTNAME}.buildtree.${phone}.${state}.log";

#$mach = `~/51..tools/machine_type.csh`;
#$BUILDTREE = "/net/alf19/usr2/eht/s3/bin.$mach/bldtree";
#$BUILDTREE = "~rsingh/09..sphinx3code/trees_cont/bin.$mach/bldtree";

$BUILDTREE = "${CFG_BIN_DIR}/bldtree";

#	-stwt     1.0 0.1 0.01 
#	-stwt     1.0 0.3 0.2 
#	-ssplitthr 8e-4 \
#	-csplitthr 1e-5 \

system ("$BUILDTREE -treefn   $unprunedtreedir/$phone-$state.dtree -moddeffn $mdef_file -mixwfn   $mixture_wt_file -ts2cbfn  .semi. -mwfloor  1e-30 -psetfn ${CFG_QUESTION_SET} -phone    $phone -state    $state -stwt     1.0 0.3 0.1 0.01 0.001 -ssplitmin 1 -ssplitmax 5 -ssplitthr 0 -csplitmin 1 -csplitmax 500 -csplitthr 0 >&! $logfile");

exit 0
