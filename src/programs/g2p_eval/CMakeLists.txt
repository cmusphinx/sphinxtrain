set(PROGRAM g2p_eval)
set(SRCS
main.cpp
Phonetisaurus.cpp
phonetisaurus-g2p.cpp
${CMAKE_SOURCE_DIR}/src/programs/g2p_train/FstPathFinder.cpp
  )

add_executable(${PROGRAM} ${SRCS})
target_link_libraries(${PROGRAM} sphinxtrain ${FST} ${NGRAM})
target_include_directories(
  ${PROGRAM} PRIVATE ${CMAKE_BINARY_DIR}
  ${PROGRAM} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROGRAM} PUBLIC ${CMAKE_SOURCE_DIR}/include
  ${PROGRAM} INTERFACE ${CMAKE_SOURCE_DIR}/include
  )
install(TARGETS ${PROGRAM} RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/sphinxtrain)
