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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <vector>

// FSR API (Pismo File System Runtime) - runtime-linked via fsrapi.dll.
// This is the ONE translation unit in the program that may define
// FSRAPI_RUNTIME_LINK. The loader state it brings in (module handle and
// function pointer table) has internal linkage, while the FsrXxx() wrappers
// using it have external linkage. A second translation unit doing the same
// violates the one-definition rule: the linker keeps one arbitrary copy of
// each wrapper, so a wrapper can end up reading a function pointer table
// that was never filled in and the call jumps to NULL. All other units call
// the fsrXxx() methods of PFMProxy instead.
#define FSRAPI_RUNTIME_LINK fsrapi
#include "fsrapi.h"

#include "PFMProxy.h"
#include "PFMMonitorThread.h"

PFMProxy PFMProxy::instance_;

PFMProxy::PFMProxy():
	isInitialized_(false), isPFMPresent_(false),
	pPFMMonitorThread_(NULL)
{
}

PFMProxy::~PFMProxy()
{
}

void PFMProxy::initialize()
{
	if(!isInitialized_)
	{
		// Loading fsrapi.dll only proves that the client side API shim is
		// reachable. That DLL comes with the developer kit and may well sit next
		// to our own executable, so it loads fine even when the FSR runtime
		// (kernel module plus support daemons) was never installed. FsrSysStart()
		// is the first call that actually talks to that runtime, so it is what
		// decides whether mounting can work at all. Without this check the
		// application looks healthy and only fails later, at mount time, with a
		// bare Win32 error number.
		bool loaded = (fsrApiLoad() != 0);

		std::ostringstream ostr;
		ostr << "Loading FSR API, result: " << (loaded ? "success" : "failed") << std::endl;
		OutputDebugStringA(ostr.str().c_str());

		if(loaded)
		{
			int err = fsrSysStart(NULL);

			std::ostringstream ostr2;
			ostr2 << "Starting FSR runtime, result: " << err;
			if(err != 0)
				ostr2 << " (" << formatSystemError(err) << ")";
			ostr2 << std::endl;
			OutputDebugStringA(ostr2.str().c_str());

			if(err == 0)
			{
				isPFMPresent_ = true;
				pfmVersionString_ = "fsrapi";
			}
		}

		isInitialized_ = true;
	}
}


bool PFMProxy::startMonitorThread()
{
	if(!isPFMPresent_)
		return false;
	if(pPFMMonitorThread_ != NULL)
		return true;

	pPFMMonitorThread_ = new PFMMonitorThread();
	if(!pPFMMonitorThread_->startThread())
	{
		delete pPFMMonitorThread_;
		pPFMMonitorThread_ = NULL;
		return false;
	}
	
	return true;
}

void PFMProxy::closingDown()
{
	// Tell the monitor thread to stop sending events when application is closing down
	if(pPFMMonitorThread_ != NULL)
		pPFMMonitorThread_->sendEvents(false);
}

void PFMProxy::uninitialize()
{
	if(pPFMMonitorThread_ != NULL)
	{
		pPFMMonitorThread_->stopThread();
		delete pPFMMonitorThread_;
		pPFMMonitorThread_ = NULL;
	}

	fsrApiUnload();

	isInitialized_ = false;
}

std::string PFMProxy::formatSystemError(int err)
{
#if defined(_WIN32)
	char *pBuffer = NULL;
	DWORD len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, static_cast<DWORD>(err), 0, reinterpret_cast<LPSTR>(&pBuffer), 0, NULL);

	std::string msg;
	if(len != 0 && pBuffer != NULL)
		msg.assign(pBuffer, len);
	if(pBuffer != NULL)
		LocalFree(pBuffer);

	// FormatMessage appends a full stop and a line break
	while(!msg.empty()
		&& (msg[msg.length() - 1] == '\r' || msg[msg.length() - 1] == '\n'
			|| msg[msg.length() - 1] == '.' || msg[msg.length() - 1] == ' '))
		msg.erase(msg.length() - 1);
#else
	const char *pStr = strerror(err);
	std::string msg(pStr != NULL ? pStr : "");
#endif

	if(msg.empty())
		msg = "unknown error";
	return msg;
}

