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
		                                                                                

#ifndef _RLogNode_incl_
#define _RLogNode_incl_

#include <list>
#include <set>
#include <time.h>
#include <string>

#include <rlog/common.h>
#include <rlog/Mutex.h>

namespace rlog
{
    class RLOG_DECL RLogNode;

    /*! @struct RLogData <rlog/RLogNode.h>
      @brief Data published through RLogNode

      RLogData is the data which is published from rDebug(), rWarning(),
      rError() and rLog() macros.  It contains a link to the publisher, the
      (approximate) time of the publication, and the formatted message.

      Note that none of the data in the publication is considered to be static.
      Once control has returned to the publisher, the data may be destroyed. If
      it is necessary to hang on to any of the data, a deep copy must be made.

    */
    struct RLOG_DECL RLogData
    {
	struct PublishLoc *publisher;
	//! time of publication
	time_t time;
	//! formatted msg - gets destroyed when publish() call returns.
	const char *msg; 

	// track which nodes have seen this message, to avoid getting
	// duplicates.  It would be nice if we could enforce this via the node
	// structure instead, but that is much harder.
	std::set< RLogNode * > seen;
    };

    // documentation in implementation file
    class RLogNode
    {
    public:
	RLogNode();
	virtual ~RLogNode();

	// remove this node from the subscription network
	virtual void clear();

	virtual void publish( const RLogData &data );

	// primary interface
	virtual void addPublisher( RLogNode * );
	virtual void dropPublisher( RLogNode *, bool callbacks=true );

	bool enabled() const;

	// used internally - see documentation
	virtual void addSubscriber( RLogNode * );
	virtual void dropSubscriber( RLogNode * );

	virtual void isInterested( RLogNode *node, bool isInterested );

    protected:
	// called by RLogNode when enable state changed.
	virtual void setEnabled(bool newState);

	//! list of nodes we are subscribed to
	std::list< RLogNode * > publishers;

	//! list of nodes we publish to
	std::list< RLogNode * > subscribers;

	//! list of subscribers that are interested in receiving publications..
	std::list< RLogNode * > interestList;

	Mutex mutex;
    };
}

#endif

