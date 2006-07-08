#!/usr/bin/perl

#**************************************************************************
# This script takes a set of (Sphinx3) continuous acoustic models (one number-
# of-senones and one or more number-of-Gaussians) and packages them into
# a form from which they can be built into Sphinx-4 acoustic model JAR files,
# per "How to Use Models from SphinxTrain in Sphinx-4"
# (http://cmusphinx.sourceforge.net/sphinx4/doc/UsingSphinxTrainModels.html).
# The Sphinx-4 models are created in <topdir>/sphinx4_models/."
# It requires the following configurable variables from etc/sphinx_config.cfg
# (plus several others that are computed therein):
#   $CFG_S4_MODEL_BASENAME
#   $CFG_N_TIED_STATES
#   $CFG_INITIAL_NUM_DENSITIES
#   $CFG_FINAL_NUM_DENSITIES
#   $CFG_AUG_SUFFIX (optional)
# Usage:
#   make_s4_model.pl -cfg [<cfg_file>] [<starting_ndens> [<final_ndens>]]
#**************************************************************************
#
# Author: J. J. Wolf
# Date: 7 Jan 05, rev. 24 Mar 05
#
#**************************************************************************

use File::Path;

my $index = 0;
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    $index = 2;
} else {
    $cfg_file = "etc/sphinx_train.cfg";
}

if (! -s "$cfg_file") {
    print ("unable to find default configuration file, use -cfg <cfg_file> or create etc/sphinx_train.cfg for default\n");
    exit -3;
}
require $cfg_file;

