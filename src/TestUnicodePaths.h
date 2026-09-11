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

#ifndef TESTUNICODEPATHS_H
#define TESTUNICODEPATHS_H

#include "boost/filesystem.hpp"

class TestUnicodePaths
{
public:

	// A filename mixing CJK characters, a supplementary-plane emoji (surrogate
	// pair) and a combining-accent Latin character must round-trip through
	// create, write, read and directory listing unchanged.
	static bool unicodeFilenameTest(const boost::filesystem::path &testpath);

private:
	TestUnicodePaths() { }
	virtual ~TestUnicodePaths() { }
};

#endif
