/**
 * Copyright (C) 2014 Roman Hiestand
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

#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include "windows.h"
#endif

#include "fs_layer.h"

#include <errno.h>
#include <stdio.h>

#if defined(HAVE_IO_H)
#include <io.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>

#if defined(HAVE_SYS_UTIME_H)
#include <sys/utime.h>
#elif defined(HAVE_UTIME_H)
#include <utime.h>
#endif

#include <time.h>
#include <stdarg.h>
#include <cctype> 
#include <clocale>

#include <boost/scoped_array.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/locale.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>

#if defined(_WIN32)
#include <share.h>
#endif

/**
 * Convert a UTF-16 string to UTF-8.
 */
static std::string wchar_to_utf8_cstr(const wchar_t *str)
{
	return boost::locale::conv::utf_to_utf<char>(str);
}

/**
 * Convert a UTF-8 string to UTF-16 (C string).
 */
static void utf8_to_wchar_buf(const char *src, wchar_t *res, int maxlen)
{
	std::wstring conv = boost::locale::conv::utf_to_utf<wchar_t>(src);

#if defined(_MSC_VER)
	wcscpy_s(res, maxlen, conv.c_str());
#else
	if(conv.size() < maxlen)
		std::wcscpy(res, conv.c_str());
	else
		*res = L'\0';
#endif
}

/**
 * Convert a UTF-8 string to UTF-16 (C++ string).
 */
static std::wstring utf8_to_wchar(const char *src)
{
	return boost::locale::conv::utf_to_utf<wchar_t>(src);
}

/**
 * Converts a file name in UTF-8 to Windows extended-length path.
 *
 * Inspired by encfs4win by Frediano Ziglio
 */
std::wstring utf8_to_wfn(const std::string& src)
{
	// Replace all forward slashes with backslashes
	std::string path_backslash = boost::replace_all_copy(src, "/", "\\");

	// For paths starting with a drive letter and a colon (e.g. "C:"),
	// add the prefix "\\?\".
	// For paths starting with "\\", add prefix "\\?\UNC" and remove one "\".
	std::wstring path_utf16 = utf8_to_wchar(path_backslash.c_str());

	if(path_utf16.length() < 3)
		return path_utf16;

	// Check whether the first letter is alphanumeric in "C" locale
	std::locale c_loc("C");
	if(std::isalpha(path_utf16[0], c_loc)
		&& path_utf16[1] == L':')
	{
		path_utf16 = std::wstring(L"\\\\?\\") + path_utf16;
	}
	else if(path_utf16[0] == L'\\'
		&& path_utf16[1] == L'\\')
	{
		path_utf16 = std::wstring(L"\\\\?\\UNC") + path_utf16.substr(1);
	}

	return path_utf16;
}

/**
 * Converts a 8-bit file name to boost::filesystem::path.
 *
 * On Windows, the file name is in UTF-8, and gets converted to a
 * extended-length path (prefix "\\?\").
 * On Unix, the file name is in the current codepage (usually UTF-8).
 */
boost::filesystem::path fs_layer::stringToFSPath(const std::string &str)
{
#if defined(_WIN32)
	boost::filesystem::path p(utf8_to_wfn(str));
#else
	boost::filesystem::path p(str);
#endif

	return p;
}

std::string fs_layer::readFileToString(const char *fn)
{
	struct stat stbuf;
	memset( &stbuf, 0, sizeof(struct stat));
	if( fs_layer::lstat( fn, &stbuf ) != 0)
		return std::string();

	int fd = open(fn, O_RDONLY);
	if(fd < 0)
		return std::string();

	std::string buf;
	if(stbuf.st_size > 0)
	{
		buf.resize(stbuf.st_size);
		read(fd, &(buf[0]), static_cast<unsigned int>(stbuf.st_size));
	}
	close(fd);

	return buf;
}

bool fs_layer::writeFileFromString(const char *fn, const std::string &str)
{
	int fd = open(fn, O_RDWR | O_CREAT, 0640);
	if(fd < 0)
		return false;

	if(!str.empty())
	{
		write(fd, &(str[0]), static_cast<unsigned int>(str.size()));
	}

	close(fd);

	return true;
}

