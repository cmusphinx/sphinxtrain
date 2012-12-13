bin_SCRIPTS = sphinxtrain

scriptsdir=$(pkglibdir)/scripts

nobase_scripts_SCRIPTS = \
	000.comp_feat/make_feats.pl \
	000.comp_feat/slave_feat.pl \
	0000.g2p_train/g2p_train.pl \
	0000.g2p_train/evaluate.py \
	0000.g2p_train/calculateER.py \
	00.verify/verify_all.pl \
	01.lda_train/baum_welch.pl \
	01.lda_train/lda_train.pl \
	01.lda_train/norm_and_launchbw.pl \
	01.lda_train/norm.pl \
	01.lda_train/slave_lda.pl \
	02.mllt_train/baum_welch.pl \
	02.mllt_train/mllt_train.pl \
	02.mllt_train/norm_and_launchbw.pl \
	02.mllt_train/norm.pl \
	02.mllt_train/slave_mllt.pl \
	05.vector_quantize/agg_seg.pl \
	05.vector_quantize/kmeans.pl \
	05.vector_quantize/slave.VQ.pl \
	10.falign_ci_hmm/baum_welch.pl \
	10.falign_ci_hmm/norm_and_launchbw.pl \
	10.falign_ci_hmm/norm.pl \
	10.falign_ci_hmm/slave_convg.pl \
	10.falign_ci_hmm/split_gaussians.pl \
	11.force_align/force_align.pl \
	11.force_align/make_spd.pl \
	11.force_align/slave_align.pl \
	12.vtln_align/slave_align.pl \
	12.vtln_align/vtln_align.pl \
	20.ci_hmm/baum_welch.pl \
	20.ci_hmm/norm_and_launchbw.pl \
	20.ci_hmm/norm.pl \
	20.ci_hmm/slave_convg.pl \
	20.ci_hmm/split_gaussians.pl \
	30.cd_hmm_untied/baum_welch.pl \
	30.cd_hmm_untied/norm_and_launchbw.pl \
	30.cd_hmm_untied/norm.pl \
	30.cd_hmm_untied/slave_convg.pl \
	40.buildtrees/buildtree.pl \
	40.buildtrees/make_questions.pl \
	40.buildtrees/slave.treebuilder.pl \
	45.prunetree/prunetree.pl \
	45.prunetree/slave.state-tying.pl \
	45.prunetree/tiestate.pl \
	50.cd_hmm_tied/baum_welch.pl \
	50.cd_hmm_tied/norm_and_launchbw.pl \
	50.cd_hmm_tied/norm.pl \
	50.cd_hmm_tied/slave_convg.pl \
	50.cd_hmm_tied/split_gaussians.pl \
	60.lattice_generation/force_align.pl \
	60.lattice_generation/s3decode.pl \
	60.lattice_generation/slave_genlat.pl \
	61.lattice_pruning/lattice_prune.pl \
	61.lattice_pruning/slave_prune.pl \
	62.lattice_conversion/lattice_conv.pl \
	62.lattice_conversion/slave_conv.pl \
	65.mmie_train/baum_welch.pl \
	65.mmie_train/norm_and_launchbw.pl \
	65.mmie_train/norm.pl \
	65.mmie_train/slave_convg.pl \
	80.mllr_adapt/baum_welch.pl \
	80.mllr_adapt/mllr_solve.pl \
	80.mllr_adapt/slave_adapt.pl \
	90.deleted_interpolation/deleted_interpolation.pl \
	decode/psdecode.pl \
	decode/slave.pl \
	decode/verify_dec.pl \
	decode/word_align.pl \
	lib/Queue/Job.pm \
	lib/Queue/PBS.pm \
	lib/Queue.pm \
	lib/Queue/POSIX.pm \
	lib/SimpleConfig.pm \
	lib/SphinxTrain/Config.pm \
	lib/SphinxTrain/Util.pm \
	lib/test_SimpleConfig.pl \
	lib/test_SimpleConfig.txt \
	prepare/make_phoneset.pl \
	prepare/maketopology.pl

EXTRA_DIST = $(nobase_scripts_SCRIPTS) sphinxtrain

