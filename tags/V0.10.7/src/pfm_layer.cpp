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

static PfmOpenAttribs zeroOpenAttribs = {};
static PfmAttribs zeroAttribs = {};
static PfmMediaInfo zeroMediaInfo = {};

PFMLayer::PFMLayer() :
	marshaller(NULL),
	newFileID_(1)
{
}

PFMLayer::~PFMLayer()
{
	// Close openFiles_
	OpenFileMapType::iterator iter = openFiles_.begin();
	while(iter != openFiles_.end())
	{
		OpenFile &cur = iter->second;

		// Save some attributes for later.
		// Reason: We need the file to be closed in order to delete it.
		// To close it, we have to delete the OpenFile instance.
		bool isDeleted = cur.isDeleted_;
		bool isFile = cur.isFile_;
		std::string pathName = cur.pathName_;

		// The file is closed. Remove from the list of open files
		openFiles_.erase(iter);
		openIdMap_.erase(pathName);
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
	wchar_t driveLetter, bool useCaching, bool worldWrite, bool localDrive,
	bool startBrowser, std::ostream &ostr)
{
	rootFS_ = rootFS;
	mountName_ = mountDir;
	if(useCaching)
		fileStatCache_.setCacheSize(1000);
	else
		fileStatCache_.setCacheSize(0);

	// The following code is copied mostly from the Pismo File Mount's example code tempfs.cpp
	int error = 0;
	PfmMount* mount = 0;
	PfmMountCreateParams mcp;
	PfmMarshallerServeParams msp;

	msp.dispatch = this;

	mcp.mountSourceName = mountDir;
	mcp.mountFlags |= pfmMountFlagUnmountOnRelease;	// | pfmMountFlagWorldOwned;
	if(localDrive)
		mcp.mountFlags |= pfmMountFlagLocalDriveType;
	if(startBrowser)
		mcp.mountFlags |= pfmMountFlagBrowse;
	if(worldWrite)
		mcp.mountFlags |= (pfmMountFlagWorldRead | pfmMountFlagWorldWrite);

	if(driveLetter != L'-')
	{
		mcp.driveLetter = driveLetter;
		mcp.mountFlags |= pfmMountFlagUncOnly;
	}

	error = PfmMarshallerFactory(&marshaller);
	if(error)
	{
		ostr << "ERROR: " << error << " Unable to create marshaller" << std::endl;
		return;
	}

	error = create_pipe(&msp.toFormatterRead,&mcp.toFormatterWrite);
	if(!error)
	{
		error = create_pipe(&mcp.fromFormatterRead,&msp.fromFormatterWrite);
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
	//int retVal = marshaller->ServeReadWrite(this, volumeFlags, EncFSMPStrings::formatterName8_.c_str(), toFormatterRead, fromFormatterWrite);
	
	msp.formatterName = EncFSMPStrings::formatterName8_.c_str();
	msp.volumeFlags |= volumeFlags;
	marshaller->ServeDispatch(&msp);

	if(mount)
		mount->Release();

	if(marshaller)
	{
		marshaller->Release();
		marshaller = 0;
	}

	close_fd(msp.toFormatterRead);
	close_fd(msp.fromFormatterWrite);
}


int64_t UnixTimeToFileTime(time_t t)
{
	return (static_cast<int64_t>(t) * 10000000LL) + 116444736000000000LL;
}

long FileTimeToUnixTime(int64_t t)
{
	return static_cast<long>((t - 116444736000000000LL) / 10000000LL);
}

void CCALL PFMLayer::Open(PfmMarshallerOpenOp* op, void* formatterUse)
{
	int perr = 0;

	if(!rootFS_)
		perr = pfmErrorFailed;

	const PfmNamePart* nameParts = op->NameParts();
	size_t namePartCount = op->NamePartCount();
	int8_t createFileType = op->CreateFileType();
	int8_t createFileFlags = op->CreateFileFlags();
	int64_t writeTime = op->WriteTime();
	int64_t newCreateOpenId = op->NewCreateOpenId();
	int64_t newExistingOpenId = op->NewExistingOpenId();
	int8_t existingAccessLevel = op->ExistingAccessLevel();
	bool existed = false;
	PfmOpenAttribs openAttribs = zeroOpenAttribs;
	int64_t parentFileId = 0;
	std::wstring endName;

	if(perr == 0)
	{
		// Construct name
		std::string path("/"), parentFolder;		// In UTF-8 encoding
		for(size_t i = 0; i < namePartCount; i++)
		{
			parentFolder = path;

			path = fs_layer::concat_path(path, nameParts[i].name8, true);
		}

		// Check whether parent folder exists

		// Root folder always exists
		if(parentFolder.empty())
		{
			parentFileId = 0;	// Special fileId for "parent" of root
		}
		else if(parentFolder == "/")
		{
			parentFileId = createFileId(parentFolder);
		}
		else
		{
			// Look in open files
			bool parentFolderFound = false;
			OpenFile *pParentOpenFile = findOpenFileByName(parentFolder);
			if(pParentOpenFile != NULL
				&& !pParentOpenFile->isFile_)
			{
				parentFolderFound = true;
				parentFileId = pParentOpenFile->fileId_;
			}

			if(!parentFolderFound)
			{
				// Not found in open files, look on file system
				std::string cipherPath = rootFS_->root->cipherPath(parentFolder.c_str());
				efs_stat stat;
				int ret = fileStatCache_.stat(cipherPath.c_str(), &stat);
				if(ret != 0)
					perr = pfmErrorParentNotFound;
				if((perr == 0) && ((stat.st_mode & S_IFDIR) == 0))	// Check whether it is a directory
					perr = pfmErrorParentNotFound;

				if(perr == 0)
					parentFileId = createFileId(parentFolder);

			}
		}

		if(perr == 0)
		{
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
						perr = pfmErrorNotFound;
					else
						perr = pfmErrorDeleted;			// We can't create a file with the same name as a deleted file
				}
				else
				{
					// Check for increase in access level for files

					if(existingAccessLevel >= pfmAccessLevelWriteData
						&& pOpenFile->isFile_)
					{
						// Check for access level and fail if source file is not writable
						if(pOpenFile->isReadOnly_)
							perr = pfmErrorAccessDenied;
						else if(pOpenFile->isOpenedReadOnly_)
						{
							// Try to reopen file with write access
							pOpenFile->fileNode_.reset();

							int res = 0;
							int8_t accessLevel = determineAccessLevel(pOpenFile->isReadOnly_, existingAccessLevel);
							pOpenFile->fileNode_ =
								rootFS_->root->openNode(pOpenFile->pathName_.c_str(), "open", makeOpenFileFlags(static_cast<PT_INT8>(accessLevel)), &res);
							if(!pOpenFile->fileNode_)
							{
								// Failed, try to open again for reading
								pOpenFile->fileNode_ =
									rootFS_->root->openNode(pOpenFile->pathName_.c_str(), "open", makeOpenFileFlags(pfmAccessLevelReadData), &res);
								perr = pfmErrorAccessDenied;
							}
							if(perr == 0)
								pOpenFile->isOpenedReadOnly_ = false;
						}
					}

					openExisting(pOpenFile, &openAttribs, existingAccessLevel);

					createEndName(endName, pOpenFile->pathName_.c_str());
					existed = true;
				}

				// Return here
				op->Complete(perr, existed, &openAttribs, parentFileId, endName.c_str(), 0, 0, 0, 0);
				return;
			}
		}
		// Check whether path is a directory or a file
		bool isDir = false;
		{
			std::string cipherPath = rootFS_->root->cipherPath(path.c_str());
			efs_stat stat;
			int ret = fileStatCache_.stat(cipherPath.c_str(), &stat);
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
					rootFS_->root->openNode(path.c_str(), "open", makeOpenFileFlags(existingAccessLevel), &res);

				if(fileNode)
				{
					perr = openFileOp(fileNode, res, &openAttribs, newExistingOpenId, existingAccessLevel, path);
					if(perr == 0)
					{
						createEndName(endName, path.c_str());
						existed = true;
					}
				}
				else
				{
					// File or directory did not yet exist
					existed = false;

					// From doc: If the indicated file does not exist and the newCreateOpenId parameter is zero then the formatter should return pfmErrorNotFound
					if(newCreateOpenId == 0
						|| createFileType == pfmFileTypeNone)
					{
						res = std::abs(res);
						if(errno == EACCES)
							perr = pfmErrorAccessDenied;
						else if(errno == EISDIR)
							perr = pfmErrorNotAFile;
						else if(errno == ENOENT)
							perr = pfmErrorNotFound;
						else perr = pfmErrorInvalid;
					}

					if(perr == 0)
					{
						// Create new file
						perr = createOp(path, createFileType, createFileFlags, writeTime, newCreateOpenId, &openAttribs);

						if(perr == 0)
							createEndName(endName, path.c_str());
					}
				}
			}
			catch(rlog::Error &err)
			{
				reportEncFSMPErr(L"Open failed", path, err);
			}
		}
		else
		{
			// Directory exists, "open" it
			perr = openDirOp(&openAttribs, newExistingOpenId, existingAccessLevel, path);

			if(perr == 0)
			{
				createEndName(endName, path.c_str());
				existed = true;
			}
		}
	}

	op->Complete(perr, existed, &openAttribs, parentFileId, endName.c_str(), 0, 0, 0, 0);
}

