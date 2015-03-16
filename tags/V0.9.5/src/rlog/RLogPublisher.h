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
		                                                                                

#ifndef _RLogPublisher_incl_
#define _RLogPublisher_incl_

#include <rlog/common.h>
#include <rlog/RLogNode.h>

#include <stdarg.h>

namespace rlog
{
    class RLOG_DECL RLogChannel;

    // documentation in implementation file
    class RLOG_DECL RLogPublisher : public RLogNode
    {
    public:
	RLogPublisher(PublishLoc *src);
	RLogPublisher();
	virtual ~RLogPublisher();

	// metadata about the publisher and its publication
	PublishLoc *src;
   
	static void Publish( PublishLoc *, RLogChannel *, 
		             const char *format, ... );
	static void PublishVA( PublishLoc *, RLogChannel *, 
		             const char *format, va_list args );

    protected:
	virtual void setEnabled(bool newState);

	RLogPublisher( const RLogPublisher & );
	RLogPublisher & operator = ( const RLogPublisher & );
    };

} // namespace rlog

#endif

