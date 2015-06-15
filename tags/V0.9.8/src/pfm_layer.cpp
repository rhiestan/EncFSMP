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

#include "portability.cpp"

#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include <fcntl.h>

#include "efs_config.h"
#include "pfm_layer.h"
#include "EncFSMPStrings.h"
#include "EncFSMPLogger.h"

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/functional/hash.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

// libencfs
#include "Cipher.h"
#include "DirNode.h"
#include "MemoryPool.h"
#include "Interface.h"
#include "FileUtils.h"
#include "StreamNameIO.h"
#include "BlockNameIO.h"
#include "NullNameIO.h"

#include "rlog/rlog.h"

//#define PFM_ACCESS_LEVEL_WORKAROUND 1

PFMLayer::PFMLayer() :
	marshaller(NULL),
	newFileID_(1)
{
}

PFMLayer::~PFMLayer()
{
	// Close openFiles_
	std::list< OpenFile >::iterator iter = openFiles_.begin();
	while(iter != openFiles_.end())
	{
		OpenFile &cur = *iter;

		// Save some attributes for later.
		// Reason: We need the file to be closed in order to delete it.
		// To close it, we have to delete the OpenFile instance.
		bool isDeleted = cur.isDeleted_;
		bool isFile = cur.isFile_;
		std::string pathName = cur.pathName_;

		// The file is closed. Remove from the list of open files
		openFiles_.erase(iter);
		// Don't access cur after this point

		if(isDeleted)
		{
			try
			{
				if(isFile)
				{
					rootFS_->root->unlink(pathName.c_str());
				}
				else
				{
					std::string cipherPathName = rootFS_->root->cipherPath(pathName.c_str());
					fs_layer::rmdir(cipherPathName.c_str());
				}
			}
			catch( rlog::Error &err )
			{
				reportRLogErr(err);
			}
		}

		iter = openFiles_.begin();
	}

}

void PFMLayer::startFS(RootPtr rootFS, const wchar_t *mountDir, PfmApi *pfmApi,
	wchar_t driveLetter, bool worldWrite, bool localDrive,
	bool startBrowser, std::ostream &ostr)
{
	rootFS_ = rootFS;
	mountName_ = mountDir;

	// The following code is copied mostly from the Pismo File Mount's example code tempfs.cpp
	int error = 0;
	PfmMount* mount = 0;
	FD_T toFormatterRead = FD_INVALID;
	FD_T fromFormatterWrite = FD_INVALID;
	PfmMountCreateParams mcp;

	PfmMountCreateParams_Init(&mcp);
	mcp.mountSourceName = mountDir;
	mcp.mountFlags |= pfmMountFlagUncOnly | pfmMountFlagUnmountOnRelease;
	if(localDrive)
		mcp.mountFlags |= pfmMountFlagLocalDriveType;
	if(startBrowser)
		mcp.mountFlags |= pfmMountFlagBrowse;
	if(worldWrite)
		mcp.mountFlags |= (pfmMountFlagWorldRead | pfmMountFlagWorldWrite);

	if(driveLetter != L'-')
		mcp.driveLetter = driveLetter;

	error = PfmMarshallerFactory(&marshaller);
	if(error)
	{
		ostr << "ERROR: " << error << " Unable to create marshaller" << std::endl;
		return;
	}

	error = create_pipe(&toFormatterRead,&mcp.toFormatterWrite);
	if(!error)
	{
		error = create_pipe(&mcp.fromFormatterRead,&fromFormatterWrite);
	}
	if(error)
	{
		ostr << "ERROR: " << error << " Unable to create pipes" << std::endl;
		return;
	}

	error = pfmApi->MountCreate(&mcp,&mount);
	if(error)
	{
		ostr << "ERROR: " << error << " Unable to create mount" << std::endl;
		return;
	}

	// Close driver end pipe handles now. Driver has duplicated what
	// it needs. If these handles are not closed then pipes will not
	// break if driver disconnects, leaving us stuck in the
	// marshaller.
	close_fd(mcp.toFormatterWrite);
	close_fd(mcp.fromFormatterRead);


	// If tracemon is installed and running then diagnostic
	// messsages can be viewed in the "encFSMP" channel.
	marshaller->SetTrace(EncFSMPStrings::formatterName_.c_str());
	// Also send diagnostic messages to stdout. This can slow
	// things down quite a bit.
//	marshaller->SetStatus(stdout_fd());

	// The marshaller serve function will return at unmount or
	// if driver disconnects.
	int volumeFlags = pfmVolumeFlagCaseSensitive;	//pfmVolumeFlagEncrypted | pfmVolumeFlagCaseSensitive;
	int retVal = marshaller->ServeReadWrite(this, volumeFlags, EncFSMPStrings::formatterName8_.c_str(), toFormatterRead, fromFormatterWrite);

#if defined(_WIN32)
	if(retVal > 0 && retVal != 109)
	{
		char *err;
		if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			retVal,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
			(LPSTR) &err,
			0,
			NULL))
		{
			ostr << "Error: " << retVal << ": " << err;
			LocalFree(err);
		}
	}
#endif

	if(mount)
		mount->Release();

	if(marshaller)
	{
		marshaller->Release();
		marshaller = 0;
	}

	close_fd(toFormatterRead);
	close_fd(fromFormatterWrite);
}


void CCALL PFMLayer::ReleaseName(wchar_t* name)
{
	delete [] name;
}


int64_t UnixTimeToFileTime(time_t t)
{
	return (t * 10000000L) + 116444736000000000L;
}

long FileTimeToUnixTime(int64_t t)
{
	return static_cast<long>((t - 116444736000000000L) / 10000000L);
}