void CCALL PFMLayer::Replace(PfmMarshallerReplaceOp* op, void* formatterUse)
{
	int64_t targetOpenId = op->TargetOpenId();
	int64_t targetParentFileId = op->TargetParentFileId();
	const PfmNamePart* targetEndName = op->TargetEndName();
	uint8_t createFileFlags = op->CreateFileFlags();
	int64_t writeTime = op->WriteTime();
	int64_t newCreateOpenId = op->NewCreateOpenId();
	int perr = 0;
	PfmOpenAttribs openAttribs = zeroOpenAttribs;

	OpenFile *pOpenFile = getOpenFile(targetOpenId);
	if(pOpenFile == NULL)
		perr = pfmErrorFailed;

	// We do not support Replace for folders
	if((perr == 0) && (!pOpenFile->isFile_))
		perr = pfmErrorInvalid;

	if((perr == 0) && (pOpenFile->isDeleted_))
		perr = pfmErrorDeleted;

	if(perr == 0)
	{
		// The documentation of PFM states that the new file should be created with the same name
		// as the existing one, then the old file should be deleted. Deletion under PFM means
		// that only the directory entry is deleted, but not the file itself. The file is physically
		// deleted only when it is closed.
		// Since here deletion is implemented by only setting the flag isDeleted, this would
		// mean that two files have the same name until the old one gets deleted. This obviously doesn't
		// work.
		// Workaround: Rename the old file, mark it as deleted, then create the new file

		std::string oldPath = pOpenFile->pathName_;

		perr = deleteOp(pOpenFile);

		if(perr == 0)
		{
			// Create new file
			int createFileType = (pOpenFile->isFile_ ? pfmFileTypeFile : pfmFileTypeFolder);
			perr = createOp(oldPath, createFileType, createFileFlags, writeTime, newCreateOpenId, &openAttribs);
		}
	}

	op->Complete(perr, &openAttribs, 0);
}