int fs_layer::gettimeofday (struct fs_layer::timeval_fs *tv, void *tz)
{
	if (!tv)
	{
		errno = EINVAL;
		return -1;
	}

	// Get current UTC time
	boost::posix_time::ptime now(boost::posix_time::microsec_clock::universal_time());

	// Calculate time between epoch and now
	boost::posix_time::ptime time_t_epoch(boost::gregorian::date(1970,1,1));
	boost::posix_time::time_duration diff = now - time_t_epoch;

	// Convert to microseconds
	boost::posix_time::time_duration::tick_type diff_msec = diff.total_microseconds();

	tv->tv_sec  = static_cast<long>(diff_msec / 1000000UL);
	tv->tv_usec = static_cast<long>(diff_msec % 1000000UL);

	return 0;
}

int fs_layer::close(int fd)
{
#if defined(_WIN32)
	return _close(fd);
#else
	return ::close(fd);
#endif
}

int fs_layer::fsync(int fd)
{
#if defined(_WIN32)
	return _commit(fd);
#else
	return ::fsync(fd);
#endif
}

int fs_layer::fdatasync(int fd)
{
#if defined(_WIN32)
	return _commit(fd);
#else
	return ::fsync(fd);
#endif
}

ssize_t fs_layer::pread(int fd, void *buf, size_t count, int64_t offset)
{
#if defined(_WIN32)
	int64_t oldpos = _telli64(fd);
	_lseeki64(fd, offset, SEEK_SET);
	int ret = _read(fd, buf, count);
	_lseeki64(fd, oldpos, SEEK_SET);
	return ret;
#else
	return ::pread(fd, buf, count, offset);
#endif
}

ssize_t fs_layer::pwrite(int fd, const void *buf, size_t count, int64_t offset)
{
#if defined(_WIN32)
	int64_t oldpos = _telli64(fd);
	_lseeki64(fd, offset, SEEK_SET);
	int ret = _write(fd, buf, count);
	_lseeki64(fd, oldpos, SEEK_SET);
	return ret;
#else
	return ::pwrite(fd, buf, count, offset);
#endif
}

int fs_layer::read(int fd, void *buf, unsigned int count)
{
#if defined(_WIN32)
	return _read(fd, buf, count);
#else
	return ::read(fd, buf, count);
#endif
}

int fs_layer::write(int fd, const void *buf, unsigned int count)
{
#if defined(_WIN32)
	return _write(fd, buf, count);
#else
	return ::write(fd, buf, count);
#endif
}

int fs_layer::ftruncate(int fd, int64_t length)
{
#if defined(_WIN32)
	HANDLE h = (HANDLE)_get_osfhandle(fd);
	if(h == INVALID_HANDLE_VALUE)
	{
		errno = EBADF;
		return -1;
	}
	LARGE_INTEGER oldPos, newPos;
	newPos.QuadPart = length;
	BOOL ret = SetFilePointerEx(h, newPos, &oldPos, FILE_BEGIN);
	if(!ret)
	{
		errno = EINVAL;
		return -1;
	}
	ret = SetEndOfFile(h);
	if(!ret)
	{
		errno = EINVAL;
		return -1;
	}
/*	ret = SetFilePointerEx(h, oldPos, NULL, FILE_BEGIN);
	if(!ret)
	{
		errno = EINVAL;
		return -1;
	}*/
	return 0;
#else
	return ::ftruncate(fd, length);
#endif
}

int fs_layer::truncate(const char *path, int64_t length)
{
	boost::filesystem::path p(stringToFSPath(path));

	boost::system::error_code ec;
	boost::filesystem::resize_file(p, length, ec);

	if(ec)
	{
		errno = ec.value();
		return -1;
	}

	return 0;
}


//#include <sys/utime.h>

