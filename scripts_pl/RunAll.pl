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

# What pieces would you like to compute.

@sample_steps = ("$CFG_SCRIPT_DIR/00.verify/verify_all.pl -cfg $cfg_file",
		 "$CFG_SCRIPT_DIR/01.vector_quantize/slave.VQ.pl -cfg $cfg_file",
		 "$CFG_SCRIPT_DIR/02.ci_schmm/slave_convg.pl -cfg $cfg_file 1",
		 "$CFG_SCRIPT_DIR/03.makeuntiedmdef/make_untied_mdef.pl -cfg $cfg_file",
		 "$CFG_SCRIPT_DIR/04.cd_schmm_untied/slave_convg.pl -cfg $cfg_file 1",
		 "$CFG_SCRIPT_DIR/05.buildtrees/make_questions.pl -cfg $cfg_file",
		 "$CFG_SCRIPT_DIR/05.buildtrees/slave.treebuilder.pl -cfg $cfg_file",
		 "$CFG_SCRIPT_DIR/06.prunetree/slave.state-tie-er.pl -cfg $cfg_file",
		 "$CFG_SCRIPT_DIR/07.cd-schmm/slave_convg.pl -cfg $cfg_file 1",
		 "$CFG_SCRIPT_DIR/08.deleted-interpolation/deleted_interpolation.pl -cfg $cfg_file"
		 );


@steps = ("$CFG_SCRIPT_DIR/00.verify/verify_all.pl -cfg $cfg_file",
	  "$CFG_SCRIPT_DIR/01.vector_quantize/slave.VQ.pl -cfg $cfg_file",
	  "$CFG_SCRIPT_DIR/02.ci_schmm/slave_convg.pl -cfg $cfg_file 1",
	  "$CFG_SCRIPT_DIR/03.makeuntiedmdef/make_untied_mdef.pl -cfg $cfg_file",
#	  "$CFG_SCRIPT_DIR/04.cd_schmm_untied/slave_convg.pl -cfg $cfg_file 1",
#	  "$CFG_SCRIPT_DIR/05.buildtrees/make_questions.pl -cfg $cfg_file",
#	  "$CFG_SCRIPT_DIR/05.buildtrees/slave.treebuilder.pl -cfg $cfg_file",
#	  "$CFG_SCRIPT_DIR/06.prunetree/slave.state-tie-er.pl -cfg $cfg_file",
#	  "$CFG_SCRIPT_DIR/07.cd-schmm/slave_convg.pl -cfg $cfg_file 1",
#	  "$CFG_SCRIPT_DIR/08.deleted-interpolation/deleted_interpolation.pl -cfg $cfg_file"
	  );


for $step (@steps) {
    $ret_value = system ($step);
    die "Something failed: ($step)\n" if $ret_value;
}