void CCALL PFMLayer::Move(PfmMarshallerMoveOp* op, void* formatterUse)
{
	int64_t sourceOpenId = op->SourceOpenId();
	int64_t sourceParentFileId = op->SourceParentFileId();
	const PfmNamePart* sourceEndName = op->SourceEndName();
	const PfmNamePart* targetNameParts = op->TargetNameParts();
	size_t targetNamePartCount = op->TargetNamePartCount();
	bool deleteSource = !!op->DeleteSource();
	int64_t writeTime = op->WriteTime();
	int8_t existingAccessLevel = op->ExistingAccessLevel();
	int64_t newExistingOpenId = op->NewExistingOpenId();
	int perr = 0;
	bool existed = false;
	PfmOpenAttribs openAttribs = zeroOpenAttribs;
	int64_t parentFileId = 0;
	std::wstring endName;

	OpenFile *pOpenFile = getOpenFile(sourceOpenId);
	if(pOpenFile == NULL)
		perr = pfmErrorFailed;

	// if pOpenFile is root, fail
	if((perr == 0) && (fs_layer::is_same_path(pOpenFile->pathName_, "")
		|| fs_layer::is_same_path(pOpenFile->pathName_, "/")))
		perr = pfmErrorAccessDenied;

	if(perr == 0)
	{
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
			parentFileId = pParentOpenFile->fileId_;
		}

		if(!parentFolderFound)
		{
			try
			{
				// Not found in open files, look on file system
				DirTraverse dirTParent = rootFS_->root->openDir(parentFolder.c_str());
				// If not, return pfmErrorParentNotFound
				if(!dirTParent.valid())
					perr = pfmErrorParentNotFound;
			}
			catch(rlog::Error &err)
			{
				// Not an error
				perr = pfmErrorParentNotFound;
			}

			if(perr == 0)
				parentFileId = createFileId(parentFolder);
		}

		if(perr == 0
			&& deleteSource == 0
			&& !pOpenFile->isDeleted_)
		{
			// Request to make a hard link: We do not support links
			perr = pfmErrorInvalid;
		}

		if(pOpenFile->isDeleted_)
		{
			// Undelete
			pOpenFile->isDeleted_ = false;

			// TODO: Check whether target file already exists

			// Apply name change
			perr = renameOp(pOpenFile, path);
		}

		if(perr == 0)
		{
			// === Check whether target file already exists

			// Check whether target file is opened already
			OpenFile *pOpenFileTarget = findOpenFileByName(path);
			if(pOpenFileTarget != NULL)
			{
				if(pOpenFileTarget->isDeleted_)
				{
					// Found the file, but it is marked as deleted
					// This is (should be) extremely improbable, but should lead to an error.
					perr = pfmErrorInvalid;
				}
				else
				{
					openExisting(pOpenFileTarget, &openAttribs, existingAccessLevel);

					createEndName(endName, path.c_str());

					existed = true;

					op->Complete(perr, existed, &openAttribs, parentFileId, endName.c_str(), 0, 0, 0, 0);
					return;
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
				catch(rlog::Error &err)
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
						rootFS_->root->openNode(path.c_str(), "open", makeOpenFileFlags(existingAccessLevel), &res);

					if(fileNode)
					{
						// Target file already exists. Perform open
						perr = openFileOp(fileNode, res, &openAttribs, newExistingOpenId, existingAccessLevel, path);

						if(perr == 0)
						{
							createEndName(endName, path.c_str());
							existed = true;
						}
					}
					else
					{
						// File or directory did not yet exist
						existed = false;

						perr = renameOp(pOpenFile, path);

						if(perr == 0)
						{
							openExisting(pOpenFile, &openAttribs, pfmAccessLevelWriteData);

							createEndName(endName, path.c_str());
						}
					}
				}
				catch(rlog::Error &err)
				{
					reportEncFSMPErr(L"File not found in move operation", path, err);
					perr = pfmErrorFailed;
				}
			}
			else
			{
				// Directory exists, "open" it
				perr = openDirOp(&openAttribs, newExistingOpenId, existingAccessLevel, path);

				if(perr == 0)
				{
					createEndName(endName, path.c_str());
					existed = true;
				}
			}
		}
	}
	op->Complete(perr, existed, &openAttribs, parentFileId, endName.c_str(), 0, 0, 0, 0);
}

