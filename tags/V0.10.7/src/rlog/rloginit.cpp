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
		                                                                                

#include "rlog.h"
#include "rloginit.h"

#include <list>

using namespace rlog;
using namespace std;


RLogModule::~RLogModule()
{
}

void RLogModule::init( int &argc, char **argv )
{
    (void)argc;
    (void)argv;
}



static std::list< RLogModule * > moduleList;
static int *gArgc =0;
static char **gArgv = 0;

void rlog::RLogInit( int &argc, char **argv )
{
    gArgc = &argc;
    gArgv = argv;

    list<RLogModule*>::const_iterator it;
    for(it = moduleList.begin(); it != moduleList.end(); ++it)
	(*it)->init( argc, argv );
}

RLogModule * rlog::RegisterModule( RLogModule *module )
{
    moduleList.push_back( module );
    if(gArgc)
	module->init( *gArgc, gArgv );

    return module;
}


