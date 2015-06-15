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
		                                                                                
                             
#include "rloglocation.h"

#include "Lock.h"

#include <map>


using namespace rlog;
using namespace std;

namespace rlog
{
    typedef map<string, FileNode*> FileNodeMap;

    static FileNodeMap gFileMap;

    /*
	locks for global maps
    */
    static Mutex gMapLock;
}


/*! @class rlog::FileNode <rlog/rloglocation.h>
  @brief Provides filename based logging nodes.

  This allows subscribing to messages only from particular files.
  For example,
  @code

  int main()
  {
      // display some messages to stderr
      StdioNode std( STDERR_FILENO );

      // subscribe to everything from this file
      std.subscribeTo( FileNode::Lookup( __FILE__ ) );

      // and everything from "important.cpp"
      std.subscribeTo( FileNode::Lookup( "important.cpp" ));
  }
  @endcode

  Note that file names are not required to be unique across the entire
  program.  Different components may contain the same filename, which is
  why there is a second Lookup function which also takes the component
  name.

  @see RLogChannel
  @author Valient Gough
*/


FileNode::FileNode(const char *_cn, const char *_fileName)
    : RLogNode()
    , componentName( _cn )
    , fileName( _fileName )
{
}

FileNode::FileNode(const char *_fileName)
    : RLogNode()
    , fileName( _fileName )
{
}

FileNode::~FileNode()
{
}

FileNode *
FileNode::Lookup( const char *fileName )
{
    Lock lock( &gMapLock );

    // no component specified, so look for componentless filename node
    FileNodeMap::const_iterator it = gFileMap.find( fileName );

    if(it != gFileMap.end())
    {
	return it->second;
    } else
    {
	// create the componentless filename node.  We can't create a fully
	// componentized version because we don't have a component name..
	FileNode *node = new FileNode( fileName );
	gFileMap.insert( make_pair( fileName, node ));

	return node;
    }
}

FileNode *
FileNode::Lookup(const char *componentName, const char *fileName)
{
    // do this first before we take out the lock
    FileNode *partial = Lookup( fileName );

    Lock lock( &gMapLock );

    // fullName is "[componentName]::[fileName]"
    string fullName = componentName;
    fullName += "::";
    fullName += fileName;

    FileNodeMap::const_iterator it = gFileMap.find( fullName );

    if(it != gFileMap.end())
    {
	return it->second;
    } else
    {
	FileNode *node = new FileNode( componentName, fileName );
	gFileMap.insert( make_pair( fullName, node ));

	// partial node never publishes, but it can forward publications from
	// the fully specified nodes..
	partial->addPublisher( node );

	return node;
    }
}