int
fs_layer::utimes(const char *filename, const struct fs_layer::timeval_fs times[2])
{
	boost::filesystem::path fn(stringToFSPath(filename));


	std::time_t new_time = times[0].tv_sec;
	boost::system::error_code ec;
	boost::filesystem::last_write_time(fn, new_time, ec);

	if(ec)
	{
		errno = ec.value();
		return -1;
	}

	return 0;
}

int fs_layer::statvfs(const char *path, struct statvfs *fs)
{
#if defined(_WIN32)
	boost::filesystem::path p(utf8_to_wfn(path));
#else
	boost::filesystem::path p(path);
#endif

	boost::system::error_code ec;
	boost::filesystem::space_info spaceInfo = boost::filesystem::space(p, ec);

	assert(false);		// TODO: Implement
/*
unsigned long f_bsize    File system block size. 
unsigned long f_frsize   Fundamental file system block size. 
fsblkcnt_t    f_blocks   Total number of blocks on file system in units of f_frsize. 
fsblkcnt_t    f_bfree    Total number of free blocks. 
fsblkcnt_t    f_bavail   Number of free blocks available to 
                         non-privileged process. 
fsfilcnt_t    f_files    Total number of file serial numbers. 
fsfilcnt_t    f_ffree    Total number of free file serial numbers. 
fsfilcnt_t    f_favail   Number of file serial numbers available to 
                         non-privileged process. 
unsigned long f_fsid     File system ID. 
unsigned long f_flag     Bit mask of f_flag values. 
unsigned long f_namemax  Maximum filename length.*/
	return 0;
}

int fs_layer::creat(const char *fn, unsigned short mode)
{
	int fd = 0;
#if defined(_WIN32)
	boost::filesystem::path fn_path(utf8_to_wfn(fn));
	int flags = _O_CREAT | _O_TRUNC | _O_RDWR | _O_BINARY | _O_SEQUENTIAL;
	int shflag = _SH_DENYRW;
	int pmode =  _S_IREAD | _S_IWRITE;
#	if defined(HAVE__WSOPEN_S)
	errno_t err = _wsopen_s(&fd, fn_path.wstring().c_str(), flags,
		shflag, pmode);
	if(err)
		return err;
#	else
	fd = _wsopen(fn_path.wstring().c_str(), flags, shflag, pmode);
	if(fd < 0)
		return errno;
#	endif

#else
	fd = ::creat(fn, mode);
#endif

	return fd;
}

int fs_layer::open(const char *fn, int flags, ...)
{
	int fd = 0;

	int mode = 0;
	va_list ap;
	va_start(ap, flags);
	if (flags & O_CREAT)
		mode = va_arg(ap, int);
	va_end(ap);


	// Strip off illegal bits in mode
#if defined(_WIN32)
	mode &= (_S_IREAD | _S_IWRITE);
#else
	mode &= (S_IREAD | S_IWRITE);
#endif

#if defined(_WIN32)
	boost::filesystem::path fn_path(utf8_to_wfn(fn));
	flags |= _O_BINARY;
	int shflag = _SH_DENYRW;


#	if defined(HAVE__WSOPEN_S)
	errno_t err = _wsopen_s(&fd, fn_path.wstring().c_str(), flags,
		shflag, mode);
	if(err)
	{
		errno = err;
		return -1;
	}
#	else
	fd = _wsopen(fn_path.wstring().c_str(), flags, shflag, mode);
#	endif
#else
	fd = ::open(fn, flags, mode);
#endif

	return fd;
}

int fs_layer::utime(const char *filename, struct utimbuf *times)
{
	if (!times)
		return fs_layer::utimes(filename, NULL);
	
	struct fs_layer::timeval_fs tm[2];
	tm[0].tv_sec = times->actime;
	tm[0].tv_usec = 0;
	tm[1].tv_sec = times->modtime;
	tm[1].tv_usec = 0;
	return fs_layer::utimes(filename, tm);
}

