cmake_minimum_required(VERSION 3.14) # I like pie

# After an Xcode upgrade, CMakeCache may keep CMAKE_OSX_SYSROOT pointing at a removed
# SDK; the link step then fails (e.g. missing .../usr/lib/libm.tbd). Reset to the
# active SDK when the cached path is missing (must run before project()).
if(APPLE)
  set(_sphinxtrain_sysroot_ok FALSE)
  if(CMAKE_OSX_SYSROOT AND EXISTS "${CMAKE_OSX_SYSROOT}")
    set(_sphinxtrain_sysroot_ok TRUE)
  endif()
  if(NOT _sphinxtrain_sysroot_ok)
    execute_process(
      COMMAND xcrun --sdk macosx --show-sdk-path
      OUTPUT_VARIABLE _sphinxtrain_osx_sdk
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
      RESULT_VARIABLE _sphinxtrain_xcrun_rv
    )
    if(_sphinxtrain_xcrun_rv EQUAL 0 AND _sphinxtrain_osx_sdk AND EXISTS "${_sphinxtrain_osx_sdk}")
      set(CMAKE_OSX_SYSROOT "${_sphinxtrain_osx_sdk}" CACHE PATH "macOS SDK path" FORCE)
      message(STATUS "CMAKE_OSX_SYSROOT -> ${CMAKE_OSX_SYSROOT}")
    endif()
  endif()
endif()

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
  # Suppress warnings for legacy code patterns that are acceptable
  add_compile_options(
    -Wno-sign-compare              # Many int32/uint32 comparisons in legacy code
    -Wno-unused-parameter          # Function signatures must match for API consistency
    -Wno-unused-but-set-variable   # Variables kept for debugging/documentation value
    -Wno-pointer-sign              # Intentional int32*/uint32* pointer conversions
    -Wno-missing-field-initializers # Auto-generated LTS rules
  )
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

# FindBLAS / FindLAPACK cache absolute paths under the SDK; after an Xcode upgrade those
# files may move (e.g. .../MacOSX26.1.sdk/...) while CMAKE_OSX_SYSROOT was fixed above.
# Also drop paths that still exist but live under a different SDK tree than CMAKE_OSX_SYSROOT.
if(APPLE)
  set(_sphinxtrain_apple_libcache_vars
    BLAS_Accelerate_LIBRARY
    BLAS_blas_LIBRARY
    BLAS_LIBRARY
    LAPACK_Accelerate_LIBRARY
    LAPACK_lapack_LIBRARY
    LAPACK_LIBRARY
    MATH_LIBRARY
  )
  foreach(_sphinxtrain_v IN LISTS _sphinxtrain_apple_libcache_vars)
    if(${_sphinxtrain_v})
      set(_sphinxtrain_p "${${_sphinxtrain_v}}")
      if(_sphinxtrain_p MATCHES "^/")
        set(_sphinxtrain_drop FALSE)
        if(NOT EXISTS "${_sphinxtrain_p}")
          set(_sphinxtrain_drop TRUE)
        elseif(CMAKE_OSX_SYSROOT AND _sphinxtrain_p MATCHES "/[^/]+\\.sdk/")
          string(LENGTH "${CMAKE_OSX_SYSROOT}" _sphinxtrain_sl)
          if(_sphinxtrain_sl GREATER 0)
            string(SUBSTRING "${_sphinxtrain_p}" 0 ${_sphinxtrain_sl} _sphinxtrain_head)
            if(NOT _sphinxtrain_head STREQUAL CMAKE_OSX_SYSROOT)
              set(_sphinxtrain_drop TRUE)
            endif()
          endif()
        endif()
        if(_sphinxtrain_drop)
          unset(${_sphinxtrain_v} CACHE)
        endif()
      endif()
    endif()
  endforeach()
  foreach(_sphinxtrain_listvar BLAS_LIBRARIES LAPACK_LIBRARIES)
    if(${_sphinxtrain_listvar})
      set(_sphinxtrain_drop_libs FALSE)
      foreach(_sphinxtrain_item IN LISTS ${_sphinxtrain_listvar})
        if(_sphinxtrain_item MATCHES "^/")
          if(NOT EXISTS "${_sphinxtrain_item}")
            set(_sphinxtrain_drop_libs TRUE)
          elseif(CMAKE_OSX_SYSROOT AND _sphinxtrain_item MATCHES "/[^/]+\\.sdk/")
            string(LENGTH "${CMAKE_OSX_SYSROOT}" _sphinxtrain_sl)
            if(_sphinxtrain_sl GREATER 0)
              string(SUBSTRING "${_sphinxtrain_item}" 0 ${_sphinxtrain_sl} _sphinxtrain_head)
              if(NOT _sphinxtrain_head STREQUAL CMAKE_OSX_SYSROOT)
                set(_sphinxtrain_drop_libs TRUE)
              endif()
            endif()
          endif()
        endif()
      endforeach()
      if(_sphinxtrain_drop_libs)
        unset(${_sphinxtrain_listvar} CACHE)
        if(_sphinxtrain_listvar STREQUAL "BLAS_LIBRARIES")
          unset(BLAS_FOUND CACHE)
        else()
          unset(LAPACK_FOUND CACHE)
        endif()
      endif()
    endif()
  endforeach()
endif()

# Compile all the things
add_subdirectory(src)

# SphinxTrain headers are not meant for public consumption, do not
# install them.

# Install various other stuff though!
install(DIRECTORY etc DESTINATION ${CMAKE_INSTALL_DATADIR}/sphinxtrain)
add_subdirectory(scripts)
# This is unfortunate but will do for now
add_subdirectory(python)