bool PFMProxy::unmount(const wxString &mountName)
{
	bool isOK = false;
	FsrMalloc_i fsrMalloc;
	FsrMountInfo** miList = NULL;
	size_t count = fsrMountGetList(&fsrMalloc, &miList, NULL, NULL);

	for(size_t i = 0; i < count; i++)
	{
		if(miList[i] == NULL || miList[i]->mountSourceName == NULL)
			continue;

		// Strip any leading path components, matching the old behaviour of
		// using wxFileName::GetFullName() on the (wide) source name.
		wxString sourceNameStr(miList[i]->mountSourceName, wxConvUTF8);
		wxFileName nameFN(sourceNameStr);
		wxString correctedName = nameFN.GetFullName();

		if(correctedName == mountName)
		{
			PT_FD_T mountFd = PT_FD_INVALID;
			int err = fsrMountOpenByInfo(&mountFd, miList[i]);
			if(err == 0 && mountFd != PT_FD_INVALID)
			{
				fsrMountUnmount(mountFd, NULL, 0);
				fsrSystemCloseFd(mountFd);
				isOK = true;
			}
		}
	}

	// fsrMalloc destructor frees all allocations made by FsrMountGetList.
	return isOK;
}

// The FsrXxx() wrappers below load fsrapi.dll on first use themselves, so no
// explicit initialization is needed in the proxy methods. fsrApiLoad() exists
// so that callers can find out up front whether the runtime is installed.

int PFMProxy::fsrApiLoad() {
	return FsrApiLoad();
}

void PFMProxy::fsrApiUnload() {
	FsrApiUnload();
}

// --- Simple Functions (Value/Void Returns) ---

int PFMProxy::fsrSysStart(pin_i* cancelPin) {
	return FsrSysStart(cancelPin);
}

int PFMProxy::fsrMountCreate(PT_FD_T* fd, pin_i* cancelPin, const FsrMountCreateParams* mcp) {
	return FsrMountCreate(fd, cancelPin, mcp);
}

int PFMProxy::fsrMountUnmount(PT_FD_T fd, pin_i* cancelPin, int unmountFlags) {
	return FsrMountUnmount(fd, cancelPin, unmountFlags);
}

int PFMProxy::fsrMountFlush(PT_FD_T fd) {
	return FsrMountFlush(fd);
}

int PFMProxy::fsrMountControl(PT_FD_T fd, PT_UINT8 scope, PT_SZC_T interfaceId, int controlCode, const void* input, PT_SIZE_T inputSize, void* output, PT_SIZE_T maxOutputSize, PT_SIZE_T* outputSize) {
	return FsrMountControl(fd, scope, interfaceId, controlCode, input, inputSize, output, maxOutputSize, outputSize);
}

// --- Get Info Functions (Integer Returns)---

int PFMProxy::fsrMountGetInfo(malloc_i* ma, FsrMountInfo** mi, PT_FD_T fd) {
	return FsrMountGetInfo(ma, mi, fd);
}

pin_i* PFMProxy::fsrRetainMountListChangePin(pin_i* cancelPin) {
	return FsrRetainMountListChangePin(cancelPin);
}

void PFMProxy::fsrReleaseMountListChangePin(pin_i* pin) {
	FsrReleaseMountListChangePin(pin);
}

PT_SIZE_T PFMProxy::fsrMountGetList(malloc_i* ma, FsrMountInfo*** mi, unsigned* driveLetterMask, PT_SZC_T appId) {
	return FsrMountGetList(ma, mi, driveLetterMask, appId);
}

int PFMProxy::fsrMountOpenByInfo(PT_FD_T* fd, const FsrMountInfo* mi) {
	return FsrMountOpenByInfo(fd, mi);
}

int PFMProxy::fsrMountOpenByFile(PT_FD_T* fd, PT_SZC_T file) {
	return FsrMountOpenByFile(fd, file);
}

int PFMProxy::fsrMountOpenByMountPoint(PT_FD_T* fd, PT_SZC_T mountPoint) {
	return FsrMountOpenByMountPoint(fd, mountPoint);
}

int PFMProxy::fsrMountGetInfoByFile(malloc_i* ma, FsrMountInfo** mi, PT_SZC_T file) {
	return FsrMountGetInfoByFile(ma, mi, file);
}

int PFMProxy::fsrMountGetInfoByMountPoint(malloc_i* ma, FsrMountInfo** mi, PT_SZC_T mountPoint) {
	return FsrMountGetInfoByMountPoint(ma, mi, mountPoint);
}

int PFMProxy::fsrCreateDriveLetterLink(char driveLetter, PT_SZC_T target) {
	return FsrCreateDriveLetterLink(driveLetter, target);
}

void PFMProxy::fsrDeleteDriveLetterLink(char driveLetter) {
	FsrDeleteDriveLetterLink(driveLetter);
}

int PFMProxy::fsrConvertErrToPerr(int err) {
	return FsrConvertErrToPerr(err);
}

