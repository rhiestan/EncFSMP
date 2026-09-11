/**
* Copyright (C) 2026 Roman Hiestand
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

// Unlike the other Test* files, this one needs no mounted volume: FileStatCache
// is a self-contained class that wraps fs_layer::stat() with a small cache, so
// it can be exercised directly against ordinary temp files.

#include "doctest/doctest.h"

#include "FileStatCache.h"
#include "TestFileHelper.h"

#include <boost/filesystem.hpp>
#include <cstdio>

namespace
{
	// Creates a small file with known content and returns its path.
	boost::filesystem::path makeTestFile(const boost::filesystem::path &dir, const char *content)
	{
		boost::filesystem::path fn = dir / boost::filesystem::unique_path();
		FILE *fp = TestFileHelper::fopen(fn, "wb");
		REQUIRE(fp != nullptr);
		std::fputs(content, fp);
		std::fclose(fp);
		return fn;
	}
}

TEST_SUITE("UnitTests")
{
	TEST_CASE("FileStatCache")
	{
		boost::system::error_code ec;
		boost::filesystem::path dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
		REQUIRE(boost::filesystem::create_directories(dir, ec));

		FileStatCache cache;

		SUBCASE("returns correct data for a real file")
		{
			boost::filesystem::path fn = makeTestFile(dir, "0123456789");
			efs_stat st;
			int ret = cache.stat(fn.string().c_str(), &st);
			CHECK(ret == 0);
			CHECK(st.st_size == 10);
		}

		SUBCASE("a cached stat survives deletion of the underlying file")
		{
			boost::filesystem::path fn = makeTestFile(dir, "0123456789");
			efs_stat st;
			REQUIRE(cache.stat(fn.string().c_str(), &st) == 0);
			REQUIRE(st.st_size == 10);

			boost::filesystem::remove(fn, ec);

			// Must still be served from the cache, not re-queried from disk.
			efs_stat st2;
			int ret2 = cache.stat(fn.string().c_str(), &st2);
			CHECK(ret2 == 0);
			CHECK(st2.st_size == 10);
		}

		SUBCASE("forgetCachedStat evicts only the given path")
		{
			boost::filesystem::path fnA = makeTestFile(dir, "A");
			boost::filesystem::path fnB = makeTestFile(dir, "B");
			efs_stat st;
			REQUIRE(cache.stat(fnA.string().c_str(), &st) == 0);
			REQUIRE(cache.stat(fnB.string().c_str(), &st) == 0);

			boost::filesystem::remove(fnA, ec);
			boost::filesystem::remove(fnB, ec);

			cache.forgetCachedStat(fnA.string().c_str());

			// fnA was forgotten: re-queried from disk, now fails since it's deleted.
			CHECK(cache.stat(fnA.string().c_str(), &st) != 0);
			// fnB is still cached: stale but successful result.
			CHECK(cache.stat(fnB.string().c_str(), &st) == 0);
		}

		SUBCASE("clearCache evicts everything")
		{
			boost::filesystem::path fn = makeTestFile(dir, "content");
			efs_stat st;
			REQUIRE(cache.stat(fn.string().c_str(), &st) == 0);

			boost::filesystem::remove(fn, ec);
			cache.clearCache();

			CHECK(cache.stat(fn.string().c_str(), &st) != 0);
		}

		SUBCASE("setCacheSize(1) evicts the previous entry once a new path is stat'd")
		{
			cache.setCacheSize(1);
			CHECK(cache.getCachesize() == 1);

			boost::filesystem::path fnA = makeTestFile(dir, "A");
			boost::filesystem::path fnB = makeTestFile(dir, "B");
			efs_stat st;
			REQUIRE(cache.stat(fnA.string().c_str(), &st) == 0);

			boost::filesystem::remove(fnA, ec);

			// Caching fnB should evict the only slot (fnA).
			REQUIRE(cache.stat(fnB.string().c_str(), &st) == 0);

			// fnA is no longer cached, so it's re-queried from disk and fails (deleted).
			CHECK(cache.stat(fnA.string().c_str(), &st) != 0);
		}

		boost::filesystem::remove_all(dir, ec);
	}
}
