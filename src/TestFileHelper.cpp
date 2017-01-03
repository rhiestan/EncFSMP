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
#include "TestFileHelper.h"

#include <iostream>

#if defined(_WIN32)
#include <share.h>
#endif

FILE *TestFileHelper::fopen(const boost::filesystem::path &filename, const char *mode)
{
	FILE *fp = NULL;

#if defined(_WIN32)
	// Convert mode string to wide string (works since we know mode is ASCII-only)
	std::wstring mode_w(mode, mode + strlen(mode));

	// On Windows, boost::filesystem::path::c_str() returns a wchar_t *
	#if defined(_MSC_VER)
		errno_t err = _wfopen_s(&fp, filename.c_str(), mode_w.c_str());
		if(err != 0)
			std::cout << "_wfopen_s failed with return code " << err << std::endl;
	#else
		fp = _wfopen(filename.c_str(), mode_w.c_str());
	#endif
#else
	// On other platforms, c_str() is char *
	fp = fopen(filename.c_str(), mode);
#endif

	return fp;
}

FILE *TestFileHelper::fopen_shared(const boost::filesystem::path &filename, const char *mode)
{
	FILE *fp = NULL;

#if defined(_WIN32)
	// Convert mode string to wide string (works since we know mode is ASCII-only)
	std::wstring mode_w(mode, mode + strlen(mode));

	// On Windows, boost::filesystem::path::c_str() returns a wchar_t *
	fp = _wfsopen(filename.c_str(), mode_w.c_str(), _SH_DENYNO);
#else
	// On other platforms, c_str() is char *
	fp = fopen(filename.c_str(), mode);
#endif

	return fp;
}
