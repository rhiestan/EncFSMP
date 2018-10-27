cmake_minimum_required(VERSION 2.8.12)

project(Easyloggingpp CXX)

macro(require_cpp11)
        if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.0)
                # CMake 3.1 has built-in CXX standard checks.
                message("-- Setting C++11")
                set(CMAKE_CXX_STANDARD 11)
                set(CMAKE_CXX_STANDARD_REQUIRED on)
        else()
                if (CMAKE_CXX_COMPILER_ID MATCHES "GCC")
                    message ("-- GNU CXX (-std=c++11)")
                    list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
                elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                    message ("-- CLang CXX (-std=c++11)")
                    list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
                else()
                    message ("-- Easylogging++ requires C++11. Your compiler does not support it.")
                endif()
        endif()
endmacro()

set(ELPP_MAJOR_VERSION "9")
set(ELPP_MINOR_VERSION "95")
set(ELPP_PATCH_VERSION "0")
set(ELPP_VERSION_STRING "${ELPP_MAJOR_VERSION}.${ELPP_MINOR_VERSION}.${ELPP_PATCH_VERSION}")

set(ELPP_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "The directory the headers are installed in")

require_cpp11()
add_library(easyloggingpp STATIC easylogging++.cc)
target_include_directories(easyloggingpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Disable default log file, thread-safe, Unicode
target_compile_definitions(easyloggingpp PUBLIC ELPP_NO_DEFAULT_LOG_FILE ELPP_THREAD_SAFE ELPP_UNICODE)

