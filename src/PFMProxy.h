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

#ifndef PFMPROXY_H
#define PFMPROXY_H

class PFMMonitorThread;

#include <stdlib.h>
#include <string>
#include <vector>

// FSR API (Pismo File System Runtime): types and interfaces only.
// FSRAPI_RUNTIME_LINK must NOT be defined here. The runtime-link section of
// fsrapi.h declares its loader state (module handle and function pointer
// table) as "static" while the FsrXxx() wrappers around it have external
// linkage. Including it in more than one translation unit therefore breaks
// the one-definition rule: the linker keeps one arbitrary copy of each
// wrapper, so a wrapper may end up reading a function pointer table that was
// never filled in, and the call jumps to NULL. PFMProxy.cpp is the single
// translation unit that owns the loader; everybody else goes through the
// fsr* methods below.
#include "fsrapi.h"

/**
 * malloc_i implementation forwarding to the C heap, with allocation tracking
 * so that all blocks handed out to the FSR API are freed together when the
 * instance goes out of scope.
 *
 * The memory MUST be zero initialized. This is not documented anywhere in the
 * FSR kit, but the runtime relies on it: for every string in FsrMountInfo it
 * allocates strlen+1 bytes and then copies only strlen bytes, leaving the
 * terminating zero to come from the buffer itself. With plain malloc() the
 * strings come back unterminated, so comparing formatterName or
 * mountSourceName reads past the end of the block and never matches.
 */
struct FsrMalloc_i : malloc_i
{
   std::vector<void*> allocs_;

   void* PT_CCALL alloc(PT_SIZE_T size) override
   {
      void* ptr = ::calloc(size, 1);
      if(ptr)
         allocs_.push_back(ptr);
      return ptr;
   }

   ~FsrMalloc_i()
   {
      for(size_t i = 0; i < allocs_.size(); i++)
         ::free(allocs_[i]);
   }
};

class PFMProxy
{
public:
   PFMProxy();
   virtual ~PFMProxy();

   void initialize();
   bool startMonitorThread();
   void closingDown();
   void uninitialize();
   bool isPFMPresent() const { return isPFMPresent_; }
   std::string getPFMVersionString() const { return pfmVersionString_; }
   bool unmount(const wxString &mountName);

   /**
    * Turns an FSR error code into readable text. The FsrXxx() functions return
    * plain system error codes (GetLastError() values on Windows, errno
    * elsewhere), not the fsrError* enum, which is what FsrConvertErrToPerr()
    * translates them into.
    */
   static std::string formatSystemError(int err);


   // Proxy functions for FSR API calls
   /* Proxy for FsrApiLoad() */
   int fsrApiLoad();

   /* Proxy for FsrApiUnload() */
   void fsrApiUnload();

   /* Proxy for FsrSysStart( pin_i* cancelPin) */
   int fsrSysStart(pin_i* cancelPin);

   /* Proxy for FsrMountCreate( PT_FD_T* fd, pin_i* cancelPin, const FsrMountCreateParams* mcp) */
   int fsrMountCreate(PT_FD_T* fd, pin_i* cancelPin, const FsrMountCreateParams* mcp);

   /* Proxy for FsrMountUnmount( PT_FD_T fd, pin_i* cancelPin, int unmountFlags) */
   int fsrMountUnmount(PT_FD_T fd, pin_i* cancelPin, int unmountFlags);

   /* Proxy for FsrMountFlush( PT_FD_T fd) */
   int fsrMountFlush(PT_FD_T fd);

   /* Proxy for FsrMountControl(...) */
   int fsrMountControl(PT_FD_T fd, PT_UINT8 scope, PT_SZC_T interfaceId, int controlCode, const void* input, PT_SIZE_T inputSize, void* output, PT_SIZE_T maxOutputSize, PT_SIZE_T* outputSize);

   /* Proxy for FsrMountGetInfo( malloc_i* ma, FsrMountInfo** mi, PT_FD_T fd) */
   int fsrMountGetInfo(malloc_i* ma, FsrMountInfo** mi, PT_FD_T fd);

   /* Proxy for FsrRetainMountListChangePin( pin_i* cancelPin) */
   pin_i* fsrRetainMountListChangePin(pin_i* cancelPin);

   /* Proxy for FsrReleaseMountListChangePin( pin_i* pin) */
   void fsrReleaseMountListChangePin(pin_i* pin);

   /* Proxy for FsrMountGetList(...) */
   PT_SIZE_T fsrMountGetList(malloc_i* ma, FsrMountInfo*** mi, unsigned* driveLetterMask, PT_SZC_T appId);

   /* Proxy for FsrMountOpenByInfo( PT_FD_T* fd, const FsrMountInfo* mi) */
   int fsrMountOpenByInfo(PT_FD_T* fd, const FsrMountInfo* mi);

   /* Proxy for FsrMountOpenByFile( PT_FD_T* fd, PT_SZC_T file) */
   int fsrMountOpenByFile(PT_FD_T* fd, PT_SZC_T file);

   /* Proxy for FsrMountOpenByMountPoint( PT_FD_T* fd, PT_SZC_T mountPoint) */
   int fsrMountOpenByMountPoint(PT_FD_T* fd, PT_SZC_T mountPoint);

