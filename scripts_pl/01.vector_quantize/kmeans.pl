#!/usr/local/bin/perl5 

# RAH Force passage of config file, or look for it in the current directory
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    die "-cfg specified, but unable to find file $ARGV[1]" unless (-s $cfg_file);
    require $cfg_file;
} else {
    $cfg_file = "./sphinx_train.cfg";
    die "Must specify -cfg or create default file ./sphinx_train.cfg" unless (-s  $cfg_file);
    require ("./sphinx_train.cfg");
    &ST_LogWarning ("-cfg not specified, using the default ./sphinx_train.cfg");
}

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

