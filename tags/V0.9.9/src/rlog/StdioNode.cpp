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
		                                                                                

#include "StdioNode.h"

#include "RLogPublisher.h"
#include "RLogChannel.h"
#ifndef _WIN32
#include <pthread.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"

#  ifdef HAVE_SSTREAM
#    include <sstream>
#  else
// old STL - use strstream instead of stringstream
#    include <strstream.h>
#    define USE_STRSTREAM
#  endif

#else
#  include <sstream>
#endif

#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32
#  include <io.h>
#  define write(fd, buf, n) _write((fd), (buf), static_cast<unsigned>(n))
#else
#  include <unistd.h>
#  define USE_COLOURS
#endif

using namespace rlog;
using namespace std;


const char kNormalColor[] = "\033[0m";
const char kRedColor[]    = "\033[31m";
const char kGreenColor[]  = "\033[32m";
const char kYellowColor[] = "\033[33m";


/*! @class rlog::StdioNode <rlog/StdioNode.h>
  @brief Logs subscribed messages to a file descriptor.

  This displays all subscribed messages to a file descriptor.  If the
  output is a terminal, then the messages are colorized (yellow for
  warnings, red for errors).

  For example, to log to stderr:
  @code
  int main(int argc, char **argv)
  {
      // tell RLog the program name..
      RLog_Init( argv[0] );

      // log to standard error
      StdioNode stdlog( STDERR_FILENO );

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

/*! @param _fdOut File descriptor to send output
  @param flags bitmask of the following options: 
  @code
      OutputChannel - Includes the channel name in the output. e.g. [debug]
      OutputContext - Includes the filename and line number in the output. e.g. OpsecAgent.cpp:209
      OutputColor   - Output to a TTY is colored based on log level (not supported by Win32 console)
      OutputThreadId - Includes the thread id in the output e.g. [tid:37333936] (Not supported on Win32)
  @endcode
*/
StdioNode::StdioNode(int _fdOut, int flags)
    : RLogNode()
    , fdOut( _fdOut )
{
    if(flags == DefaultOutput)
	flags = OutputColor | OutputContext;

#ifdef USE_COLOURS
    colorize = (flags & OutputColor) && isatty( fdOut );
#else
    colorize = false;
#endif
    outputThreadId = (flags & OutputThreadId);
    outputContext   = (flags & OutputContext);
    outputChannel   = (flags & OutputChannel);
}

StdioNode::StdioNode(int _fdOut, bool colorizeIfTTY)
    : RLogNode()
    , fdOut( _fdOut )
{
#ifdef USE_COLOURS
    colorize = colorizeIfTTY && isatty( fdOut );
#else
    (void)colorizeIfTTY;
    colorize = false;
#endif
    outputThreadId = false;
    outputContext  = true;
    outputChannel  = false;
}

StdioNode::~StdioNode()
{
}

/*! Subscribe to log messages.

   Example:
   @code
   StdioNode log( STDERR_FILENO );
   // subscribe to error and warning messages
   log.subscribeTo( GetGlobalChannel("error" ));
   log.subscribeTo( RLOG_CHANNEL("warning" ));
   @endcode
*/
void
StdioNode::subscribeTo( RLogNode *node )
{
    addPublisher( node );

    node->isInterested( this, true );
}

void
StdioNode::publish( const RLogData &data )
{
    char timeStamp[32];
    time_t errTime = data.time;
    tm currentTime;
   
#ifdef HAVE_LOCALTIME_R
    localtime_r( &errTime, &currentTime );
#else
    currentTime = *localtime( &errTime );
#endif

    const char *color = NULL;
    if(colorize)
    {
	sprintf(timeStamp, "%s%02i:%02i:%02i%s ",
		kGreenColor,
		currentTime.tm_hour,
		currentTime.tm_min,
		currentTime.tm_sec,
		kNormalColor);
    
	string channel = data.publisher->channel->name();
	LogLevel level = data.publisher->channel->logLevel();

	switch(level)
	{
	case Log_Critical:
	case Log_Error:
	    color = kRedColor;
	    break;
	case Log_Warning:
	    color = kYellowColor;
	    break;
	case Log_Notice:
	case Log_Info:
	case Log_Debug:
	case Log_Undef:
	    break;
	}
    } else
    {
	sprintf(timeStamp, "%02i:%02i:%02i ",
		currentTime.tm_hour,
		currentTime.tm_min,
		currentTime.tm_sec);
    }

#ifdef USE_STRSTREAM
    ostrstream ss;
#else
    ostringstream ss;
#endif

    ss << timeStamp;
    if (outputChannel) {
        ss << '[' << data.publisher->channel->name() << "] ";
    }
    if (outputContext) {
    ss << "(" << data.publisher->fileName << ':'
	<< data.publisher->lineNum << ") ";
    }
#ifndef _WIN32
    if (outputThreadId) {
        char tid[16] = "";
        snprintf(tid,15,"%p",pthread_self());
        ss << "[tid:" << tid << "] ";
    }
#endif

    if(color)
	ss << color;

    ss << data.msg;
    
    if(color)
	ss << kNormalColor;

    ss << '\n';

    string out = ss.str();
    write( fdOut, out.c_str(), out.length() );
}

