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
                             
#ifndef _FileUtils_incl_
#define _FileUtils_incl_

#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

#include "encfs.h"
#include "Interface.h"
#include "CipherKey.h"
#include "FSConfig.h"

// true if the path points to an existing node (of any type)
bool fileExists( const char *fileName );
// true if path is a directory
bool isDirectory( const char *fileName );
// true if starts with '/'
bool isAbsolutePath( const char *fileName );
// pointer to just after the last '/'
const char *lastPathElement( const char *name );

std::string parentDirectory( const std::string &path );

// ask the user for permission to create the directory.  If they say ok, then
// do it and return true.
bool userAllowMkdir(const char *dirPath, mode_t mode );
bool userAllowMkdir(int promptno, const char *dirPath, mode_t mode );

class Cipher;
class DirNode;

struct EncFS_Root
{
    boost::shared_ptr<Cipher> cipher;
    CipherKey volumeKey;
    boost::shared_ptr<DirNode> root;

    EncFS_Root();
    ~EncFS_Root();
};

typedef boost::shared_ptr<EncFS_Root> RootPtr;

enum ConfigMode
{
    Config_Prompt,
    Config_Standard,
    Config_Paranoia
};

struct EncFS_Opts
{
    std::string rootDir;
    bool createIfNotFound;  // create filesystem if not found
    bool idleTracking; // turn on idle monitoring of filesystem
    bool mountOnDemand; // mounting on-demand

    bool checkKey;  // check crypto key decoding
    bool forceDecode; // force decode on MAC block failures

    std::string passwordProgram; // path to password program (or empty)
    bool useStdin; // read password from stdin rather then prompting
    bool annotate; // print annotation line prompt to stderr.

    bool ownerCreate; // set owner of new files to caller

    bool reverseEncryption; // Reverse encryption

    ConfigMode configMode;

    // Added by R. Hiestand
    bool useExternalConfigFile;
    std::string externalConfigFileName;

    EncFS_Opts()
    {
        createIfNotFound = true;
        idleTracking = false;
        mountOnDemand = false;
        checkKey = true;
        forceDecode = false;
        useStdin = false;
        annotate = false;
        ownerCreate = false;
        reverseEncryption = false;
        configMode = Config_Prompt;
        useExternalConfigFile = false;
    }
};

/*
    Read existing config file.  Looks for any supported configuration version.
*/
ConfigType readConfig( const std::string &rootDir, 
        const boost::shared_ptr<EncFSConfig> &config,
		bool useExternalConfigFile, const std::string &externalConfigFileName ); 

/*
    Save the configuration.  Saves back as the same configuration type as was
    read from.
*/
bool saveConfig( ConfigType type, const std::string &rootdir, 
	const boost::shared_ptr<EncFSConfig> &config,
	bool useExternalConfigFile, const std::string &externalConfigFileName);

class EncFS_Context;

RootPtr initFS( EncFS_Context *ctx, const boost::shared_ptr<EncFS_Opts> &opts,
	std::ostream &ostr);

RootPtr createV6Config( EncFS_Context *ctx, 
                        const boost::shared_ptr<EncFS_Opts> &opts );

void showFSInfo( const boost::shared_ptr<EncFSConfig> &config );

bool readV4Config( const char *configFile, 
        const boost::shared_ptr<EncFSConfig> &config,
	struct ConfigInfo *);
bool writeV4Config( const char *configFile, 
        const boost::shared_ptr<EncFSConfig> &config);

bool readV5Config( const char *configFile, 
        const boost::shared_ptr<EncFSConfig> &config,
	struct ConfigInfo *);
bool writeV5Config( const char *configFile, 
        const boost::shared_ptr<EncFSConfig> &config);

bool readV6Config( const char *configFile, 
        const boost::shared_ptr<EncFSConfig> &config,
	struct ConfigInfo *);
bool writeV6Config( const char *configFile, 
        const boost::shared_ptr<EncFSConfig> &config);

#endif
