//----------------------------------------------------------------------------
// Copyright 2012-2013 Joe Lowe
//
// Permission is granted to any person obtaining a copy of this Software,
// to deal in the Software without restriction, including the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and sell copies of
// the Software.
//
// The above copyright and permission notice must be left intact in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS WITHOUT WARRANTY.
//----------------------------------------------------------------------------
// file name:  portability.cpp
// created:    2012.05.04
//----------------------------------------------------------------------------

#ifdef _WIN32

// Necessary for MinGW headers to define INT64_C properly
#define __STDC_CONSTANT_MACROS
#include <stdint.h>

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned uint32_t;

#if !defined(__MINGW32__)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif

#if !defined(INT64_C)
#	define INT64_C(c) c##I64
#endif
#if !defined(UINT64_C)
#	define UINT64_C(c) c##UI64
#endif
#define CCALL __cdecl
#pragma warning(disable: 4100) // Unreferenced parameter.
#pragma warning(disable: 4355) // This reference in base constructor list.
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#undef __GOT_SECURE_LIB__
#undef __STDC_SECURE_LIB__
#else
#define __STDC_CONSTANT_MACROS
#include <stdint.h>
#define CCALL
#endif
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#define IGNORE_LEAK(e)
#define ASSERT(e)

#include <stddef.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x500
// Workaround header incompatibilities between VC11 headers
// and older SDK.
#undef __useHeader
#undef __on_failure
#include <windows.h>
#define FD_T HANDLE
#define FD_INVALID INVALID_HANDLE_VALUE

#else

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#define FD_T int
#define FD_INVALID -1

#endif

#ifdef _WIN32

static int sswcmpf(const wchar_t* a,const wchar_t* b)
{
   return _wcsicmp(a,b);
}

static HANDLE stdout_fd(void)
{
   return GetStdHandle(STD_OUTPUT_HANDLE);
}

static void close_fd(HANDLE fd)
{
   if(fd != FD_INVALID)
   {
      CloseHandle(fd);
   }
}

static long pfmLastPipeInstance = 0;

static int/*err*/ create_socket_(int/*bool*/ bidir,int/*bool*/ async,
   HANDLE* outHandle1,HANDLE* outHandle2)
{
   int err = 0;
   HANDLE handle1;
   HANDLE handle2 = INVALID_HANDLE_VALUE;
   int options1 = FILE_FLAG_FIRST_PIPE_INSTANCE;
   int options2 = 0;
   int access2 = GENERIC_WRITE;
   __int64 now;
   static const size_t maxPipeNameChars = 100;
   wchar_t pipeName[maxPipeNameChars];

   if(bidir)
   {
      options1 |= PIPE_ACCESS_DUPLEX;
      access2 |= GENERIC_READ;
   }
   else
   {
      options1 |= PIPE_ACCESS_INBOUND;
   }
   if(async)
   {
      options1 |= FILE_FLAG_OVERLAPPED;
      options2 |= FILE_FLAG_OVERLAPPED;
   }
      // CreatePipe creates handles that do not support overlapped IO and
      // are unidirectional. Use named pipe instead.
   GetSystemTimeAsFileTime(reinterpret_cast< ::FILETIME*>(&now));
   wsprintfW(pipeName,L"\\\\.\\Pipe\\AnonymousPipe.%i.%p.%i.%I64i",
      GetCurrentProcessId(),&pfmLastPipeInstance,
      InterlockedIncrement(&pfmLastPipeInstance),now);
   handle1 = CreateNamedPipeW(pipeName,options1,PIPE_TYPE_BYTE|
      PIPE_READMODE_BYTE|PIPE_WAIT,1/*pipeCount*/,0/*outBufSize*/,
      0/*inBufSize*/,30*1000/*timeout*/,0/*security*/);
   if(!handle1)
   {
      handle1 = INVALID_HANDLE_VALUE;
   }
   if(handle1 == INVALID_HANDLE_VALUE)
   {
      err = GetLastError();
   }
   if(!err)
   {
      handle2 = CreateFileW(pipeName,access2,0/*shareMode*/,0/*security*/,
         OPEN_EXISTING,options2,0/*template*/);
      if(handle2 == INVALID_HANDLE_VALUE)
      {
         err = GetLastError();
         CloseHandle(handle1);
         handle1 = INVALID_HANDLE_VALUE;
      }
   }

   *outHandle1 = handle1;
   *outHandle2 = handle2;
   return err;
}

static int/*err*/ create_pipe(HANDLE* read,HANDLE* write)
{
   return create_socket_(0/*bidir*/,1/*async*/,read,write);
}

#else

static int sswcmpf(const wchar_t* a,const wchar_t* b)
{
   return wcscmp(a,b);
}

static int stdout_fd(void)
{
   return 1;
}

static void close_fd(int fd)
{
   if(fd != FD_INVALID)
   {
      close(fd);
   }
}

static int/*err*/ create_pipe(int* read,int* write)
{
   int err = 0;
   int fds[2];

      // pipe() sys call is not good enough on some platforms.
      // Instead use unidirectional domain socket.
   if(socketpair(AF_UNIX,SOCK_STREAM,0,fds) != 0)
   {
      err = errno;
      fds[0] = -1;
      fds[1] = -1;
   }
   if(!err)
   {
         // Convert socket to uni-directional.
      shutdown(fds[0],SHUT_WR);
      shutdown(fds[1],SHUT_RD);
   }

   *read = fds[0];
   *write = fds[1];
   return err;
}

#endif
