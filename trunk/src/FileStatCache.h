/**
 * Copyright (C) 2015 Roman Hiestand
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FILESTATCACHE_H
#define FILESTATCACHE_H

#include "config.h"

#include <map>
#include <deque>

// Required for efs_stat
#include "fs_layer.h"

/**
 * This class caches results of stat().
 *
 * stat() is used many times, and it is helpful to cache its result.
 */
class FileStatCache
{
public:
	FileStatCache();
	virtual ~FileStatCache();

	void clearCache();

	void setCacheSize(int cacheSize);
	int getCachesize() const { return cacheSize_; }

	int stat(const char *path, efs_stat *buffer);

	void forgetCachedStat(const char *path);

protected:
	void addStatToCache(const char *path, int retVal, efs_stat *buffer);

private:
	struct CacheEntry
	{
		int64_t time_;		// When the stat was made, used for cache eviction
		efs_stat stat_;
		int retVal_;		// Return value of stat
	};

	typedef std::map<std::string, CacheEntry> FileStatCacheType;
	FileStatCacheType cache_;

	int cacheSize_;
};

#endif
