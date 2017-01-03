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

#include "TestBigFile.h"
#include "TestFileHelper.h"

#include <stdint.h>

/**
 * Encrypt using XTEA.
 *
 * Implementation fro Wikipedia.
 */
void TestBigFile::encipher_xtea(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4])
{
	unsigned int i;
	uint32_t v0 = v[0], v1 = v[1], sum = 0, delta = 0x9E3779B9;
	for(i = 0; i < num_rounds; i++) {
		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
		sum += delta;
		v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
	}
	v[0] = v0; v[1] = v1;
}

/**
 * Generate a single uint64_t block.
 */
uint64_t TestBigFile::gendata_single(int64_t offset)
{
	const int num_rounds = 64;
	const uint32_t key[4] = { 0x12345678, 0x98765432, 0x01234567, 0x87654321 };

	uint32_t v[2];
	v[0] = static_cast<uint32_t>(offset & 0xffffffffLL);
	v[1] = static_cast<uint32_t>((offset >> 32) & 0xffffffffLL);

	encipher_xtea(num_rounds, v, key);

	uint64_t retVal = (static_cast<uint64_t>(v[1]) << 32)
		| static_cast<uint64_t>(v[0]);

	return retVal;
}

/**
 * Internal helper routine to copy a uint64_t to unsigned char.
 */
void TestBigFile::copydata(unsigned char *dataout, uint64_t datain)
{
	for(int i = 0; i < sizeof(uint64_t); i++)
	{
		*dataout = static_cast<unsigned char>(datain);
		dataout++;
		datain >>= 8;
	}
}

/**
 * Generate "random" data using xtea encryption.
 *
 * Using the same offset, the routine will generate the same data.
 */
void TestBigFile::gendata(int64_t offset, int64_t sz, unsigned char *data)
{
	unsigned char *lastdata = data + sz;
	int64_t first_offset = (offset & ~7LL);	// Round down to a multiple of 8 by clearing the lowest 3 bits
	int64_t first_remainder = offset - first_offset;
	int64_t offset_8_running = offset / 8;

	if(first_remainder > 0)
	{
		uint64_t first_gendata = gendata_single(first_offset / 8);
		for(int64_t i = 0; i < 8; i++)
		{
			if(i >= first_remainder)
			{
				*data = static_cast<unsigned char>(first_gendata);
				data++;
			}
			first_gendata >>= 8;
		}
		offset_8_running++;
	}

	int64_t sz_8 = sz / 8;
	if(first_remainder > 0)
		sz_8 = (sz - (8 - first_remainder)) / 8;
	for(int64_t i = 0; i < sz_8; i++)
	{
		uint64_t gendata = gendata_single(offset_8_running);
		copydata(data, gendata);
		data += 8;
		offset_8_running++;
	}

	if(data < lastdata)
	{
		uint64_t lastgendata = gendata_single(offset_8_running);
		while(data < lastdata)
		{
			*data = static_cast<unsigned char>(lastgendata);
			data++;
			lastgendata >>= 8;
		}
	}
}

void TestBigFile::test_bigchunk(int64_t offset)
{
	unsigned char buf[10240], bufc[1024];
	gendata(offset, 10240, buf);

	for(int64_t off = 0; off < 9000; off++)
	{
		gendata(offset + off, 1024, bufc);
		for(int64_t i = 0; i < 1024; i++)
		{
			if(buf[i + off] != bufc[i])
			{
				printf("Error at %lld\n", i);
			}
		}
	}

	for(int64_t off = 0; off < 10; off++)
	{
		for(int64_t sz = 900; sz < 950; sz++)
		{
			unsigned char testbuf[1000];
			for(int i = 0; i < 1000; i++)
				testbuf[i] = 0xab;
			gendata(off, sz, testbuf + 10);
			for(int i = 0; i < 10; i++)
				if(testbuf[i] != 0xab)
					printf("Error at %lld\n", i);
			for(int i = sz + 10; i < 1000; i++)
				if(testbuf[i] != 0xab)
					printf("Error at %lld\n", i);
		}
	}
}

/**
 * Create a file with "random" data.
 */
bool TestBigFile::gentestfile(const boost::filesystem::path &filename, int64_t filesize)
{
	FILE *fp = NULL;

	fp = TestFileHelper::fopen(filename, "wb");
	if(fp == NULL)
		return false;

	const int64_t bufsize = 1024;
	unsigned char buf[bufsize];
	int64_t remainingbytes = filesize;
	int64_t offset = 0;
	while(remainingbytes > 0)
	{
		int64_t curbytes = std::min(bufsize, remainingbytes);
		gendata(offset, curbytes, buf);
		fwrite(buf, 1, curbytes, fp);
		offset += curbytes;
		remainingbytes -= curbytes;
	}
	fclose(fp);

	return true;
}

/**
 * Read back the test file and compare with the written data.
 */
bool TestBigFile::checkfile(const boost::filesystem::path &filename, int64_t filesize)
{
	FILE *fp = NULL;
	fp = TestFileHelper::fopen(filename, "rb");
	if(fp == NULL)
		return false;

	const int64_t bufsize = 1024;
	unsigned char buf[bufsize], bufc[bufsize];

	for(int i = 0; i < 100; i++)
	{
		// Generate random number between 0 and 1
		float rnd = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
		int64_t sz = 1024LL * 1024LL;	//static_cast<int64_t>(rnd * static_cast<float>(filesize / 4));
		rnd = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
		//int64_t offset = static_cast<int64_t>(rnd * static_cast<float>(filesize - sz));
		int64_t offset = 1024LL * 1024LL * 1024LL * 2LL + static_cast<int64_t>(rnd * static_cast<float>(filesize - sz - 1024LL * 1024LL * 1024LL * 2LL));

		int retVal = _fseeki64(fp, offset, SEEK_SET);
		if(retVal != 0)
		{
			printf("Error in fseek: %d, %d\n", retVal, errno);
			return false;
		}
		int64_t remainingbytes = sz;
		int64_t cur_offset = offset;
		while(remainingbytes > 0)
		{
			int64_t curbytes = std::min(bufsize, remainingbytes);
			size_t retValfread = fread(buf, 1, curbytes, fp);
			if(retValfread != curbytes)
			{
				printf("Error in fread: %d, %d\n", retValfread, errno);
				return false;
			}
			gendata(cur_offset, curbytes, bufc);
			for(int64_t j = 0; j < curbytes; j++)
			{
				if(buf[j] != bufc[j])
				{
					printf("Difference: %d\n", j);
					return false;
				}
			}
			cur_offset += curbytes;
			remainingbytes -= curbytes;
		}
	}

	fclose(fp);
	return true;
}

/**
 * Creates a test file of 3GB size, then reads it back to check data integrity.
 *
 * This test mostly checks whether the file system supports files > 2GB.
 */
bool TestBigFile::runTest(const boost::filesystem::path &testpath)
{
	boost::filesystem::path testfile(testpath);
	testfile /= boost::filesystem::unique_path();

	const __int64 filesize = 1024LL * 1024LL * 1024LL * 3LL; // 3GB

	if(!gentestfile(testfile, filesize))
		return false;
	if(!checkfile(testfile, filesize))
		return false;

	boost::system::error_code ec;
	if(!boost::filesystem::remove(testfile, ec))
		return false;

	return true;
}