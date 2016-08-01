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
		                                                                                
                             
#ifndef _RLogInit_incl_
#define _RLogInit_incl_


namespace rlog
{
    /*! @class RLogModule <rlog/rloginit.h>
      @brief Allows registration of external modules to rlog.

      Currently this only allows for initialization callbacks.  When RLogInit()
      is called, init() is called on all modules which have been registered.

      @author Valient Gough
    */
    class RLOG_DECL RLogModule
    {
    public:
	virtual ~RLogModule();

	/*! Called by RLogInit() to give the modules the command-line arguments
	*/
	virtual void init( int &argv, char **argc );

	/*! Must be implemented to return the name of the module.
	*/
	virtual const char *moduleName() const =0;
    };

    /*! @relates RLogModule

      Registers the module - which will have init() called when RLogInit is
      called.
      Returns the module so that it can be used easily as a static initializer.
      @code
      class MyModule : public rlog::RLogModule
      {
      public:
          virtual const char *moduleName() const {return "MyModule";}
      };
      static RLogModule * testModule = rlog::RegisterModule( new MyModule() );
      @endcode
    */
    RLogModule * RegisterModule( RLogModule * module );
}

#endif

