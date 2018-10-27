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

#ifndef PFM_LAYER_H
#define PFM_LAYER_H

namespace encfs
{
class DirTraverse;
}

#include "config.h"

#include <list>
#include <map>
#include <stdint.h>

#include "FileStatCache.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifndef _INC_WINDOWS
#define _INC_WINDOWS
#endif

// Pismo File Mount
#include "pfmapi.h"

// libencfs
#include "FileUtils.h"
#include "FileNode.h"
#include "Error.h"

// easyloggingpp
#include "easylogging++.h"

#ifdef _WIN32
#define CCALL __cdecl
#else
#define CCALL
#endif

using encfs::RootPtr;

class PFMLayer: public PfmFormatterDispatch
{
public:
	PFMLayer();
	virtual ~PFMLayer();

	void startFS(RootPtr rootFS, const wchar_t *mountDir, PfmApi *pfmApi,
		wchar_t driveLetter, bool useCaching, bool worldWrite, bool localDrive, 
		bool startBrowser, std::ostream &ostr);

	// PfmFormatterDispatch
	void CCALL Open(PfmMarshallerOpenOp* op, void* formatterUse);
	void CCALL Replace(PfmMarshallerReplaceOp* op, void* formatterUse);
	void CCALL Move(PfmMarshallerMoveOp* op, void* formatterUse);
	void CCALL MoveReplace(PfmMarshallerMoveReplaceOp* op, void* formatterUse);
	void CCALL Delete(PfmMarshallerDeleteOp* op, void* formatterUse);
	void CCALL Close(PfmMarshallerCloseOp* op, void* formatterUse);
	void CCALL FlushFile(PfmMarshallerFlushFileOp* op, void* formatterUse);
	void CCALL List(PfmMarshallerListOp* op, void* formatterUse);
	void CCALL ListEnd(PfmMarshallerListEndOp* op, void* formatterUse);
	void CCALL Read(PfmMarshallerReadOp* op, void* formatterUse);
	void CCALL Write(PfmMarshallerWriteOp* op, void* formatterUse);
	void CCALL SetSize(PfmMarshallerSetSizeOp* op, void* formatterUse);
	void CCALL Capacity(PfmMarshallerCapacityOp* op, void* formatterUse);
	void CCALL FlushMedia(PfmMarshallerFlushMediaOp* op, void* formatterUse);
	void CCALL Control(PfmMarshallerControlOp* op, void* formatterUse);
	void CCALL MediaInfo(PfmMarshallerMediaInfoOp* op, void* formatterUse);
	void CCALL Access(PfmMarshallerAccessOp* op, void* formatterUse);
	void CCALL ReadXattr(PfmMarshallerReadXattrOp* op, void* formatterUse);
	void CCALL WriteXattr(PfmMarshallerWriteXattrOp* op, void* formatterUse);

	/**
	 * Class to hold information about a file list operation.
	 */
	class FileList
	{
	public:
		FileList(): listId_(0), hasPreviousResult_(false) { }
		FileList(const FileList &o) { copy(o); }
		virtual ~FileList() { }
		FileList &copy(const FileList &o)
		{
			listId_ = o.listId_;
			pDirT_ = o.pDirT_;
			hasPreviousResult_ = o.hasPreviousResult_;
			prevAttribs_ = o.prevAttribs_;
			prevName_ = o.prevName_;

			return *this;
		}
		FileList & operator=(const FileList & o)
		{
			return copy(o);
		}

		int64_t listId_;
		std::shared_ptr<encfs::DirTraverse> pDirT_;

		bool hasPreviousResult_;
		PfmAttribs prevAttribs_;
		std::string prevName_;
	};

