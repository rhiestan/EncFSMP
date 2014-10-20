/*****************************************************************************
 * Author:   Valient Gough <vgough@pobox.com>
 *
 *****************************************************************************
 * Copyright (c) 2002-2003, Valient Gough
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

#include <cstring>

#include "RLogChannel.h"
#include "rlog.h"
#include "Lock.h"

using namespace std;
using namespace rlog;

const char GlobalComponent[] = "/";

namespace rlog
{
    static RLogChannel *gRootChannel =0;

    // big lock around channel lookups..
	static Mutex *getChannelLock()			// Changed by RH: Workaround for crash by
	{										// employing "construct on first use" idiom
	    static Mutex *gChannelLock = new Mutex();
		return gChannelLock;
	}
  
    // Use GetComponentChannel here because we want to reference the global
    // versions, not the componentized versions..
    // We'll use 
    RLogChannel *_RLDebugChannel = GetGlobalChannel( "debug", Log_Debug );
    RLogChannel *_RLInfoChannel = GetGlobalChannel( "info", Log_Info );
    RLogChannel *_RLWarningChannel = GetGlobalChannel( "warning", Log_Warning );
    RLogChannel *_RLErrorChannel = GetGlobalChannel( "error", Log_Error );
}


/*! @class rlog::RLogChannel <rlog/RLogChannel.h>
  @brief Implements a hierarchical logging channel

  You should not need to use RLogChannel directly under normal
  circumstances. See RLOG_CHANNEL() macro, GetComponentChannel() and
  GetGlobalChannel()

  RLogChannel implements channel logging support.  A channel is a named
  logging location which is global to the program.  Channels are
  hierarchically related.
  
  For example, if somewhere in your program a message is logged to
  "debug/foo/bar", then it will be delived to any subscribers to
  "debug/foo/bar", or subscribers to "debug/foo", or subscribers to
  "debug".   Subscribing to a channel means you will receive anything
  published on that channel or sub-channels.

  As a special case, subscribing to the channel "" means you will receive
  all messages - as every message has a channel and the empty string "" is
  considered to mean the root of the channel tree.

  In addition, componentized channels are all considered sub channels of
  the global channel hierarchy.  All rDebug(), rWarning(), and rError()
  macros publish to the componentized channels (component defined by
  RLOG_COMPONENT).

  @code
      // get the "debug" channel for our component.  This is the same as
      // what rDebug() publishes to.
      RLogChannel *node = RLOG_CHANNEL( "debug", Log_Debug );
      // equivalent to
      RLogChannel *node = GetComponentChannel( RLOG_COMPONENT, "debug" );

      // Or, get the global "debug" channel, which will have messages from
      // *all* component's "debug" channels.
      RLogChannel *node = GetGlobalChannel( "debug", Log_Debug );
  @endcode

  @author Valient Gough
  @see RLOG_CHANNEL()
  @see GetComponentChannel()
  @see GetGlobalChannel()
*/



RLogChannel::RLogChannel( const string &n, LogLevel level )
    : RLogNode()
    , _name( n )
    , _level( level )
{
}

RLogChannel::~RLogChannel()
{
}

const std::string &RLogChannel::name() const
{
    return _name;
}

LogLevel RLogChannel::logLevel() const
{
    return _level;
}

void RLogChannel::setLogLevel(LogLevel level)
{
    _level = level;
}

RLogChannel *RLogChannel::getComponent( RLogChannel *parent,
	const char *component )
{
    ComponentMap::const_iterator it = components.find( component );
    if(it == components.end())
    {
	RLogChannel *ch = new RLogChannel( _name, _level );
	components.insert( make_pair( component, ch ) );

	// connect to its parent
	if(parent)
	    parent->addPublisher( ch );

	// connect to globalized version
	addPublisher( ch );

	return ch;
    } else
    {
	return it->second;
    }
}

/*! @relates RLogChannel
  @brief Return the named channel across all components.

  Channels are hierarchical.  See RLogChannel for more detail.
  The global channel contains messages for all component channels.

  For example, subscribing to the global "debug" means the subscriber would
  also get messages from <Component , "debug">, and <Component-B, "debug">, and
  <Component-C, "debug/foo">, etc.

  @author Valient Gough
*/
RLogChannel *rlog::GetGlobalChannel( const char *path, LogLevel level )
{
    return GetComponentChannel( GlobalComponent, path, level );
}

/*!  @relates RLogChannel
  @brief Return the named channel for a particular component.

  @author Valient Gough
*/
RLogChannel *rlog::GetComponentChannel( const char *component, 
	const char *path, LogLevel level )
{
    // since we much with the globally visible channel tree, hold a lock..

    Lock lock( getChannelLock() );

    string currentPath;

    if(!gRootChannel)
	gRootChannel = new RLogChannel( "", level );

    RLogChannel *current = gRootChannel;
    RLogChannel *currentComponent = 0;
    if(strcmp( component, GlobalComponent ) != 0)
	currentComponent = gRootChannel->getComponent( 0, component );

    while( *path )
    {
	// if log level is currently undefined but we now know what it is, then
	// define it..
	if((current->logLevel() == Log_Undef) && (level != Log_Undef))
	    current->setLogLevel( level );

	const char *next = strchr( path , '/' );
	size_t len = next ? next - path : strlen( path );

	if(len > 1)
	{
	    string pathEl( path, len );

	    if(!currentPath.empty())
		currentPath += '/';
	    currentPath += pathEl;

	    ComponentMap::const_iterator it = 
		current->subChannels.find( pathEl );
	    if(it != current->subChannels.end())
	    {
		// found.  possibly creating sub-map
		current = it->second;
	    } else
	    {
		// create
		RLogChannel *nm = new RLogChannel( currentPath, level );
		current->subChannels.insert( make_pair(pathEl, nm) );

		current->addPublisher( nm );

		current = nm;
	    }
	    // track componentized version
	    if(currentComponent)
    		currentComponent = current->getComponent( currentComponent,
			component );

	    path += len;
	} else
	{
	    // skip separator character
	    ++path;
	}
    }

    if(currentComponent)
	return currentComponent;
    else
	return current;
}

void
RLogChannel::publish(const RLogData &data)
{
    set< RLogNode* >::const_iterator it = data.seen.find( this );
    if(it == data.seen.end())
    {
	const_cast<RLogData&>(data).seen.insert( this );
	RLogNode::publish( data );
    }
}