$| = 1;				# Turn on autoflushing
# We make a Sphinx-4 model for each number of gaussian densities specified
# on the command line or else in <cfg_file>
$initial_n_densities = ($#ARGV >= $index) ? $ARGV[$index] :
    $CFG_INITIAL_NUM_DENSITIES;
$final_n_densities = ($#ARGV >= ($index+1)) ? $ARGV[$index+1] :
    $CFG_FINAL_NUM_DENSITIES;

&ST_Log ("MODULE: 11  Make Sphinx-4 models $CFG_S4_MODEL_BASENAME $CFG_N_TIED_STATES ${initial_n_densities}-${final_n_densities}\n");

# These are used in step 4 to make the model.props file
$CFG_MAKES4MODEL_SCRIPT = $0;
$CFG_TIMESTAMP = `date`; chomp($CFG_TIMESTAMP);
$CFG_TEMPLATE = "$CFG_BASE_DIR/etc/s4_model.props.template";

# Check to be sure that user has initialized and renamed the template file to
# s4_model.props.template.
unless (-r $CFG_TEMPLATE)
{   ST_LogFatalError(
	"User must initialize a model.props template file $CFG_TEMPLATE");
}

# Create directory to hold Sphinx4 models
$s4mdir = "$CFG_BASE_DIR/sphinx4_models";
mkdir($s4mdir,0777) unless (-d $s4mdir);

# Determine the set of density-counts -- doubling except possibly at the top end
@n_densities = ($n = $initial_n_densities);
until ($n >= $final_n_densities)
{   $n = ($n * 2 <= $final_n_densities) ? ($n * 2) : ($final_n_densities - $n);
    push (@n_densities, $n);
}
foreach $CFG_N_DENSITIES (@n_densities)
{
    ####################################################################
    # 0. Create directories (flushing any previous set)
    ####################################################################

    $fullmodelname = "${CFG_S4_MODEL_BASENAME}_${CFG_N_TIED_STATES}_${CFG_N_DENSITIES}";
    $topdir =  "$s4mdir/$fullmodelname";
    $dictdir = "$topdir/dict";
    $etcdir =  "$topdir/etc";
    
    rmtree($topdir) if (-d $topdir);
    mkdir($topdir,0777);
    mkdir($dictdir,0777);
    mkdir($etcdir,0777);

    ####################################################################
    # 1. The model data is in a model_parameters subdirectory; we include it  
    # at the top level via a symlink.
    ####################################################################

    $modelname = "${CFG_EXPTNAME}.cd_${CFG_DIRLABEL}_${CFG_N_TIED_STATES}_${CFG_N_DENSITIES}";
    $hmmdir    = "${CFG_BASE_DIR}/model_parameters/$modelname";
    if (-d $hmmdir)
    {   symlink($hmmdir, "$topdir/$modelname") ||
	    ST_LogFatalError("Can't symlink $hmmdir");
    }
    else
    {   ST_LogFatalError("Can't find $hmmdir"); }
    &ST_Log ("    Created $topdir/$modelname\n");

    ####################################################################
    # 2. The dictdir contains (links to) the training and filler dictionaries,
    # and let's include the phone list as well for documentation.  (There
    # seems to be little point in including the training dictionary, since
    # it may not be usable by apps, but that's the custom of the Sphinx4 folks.)
    ####################################################################

    if (-r $CFG_DICTIONARY)
    {   symlink($CFG_DICTIONARY, "$dictdir/${CFG_DB_NAME}.dic") ||
	    ST_LogFatalError("Can't symlink $CFG_DICTIONARY");
    }
    else
    {   ST_LogFatalError("Can't find $CFG_DICTIONARY"); }

    if (-r $CFG_FILLERDICT)
    {   symlink($CFG_FILLERDICT, "$dictdir/${CFG_DB_NAME}.filler") ||
	    ST_LogFatalError("Can't symlink $CFG_FILLERDICT");
    }
    else
    {   ST_LogFatalError("Can't find $CFG_FILLERDICT"); }

    if (-r $CFG_RAWPHONEFILE)
    {   symlink($CFG_RAWPHONEFILE, "$dictdir/${CFG_DB_NAME}.phone") ||
	    ST_LogFatalError("Can't symlink $CFG_RAWPHONEFILE");
    }
    else
    {   ST_LogFatalError("Can't find $CFG_RAWPHONEFILE"); }
    &ST_Log ("    Created and filled $dictdir\n");


    ####################################################################
    # 3. The etcdir contains (links to) the CD-tied and CI .mdef files (the
    # latter isn't used either, but it's useful documentation).
    ####################################################################

    $modarchdir  = "$CFG_BASE_DIR/model_architecture";
    $ci_mdef     = "${CFG_EXPTNAME}.ci.mdef";
    # original and augmented .mdef files
    $cd_mdef = "${CFG_EXPTNAME}.${CFG_N_TIED_STATES}.mdef";
    $cd_mdef_aug = "${CFG_EXPTNAME}.${CFG_N_TIED_STATES}_${CFG_AUG_SUFFIX}.mdef";

    # If an augmented cd .mdef file exists, use that; else use the
    # unaugmented one.
    $which = "";
    if (-r "$modarchdir/$cd_mdef_aug")
    {   symlink("$modarchdir/$cd_mdef_aug", "$etcdir/$cd_mdef") ||
	    ST_LogFatalError("Can't symlink $modarchdir/$cd_mdef_aug");
	$which = "augmented $modarchdir/$cd_mdef_aug";
    }
    elsif (-r "$modarchdir/$cd_mdef")
    {   symlink("$modarchdir/$cd_mdef", "$etcdir/$cd_mdef") ||
	    ST_LogFatalError("Can't symlink $modarchdir/$cd_mdef");
	$which = "unaugmented $modarchdir/$cd_mdef";
    }
    else
    {   ST_LogFatalError("Can't find $modarchdir/$cd_mdef_aug or $modarchdir/$cd_mdef"); }

    if (-r "$modarchdir/$ci_mdef")
    {   symlink("$modarchdir/$ci_mdef", "$etcdir/$ci_mdef") ||
	    ST_LogFatalError("Can't symlink $modarchdir/$ci_mdef");
    }
    else
    {   ST_LogFatalError("Can't find $modarchdir/$ci_mdef"); }
    &ST_Log ("    Created and filled $etcdir
        using $which\n");

    ####################################################################
    # 4. Finally, we create the model.props file at the top level, using
    # a (perl) template in etc/, WHICH THE USER MUST HAVE CUSTOMIZED, and
    # which contains a perl string with variables,
    # all of which will be interpolated from $CFG_N_DENSITIES (above),
    # the four $CFG_ variables defined at the top, and some from
    # <cfg_file>.
    ####################################################################

    # Define this before reading in $CFG_TEMPLATE so that it can evaluate
    # $CFG_PROPSFILE correctly
    $CFG_PROPSFILE = "$topdir/model.props";

    # Note: require doesn't read in the file each time around the loop,
    # but do does.
    unless ($return = do $CFG_TEMPLATE)
    {   ST_LogFatalError("Couldn't parse $CFG_TEMPLATE: $@") if $@;
	ST_LogFatalError("Couldn't do $CFG_TEMPLATE") unless defined $return;
	ST_LogFatalError("Couldn't run $CFG_TEMPLATE");
    }
    open (PROPS, ">$CFG_PROPSFILE") ||
	ST_LogFatalError("Can't create $CFG_PROPSFILE");
    #print "${CFG_TEMPLATE}:\n";    # DEBUG
    #print $MODELPROPS_STRING;	# DEBUG
    print PROPS $MODELPROPS_STRING;
    close PROPS;
    &ST_Log ("    Created $CFG_PROPSFILE\n\n");
}				# end foreach
