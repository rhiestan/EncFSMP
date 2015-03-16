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
		                                                                                

#include "RLogNode.h"

#include "Lock.h"

using namespace rlog;
using namespace std;


/*! @class rlog::RLogNode <rlog/RLogNode.h>
  @brief Core of publication system, forms activation network.

  RLogNode formes the core of the publication system.  It has two primary
  purposes :
  - link publications with subscribers
  - transfer meta-data in the form or node activations

  Both publishers (eg RLogPublisher) and subscribers (eg StdioNode) are
  derived from RLogNode, although RLogNode can be used entirely unmodified.

  An RLogNode contains a list of publishers which it is linked with.  It
  also contains a list of subscribers which it is linked with.
  Publications always flow from publishers to subscribers, and activation
  information flows the opposite direction from subscribers to publishers.

  An RLogNode by default acts as both a subscriber and publisher -- when
  it has been subscribed to another node and receives a publication it
  simply repeats the information to all of its subscribers.

  More specifically, it only publishes to subscribers who have also voiced
  an interest in receiving the publication.  If a node has no subscribers
  which are also interested (or no subscribers at all), then it can be said
  to be dormant and it tells the publishers that it is subscribed to that
  it is no longer interested.  This propogates all the way up to
  RLogPublishers which will disable the logging statement completely if
  there are no interested parties.

  @author Valient Gough
*/



/*! @brief Instantiate an empty RLogNode.  No subscribers or publishers..
*/
RLogNode::RLogNode()
{
}

/*! @brief Disconnects from any remaining subscribers and publishers
*/
RLogNode::~RLogNode()
{
    clear();
}

/*! @brief Force disconnection from any subscribers or publishers
*/
void
RLogNode::clear()
{
    Lock lock( &mutex );

    list<RLogNode*>::iterator it;
    //bool enabled = !interestList.empty();

    for(it = publishers.begin(); it != publishers.end(); ++it)
    {
	//if(enabled)
	    (*it)->isInterested( this, false );
	(*it)->dropSubscriber( this );
    }

    // unsubscribe
    // dropPublisher(.., false) tells not to callback to our dropSubscriber or
    // isInterested methods (othrewise we'd have locking issues)...
    for(it = subscribers.begin(); it != subscribers.end(); ++it)
	(*it)->dropPublisher( this, false );

    subscribers.clear();
    interestList.clear();

    setEnabled( false );
}

/*! @brief Publish data.

  This iterates over the list of subscribers which have stated interest and
  sends them the data.
*/
void
RLogNode::publish( const RLogData &data )
{
    Lock lock( &mutex );

    list<RLogNode*>::iterator it;
    for(it = interestList.begin(); it != interestList.end(); ++it)
	(*it)->publish( data );
}

/*! @brief Have this node subscribe to a new publisher.

  We become a subscriber of the publisher.  The publisher's addSubscriber()
  function is called to complete the link.

  If our node is active then we also tell the publisher that we want
  publications.
*/
void
RLogNode::addPublisher( RLogNode *publisher )
{
    Lock lock( &mutex );

    publishers.push_back( publisher );

    publisher->addSubscriber( this );

    if( !interestList.empty() )
	publisher->isInterested( this, true );
}

/*! @brief Drop our subscription to a publisher

  A callback parameter is provided to help avoid loops in the code which may
  affect the thread locking code.

  @param callback If True, then we call publisher->dropSubscriber() to make
  sure the publisher also drops us as a subscriber.
*/
void
RLogNode::dropPublisher( RLogNode *publisher, bool callback )
{
    Lock lock( &mutex );

    publishers.remove( publisher );

    if(callback)
    {
	if( !interestList.empty() )
	    publisher->isInterested( this, false );

	publisher->dropSubscriber( this );
    }
}

/*! @brief Add a subscriber.

  Normally a subscriber calls this for itself when it's addPublisher() method is
  called.
*/
void
RLogNode::addSubscriber( RLogNode *subscriber )
{
    Lock lock( &mutex );
    subscribers.push_back( subscriber );
}

/*! @brief Remove a subscriber.

  Normally a subscriber calls this for itself when it's dropPublisher() method
  is called.

  Note that the subscriber list is kept separate from the interest list.  If
  the subscriber is active, then you must undo that by calling
  isInterested(subscriber, false) in addition to dropSubscriber
*/
void
RLogNode::dropSubscriber( RLogNode *subscriber )
{
    Lock lock( &mutex );
    subscribers.remove( subscriber );
}

/*! @brief Change the state of one of our subscribers.  

  This allows a subscriber to say that it wants to be notified of publications
  or not.  The @a node should already be registered as a subscriber.

  If we previously had no interested parties and now we do, then we need to
  notify the publishers in our publishers list that we are now interested.

  If we previously had interested parties and we remove the last one, then we
  can notify the publishers that we are no longer interested..
*/
void
RLogNode::isInterested( RLogNode *node, bool interest)
{
    Lock lock( &mutex );
    bool changeInterest;

    if(interest)
    {
	changeInterest = interestList.empty();
	interestList.push_back( node );
    } else
    {
	interestList.remove( node );
	changeInterest = interestList.empty();
    }
	    
    if(changeInterest)
    {
	list<RLogNode*>::iterator it;
	for(it = publishers.begin(); it != publishers.end(); ++it)
	    (*it)->isInterested( this, interest );

	// let derived classes know that we have become active (or dormant)
	setEnabled( interest );
    }
}

/*! @brief Returns @e true if this node is active
  @return @e true if we have one or more interested subscribers, otherwise false
*/
bool
RLogNode::enabled() const
{
    return !interestList.empty();
}

/*! @brief For derived classes to get notified of activation status change.

  This is called by isInterested() when our state changes.  If @e true is
  passed, then this node has become active.  If @e false is passed, then this
  node has just become dormant.
*/
void
RLogNode::setEnabled(bool)
{
}