int fs_layer::mkdir(const char *fn, int mode)
{
	boost::filesystem::path fn_path(stringToFSPath(fn));

	boost::system::error_code ec;
	boost::filesystem::create_directory(fn_path, ec);

	if(ec)
	{
		errno = ec.value();
		return -1;
	}

	boost::filesystem::perms perm;

	// Translate permissions
	if(mode & S_IRUSR)
		perm |= boost::filesystem::owner_read;
	if(mode & S_IWUSR)
		perm |= boost::filesystem::owner_write;
	if(mode & S_IXUSR)
		perm |= boost::filesystem::owner_exe;
	if(mode & S_IRGRP)
		perm |= boost::filesystem::group_read;
	if(mode & S_IWGRP)
		perm |= boost::filesystem::group_write;
	if(mode & S_IXGRP)
		perm |= boost::filesystem::group_exe;
	if(mode & S_IROTH)
		perm |= boost::filesystem::others_read;
	if(mode & S_IWOTH)
		perm |= boost::filesystem::others_write;
	if(mode & S_IXOTH)
		perm |= boost::filesystem::others_exe;
	if(mode & S_ISUID)
		perm |= boost::filesystem::set_uid_on_exe;
	if(mode & S_ISGID)
		perm |= boost::filesystem::set_gid_on_exe;

	boost::filesystem::permissions(fn_path, perm, ec);
	if(ec)
	{
		errno = ec.value();
		return -1;
	}

	return 0;

	return 0;
}

int fs_layer::rename(const char *oldpath, const char *newpath)
{
	boost::filesystem::path oldfn_path(stringToFSPath(oldpath));
	boost::filesystem::path newfn_path(stringToFSPath(newpath));

	boost::system::error_code ec;
	boost::filesystem::rename(oldfn_path, newfn_path, ec);

	if(ec)
	{
		errno = ec.value();
		return -1;
	}
	return 0;
}

int  fs_layer::unlink(const char *fn)
{
	boost::filesystem::path fn_path(stringToFSPath(fn));
	
	boost::system::error_code ec;

	// Check if fn_path is a file
	boost::filesystem::file_status status = boost::filesystem::status(fn_path, ec);
	if(ec)
	{
		errno = ec.value();
		return -1;
	}
	if(!boost::filesystem::is_regular_file(status)
		&& !boost::filesystem::is_symlink(status))
	{
		errno = EPERM;
		return -1;
	}

	boost::filesystem::remove(fn_path, ec);

	if(ec)
	{
		errno = ec.value();
		return -1;
	}
	return 0;
}

int fs_layer::rmdir(const char *fn)
{
	boost::filesystem::path fn_path(stringToFSPath(fn));

	boost::system::error_code ec;

	// Check if fn_path is a directory
	boost::filesystem::file_status status = boost::filesystem::status(fn_path, ec);
	if(ec)
	{
		errno = ec.value();
		return -1;
	}
	if(!boost::filesystem::is_directory(status))
	{
		errno = ENOTDIR;
		return -1;
	}

	boost::filesystem::remove(fn_path, ec);

	if(ec)
	{
		errno = ec.value();
		return -1;
	}
	return 0;
}

