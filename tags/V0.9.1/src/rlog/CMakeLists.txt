cmake_minimum_required(VERSION 2.6)
project(rlog)

#ADD_DEFINITIONS(-DBUILD_LIB)
INCLUDE_DIRECTORIES("..")
ADD_DEFINITIONS(-DRLOG_COMPONENT="encfs")

IF(MSVC)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
	SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP" )
	SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP" )
	SET( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Ox /Ob2 /Oi /Ot /GL /GS- /fp:fast /MP" )
	SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Ob2 /Oi /Ot /GL /GS- /fp:fast /MP" )
ENDIF(MSVC)
IF(CMAKE_COMPILER_IS_GNUCC)
	SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic" )
	SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic" )
	SET( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -Wall -pedantic -funroll-loops -ffast-math -fsched-spec-load -fomit-frame-pointer" )
	SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall -pedantic -funroll-loops -ffast-math -fsched-spec-load -fomit-frame-pointer" )
ENDIF(CMAKE_COMPILER_IS_GNUCC)

SET(ALL_SRC RLogChannel.cpp Error.cpp rlog.cpp rloginit.cpp
	rloglocation.cpp RLogNode.cpp RLogPublisher.cpp
	RLogTime.cpp StdioNode.cpp )

SET(ALL_HEADERS common.h Error.h Lock.h Mutex.h
	rlog-c99.h rlog-novariadic.h rlog-prec99.h rlog.h
	RLogChannel.h rloginit.h rloglocation.h RLogNode.h
	RLogPublisher.h RLogTime.h StdioNode.h )

ADD_LIBRARY(rlog STATIC ${ALL_SRC} ${ALL_HEADERS})