int/*error*/ CCALL PFMLayer::Open(const PfmNamePart* nameParts, size_t namePartCount,
	int8_t createFileType, uint8_t createFileFlags, int64_t writeTime,
	int64_t newCreateOpenId, int8_t existingAccessLevel, int64_t newExistingOpenId,
	uint8_t/*bool*/ * existed, PfmOpenAttribs* openAttribs, int64_t* parentFileId,
	wchar_t** endName)
{
	if(!rootFS_)
		return pfmErrorFailed;

	// Construct name
	std::string path("/"), parentFolder;		// In UTF-8 encoding
	for(size_t i = 0; i < namePartCount; i++)
	{
		parentFolder = path;

		path = fs_layer::concat_path(path, nameParts[i].name8, true);
	}

	// Check whether parent folder exists
	// First, look in open files
	bool parentFolderFound = false;
	OpenFile *pParentOpenFile = findOpenFileByName(parentFolder);
	if(pParentOpenFile != NULL
		&& !pParentOpenFile->isFile_)
	{
		parentFolderFound = true;
		if(parentFileId != NULL)
			(*parentFileId) = pParentOpenFile->fileId_;
	}

	if(!parentFolderFound)
	{
		// Not found in open files, look on file system
		std::string cipherPath = rootFS_->root->cipherPath(parentFolder.c_str());
		efs_stat stat;
		int ret = fs_layer::stat(cipherPath.c_str(), &stat);
		if(ret != 0)
			return pfmErrorParentNotFound;
		if((stat.st_mode & S_IFDIR) == 0)	// Check whether it is a directory
			return pfmErrorParentNotFound;

		if(parentFileId != NULL)
			(*parentFileId) = createFileId(parentFolder);

	}

	int retVal = pfmErrorNotFound;

	// Check whether file is opened already
	OpenFile *pOpenFile = findOpenFileByName(path);
	if(pOpenFile != NULL)
	{
		if(pOpenFile->isDeleted_)
		{
			// Found the file, but it is marked as deleted

			// From doc: If the indicated file does not exist and the newCreateOpenId parameter is zero then the formatter should return pfmErrorNotFound
			if(newCreateOpenId == 0
				|| createFileType == pfmFileTypeNone)
				return pfmErrorNotFound;

			// We can't create a file with the same name as a deleted file
			return pfmErrorDeleted;
		}
		else
		{
			openExisting(pOpenFile, openAttribs, existingAccessLevel);

			createEndName(endName, pOpenFile->pathName_.c_str());
			*existed = true;

			return 0;
		}
	}

	// Check whether path is a directory or a file
	bool isDir = false;
	{
		std::string cipherPath = rootFS_->root->cipherPath(path.c_str());
		efs_stat stat;
		int ret = fs_layer::stat(cipherPath.c_str(), &stat);
		if(ret == 0
			&& (stat.st_mode & S_IFDIR))	// Check whether it is a directory
			isDir = true;
	}

	// Check whether file exists
	if(!isDir)
	{
		int res = 0;

		try
		{
			boost::shared_ptr<FileNode> fileNode = 
				rootFS_->root->openNode( path.c_str(), "open", makeOpenFileFlags(existingAccessLevel), &res );

			if(fileNode)
			{
				retVal = openFileOp(fileNode, res, openAttribs, newExistingOpenId, existingAccessLevel, path);
				if(retVal != 0)
					return retVal;

				createEndName(endName, path.c_str());
				*existed = true;
			}
			else
			{
				// File or directory did not yet exist
				*existed = false;

				// From doc: If the indicated file does not exist and the newCreateOpenId parameter is zero then the formatter should return pfmErrorNotFound
				if(newCreateOpenId == 0
					|| createFileType == pfmFileTypeNone)
				{
					res = std::abs(res);
					if(errno == EACCES)
						return pfmErrorAccessDenied;
					else if(errno == EISDIR)
						return pfmErrorNotAFile;
					else if(errno == ENOENT)
						return pfmErrorNotFound;
					else return pfmErrorInvalid;
				}

				// Create new file
				retVal = createOp(path, createFileType, createFileFlags, writeTime, newCreateOpenId, openAttribs);
				if(retVal != 0)
					return retVal;

				createEndName(endName, path.c_str());
			}
		}
		catch( rlog::Error &err )
		{
			reportEncFSMPErr(L"Open failed", path, err);
		}
	}
	else
	{
		// Directory exists, "open" it
		retVal = openDirOp(openAttribs, newExistingOpenId, existingAccessLevel, path);
		if(retVal != 0)
			return retVal;

		createEndName(endName, path.c_str());
		*existed = true;
	}

	return retVal;
}

int/*error*/ CCALL PFMLayer::Replace(int64_t targetOpenId,int64_t targetParentFileId,
	const PfmNamePart* targetEndName, uint8_t createFileFlags,
	int64_t writeTime, int64_t newCreateOpenId, PfmOpenAttribs* openAttribs)
{
	OpenFile *pOpenFile = getOpenFile(targetOpenId);
	if(pOpenFile == NULL)
		return pfmErrorFailed;

	// We do not support Replace for folders
	if(!pOpenFile->isFile_)
		return pfmErrorInvalid;

	if(pOpenFile->isDeleted_)
		return pfmErrorDeleted;

	// The documentation of PFM states that the new file should be created with the same name
	// as the existing one, then the old file should be deleted. Deletion under PFM means
	// that only the directory entry is deleted, but not the file itself. The file is physically
	// deleted only when it is closed.
	// Since here deletion is implemented by only setting the flag isDeleted, this would
	// mean that two files have the same name until the old one gets deleted. This obviously doesn't
	// work.
	// Workaround: Rename the old file, mark it as deleted, then create the new file

	std::string oldPath = pOpenFile->pathName_;

	int retVal = deleteOp(pOpenFile);

	// Create new file
	int createFileType = (pOpenFile->isFile_ ? pfmFileTypeFile : pfmFileTypeFolder);
	retVal = createOp(oldPath, createFileType, createFileFlags, writeTime, newCreateOpenId, openAttribs);

	return retVal;
}

int/*error*/ CCALL PFMLayer::Move(int64_t sourceOpenId, int64_t sourceParentFileId,
	const PfmNamePart* sourceEndName,
	const PfmNamePart* targetNameParts, size_t targetNamePartCount,
	uint8_t/*bool*/ deleteSource, int64_t writeTime, int8_t existingAccessLevel,
	int64_t newExistingOpenId, uint8_t/*bool*/ * existed, PfmOpenAttribs* openAttribs,
	int64_t* parentFileId, wchar_t** endName)
{
	OpenFile *pOpenFile = getOpenFile(sourceOpenId);
	if(pOpenFile == NULL)
		return pfmErrorFailed;

	// if pOpenFile is root, fail
	if(fs_layer::is_same_path(pOpenFile->pathName_, "")
		|| fs_layer::is_same_path(pOpenFile->pathName_, "/"))
		return pfmErrorAccessDenied;

	// Construct name
	std::string path("/"), parentFolder;		// In UTF-8 encoding
	for(size_t i = 0; i < targetNamePartCount; i++)
	{
		parentFolder = path;

		path = fs_layer::concat_path(path, targetNameParts[i].name8, true);
	}

	// Check whether parent folder exists
	// First, look in open files
	bool parentFolderFound = false;
	OpenFile *pParentOpenFile = findOpenFileByName(parentFolder);
	if(pParentOpenFile != NULL
		&& !pParentOpenFile->isFile_)
	{
		parentFolderFound = true;
		if(parentFileId != NULL)
			(*parentFileId) = pParentOpenFile->fileId_;
	}

	if(!parentFolderFound)
	{
		try
		{
			// Not found in open files, look on file system
			DirTraverse dirTParent = rootFS_->root->openDir(parentFolder.c_str());
			// If not, return pfmErrorParentNotFound
			if(!dirTParent.valid())
				return pfmErrorParentNotFound;
		}
		catch( rlog::Error &err )
		{
			// Not an error
			return pfmErrorParentNotFound;
		}

		if(parentFileId != NULL)
			(*parentFileId) = createFileId(parentFolder);
	}

	if(deleteSource == 0
		&& !pOpenFile->isDeleted_)
	{
		// Request to make a hard link: We do not support links
		return pfmErrorInvalid;
	}

	if(pOpenFile->isDeleted_)
	{
		// Undelete
		pOpenFile->isDeleted_ = false;

		// TODO: Check whether target file already exists

		// Apply name change
		return renameOp(pOpenFile, path);
	}

	int retVal = 0;

	// === Check whether target file already exists

	// Check whether target file is opened already
	OpenFile *pOpenFileTarget = findOpenFileByName(path);
	if(pOpenFileTarget != NULL)
	{
		if(pOpenFileTarget->isDeleted_)
		{
			// Found the file, but it is marked as deleted
			// This is (should be) extremely improbable, but should lead to an error.
			return pfmErrorInvalid;
		}
		else
		{
			openExisting(pOpenFileTarget, openAttribs, existingAccessLevel);

			createEndName(endName, path.c_str());

			*existed = true;

			return 0;
		}
	}

	// Check whether (target) path is a directory or a file
	bool isDir = false;
	{
		try
		{
			DirTraverse dirT = rootFS_->root->openDir(path.c_str());
			if(dirT.valid())
				isDir = true;
		}
		catch( rlog::Error &err )
		{
			// Not an error
		}
	}

	if(!isDir)
	{
		try
		{
			int res = 0;
			boost::shared_ptr<FileNode> fileNode = 
				rootFS_->root->openNode( path.c_str(), "open", makeOpenFileFlags(existingAccessLevel), &res );

			if(fileNode)
			{
				// Target file already exists. Perform open
				retVal = openFileOp(fileNode, res, openAttribs, newExistingOpenId, existingAccessLevel, path);
				if(retVal != 0)
					return retVal;

				createEndName(endName, path.c_str());
				*existed = true;
			}
			else
			{
				// File or directory did not yet exist
				*existed = false;

				int retVal = renameOp(pOpenFile, path);
				if(retVal != 0)
					return retVal;

				openExisting(pOpenFile, openAttribs, pfmAccessLevelWriteData);

				createEndName(endName, path.c_str());
				retVal = 0;
			}
		}
		catch( rlog::Error &err )
		{
			reportEncFSMPErr(L"File not found in move operation", path, err);
			retVal = pfmErrorFailed;
		}
	}
	else
	{
		// Directory exists, "open" it
		retVal = openDirOp(openAttribs, newExistingOpenId, existingAccessLevel, path);
		if(retVal != 0)
			return retVal;

		createEndName(endName, path.c_str());
		*existed = true;
	}

	return retVal;
}