void CCALL PFMLayer::MoveReplace(PfmMarshallerMoveReplaceOp* op, void* formatterUse)
{
	int64_t sourceOpenId = op->SourceOpenId();
	int64_t sourceParentFileId = op->SourceParentFileId();
	const PfmNamePart* sourceEndName = op->SourceEndName();
	int64_t targetOpenId = op->TargetOpenId();
	int64_t targetParentFileId = op->TargetParentFileId();
	const PfmNamePart* targetEndName = op->TargetEndName();
	uint8_t/*bool*/ deleteSource = op->DeleteSource();
	int64_t writeTime = op->WriteTime();
	int perr = 0;

	OpenFile *pSourceOpenFile = getOpenFile(sourceOpenId);
	if(pSourceOpenFile == NULL)
		perr = pfmErrorFailed;

	OpenFile *pTargetOpenFile = NULL;
	if(perr == 0)
	{
		pTargetOpenFile = getOpenFile(targetOpenId);
		if(pTargetOpenFile == NULL)
			perr = pfmErrorFailed;
	}

	if(perr == 0)
	{
		if(pSourceOpenFile == pTargetOpenFile)
			perr = pfmErrorInvalid;
	}

	// if pTargetOpenFile is root, fail
	if((perr == 0) &&
		(fs_layer::is_same_path(pTargetOpenFile->pathName_, "")
		|| fs_layer::is_same_path(pTargetOpenFile->pathName_, "/")))
		perr = pfmErrorAccessDenied;

	if((perr == 0) && (pTargetOpenFile->isDeleted_))
		perr = pfmErrorDeleted;

	if(perr == 0
		&& deleteSource == 0
		&& !pSourceOpenFile->isDeleted_)
	{
		// Request to make a hard link: We do not support links
		perr = pfmErrorInvalid;
	}

	std::string newPath = pTargetOpenFile->pathName_;

	// Delete target
	if(perr == 0)
		perr = deleteOp(pTargetOpenFile);

	// Rename source to target
	if(perr == 0)
		perr = renameOp(pSourceOpenFile, newPath);

	op->Complete(perr);
}

void CCALL PFMLayer::Delete(PfmMarshallerDeleteOp* op, void* formatterUse)
{
	int64_t openId = op->OpenId();
	int64_t parentFileId = op->ParentFileId();
	const PfmNamePart* endName = op->EndName();
	int64_t writeTime = op->WriteTime();
	int perr = 0;

	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		perr = pfmErrorFailed;

	// if pOpenFile is root, fail
	if((perr == 0) && (fs_layer::is_same_path(pOpenFile->pathName_, "")
		|| fs_layer::is_same_path(pOpenFile->pathName_, "/")))
		perr = pfmErrorAccessDenied;

	if((perr == 0) && (pOpenFile->isDeleted_))
		perr = pfmErrorDeleted;

	if(perr == 0)
		perr = deleteOp(pOpenFile);

	op->Complete(perr);
}

