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

// Internal NT APIs: Requires Windows SDK
#include "Winternl.h"

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

static void displayNtError(DWORD errCode, HMODULE hMod)
{
	//LPVOID lpMessageBuffer;
	wchar_t *err;
	if(!FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_FROM_HMODULE,
		hMod,
		errCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&err,	//(LPTSTR) &lpMessageBuffer,  
		0,
		NULL))
		return;

	// Now display the string.
	std::wcerr << L"ERROR: " << errCode << L": "
		<< err << std::endl;

	// Free the buffer allocated by the system.
	LocalFree(err);
}

#if defined(_MSCVER)
typedef __kernel_entry NTSTATUS(NTAPI *PNtCreateFile)(
	OUT PHANDLE FileHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	IN PLARGE_INTEGER AllocationSize OPTIONAL,
	IN ULONG FileAttributes,
	IN ULONG ShareAccess,
	IN ULONG CreateDisposition,
	IN ULONG CreateOptions,
	IN PVOID EaBuffer OPTIONAL,
	IN ULONG EaLength
	);
typedef __kernel_entry NTSTATUS(NTAPI *PNtClose)(
	IN HANDLE Handle
	);
typedef NTSTATUS(NTAPI *PNtWriteFile)(
	_In_      HANDLE FileHandle,
	_In_opt_  HANDLE Event,
	_In_opt_  PIO_APC_ROUTINE ApcRoutine,
	_In_opt_  PVOID ApcContext,
	_Out_     PIO_STATUS_BLOCK IoStatusBlock,
	_In_      PVOID Buffer,
	_In_      ULONG Length,
	_In_opt_  PLARGE_INTEGER ByteOffset,
	_In_opt_  PULONG Key
	);
typedef VOID(__stdcall *PRtlInitUnicodeString)(
	PUNICODE_STRING DestinationString,
	PCWSTR SourceString
	);
#endif

bool TestFileWin32::supersedeTest(const boost::filesystem::path &testpath)
{
	bool retVal = true;
	boost::system::error_code ec;

	HANDLE fh;
	HMODULE hMod;
	hMod = LoadLibrary(L"NtDll.dll");
	if(hMod == NULL)
	{
		printLastError();
		return false;
	}

#if defined(_MSCVER)
	PNtCreateFile pNtCreateFile = (PNtCreateFile)GetProcAddress(hMod, "NtCreateFile");
	PNtClose pNtClose = (PNtClose)GetProcAddress(hMod, "NtClose");
	PNtWriteFile pNtWriteFile = (PNtWriteFile)GetProcAddress(hMod, "NtWriteFile");
	PRtlInitUnicodeString pRtlInitUnicodeString = (PRtlInitUnicodeString)GetProcAddress(hMod, "RtlInitUnicodeString");

	// Create test directory
	boost::filesystem::path wintestpath = testpath / boost::filesystem::unique_path();
	if(!boost::filesystem::create_directories(wintestpath, ec))
		return false;

	boost::filesystem::path testfilename = wintestpath / L"encfsmp_test_supersede.txt";
	std::wstring testfilenameW = testfilename.c_str();
	const wchar_t *filename = testfilenameW.c_str();

	// Create file
	FILE *fd = TestFileHelper::fopen(testfilename, "wb");
	if(fd == NULL)
		retVal = false;
	unsigned char buf[10];
	for(int i = 0; i < 10; i++)
		buf[i] = static_cast<unsigned char>(i);
	fwrite(buf, 1, 10, fd);
	fclose(fd);

	// Call NtCreateFile
	OBJECT_ATTRIBUTES objectAttrs;
	IO_STATUS_BLOCK ioStatus;
	UNICODE_STRING filenameUS;

	(*pRtlInitUnicodeString)(&filenameUS, filename);
	InitializeObjectAttributes(&objectAttrs,
		&filenameUS,
		0, NULL, NULL);

	NTSTATUS ntStatus = (*pNtCreateFile)(&fh, FILE_GENERIC_WRITE,
		&objectAttrs, &ioStatus, 0, FILE_ATTRIBUTE_NORMAL,
		0, FILE_SUPERSEDE, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

	if(!NT_SUCCESS(ntStatus))
	{
		displayNtError(ntStatus, hMod);
		displayNtError(ioStatus.Status, hMod);
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

		(*pNtClose)(fh);
	}

	if(hMod != INVALID_HANDLE_VALUE)
		FreeLibrary(hMod);

	// Delete file
	if(!boost::filesystem::remove(testfilename, ec))
		retVal = false;

	// Remove directory
	if(!boost::filesystem::remove_all(wintestpath, ec))
		retVal = false;
#endif
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
