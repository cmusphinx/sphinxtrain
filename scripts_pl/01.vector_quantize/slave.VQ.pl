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


#*****************************************************************************
# The agg_seg script aggregates all the training feature vectors into a 
# single dump file and the kmeans script uses the contents of this dump
# file to compute the vq centroids in the vector space 
#*****************************************************************************

#Clean up from previous runs
system "rm -rf $CFG_VQ_LOG_DIR/*" if -d "$CFG_VQ_LOG_DIR";

# No error checking
&ST_Log ("MODULE: 01 Vector Quantization\n");

system ("$CFG_VQ_PERL_DIR/agg_seg.pl -cfg $cfg_file");
system ("$CFG_VQ_PERL_DIR/kmeans.pl -cfg $cfg_file");