void CCALL PFMLayer::Close(PfmMarshallerCloseOp* op, void* formatterUse)
{
	int64_t openId = op->OpenId();
	int64_t openSequence = op->OpenSequence();
	int perr = 0;

	OpenFileMapType::iterator iter = openFiles_.find(openId);
	if(iter != openFiles_.end())
	{
		OpenFile &cur = iter->second;

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
			openIdMap_.erase(pathName);
			// Don't access cur after this point

			if(isDeleted)
			{
				try
				{
					if(isFile)
					{
						int retVal = rootFS_->root->unlink(pathName.c_str());
						if(retVal != 0)												// See above: Is file is not closed, delete does not work
							throw rlog::Error("PFMLayer::Close", __FILE__,
								__FUNCTION__, __LINE__, "Could not delete file");
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
		perr = 0;
	}
	else
		perr = pfmErrorFailed;

	op->Complete(perr);
}

void CCALL PFMLayer::FlushFile(PfmMarshallerFlushFileOp* op, void* formatterUse)
{
	int64_t openId = op->OpenId();
	uint8_t flushFlags = op->FlushFlags();
	uint8_t fileFlags = op->FileFlags();
	uint8_t color = op->Color();
	int64_t createTime = op->CreateTime();
	int64_t accessTime = op->AccessTime();
	int64_t writeTime = op->WriteTime();
	int64_t changeTime = op->ChangeTime();
	int perr = 0;
	PfmOpenAttribs openAttribs = zeroOpenAttribs;

	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		perr = pfmErrorFailed;
	else
	{
		if(pOpenFile->isFile_)
		{
			const char *cipherName = pOpenFile->fileNode_->cipherName();
			if(fileFlags != pfmFileFlagsInvalid)
			{
				pOpenFile->fileFlags_ = fileFlags;

				// Change read only flag
				efs_stat buf;
				if(fileStatCache_.stat(cipherName, &buf) < 0)
					perr = pfmErrorFailed;
				else
				{
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
						perr = pfmErrorFailed;
					else
						fileStatCache_.forgetCachedStat(cipherName);
				}
			}

			if((perr == 0) && (writeTime != pfmTimeInvalid))
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

				fileStatCache_.forgetCachedStat(cipherName);
			}
		}

		if((perr == 0) && (flushFlags & pfmFlushFlagOpen))
			openExisting(pOpenFile, &openAttribs, pfmAccessLevelWriteData);
	}

	op->Complete(perr, &openAttribs, 0);
}

void CCALL PFMLayer::List(PfmMarshallerListOp* op, void* formatterUse)
{
	int64_t openId = op->OpenId();
	int64_t listId = op->ListId();
	int perr = 0;
	bool noMore = false;

	if(!rootFS_)
		perr = pfmErrorFailed;
	OpenFile *pOpenFile = NULL;
	if(perr == 0)
		pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		perr = pfmErrorFailed;
	if(perr == 0 && pOpenFile->isFile_)
		perr = pfmErrorNotAFolder;
	if(perr == 0 && pOpenFile->isDeleted_)
		perr = pfmErrorDeleted;

	if(perr != 0)
	{
		op->Complete(perr, noMore);
		return;
	}

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
				perr = pfmErrorFailed;

			if(perr == 0)
			{
				fl.listId_ = listId;
				boost::shared_ptr<DirTraverse> pDirTTemp(new DirTraverse(dirT));
				fl.pDirT_ = pDirTTemp;
				pOpenFile->fileLists_.push_back(fl);
				pFileList = &(pOpenFile->fileLists_.back());
			}
		}
		catch( rlog::Error &err )
		{
			reportEncFSMPErr(L"Could not open directory", pOpenFile->pathName_, err);
			perr = pfmErrorFailed;
		}

		if(perr != 0)
		{
			op->Complete(perr, noMore);
			return;
		}
	}

	// Process result from previous call
	if(pFileList->hasPreviousResult_)
	{
		uint8_t wasAdded = 1;
		wasAdded = op->Add8(&(pFileList->prevAttribs_), pFileList->prevName_.c_str());

		if(wasAdded)
			pFileList->hasPreviousResult_ = false;
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
				//listResult->NoMore();
				noMore = true;
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

						if( !fileStatCache_.stat(cpath.c_str(), &buf) )		//fs_layer::lstat( cpath.c_str(), &buf ))
						{
							uint8_t wasAdded = 1;
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
										int err = pOpenFile->fileNode_->getAttr(&buf_ue, &fileStatCache_);
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
											int err = fileNode->getAttr(&buf_ue, &fileStatCache_);
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
								wasAdded = op->Add8(&attribs, name.c_str());

							if(!wasAdded)
							{
								doCont = false;

								// Save current result for later
								pFileList->hasPreviousResult_ = true;
								pFileList->prevAttribs_ = attribs;
								pFileList->prevName_ = name;
							}
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
			perr = pfmErrorFailed;
		}
	}

	op->Complete(perr, noMore);
}

void CCALL PFMLayer::ListEnd(PfmMarshallerListEndOp* op, void* formatterUse)
{
	int64_t openId = op->OpenId();
	int64_t listId = op->ListId();
	int perr = 0;

	if(!rootFS_)
		perr = pfmErrorFailed;
	OpenFile *pOpenFile = getOpenFile(openId);
	if(perr == 0 && pOpenFile == NULL)
		perr = pfmErrorFailed;
	if(perr = 0 && pOpenFile->isFile_)
		perr = pfmErrorNotAFolder;

	if(perr == 0)
	{
		FileList *pFileList = NULL;
		std::list<FileList>::iterator iter = pOpenFile->fileLists_.begin();
		while(iter != pOpenFile->fileLists_.end())
		{
			FileList &cur = *iter;
			if(cur.listId_ == listId)
			{
				pOpenFile->fileLists_.erase(iter);
				op->Complete(perr);
				return;
			}
			iter++;
		}
	}
	op->Complete(perr);
}