int/*error*/ CCALL PFMLayer::MoveReplace(int64_t sourceOpenId, int64_t sourceParentFileId,
	const PfmNamePart* sourceEndName, int64_t targetOpenId,
	int64_t targetParentFileId, const PfmNamePart* targetEndName,
	uint8_t/*bool*/ deleteSource, int64_t writeTime)
{
	OpenFile *pSourceOpenFile = getOpenFile(sourceOpenId);
	if(pSourceOpenFile == NULL)
		return pfmErrorFailed;

	OpenFile *pTargetOpenFile = getOpenFile(targetOpenId);
	if(pTargetOpenFile == NULL)
		return pfmErrorFailed;

	if(pSourceOpenFile == pTargetOpenFile)
		return pfmErrorInvalid;

	// if pTargetOpenFile is root, fail
	if(fs_layer::is_same_path(pTargetOpenFile->pathName_, "")
		|| fs_layer::is_same_path(pTargetOpenFile->pathName_, "/"))
		return pfmErrorAccessDenied;

	if(pTargetOpenFile->isDeleted_)
		return pfmErrorDeleted;

	if(deleteSource == 0
		&& !pSourceOpenFile->isDeleted_)
	{
		// Request to make a hard link: We do not support links
		return pfmErrorInvalid;
	}

	std::string newPath = pTargetOpenFile->pathName_;

	// Delete target
	int retVal = deleteOp(pTargetOpenFile);
	if(retVal != 0)
		return retVal;

	// Rename source to target
	return renameOp(pSourceOpenFile, newPath);
}

int/*error*/ CCALL PFMLayer::Delete(int64_t openId,int64_t parentFileId,const PfmNamePart* endName,int64_t writeTime)
{
	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		return pfmErrorFailed;

	// if pOpenFile is root, fail
	if(fs_layer::is_same_path(pOpenFile->pathName_, "")
		|| fs_layer::is_same_path(pOpenFile->pathName_, "/"))
		return pfmErrorAccessDenied;

	if(pOpenFile->isDeleted_)
		return pfmErrorDeleted;

	return deleteOp(pOpenFile);
}

