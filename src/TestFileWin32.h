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

#ifndef TEST_FILE_WIN32_H
#define TEST_FILE_WIN32_H

#include "boost/filesystem.hpp"

class TestFileWin32
{
public:

	static bool supersedeTest(const boost::filesystem::path &testpath);
	static bool replaceTest(const boost::filesystem::path &testpath);
	static bool checkPFMRegrTest(const boost::filesystem::path &testpath);

	// Deletes a file while a handle to it (opened with FILE_SHARE_DELETE) stays
	// open. The name must disappear immediately, but the still-open handle must
	// keep reading the original data until it is closed.
	static bool deleteWhileOpenTest(const boost::filesystem::path &testpath);

	// After deleting a file while a handle to it is still open, a new file
	// must be creatable under the same name right away, independent of the
	// old (deleted, but still-open) file's data.
	static bool recreateAfterDeleteWhileOpenTest(const boost::filesystem::path &testpath);

	// SetFileTime's write time must round-trip through GetFileTime, within the
	// filesystem's time resolution.
	static bool fileTimeRoundTripTest(const boost::filesystem::path &testpath);

	// Regression guard: creating a directory whose name is long enough that
	// encfs's Block filename encoding pushes the ciphertext past the host
	// filesystem's 255-char per-component limit must fail cleanly with
	// ERROR_ACCESS_DENIED. This used to instead crash the mount-serving
	// thread - see PFMLayer::createOp()'s folder branch and
	// EncFSMPTestApp::OnInit()'s EncFSMPLogger::setup() call.
	static bool longNameCrashRepro(const boost::filesystem::path &testpath);

	// A non-shared handle (share mode 0) must make a concurrent open attempt
	// fail with ERROR_SHARING_VIOLATION, and once that first handle is closed
	// the name must become openable again.
	static bool sharingViolationTest(const boost::filesystem::path &testpath);

	// SetEndOfFile-based truncate must discard data beyond the new size, and
	// growing a file that way must zero-fill the new region rather than
	// exposing stale/uninitialized bytes.
	static bool truncateGrowTest(const boost::filesystem::path &testpath);

protected:
	static void printLastError();

private:
	TestFileWin32() { }
	virtual ~TestFileWin32() { }
};

#endif