void CCALL PFMLayer::Read(PfmMarshallerReadOp* op, void* formatterUse)
{
	int64_t openId = op->OpenId();
	uint64_t fileOffset = op->FileOffset();
	void* data = op->Data();
	size_t requestedSize = op->RequestedSize();
	int perr = 0;
	size_t actualSize = 0;

	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		perr = pfmErrorFailed;
	if((perr == 0) && (!pOpenFile->isFile_))
		perr = pfmErrorNotAFile;
	if((perr == 0) && (pOpenFile->isDeleted_))
		perr = pfmErrorDeleted;

	if(perr == 0)
	{
		boost::shared_ptr<FileNode> fileNode = pOpenFile->fileNode_;
		if(!fileNode)
			perr = pfmErrorFailed;
		else
		{
			try
			{
				ssize_t actSize = fileNode->read(static_cast<efs_off_t>(fileOffset), reinterpret_cast<unsigned char *>(data), requestedSize);
				actualSize = actSize;
			}
			catch(rlog::Error &err)
			{
				reportEncFSMPErr(L"Error during read operation", pOpenFile->pathName_, err);
				perr = pfmErrorFailed;
			}
		}
	}
	op->Complete(perr, actualSize);
}

void CCALL PFMLayer::Write(PfmMarshallerWriteOp* op, void* formatterUse)
{
	int64_t openId = op->OpenId();
	uint64_t fileOffset = op->FileOffset();
	const void* data = op->Data();
	size_t requestedSize = op->RequestedSize();
	int perr = 0;
	size_t actualSize = 0;

	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		perr = pfmErrorFailed;
	if((perr == 0) && (!pOpenFile->isFile_))
		perr = pfmErrorNotAFile;
	if((perr == 0) && (pOpenFile->isDeleted_))
		perr = pfmErrorDeleted;

	if(perr == 0)
	{
		boost::shared_ptr<FileNode> fileNode = pOpenFile->fileNode_;
		if(!fileNode)
			perr = pfmErrorFailed;
		else
		{
			try
			{
				bool isOK = fileNode->write(static_cast<efs_off_t>(fileOffset), const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(data)),
					requestedSize);
				if(isOK)
					actualSize = requestedSize;
				else
					actualSize = 0;	// We don't have more information
				if(!isOK)
					perr = pfmErrorFailed;
			}
			catch(rlog::Error &err)
			{
				reportEncFSMPErr(L"Error during write operation", pOpenFile->pathName_, err);
				perr = pfmErrorFailed;
			}
			if(perr == 0)
			{
				// Size and/or last write time has changed, forget cached file stat
				fileStatCache_.forgetCachedStat(fileNode->cipherName());
			}
		}
	}
	op->Complete(perr, actualSize);
}

void CCALL PFMLayer::SetSize(PfmMarshallerSetSizeOp* op, void* formatterUse)
{
	int64_t openId = op->OpenId();
	uint64_t fileSize = op->FileSize();
	int perr = 0;

	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		perr = pfmErrorFailed;
	if((perr == 0) && (!pOpenFile->isFile_))
		perr = pfmErrorNotAFile;
	if((perr == 0) && (pOpenFile->isDeleted_))
		perr = pfmErrorDeleted;

	if(perr == 0)
	{
		boost::shared_ptr<FileNode> fileNode = pOpenFile->fileNode_;
		if(!fileNode)
			perr = pfmErrorFailed;
		else
		{
			try
			{
				int retVal = fileNode->truncate(static_cast<efs_off_t>(fileSize));
				if(retVal < 0)
					perr = pfmErrorFailed;
			}
			catch( rlog::Error &err )
			{
				reportEncFSMPErr(L"Error during SetSize operation", pOpenFile->pathName_, err);
				perr = pfmErrorFailed;
			}

			if(perr == 0)
			{
				// Size has changed, forget cached file stat
				fileStatCache_.forgetCachedStat( fileNode->cipherName() );

				pOpenFile->fileSize_ = fileSize;
			}
		}
	}
	op->Complete(perr);
}

void CCALL PFMLayer::Capacity(PfmMarshallerCapacityOp* op, void* formatterUse)
{
	uint64_t totalCapacity = 0;
	uint64_t availableCapacity = 0;
	int perr = 0;

	// Retrieve absolute path of encrypted directory
	if(!rootFS_)
		perr = pfmErrorFailed;
	else
	{
		std::string rootDir = rootFS_->root->rootDirectory();

		uint64_t totalCapacityLocal, availableCapacityLocal;
		if(!fs_layer::capacity(rootDir, totalCapacityLocal, availableCapacityLocal))
			perr = pfmErrorInvalid;

		totalCapacity = totalCapacityLocal;
		availableCapacity = availableCapacityLocal;
	}
	op->Complete(perr, totalCapacity, availableCapacity);
}

void CCALL PFMLayer::FlushMedia(PfmMarshallerFlushMediaOp* op, void* formatterUse)
{
	op->Complete(pfmErrorSuccess, -1/*msecFlushDelay*/);
}

