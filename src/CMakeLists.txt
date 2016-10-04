add_executable(MiguSC main.cpp util.h
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

add_subdirectory(triangle)

add_dependencies(MiguSC triangle)

target_link_libraries(MiguSC triangle)

add_definitions(-D_USE_MATH_DEFINES)