// --- System Plumbing Functions (Integer/Void Returns)---

int PFMProxy::fsrSystemCreatePipe(PT_FD_T* readFd, PT_FD_T* writeFd) {
	return FsrSystemCreatePipe(readFd, writeFd);
}

int PFMProxy::fsrSystemCreateSocket(PT_FD_T* fd1, PT_FD_T* fd2) {
	return FsrSystemCreateSocket(fd1, fd2);
}

void PFMProxy::fsrSystemCloseFd(PT_FD_T fd) {
	FsrSystemCloseFd(fd);
}

// --- Factory Functions & Global Wrappers (Integer Returns)— -

int PFMProxy::fsrServerFactory(FsrServer** server, PT_FD_T transport, PT_SZC_T transportSecurity, PT_FD_T status, PT_SZC_T traceChannelName, PT_SZC_T formatterName, int volumeFlags) {
	return FsrServerFactory(server, transport, transportSecurity, status, traceChannelName, formatterName, volumeFlags);
}

int PFMProxy::fsrCacheLoad(malloc_i* ma, pin_i* cancelPin, PT_SZ_T* message, PT_SZ_T* handle, PT_FD_T* controlFd, PT_FD_T storageFd, PT_UINT64 storageOff, PT_UINT64 storageSize) {
	return FsrCacheLoad(ma, cancelPin, message, handle, controlFd, storageFd, storageOff, storageSize);
}

int PFMProxy::fsrCacheUnload(PT_FD_T fd) {
	return FsrCacheUnload(fd);
}

int PFMProxy::fsrCachePoll(PT_FD_T fd) {
	return FsrCachePoll(fd);
}

int PFMProxy::fsrCacheGetFileStatus(PT_FD_T fd, PT_SIZE_T fileCount, const FsrCacheFileInfo* fileTable, PT_UINT64* statusTable) {
	return FsrCacheGetFileStatus(fd, fileCount, fileTable, statusTable);
}

PT_SZ_T PFMProxy::fsrReport(malloc_i* ma) {
	return FsrReport(ma);
}

PT_SZ_T PFMProxy::fsrMountReport(malloc_i* ma, PT_FD_T fd) {
	return FsrMountReport(ma, fd);
}

int PFMProxy::fsrClientFactory(FsrClient** client, PT_SZC_T traceName, int clientFlags, int clientFileNameType, pt_transport2_i** transport, PT_SZC_T transportSecurity) {
	return FsrClientFactory(client, traceName, clientFlags, clientFileNameType, transport, transportSecurity);
}

int PFMProxy::fsrFormatterResFactory(FsresVolume** volume, pin_i* cancelPin, FsrFormatter** formatter) {
	return FsrFormatterResFactory(volume, cancelPin, formatter);
}

// --- Formatter Factories (Integer Returns)---

int PFMProxy::fsrResFormatterFactory(FsrFormatter** formatter, FsresVolume* volume) {
	return FsrResFormatterFactory(formatter, volume);
}

int PFMProxy::fsrShimFormatterFactory(FsrFormatter** formatter, PfmFormatterDispatch* dispatch) {
	return FsrShimFormatterFactory(formatter, dispatch);
}

int PFMProxy::fsrReadOnlyFormatterFactory(FsrFormatter** formatter) {
	return FsrReadOnlyFormatterFactory(formatter);
}

int PFMProxy::fsrFixNamesFormatterFactory(FsrFormatter** formatter, int serverFileNameType, int clientFileNameType) {
	return FsrFixNamesFormatterFactory(formatter, serverFileNameType, clientFileNameType);
}

int PFMProxy::fsrLogFormatterFactory(FsrFormatter** formatter, PT_SZC_T logName) {
	return FsrLogFormatterFactory(formatter, logName);
}

int PFMProxy::fsrPolicyFormatterFactory(FsrFormatter** formatter, FsrPolicyFormatterOwner* owner) {
	return FsrPolicyFormatterFactory(formatter, owner);
}

int PFMProxy::fsrCacheFormatterFactory(FsrFormatter** formatter, PT_SZC_T traceName, PT_SZC_T cacheHandle, PT_SZC_T volumeId, PT_SZC_T secret) {
	return FsrCacheFormatterFactory(formatter, traceName, cacheHandle, volumeId, secret);
}

int PFMProxy::fsrRedirFormatterFactory(FsrFormatter** formatter, PT_SZC_T root_folder, int concurrency) {
	return FsrRedirFormatterFactory(formatter, root_folder, concurrency);
}
