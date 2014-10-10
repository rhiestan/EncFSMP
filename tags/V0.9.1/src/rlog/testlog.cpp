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
		                                                                                

#include <rlog/rlog.h>
#include <rlog/rloglocation.h>
#include <rlog/RLogChannel.h>
#include <rlog/RLogTime.h>

#include <iostream>

using namespace rlog;
using namespace std;


// setup our own log handler to catch messages

#define Warn10(base,limit) \
    { rWarning("%i of %i", 1+base, limit);  }\
    { rWarning("%i of %i", 2+base, limit);  }\
    { rWarning("%i of %i", 3+base, limit);  }\
    { rWarning("%i of %i", 4+base, limit);  }\
    { rWarning("%i of %i", 5+base, limit);  }\
    { rWarning("%i of %i", 6+base, limit);  }\
    { rWarning("%i of %i", 7+base, limit);  }\
    { rWarning("%i of %i", 8+base, limit);  }\
    { rWarning("%i of %i", 9+base, limit);  }\
    { rWarning("%i of %i", 10+base, limit); }

#define Warn100(base, limit) \
    Warn10(base,limit) \
    Warn10(10+base,limit) \
    Warn10(20+base,limit) \
    Warn10(30+base,limit) \
    Warn10(40+base,limit) \
    Warn10(50+base,limit) \
    Warn10(60+base,limit) \
    Warn10(70+base,limit) \
    Warn10(80+base,limit) \
    Warn10(90+base,limit)

#define Warn1000(base, limit) \
    Warn100(base,limit) \
    Warn100(100+base,limit) \
    Warn100(200+base,limit) \
    Warn100(300+base,limit) \
    Warn100(400+base,limit) \
    Warn100(500+base,limit) \
    Warn100(600+base,limit) \
    Warn100(700+base,limit) \
    Warn100(800+base,limit) \
    Warn100(900+base,limit)


const int WarnCount = 200;
int testfunc()
{
    // NOTE:  this results in all the warning messages on the same line!  Since
    // the line determines the message id, they end up all being called the
    // same thing (examine the pre-processed output).   

    // This doesn't actually affect anything (or make the timings come out any
    // better, since the variable locations are statics in mutually private
    // scopes.  So they are still individually stored and checked, just that
    // they all have the same signature so we couldn't subscribe to one and not
    // others (same component, same file, function, line...).

    // You'd get the same timings by putting all rWarning messages on their own
    // lines (yes, I tried), it just clutters this test code...
    Warn100(0, WarnCount);
    Warn100(100, WarnCount);

    return 0;
}

class TestSubscriber : public RLogNode
{
public:
    TestSubscriber()
    {
	count = 0;
    }
    virtual ~TestSubscriber()
    {
    }
    
    void subscribe( RLogNode *node )
    {
	addPublisher( node );	
	node->isInterested( this, true );
    }


    virtual void publish( const RLogData &data )
    {
	(void)data;
	++count;
    }

    int count;

    RLOG_NO_COPY(TestSubscriber);
};


int main(int argc, char **argv)
{
    RLogInit( argc, argv );

    rDebug("debug test");
    rWarning("warning test");
    rError("error test");

    cerr << "timing calls with a test function containing " << WarnCount 
	<< " warning messages\n";
    cerr << endl;

    rlog_time start, end;
    rlog_get_time(&start);
    testfunc();
    rlog_get_time(&end);
    cerr << "initialization time per msg = approx " 
	<< rlog_time_diff(end,start) / WarnCount << " " 
	<< rlog_time_unit() << "\n";

    cerr << endl;
    cerr << "calling testfunc() a few times with all dormant messages:\n";
    bool warn = false;
    for(int i=0; i<4; ++i)
    {
	rlog_get_time(&start);
	testfunc();
	rlog_get_time(&end);
	double avg = (double)rlog_time_diff(end, start) / (double)WarnCount;
	cerr << "overhead for each dormant msg averaged "
	    << avg << " " << rlog_time_unit() << "\n";
	if(avg < 1)
	    warn = true;
    }
    cerr << endl;

    if(warn)
    {
	cerr << "Note: reconfigure with --enable-rdtsc to use more "
	    << "precise timer " << endl;
    }

    // now enable a subscriber and time how long it takes both to subscribe,
    // and to actually publish the messages..
    TestSubscriber subscriber;

    rlog_get_time(&start);
    subscriber.subscribe( FileNode::Lookup( __FILE__ ));
    rlog_get_time(&end);
    
    cerr << "subscription overhead per msg = approx "
	<< rlog_time_diff(end , start) / WarnCount 
	<< " " << rlog_time_unit() << "\n";
   
    
    cerr << endl;
    cerr << "calling testfunc() a few times with all active messages:\n";
    // time how long it takes to actually publish the error messages..
    // This also prints out the call count, which should be equal to the number
    // of warning messages times the number of loops here..
    for(int i=0; i<4; ++i)
    {
	rlog_get_time(&start);
	testfunc();
	rlog_get_time(&end);
    
	cerr << "overhead for each active msg = approx "
    	    << rlog_time_diff(end , start) / WarnCount 
	    << " " << rlog_time_unit() << "\n";
    }

    return 0;
}