int/*error*/ CCALL PFMLayer::Close(int64_t openId,int64_t openSequence)
{
	std::list< OpenFile >::iterator iter = openFiles_.begin();
	while(iter != openFiles_.end())
	{
		OpenFile &cur = *iter;

		if(cur.openId_ == openId)
		{
			if(openSequence >= cur.sequenceId_)
			{
				// Save some attributes for later.
				// Reason: We need the file to be closed in order to delete it.
				// To close it, we have to delete the OpenFile instance.
				bool isDeleted = cur.isDeleted_;
				bool isFile = cur.isFile_;
				std::string pathName = cur.pathName_;

				// The file is closed. Remove from the list of open files
				openFiles_.erase(iter);
				// Don't access cur after this point

				if(isDeleted)
				{
					try
					{
						if(isFile)
						{
							std::cout << "Unlink: " << pathName.c_str() << std::endl;
							int retVal = rootFS_->root->unlink(pathName.c_str());
							std::cout << "retVal: " << retVal << std::endl;
						}
						else
						{
							std::string cipherPathName = rootFS_->root->cipherPath(pathName.c_str());
							if(fs_layer::rmdir(cipherPathName.c_str()) < 0)
							{
								fs_layer::chmod(cipherPathName.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
								fs_layer::rmdir(cipherPathName.c_str());
							}
						}
					}
					catch( rlog::Error &err )
					{
						reportEncFSMPErr(L"File deletion failed", pathName, err);
					}
				}

			}
			return 0;
		}
		iter++;
	}
	return pfmErrorFailed;
}

int/*error*/ CCALL PFMLayer::FlushFile(int64_t openId,uint8_t flushFlags,uint8_t fileFlags,uint8_t color,int64_t createTime,int64_t accessTime,int64_t writeTime,int64_t changeTime,PfmOpenAttribs* openAttribs)
{
	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		return pfmErrorFailed;

	if(pOpenFile->isFile_)
	{
		const char *cipherName = pOpenFile->fileNode_->cipherName();
		if(fileFlags != pfmFileFlagsInvalid)
		{
			pOpenFile->fileFlags_ = fileFlags;

			// Change read only flag
			efs_stat buf;
			if(fs_layer::stat(cipherName, &buf) < 0)
				return pfmErrorFailed;
			int mode = buf.st_mode;

			if((fileFlags & pfmFileFlagReadOnly) != 0)
			{
				pOpenFile->isReadOnly_ = true;
				mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
			}
			else
			{
				pOpenFile->isReadOnly_ = false;
				mode |= (S_IWUSR | S_IWGRP | S_IWOTH);
			}
			if(fs_layer::chmod(cipherName, mode) < 0)
				return pfmErrorFailed;
		}

		if(writeTime != pfmTimeInvalid)
		{
			// Apply writeTime
			struct fs_layer::timeval_fs tm[2];
			tm[0].tv_sec = FileTimeToUnixTime(writeTime);
			tm[0].tv_usec = 0;
			tm[1].tv_sec = FileTimeToUnixTime(writeTime);
			tm[1].tv_usec = 0;
			if(pOpenFile->fd_ < 0)
				fs_layer::utimes(cipherName, tm);
			else
				fs_layer::futimes(pOpenFile->fd_, tm);

			pOpenFile->accessTime_ = writeTime;
			pOpenFile->changeTime_ = writeTime;
			pOpenFile->writeTime_ = writeTime;
			pOpenFile->createTime_ = writeTime;
		}
	}

	if(flushFlags & pfmFlushFlagOpen)
		openExisting(pOpenFile, openAttribs, pfmAccessLevelWriteData);

	return 0;
}

int/*error*/ CCALL PFMLayer::List(int64_t openId,int64_t listId,PfmMarshallerListResult* listResult)
{
	if(!rootFS_)
		return pfmErrorFailed;
	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		return pfmErrorFailed;
	if(pOpenFile->isFile_)
		return pfmErrorNotAFolder;
	if(pOpenFile->isDeleted_)
		return pfmErrorDeleted;

	// Check whether listId already exists
	FileList *pFileList = NULL;
	std::list<FileList>::iterator iter = pOpenFile->fileLists_.begin();
	while(iter != pOpenFile->fileLists_.end())
	{
		FileList &cur = *iter;
		if(cur.listId_ == listId)
		{
			pFileList = &cur;
		}
		iter++;
	}

	FileList fl;
	if(pFileList == NULL)
	{
		try
		{
			// Create new list
			DirTraverse dirT = rootFS_->root->openDir(pOpenFile->pathName_.c_str());
			if(!dirT.valid())
				return pfmErrorFailed;

			fl.listId_ = listId;
			boost::shared_ptr<DirTraverse> pDirTTemp ( new DirTraverse(dirT) );
			fl.pDirT_ = pDirTTemp;
			pOpenFile->fileLists_.push_back(fl);
			pFileList = &fl;
		}
		catch( rlog::Error &err )
		{
			reportEncFSMPErr(L"Could not open directory", pOpenFile->pathName_, err);
			return pfmErrorFailed;
		}
	}

	bool doCont = true;
	while(doCont)
	{
		int fileType = 0;
		try
		{
			std::string name = pFileList->pDirT_->nextPlaintextName(&fileType);
			if(name.empty())
			{
				listResult->NoMore();
				doCont = false;
			}
			else
			{
				if(name != "." && name != "..")
				{
					std::string plainPath = fs_layer::concat_path(pOpenFile->pathName_, name, true);
					std::string cpath = rootFS_->root->cipherPath(plainPath.c_str());

					// Check whether a file with name plainPath is opened (and deleted)
					bool isDeleted = false;
					OpenFile *pOpenFile = findOpenFileByName(plainPath);
					if(pOpenFile != NULL && pOpenFile->isDeleted_)
						isDeleted = true;

					if(!isDeleted		// Skip deleted, but not yet closed files
						&& !isSkippedFile(plainPath))
					{
						efs_stat buf;

						if( !fs_layer::lstat( cpath.c_str(), &buf ))
						{
							uint8_t needMore = 1;
							PfmAttribs attribs;
							bool skipThisFile = false;

							if(S_ISREG(buf.st_mode))
								attribs.fileType = pfmFileTypeFile;
							else if(S_ISDIR(buf.st_mode))
								attribs.fileType = pfmFileTypeFolder;
							else
								attribs.fileType = pfmFileTypeNone;
							attribs.fileFlags = 0;
							if((buf.st_mode & S_IWUSR) == 0)
								attribs.fileFlags |= pfmFileFlagReadOnly;
							if(isHiddenFile(plainPath))
								attribs.fileFlags |= pfmFileFlagHidden;
#if defined(EFS_MACOSX)
							attribs.fileFlags |= pfmFileFlagArchive;	// Inverted logic of archive flag on OS X
#endif
							attribs.fileId = createFileId(plainPath);
							attribs.fileSize = buf.st_size;
							attribs.resourceSize = 0;
							attribs.color = 0;
							attribs.extraFlags = 0;

							if(attribs.fileType == pfmFileTypeFile)
							{
								// Determine file size for unencrypted file
								efs_stat buf_ue;
								bool getAttrSuccess = false;

								try
								{
									// If file is open, use this fileNode, as physical file might be locked
									OpenFile *pOpenFile = findOpenFileByName(plainPath);
									if(pOpenFile != NULL)
									{
										int err = pOpenFile->fileNode_->getAttr(&buf_ue);
										if(err == 0)
											getAttrSuccess = true;
									}

									if(!getAttrSuccess)
									{
										int res = 0;
										const int flags = makeOpenFileFlags(true);
										boost::shared_ptr<FileNode> fileNode = 
											rootFS_->root->lookupNode( plainPath.c_str(), "open" );
										if(fileNode)
										{
											int err = fileNode->getAttr(&buf_ue);
											if(err == 0)
												getAttrSuccess = true;
										}
									}
								}
								catch( rlog::Error &err )
								{
									reportEncFSMPErr(L"Error during directory listing", cpath, err);
									skipThisFile = true;
								}

								if(!getAttrSuccess)
									skipThisFile = true;
								attribs.fileSize = buf_ue.st_size;
							}

							attribs.accessTime = UnixTimeToFileTime(buf.st_atime);
							attribs.createTime = UnixTimeToFileTime(buf.st_ctime);
							attribs.writeTime = UnixTimeToFileTime(buf.st_mtime);
							attribs.changeTime = UnixTimeToFileTime(buf.st_mtime);

							if(!skipThisFile)
								listResult->Add8(&attribs, name.c_str(), &needMore);

							if(!needMore)
								doCont = false;
						}
						else
						{
							reportEncFSMPErr(L"Couldn't get status on file", cpath);
//							return pfmErrorFailed;
						}
					}

				}
			}
		}
		catch( rlog::Error &err )
		{
			reportEncFSMPErr(L"Error in directory listing", "", err);
			return pfmErrorFailed;
		}
	}

	return 0;
}

int/*error*/ CCALL PFMLayer::ListEnd(int64_t openId,int64_t listId)
{
	if(!rootFS_)
		return pfmErrorFailed;
	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		return pfmErrorFailed;
	if(pOpenFile->isFile_)
		return pfmErrorNotAFolder;

	FileList *pFileList = NULL;
	std::list<FileList>::iterator iter = pOpenFile->fileLists_.begin();
	while(iter != pOpenFile->fileLists_.end())
	{
		FileList &cur = *iter;
		if(cur.listId_ == listId)
		{
			pOpenFile->fileLists_.erase(iter);
			return 0;
		}
		iter++;
	}

	return 0;
}

int/*error*/ CCALL PFMLayer::Read(int64_t openId,uint64_t fileOffset,void* data,size_t requestedSize,size_t* outActualSize)
{
	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		return pfmErrorFailed;
	if(!pOpenFile->isFile_)
		return pfmErrorNotAFile;
	if(pOpenFile->isDeleted_)
		return pfmErrorDeleted;

	boost::shared_ptr<FileNode> fileNode = pOpenFile->fileNode_;
	if(!fileNode)
		return pfmErrorFailed;

	try
	{
		ssize_t actSize = fileNode->read(static_cast<efs_off_t>(fileOffset), reinterpret_cast<unsigned char *>(data), requestedSize);
		if(outActualSize != NULL)
			(*outActualSize) = actSize;
	}
	catch( rlog::Error &err )
	{
		reportEncFSMPErr(L"Error during read operation", pOpenFile->pathName_, err);
		return pfmErrorFailed;
	}

	return 0;
}

int/*error*/ CCALL PFMLayer::Write(int64_t openId,uint64_t fileOffset,const void* data,size_t requestedSize,size_t* outActualSize)
{
	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		return pfmErrorFailed;
	if(!pOpenFile->isFile_)
		return pfmErrorNotAFile;
	if(pOpenFile->isDeleted_)
		return pfmErrorDeleted;

	boost::shared_ptr<FileNode> fileNode = pOpenFile->fileNode_;
	if(!fileNode)
		return pfmErrorFailed;

	try
	{
		bool isOK = fileNode->write(static_cast<efs_off_t>(fileOffset), const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(data)),
			requestedSize);
		if(outActualSize != NULL)
		{
			if(isOK)
				(*outActualSize) = requestedSize;
			else
				(*outActualSize) = 0;	// We don't have more information
		}
		if(!isOK)
			return pfmErrorFailed;
	}
	catch( rlog::Error &err )
	{
		reportEncFSMPErr(L"Error during write operation", pOpenFile->pathName_, err);
		return pfmErrorFailed;
	}

	return 0;
}

