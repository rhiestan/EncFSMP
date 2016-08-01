
INCLUDES = $(all_includes) $(USER_INCLUDES) -I$(top_srcdir)
# build everything in the "rlog" component.. 
AM_CXXFLAGS = -DRLOG_COMPONENT="rlog" -DUSE_VALGRIND=@USE_VALGRIND@


lib_LTLIBRARIES = librlog.la

librlog_la_SOURCES = \
    rlog.cpp \
    rloginit.cpp \
    rloglocation.cpp \
    Error.cpp \
    RLogChannel.cpp \
    RLogNode.cpp \
    RLogPublisher.cpp \
    RLogTime.cpp \
    StdioNode.cpp \
    SyslogNode.cpp

librlog_la_LDFLAGS = -version-info $(LIBRLOG_VERSION) $(USER_LDFLAGS)

librlogincludedir=$(includedir)/rlog
librloginclude_HEADERS = \
    common.h \
    Error.h \
    Lock.h \
    Mutex.h \
    rlog-c99.h \
    RLogChannel.h \
    rlog.h \
    rloginit.h \
    rloglocation.h \
    RLogNode.h \
    rlog-novariadic.h \
    rlog-prec99.h \
    RLogPublisher.h \
    RLogTime.h \
    StdioNode.h \
    SyslogNode.h


noinst_PROGRAMS = test testlog

test_SOURCES = test.cpp
test_LDADD = librlog.la 

testlog_SOURCES = testlog.cpp
testlog_LDADD = librlog.la
testlog_CXXFLAGS = $(AM_CXXFLAGS)

