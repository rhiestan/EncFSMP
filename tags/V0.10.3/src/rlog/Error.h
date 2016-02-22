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
		                                                                                
                             
#ifndef _Error_incl_
#define _Error_incl_

#include <stdexcept>
#include <rlog/common.h>

#include <string>

namespace rlog
{
    class RLOG_DECL RLogChannel;

    // Documentation in implementation file
    class RLOG_DECL Error : public std::runtime_error
    {
    public:
	Error( const char *component, const char *file, const char *function,
		int line, const char *msg );
	Error( const char *component, const char *file, const char *function,
		int line, const std::string &msg );
	Error(const Error &src );
	virtual ~Error() throw();

	Error &operator = (const Error &src);

	void log( RLogChannel * channel ) const;

	const char *component() const;
	const char *file() const;
	const char *function() const;
	int line() const;
	const char *message() const;

    private:
	struct ErrorData *data;
    };

    std::string _format_msg( const char *fmt, ... ) PRINTF(1,2);
}

#define _ERROR_IMPL( COMPONENT, MSG ) \
    rlog::Error( STR(COMPONENT), __FILE__, __FUNCTION__, __LINE__, MSG )

#define RL_ERROR( MSG ) _ERROR_IMPL( RLOG_COMPONENT, MSG )

#if C99_VARIADAC_MACROS
#define _ERROR_IMPL_VA( COMPONENT, FMT, ... ) \
    rlog::Error( STR(COMPONENT), __FILE__, __FUNCTION__, __LINE__, \
	    rlog::_format_msg( FMT, __VA_ARGS__ ) )
#define ERROR_FMT( FMT, ... ) _ERROR_IMPL_VA( RLOG_COMPONENT, FMT, __VA_ARGS__ )
#elif PREC99_VARIADIC_MACROS
#define _ERROR_IMPL_VA( COMPONENT, FMT, ARGS... ) \
    rlog::Error( STR(COMPONENT), __FILE__, __FUNCTION__, __LINE__, \
	    rlog::_format_msg( FMT, ##ARGS ) )
#define ERROR_FMT( FMT, ARGS... ) _ERROR_IMPL_VA( RLOG_COMPONENT, FMT, ##ARGS )
#else
// TODO: implement for no variadics case..
#endif

#endif

