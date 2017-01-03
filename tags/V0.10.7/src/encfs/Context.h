/*****************************************************************************
 * Author:   Valient Gough <vgough@pobox.com>
 *
 *****************************************************************************
 * Copyright (c) 2007, Valient Gough
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

#ifndef _Context_incl_
#define _Context_incl_

#include "encfs.h"
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <set>

#ifdef USE_HASHMAP
#include <ext/hash_map>
#else
#include <map>
#endif

using boost::shared_ptr;
struct EncFS_Args;
struct EncFS_Opts;
class FileNode;
class DirNode;

class EncFS_Context
{
public:
    EncFS_Context();
    ~EncFS_Context();

    shared_ptr<FileNode> getNode(void *ptr);
    shared_ptr<FileNode> lookupNode(const char *path);

    int getAndResetUsageCounter();
    int openFileCount() const;

    void *putNode(const char *path, const shared_ptr<FileNode> &node);

    void eraseNode(const char *path, void *placeholder);

    void renameNode(const char *oldName, const char *newName);

    void setRoot(const shared_ptr<DirNode> &root);
    shared_ptr<DirNode> getRoot(int *err);
    bool isMounted();

    shared_ptr<EncFS_Args> args;
    shared_ptr<EncFS_Opts> opts;
    bool publicFilesystem;

    // root path to cipher dir
    std::string rootCipherDir;

    // for idle monitor
    bool running;
    boost::condition_variable wakeupCond;
	boost::mutex wakeupMutex;

private:
    /* This placeholder is what is referenced in FUSE context (passed to
     * callbacks).
     *
     * A FileNode may be opened many times, but only one FileNode instance per
     * file is kept.  Rather then doing reference counting in FileNode, we
     * store a unique Placeholder for each open() until the corresponding
     * release() is called.  shared_ptr then does our reference counting for
     * us.
     */
    struct Placeholder
    {
	shared_ptr<FileNode> node;

	Placeholder( const shared_ptr<FileNode> &ptr ) : node(ptr) {}
    };

    // set of open files, indexed by path
#ifdef USE_HASHMAP
    typedef __gnu_cxx::hash_map<std::string, 
	    std::set<Placeholder*> > FileMap;
#else
    typedef std::map< std::string, 
	    std::set<Placeholder*> > FileMap;
#endif

    mutable boost::mutex contextMutex;
    FileMap openFiles;

    int usageCount;
    shared_ptr<DirNode> root;
};

int remountFS( EncFS_Context *ctx, std::ostream &ostr );

#endif

