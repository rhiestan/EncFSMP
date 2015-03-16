/*****************************************************************************
 * Author:   Valient Gough <vgough@pobox.com>
 *
 *****************************************************************************
 * Copyright (c) 2002-2004, Valient Gough
 *
 * This library is free software; you can distribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL), as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the LGPL in the file COPYING for more
 * details.
 *
 */
		                                                                                

#ifndef _rlog_incl_
#define _rlog_incl_

#define CONCAT2(A,B) A##B
#define CONCAT(A,B) CONCAT2(A,B)
#define STR(X) #X

#include <rlog/common.h>

/*! @file rlog.h
  @brief Defines macros for debug, warning, and error messages.

*/

// may be useful for checking from configuration files
#define CURRENT_RLOG_VERSION 20040503
extern "C" int RLogVersion();

namespace rlog
{

    class RLogChannel;
    class RLogPublisher;
    class RLogNode;

    /*! @enum LogLevel
      Logging level definitions.
    */
    enum LogLevel
    {
	Log_Undef    =0, //!< undefined level
	Log_Critical,    //!< critical conditions
	Log_Error,       //!< error conditions
	Log_Warning,     //!< warning conditions
	Log_Notice,      //!< normal, but significant, condition
	Log_Info,        //!< informational
	Log_Debug        //!< debug level messages
    };

    /*! @fn RLogInit(int &argc, char **argv)
      @brief Initializer for external rlog modules.

      This should be called at the beginning of a program which uses RLog.  If
      it isn't called, the logging should still work anyway although some
      external features may not.

      This allows command line arguments to be passed to any rlog modules which
      might want to examine them.
    */
    void RLOG_DECL RLogInit(int &argc, char **argv);


    // Get channel with a particular component name
    RLOG_DECL RLogChannel *GetComponentChannel(const char *component, 
	                             const char *path, 
				     LogLevel level = Log_Undef);

    // the global channel receives messages for all components
    RLOG_DECL RLogChannel *GetGlobalChannel( const char *path,
				   LogLevel level = Log_Undef);

    /*! @def DEF_CHANNEL( const char *path, LogLevel level )
      @brief Returns pointer to RLogChannel struct for the given path
      @param path The hierarchical path to the channel.  Elements in the path
      are separated by '/'.

      DEF_CHANNEL gets an existing (or defines a new) log type.  For example
      "debug", "warning", "error" are predefined types.  You might define
      completely new types, like "timing", or perhaps sub-types like
      "debug/timing/foo", depending on your needs.

      Reporting paths do not need to be unique within a project (or even a
      file).

      Channels form a hierarchy.  If one subscribes to "debug", then you also
      get messages posted to more specific types such as "debug/foo".  But if
      you subscribe to a more specific type, such as "debug/foo", then you will
      not receive more general messages such as to "debug".

      Example:
      @code
      #include <rlog/rlog.h>
      #include <rlog/RLogChannel.h>

      static RLogChannel *MyChannel = DEF_CHANNEL("me/mine/allmine",Log_Info);

      func()
      {
          rLog( MyChannel, "this is being sent to my own channel" );
	  rLog( MyChannel, "%s %s", "hello", "world" );
      }

      main()
      {
          // log all messages to the "me" channel to stderr
          StdioNode stdLog( STDERR_FILENO );
          stdLog.subscribeTo( RLOG_CHANNEL ("me") );

          func();
      }
      @endcode

      @see test.cpp
     */
#define DEF_CHANNEL(path,level) RLOG_CHANNEL_IMPL(RLOG_COMPONENT, path, level)
#define RLOG_CHANNEL(path) RLOG_CHANNEL_IMPL(RLOG_COMPONENT, path, rlog::Log_Undef)
#define RLOG_CHANNEL_IMPL(COMPONENT,path,level) \
    rlog::GetComponentChannel(STR(COMPONENT),path,level)

    /*
	Pre-defined channels, 
	"debug", "warning", and "error".

	You can of course defined sub-channels based on the predefined types,
	such as "debug/level1", or whatever.
    */
    extern RLOG_DECL RLogChannel *_RLDebugChannel;
    extern RLOG_DECL RLogChannel *_RLInfoChannel;
    extern RLOG_DECL RLogChannel *_RLWarningChannel;
    extern RLOG_DECL RLogChannel *_RLErrorChannel;

