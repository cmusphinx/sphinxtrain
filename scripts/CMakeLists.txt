set(SCRIPTDIRS
0000.g2p_train
000.comp_feat
00.verify
01.lda_train
02.mllt_train
05.vector_quantize
10.falign_ci_hmm
11.force_align
12.vtln_align
20.ci_hmm
30.cd_hmm_untied
40.buildtrees
45.prunetree
50.cd_hmm_tied
60.lattice_generation
61.lattice_pruning
62.lattice_conversion
65.mmie_train
80.mllr_adapt
90.deleted_interpolation
decode
lib
prepare
)
install(DIRECTORY ${SCRIPTDIRS} DESTINATION ${CMAKE_INSTALL_DATADIR}/sphinxtrain/scripts)
install(PROGRAMS sphinxtrain TYPE BIN)