int fs_layer::stat(const char *fn, struct stat *buf)
{
	boost::filesystem::path fn_path(stringToFSPath(fn));

	boost::system::error_code ec;
	boost::filesystem::file_status status = boost::filesystem::status(fn_path, ec);
	if(ec)
	{
		errno = ec.value();
		return -1;
	}
	std::time_t lwt = boost::filesystem::last_write_time(fn_path, ec);
	if(ec)
	{
		errno = ec.value();
		return -1;
	}
	uintmax_t fsize = 0;
	if(boost::filesystem::is_regular_file(status))
		fsize = boost::filesystem::file_size(fn_path, ec);
	if(ec)
	{
		errno = ec.value();
		return -1;
	}
	uintmax_t hlc = boost::filesystem::hard_link_count(fn_path, ec);
	if(ec)
	{
		errno = ec.value();
		return -1;
	}

	unsigned short mode = 0;
	// File type
	if(boost::filesystem::is_regular_file(status))
		mode |= S_IFREG;
	if(boost::filesystem::is_directory(status))
		mode |= S_IFDIR;
	if(boost::filesystem::is_symlink(status))
		mode |= S_IFLNK;
	if(boost::filesystem::is_other(status))
		mode |= S_IFCHR;	// ???

	// Permissions
	if(status.permissions() & boost::filesystem::owner_read)
		mode |= S_IRUSR;
	if(status.permissions() & boost::filesystem::owner_write)
		mode |= S_IWUSR;
	if(status.permissions() & boost::filesystem::owner_exe)
		mode |= S_IXUSR;
	if(status.permissions() & boost::filesystem::group_read)
		mode |= S_IRGRP;
	if(status.permissions() & boost::filesystem::group_write)
		mode |= S_IWGRP;
	if(status.permissions() & boost::filesystem::group_exe)
		mode |= S_IXGRP;
	if(status.permissions() & boost::filesystem::others_read)
		mode |= S_IROTH;
	if(status.permissions() & boost::filesystem::others_write)
		mode |= S_IWOTH;
	if(status.permissions() & boost::filesystem::others_exe)
		mode |= S_IXOTH;

#if defined(_WIN32)
	// Ignore the read-only flag for folders on Windows
	// See for example http://support.microsoft.com/kb/326549
	if(boost::filesystem::is_directory(status))
		mode |= (S_IWUSR | S_IWGRP | S_IWOTH);
#endif

	if(status.permissions() & boost::filesystem::set_uid_on_exe)
		mode |= S_ISUID;
	if(status.permissions() & boost::filesystem::set_gid_on_exe)
		mode |= S_ISGID;

	buf->st_dev = buf->st_rdev = 0;
	buf->st_ino = 0;
	buf->st_mode = mode;
	buf->st_nlink = static_cast<short>(hlc);
	buf->st_uid = 0;
	buf->st_gid = 0;
	buf->st_size = fsize;
	buf->st_atime = lwt;
	buf->st_mtime = lwt;
	buf->st_ctime = lwt;

	return 0;
}

int fs_layer::chmod(const char* fn, int mode)
{
	boost::filesystem::path fn_path(stringToFSPath(fn));

	boost::system::error_code ec;
	boost::filesystem::perms perm = boost::filesystem::no_perms;

	// Translate permissions
	if(mode & S_IRUSR)
		perm |= boost::filesystem::owner_read;
	if(mode & S_IWUSR)
		perm |= boost::filesystem::owner_write;
	if(mode & S_IXUSR)
		perm |= boost::filesystem::owner_exe;
	if(mode & S_IRGRP)
		perm |= boost::filesystem::group_read;
	if(mode & S_IWGRP)
		perm |= boost::filesystem::group_write;
	if(mode & S_IXGRP)
		perm |= boost::filesystem::group_exe;
	if(mode & S_IROTH)
		perm |= boost::filesystem::others_read;
	if(mode & S_IWOTH)
		perm |= boost::filesystem::others_write;
	if(mode & S_IXOTH)
		perm |= boost::filesystem::others_exe;
	if(mode & S_ISUID)
		perm |= boost::filesystem::set_uid_on_exe;
	if(mode & S_ISGID)
		perm |= boost::filesystem::set_gid_on_exe;

	boost::filesystem::permissions(fn_path, perm, ec);
	if(ec)
	{
		errno = ec.value();
		return -1;
	}

	return 0;
}

struct fs_layer::DIR
{
	boost::filesystem::directory_iterator *iter;
	fs_layer::fs_dirent ent;
};

fs_layer::DIR* fs_layer::opendir(const char *name)
{
	boost::system::error_code ec;

	// Convert name to boost path
	boost::filesystem::path path(stringToFSPath(name));

	fs_layer::DIR *dir = new fs_layer::DIR;
	dir->iter = new boost::filesystem::directory_iterator(path, ec);

	if(ec)
	{
		errno = ec.value();

		// Some error occurred
		delete dir->iter;
		delete dir;
		return NULL;
	}

	return dir;
}

int fs_layer::closedir(fs_layer::DIR* dir)
{
	delete dir->iter;
	delete dir;
	return 0;
}

