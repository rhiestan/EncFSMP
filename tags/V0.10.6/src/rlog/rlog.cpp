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
		                                                                                

#include "rlog.h"
#include "rloglocation.h"
#include "RLogPublisher.h"

#include <stdio.h>
#include <stdarg.h>

#include "RLogChannel.h"
#include "Error.h"

#include "Lock.h"

#define UNUSED(x) (void)(x)

#if USE_VALGRIND
#include <valgrind/valgrind.h>
#endif

using namespace std;
using namespace rlog;


extern "C" 
int RLogVersion()
{
    return CURRENT_RLOG_VERSION;
}

PublishLoc::~PublishLoc()
{
    disable();
    if(pub != NULL)
    {
	delete pub;
	pub = NULL;
    }
}

void rlog::RLog_Register(PublishLoc *loc, RLogChannel *channel,
	const char *format, ... )
{
    static Mutex registrationLock;
    Lock lock( &registrationLock );

    loc->channel = channel;

    RLogPublisher *pub = new RLogPublisher(loc);

    loc->pub = pub;
    loc->publish = RLogPublisher::Publish;

    if(pub->enabled())
    {
	loc->enable();

	// pass through to the publication function since it is active at
	// birth.
	va_list args;
	va_start (args, format);
	RLogPublisher::PublishVA( loc, channel, format, args );
	va_end( args );
    } else
	loc->disable();
} 

/*
    throw an error structure with the file/line/ component data, etc.
*/
void rlog::rAssertFailed(const char *component, const char *file, 
	const char *function, int line, const char *conditionStr)
{
#if USE_VALGRIND
    VALGRIND_PRINTF_BACKTRACE("Assert failed: %s", conditionStr);
#endif
    throw Error( component, file, function, line, conditionStr );
}


#if !C99_VARIADAC_MACROS && !PREC99_VARIADAC_MACROS

_rMessageProxy::_rMessageProxy( rlog::RLogChannel *channel,
				rlog::PublishLoc *loc )
{
    this->loc = loc;
    loc->channel = channel;
}

void _rMessageProxy::log(const char *format, ...)
{
    va_list ap;
    va_start( ap, format );
    doLog(format, ap);
    va_end( ap );
}

void _rMessageProxy::doLog(const char *format, va_list ap)
{
    if(!loc->pub)
    {
	loc->publish = RLogPublisher::Publish;

	RLogPublisher *pub = new RLogPublisher(loc);
	loc->pub = pub;

	if(pub->enabled())
	    loc->enable();
	else
	    loc->disable();
    }

    if(loc->isEnabled())
	RLogPublisher::PublishVA( loc, loc->channel, format, ap );
}

void _rMessageProxy::log(RLogChannel *channel, const char *format, ...)
{
    loc->channel = channel;
    va_list ap;
    va_start( ap, format );
    doLog(format, ap);
    va_end( ap );
}

#endif
