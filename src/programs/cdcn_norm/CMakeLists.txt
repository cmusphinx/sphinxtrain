set(PROGRAM cdcn_norm)
set(SRCS
cepdist.c
correction.c
initialize.c
main.c
max_q.c
mmse_x.c
norm_utt.c
readdistribution.c
resfft.c
rsfft.c
two_D_ize.c
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
