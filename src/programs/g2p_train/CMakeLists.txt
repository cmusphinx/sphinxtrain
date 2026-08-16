set(PROGRAM g2p_train)
set(SRCS
g2p_train.cpp
main.cpp
${CMAKE_SOURCE_DIR}/src/upstream/Phonetisaurus/src/lib/M2MFstAligner.cc
${CMAKE_SOURCE_DIR}/src/upstream/Phonetisaurus/src/lib/LatticePruner.cc
${CMAKE_SOURCE_DIR}/src/upstream/Phonetisaurus/src/lib/util.cc
  )

add_executable(${PROGRAM} ${SRCS})
target_link_libraries(${PROGRAM} sphinxtrain
  ${FST} ${FSTSCRIPT} ${FSTFAR} ${FSTFARSCRIPT} ${NGRAM})
target_include_directories(
  ${PROGRAM} PRIVATE ${CMAKE_BINARY_DIR}
  ${PROGRAM} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROGRAM} PRIVATE ${CMAKE_SOURCE_DIR}/src/upstream/Phonetisaurus/src
  ${PROGRAM} PRIVATE ${CMAKE_SOURCE_DIR}/src/upstream/Phonetisaurus/src/3rdparty/utfcpp
  ${PROGRAM} PUBLIC ${CMAKE_SOURCE_DIR}/include
  ${PROGRAM} INTERFACE ${CMAKE_SOURCE_DIR}/include
  )
install(TARGETS ${PROGRAM} RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/sphinxtrain)
