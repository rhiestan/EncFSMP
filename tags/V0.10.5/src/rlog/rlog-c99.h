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

/*! @def _rMessageDef
  Defines a static RLogPublisher and points it to the registration function for
  the first call.
  @internal
*/
#define _rMessageDef(ID, COMPONENT) \
  static rlog::PublishLoc ID RLOG_SECTION = {& ID ## _enabled, \
      &rlog::RLog_Register, 0, STR(COMPONENT), __FILE__, \
      __FUNCTION__, __LINE__, 0};

/*! @def _rMessageCall
  Checks if the RLogPublisher is enabled and publishes the message if so.
  @internal
*/
#if HAVE_PRINTF_FP || !HAVE_PRINTF_ATTR
#  define _rMessageCall(ID, COMPONENT, CHANNEL, ...) \
  static bool ID ## _enabled = true; \
  if(unlikely(ID ## _enabled)) \
  { \
    _rMessageDef(ID, COMPONENT) \
    (*ID.publish)( &ID, CHANNEL, ##__VA_ARGS__ ); \
  }
#else // no PRINTF attributes..
# define _rMessageCall(ID, COMPONENT, CHANNEL, ...) \
  static bool ID ## _enabled = true; \
  if(unlikely(ID ## _enabled))  \
  { \
    _rMessageDef(ID, COMPONENT) \
    (*ID.publish)( &ID, CHANNEL, ##__VA_ARGS__ ); \
    rlog::__checkArgs( 0, ##__VA_ARGS__ ); \
  }
#endif

/*! @def _rMessage(ID, CHANNEL, ... )
 
  Combines the publisher definition (_rMessageDef) and invokation
  (_rMessageCall)

  enclose in do{}while(0) to insure that it acts as a single statement even if
  placed in various if/else constructs..
  @internal
*/
#define _rMessage(ID, CHANNEL, ... ) \
  do { _rMessageCall(ID, RLOG_COMPONENT, CHANNEL, ##__VA_ARGS__ ) } while(0)

/*! @addtogroup RLogMacros
  These macros are the primary interface for logging messages:
  - rDebug(format, ...)
  - rInfo(format, ...)
  - rWarning(format, ...)
  - rError(format, ...)
  - rLog(channel, format, ...)
  - rAssert( condition )
  @{
*/

/*! @def rDebug(format, ...)
    @brief Log a message to the "debug" channel.  Takes printf style arguments.

    Format is ala printf, eg:
    @code
    rDebug("I'm sorry %s, I can't do %s", name, request);
    @endcode

    When using a recent GNU compiler, it should automatically detect format
    string / argument mismatch just like it would with printf.

    Note that unless there are subscribers to this message, it will do nothing.
*/
#define rDebug(...) \
  _rMessage( LOGID, rlog::_RLDebugChannel, ##__VA_ARGS__ )

/*! @def rInfo(format, ...)
    @brief Log a message to the "debug" channel.  Takes printf style arguments.

    Format is ala printf, eg:
    @code
    rInfo("I'm sorry %s, I can't do %s", name, request);
    @endcode

    When using a recent GNU compiler, it should automatically detect format
    string / argument mismatch just like it would with printf.

    Note that unless there are subscribers to this message, it will do nothing.
*/
#define rInfo(...) \
  _rMessage( LOGID, rlog::_RLInfoChannel, ##__VA_ARGS__ )

/*! @def rWarning(format, ...)
    @brief Log a message to the "warning" channel.  Takes printf style
    arguments.

    Output a warning message - meant to indicate that something doesn't seem
    right.

    Format is ala printf, eg:
    @code
    rWarning("passed %i, expected %i, continuing", foo, bar);
    @endcode

    When using a recent GNU compiler, it should automatically detect format
    string / argument mismatch just like it would with printf.

    Note that unless there are subscribers to this message, it will do nothing.
*/
#define rWarning(...) \
  _rMessage( LOGID, rlog::_RLWarningChannel, ##__VA_ARGS__ )

/*! @def rError(...)
    @brief Log a message to the "error" channel. Takes printf style arguments.

    An error indicates that something has definately gone wrong.

    Format is ala printf, eg:
    @code
    rError("bad input %s, aborting request", input);
    @endcode

    When using a recent GNU compiler, it should automatically detect format
    string / argument mismatch just like it would with printf.

    Note that unless there are subscribers to this message, it will do nothing.
*/
#define rError(...) \
  _rMessage( LOGID, rlog::_RLErrorChannel, ##__VA_ARGS__ )

/*! @def rLog(channel,format,...)
    @brief Log a message to a user defined channel. Takes a channel and printf
    style arguments.

    An error indicates that something has definately gone wrong.

    Format is ala printf, eg:
    @code
    static RLogChannel * MyChannel = RLOG_CHANNEL( "debug/mine" );
    rLog(MyChannel, "happy happy, joy joy");
    @endcode

    When using a recent GNU compiler, it should automatically detect format
    string / argument mismatch just like it would with printf.

    Note that unless there are subscribers to this message, it will do nothing.
*/
#define rLog(channel, ...) \
  _rMessage( LOGID, channel, ##__VA_ARGS__ )


