set(SRCS
libs/libio/s3gau_full_io.c
libs/libio/s3map_io.c
libs/libio/s3tmat_io.c
libs/libio/segdmp.c
libs/libio/model_def_io.c
libs/libio/s3gau_io.c
libs/libio/topo_read.c
libs/libio/s3lamb_io.c
libs/libio/s3phseg_io.c
libs/libio/s3cb2mllr_io.c
libs/libio/swap.c
libs/libio/s3acc_io.c
libs/libio/s3mixw_io.c
libs/libio/pset_io.c
libs/libio/s3ts2cb_io.c
libs/libio/corpus.c
libs/libio/s3io.c
libs/libsphinxbase/util/ckd_alloc.c
libs/libsphinxbase/util/logmath.c
libs/libsphinxbase/util/cmd_ln.c
libs/libsphinxbase/util/bitvec.c
libs/libsphinxbase/util/genrand.c
libs/libsphinxbase/util/hash_table.c
libs/libsphinxbase/util/strfuncs.c
libs/libsphinxbase/util/matrix.c
libs/libsphinxbase/util/pio.c
libs/libsphinxbase/util/heap.c
libs/libsphinxbase/util/case.c
libs/libsphinxbase/util/err.c
libs/libsphinxbase/util/glist.c
libs/libsphinxbase/util/dtoa.c
libs/libsphinxbase/util/bitarr.c
libs/libsphinxbase/util/profile.c
libs/libsphinxbase/util/bio.c
libs/libsphinxbase/util/filename.c
libs/libsphinxbase/util/priority_queue.c
libs/libsphinxbase/util/errno.c
libs/libsphinxbase/util/listelem_alloc.c
libs/libsphinxbase/util/mmio.c
libs/libsphinxbase/feat/cmn.c
libs/libsphinxbase/feat/agc.c
libs/libsphinxbase/feat/lda.c
libs/libsphinxbase/feat/feat.c
libs/libsphinxbase/feat/cmn_live.c
libs/libsphinxbase/fe/fe_warp_inverse_linear.c
libs/libsphinxbase/fe/fe_sigproc.c
libs/libsphinxbase/fe/yin.c
libs/libsphinxbase/fe/fe_interface.c
libs/libsphinxbase/fe/fe_warp_affine.c
libs/libsphinxbase/fe/fixlog.c
libs/libsphinxbase/fe/fe_warp.c
libs/libsphinxbase/fe/fe_prespch_buf.c
libs/libsphinxbase/fe/fe_noise.c
libs/libsphinxbase/fe/fe_warp_piecewise_linear.c
libs/libclust/kmeans.c
libs/libclust/div.c
libs/libclust/kdtree.c
libs/libclust/metric.c
libs/libmllr/mllr_io.c
libs/libmllr/mllr.c
libs/libmodinv/mod_inv.c
libs/libmodinv/gauden.c
libs/libcommon/state_seq.c
libs/libcommon/quest.c
libs/libcommon/vector.c
libs/libcommon/best_q.c
libs/libcommon/ck_seg.c
libs/libcommon/cvt2triphone.c
libs/libcommon/mk_phone_list.c
libs/libcommon/remap.c
libs/libcommon/heap.c
libs/libcommon/itree.c
libs/libcommon/acmod_set.c
libs/libcommon/mk_trans_seq.c
libs/libcommon/mk_ts2ci.c
libs/libcommon/mk_phone_seq.c
libs/libcommon/was_added.c
libs/libcommon/dtree.c
libs/libcommon/ts2cb.c
libs/libcommon/mk_sseq.c
libs/libcommon/lexicon.c
  )
set(LAPACK_SRCS
libs/libsphinxbase/util/slamch.c
libs/libsphinxbase/util/slapack_lite.c
libs/libsphinxbase/util/blas_lite.c
libs/libsphinxbase/util/f2c_lite.c
)
find_package(BLAS)
find_package(LAPACK)
find_library(MATH_LIBRARY m)
if(NOT LAPACK_FOUND)
  message("System-wide LAPACK not found, will use internal version")
  add_library(sphinxtrain ${SRCS} ${LAPACK_SRCS})
else()
  add_library(sphinxtrain ${SRCS})
  target_link_libraries(sphinxtrain PUBLIC ${LAPACK_LIBRARIES})
endif()

target_include_directories(
  sphinxtrain PRIVATE ${CMAKE_BINARY_DIR}
  sphinxtrain PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  sphinxtrain PUBLIC ${CMAKE_SOURCE_DIR}/include
  sphinxtrain INTERFACE ${CMAKE_SOURCE_DIR}/include
  )
if(APPLE)
  # Things we might need are here
  target_link_directories(sphinxtrain PUBLIC /usr/local/lib)
endif()
find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
  target_link_libraries(sphinxtrain PUBLIC ${MATH_LIBRARY})
endif()
# Shared library version != package version, but we will make it the
# same for now to avoid confusion
set_target_properties(sphinxtrain PROPERTIES
  VERSION 5.0.0
  SOVERSION 5
  WINDOWS_EXPORT_ALL_SYMBOLS ON
  )
install(TARGETS sphinxtrain LIBRARY)

add_subdirectory(programs/agg_seg)
add_subdirectory(programs/bldtree)
add_subdirectory(programs/bw)
add_subdirectory(programs/cdcn_norm)
add_subdirectory(programs/cdcn_train)
add_subdirectory(programs/cp_parm)
add_subdirectory(programs/delint)
add_subdirectory(programs/inc_comp)
add_subdirectory(programs/init_gau)
add_subdirectory(programs/init_mixw)
add_subdirectory(programs/kdtree)
add_subdirectory(programs/kmeans_init)
add_subdirectory(programs/make_quests)
add_subdirectory(programs/map_adapt)
add_subdirectory(programs/mixw_interp)
add_subdirectory(programs/mk_flat)
add_subdirectory(programs/mk_mdef_gen)
add_subdirectory(programs/mk_mllr_class)
add_subdirectory(programs/mk_s2sendump)
add_subdirectory(programs/mk_ts2cb)
add_subdirectory(programs/mllr_solve)
add_subdirectory(programs/mllr_transform)
add_subdirectory(programs/norm)
add_subdirectory(programs/param_cnt)
add_subdirectory(programs/printp)
add_subdirectory(programs/prunetree)
add_subdirectory(programs/sphinx_cepview)
add_subdirectory(programs/sphinx_fe)
add_subdirectory(programs/sphinx3_align)
add_subdirectory(programs/tiestate)

option(BUILD_G2P "Build G2P trainer (requires OpenFST)" OFF)
if(BUILD_G2P)
  find_library(FST fst)
  find_library(FSTSCRIPT fstscript)
  find_library(FSTFAR fstfar)
  find_library(FSTFARSCRIPT fstfarscript)
  find_library(NGRAM ngram)
  add_subdirectory(programs/g2p_eval)
  add_subdirectory(programs/g2p_train)
endif()
