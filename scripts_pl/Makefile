# ====================================================================
# Copyright (c) 2000 Carnegie Mellon University.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer. 
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# This work was supported in part by funding from the Defense Advanced 
# Research Projects Agency and the National Science Foundation of the 
# United States of America, and the CMU Sphinx Speech Consortium.
#
# THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
# ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
# NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ====================================================================
#
#  Perl scripts
#
# ====================================================================
TOP=..
DIRNAME=scripts_pl
BUILD_DIRS=
ALL_DIRS=
FESTIVAL_SCRIPTS = make_dict.festival
MODULES = lib/Queue/POSIX.pm			\
	lib/Queue/Job.pm			\
	lib/Queue/PBS.pm			\
	lib/SphinxTrain/Config.pm		\
	lib/SphinxTrain/Util.pm			\
	lib/Queue.pm
SCRIPTS = 00.verify/verify_all.pl				\
	03.force_align/force_align.pl				\
	03.force_align/slave_align.pl				\
	05.lda_train/lda_train.pl				\
	10.vector_quantize/agg_seg.pl				\
	10.vector_quantize/kmeans.pl				\
	10.vector_quantize/slave.VQ.pl				\
	20.ci_hmm/baum_welch.pl					\
	20.ci_hmm/norm_and_launchbw.pl				\
	20.ci_hmm/norm.pl					\
	20.ci_hmm/slave_convg.pl				\
	30.cd_hmm_untied/baum_welch.pl				\
	30.cd_hmm_untied/norm_and_launchbw.pl			\
	30.cd_hmm_untied/norm.pl				\
	30.cd_hmm_untied/slave_convg.pl				\
	40.buildtrees/buildtree.pl				\
	40.buildtrees/make_questions.pl				\
	40.buildtrees/prunetree.pl				\
	40.buildtrees/slave.treebuilder.pl			\
	40.buildtrees/tiestate.pl				\
	50.cd_hmm_tied/baum_welch.pl				\
	50.cd_hmm_tied/norm_and_launchbw.pl			\
	50.cd_hmm_tied/norm.pl					\
	50.cd_hmm_tied/slave_convg.pl				\
	50.cd_hmm_tied/split_gaussians.pl			\
	60.lattice_generation/slave_genlat.pl			\
	60.lattice_generation/force_align.pl			\
	60.lattice_generation/s3decode.pl			\
	61.lattice_pruning/slave_prune.pl			\
	61.lattice_pruning/lattice_prune.pl			\
	62.lattice_conversion/slave_conv.pl			\
	62.lattice_conversion/lattice_conv.pl			\
	65.mmie_train/slave_convg.pl				\
	65.mmie_train/baum_welch.pl				\
	65.mmie_train/norm_and_launchbw.pl			\
	65.mmie_train/norm.pl					\
	90.deleted_interpolation/deleted_interpolation.pl	\
	decode/slave.pl						\
	copy_setup.pl						\
	make_feats.pl						\
	maketopology.pl						\
	RunAll.pl						\
	setup_SphinxTrain.pl					\
	setup_tutorial.pl					\
	texFormat.pl

FILES = Makefile RunAll.pl $(TMPSCRIPTS) \
        $(FESTIVAL_SCRIPTS) $(MODULES) \
	$(SCRIPTS)

ALL = make_dict

include $(TOP)/config/common_make_rules

LOCAL_CLEAN = make_dict

make_dict: make_dict.festival
	@ echo "#!/bin/sh" > $@
	@ echo "\"true\"; exec "$(FESTIVAL)' --script $$0 $$*' >>$@
	@ cat $< >>$@
	@ chmod +x $@
