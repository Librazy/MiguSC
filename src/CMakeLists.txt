if(POLICY CMP0020)
  cmake_policy(PUSH)
  cmake_policy(SET CMP0020 NEW)
endif()

add_library(MiguSC STATIC 
  main.cpp main.h util.h
)

if(WIN32)
   set_target_properties(MiguSC PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(MiguSC PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
   set_target_properties(MiguSC PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:windows")
   set_target_properties(MiguSC PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:windows")
endif(WIN32)

target_link_libraries( MiguSC ${OpenCV_LIBRARIES} )
target_link_libraries( MiguSC ${OPENGL_LIBRARIES} )
target_link_libraries( MiguSC ${OPENMESH_LIBRARIES} )

target_link_libraries( MiguSC Qt5::Core  Qt5::Widgets Qt5::Gui Qt5::OpenGL)

if(POLICY CMP0020)
  cmake_policy(POP)
endif()

add_subdirectory(triangle)

add_dependencies(MiguSC triangle)

target_link_libraries(MiguSC triangle)

add_definitions(-D_USE_MATH_DEFINES)