fs_layer::fs_dirent* fs_layer::readdir(fs_layer::DIR* dir)
{
	if(dir == NULL
		|| dir->iter == NULL)
		return NULL;

	// Check for no more files
	if(*(dir->iter) == boost::filesystem::directory_iterator())
		return NULL;

	const boost::filesystem::directory_entry &dirEntry = *(*dir->iter);
#if defined(_WIN32)
	std::string path = wchar_to_utf8_cstr(dirEntry.path().filename().wstring().c_str());
#else
	std::string path(dirEntry.path().filename().string());
#endif
	strncpy(dir->ent.d_name, path.c_str(), sizeof(dir->ent.d_name));
	dir->ent.d_name[sizeof(dir->ent.d_name)-1] = 0;
#if defined(_WIN32)
	dir->ent.d_namlen = strlen(dir->ent.d_name);
#endif
	(*(dir->iter))++;

	return &dir->ent;
}

/**
 * Concatenates two paths, adding separator if necessary.
 *
 * path1 and path2 are in UTF-8
 * genericPath: If true, adds '/'. If false, adds native separators.
 */
std::string fs_layer::concat_path(const std::string &path1,
	const std::string &path2, bool genericPath)
{
	// Convert name to boost path
#if defined(_WIN32)
	boost::filesystem::path ppath1(utf8_to_wchar(path1.c_str()));
	boost::filesystem::path ppath2(utf8_to_wchar(path2.c_str()));
#else
	boost::filesystem::path ppath1(path1);
	boost::filesystem::path ppath2(path2);
#endif

	ppath1 /= ppath2;	// Append with separator, if necessary

	std::string concat_path;
	
	if(genericPath)
		concat_path = wchar_to_utf8_cstr(ppath1.generic_wstring().c_str());
	else
		concat_path = wchar_to_utf8_cstr(ppath1.wstring().c_str());

	return concat_path;
}

bool fs_layer::is_same_path(const std::string &path1,
	const std::string &path2)
{
	boost::filesystem::path ppath1(stringToFSPath(path1));
	boost::filesystem::path ppath2(stringToFSPath(path2));

	return (ppath1.compare(ppath2) == 0);
}

std::string fs_layer::extract_path(const std::string &fullpath)
{
#if defined(_WIN32)
	boost::filesystem::path ppath(utf8_to_wchar(fullpath.c_str()));
	std::wstring fn( ppath.parent_path().c_str() );
	return wchar_to_utf8_cstr(fn.c_str());
#else
	boost::filesystem::path ppath(fullpath);
	return ppath.parent_path().string();
#endif
}

std::string fs_layer::extract_filename(const std::string &fullpath)
{
#if defined(_WIN32)
	boost::filesystem::path ppath(utf8_to_wchar(fullpath.c_str()));
	std::wstring fn( ppath.filename().c_str() );
	if(boost::algorithm::ends_with(fn, L"/"))
		fn.resize( fn.size() - 1 );
	return wchar_to_utf8_cstr(fn.c_str());
#else
	boost::filesystem::path ppath(fullpath);
	return ppath.filename().string();
#endif
}

std::string fs_layer::canonical(const std::string &fullpath)
{
	boost::filesystem::path ppath(stringToFSPath(fullpath));

	boost::system::error_code ec;
	boost::filesystem::path canonicalPath = boost::filesystem::canonical(ppath, ec);

	if(ec)
		return std::string();

#if defined(_WIN32)
	std::string pathOut = wchar_to_utf8_cstr(canonicalPath.filename().wstring().c_str());
#else
	std::string pathOut(canonicalPath.filename().string());
#endif

	return pathOut;
}

bool fs_layer::capacity(const std::string &rootDir,
	uint64_t &totalCapacity, uint64_t &availableCapacity)
{
	boost::filesystem::path p(stringToFSPath(rootDir));

	boost::system::error_code ec;
	boost::filesystem::space_info spaceInfo = boost::filesystem::space(p, ec);
	if(ec)
		return false;

	totalCapacity = spaceInfo.capacity;
	availableCapacity = spaceInfo.available;

	return true;
}
