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

#include "CommonIncludes.h"
#include "TestReadOnlyFlag.h"
#include "TestFileHelper.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include <cstdio>

bool TestReadOnlyFlag::readOnlyAttributeTest(const boost::filesystem::path &testpath)
{
	bool retVal = true;
	boost::system::error_code ec;

	boost::filesystem::path wintestpath = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(wintestpath, ec))
		return false;

	boost::filesystem::path testfilename = wintestpath / L"encfsmp_test_readonly.txt";
	std::wstring testfilenameW = testfilename.wstring();

	// Create file with initial content
	FILE *fd = TestFileHelper::fopen(testfilename, "wb");
	if(fd == NULL)
	{
		boost::filesystem::remove_all(wintestpath, ec);
		return false;
	}
	fputs("content", fd);
	fclose(fd);

	// Mark it read-only
	if(!SetFileAttributesW(testfilenameW.c_str(), FILE_ATTRIBUTE_READONLY))
		retVal = false;

	// The attribute must be reported back
	DWORD attrs = GetFileAttributesW(testfilenameW.c_str());
	if(attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_READONLY))
		retVal = false;

	// Opening for write must now fail
	HANDLE writeHandle = CreateFileW(testfilenameW.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(writeHandle != INVALID_HANDLE_VALUE)
	{
		retVal = false;
		CloseHandle(writeHandle);
	}
	else if(GetLastError() != ERROR_ACCESS_DENIED)
		retVal = false;

	// Reading must still work while the file is read-only
	HANDLE readHandle = CreateFileW(testfilenameW.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(readHandle == INVALID_HANDLE_VALUE)
		retVal = false;
	else
		CloseHandle(readHandle);

	// Clear the read-only attribute again
	if(!SetFileAttributesW(testfilenameW.c_str(), FILE_ATTRIBUTE_NORMAL))
		retVal = false;

	attrs = GetFileAttributesW(testfilenameW.c_str());
	if(attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_READONLY))
		retVal = false;

	// Writing must succeed again now that read-only is cleared
	HANDLE writeHandle2 = CreateFileW(testfilenameW.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(writeHandle2 == INVALID_HANDLE_VALUE)
		retVal = false;
	else
		CloseHandle(writeHandle2);

	if(!boost::filesystem::remove_all(wintestpath, ec))
		retVal = false;

	return retVal;
}
