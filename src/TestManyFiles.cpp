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

#include "TestManyFiles.h"
#include "TestFileHelper.h"

#include <iostream>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>

/**
 * This test creates a directory with many (~2000) files, and checks whether they appear in the directory listing.
 *
 * This tests the correct behaviour of PFMLayer::List (i.e. the handling of the return code of PfmMarshallerListOp::Add8).
 */
bool TestManyFiles::runTest(const boost::filesystem::path &testpath)
{
	const int manyFilesCount = 2000;
	boost::system::error_code ec;
	bool retVal = true;

	// Create test directory
	boost::filesystem::path tmptestpath = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(tmptestpath, ec))
		return false;

	for(int i = 0; i < manyFilesCount; i++)
	{
		boost::filesystem::path testfilename = tmptestpath /
			(std::string("file_") + boost::lexical_cast<std::string>(i) + std::string(".txt"));

		// Create file
		FILE *fd = TestFileHelper::fopen(testfilename, "wb");
		if(fd == NULL)
			retVal = false;
		else
		{
			unsigned char buf[10];
			for(int i = 0; i < 10; i++)
				buf[i] = static_cast<unsigned char>(i);
			fwrite(buf, 1, 10, fd);
			fclose(fd);
		}
	}

	// List the directory
	std::vector<boost::filesystem::path> dirEntries;
	boost::filesystem::directory_iterator dir_iter(tmptestpath, ec);
	while(dir_iter != boost::filesystem::directory_iterator())
	{
		const boost::filesystem::directory_entry &dirEntry = *(dir_iter);
		dirEntries.push_back(dirEntry.path().filename());
		dir_iter++;
	}

	if(dirEntries.size() != manyFilesCount)
		retVal = false;

	// Remove directory
	if(!boost::filesystem::remove_all(tmptestpath, ec))
		retVal = false;

	return retVal;
}
