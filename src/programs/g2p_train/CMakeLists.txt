set(PROGRAM g2p_train)
set(SRCS
FstPathFinder.cpp
g2p_train.cpp
M2MFstAligner.cpp
main.cpp
  )

add_executable(${PROGRAM} ${SRCS})
target_link_libraries(${PROGRAM} sphinxtrain
  ${FST} ${FSTSCRIPT} ${FSTFAR} ${FSTFARSCRIPT} ${NGRAM})
target_include_directories(
  ${PROGRAM} PRIVATE ${CMAKE_BINARY_DIR}
  ${PROGRAM} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROGRAM} PUBLIC ${CMAKE_SOURCE_DIR}/include
  ${PROGRAM} INTERFACE ${CMAKE_SOURCE_DIR}/include
  )
install(TARGETS ${PROGRAM} RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/sphinxtrain)
