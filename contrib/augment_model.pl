#!/usr/bin/perl

#**************************************************************************
# This script produces an augmented model, which has
# additional (beyond the training data) triphones produced by walking the
# pruned decision trees and selecting from the existing set of senones.
# The additional triphones come from using a dictionary that's augmented with
# words from outside the training corpus.  (This extra step can be avoided
# by using an augmented dictionary in Module 06, or for that matter, in
# all the training steps.)
#**************************************************************************
#
# Author: J. J. Wolf using parts of scripts by Alan W Black (awb@cs.cmu.edu)
#
#**************************************************************************

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

my $logdir = "$CFG_LOG_DIR/10.augment_model";
mkdir ($logdir,0777) unless -d $logdir;

$| = 1; # Turn on autoflushing
&ST_Log ("MODULE: 10 Augment ($CFG_AUG_SUFFIX) ${CFG_N_TIED_STATES}-senone map and phone files\n");

#**************************************************************************
# (1) Generate augmented "alltriphones" model definition file
# Note: if doing multiple senone sets, this needs to be done only once.
#**************************************************************************
my $logfile = "$logdir/$CFG_EXPTNAME.build.alltriphones_$CFG_AUG_SUFFIX.mdef.log";
$MAKE_MDEF = "$CFG_BIN_DIR/mk_mdef_gen";
$modarchdir          = "$CFG_BASE_DIR/model_architecture";
$AUG_ALLTRIPHONESMDDEF = "$modarchdir/$CFG_EXPTNAME.alltriphones_$CFG_AUG_SUFFIX.mdef";
$phonefile           = "$modarchdir/$CFG_EXPTNAME.phonelist";

&ST_Log ("    Generate new 'alltriphones' mdef file from augmented dictionary $CFG_AUG_DICT\n");
ST_HTML_Print("    mk_mdef_gen  " .
	      ST_FormatURL($logfile, "Log File") . "\n");
system ("$MAKE_MDEF -phnlstfn $phonefile -oalltphnmdef $AUG_ALLTRIPHONESMDDEF -dictfn $CFG_AUG_DICT -fdictfn $CFG_FILLERDICT -n_state_pm $CFG_STATESPERHMM 2>$logfile");

# Extract and log useful phone/triphone counts from the log file.
if (open(LOG, $logfile))
{   while (<LOG>)
    {   if (/unique|n_base/)	# these mark the lines we want
	{   s/INFO:\s+mk_mdef_gen.c\(\d+\):\s/\t/g;
	    ST_Log($_);
	}
    }
    close(LOG);
}
else
{   ST_LogWarning("Failed to extract triphone counts from $logfile\n");
}

#**************************************************************************
# (2) Use 'tiestate' to walk the trees and produce an augmented tied-model
# definition file.
#**************************************************************************

my $logfile = "$logdir/$CFG_EXPTNAME.aug_tiestate.${CFG_N_TIED_STATES}_$CFG_AUG_SUFFIX.log";
my $prunedtreedir = "$CFG_BASE_DIR/trees/$CFG_EXPTNAME.$CFG_N_TIED_STATES";
my $tied_mdef_file = "$CFG_BASE_DIR/model_architecture/$CFG_EXPTNAME.${CFG_N_TIED_STATES}_$CFG_AUG_SUFFIX.mdef";
my $TIESTATE = "$CFG_BIN_DIR/tiestate";

&ST_Log ("    Walk the trees, produce augmented tied-state mdef file  ");
print ("\n");
&ST_HTML_Print(ST_FormatURL($logfile, "Log File") . "\n");
system("$TIESTATE -imoddeffn $AUG_ALLTRIPHONESMDDEF -omoddeffn $tied_mdef_file -treedir $prunedtreedir -psetfn $CFG_QUESTION_SET 2>$logfile");

#**************************************************************************
# (3) Produce new Sphinx2 map and phone files from the augmented tied-model 
# definition file (only if semi-continuous model).
#**************************************************************************

# Do step 3 only for Sphinx2 nodels
if ($CFG_HMM_TYPE eq ".semi.")
{   my $logfile = "$logdir/$CFG_EXPTNAME.make_aug_phone_map.${CFG_N_TIED_STATES}_$CFG_AUG_SUFFIX.log";
    my $s2dir = "$CFG_BASE_DIR/model_parameters/$CFG_EXPTNAME.s2models_$CFG_N_TIED_STATES";

    &ST_Log ("    Make augmented phone and map files\n");
    system("$CFG_BIN_DIR/mk_s2phonemap -moddeffn $tied_mdef_file -phonefn $s2dir/phone_$CFG_AUG_SUFFIX -mapfn $s2dir/map_$CFG_AUG_SUFFIX >$logfile 2>&1");
}
