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

#include "TestUnicodePaths.h"
#include "TestFileHelper.h"

#include <cstdio>
#include <cstring>

bool TestUnicodePaths::unicodeFilenameTest(const boost::filesystem::path &testpath)
{
	boost::system::error_code ec;
	bool retVal = true;

	boost::filesystem::path dir = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(dir, ec))
		return false;

	// CJK characters + a supplementary-plane emoji (encoded as a UTF-16
	// surrogate pair) + a combining acute accent - each exercises a different
	// corner of UTF-16 <-> UTF-8 name conversion between Windows and encfs.
	boost::filesystem::path fn = dir / L"测试\U0001F600é.txt";
	const char content[] = "unicode-content";

	FILE *fp = TestFileHelper::fopen(fn, "wb");
	if(fp == NULL)
	{
		boost::filesystem::remove_all(dir, ec);
		return false;
	}
	fputs(content, fp);
	fclose(fp);

	if(!boost::filesystem::exists(fn, ec))
		retVal = false;

	FILE *rp = TestFileHelper::fopen(fn, "rb");
	if(rp == NULL)
		retVal = false;
	else
	{
		char buf[64] = {};
		size_t n = fread(buf, 1, sizeof(buf) - 1, rp);
		buf[n] = '\0';
		if(strcmp(buf, content) != 0)
			retVal = false;
		fclose(rp);
	}

	// The name must show up correctly - and exactly once - in a directory listing
	int matchCount = 0;
	for(boost::filesystem::directory_iterator it(dir, ec), end; !ec && it != end; ++it)
	{
		if(it->path().filename() == fn.filename())
			matchCount++;
	}
	if(matchCount != 1)
		retVal = false;

	boost::filesystem::remove_all(dir, ec);
	return retVal;
}
