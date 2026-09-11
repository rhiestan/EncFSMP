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

// PFMProxy::formatSystemError() is a pure function of an integer error code,
// so it can be tested directly without starting the FSR runtime or mounting
// anything. Including PFMProxy.h here is safe: it only declares types (it
// never defines FSRAPI_RUNTIME_LINK, which is confined to PFMProxy.cpp).

#include "CommonIncludes.h"

#include "doctest/doctest.h"

#include "PFMProxy.h"

TEST_SUITE("UnitTests")
{
	TEST_CASE("PFMProxy::formatSystemError")
	{
		SUBCASE("a known error code returns a readable, trimmed message")
		{
			// ERROR_FILE_NOT_FOUND on Windows and ENOENT on POSIX are both 2,
			// so this is a meaningful check on either platform.
			std::string msg = PFMProxy::formatSystemError(2);
			CHECK_FALSE(msg.empty());
			CHECK(msg != "unknown error");

			char last = msg[msg.length() - 1];
			CHECK(last != '.');
			CHECK(last != '\r');
			CHECK(last != '\n');
			CHECK(last != ' ');
		}

		SUBCASE("an unrecognized error code still returns a non-empty string")
		{
			std::string msg = PFMProxy::formatSystemError(0x7FFFFFFF);
			CHECK_FALSE(msg.empty());
		}
	}
}