int/*error*/ CCALL PFMLayer::SetSize(int64_t openId,uint64_t fileSize)
{
	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		return pfmErrorFailed;
	if(!pOpenFile->isFile_)
		return pfmErrorNotAFile;
	if(pOpenFile->isDeleted_)
		return pfmErrorDeleted;

	boost::shared_ptr<FileNode> fileNode = pOpenFile->fileNode_;
	if(!fileNode)
		return pfmErrorFailed;

	try
	{
		int retVal = fileNode->truncate(static_cast<efs_off_t>(fileSize));
		if(retVal < 0)
			return pfmErrorFailed;
	}
	catch( rlog::Error &err )
	{
		reportEncFSMPErr(L"Error during SetSize operation", pOpenFile->pathName_, err);
		return pfmErrorFailed;
	}

	pOpenFile->fileSize_ = fileSize;
	return 0;
}

int/*error*/ CCALL PFMLayer::Capacity(uint64_t* totalCapacity,uint64_t* availableCapacity)
{
	// Retrieve absolute path of encrypted directory
	if(!rootFS_)
		return pfmErrorFailed;

	std::string rootDir = rootFS_->root->rootDirectory();

	uint64_t totalCapacityLocal, availableCapacityLocal;
	if(!fs_layer::capacity(rootDir, totalCapacityLocal, availableCapacityLocal))
		return pfmErrorInvalid;

	if(totalCapacity != NULL)
		(*totalCapacity) = totalCapacityLocal;
	if(availableCapacity != NULL)
		(*availableCapacity) = availableCapacityLocal;

	return 0;
}

int/*error*/ CCALL PFMLayer::FlushMedia(uint8_t/*bool*/ * mediaClean)
{
	if(mediaClean != NULL)
		(*mediaClean) = true;
	return 0;
}

int/*error*/ CCALL PFMLayer::Control(int64_t openId,int8_t accessLevel,int controlCode,const void* input,size_t inputSize,void* output,size_t maxOutputSize,size_t* outputSize)
{
	return pfmErrorAccessDenied;
}

int/*error*/ CCALL PFMLayer::MediaInfo(int64_t openId,PfmMediaInfo* mediaInfo,wchar_t** mediaLabel)
{
	wchar_t *stringCopy = new wchar_t[ mountName_.length() + 1];
	wcscpy(stringCopy, &(mountName_[0]));
	(*mediaLabel) = stringCopy;

	return 0;
}

int/*error*/ CCALL PFMLayer::Access(int64_t openId,int8_t accessLevel,PfmOpenAttribs* openAttribs)
{
	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		return pfmErrorFailed;

#if !defined(PFM_ACCESS_LEVEL_WORKAROUND)
	// Check for access level and fail if source file is not writable
	if(accessLevel >= pfmAccessLevelWriteData
		&& pOpenFile->isReadOnly_)
		return pfmErrorAccessDenied;
#endif

	pOpenFile->sequenceId_++;

	openAttribs->openId = pOpenFile->openId_;
	openAttribs->openSequence = pOpenFile->sequenceId_;
	openAttribs->accessLevel = (pOpenFile->isReadOnly_ ? pfmAccessLevelWriteInfo : pfmAccessLevelWriteData);

	if(pOpenFile->isFile_)
		openAttribs->attribs.fileType = pfmFileTypeFile;
	else
		openAttribs->attribs.fileType = pfmFileTypeFolder;
	openAttribs->attribs.fileFlags = pOpenFile->fileFlags_;
	openAttribs->attribs.fileId = pOpenFile->fileId_;
	openAttribs->attribs.fileSize = pOpenFile->fileSize_;

	openAttribs->attribs.accessTime = pOpenFile->accessTime_;
	openAttribs->attribs.createTime = pOpenFile->createTime_;
	openAttribs->attribs.writeTime = pOpenFile->writeTime_;
	openAttribs->attribs.changeTime = pOpenFile->changeTime_;

	return 0;
}

PFMLayer::FileID *PFMLayer::getFileID(const std::string &path)
{
	std::list< FileID >::iterator iter = fileIDs_.begin();
	while(iter != fileIDs_.end())
	{
		FileID *pCur = &(*iter);
		if(fs_layer::is_same_path(pCur->pathName_, path))
			return pCur;

		iter++;
	}

	return NULL;
}

int64_t PFMLayer::addFileID(const std::string &path)
{
	FileID newFileID;
	newFileID.pathName_ = path;
	newFileID.fileID_ = newFileID_++;
	fileIDs_.push_back(newFileID);


/*
	std::stringstream ostr;
	ostr << path.c_str() << ": " << newFileID.fileID_ << std::endl;
	std::string debugString = ostr.str();
#if defined(_WIN32)
	OutputDebugStringA(debugString.c_str());
#endif
*/
	return newFileID.fileID_;
}

bool PFMLayer::renameFileID(int64_t fileId, const std::string &newpath)
{
	std::list< FileID >::iterator iter = fileIDs_.begin();
	while(iter != fileIDs_.end())
	{
		FileID *pCur = &(*iter);
		if(pCur->fileID_ == fileId)
		{
			pCur->pathName_ = newpath;
			return true;
		}

		iter++;
	}

	return false;
}

bool PFMLayer::deleteFileID(int64_t fileId)
{
	std::list< FileID >::iterator iter = fileIDs_.begin();
	while(iter != fileIDs_.end())
	{
		FileID *pCur = &(*iter);
		if(pCur->fileID_ == fileId)
		{
			fileIDs_.erase(iter);
			return true;
		}

		iter++;
	}

	return false;
}

PFMLayer::OpenFile *PFMLayer::getOpenFile(int64_t openId)
{
	PFMLayer::OpenFile *pOpenFile = NULL;

	std::list< OpenFile >::iterator iter = openFiles_.begin();
	while(iter != openFiles_.end())
	{
		OpenFile &cur = *iter;
		if(cur.openId_ == openId)
		{
			return &cur;
		}

		iter++;
	}

	return NULL;
}

PFMLayer::OpenFile *PFMLayer::findOpenFileByName(const std::string &path)
{
	PFMLayer::OpenFile *pOpenFile = NULL;

	std::list< OpenFile >::iterator iter = openFiles_.begin();
	while(iter != openFiles_.end())
	{
		OpenFile &cur = *iter;
		if(fs_layer::is_same_path(path,
				cur.pathName_))
		{
			return &cur;
		}

		iter++;
	}

	return NULL;
}

int64_t PFMLayer::createFileId(const std::string &fn)
{
	FileID *pFileID = getFileID(fn);
	if(pFileID == NULL)
	{
		return addFileID(fn);
	}

	return pFileID->fileID_;
}

