add_executable(DAY2 main.cpp
)

if(WIN32)
   set_target_properties(DAY2 PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(DAY2 PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
   set_target_properties(DAY2 PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:windows")
   set_target_properties(DAY2 PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:windows")
endif(WIN32)

target_link_libraries( DAY2 ${OpenCV_LIBRARIES} )
target_link_libraries( DAY2 ${OPENGL_LIBRARIES} )
     
install (TARGETS DAY2 DESTINATION bin)
add_subdirectory(triangle)

add_dependencies(DAY2 triangle)