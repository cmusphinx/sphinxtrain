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


#*************************************************************************
# This script runs the build_tree script for each state of each basephone
#*************************************************************************

&ST_Log ("\tBuilding Trees\n");

my ($phone,$state);
my $scriptdir = "{$CFG_SCRIPT_DIR}/05.buildtrees";
my $logdir = "${CFG_LOG_DIR}/05.buildtrees";
mkdir ($logdir,0777) unless -d $logdir;


# Clean up 
print "\tCleaning up old log files...\n";
system ("rm -f $logdir/*");

my $mdef_file       = "${CFG_BASE_DIR}/model_architecture/${CFG_EXPTNAME}.untied.mdef";
my $mixture_wt_file = "${CFG_BASE_DIR}/model_parameters/${CFG_EXPTNAME}.cd_semi_untied/mixture_weights";
my $tree_base_dir   = "${CFG_BASE_DIR}/trees";
my $unprunedtreedir = "$tree_base_dir/${CFG_EXPTNAME}.unpruned";
mkdir ($tree_base_dir,0777) unless -d "tree_base_dir";
mkdir ($unprunedtreedir,0777) unless -d $unprunedtreedir;

# For every phone submit each possible state
&ST_Log ("\tProcessing each phone with each state\n");
open INPUT,"${CFG_RAWPHONEFILE}";
foreach $phone (<INPUT>) {
    chomp $phone;
    if (($phone =~ m/^(\+).*(\+)$/) || ($phone =~ m/^SIL$/)) {
	&ST_Log ("\t\tSkipping $phone\n");
	next;
    }

    $state = 0;
    while ( $state < $CFG_STATESPERHMM) {
	&BuildTree ($phone,$state);
	$state++;
    }
    close INPUT;
}

exit 0;


# SubRoutine created from build_tree.pl
sub BuildTree ()
{
    my $phn = shift;
    my $stt = shift;

    my $logfile = "$logdir/${CFG_EXPTNAME}.buildtree.${phn}.${stt}.log";

    &ST_Log ("\t\t${phn} ${stt} ");
    &ST_HTML_Print ("<A HREF=\"$logfile\">Log File</A>\n");

    #$mach = `~/51..tools/machine_type.csh`;
    #$BUILDTREE = "/net/alf19/usr2/eht/s3/bin.$mach/bldtree";
    #$BUILDTREE = "~rsingh/09..sphinx3code/trees_cont/bin.$mach/bldtree";
    
    $BUILDTREE = "${CFG_BIN_DIR}/bldtree";

    # RAH 7.21.2000 - These were other possible values for these
    # variables, I'm not sure the circumstance that would dictate
    # either set of values
    #	-stwt     1.0 0.1 0.01 
    #	-stwt     1.0 0.3 0.2 
    #	-ssplitthr 8e-4 \
    #	-csplitthr 1e-5 \

    open LOG,">$logfile";    
    if (open PIPE, "$BUILDTREE -treefn $unprunedtreedir/$phn-$stt.dtree -moddeffn $mdef_file -mixwfn $mixture_wt_file -ts2cbfn ${CFG_HMM_TYPE} -mwfloor 1e-30 -psetfn ${CFG_QUESTION_SET} -phone $phn -state $stt -stwt 1.0 0.3 0.1 0.01 0.001 -ssplitmin 1 -ssplitmax 5 -ssplitthr 0 -csplitmin 1 -csplitmax 500 -csplitthr 0 2>&1 |") {
	
	while (<PIPE>) {
	    print LOG "$_";
	}
	close PIPE;
	close LOG;
    } else {
	&ST_LogError ("Unable to start $BUILDTREE\n");
	print LOG "Unable to start $BUILDTREE\n";
	close LOG;
    }
}
