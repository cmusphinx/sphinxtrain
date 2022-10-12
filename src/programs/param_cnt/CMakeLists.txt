set(PROGRAM param_cnt)
set(SRCS
cb_cnt.c
enum_corpus.c
main.c
param_cnt.c
parse_cmd_ln.c
phone_cnt.c
ts_cnt.c
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
