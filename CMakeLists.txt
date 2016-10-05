cmake_minimum_required(VERSION 2.6)
project(MiguSC)

if((NOT DEFINED OpenMesh_DIR) OR (OpenMesh_DIR EQUAL "OpenMesh_DIR-NOTFOUND") OR (OpenMesh_DIR EQUAL "OpenMesh_DIR_NOT_FOUND"))
  set(OpenMesh_DIR "OpenMesh_DIR-NOTFOUND" CACHE PATH "Path to OpenMesh library.")
  message(FATAL_ERROR "Path to OpenMesh library not set.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

find_package(OpenCV REQUIRED )
find_package(OpenGL REQUIRED )
find_package(OpenMesh REQUIRED)
find_package(SuiteSparse REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIR})
include_directories(${OpenGL_INCLUDE_DIR})
include_directories(${OPENMESH_INCLUDE_DIR})
include_directories(${SuiteSparse_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIR})

link_directories(${OpenCV_LIBRARY_DIR})
link_directories(${OpenGL_LIBRARY_DIR})
link_directories(${SuiteSparse_LIB_DIR})
link_directories(${OPENMESH_LIBRARY_DIR})

if(MSVC)

  if(OpenCV_STATIC)
    foreach(flag_var
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif()
      if(${flag_var} MATCHES "/MDd")
        string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
      endif()
    endforeach(flag_var)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:atlthunk.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:msvcrtd.lib")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:libcmt.lib")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libcmtd.lib")
  endif()
endif()

set (CMAKE_CXX_STANDARD 14)

include_directories(${DAY2_SOURCE_DIR}/src)
add_subdirectory(src)