void CCALL PFMLayer::Control(PfmMarshallerControlOp* op, void* formatterUse)
{
	op->Complete(pfmErrorInvalid, 0/*outputSize*/);
}

void CCALL PFMLayer::MediaInfo(PfmMarshallerMediaInfoOp* op, void* formatterUse)
{
	PfmMediaInfo mediaInfo = zeroMediaInfo;
	std::wstring mediaLabel = mountName_;

	op->Complete(pfmErrorSuccess, &mediaInfo, mediaLabel.c_str());
}

void CCALL PFMLayer::Access(PfmMarshallerAccessOp* op, void* formatterUse)
{
	int64_t openId = op->OpenId();
	int8_t accessLevel = op->AccessLevel();

	int perr = 0;
	PfmOpenAttribs openAttribs = zeroOpenAttribs;

	OpenFile *pOpenFile = getOpenFile(openId);
	if(pOpenFile == NULL)
		perr = pfmErrorFailed;
	else
	{
#if !defined(PFM_ACCESS_LEVEL_WORKAROUND)
		// Check for access level and fail if source file is not writable
		if(accessLevel >= pfmAccessLevelWriteData)
		{
			if(pOpenFile->isReadOnly_)
				perr = pfmErrorAccessDenied;

			if((perr == 0) && (pOpenFile->isOpenedReadOnly_))
			{
				// Try to reopen file with write access
				pOpenFile->fileNode_.reset();

				int res = 0;
				pOpenFile->fileNode_ =
					rootFS_->root->openNode(pOpenFile->pathName_.c_str(), "open", makeOpenFileFlags(accessLevel), &res);
				if(!pOpenFile->fileNode_)
				{
					// Failed, try to open again for reading
					pOpenFile->fileNode_ =
						rootFS_->root->openNode(pOpenFile->pathName_.c_str(), "open", makeOpenFileFlags(pfmAccessLevelReadData), &res);
					perr = pfmErrorAccessDenied;
				}
				if(perr == 0)
					pOpenFile->isOpenedReadOnly_ = false;
			}
		}
#endif

		if(perr == 0)
		{
			pOpenFile->sequenceId_++;

			openAttribs.openId = pOpenFile->openId_;
			openAttribs.openSequence = pOpenFile->sequenceId_;
			openAttribs.accessLevel = ((pOpenFile->isReadOnly_ || pOpenFile->isOpenedReadOnly_) ? pfmAccessLevelDelete : pfmAccessLevelWriteData);

			if(pOpenFile->isFile_)
				openAttribs.attribs.fileType = pfmFileTypeFile;
			else
				openAttribs.attribs.fileType = pfmFileTypeFolder;
			openAttribs.attribs.fileFlags = pOpenFile->fileFlags_;
			openAttribs.attribs.fileId = pOpenFile->fileId_;
			openAttribs.attribs.fileSize = pOpenFile->fileSize_;

			openAttribs.attribs.accessTime = pOpenFile->accessTime_;
			openAttribs.attribs.createTime = pOpenFile->createTime_;
			openAttribs.attribs.writeTime = pOpenFile->writeTime_;
			openAttribs.attribs.changeTime = pOpenFile->changeTime_;
		}
	}
	op->Complete(perr, &openAttribs, 0);
}

void CCALL PFMLayer::ReadXattr(PfmMarshallerReadXattrOp* op, void* formatterUse)
{
	op->Complete(pfmErrorNotFound, 0/*xattrSize*/, 0/*transferredSize*/);
}

void CCALL PFMLayer::WriteXattr(PfmMarshallerWriteXattrOp* op, void* formatterUse)
{
	op->Complete(pfmErrorAccessDenied, 0/*transferredSize*/);
}

int64_t PFMLayer::getFileID(const std::string &path)
{
	FileIDMap::const_iterator iter = fileIDs_.find(path);
	if(iter == fileIDs_.end())
		return -1;

	return iter->second;
}

int64_t PFMLayer::addFileID(const std::string &path)
{
	int64_t newID = newFileID_++;
	fileIDs_[path] = newID;

/*
	std::stringstream ostr;
	ostr << path.c_str() << ": " << newFileID.fileID_ << std::endl;
	std::string debugString = ostr.str();
#if defined(_WIN32)
	OutputDebugStringA(debugString.c_str());
#endif
*/
	return newID;
}

bool PFMLayer::renameFileID(int64_t fileId, const std::string &newpath)
{
	FileIDMap::iterator iter = fileIDs_.begin();
	while(iter != fileIDs_.end())
	{
		int64_t curFileId = iter->second;
		if(curFileId == fileId)
		{
			fileIDs_.erase(iter);
			fileIDs_[newpath] = fileId;
			return true;
		}

		iter++;
	}

	return false;
}

