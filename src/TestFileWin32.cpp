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
#include "TestFileWin32.h"
#include "TestFileHelper.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include <cstring>

void TestFileWin32::printLastError()
{
	DWORD errCode = GetLastError();
	wchar_t *err;
	if(!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
		(LPTSTR)&err,
		0,
		NULL))
		return;

	//	static char buffer[1024];
	//	_snprintf(buffer, sizeof(buffer), "ERROR: %d: %s\n", errCode, err);
	//	OutputDebugStringA(buffer);
	std::wcerr << L"ERROR: " << errCode << L": "
		<< err << std::endl;
	LocalFree(err);
}

bool TestFileWin32::supersedeTest(const boost::filesystem::path &testpath)
{
	bool retVal = true;
	boost::system::error_code ec;

	// Create test directory
	boost::filesystem::path wintestpath = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(wintestpath, ec))
		return false;

	boost::filesystem::path testfilename = wintestpath / L"encfsmp_test_supersede.txt";
	std::wstring testfilenameW = testfilename.wstring();

	// Create file
	FILE *fd = TestFileHelper::fopen(testfilename, "wb");
	if(fd == NULL)
		retVal = false;
	unsigned char buf[10];
	for(int i = 0; i < 10; i++)
		buf[i] = static_cast<unsigned char>(i);
	fwrite(buf, 1, 10, fd);
	fclose(fd);

	// CREATE_ALWAYS on an existing file supersedes/truncates its content - the
	// closest equivalent to FILE_SUPERSEDE reachable through the Win32 API
	// (there is no dwCreationDisposition value that maps to the native
	// FILE_SUPERSEDE disposition itself; CREATE_ALWAYS maps to FILE_OVERWRITE_IF).
	HANDLE fh = CreateFileW(testfilenameW.c_str(), GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(fh == INVALID_HANDLE_VALUE)
	{
		printLastError();
		retVal = false;
	}
	else
	{
		const char buf[] = "Hallo";
		DWORD bytesWritten = 0;
		BOOL isOK = WriteFile(fh, buf, sizeof(buf), &bytesWritten, NULL);

		if(!isOK)
		{
			printLastError();
			retVal = false;
		}

		CloseHandle(fh);
	}

	// Delete file
	if(!boost::filesystem::remove(testfilename, ec))
		retVal = false;

	// Remove directory
	if(!boost::filesystem::remove_all(wintestpath, ec))
		retVal = false;

	return retVal;
}

bool TestFileWin32::replaceTest(const boost::filesystem::path &testpath)
{
	/*
	isOK = ReplaceFile(TESTPATH L"one.txt",			// lpReplacedFileName
		TESTPATH L"two.txt",						// lpReplacementFileName
		NULL, REPLACEFILE_IGNORE_MERGE_ERRORS, 0, 0);

	if(!isOK)
		printLastError();

	printFileInfo(TESTPATH L"one.txt");
	printFileInfo(TESTPATH L"two.txt");
	printFileInfo(TESTPATH L"three.txt");
	*/

	return true;
}

