/*****************************************************************************
 * Author:   Valient Gough <vgough@pobox.com>
 *
 *****************************************************************************
 * Copyright (c) 2003, Valient Gough
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
		                                                                                
                             
#ifndef _RLogChannel_incl_
#define _RLogChannel_incl_

#include <string>
#include <map>

#include <rlog/rlog.h>
#include <rlog/RLogNode.h>

namespace rlog
{
    class RLOG_DECL RLogPublisher;
    class RLOG_DECL RLogChannel;
    typedef std::map<std::string, RLogChannel*> ComponentMap;

    // documentation in implementation file
    class RLOG_DECL RLogChannel : public RLogNode
    {
    public:
	RLogChannel( const std::string &name, LogLevel level );
	virtual ~RLogChannel();


	virtual void publish(const RLogData &data);


	const std::string &name() const;

	LogLevel logLevel() const;
	void setLogLevel(LogLevel level);

    protected:
	friend RLogChannel *rlog::GetComponentChannel( const char *component,
		const char *path, LogLevel level );
	RLogChannel *getComponent(RLogChannel *componentParent, 
		                  const char *component);

    private:
	//! the full channel name
	std::string _name; 
	LogLevel _level;

	ComponentMap subChannels;
	ComponentMap components;

	RLogChannel( const RLogChannel & );
	RLogChannel &operator = ( const RLogChannel & );
    };

}

#endif
