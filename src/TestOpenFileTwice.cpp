/**
* Copyright (C) 2016 Roman Hiestand
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

#include "TestOpenFileTwice.h"
#include "TestFileHelper.h"

#include <iostream>

bool TestOpenFileTwice::runTest(const boost::filesystem::path &testpath)
{
	boost::system::error_code ec;
	bool retVal = true;

	// Create test directory
	boost::filesystem::path wintestpath = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(wintestpath, ec))
		return false;

	boost::filesystem::path testfilename = wintestpath / L"encfsmp_test_opentwice.txt";

	// Create file
	FILE *fd = TestFileHelper::fopen_shared(testfilename, "wb");
	if(fd == NULL)
		retVal = false;
	unsigned char buf[10];
	for(int i = 0; i < 10; i++)
		buf[i] = static_cast<unsigned char>(i);
	fwrite(buf, 1, 10, fd);
	fclose(fd);

	// Open the file once for read-only
	FILE *fdro = TestFileHelper::fopen_shared(testfilename, "rb");
	if(fdro == NULL)
		retVal = false;

	// Read some bytes
	if(fread(buf, 1, 10, fdro) != 10)
		retVal = false;

	// Open the file a second time for writing
	FILE *fdwr = TestFileHelper::fopen_shared(testfilename, "wb");
	if(fdwr == NULL)
	{
		std::cout << "fopen twice failed, errno = " << errno << std::endl;
		retVal = false;
	}

	if(fdwr != NULL)
		fclose(fdwr);
	if(fdro != NULL)
		fclose(fdro);

	// Delete file
	if(!boost::filesystem::remove(testfilename, ec))
		retVal = false;

	// Remove directory
	if(!boost::filesystem::remove_all(wintestpath, ec))
		retVal = false;

	return retVal;
}
