# .. cmake_module::
#
#    Module that checks whether SuperLU is available and usable.
#    SuperLU must be a version released after the year 2005.
#
#    Variables used by this module which you may want to set:
#
#    :ref:`SUPERLU_ROOT`
#       Path list to search for SuperLU
#
#    Sets the follwing variables:
#
#    :code:`SUPERLU_FOUND`
#       True if SuperLU available and usable.
#
#    :code:`SUPERLU_MIN_VERSION_4_3`
#       True if SuperLU version >= 4.3.
#
#    :code:`SUPERLU_WITH_VERSION`
#       Human readable string containing version information.
#
#    :code:`SUPERLU_INCLUDE_DIRS`
#       Path to the SuperLU include dirs.
#
#    :code:`SUPERLU_LIBRARIES`
#       Name to the SuperLU library.
#
# .. cmake_variable:: SUPERLU_ROOT
#
#    You may set this variable to have :ref:`FindSuperLU` look
#    for the SuperLU package in the given path before inspecting
#    system paths.
#

# look for BLAS

# look for header files, only at positions given by the user
find_path(SUPERLU_INCLUDE_DIR
  NAMES supermatrix.h
  PATHS ${SUPERLU_ROOT}
  PATH_SUFFIXES "superlu" "SuperLU" "include/superlu" "include" "SRC"
  NO_DEFAULT_PATH
)

# look for header files, including default paths
find_path(SUPERLU_INCLUDE_DIR
  NAMES supermatrix.h
  PATH_SUFFIXES "superlu" "SuperLU" "include/superlu" "include" "SRC"
)

find_path(SUPERLU_LIBRARY_DIR
  NAMES "superlu.lib" 
  PATHS ${SUPERLU_PREFIX} ${SUPERLU_ROOT}
  PATH_SUFFIXES "lib" "lib32" "lib64"
)
# look for library, only at positions given by the user
find_library(SUPERLU_LIBRARY_RELEASE
  NAMES "superlu.lib"
  PATHS ${SUPERLU_PREFIX} ${SUPERLU_ROOT}
  PATH_SUFFIXES "lib" "lib32" "lib64"
)

find_library(SUPERLU_LIBRARY_DEBUG
  NAMES "superlud.lib"
  PATHS ${SUPERLU_PREFIX} ${SUPERLU_ROOT}
  PATH_SUFFIXES "lib" "lib32" "lib64"
)

find_library(BLAS_LIBRARY_RELEASE
  NAMES  "blas.lib"
  PATHS ${SUPERLU_PREFIX} ${SUPERLU_ROOT}
  PATH_SUFFIXES "lib" "lib32" "lib64"
)
find_library(BLAS_LIBRARY_DEBUG
  NAMES "blasd.lib"
  PATHS ${SUPERLU_PREFIX} ${SUPERLU_ROOT}
  PATH_SUFFIXES "lib" "lib32" "lib64"
)

# check version specific macros
include(CheckCSourceCompiles)
include(CMakePushCheckState)

# we need if clauses here because variable is set variable-NOTFOUND
# if the searches above were not successful
# Without them CMake print errors like:
# "CMake Error: The following variables are used in this project, but they are set to NOTFOUND.
# Please set them or make sure they are set and tested correctly in the CMake files:"
#

if(SUPERLU_MIN_VERSION_4_3)
  set(SUPERLU_WITH_VERSION "SuperLU >= 4.3" CACHE STRING
    "Human readable string containing SuperLU version information.")
else()
  set(SUPERLU_WITH_VERSION "SuperLU <= 4.2, post 2005" CACHE STRING
    "Human readable string containing SuperLU version information.")
endif(SUPERLU_MIN_VERSION_4_3)

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "SuperLU"
  DEFAULT_MSG
  SUPERLU_INCLUDE_DIR
  SUPERLU_LIBRARY_DIR
)
find_package_handle_standard_args(
  "BLAS"
  DEFAULT_MSG
  BLAS_LIBRARY_RELEASE
  BLAS_LIBRARY_DEBUG
)
mark_as_advanced(SUPERLU_INCLUDE_DIR SUPERLU_LIBRARY)

# if both headers and library are found, store results
if(SUPERLU_FOUND)
  set(SUPERLU_INCLUDE_DIRS ${SUPERLU_INCLUDE_DIR})
  set(SUPERLU_LIBRARIES_RELEASE    ${SUPERLU_LIBRARY_RELEASE})
  set(SUPERLU_LIBRARIES_DEBUG    ${SUPERLU_LIBRARY_DEBUG})
  set(SUPERLU_LIBRARIES " ${SUPERLU_LIBRARY_DEBUG} ${SUPERLU_LIBRARY_RELEASE}")
  # log result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    "Determining location of ${SUPERLU_WITH_VERSION} succeeded:\n"
    "Include directory: ${SUPERLU_INCLUDE_DIRS}\n"
    "Library directory: ${SUPERLU_LIBRARIES}\n\n")
  set(SUPERLU_DUNE_COMPILE_FLAGS "-I${SUPERLU_INCLUDE_DIRS}"
    CACHE STRING "Compile flags used by DUNE when compiling SuperLU programs")
  set(SUPERLU_DUNE_LIBRARIES ${SUPERLU_LIBRARIES} ${BLAS_LIBRARIES}
    CACHE STRING "Libraries used by DUNE when linking SuperLU programs")
else(SUPERLU_FOUND)
  # log errornous result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determining location of SuperLU failed:\n"
    "Include directory: ${SUPERLU_INCLUDE_DIRS}\n"
    "Library directory: ${SUPERLU_LIBRARIES}\n\n")
endif(SUPERLU_FOUND)

# set HAVE_SUPERLU for config.h
set(HAVE_SUPERLU ${SUPERLU_FOUND})
set(HAVE_SUPERLU ${SUPERLU_FOUND})