bool TestFileWin32::checkPFMRegrTest(const boost::filesystem::path &testpath)
{
	boost::filesystem::path testfilename = testpath / L"pfmregrtestfile.txt";
	std::wstring testfilenameW = testfilename.c_str();
	const wchar_t *filename = testfilenameW.c_str();

	// Make sure test file does not yet exist
	DWORD fileAttrs = GetFileAttributesW(filename);
	if(fileAttrs != INVALID_FILE_ATTRIBUTES)
	{
		DeleteFileW(filename);
	}

	// Create file
	HANDLE fh;
	fh = CreateFile(filename,
		GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if(fh == INVALID_HANDLE_VALUE)
	{
		printLastError();
		return false;
	}

	const char buf[] = "abcdefgh";
	DWORD bytesWritten = 0, bytesRead = 0;
	BOOL isOK = WriteFile(fh, buf, strlen(buf), &bytesWritten, NULL);

	if(!isOK)
		printLastError();
	CloseHandle(fh);

	// Sleep 2 seconds to let the filesystem settle
	Sleep(2000);

	// Open file again
	fh = CreateFile(filename,
		GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING,	// TRUNCATE_EXISTING
		FILE_ATTRIBUTE_NORMAL, NULL);
	if(fh == INVALID_HANDLE_VALUE)
	{
		printLastError();
		return false;
	}

	const char buf2[] = "abcdefghi";
	isOK = WriteFile(fh, buf2, strlen(buf2), &bytesWritten, NULL);

	if(!isOK)
		printLastError();
	CloseHandle(fh);

	bool retVal = true;

	// Check file size
	WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
	isOK = GetFileAttributesEx(filename, GetFileExInfoStandard, &fileAttrData);
	if(!isOK)
		printLastError();
	int64_t fs = static_cast<int64_t>(fileAttrData.nFileSizeLow)
		| (static_cast<int64_t>(fileAttrData.nFileSizeHigh) << 32);
//	std::wcout << L"File size: " << fs << std::endl;
	if(fs == 9)//13)
		retVal = true;
//		std::wcout << L"OK" << std::endl;
	else
		retVal = false;
//		std::wcout << L"Error!" << std::endl;

	// Delete file
	isOK = DeleteFileW(filename);
	if(!isOK)
		printLastError();

	return retVal;
}

bool TestFileWin32::deleteWhileOpenTest(const boost::filesystem::path &testpath)
{
	bool retVal = true;
	boost::system::error_code ec;

	boost::filesystem::path wintestpath = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(wintestpath, ec))
		return false;

	boost::filesystem::path testfilename = wintestpath / L"encfsmp_test_delete_open.txt";
	std::wstring testfilenameW = testfilename.wstring();

	const char content[] = "0123456789";

	// Create the file with known content
	FILE *fd = TestFileHelper::fopen(testfilename, "wb");
	if(fd == NULL)
	{
		boost::filesystem::remove_all(wintestpath, ec);
		return false;
	}
	fwrite(content, 1, sizeof(content) - 1, fd);
	fclose(fd);

	// Open a handle for reading, sharing delete access so DeleteFileW can
	// succeed while this handle stays open. This emulates the POSIX
	// unlink-while-open guarantee (deleteOp()'s doc comment: "file data must
	// remain accessible until the file is closed") on top of Windows, which
	// normally requires FILE_SHARE_DELETE to be requested explicitly.
	HANDLE readerHandle = CreateFileW(testfilenameW.c_str(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(readerHandle == INVALID_HANDLE_VALUE)
	{
		printLastError();
		boost::filesystem::remove_all(wintestpath, ec);
		return false;
	}

	// Delete the file while the reader handle is still open
	if(!DeleteFileW(testfilenameW.c_str()))
	{
		printLastError();
		retVal = false;
	}

	// The name must be gone immediately, even though the data is still
	// accessible through the still-open handle.
	DWORD attrsAfterDelete = GetFileAttributesW(testfilenameW.c_str());
	if(attrsAfterDelete != INVALID_FILE_ATTRIBUTES)
		retVal = false;

	// The still-open reader handle must still be able to read the original content
	char readBuf[16] = {};
	DWORD bytesRead = 0;
	BOOL readOk = ReadFile(readerHandle, readBuf, sizeof(content) - 1, &bytesRead, NULL);
	if(!readOk || bytesRead != sizeof(content) - 1 || memcmp(readBuf, content, sizeof(content) - 1) != 0)
		retVal = false;

	CloseHandle(readerHandle);

	// Now that the last handle is closed, the file must really be gone
	HANDLE reopenHandle = CreateFileW(testfilenameW.c_str(), GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(reopenHandle != INVALID_HANDLE_VALUE)
	{
		retVal = false;
		CloseHandle(reopenHandle);
	}
	else if(GetLastError() != ERROR_FILE_NOT_FOUND)
		retVal = false;

	if(!boost::filesystem::remove_all(wintestpath, ec))
		retVal = false;

	return retVal;
}

bool TestFileWin32::recreateAfterDeleteWhileOpenTest(const boost::filesystem::path &testpath)
{
	bool retVal = true;
	boost::system::error_code ec;

	boost::filesystem::path wintestpath = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(wintestpath, ec))
		return false;

	boost::filesystem::path testfilename = wintestpath / L"encfsmp_test_recreate.txt";
	std::wstring testfilenameW = testfilename.wstring();

	// Create the original file
	FILE *fd = TestFileHelper::fopen(testfilename, "wb");
	if(fd == NULL)
	{
		boost::filesystem::remove_all(wintestpath, ec);
		return false;
	}
	fputs("old", fd);
	fclose(fd);

	// Open a handle, share-delete, then delete it while the handle stays open
	HANDLE oldHandle = CreateFileW(testfilenameW.c_str(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(oldHandle == INVALID_HANDLE_VALUE)
	{
		printLastError();
		boost::filesystem::remove_all(wintestpath, ec);
		return false;
	}
	if(!DeleteFileW(testfilenameW.c_str()))
	{
		printLastError();
		retVal = false;
	}

	// A brand new file must be creatable under the same name right away - the
	// deleted (but still-open) old entry must not "block" the name.
	FILE *newFd = TestFileHelper::fopen(testfilename, "wb");
	if(newFd == NULL)
		retVal = false;
	else
	{
		fputs("new", newFd);
		fclose(newFd);
	}

	// Reading via the old handle must still see "old", never "new"
	char oldReadBuf[16] = {};
	DWORD bytesRead = 0;
	BOOL readOk = ReadFile(oldHandle, oldReadBuf, 3, &bytesRead, NULL);
	if(!readOk || bytesRead != 3 || memcmp(oldReadBuf, "old", 3) != 0)
		retVal = false;

	CloseHandle(oldHandle);

	// And a fresh open must see the new content
	FILE *checkFd = TestFileHelper::fopen(testfilename, "rb");
	if(checkFd == NULL)
		retVal = false;
	else
	{
		char checkBuf[16] = {};
		size_t n = fread(checkBuf, 1, 3, checkFd);
		if(n != 3 || memcmp(checkBuf, "new", 3) != 0)
			retVal = false;
		fclose(checkFd);
	}

	if(!boost::filesystem::remove(testfilename, ec))
		retVal = false;
	if(!boost::filesystem::remove_all(wintestpath, ec))
		retVal = false;

	return retVal;
}

namespace
{
	// Builds a FILETIME aligned to a whole second (so it survives the
	// filesystem's 1-second time resolution unchanged), offset from "now" by
	// the given number of seconds (may be negative).
	FILETIME makeTestFileTime(int64_t secondsFromNow)
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		FILETIME ft;
		SystemTimeToFileTime(&st, &ft);

		ULARGE_INTEGER uli;
		uli.LowPart = ft.dwLowDateTime;
		uli.HighPart = ft.dwHighDateTime;
		uli.QuadPart -= uli.QuadPart % 10000000ULL;	// round down to whole second
		uli.QuadPart += static_cast<uint64_t>(secondsFromNow * 10000000LL);

		FILETIME result;
		result.dwLowDateTime = uli.LowPart;
		result.dwHighDateTime = uli.HighPart;
		return result;
	}

	bool sameSecond(const FILETIME &a, const FILETIME &b)
	{
		ULARGE_INTEGER ua, ub;
		ua.LowPart = a.dwLowDateTime;
		ua.HighPart = a.dwHighDateTime;
		ub.LowPart = b.dwLowDateTime;
		ub.HighPart = b.dwHighDateTime;

		int64_t diff = static_cast<int64_t>(ua.QuadPart) - static_cast<int64_t>(ub.QuadPart);
		if(diff < 0)
			diff = -diff;
		return diff < 10000000LL;	// within 1 second
	}
}

bool TestFileWin32::fileTimeRoundTripTest(const boost::filesystem::path &testpath)
{
	bool retVal = true;
	boost::system::error_code ec;

	boost::filesystem::path wintestpath = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(wintestpath, ec))
		return false;

	boost::filesystem::path testfilename = wintestpath / L"encfsmp_test_filetime.txt";
	std::wstring testfilenameW = testfilename.wstring();

	HANDLE fh = CreateFileW(testfilenameW.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fh == INVALID_HANDLE_VALUE)
	{
		printLastError();
		boost::filesystem::remove_all(wintestpath, ec);
		return false;
	}

	// Only the write time is checked for round-trip fidelity here: fs_layer
	// (utimes/futimes) only ever plumbs a single timestamp through to the
	// backing file on both Windows and POSIX, so PFMLayer::FlushFile applies
	// WriteTime() to createTime_/accessTime_/changeTime_/writeTime_ alike.
	// Distinct creation/access-time fidelity would need a real fs_layer
	// extension (and, for creation time, a platform-specific setter, since
	// POSIX has none) - out of scope here, so not asserted.
	FILETIME writeTime = makeTestFileTime(-100);

	if(!SetFileTime(fh, NULL, NULL, &writeTime))
	{
		std::wcerr << L"[FILETIME-DEBUG] SetFileTime failed" << std::endl;
		printLastError();
		retVal = false;
	}
	CloseHandle(fh);

	// Reopen and read the time back
	HANDLE fh2 = CreateFileW(testfilenameW.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fh2 == INVALID_HANDLE_VALUE)
	{
		printLastError();
		boost::filesystem::remove_all(wintestpath, ec);
		return false;
	}

	FILETIME writeTimeOut = {};
	BOOL isOK = GetFileTime(fh2, NULL, NULL, &writeTimeOut);
	CloseHandle(fh2);

	if(!isOK)
	{
		std::wcerr << L"[FILETIME-DEBUG] GetFileTime failed" << std::endl;
		printLastError();
		retVal = false;
	}
	else if(!sameSecond(writeTime, writeTimeOut))
	{
		auto toU64 = [](const FILETIME &ft) -> uint64_t
		{
			ULARGE_INTEGER u;
			u.LowPart = ft.dwLowDateTime;
			u.HighPart = ft.dwHighDateTime;
			return u.QuadPart;
		};
		std::wcerr << L"[FILETIME-DEBUG] writeTime mismatch: expected=" << toU64(writeTime)
			<< L" actual=" << toU64(writeTimeOut) << std::endl;
		retVal = false;
	}

	if(!boost::filesystem::remove_all(wintestpath, ec))
		retVal = false;

	return retVal;
}

bool TestFileWin32::longNameCrashRepro(const boost::filesystem::path &testpath)
{
	bool retVal = true;
	boost::system::error_code ec;

	boost::filesystem::path wintestpath = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(wintestpath, ec))
		return false;

	// 200 chars is long enough that encfs's Block name encoding (padding to
	// the cipher block size, a 2-byte checksum, then base64) expands this
	// past the host filesystem's 255-char per-component limit, so the
	// underlying mkdir() fails. This must be reported back as a clean
	// CreateDirectoryW failure (ERROR_ACCESS_DENIED) - it must not crash the
	// mount-serving thread, which it used to: PFMLayer::createOp()'s folder
	// branch ignored mkdir()'s return value and happily registered a phantom
	// open-file entry for a directory that was never actually created, and
	// separately, logging a warning about the failure by itself crashed
	// whenever fired from a non-main thread with easylogging++ unconfigured.
	std::wstring longName(200, L'b');
	boost::filesystem::path dir = wintestpath / longName;
	if(CreateDirectoryW(dir.wstring().c_str(), NULL) || GetLastError() != ERROR_ACCESS_DENIED)
		retVal = false;

	if(!boost::filesystem::remove_all(wintestpath, ec))
		retVal = false;

	return retVal;
}

bool TestFileWin32::renameWhileOpenTest(const boost::filesystem::path &testpath)
{
	bool retVal = true;
	boost::system::error_code ec;

	boost::filesystem::path wintestpath = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(wintestpath, ec))
		return false;

	boost::filesystem::path oldName = wintestpath / L"encfsmp_test_rename_old.txt";
	boost::filesystem::path newName = wintestpath / L"encfsmp_test_rename_new.txt";
	std::wstring oldNameW = oldName.wstring();
	std::wstring newNameW = newName.wstring();

	const char content[] = "0123456789";

	FILE *fd = TestFileHelper::fopen(oldName, "wb");
	if(fd == NULL)
	{
		boost::filesystem::remove_all(wintestpath, ec);
		return false;
	}
	fwrite(content, 1, sizeof(content) - 1, fd);
	fclose(fd);

	// FILE_SHARE_DELETE is required for MoveFileExW to succeed against a
	// file that has an open handle - this mirrors deleteWhileOpenTest's
	// use of the same share flag for the analogous delete case.
	HANDLE fh = CreateFileW(oldNameW.c_str(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fh == INVALID_HANDLE_VALUE)
	{
		printLastError();
		boost::filesystem::remove_all(wintestpath, ec);
		return false;
	}

	if(!MoveFileExW(oldNameW.c_str(), newNameW.c_str(), 0))
	{
		printLastError();
		retVal = false;
	}

	// The old name must be gone immediately, even with the handle still open.
	DWORD attrsOld = GetFileAttributesW(oldNameW.c_str());
	if(attrsOld != INVALID_FILE_ATTRIBUTES)
		retVal = false;

	// The still-open handle must keep reading the same underlying file,
	// unaffected by the rename of its name.
	char readBuf[16] = {};
	DWORD bytesRead = 0;
	BOOL readOk = ReadFile(fh, readBuf, sizeof(content) - 1, &bytesRead, NULL);
	if(!readOk || bytesRead != sizeof(content) - 1 || memcmp(readBuf, content, sizeof(content) - 1) != 0)
		retVal = false;

	CloseHandle(fh);

	// After closing, the file must be reachable only under the new name.
	HANDLE reopenOld = CreateFileW(oldNameW.c_str(), GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(reopenOld != INVALID_HANDLE_VALUE)
	{
		retVal = false;
		CloseHandle(reopenOld);
	}

	HANDLE reopenNew = CreateFileW(newNameW.c_str(), GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(reopenNew == INVALID_HANDLE_VALUE)
	{
		printLastError();
		retVal = false;
	}
	else
		CloseHandle(reopenNew);

	if(!boost::filesystem::remove_all(wintestpath, ec))
		retVal = false;

	return retVal;
}

bool TestFileWin32::sharingViolationTest(const boost::filesystem::path &testpath)
{
	bool retVal = true;
	boost::system::error_code ec;

	boost::filesystem::path wintestpath = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(wintestpath, ec))
		return false;

	boost::filesystem::path testfilename = wintestpath / L"encfsmp_test_sharing.txt";
	std::wstring testfilenameW = testfilename.wstring();

	// Create the file, then reopen it exclusively (share mode 0) - no other
	// handle, reader or writer, may be opened concurrently.
	HANDLE fh = CreateFileW(testfilenameW.c_str(), GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fh == INVALID_HANDLE_VALUE)
	{
		printLastError();
		boost::filesystem::remove_all(wintestpath, ec);
		return false;
	}

	// A second open with no sharing granted by the first handle must fail
	// with ERROR_SHARING_VIOLATION, not succeed and not fail with some other
	// error.
	HANDLE fh2 = CreateFileW(testfilenameW.c_str(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fh2 != INVALID_HANDLE_VALUE)
	{
		retVal = false;
		CloseHandle(fh2);
	}
	else if(GetLastError() != ERROR_SHARING_VIOLATION)
	{
		printLastError();
		retVal = false;
	}

	CloseHandle(fh);

	// Once the exclusive handle is closed, the name must be openable again.
	HANDLE fh3 = CreateFileW(testfilenameW.c_str(), GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fh3 == INVALID_HANDLE_VALUE)
	{
		printLastError();
		retVal = false;
	}
	else
		CloseHandle(fh3);

	if(!boost::filesystem::remove_all(wintestpath, ec))
		retVal = false;

	return retVal;
}

bool TestFileWin32::truncateGrowTest(const boost::filesystem::path &testpath)
{
	bool retVal = true;
	boost::system::error_code ec;

	boost::filesystem::path wintestpath = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(wintestpath, ec))
		return false;

	boost::filesystem::path testfilename = wintestpath / L"encfsmp_test_truncate.txt";
	std::wstring testfilenameW = testfilename.wstring();

	const char content[] = "0123456789ABCDEF";	// 16 bytes

	HANDLE fh = CreateFileW(testfilenameW.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fh == INVALID_HANDLE_VALUE)
	{
		printLastError();
		boost::filesystem::remove_all(wintestpath, ec);
		return false;
	}

	DWORD bytesWritten = 0;
	if(!WriteFile(fh, content, sizeof(content) - 1, &bytesWritten, NULL) || bytesWritten != sizeof(content) - 1)
	{
		printLastError();
		retVal = false;
	}

	// Truncate down to 4 bytes via SetEndOfFile
	LARGE_INTEGER newPos;
	newPos.QuadPart = 4;
	if(!SetFilePointerEx(fh, newPos, NULL, FILE_BEGIN) || !SetEndOfFile(fh))
	{
		printLastError();
		retVal = false;
	}

	// Grow back to 20 bytes - the new region (bytes 4..19) must read back as
	// zero-fill, never leftover/stale data from before the truncate.
	newPos.QuadPart = 20;
	if(!SetFilePointerEx(fh, newPos, NULL, FILE_BEGIN) || !SetEndOfFile(fh))
	{
		printLastError();
		retVal = false;
	}

	CloseHandle(fh);

	// Verify size and content by reopening
	HANDLE fh2 = CreateFileW(testfilenameW.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fh2 == INVALID_HANDLE_VALUE)
	{
		printLastError();
		boost::filesystem::remove_all(wintestpath, ec);
		return false;
	}

	LARGE_INTEGER fileSize = {};
	if(!GetFileSizeEx(fh2, &fileSize) || fileSize.QuadPart != 20)
	{
		printLastError();
		retVal = false;
	}

	unsigned char readBuf[20] = {};
	DWORD bytesRead = 0;
	BOOL readOk = ReadFile(fh2, readBuf, sizeof(readBuf), &bytesRead, NULL);
	CloseHandle(fh2);

	if(!readOk || bytesRead != sizeof(readBuf))
		retVal = false;
	else
	{
		if(memcmp(readBuf, content, 4) != 0)
			retVal = false;
		for(int i = 4; i < 20; i++)
		{
			if(readBuf[i] != 0)
			{
				retVal = false;
				break;
			}
		}
	}

	if(!boost::filesystem::remove_all(wintestpath, ec))
		retVal = false;

	return retVal;
}

