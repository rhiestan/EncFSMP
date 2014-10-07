/*****************************************************************************
 * Author:   Valient Gough <vgough@pobox.com>
 *
 *****************************************************************************
 * Copyright (c) 2004, Valient Gough
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.  
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ConfigReader.h"
#include "fs_layer.h"

#include <rlog/rlog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <cstring>


using namespace std;
using namespace rlog;


ConfigReader::ConfigReader()
{
}

ConfigReader::~ConfigReader()
{
}

// read the entire file into a ConfigVar instance and then use that to decode
// into mapped variables.
bool 
ConfigReader::load(const char *fileName)
{
    struct stat stbuf;
    memset( &stbuf, 0, sizeof(struct stat));
    if( fs_layer::lstat( fileName, &stbuf ) != 0)
	return false;

    int size = stbuf.st_size;

	int fd = fs_layer::open( fileName, O_RDONLY );
	if(fd < 0)
	return false;

    char *buf = new char[size];

    int res = fs_layer::read( fd, buf, size );
    fs_layer::close( fd );

    if( res != size )
    {
	rWarning("Partial read of config file, expecting %i bytes, got %i",
		size, res);
	delete[] buf;
	return false;
    }

    ConfigVar in;
    in.write( (unsigned char *)buf, size );
    delete[] buf;

    return loadFromVar( in );
}

bool 
ConfigReader::loadFromVar(ConfigVar &in)
{
    in.resetOffset();

    // parse.
    int numEntries = in.readInt();

    for(int i=0; i<numEntries; ++i)
    {
	string key, value;
	in >> key >> value;

	if(key.length() == 0)
	{
	    rError("Invalid key encoding in buffer");
	    return false;
	}
	ConfigVar newVar( value );
	vars.insert( make_pair( key, newVar ) );
    }

    return true;
}

bool
ConfigReader::save(const char *fileName) const
{
    // write everything to a ConfigVar, then output to disk
    ConfigVar out = toVar();

    int fd = fs_layer::open( fileName, O_RDWR | O_CREAT, 0640 );
    if(fd >= 0)
    {
	int retVal = fs_layer::write( fd, out.buffer(), out.size() );
	fs_layer::close( fd );
	if(retVal != out.size())
	{
	    rError("Error writing to config file %s", fileName);
	    return false;
	}
    } else
    {
	rError("Unable to open or create file %s", fileName);
	return false;
    }

    return true;
}

ConfigVar
ConfigReader::toVar() const
{
    // write everything to a ConfigVar, then output to disk
    ConfigVar out;
    out.writeInt( vars.size() );
    map<string, ConfigVar>::const_iterator it;
    for(it = vars.begin(); it != vars.end(); ++it)
    {
	out.writeInt( it->first.size() );
	out.write( (unsigned char*)it->first.data(), it->first.size() );
	out.writeInt( it->second.size() );
	out.write( (unsigned char*)it->second.buffer(), it->second.size() );
    }

    return out;
}

ConfigVar ConfigReader::operator[] ( const std::string &varName ) const
{
    // read only
    map<string, ConfigVar>::const_iterator it = vars.find( varName );
    if( it == vars.end() )
	return ConfigVar();
    else
	return it->second;
}

ConfigVar &ConfigReader::operator[] ( const std::string &varName )
{
    return vars[ varName ];
}