void PFMLayer::createEndName(wchar_t** endName, const char *fullPathName)
{
	if(endName != NULL)
	{
		// Convert filename from UTF8 to UTF16
		std::string fileName = fs_layer::extract_filename(std::string(fullPathName));
		std::wstring fileNameUTF16 = boost::locale::conv::utf_to_utf<wchar_t>(fileName.c_str());
		wchar_t *newFileNameUTF16 = new wchar_t[fileNameUTF16.length() + 1];
		wcscpy(newFileNameUTF16, fileNameUTF16.c_str());
		(*endName) = newFileNameUTF16;
	}
}

int PFMLayer::createOp(const std::string &path, int8_t createFileType, uint8_t createFileFlags,
	int64_t writeTime, int64_t newCreateOpenId,PfmOpenAttribs* openAttribs)
{
	try
	{
		if(createFileType == pfmFileTypeFile)
		{
#if defined(_WIN32)
			int flags = _O_BINARY;
#else
			int flags = 0;
#endif
			int res = 0;
#if defined(PFM_ACCESS_LEVEL_WORKAROUND)
			PT_UINT8 accessLevel = pfmAccessLevelWriteData;
#else
			PT_UINT8 accessLevel = pfmAccessLevelWriteInfo;
#endif
			// Create file
/*			if(createFileFlags != pfmFileFlagsInvalid)
			{
				if(createFileFlags & pfmFileFlagReadOnly)
#if defined(_WIN32)
					flags |= _O_RDONLY;
				else
					flags |= _O_RDWR;
#else
					flags |= O_RDONLY;
				else
					flags |= O_RDWR;
#endif
			}*/

#if defined(_WIN32)
			flags |= _O_RDWR;
#else
			flags |= O_RDWR;
#endif
			boost::shared_ptr<FileNode> fileNodeNew = 
				//rootFS_->root->openNode( path.c_str(), "open", flags, &res );
				rootFS_->root->lookupNode( path.c_str(), "mknod" );
#if defined(_WIN32)
			mode_t mode = _S_IREAD | _S_IFREG;
#else
			mode_t mode = S_IREAD | S_IFREG;
#endif
/*			if(createFileFlags != pfmFileFlagsInvalid)
			{
				if((createFileFlags & pfmFileFlagReadOnly) == 0)
				{
#if defined(_WIN32)
					mode |= _S_IWRITE;
#else
					mode |= S_IWRITE;
#endif
					accessLevel = pfmAccessLevelWriteData;
				}
			}*/
#if defined(_WIN32)
			mode |= _S_IWRITE;
#else
			mode |= S_IWRITE;
#endif
			accessLevel = pfmAccessLevelWriteData;




			res = fileNodeNew->mknod( mode, 0 );
			if(res != 0)
				return pfmErrorAccessDenied;

			// mknod creates a file, but does not open it.
			res = fileNodeNew->open( flags );
			if(res < 0)
				return pfmErrorFailed;

			OpenFile of;
			of.isFile_ = true;
			of.fileNode_ = fileNodeNew;
			of.openId_ = newCreateOpenId;
			of.sequenceId_ = 1;
			of.fd_ = res;
			of.pathName_ = path;
			of.isReadOnly_ = (accessLevel == pfmAccessLevelWriteInfo);
			of.fileId_ = createFileId(path);

			openAttribs->openId = newCreateOpenId;
			openAttribs->openSequence = 1;
			openAttribs->accessLevel = accessLevel;
			openAttribs->attribs.fileType = pfmFileTypeFile;
			openAttribs->attribs.fileFlags = createFileFlags;
			openAttribs->attribs.fileId = of.fileId_;
			openAttribs->attribs.fileSize = 0;

			openAttribs->attribs.accessTime = writeTime;
			openAttribs->attribs.createTime = writeTime;
			openAttribs->attribs.writeTime = writeTime;
			openAttribs->attribs.changeTime = writeTime;

			of.fileFlags_ = openAttribs->attribs.fileFlags;	// Store attributes in OpenFile class
			of.fileSize_ = openAttribs->attribs.fileSize;
			of.accessTime_ = openAttribs->attribs.accessTime;
			of.createTime_ = openAttribs->attribs.createTime;
			of.writeTime_ = openAttribs->attribs.writeTime;
			of.changeTime_ = openAttribs->attribs.changeTime;

			openFiles_.push_back(of);

			// Apply writeTime
			struct fs_layer::timeval_fs tm[2];
			tm[0].tv_sec = FileTimeToUnixTime(writeTime);
			tm[0].tv_usec = 0;
			tm[1].tv_sec = FileTimeToUnixTime(writeTime);
			tm[1].tv_usec = 0;
			fs_layer::utimes(fileNodeNew->cipherName(), tm);
		}
		else if(createFileType == pfmFileTypeFolder)
		{
			// Create directory
			mode_t mode = S_IRUSR | S_IRGRP | S_IROTH;
#if defined(PFM_ACCESS_LEVEL_WORKAROUND)
			PT_UINT8 accessLevel = pfmAccessLevelWriteData;
#else
			PT_UINT8 accessLevel = pfmAccessLevelWriteInfo;
#endif
			if(createFileFlags != pfmFileFlagsInvalid)
			{
				if((createFileFlags & pfmFileFlagReadOnly) == 0)
				{
					mode |= S_IWUSR | S_IXUSR | S_IWGRP | S_IXGRP | S_IWOTH | S_IXOTH;
					accessLevel = pfmAccessLevelWriteData;
				}
			}
			rootFS_->root->mkdir( path.c_str(), mode);

			OpenFile of;
			of.isFile_ = false;
			of.openId_ = newCreateOpenId;
			of.sequenceId_ = 1;
			of.pathName_ = path;
			of.isReadOnly_ = (accessLevel == pfmAccessLevelWriteInfo);
			of.fileId_ = createFileId(path);

			openAttribs->openId = newCreateOpenId;
			openAttribs->openSequence = 1;
			openAttribs->accessLevel = accessLevel;
			openAttribs->attribs.fileType = pfmFileTypeFolder;
			openAttribs->attribs.fileFlags = createFileFlags;
			openAttribs->attribs.fileId = of.fileId_;
			openAttribs->attribs.fileSize = 0;

			openAttribs->attribs.accessTime = writeTime;
			openAttribs->attribs.createTime = writeTime;
			openAttribs->attribs.writeTime = writeTime;
			openAttribs->attribs.changeTime = writeTime;

			of.fileFlags_ = openAttribs->attribs.fileFlags;	// Store attributes in OpenFile class
			of.fileSize_ = openAttribs->attribs.fileSize;
			of.accessTime_ = openAttribs->attribs.accessTime;
			of.createTime_ = openAttribs->attribs.createTime;
			of.writeTime_ = openAttribs->attribs.writeTime;
			of.changeTime_ = openAttribs->attribs.changeTime;

			openFiles_.push_back(of);

			// Apply writeTime
			std::string cipherPath = rootFS_->root->cipherPath(path.c_str());
			struct fs_layer::timeval_fs tm[2];
			tm[0].tv_sec = FileTimeToUnixTime(writeTime);
			tm[0].tv_usec = 0;
			tm[1].tv_sec = FileTimeToUnixTime(writeTime);
			tm[1].tv_usec = 0;
			fs_layer::utimes(cipherPath.c_str(), tm);
		}
		else
			return pfmErrorInvalid;		// We only create files and folders, nothing else
	}
	catch( rlog::Error &err )
	{
		reportEncFSMPErr(L"Error during create", path, err);
		return pfmErrorFailed;
	}

	return 0;
}

