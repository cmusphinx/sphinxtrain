set(PROGRAM bw)
set(SRCS
accum.c
backward.c
baum_welch.c
forward.c
main.c
next_utt_states.c
train_cmd_ln.c
viterbi.c
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
