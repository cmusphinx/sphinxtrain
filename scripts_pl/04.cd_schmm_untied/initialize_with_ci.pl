#!/usr/opt/PERL5004/bin/perl -w

require "/sphx_train/testing/scripts_pl/sphinx_train.cfg";


#**************************************************************************
# This script copies the ci model-parameters into cd-model parameters
# as initial model-parameters for the cd training.
# This is called "initializing cd models with ci models"
#**************************************************************************
die "USAGE: $0" if shift;

# Definitions

$cihmmdir = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.ci_semi";
$cdhmmdir = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.cd_semi_untied";

mkdir ($cdhmmdir,0777) unless -d $cdhmmdir;

$logdir  =  "${CFG_BASE_DIR}/logdir/04.cd_schmm_untied";
mkdir ($logdir,0777) unless -d $logdir;
$logfile = "$logdir/${CFG_EXPTNAME}.copycitocd.log";

#set mach = `~/51..tools/machine_type.csh`
#set COPY_CI_TO_CD = /net/alf19/usr2/eht/s3/bin.$mach/init_mixw
#set COPY_CI_TO_CD = ~/09..sphinx3code/trainer/bin.$mach/init_mixw
$COPY_CI_TO_CD = "${CFG_BIN_DIR}/init_mixw";

system ("$COPY_CI_TO_CD \
	-src_moddeffn ${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.ci.mdef \
	-src_ts2cbfn  ${CFG_TYPE} \
	-src_mixwfn   $cihmmdir/mixture_weights \
	-src_meanfn   $cihmmdir/means \
	-src_varfn    $cihmmdir/variances \
	-src_tmatfn   $cihmmdir/transition_matrices \
	-dest_moddeffn ${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.untied.mdef \
	-dest_ts2cbfn  ${CFG_TYPE} \
	-dest_mixwfn   $cdhmmdir/mixture_weights \
	-dest_meanfn   $cdhmmdir/means \
	-dest_varfn    $cdhmmdir/variances \
	-dest_tmatfn   $cdhmmdir/transition_matrices \
        -feat          ${CFG_FEATURE} \
	-ceplen        ${CFG_VECTOR_LENGTH} \
>&! $logfile");

exit 0
