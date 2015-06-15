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

#ifndef FS_LAYER_H
#define FS_LAYER_H

#include "config.h"
#include <string>

#if defined(HAVE_SYS_UTIME_H)
#include <sys/utime.h>
#elif defined(HAVE_UTIME_H)
#include <utime.h>
#endif

#if defined(HAVE_SYS_STAT_H)
#include <sys/stat.h>
#endif

#if defined(HAVE_STDINT_H)
#include <stdint.h>
#endif
#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif
#if defined(HAVE_SYS_DIR_H)
#include <sys/dir.h>
#endif

#include <boost/filesystem.hpp>

// Definition of internal off_t type
#if OFF_T_SIZE >= 8
typedef off_t efs_off_t;
#else
#	if defined(HAVE_OFF64_T)
typedef off64_t efs_off_t;
#	else
#		if defined(_MSC_VER)
typedef signed __int64 efs_off_t;
#		else
typedef signed long long efs_off_t;
#		endif
#	endif
#endif

// Definition of the internal efs_stat type
#if defined(HAVE_STRUCT_STAT64)
typedef struct stat64 efs_stat;
#else
#	if defined(HAVE_STRUCT__STAT64)
typedef struct _stat64 efs_stat;
#	else
#		if defined(HAVE_STRUCT___STAT64)
typedef struct __stat64 efs_stat;
#		else
#			error No suitable stat64 struct found
#		endif
#	endif
#endif

class fs_layer
{

public:
	/*
	 * Taken from the BSD file sys/time.h.
	 */
	struct timeval_fs
	{
		long tv_sec;		// seconds
		long tv_usec;		// microseconds
	};

	static int gettimeofday(struct fs_layer::timeval_fs *, void *);


	static boost::filesystem::path stringToFSPath(const std::string &str);
	static std::string readFileToString(const char *fn);
	static bool writeFileFromString(const char *fn, const std::string &str);

	static int fsync(int fd);
	static int fdatasync(int fd);

	static int64_t pread(int fd, void *buf, int64_t count, int64_t offset);
	static int64_t pwrite(int fd, const void *buf, int64_t count, int64_t offset);

	static int read(int fd, void *buf, unsigned int count);
	static int write(int fd, const void *buf, unsigned int count);

	static int truncate(const char *path, int64_t length);
	static int ftruncate(int fd, int64_t length);
	static int statvfs(const char *path, struct statvfs *buf);
	static int utimes(const char *filename, const struct fs_layer::timeval_fs times[2]);
	static int futimes(int fd, const struct fs_layer::timeval_fs times[2]);
	static int utime(const char *filename, struct utimbuf *times);
	static int creat(const char *fn, unsigned short mode);
	static int open(const char *fn, int flags, ...);
	static int close(int fd);
	static int mkdir(const char *fn, int mode);
	static int rename(const char *oldpath, const char *newpath);
	static int unlink(const char *path);
	static int rmdir(const char *path);
	static int stat(const char *path, efs_stat *buffer);
	static inline int lstat(const char *path, efs_stat *buffer) {
		return stat(path, buffer);
	}
	static int chmod (const char*, int);

#if !defined(_WIN32)
	typedef struct ::dirent fs_dirent;
#else
typedef struct dirent
{
        long            d_ino;          /* Always zero. */
        unsigned short  d_namlen;       /* Length of name in d_name. */
        char            d_name[260]; /* TODO   File name. */
} fs_dirent;
#endif

	struct DIR;
	static DIR *opendir(const char *name);
	static int closedir(DIR* dir);
	static fs_dirent* readdir(DIR* dir);

	static std::string concat_path(const std::string &path1,
		const std::string &path2, bool genericPath = false);
	static bool is_same_path(const std::string &path1,
		const std::string &path2);
	static std::string extract_path(const std::string &fullpath);
	static std::string extract_filename(const std::string &fullpath);
	static std::string canonical(const std::string &fullpath);

	static bool capacity(const std::string &rootDir,
		uint64_t &totalCapacity, uint64_t &availableCapacity);
};

#endif
