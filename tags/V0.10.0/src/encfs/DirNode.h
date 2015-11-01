/*****************************************************************************
 * Author:   Valient Gough <vgough@pobox.com>
 *
 *****************************************************************************
 * Copyright (c) 2003, Valient Gough
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

#ifndef _DirNode_incl_
#define _DirNode_incl_

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#endif

#if defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#if !defined(_MSC_VER)
#	include <dirent.h>
#endif
#include <sys/types.h>

#include <map>
#include <list>
#include <vector>
#include <string>
#include <boost/thread.hpp>

#include "FileNode.h"
#include "NameIO.h"
#include "CipherKey.h"
#include "FSConfig.h"

using boost::shared_ptr;

class Cipher;
class RenameOp;
struct RenameEl;
class EncFS_Context;

class DirTraverse
{
public:
    DirTraverse(const boost::shared_ptr<fs_layer::DIR> &dirPtr, uint64_t iv,
	        const boost::shared_ptr<NameIO> &naming);
    DirTraverse(const DirTraverse &src);
    ~DirTraverse();

    DirTraverse &operator = (const DirTraverse &src);

    // returns FALSE to indicate an invalid DirTraverse (such as when
    // an invalid directory is requested for traversal)
    bool valid() const;

    // return next plaintext filename
    // If fileType is not 0, then it is used to return the filetype (or 0 if
    // unknown)
    std::string nextPlaintextName(int *fileType=0, ino_t *inode=0);

    /* Return cipher name of next undecodable filename..
       The opposite of nextPlaintextName(), as that skips undecodable names..
    */
    std::string nextInvalid();
private:

    boost::shared_ptr<fs_layer::DIR> dir; // struct DIR
    // initialization vector to use.  Not very general purpose, but makes it
    // more efficient to support filename IV chaining..
    uint64_t iv; 
    boost::shared_ptr<NameIO> naming;
};
inline bool DirTraverse::valid() const { return dir != 0; }

#ifdef USE_HASHMAP
namespace __gnu_cxx
{
    template<> struct hash<std::string>
    {
	size_t operator() (const std::string &__s) const
	{
	    return __stl_hash_string( __s.c_str() );
	}
    };
}
#endif

class DirNode
{
public:
    // sourceDir points to where raw files are stored
    DirNode(EncFS_Context *ctx,
            const std::string &sourceDir,
            const FSConfigPtr &config );
    ~DirNode();

    // return the path to the root directory
    std::string rootDirectory();

    // find files
    boost::shared_ptr<FileNode> lookupNode( const char *plaintextName, 
	                      const char *requestor );

    /*
	Combined lookupNode + node->open() call.  If the open fails, then the
	node is not retained.  If the open succeeds, then the node is returned.
    */
    boost::shared_ptr<FileNode> openNode( const char *plaintextName, 
	    const char *requestor, int flags, int *openResult );

    std::string cipherPath( const char *plaintextPath );
    std::string cipherPathWithoutRoot( const char *plaintextPath );
    std::string plainPath( const char *cipherPath );

    // relative cipherPath is the same as cipherPath except that it doesn't
    // prepent the mount point.  That it, it doesn't return a fully qualified
    // name, just a relative path within the encrypted filesystem.
    std::string relativeCipherPath( const char *plaintextPath );

    /*
	Returns true if file names are dependent on the parent directory name.
	If a directory name is changed, then all the filenames must also be
	changed.
    */
    bool hasDirectoryNameDependency() const;

    // unlink the specified file
    int unlink( const char *plaintextName );

    // traverse directory
    DirTraverse openDir( const char *plainDirName );

    // uid and gid are used as the directory owner, only if not zero
    int mkdir( const char *plaintextPath, mode_t mode,
	    uid_t uid = 0, gid_t gid = 0);

    int rename( const char *fromPlaintext, const char *toPlaintext );

    int link( const char *from, const char *to );
    
    // returns idle time of filesystem in seconds
    int idleSeconds();

protected:

    /*
	notify that a file is being renamed. 
	This renames the internal node, if any.  If the file is not open, then
	this call has no effect.
	Returns the FileNode if it was found.
    */
    boost::shared_ptr<FileNode> renameNode( const char *from, const char *to );
    boost::shared_ptr<FileNode> renameNode( const char *from, const char *to, 
	                             bool forwardMode );

    /*
	when directory IV chaining is enabled, a directory can't be renamed
	without renaming all its contents as well.  recursiveRename should be
	called after renaming the directory, passing in the plaintext from and
	to paths.
    */
    boost::shared_ptr<RenameOp> newRenameOp( const char *from, const char *to );

private:

    friend class RenameOp;

    bool genRenameList( std::list<RenameEl> &list, const char *fromP,
	    const char *toP );
    
    boost::shared_ptr<FileNode> findOrCreate( const char *plainName);

    boost::mutex mutex;

    EncFS_Context *ctx;

    // passed in as configuration
    std::string rootDir;
    FSConfigPtr fsConfig;

    boost::shared_ptr<NameIO> naming;
};

#endif