int PFMLayer::renameOp(PFMLayer::OpenFile *pOpenFile, const std::string &newPath)
{
	int openFlags = makeOpenFileFlags(pOpenFile->isReadOnly_);

	try
	{
		bool reopen = false;
		if(pOpenFile->fileNode_)	// Close file
		{
			reopen = true;
			pOpenFile->fileNode_.reset();
		}

		// Apply name change (works for folders and for files)
		int res = rootFS_->root->rename( pOpenFile->pathName_.c_str(), newPath.c_str() );
		if(res < 0)
		{
			if(errno == EACCES)
				return pfmErrorAccessDenied;
			else if(errno == EISDIR)
				return pfmErrorNotAFile;
			else if(errno == ENOTDIR)
				return pfmErrorNotAFolder;
			else return pfmErrorInvalid;
		}
		pOpenFile->pathName_ = newPath;
		if(!renameFileID(pOpenFile->fileId_, newPath))
			pfmErrorInvalid;

		if(reopen)
		{
			boost::shared_ptr<FileNode> fileNode = 
				rootFS_->root->openNode( newPath.c_str(), "open", openFlags, &res );
			if(!fileNode)
				return pfmErrorInvalid;
			pOpenFile->fileNode_ = fileNode;
		}
	}
	catch( rlog::Error &err )
	{
		reportEncFSMPErr(L"Error during rename operation", pOpenFile->pathName_, err);
		return pfmErrorFailed;
	}

	return 0;
}

/**
 * Deletion for files and folders.
 *
 * Deletion in PFM is defined as follows:
 * - When Delete is called, only the file name (i.e. the index in the directory) is removed
 * - File data must remain accessible until the file is closed
 *
 * This is implemented here as follows:
 * - When delete is called, the file is renamed on disk (and moved to root dir) to allow for new files
 *   to be created under the old name
 * - The OpenFile.isDeleted_ flag is set
 * - Undelete is possible with PFMLayer::Move until file is closed
 * - The file is deleted finally when PFMLayer::Close is called.
 */
int PFMLayer::deleteOp(PFMLayer::OpenFile *pOpenFile)
{
	if(!pOpenFile->isFile_)
	{
		try
		{
			// Check whether folder contains files. If yes, return pfmErrorNotEmpty
			DirTraverse dirT = rootFS_->root->openDir(pOpenFile->pathName_.c_str());
			if(!dirT.valid())
				return pfmErrorFailed;

			boost::shared_ptr<DirTraverse> pDirT ( new DirTraverse(dirT) );
			int fileType = 0;
			std::string name = pDirT->nextPlaintextName(&fileType);
			while(name == "." || name == "..")
			{
				name = pDirT->nextPlaintextName(&fileType);
			}

			if(!name.empty())
				return pfmErrorNotEmpty;
		}
		catch( rlog::Error &err )
		{
			reportEncFSMPErr(L"Error during delete operation", pOpenFile->pathName_, err);
			return pfmErrorFailed;
		}
	}

	// Generate random file name
	boost::random::random_device rng;
	boost::random::uniform_int_distribution<> random_int(0, 255);

	std::stringstream ostr;
	ostr << "/encfsmp_deleted_" << std::hex;
	ostr << random_int(rng) << "-";
	ostr << random_int(rng) << "-";
	ostr << random_int(rng) << "-";
	ostr << random_int(rng);
	std::string newname = ostr.str();

	// Rename old file
	int retVal = renameOp(pOpenFile, newname);
	if(retVal != 0)
		return retVal;

	// Delete it
	pOpenFile->isDeleted_ = true;
	
	deleteFileID(pOpenFile->fileId_);

	return 0;
}

void PFMLayer::openExisting(PFMLayer::OpenFile *pOpenFile, PfmOpenAttribs *openAttribs,
	PT_UINT8 accessLevel)
{
	accessLevel = determineAccessLevel(pOpenFile->isReadOnly_, accessLevel);

	pOpenFile->sequenceId_++;

	openAttribs->openId = pOpenFile->openId_;
	openAttribs->openSequence = pOpenFile->sequenceId_;
	openAttribs->accessLevel = accessLevel;

	if(pOpenFile->isFile_)
		openAttribs->attribs.fileType = pfmFileTypeFile;
	else
		openAttribs->attribs.fileType = pfmFileTypeFolder;
	openAttribs->attribs.fileFlags = pOpenFile->fileFlags_;
	openAttribs->attribs.fileId = pOpenFile->fileId_;
	openAttribs->attribs.fileSize = pOpenFile->fileSize_;

	openAttribs->attribs.accessTime = pOpenFile->accessTime_;
	openAttribs->attribs.createTime = pOpenFile->createTime_;
	openAttribs->attribs.writeTime = pOpenFile->writeTime_;
	openAttribs->attribs.changeTime = pOpenFile->changeTime_;
}

int PFMLayer::openFileOp(boost::shared_ptr<FileNode> fileNode, int fd, PfmOpenAttribs *openAttribs,
	int64_t newExistingOpenId, PT_UINT8 accessLevel, const std::string &path)
{
	efs_stat buf;

	try
	{
		int err = fileNode->getAttr(&buf);
		if(err != 0)
			return pfmErrorFailed;
	}
	catch( rlog::Error &err )
	{
		reportEncFSMPErr(L"Error during open", path, err);
		return pfmErrorFailed;
	}

	// File found
	OpenFile of;
	of.isFile_ = true;
	of.fileNode_ = fileNode;
	of.openId_ = newExistingOpenId;
	of.sequenceId_ = 1;
	of.fd_ = fd;
	of.pathName_ = path;
	of.isReadOnly_ = ((buf.st_mode & S_IWUSR) == 0);
	of.fileId_ = createFileId(path);

	accessLevel = determineAccessLevel(of.isReadOnly_, accessLevel);

	openAttribs->openId = newExistingOpenId;
	openAttribs->openSequence = 1;
	openAttribs->accessLevel = accessLevel;
	openAttribs->attribs.fileType = pfmFileTypeFile;
	openAttribs->attribs.fileFlags = 0;
	if(of.isReadOnly_)
		openAttribs->attribs.fileFlags |= pfmFileFlagReadOnly;
	if(isHiddenFile(path))
		openAttribs->attribs.fileFlags |= pfmFileFlagHidden;
#if defined(EFS_MACOSX)
	openAttribs->attribs.fileFlags |= pfmFileFlagArchive;	// Inverted logic of archive flag on OS X
#endif
	openAttribs->attribs.fileId = of.fileId_;
	openAttribs->attribs.fileSize = buf.st_size;

	openAttribs->attribs.accessTime = UnixTimeToFileTime(buf.st_atime);
	openAttribs->attribs.createTime = UnixTimeToFileTime(buf.st_ctime);
	openAttribs->attribs.writeTime = UnixTimeToFileTime(buf.st_mtime);
	openAttribs->attribs.changeTime = UnixTimeToFileTime(buf.st_mtime);

	of.fileFlags_ = openAttribs->attribs.fileFlags;	// Store attributes in OpenFile class
	of.fileSize_ = openAttribs->attribs.fileSize;
	of.accessTime_ = openAttribs->attribs.accessTime;
	of.createTime_ = openAttribs->attribs.createTime;
	of.writeTime_ = openAttribs->attribs.writeTime;
	of.changeTime_ = openAttribs->attribs.changeTime;

	openFiles_.push_back(of);

	return 0;
}