bool PFMLayer::deleteFileID(int64_t fileId)
{
	FileIDMap::iterator iter = fileIDs_.begin();
	while(iter != fileIDs_.end())
	{
		int64_t curFileId = iter->second;
		if(curFileId == fileId)
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
	OpenFileMapType::iterator iter = openFiles_.find(openId);
	if(iter != openFiles_.end())
		return &(iter->second);

	return NULL;
}

PFMLayer::OpenFile *PFMLayer::findOpenFileByName(const std::string &path)
{
	OpenIdMapType::iterator iter = openIdMap_.find(path);
	if(iter != openIdMap_.end())
	{
		return getOpenFile(iter->second);
	}
	return NULL;
}

int64_t PFMLayer::createFileId(const std::string &fn)
{
	int64_t fileId = getFileID(fn);
	if(fileId < 0)	// not found
	{
		return addFileID(fn);
	}

	return fileId;
}

void PFMLayer::createEndName(std::wstring &endName, const char *fullPathName)
{
	// Convert filename from UTF8 to UTF16
	std::string fileName = fs_layer::extract_filename(std::string(fullPathName));
	endName = boost::locale::conv::utf_to_utf<wchar_t>(fileName.c_str());
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

			// Before actually creating the file, delete entry in fileStatCache with same name
			std::string cipherPath = rootFS_->root->cipherPath(path.c_str());
			fileStatCache_.forgetCachedStat( cipherPath.c_str() );


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
			of.isOpenedReadOnly_ = (accessLevel < pfmAccessLevelWriteData);
			of.isReadOnly_ = false;
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

			addOpenFile(of);

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

			// Before actually creating the folder, delete entry in fileStatCache with same name
			std::string cipherPath = rootFS_->root->cipherPath(path.c_str());
			fileStatCache_.forgetCachedStat( cipherPath.c_str() );

			rootFS_->root->mkdir( path.c_str(), mode);

			OpenFile of;
			of.isFile_ = false;
			of.openId_ = newCreateOpenId;
			of.sequenceId_ = 1;
			of.pathName_ = path;
			of.isOpenedReadOnly_ = (accessLevel < pfmAccessLevelWriteData);
			of.isReadOnly_ = false;
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

			addOpenFile(of);

			// Apply writeTime
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
	int openFlags = makeOpenFileFlags(pOpenFile->isReadOnly_ || pOpenFile->isOpenedReadOnly_);

	try
	{
		bool reopen = false;
		if(pOpenFile->fileNode_)	// Close file
		{
			reopen = true;
			pOpenFile->fileNode_.reset();
		}
		
		std::string oldCipherPath = rootFS_->root->cipherPath(pOpenFile->pathName_.c_str());
		fileStatCache_.forgetCachedStat( oldCipherPath.c_str() );
		std::string newCipherPath = rootFS_->root->cipherPath(newPath.c_str());
		fileStatCache_.forgetCachedStat( newCipherPath.c_str() );

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

		if(!renameOpenFile(pOpenFile, newPath))
			return pfmErrorInvalid;
		if(!renameFileID(pOpenFile->fileId_, newPath))
			return pfmErrorInvalid;

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
	accessLevel = determineAccessLevel(pOpenFile->isReadOnly_ || pOpenFile->isOpenedReadOnly_, accessLevel);

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
		int err = fileNode->getAttr(&buf, &fileStatCache_);
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
	of.isOpenedReadOnly_ = (accessLevel < pfmAccessLevelWriteData);
	of.fileId_ = createFileId(path);

	accessLevel = determineAccessLevel(of.isReadOnly_ || of.isOpenedReadOnly_, accessLevel);

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

	addOpenFile(of);

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
		int statOk = fileStatCache_.stat(cipherPath.c_str(), &buf);
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
	of.isOpenedReadOnly_ = (accessLevel < pfmAccessLevelWriteData);
	of.fileId_ = createFileId(path);

	accessLevel = determineAccessLevel(of.isReadOnly_ || of.isOpenedReadOnly_, accessLevel);

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

	addOpenFile(of);

	return 0;
}

int PFMLayer::makeOpenFileFlags(PT_INT8 accessLevel)
{
#if defined(PFM_ACCESS_LEVEL_WORKAROUND)
	accessLevel = pfmAccessLevelWriteData;
#endif

#if defined(_WIN32)
	int flags = O_BINARY;
#else
	int flags = 0;
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
	int flags = O_BINARY;
#else
	int flags = 0;
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
	OpenFileMapType::iterator iter = openFiles_.begin();
	while(iter != openFiles_.end())
	{
		OpenFile &cur = iter->second;
		
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

void PFMLayer::addOpenFile(const OpenFile &of)
{
	openFiles_[of.openId_] = of;
	openIdMap_[of.pathName_] = of.openId_;
}

bool PFMLayer::renameOpenFile(OpenFile *pOpenFile, const std::string &newPath)
{
	OpenIdMapType::iterator iter = openIdMap_.find(pOpenFile->pathName_);
	if(iter == openIdMap_.end())
		return false;
	openIdMap_.erase(iter);	// Erase old pathname from openIdMap_
	pOpenFile->pathName_ = newPath;
	openIdMap_[newPath] = pOpenFile->openId_;

	return true;
}
