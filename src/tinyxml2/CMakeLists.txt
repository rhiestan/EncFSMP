cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0063)
	cmake_policy(SET CMP0063 OLD)
endif()

project(tinyxml2)

################################
# set lib version here

set(GENERIC_LIB_VERSION "5.0.1")
set(GENERIC_LIB_SOVERSION "5")

################################
# Add definitions

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

add_library(tinyxml2_static STATIC tinyxml2.cpp tinyxml2.h)
set_target_properties(tinyxml2_static PROPERTIES
        COMPILE_DEFINITONS "TINYXML2_EXPORT"
        VERSION "${GENERIC_LIB_VERSION}"
        SOVERSION "${GENERIC_LIB_SOVERSION}")
set_target_properties( tinyxml2_static PROPERTIES OUTPUT_NAME tinyxml2 )

target_compile_definitions(tinyxml2_static PUBLIC -D_CRT_SECURE_NO_WARNINGS)

if(DEFINED CMAKE_VERSION AND NOT "${CMAKE_VERSION}" VERSION_LESS "2.8.11")
    target_include_directories(tinyxml2_static PUBLIC 
                          $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
                          $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)

    if(MSVC)
      target_compile_definitions(tinyxml2_static PUBLIC -D_CRT_SECURE_NO_WARNINGS)
    endif(MSVC)
else()
    include_directories(${PROJECT_SOURCE_DIR})

    if(MSVC)
      add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    endif(MSVC)
endif()