int PFMLayer::openDirOp(PfmOpenAttribs *openAttribs, int64_t newExistingOpenId,
	PT_UINT8 accessLevel, const std::string &path)
{
	// Get directory information
	efs_stat buf;
	try
	{
		std::string cipherPath = rootFS_->root->cipherPath(path.c_str());
		int statOk = fs_layer::stat(cipherPath.c_str(), &buf);
		if(statOk < 0)
			return pfmErrorInvalid;
	}
	catch( rlog::Error &err )
	{
		reportEncFSMPErr(L"Error during open dir operation", path, err);
		return pfmErrorFailed;
	}

	OpenFile of;
	of.isFile_ = false;
	of.openId_ = newExistingOpenId;
	of.sequenceId_ = 1;
	of.pathName_ = path;
	of.isReadOnly_ = ((buf.st_mode & S_IWUSR) == 0);
	of.fileId_ = createFileId(path);

	accessLevel = determineAccessLevel(of.isReadOnly_, accessLevel);

	openAttribs->openId = newExistingOpenId;
	openAttribs->openSequence = 1;
	openAttribs->accessLevel = accessLevel;
	openAttribs->attribs.fileType = pfmFileTypeFolder;
	openAttribs->attribs.fileFlags = 0;
	if(of.isReadOnly_)
		openAttribs->attribs.fileFlags |= pfmFileFlagReadOnly;
#if defined(EFS_MACOSX)
	openAttribs->attribs.fileFlags |= pfmFileFlagArchive;	// Inverted logic of archive flag on OS X
#endif
	openAttribs->attribs.fileId = of.fileId_;
	openAttribs->attribs.fileSize = 0;

	openAttribs->attribs.accessTime = UnixTimeToFileTime(buf.st_atime);
	openAttribs->attribs.createTime = UnixTimeToFileTime(buf.st_ctime);
	openAttribs->attribs.writeTime = UnixTimeToFileTime(buf.st_mtime);
	openAttribs->attribs.changeTime = UnixTimeToFileTime(buf.st_mtime);

	of.fileFlags_ = openAttribs->attribs.fileFlags;	// Store attributes in OpenFile class
	of.fileSize_ = openAttribs->attribs.fileSize;
	of.accessTime_ = openAttribs->attribs.accessTime;
	of.createTime_ = openAttribs->attribs.createTime;
	of.writeTime_ = openAttribs->attribs.writeTime;
	of.changeTime_ = openAttribs->attribs.changeTime;

	openFiles_.push_back(of);

	return 0;
}

int PFMLayer::makeOpenFileFlags(PT_INT8 accessLevel)
{
#if defined(PFM_ACCESS_LEVEL_WORKAROUND)
	accessLevel = pfmAccessLevelWriteData;
#endif

#if defined(_WIN32)
	int flags = O_BINARY | O_RDWR;
#else
	int flags = O_RDWR;
#endif

	if(accessLevel >= pfmAccessLevelWriteData)
		flags |= O_RDWR;
	else if(accessLevel >= pfmAccessLevelReadData)
		flags |= O_RDONLY;

	return flags;
}

int PFMLayer::makeOpenFileFlags(bool isReadOnly)
{
#if defined(PFM_ACCESS_LEVEL_WORKAROUND)
	isReadOnly = false;
#endif

#if defined(_WIN32)
	int flags = O_BINARY | O_RDWR;
#else
	int flags = O_RDWR;
#endif

	if(isReadOnly)
		flags |= O_RDONLY;
	else
		flags |= O_RDWR;

	return flags;
}

void PFMLayer::reportRLogErr(rlog::Error &err)
{
	rError("encode err: %s", err.message());
	err.log( rlog::_RLWarningChannel );

#if defined(_WIN32) && defined(_DEBUG)
	std::stringstream ostr;
	ostr << err.message() << std::endl;
	std::string debugString = ostr.str();
	OutputDebugStringA(debugString.c_str());
#endif
}

void PFMLayer::reportEncFSMPErr(const std::wstring &errStr, const std::string &fn, rlog::Error &err)
{
	EncFSMPLogger::log(errStr, fn, &err);
}

void PFMLayer::reportEncFSMPErr(const std::wstring &errStr, const std::string &fn)
{
	EncFSMPLogger::log(errStr, fn, NULL);
}

bool PFMLayer::isHiddenFile(const std::string &fullpath)
{
/*	std::string fn = fs_layer::extract_filename(fullpath);
	if(fn.length() > 1)
	{
		if(fn[0] == '.')
			return true;
	}
	*/
	return false;
}

bool PFMLayer::isSkippedFile(const std::string &fullpath)
{
	// Only skip files in the root directory
	std::string path = fs_layer::extract_path(fullpath);
	if(path != std::string("/")
		|| path.empty())
		return false;

	std::string fn = fs_layer::extract_filename(fullpath);
	if(fn.length() > 1)
	{
		if(fn.substr(0, 6) == ".encfs")
			return true;
	}
	return false;
}

PT_INT8 PFMLayer::determineAccessLevel(bool isReadOnly, PT_INT8 requestedAccessLevel)
{
	PT_INT8 accessLevel = requestedAccessLevel;
#if defined(PFM_ACCESS_LEVEL_WORKAROUND)
	accessLevel = pfmAccessLevelWriteData;
#else
	if(isReadOnly)
	{
		if(requestedAccessLevel >= pfmAccessLevelDelete)
			accessLevel = pfmAccessLevelDelete;
		else
			accessLevel = pfmAccessLevelWriteInfo;
	}
	else
	{
		accessLevel = pfmAccessLevelWriteData;
	}
/*	if(accessLevel != pfmAccessLevelDelete)
		accessLevel = (pOpenFile->isReadOnly_ ? pfmAccessLevelWriteInfo : pfmAccessLevelWriteData);
	else
		accessLevel = pfmAccessLevelWriteData;*/
#endif

	return accessLevel;
}

/**
 * A debug method for printing the currently open files.
 */
void PFMLayer::printOpenFiles(const char *msg)
{
	PFMLayer::OpenFile *pOpenFile = NULL;

#if defined(EFS_WIN32)
	OutputDebugStringA(msg);
	OutputDebugStringA(": List of open files\n");
#else
	std::cout << msg << ": List of open files" << std::endl;
#endif
	int i = 0;
	std::list< OpenFile >::iterator iter = openFiles_.begin();
	while(iter != openFiles_.end())
	{
		OpenFile &cur = *iter;
		
#if defined(EFS_WIN32)
		std::ostringstream ostr;
		ostr << " " << i << ": "
			<< cur.pathName_.c_str()
			<< (cur.isDeleted_ ? " (del)" : " (nd)")
			<< std::endl;
		OutputDebugStringA(ostr.str().c_str());
#else
		std::cout << " " << i << ": "
			<< cur.pathName_.c_str()
			<< (cur.isDeleted_ ? " (del)" : " (nd)")
			<< std::endl;
#endif
		i++;
		iter++;
	}
}

