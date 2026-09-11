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

#include "TestRenameEdgeCases.h"
#include "TestFileHelper.h"

#include <cstdio>
#include <cstring>

namespace
{
	void writeFile(const boost::filesystem::path &fn, const char *content)
	{
		FILE *fp = TestFileHelper::fopen(fn, "wb");
		if(fp != NULL)
		{
			fputs(content, fp);
			fclose(fp);
		}
	}

	bool readFileEquals(const boost::filesystem::path &fn, const char *expected)
	{
		FILE *fp = TestFileHelper::fopen(fn, "rb");
		if(fp == NULL)
			return false;
		char buf[64] = {};
		size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
		fclose(fp);
		buf[n] = '\0';
		return strcmp(buf, expected) == 0;
	}
}

bool TestRenameEdgeCases::renameOverExistingFile(const boost::filesystem::path &testpath)
{
	boost::system::error_code ec;
	bool retVal = true;

	boost::filesystem::path dir = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(dir, ec))
		return false;

	boost::filesystem::path fileA = dir / L"a.txt";
	boost::filesystem::path fileB = dir / L"b.txt";
	writeFile(fileA, "AAAA");
	writeFile(fileB, "BBBB");

	boost::filesystem::rename(fileA, fileB, ec);
	if(ec)
		retVal = false;

	if(boost::filesystem::exists(fileA, ec))
		retVal = false;
	if(!readFileEquals(fileB, "AAAA"))
		retVal = false;

	boost::filesystem::remove_all(dir, ec);
	return retVal;
}

bool TestRenameEdgeCases::renameIntoNonexistentParent(const boost::filesystem::path &testpath)
{
	boost::system::error_code ec;
	bool retVal = true;

	boost::filesystem::path dir = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(dir, ec))
		return false;

	boost::filesystem::path fileA = dir / L"a.txt";
	writeFile(fileA, "content");

	boost::filesystem::path target = dir / L"nonexistent_subdir" / L"a.txt";

	boost::filesystem::rename(fileA, target, ec);
	if(!ec)
		retVal = false;	// Must fail: parent directory does not exist

	// Source file must be untouched
	if(!boost::filesystem::exists(fileA, ec))
		retVal = false;
	if(!readFileEquals(fileA, "content"))
		retVal = false;

	boost::filesystem::remove_all(dir, ec);
	return retVal;
}

bool TestRenameEdgeCases::renameCaseOnly(const boost::filesystem::path &testpath)
{
	boost::system::error_code ec;
	bool retVal = true;

	boost::filesystem::path dir = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(dir, ec))
		return false;

	boost::filesystem::path original = dir / L"MixedCase.txt";
	boost::filesystem::path renamed = dir / L"mixedcase.txt";
	writeFile(original, "content");

	boost::filesystem::rename(original, renamed, ec);
	if(ec)
		retVal = false;

	if(!readFileEquals(renamed, "content"))
		retVal = false;

	// Exactly one file must exist in the directory afterwards - the rename
	// must not leave both the old and new name behind.
	int fileCount = 0;
	for(boost::filesystem::directory_iterator it(dir, ec), end; !ec && it != end; ++it)
		++fileCount;
	if(fileCount != 1)
		retVal = false;

	boost::filesystem::remove_all(dir, ec);
	return retVal;
}
