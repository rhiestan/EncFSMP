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

#ifndef TEST_BIGFILE_H
#define TEST_BIGFILE_H

#include "boost/filesystem.hpp"

class TestBigFile
{
public:
	static bool runTest(const boost::filesystem::path &testpath);

private:
	TestBigFile() { }
	virtual ~TestBigFile() { }

	static void encipher_xtea(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);
	static uint64_t gendata_single(int64_t offset);
	static void copydata(unsigned char *dataout, uint64_t datain);
	static void gendata(int64_t offset, int64_t sz, unsigned char *data);
	static void test_bigchunk(int64_t offset);
	static bool gentestfile(const boost::filesystem::path &filename, int64_t filesize);
	static bool checkfile(const boost::filesystem::path &filename, int64_t filesize);
};
#endif
