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

#define _rMessageDef(ID, COMPONENT) \
  static bool ID ## _enabled = true; \
  static rlog::PublishLoc ID RLOG_SECTION = {& ID ## _enabled, \
      &rlog::RLog_Register, 0, STR(COMPONENT), __FILE__, \
      __FUNCTION__, __LINE__, 0};


#if HAVE_PRINTF_FP || !HAVE_PRINTF_ATTR
#  define _rMessageCall(ID, CHANNEL, ARGS...) \
  if(unlikely(ID ## _enabled)) (*ID.publish)( &ID, CHANNEL, ##ARGS );
#else
# define _rMessageCall(ID, CHANNEL, ARGS...) \
  if(unlikely(ID ## _enabled))  \
  { \
    (*ID.publish)( &ID, CHANNEL, ##ARGS ); \
    rlog::__checkArgs( 0, ##ARGS ); \
  }
#endif



#define _rMessage(ID, CHANNEL, ARGS... ) \
  do { _rMessageDef(ID, RLOG_COMPONENT) \
       _rMessageCall(ID, CHANNEL, ##ARGS ) } while(0)


#define rDebug(ARGS...) \
  _rMessage( LOGID, rlog::_RLDebugChannel, ##ARGS )

#define rInfo(ARGS...) \
  _rMessage( LOGID, rlog::_RLInfoChannel, ##ARGS )

#define rWarning(ARGS...) \
  _rMessage( LOGID, rlog::_RLWarningChannel, ##ARGS )

#define rError(ARGS...) \
  _rMessage( LOGID, rlog::_RLErrorChannel, ##ARGS )

#define rLog(channel, ARGS...) \
  _rMessage( LOGID, channel, ##ARGS )


