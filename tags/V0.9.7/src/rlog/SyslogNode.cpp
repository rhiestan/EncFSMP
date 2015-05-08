/*****************************************************************************
 * Author:   Valient Gough <vgough@pobox.com>
 *
 *****************************************************************************
 * Copyright (c) 2004, Valient Gough
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
		                                                                                
                             
#include "SyslogNode.h"
#include "RLogPublisher.h"
#include "RLogChannel.h"

#include <string>
#include <syslog.h>

using namespace rlog;
using namespace std;


/*! @class rlog::SyslogNode <rlog/SyslogNode.h>
  @brief Logs subscribed messages using syslog.

  This logs all subscribed messages using syslog. 

  For example:
  @code
  int main(int argc, char **argv)
  {
      // tell RLog the program name..
      RLog_Init( argv[0] );

      // log to syslog, prepending "rlog-test" to every line
      SyslogNode slog( "rlog-test" );

      // show all warning and error messages, no matter what component they
      // come from.
      stdlog.subscribeTo( GetGlobalChannel( "warning" ));
      stdlog.subscribeTo( GetGlobalChannel( "error" ));
  }
  @endcode

  @see RLogChannel
  @see RLOG_CHANNEL()
  @author Valient Gough
*/

SyslogNode::SyslogNode(const char *_ident)
    : RLogNode()
    , ident( _ident )
    , option( 0 )
    , facility( LOG_USER )
{
    openlog( ident, option, facility );
}

SyslogNode::SyslogNode(const char *_ident, int _option, int _facility)
    : RLogNode()
    , ident( _ident )
    , option( _option )
    , facility( _facility )
{
    openlog( ident, option, facility );
}

SyslogNode::~SyslogNode()
{
    closelog();
}

void
SyslogNode::subscribeTo( RLogNode *node )
{
    addPublisher( node );

    node->isInterested( this, true );
}

void
SyslogNode::publish( const RLogData &data )
{
    int level = LOG_INFO;

    switch( data.publisher->channel->logLevel() )
    {
    case Log_Critical:
	level = LOG_CRIT;
	break;
    case Log_Error:
	level = LOG_ERR;
	break;
    case Log_Warning:
	level = LOG_WARNING;
	break;
    case Log_Notice:
	level = LOG_NOTICE;
	break;
    case Log_Info:
	level = LOG_INFO;
	break;
    case Log_Debug:
    case Log_Undef:
	level = LOG_DEBUG;
	break;
    }

    int priority = facility | level;
    syslog( priority, "%s", data.msg );
}


