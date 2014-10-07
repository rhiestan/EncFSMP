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
		                                                                                
                             
#ifndef _StdioNode_incl_
#define _StdioNode_incl_

#include <rlog/common.h>
#include <rlog/RLogNode.h>

namespace rlog
{
    // documentation in implementation file
    class RLOG_DECL StdioNode : public RLogNode
    {
    public:
	enum StdioFlags
	{
	    DefaultOutput  = 0, // use built-in defaults
	    OutputColor    = 1, //< output with color if sending to a TTY
	    OutputThreadId = 2, //< include the thread id (not on Win32)
	    OutputContext  = 4, //< Include filename and line number in output
	    OutputChannel  = 8  //< Include channel name in output
	};

	StdioNode( int fdOut = 2, int flags = (int)DefaultOutput );
	// for backward compatibility
	StdioNode( int fdOut, bool colorize_if_tty );

	virtual ~StdioNode();

	void subscribeTo( RLogNode *node );

    protected:
	virtual void publish( const RLogData &data );

	bool colorize;
	bool outputThreadId;
	bool outputContext;
	bool outputChannel;
	int fdOut;

	StdioNode( const StdioNode & );
	StdioNode &operator = ( const StdioNode & );
    };

}

#endif
