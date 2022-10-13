cmake_minimum_required(VERSION 3.14) # I like pie

project(SphinxTrain VERSION 5.0.0
  DESCRIPTION "CMU Sphinx Trainer"
  HOMEPAGE_URL "https://github.com/cmusphinx/sphinxtrain")
include(CMakePrintHelpers)
set(PACKAGE_NAME ${PROJECT_NAME})
string(TOLOWER ${PROJECT_NAME} PROJECT_SHORTNAME)
set(PACKAGE_VERSION ${PROJECT_VERSION})
set(PACKAGE_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set(PACKAGE_TARNAME "${PROJECT_SHORTNAME}-${PROJECT_VERSION}")
set(PACKAGE_URL ${PROJECT_HOMEPAGE_URL})
set(PACKAGE_BUGREPORT dhdaines@gmail.com)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  enable_testing()
endif()

include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckLibraryExists)
include(TestBigEndian)
include(GNUInstallDirs)

test_big_endian(WORDS_BIGENDIAN)
cmake_print_variables(WORDS_BIGENDIAN)
CHECK_TYPE_SIZE(long LONG)
CHECK_TYPE_SIZE("long long" LONG_LONG)
set(SIZEOF_LONG ${LONG})
set(SIZEOF_LONG_LONG ${LONG_LONG})
cmake_print_variables(SIZEOF_LONG SIZEOF_LONG_LONG)
CHECK_SYMBOL_EXISTS(popen stdio.h HAVE_POPEN)
CHECK_SYMBOL_EXISTS(snprintf stdio.h HAVE_SNPRINTF)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(errno.h HAVE_ERRNO_H)

# FIXME: Should be a more portable way to do this...
if(MSVC)
  add_compile_options(/W3)
else()
  add_compile_options(-Wall -Wextra)
endif()

# Don't build shared libs by default, but distributions can do it
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# Put all the output where we can find it (and run in place)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Configure a file, why don't you
configure_file(config.h.in config.h)
add_definitions(-DHAVE_CONFIG_H)

# Compile all the things
add_subdirectory(src)

# SphinxTrain headers are not meant for public consumption, do not
# install them.

# Install various other stuff though!
install(DIRECTORY etc DESTINATION ${CMAKE_INSTALL_DATADIR}/sphinxtrain)
add_subdirectory(scripts)
# This is unfortunate but will do for now
add_subdirectory(python)
