#!/usr/opt/PERL5004/bin/perl -w

require "/sphx_train/testing/scripts_pl/sphinx_train.cfg";


#*******************************************************************
#*******************************************************************

die "USAGE: $0 <iter>" if ($#ARGV != 0);

$iter = $ARGV[0];

# cover up to 100 different buffer directories....
#$buffer_dirs = ($base_dir/bwaccumdir/${exptname}_buff_? $base_dir/bwaccumdir/${exptname}_buff_??);
@buffer_dirs = (<$CFG_BASE_DIR/bwaccumdir/${CFG_EXPTNAME}_buff_*>); # Catches any number of them

$hmm_dir           = "$CFG_BASE_DIR/model_parameters/${CFG_EXPTNAME}.ci_semi";
mkdir ($hmm_dir,0777) unless -d $hmm_dir;

#new models to be produced after normalization
$mixwfn         = "$hmm_dir/mixture_weights";
$tmatfn         = "$hmm_dir/transition_matrices";
$meanfn         = "$hmm_dir/means";
$varfn          = "$hmm_dir/variances";


mkdir ($CFG_CI_LOG_DIR,0777) unless -d $CFG_CI_LOG_DIR;
$log   = "$CFG_CI_LOG_DIR/${CFG_EXPTNAME}.$iter.norm.log";

#set mach = `~rsingh/51..tools/machine_type.csh`
#set NORM   = ~rsingh/09..sphinx3code/trainer/bin.$mach/norm
$NORM   = "$CFG_BIN_DIR/norm";

system ("$NORM -accumdir @buffer_dirs -mixwfn $mixwfn -tmatfn $tmatfn -meanfn $meanfn -varfn $varfn -feat $CFG_FEATURE -ceplen  $CFG_VECTOR_LENGTH  2> $log");
`date >> $log`;

exit 0