   /* Proxy for FsrMountGetInfoByFile( malloc_i* ma, FsrMountInfo** mi, PT_SZC_T file) */
   int fsrMountGetInfoByFile(malloc_i* ma, FsrMountInfo** mi, PT_SZC_T file);

   /* Proxy for FsrMountGetInfoByMountPoint( malloc_i* ma, FsrMountInfo** mi, PT_SZC_T mountPoint) */
   int fsrMountGetInfoByMountPoint(malloc_i* ma, FsrMountInfo** mi, PT_SZC_T mountPoint);

   /* Proxy for FsrCreateDriveLetterLink( char driveLetter, PT_SZC_T target) */
   int fsrCreateDriveLetterLink(char driveLetter, PT_SZC_T target);

   /* Proxy for FsrDeleteDriveLetterLink( char driveLetter) */
   void fsrDeleteDriveLetterLink(char driveLetter);

   /* Proxy for FsrConvertErrToPerr( int err) */
   int fsrConvertErrToPerr(int err);

   /* Proxy for FsrSystemCreatePipe(...) */
   int fsrSystemCreatePipe(PT_FD_T* readFd, PT_FD_T* writeFd);

   /* Proxy for FsrSystemCreateSocket( PT_FD_T* fd1, PT_FD_T* fd2) */
   int fsrSystemCreateSocket(PT_FD_T* fd1, PT_FD_T* fd2);

   /* Proxy for FsrSystemCloseFd( PT_FD_T fd) */
   void fsrSystemCloseFd(PT_FD_T fd);

   /* Proxy for FsrServerFactory(...) */
   int fsrServerFactory(FsrServer** server, PT_FD_T transport, PT_SZC_T transportSecurity, PT_FD_T status, PT_SZC_T traceChannelName, PT_SZC_T formatterName, int volumeFlags);

   /* Proxy for FsrCacheLoad(...) */
   int fsrCacheLoad(malloc_i* ma, pin_i* cancelPin, PT_SZ_T* message, PT_SZ_T* handle, PT_FD_T* controlFd, PT_FD_T storageFd, PT_UINT64 storageOff, PT_UINT64 storageSize);

   /* Proxy for FsrCacheUnload( PT_FD_T fd) */
   int fsrCacheUnload(PT_FD_T fd);

   /* Proxy for FsrCachePoll( PT_FD_T fd) */
   int fsrCachePoll(PT_FD_T fd);

   /* Proxy for FsrCacheGetFileStatus(...) */
   int fsrCacheGetFileStatus(PT_FD_T fd, PT_SIZE_T fileCount, const FsrCacheFileInfo* fileTable, PT_UINT64* statusTable);

   /* Proxy for FsrReport( malloc_i* ma) */
   PT_SZ_T fsrReport(malloc_i* ma);

   /* Proxy for FsrMountReport( malloc_i* ma, PT_FD_T fd) */
   PT_SZ_T fsrMountReport(malloc_i* ma, PT_FD_T fd);

   /* Proxy for FsrClientFactory(...) */
   int fsrClientFactory(FsrClient** client, PT_SZC_T traceName, int clientFlags, int clientFileNameType, pt_transport2_i** transport, PT_SZC_T transportSecurity);

   /* Proxy for FsrFormatterResFactory(...) */
   int fsrFormatterResFactory(FsresVolume** volume, pin_i* cancelPin, FsrFormatter** formatter);

   /* Proxy for FsrResFormatterFactory( FsrFormatter** formatter, FsresVolume* volume) */
   int fsrResFormatterFactory(FsrFormatter** formatter, FsresVolume* volume);

   /* Proxy for FsrShimFormatterFactory( FsrFormatter** formatter, PfmFormatterDispatch* dispatch) */
   int fsrShimFormatterFactory(FsrFormatter** formatter, PfmFormatterDispatch* dispatch);

   /* Proxy for FsrReadOnlyFormatterFactory( FsrFormatter** formatter) */
   int fsrReadOnlyFormatterFactory(FsrFormatter** formatter);

   /* Proxy for FsrFixNamesFormatterFactory( FsrFormatter** formatter, int serverFileNameType, int clientFileNameType) */
   int fsrFixNamesFormatterFactory(FsrFormatter** formatter, int serverFileNameType, int clientFileNameType);

   /* Proxy for FsrLogFormatterFactory( FsrFormatter** formatter, PT_SZC_T logName) */
   int fsrLogFormatterFactory(FsrFormatter** formatter, PT_SZC_T logName);

   /* Proxy for FsrPolicyFormatterFactory( FsrFormatter** formatter, FsrPolicyFormatterOwner* owner) */
   int fsrPolicyFormatterFactory(FsrFormatter** formatter, FsrPolicyFormatterOwner* owner);

   /* Proxy for FsrCacheFormatterFactory(...) */
   int fsrCacheFormatterFactory(FsrFormatter** formatter, PT_SZC_T traceName, PT_SZC_T cacheHandle, PT_SZC_T volumeId, PT_SZC_T secret);

   /* Proxy for FsrRedirFormatterFactory(...) */
   int fsrRedirFormatterFactory(FsrFormatter** formatter, PT_SZC_T root_folder, int concurrency);


   static PFMProxy &getInstance() { return instance_; }

private:
   bool isInitialized_, isPFMPresent_;
   std::string pfmVersionString_;
   PFMMonitorThread *pPFMMonitorThread_;

   static PFMProxy instance_;
};

#endif
