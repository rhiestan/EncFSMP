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

#ifndef _rlog_time_incl
#define _rlog_time_incl

#include <rlog/common.h>

/*! @file time.h
    @brief Defines functions for getting current time and benchmarking.
*/

#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include "windows.h"

typedef __int64 rlog_time_interval;
typedef LARGE_INTEGER rlog_time;

void sleep(int seconds);
#else

#include <stdint.h>
#include <sys/time.h>
#include <unistd.h> // for sleep()

typedef timeval rlog_time;
typedef int64_t rlog_time_interval;

#endif

void rlog_get_time(rlog_time *pt);
rlog_time_interval rlog_time_diff( const rlog_time &end, const rlog_time &start );

const char *rlog_time_unit();

#endif // _rlog_time_incl
