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
		                                                                                

#ifndef _rloglocation_incl_
#define _rloglocation_incl_

#include <string>

#include <rlog/rlog.h>
#include <rlog/RLogNode.h>

namespace rlog
{
    // documentation in implementation file
    class RLOG_DECL FileNode : public RLogNode
    {
    public:
	FileNode(const char *componentName, const char *fileName);
	FileNode(const char *fileName);
	virtual ~FileNode();

	static FileNode *Lookup( const char *componentName, 
		                 const char *fileName );
	static FileNode *Lookup( const char *fileName );

	std::string componentName;
	std::string fileName;

	RLOG_NO_COPY(FileNode);
    };

}


#endif

