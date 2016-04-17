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
		                                                                                
                             
#include "Error.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#include "config.h"

#  ifdef HAVE_SSTREAM
#    include <sstream>
#  else
// old STL -- use strstream instead of stringstream
#    include <strstream.h>
#    define USE_STRSTREAM
#  endif

#else
#  include <sstream>
#endif

using namespace rlog;
using namespace std;


namespace rlog
{
    /*! @struct ErrorData <rlog/Error.h>
      @brief Internal RLog structure - holds internal data for rlog::Error

      @internal
    */
    struct ErrorData
    {
	//! for reference counting
	int usageCount; // to make copy fast

	//! component where error occured
	string component;
	//! file where error occured
	string file;
	//! function name where error occured
	string function;
	//! line number where error occured
	int line;
	//! condition string (in case of rAssert) at error location
	string msg;
    };
}

static 
string errorMessage( const char *file, int line, 
	const char *msg)
{
#ifdef USE_STRSTREAM
    ostrstream ss;
#else
    ostringstream ss;
#endif

    ss << "Assert failure at " << file << ':' << line << " -- " << msg;

    return ss.str();
}



/*! @class rlog::Error <rlog/Error.h>
  @brief Error Used as exception thrown from rAssert() on failure.
  
  Error is derived from std::runtime_error exception and is thrown from
  rAssert() to indicate the reason and location of the failure.

  @code
  void testFunc()
  {
      bool testAssert = true;
      rAssert( testAssert == false ); // fails..
  }

  void A()
  {
      try
      {
	  testFunc();
      } catch( Error &err )
      {
	  rDebug("caught assert failure from %s line %i ( %s )",
	      err.file(), err.line(), err.function() );
      }
  }
  @endcode

  @author Valient Gough
  @see rAssert()
*/


Error::Error( const char *component, const char *file, const char *function,
	int line, const char *msg )
    : runtime_error( errorMessage( file, line, msg ) )
{
    data = new ErrorData;
    data->usageCount = 1;
    data->component = component;
    data->file = file;
    data->function = function;
    data->line = line;
    data->msg = msg;
}

Error::Error( const char *component, const char *file, const char *function,
	int line, const std::string &msg )
    : runtime_error( errorMessage( file, line, msg.c_str() ) )
{
    data = new ErrorData;
    data->usageCount = 1;
    data->component = component;
    data->file = file;
    data->function = function;
    data->line = line;
    data->msg = msg;
}

Error::Error( const Error &src )
    : runtime_error( src.what() )
{
    ++src.data->usageCount;
    data = src.data;
}

Error::~Error() throw()
{
    if(data)
    {
	if(--data->usageCount == 0)
	    delete data;
	data = 0;
    }
}

Error & Error::operator = (const Error & src)
{
    if(data != src.data)
    {
	++src.data->usageCount;
	if(--data->usageCount == 0)
	    delete data;
	data = src.data;
    }
	
    return *this;
}

/*! Log the error to the given channel
*/
void Error::log( RLogChannel * channel ) const
{
    (void) channel;
}

/*! return component string.  If this was due to an rAssert() then this will
  be the definition of RLOG_COMPONENT
  at the point of the rAssert.
*/
const char *Error::component() const
{
    return data->component.c_str();
}

const char *Error::file() const
{
    return data->file.c_str();
}

const char *Error::function() const
{
    return data->function.c_str();
}

int Error::line() const
{
    return data->line;
}

const char *Error::message() const
{
    return data->msg.c_str();
}


string rlog::_format_msg( const char *format, ... )
{
    char msgBuf[64];
    va_list args;
    va_start( args, format );

    int ncpy = vsnprintf( msgBuf, sizeof(msgBuf), format, args );

    va_end( args );

    std::string result;
    if(ncpy < (int)sizeof(msgBuf))
    {
	if(ncpy > 0)
	    result = msgBuf;
	else
	    result = "RLOG internal formatting error";
    } else
    {
	// buffer wasn't large ehough, allocate a temporary
	int len = ncpy + 1;
	char *buf = new char[len];
	
	va_start( args, format );
	vsnprintf( buf, len, format, args );
	va_end( args );

	result = buf;
	delete[] buf;
    }

    return result;
}