    /*! @struct PublishLoc <rlog/rlog.h>
      @brief Internal RLog structure - static struct for each rLog() statement

      @internal
      Structure created for each log location to keep track of logging state
      and associated data.

      Only static members are initialized at build time, which is why
      RLogChannel is passed as argument.  Otherwise entire structure will have
      to be initialized at run-time which adds extra code and a guard variable
      for the struct.
    */
    struct PublishLoc
    {
	bool *enabled;
	// If the compiler supports printf attribute specification on function
	// pointers, we'll use it here so that the compiler knows to check for
	// proper printf formatting.  If it doesn't support it, then we'll
	// force the check by inserting a bogus inline function..
	//! function to call when we reach the log statement.
	void (*publish)(PublishLoc *, RLogChannel *, const char *format, ... )
	    PRINTF_FP(3,4);
	RLogNode *pub;
	const char *component;
	const char *fileName;
	const char *functionName;
	int lineNum;
	RLogChannel *channel;

	inline void enable() { *enabled = true; }
	inline void disable() { *enabled = false; }
	inline bool isEnabled() { return *enabled; }

	~PublishLoc();
    };

    /*! @fn RLog_Register
      @brief Internal RLog function - registers a log statement. 
      
      This is used by the rDebug(), rInfo(), rWarning(), rError() macros.
      @internal
     */
    void RLOG_DECL RLog_Register(PublishLoc *loc, RLogChannel *, 
	               const char *format, ... ) PRINTF(3,4);

    /*! @fn rAssertFailed
      @brief Internal RLog function - helper function for rAssert macro.
    
      @internal
    */
    void RLOG_DECL rAssertFailed( const char *component, const char *file, 
	                const char *function, int line, 
	                const char *conditionStr );

    // if we don't have printf attributes on function pointers, but we do have
    // printf attributes, then make a bogus check function..
#if !HAVE_PRINTF_FP && HAVE_PRINTF_ATTR
    void __checkArgs(int, const char *, ... ) PRINTF(2,3);

    inline void __checkArgs(int, const char *, ...)
    { }
#endif
}

#if C99_VARIADAC_MACROS
#include <rlog/rlog-c99.h>
#elif PREC99_VARIADAC_MACROS
#include <rlog/rlog-prec99.h>
#else
#include <rlog/rlog-novariadic.h>
#endif


#define _rAssertFailed(COMPONENT, COND) \
    rlog::rAssertFailed(STR(COMPONENT),__FILE__,__FUNCTION__,__LINE__, COND)

/*! @def rAssert( cond )
    @brief Assert condition - throws error if @a cond evaluates to false.

    Assert error condition.  Displays error message if condition does not
    evaluate to TRUE.

    We throw an error from rAssertFailed.  It isn't done inline so that we
    don't have to include the STL exception code here and bloat callers that
    don't use it or don't care.
*/
#define rAssert( cond ) \
    do { \
	if( unlikely((cond) == false) ) \
	{ rError( "Assert failed: " STR(cond) ); \
	  _rAssertFailed(RLOG_COMPONENT, STR(cond)); \
	} \
    } while(0)

/*! @def rAssertSilent( cond )
    @brief Assert condition - throws error if @a cond evaluates to false, but
    does not display error message.

    Assert error condition.  Similar to rAssert except that it does not display
    an error.
*/
#define rAssertSilent( cond ) \
    do { \
	if( unlikely((cond) == false) ) \
	{ _rAssertFailed(RLOG_COMPONENT, STR(cond)); } \
    } while(0)


/*! @}
*/


/* @def RLOG_NO_COPY
    @brief Disables class copy constructor and operator =.

    This macro declares a private copy constructor and assignment operator
    which prevents automatic generation of these operation by the compiler.

    Attention, it switches access to private, so use it only at the end of the
    class declaration.
    */
#define RLOG_NO_COPY(CNAME) \
    private: \
        CNAME(const CNAME&); \
	CNAME & operator = (const CNAME &)


#endif // rlog.h

