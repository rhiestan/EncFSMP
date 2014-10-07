/*****************************************************************************
 * Author:   Vadim Zeitlin <vadim@wxwidgets.org>
 *
 *****************************************************************************
 * Copyright (c) 2004 Vadim Zeitlin
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

#ifndef _Mutex_incl_
#define _Mutex_incl_

#include <rlog/common.h>

#ifdef _WIN32
    #include <windows.h>
    // conflict with our own ERROR...
    #undef ERROR

    typedef CRITICAL_SECTION rlog_mutex_t;
    #define rlog_mutex_init InitializeCriticalSection
    #define rlog_mutex_destroy DeleteCriticalSection
    #define rlog_mutex_lock EnterCriticalSection
    #define rlog_mutex_unlock LeaveCriticalSection
#else
    #include <pthread.h>

    typedef pthread_mutex_t rlog_mutex_t;
    #define rlog_mutex_init(m) pthread_mutex_init((m), 0)
    #define rlog_mutex_destroy pthread_mutex_destroy
    #define rlog_mutex_lock pthread_mutex_lock
    #define rlog_mutex_unlock pthread_mutex_unlock
#endif

namespace rlog
{
    /*! @class Mutex
	@brief Class encapsulating a critical section under Win32 or a mutex
	under Unix.

	This class should never be used standalone but always passed to Lock,
	see the example there.
    */
    class RLOG_DECL Mutex
    {
    public:
	Mutex() { rlog_mutex_init(&m_mutex); }
	~Mutex() { rlog_mutex_destroy(&m_mutex); }

	void Lock() { rlog_mutex_lock(&m_mutex); }
	void Unlock() { rlog_mutex_unlock(&m_mutex); }

    private:
	rlog_mutex_t m_mutex;

	Mutex( const Mutex & );
	Mutex & operator = ( const Mutex & );
    };
} // namespace rlog

#endif // _Mutex_incl_

