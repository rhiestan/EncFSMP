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
		                                                                                
                             
#ifndef _SyslogNode_incl_
#define _SyslogNode_incl_

#include <rlog/RLogNode.h>

namespace rlog
{
    // documentation in implementation file
    class SyslogNode : public RLogNode
    {
    public:
	SyslogNode( const char *ident );
	//! See syslog(3) for possible values of option and facility arguments.
	SyslogNode( const char *ident, int option, int facility );
	virtual ~SyslogNode();

	void subscribeTo( RLogNode *node );

    protected:
	virtual void publish( const RLogData &data );

	const char *ident;
	int option;
	int facility;
    };

}

#endif
