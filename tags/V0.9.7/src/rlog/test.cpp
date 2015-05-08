/*****************************************************************************
 * Author:   Valient Gough <vgough@pobox.com>
 *
 *****************************************************************************
 * Copyright (c) 2003-2004, Valient Gough
 * Copyright (c) 2004, Vadim Zeitlin (Win32)
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
#include "Error.h"
#include "RLogChannel.h"
#include "StdioNode.h"
#include "RLogTime.h"

#include <stdio.h>
#include <memory.h>

using namespace rlog;
using namespace std;

static RLogChannel *RLTest = RLOG_CHANNEL( "test/heartbeat" );

rlog_time_interval sleepTime(int seconds)
{
    rlog_time start, end;

    rlog_get_time( &start );
    sleep(seconds);
    rlog_get_time( &end );

    return rlog_time_diff( end, start );
}

void testFunc()
{
    bool testAssert = false;
    rAssertSilent( testAssert == true );
}

int main(int argc, char **argv)
{
    RLogInit( argc, argv );

    StdioNode stdLog;

    rlog_time start, end;
    rlog_get_time( &start );

    // subscribe to various things individually
    stdLog.subscribeTo( RLOG_CHANNEL("debug") );
    stdLog.subscribeTo( GetGlobalChannel("warning") );
    stdLog.subscribeTo( RLOG_CHANNEL("error") );
    stdLog.subscribeTo( GetGlobalChannel("test") );

    // subscribe to *everything* (root type node corresponds to all types, and
    // all publications belong to a type)
    //stdLog.subscribeTo( RLOG_CHANNEL("") );

    // subscribe to all messages from this file
    //stdLog.subscribeTo( FileNode::Lookup( __FILE__ ));
    
    rlog_get_time( &end );
    rDebug("time for StdioNode subscription setup: %i %s ", 
	    (int)rlog_time_diff(end, start), rlog_time_unit());

    rDebug("in main()");
    rWarning("testing rWarning");
    rError("testing rError");

    // test assert
    rDebug("the following assert failure is just a test");
    try
    {
	testFunc();
    } catch( Error &err )
    {
	rDebug("caught assert failure from %s line %i",
		err.file(), err.line());
        rDebug("failure in function %s, component %s",
		err.function(), err.component());
    }


    rlog_get_time( &start );

    /*
	Example of short circuting data.  If there are no subscribers to the
	following rLog() calls, then sleepTime() will not be called.  Since we
	subscribed above, it will report how long we slept for..
    */
    const int maxLoop = 4;
    for(int i=0; i<maxLoop; ++i)
    {
	//cerr << "in test loop" << endl;
	rLog( RLTest, "loop %i of %i, sleepTime(1) = %i %s", 
		i, maxLoop, static_cast<int>(sleepTime(1)), rlog_time_unit());
    }
    
    rlog_get_time( &end );

    rlog_time_interval totalTime = rlog_time_diff(end, start);

    /*
        Braces are needed as otherwise the code can't be compiled with
        compilers not supporting variadic macros.
     */
    if(totalTime == -1L)
    {
	rWarning("This message should never be seen!!");
    }

    rDebug("leaving main(), total time = %lli %s ", totalTime, rlog_time_unit());

    return 0;
}



