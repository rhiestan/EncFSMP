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
		                                                                                

#ifndef _Lock_incl_
#define _Lock_incl_

#include <rlog/Mutex.h>

namespace rlog
{
    /*! @class Lock

	Simple helper class or doing locking, so we can be sure the lock is
	released when our scope ends for whatever reason..

	Usage
	@code
	func()
	{
	    // mutex released when lock goes out of scope
	    Lock lock( &mutex );

	    ...
	}
	@endcode

	@internal
    */
    class RLOG_DECL Lock
    {
    public:
	Lock( Mutex * mutex )
	    : _mutex( mutex )
	{
	    _mutex->Lock();
	}

	~Lock()
	{
	    _mutex->Unlock();
	    _mutex = 0;
	}

	Mutex *_mutex;
    };

} // namespace rlog

#endif

