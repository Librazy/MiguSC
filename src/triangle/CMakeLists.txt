cmake_minimum_required(VERSION 2.8.12)
project(triangle)

add_library(triangle STATIC 
 acute.c
 acute.h
 acute_internal.h
 eps_writer.c
 eps_writer.h
 predicates.c
 predicates.h
 triangle.c
 triangle.h
 triangle_config.h
 triangle_internal.h
 triangle_io.c
 util.c
 util.h
 triangle_api.h
 triangle_api.c
 triangle_helper.h
 triangle_helper.c)
 
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DNO_FILE_IO)