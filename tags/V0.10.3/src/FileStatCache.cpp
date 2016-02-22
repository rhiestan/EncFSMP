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

#include "FileStatCache.h"

#include <ctime>

FileStatCache::FileStatCache() : cacheSize_(10)
{
}

FileStatCache::~FileStatCache()
{
}

void FileStatCache::clearCache()
{
	cache_.clear();
}

void FileStatCache::setCacheSize(int cacheSize)
{
	cacheSize_ = cacheSize;
}

int FileStatCache::stat(const char *path, efs_stat *buffer)
{
	int ret = 0;
	FileStatCacheType::iterator iter = cache_.find( std::string(path) );
	if(iter != cache_.end())
	{
		*buffer = iter->second.stat_;
		ret = iter->second.retVal_;
	}
	else
	{
		ret = fs_layer::stat(path, buffer);
		addStatToCache(path, ret, buffer);
	}
	return ret;
}

void FileStatCache::forgetCachedStat(const char *path)
{
	FileStatCacheType::iterator iter = cache_.find( std::string(path) );
	if(iter != cache_.end())
	{
		cache_.erase(iter);
	}
}

void FileStatCache::addStatToCache(const char *path, int retVal, efs_stat *buffer)
{
	std::string pathS(path);
	FileStatCacheType::iterator iter = cache_.find( pathS );
	if(iter != cache_.end())
	{
		// Update existing entry
		CacheEntry &entry = iter->second;
		entry.stat_ = *buffer;
		entry.retVal_ = retVal;
	}
	else if(cacheSize_ > 0)
	{
		// Add new entry
		if(static_cast<int>(cache_.size()) >= cacheSize_)
		{
			// Find least recently used, evict from cache
			FileStatCacheType::iterator iterEvict = cache_.begin();
			FileStatCacheType::iterator iterCheck = cache_.begin();
			iterCheck++;	// This is safe as cacheSize_ > 0, and cache_.size() >= cacheSize_
			while(iterCheck != cache_.end())
			{
				if(iterCheck->second.time_ < iterEvict->second.time_)
					iterEvict = iterCheck;

				iterCheck++;
			}
			cache_.erase(iterEvict);
		}

		std::time_t currentTime;
		time(&currentTime);

		CacheEntry entry;
		entry.time_ = static_cast<int64_t>(currentTime);
		entry.stat_ = *buffer;
		entry.retVal_ = retVal;
		cache_[ pathS ] = entry;
	}
}
