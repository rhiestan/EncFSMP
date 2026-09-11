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

#ifndef TESTMOUNT_H
#define TESTMOUNT_H

/**
 * Creates a fresh, temporary encfs volume and mounts it through PFMLayer/FSR
 * for the duration of the test run, so the mount-dependent tests in
 * EncFSMPTestApp.cpp have a real filesystem to exercise instead of requiring
 * the caller to have mounted one by hand beforehand (e.g. via "EncFSMPTest
 * Z:\\" with Z: already mounted).
 */
class TestMount
{
public:
	/**
	 * Creates a temporary encfs volume and mounts it at mountPath (its first
	 * character is used as the drive letter, e.g. "Z:\\"). Returns true on
	 * success. On failure, errorMsg is set and any partial state is cleaned
	 * up; the caller can still proceed - the mount-dependent tests will
	 * simply fail with their own errors, exactly as if no mount had been
	 * attempted at all.
	 */
	static bool mount(const wxString &mountPath, wxString &errorMsg);

	/**
	 * Unmounts and deletes the temporary volume created by mount(). Safe to
	 * call even if mount() was never called, or failed.
	 */
	static void unmount();

private:
	TestMount() { }
	virtual ~TestMount() { }
};

#endif
