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
		                                                                                
                             

#include "RLogPublisher.h"

#include "RLogChannel.h"
#include "rloglocation.h"
#include "rlog.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h> // in case we need memcpy

using namespace rlog;


/*! @class rlog::RLogPublisher <rlog/RLogPublisher.h>
  @brief RLog publisher used by rLog macros.

  This derives from RLogNode and interfaces to the static PublishLoc logging
  data allowing them to be enabled or disabled depending on subscriber
  status.

  An instance of this class is created for every error message location.
  Normally this class is not used directly.  

  For example, this
  @code
     rDebug( "hello world" );
  @endcode

  is turned approximatly into this:
  @code
     static bool _rl_enabled = true;
     if(_rL.enabled)
     {
	 static PublishLoc _rl = {
	     enabled:  & _rl_enabled,
	     publish:  & rlog::RLog_Register ,
	     pub: 0,
	     component: "component",
	     fileName: "myfile.cpp",
	     functionName: "functionName()",
	     lineNum: __LINE__,
	     channel: 0
	 };
	 (*_rl.publish)( &_rL, _RLDebugChannel, "hello world" );
     }
  @endcode

  The RLogPublisher instance manages the contents of the static structure
  _rL.  When someone subscribes to it's message, then _rL.publish is set to
  point to the publishing function, and when there are no subscribers then
  enabled flag is set to false.

  The code produced contains one if statement, and with optimization comes
  out to 2 instructions on an x86 computer for the unsubscribed case.
  If there are no subscribers to this message then that is all the overhead,
  plus the memory usage for the structures involved and the initial
  registration when the statement is first encountered..

  @see RLogChannel
  @author Valient Gough
 */


RLogPublisher::RLogPublisher()
{
}

RLogPublisher::RLogPublisher(PublishLoc *loc) 
    : RLogNode()
    , src( loc )
{
    // link to channel for channel based subscriptions
    // Lookup the componentized version
    RLogNode *channelNode = GetComponentChannel( src->component, 
	    src->channel->name().c_str(), src->channel->logLevel() );
    channelNode->addPublisher( this );

    // link to file
    RLogNode *fileNode = FileNode::Lookup( src->component, src->fileName );
    fileNode->addPublisher( this );
}

RLogPublisher::~RLogPublisher()
{
    clear();
}

void
RLogPublisher::setEnabled(bool active)
{
    if(src)
    {
	if(active)
	    src->enable();
	else
	    src->disable();
    }
}

void RLogPublisher::Publish( PublishLoc *loc, RLogChannel *channel,
	const char *format, ...)
{
    va_list args;
    va_start( args, format );
    PublishVA( loc, channel, format, args );
    va_end( args );
}

void RLogPublisher::PublishVA( PublishLoc *loc, RLogChannel *,
	const char *format, va_list ap )
{
    RLogData data;

    data.publisher = loc;
    data.time = time(0);
    data.msg = 0;

    char msgBuf[64];
    char *buf = msgBuf;
    size_t bufSize = sizeof(msgBuf);

    // loop until we have allocated enough space for the message
    for(int numTries = 10; numTries; --numTries)
    {
	va_list args;

	// va_copy() is defined in C99, __va_copy() in earlier definitions, and
	// automake says to fall back on memcpy if neither exist...
	//
	// FIXME: memcpy doesn't work for compilers which use array type for
	//        va_list such as Watcom
#if defined( va_copy )
	va_copy( args, ap );
#elif defined( __va_copy )
	__va_copy( args, ap );
#else
	memcpy( &args, &ap, sizeof(va_list) );
#endif

	int ncpy = vsnprintf( buf , bufSize, format, args );
	va_end( args );

	// if it worked, then return the buffer
	if( ncpy > -1 && ncpy < bufSize )
	{
	    data.msg = buf;
	    break;
	} else
	{
	    // newer implementations of vsnprintf return # bytes needed..
	    if(ncpy > 0) 
		bufSize = ncpy + 1;
	    else
		bufSize *= 2; // try twice as much space as before

	    if(buf != msgBuf)
		delete[] buf;

	    buf = new char[bufSize];
	}
    }

    loc->pub->publish( data );

    if(buf != msgBuf)
	delete[] buf;
}

