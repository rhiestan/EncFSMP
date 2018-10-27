cmake_minimum_required(VERSION 2.6)
project(libencfs)

#ADD_DEFINITIONS(-DBUILD_LIB)
INCLUDE_DIRECTORIES("..")
INCLUDE_DIRECTORIES("../tinyxml2")
INCLUDE_DIRECTORIES("../intl")

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

SET(ALL_SRC BlockFileIO.cpp BlockNameIO.cpp Cipher.cpp CipherFileIO.cpp
	CipherKey.cpp ConfigReader.cpp ConfigVar.cpp Context.cpp
	DirNode.cpp Error.cpp FileIO.cpp FileNode.cpp FileUtils.cpp Interface.cpp
	MACFileIO.cpp MemoryPool.cpp NameIO.cpp NullCipher.cpp
	NullNameIO.cpp RawFileIO.cpp SSL_Cipher.cpp StreamNameIO.cpp XmlReader.cpp
	base64.cpp openssl.cpp vasprintf.c )

SET(ALL_HEADERS BlockFileIO.h BlockNameIO.h Cipher.h CipherFileIO.h
	CipherKey.h ConfigReader.h ConfigVar.h Context.h DirNode.h
	Error.h FSConfig.h FileIO.h FileNode.h FileUtils.h Interface.h
	MACFileIO.h MemoryPool.h Mutex.h NameIO.h NullCipher.h
	NullNameIO.h Range.h RawFileIO.h SSL_Cipher.h StreamNameIO.h XmlReader.h
	base64.h i18n.h openssl.h )

ADD_LIBRARY(libencfs STATIC ${ALL_SRC} ${ALL_HEADERS})
target_compile_features(libencfs PRIVATE cxx_range_for cxx_auto_type cxx_deleted_functions cxx_nullptr)
#target_include_directories
TARGET_LINK_LIBRARIES(libencfs easyloggingpp)

# boost-versioning.h
