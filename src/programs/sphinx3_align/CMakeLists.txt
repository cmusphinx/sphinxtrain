set(PROGRAM sphinx3_align)
set(SRCS
adaptor.c
approx_cont_mgau.c
ascr.c
cb2mllr_io.c
cmu6_lts_rules.c
cont_mgau.c
corpus.c
dict2pid.c
dict.c
fast_algo_struct.c
fillpen.c
gs.c
hmm.c
interp.c
kbcore.c
kdtree.c
logs3.c
lts.c
main_align.c
mdef.c
mllr.c
ms_gauden.c
ms_mgau.c
ms_mllr.c
ms_senone.c
s2_semi_mgau.c
s3_align.c
subvq.c
tmat.c
vector.c
)

add_executable(${PROGRAM} ${SRCS})
target_link_libraries(${PROGRAM} sphinxtrain)
target_include_directories(
  ${PROGRAM} PRIVATE ${CMAKE_BINARY_DIR}
  ${PROGRAM} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROGRAM} PUBLIC ${CMAKE_SOURCE_DIR}/include
  ${PROGRAM} INTERFACE ${CMAKE_SOURCE_DIR}/include
  )
install(TARGETS ${PROGRAM} RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/sphinxtrain)