	/**
	 * Class to hold information about open files and directories.
	 */
	class OpenFile
	{
	public:
		OpenFile() : openId_(0), sequenceId_(0), fd_(-1), isFile_(true), fileId_(0),
			isDeleted_(false), isReadOnly_(false), isOpenedReadOnly_(false), fileFlags_(0),
			fileSize_(0), createTime_(0), accessTime_(0), writeTime_(0), changeTime_(0)
		{ }
		OpenFile(const OpenFile &o) { copy(o); }
		virtual ~OpenFile() { }
		OpenFile &copy(const OpenFile &o)
		{
			openId_ = o.openId_;
			sequenceId_ = o.sequenceId_;
			fd_ = o.fd_;
			fileNode_ = o.fileNode_;
			isFile_ = o.isFile_;
			isDeleted_ = o.isDeleted_;
			fileId_ = o.fileId_;
			pathName_ = o.pathName_;
			isReadOnly_ = o.isReadOnly_;
			isOpenedReadOnly_ = o.isOpenedReadOnly_;
			fileFlags_ = o.fileFlags_;
			fileSize_ = o.fileSize_;
			createTime_ = o.createTime_;
			accessTime_ = o.accessTime_;
			writeTime_ = o.writeTime_;
			changeTime_ = o.changeTime_;

			return *this;
		}
		OpenFile & operator=(const OpenFile & o)
		{
			return copy(o);
		}
		int64_t openId_, sequenceId_;
		int fd_;
		bool isFile_;	// File: true, Directory: false
		bool isDeleted_;
		int64_t fileId_;
		std::shared_ptr<encfs::FileNode> fileNode_;	// For files
		bool isReadOnly_;						// File has Read-only bit set
		bool isOpenedReadOnly_;					// File was opened read-only
		PT_UINT8 fileFlags_;
		PT_UINT64 fileSize_;
		PT_INT64 createTime_;
		PT_INT64 accessTime_;
		PT_INT64 writeTime_;
		PT_INT64 changeTime_;
		std::string pathName_;
		std::list<FileList> fileLists_;			// For directories
	};

	int64_t getFileID(const std::string &path);
	int64_t addFileID(const std::string &path);
	bool renameFileID(int64_t fileId, const std::string &newpath);
	bool deleteFileID(int64_t fileId);

	OpenFile *getOpenFile(int64_t openId);
	OpenFile *findOpenFileByName(const std::string &path);
	int64_t createFileId(const std::string &fn);
	static void createEndName(std::wstring &endName, const char *fullPathName);

	int createOp(const std::string &path, int8_t createFileType, uint8_t createFileFlags,
		int64_t writeTime, int64_t newCreateOpenId,PfmOpenAttribs* openAttribs);
	int renameOp(OpenFile *pOpenFile, const std::string &newPath);
	int deleteOp(OpenFile *pOpenFile);
	void openExisting(OpenFile *pOpenFile, PfmOpenAttribs *openAttribs,
		PT_UINT8 accessLevel);
	int openFileOp(std::shared_ptr<encfs::FileNode> fileNode, int fd, PfmOpenAttribs *openAttribs,
		int64_t newExistingOpenId, PT_UINT8 accessLevel, const std::string &path);
	int openDirOp(PfmOpenAttribs *openAttribs, int64_t newExistingOpenId,
		PT_UINT8 accessLevel, const std::string &path);
	int makeOpenFileFlags(PT_INT8 accessLevel);
	int makeOpenFileFlags(bool isReadOnly);

	void reportRLogErr(encfs::Error &err);
	void reportEncFSMPErr(const std::wstring &errStr, const std::string &fn, encfs::Error &err);
	void reportEncFSMPErr(const std::wstring &errStr, const std::string &fn);

	bool isHiddenFile(const std::string &fileName);
	bool isSkippedFile(const std::string &fileName);
	PT_INT8 determineAccessLevel(bool isReadOnly, PT_INT8 requestedAccessLevel);
	void printOpenFiles(const char *msg);
	void addOpenFile(const OpenFile &of);
	bool renameOpenFile(OpenFile *pOpenFile, const std::string &newPath);

private:

	PfmMarshaller* marshaller;

	RootPtr rootFS_;

	typedef std::map< int64_t, OpenFile > OpenFileMapType;
	OpenFileMapType openFiles_;
	typedef std::map< std::string, int64_t > OpenIdMapType;
	OpenIdMapType openIdMap_;
	int64_t newFileID_;

	typedef std::map< std::string, int64_t > FileIDMap;
	FileIDMap fileIDs_;

	FileStatCache fileStatCache_;

	std::wstring mountName_;
};

#endif
