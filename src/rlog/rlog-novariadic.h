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

#include <stdarg.h> // for va_list
#include <stdio.h> // for NULL

class RLOG_DECL _rMessageProxy
{
public:
    _rMessageProxy( rlog::RLogChannel *channel, rlog::PublishLoc *loc );

    void log(const char *format, ...) PRINTF(2,3);
    void log(rlog::RLogChannel *channel, const char *format, ...) PRINTF(3,4);

private:
    void doLog(const char *format, va_list ap);

    rlog::PublishLoc *loc;
};

#define _rMessageDef(ID, COMPONENT) \
  static bool ID ## _enabled = true; \
  static rlog::PublishLoc ID RLOG_SECTION = {& ID ## _enabled, \
      &rlog::RLog_Register, 0, STR(COMPONENT), __FILE__, \
      __FUNCTION__, __LINE__, 0}

#define _rMessage(ID, CHANNEL, COMPONENT) \
  _rMessageDef(ID, COMPONENT); \
  _rMessageProxy(CHANNEL, &ID).log

#define rDebug _rMessage( LOGID, rlog::_RLDebugChannel, RLOG_COMPONENT  )
#define rInfo _rMessage( LOGID, rlog::_RLDebugChannel, RLOG_COMPONENT )
#define rWarning _rMessage( LOGID, rlog::_RLWarningChannel, RLOG_COMPONENT )
#define rError _rMessage( LOGID, rlog::_RLErrorChannel, RLOG_COMPONENT )

#define rLog _rMessage ( LOGID, NULL, RLOG_COMPONENT)

