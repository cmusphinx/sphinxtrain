#!/usr/opt/PERL5004/bin/perl -w

require "/sphx_train/testing/scripts_pl/sphinx_train.cfg";

#*****************************************************************************
#  Baum-welch is done in several parts. This script gathers the results of
#  all those parts and then computes the discrete probability distributions
#  associated with all the states. It also computes the transition matrices.
#****************************************************************************

die "USAGE: $0 <iter>" if ($#ARGV != 0);
$iter   		= $ARGV[0];


#$bwaccumdir 	     = ( $base_dir/bwaccumdir/${exptname}_buff_? $base_dir/bwaccumdir/${exptname}_buff_?? );
$bwaccumdir 	     = "";
for (<$base_dir/bwaccumdir/${CFG_EXPTNAME}_buff_*>) {
    $bwaccumdir .= " $_";
}
$hmmdir 	     = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.cd_semi_untied";
$means               = "$hmmdir/means";
$variances           = "$hmmdir/variances";
$mixture_weights     = "$hmmdir/mixture_weights";
$transition_matrices = "$hmmdir/transition_matrices";

$logdir              = "${CFG_LOG_DIR}/04.cd_schmm_untied";
mkdir ($logdir,0777) unless $logdir;
$logfile 	     = "${CFG_LOG_DIR}/${CFG_EXPTNAME}.${iter}.norm.log";

#set mach = `~/51..tools/machine_type.csh`
#set NORM  = ~/09..sphinx3code/trainer/bin.$mach/norm
$NORM  = "$CFG_BIN_DIR/norm";

system ("$NORM -accumdir $bwaccumdir -mixwfn $mixture_weights  -tmatfn $transition_matrices -meanfn $means -varfn $variances -feat ${CFG_FEATURE} -ceplen 	${CFG_VECTOR_LENGTH}    >&! $logfile");

exit 0
