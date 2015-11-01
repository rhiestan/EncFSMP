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

#include <wx/version.h>

#if wxCHECK_VERSION(2, 9, 0) && defined(_WIN32)
#define HAVE_MODE_T		// Workaround for double defined mode_t on Windows
#endif

// Unfortunately, libencfs and wxWidgets have incompatible ssize_t definitions in 32 bits
// Workaround:
#include "config.h"
#define HAVE_SSIZE_T

#include "CommonIncludes.h"


#include "config.h"

#include "EncFSUtilities.h"

// libencfs
#include "encfs.h"

#include "FileUtils.h"
#include "ConfigReader.h"
#include "FSConfig.h"

#include "DirNode.h"
#include "FileNode.h"
#include "Cipher.h"
#include "StreamNameIO.h"
#include "BlockNameIO.h"
#include "NullNameIO.h"
#include "Context.h"

// boost
#include <boost/locale.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>

// Unfortunately, my version of libencfs is a tiny bit newer than 1.7.4 on Linux.
// This define restores full compatibility
#define EFS_COMPATIBILITY_WORKAROUND 1

EncFSUtilities::EncFSUtilities()
{
}

EncFSUtilities::~EncFSUtilities()
{
}

const int V6SubVersion = 20100713;	// Not optimal just to copy the data, but what can you do?

bool EncFSUtilities::createEncFS(const wxString &encFSPath, const wxString &password,
	const wxString &externalConfigFileName, bool useExternalConfigFile,
	const wxString &cipherAlgorithm, long cipherKeySize, long cipherBlockSize,
	const wxString &nameEncoding, long keyDerivationDuration,
	bool perBlockHMAC, bool uniqueIV, bool chainedIV, bool externalIV)
{
#if wxCHECK_VERSION(2, 9, 0)
	std::string cipherAlgo = cipherAlgorithm.ToStdString();
#else
	std::string cipherAlgo(cipherAlgorithm.mb_str());
#endif

	boost::shared_ptr<Cipher> cipher = Cipher::New( cipherAlgo, cipherKeySize );
	if(!cipher)
		return false;

#if wxCHECK_VERSION(2, 9, 0)
	std::string nameEncodingStdStr = nameEncoding.ToStdString();
#else
	std::string nameEncodingStdStr(nameEncoding.mb_str());
#endif
	rel::Interface nameIOIface = BlockNameIO::CurrentInterface();
	NameIO::AlgorithmList algorithms = NameIO::GetAlgorithmList();
	NameIO::AlgorithmList::const_iterator it;
	for(it = algorithms.begin(); it != algorithms.end(); ++it)
	{
		if(it->name == nameEncodingStdStr)
			nameIOIface = it->iface;
	}

#if defined(EFS_COMPATIBILITY_WORKAROUND)
	if(nameIOIface.current() == 4)
		nameIOIface.current() = 3;
#endif

	int blockMACBytes = 0;
	int blockMACRandBytes = 0;
	if(perBlockHMAC)
		blockMACBytes = 8;
	bool allowHoles = true;

	boost::shared_ptr<EncFSConfig> config( new EncFSConfig );

	config->cfgType = Config_V6;
	config->cipherIface = cipher->get_interface();
	config->keySize = cipherKeySize;
	config->blockSize = cipherBlockSize;
	config->nameIface = nameIOIface;
	config->creator = "EncFS " EFS_STRINGIFY(VERSION);
	config->subVersion = V6SubVersion;
	config->blockMACBytes = blockMACBytes;
	config->blockMACRandBytes = blockMACRandBytes;
	config->uniqueIV = uniqueIV;
	config->chainedNameIV = chainedIV;
	config->externalIVChaining = externalIV;
	config->allowHoles = allowHoles;

	config->salt.clear();
	config->kdfIterations = 0; // filled in by keying function
	config->desiredKDFDuration = keyDerivationDuration;

	int encodedKeySize = cipher->encodedKeySize();
	unsigned char *encodedKey = new unsigned char[ encodedKeySize ];
	CipherKey volumeKey = cipher->newRandomKey();

	// get user key and use it to encode volume key
	CipherKey userKey = config->getUserKey(std::string(password.mb_str()), "");

	cipher->writeKey( volumeKey, encodedKey, userKey );
	userKey.reset();

	config->assignKeyData(encodedKey, encodedKeySize);
	delete[] encodedKey;

	if(!volumeKey)
	{
		return false;
	}

	std::string rootDir = EncFSUtilities::wxStringToEncFSPath(encFSPath);
	std::string externalConfigFileNameStr = EncFSUtilities::wxStringToEncFSFile(externalConfigFileName);

	if(!saveConfig( Config_V6, rootDir, config, useExternalConfigFile, externalConfigFileNameStr ))
		return false;

	return true;
}

bool EncFSUtilities::getEncFSInfo(const wxString &encFSPath, const wxString &externalConfigFileName,
		bool useExternalConfigFile, EncFSInfo &info)
{
	std::string rootDir = wxStringToEncFSPath(encFSPath);
	std::string externalConfigFileNameStr = wxStringToEncFSFile(externalConfigFileName);
	boost::shared_ptr<EncFSConfig> config(new EncFSConfig);

	ConfigType type = readConfig( rootDir, config, useExternalConfigFile, externalConfigFileNameStr );
	switch(type)
	{
	case Config_None:
		info.configVersionString = wxT("Unable to load or parse config file");
		return false;
	case Config_Prehistoric:
		info.configVersionString = wxT("A really old EncFS filesystem was found. \n")
			wxT("It is not supported in this EncFS build.");
		return false;
	case Config_V3:
		info.configVersionString = wxT("Version 3 configuration; ")
			wxT("created by ") + wxString(config->creator.c_str(), *wxConvCurrent);
		break;
	case Config_V4:
		info.configVersionString = wxT("Version 4 configuration; ")
			wxT("created by ") + wxString(config->creator.c_str(), *wxConvCurrent);
		break;
	case Config_V5:
		info.configVersionString = wxT("Version 5 configuration; ")
			wxT("created by ") + wxString(config->creator.c_str(), *wxConvCurrent)
			+ wxString::Format(wxT(" (revision %i)"), config->subVersion);
		break;
	case Config_V6:
		info.configVersionString = wxT("Version 6 configuration; ")
			wxT("created by ") + wxString(config->creator.c_str(), *wxConvCurrent)
			+ wxString::Format(wxT(" (revision %i)"), config->subVersion);
		break;
	}

	boost::shared_ptr<Cipher> cipher = Cipher::New( config->cipherIface, -1 );
	info.cipherAlgorithm = wxString(config->cipherIface.name().c_str(), *wxConvCurrent);
	if(!cipher)
		info.cipherAlgorithm.Append(wxT(" (NOT supported)"));
	info.cipherKeySize = config->keySize;
	info.cipherBlockSize = config->blockSize;

	// check if we support the filename encoding interface..
	boost::shared_ptr<NameIO> nameCoder = NameIO::New( config->nameIface,
		cipher, CipherKey() );
	info.nameEncoding = wxString(config->nameIface.name().c_str(), *wxConvCurrent);
	if(!nameCoder)
		info.nameEncoding.Append(wxT(" (NOT supported)"));

	info.keyDerivationIterations = config->kdfIterations;
	info.saltSize = config->salt.size();
	info.perBlockHMAC = (config->blockMACBytes > 0);
	info.uniqueIV = config->uniqueIV;
	info.chainedIV = config->chainedNameIV;
	info.externalIV = config->externalIVChaining;
	info.allowHoles = config->allowHoles;

	return true;
}

bool EncFSUtilities::changePassword(const wxString &encFSPath,
	const wxString &externalConfigFileName, bool useExternalConfigFile,
	const wxString &oldPassword, const wxString &newPassword, wxString &errorMsg)
{
	std::string rootDir = wxStringToEncFSPath(encFSPath);
	std::string externalConfigFileNameStr = wxStringToEncFSFile(externalConfigFileName);

	boost::shared_ptr<EncFSConfig> config(new EncFSConfig);
	ConfigType cfgType = readConfig( rootDir, config, useExternalConfigFile, externalConfigFileNameStr );
	if(cfgType == Config_None)
	{
		errorMsg = wxT("Unable to load or parse config file");
		return false;
	}

	shared_ptr<Cipher> cipher = Cipher::New( config->cipherIface, config->keySize );
    if(!cipher)
    {
		errorMsg = wxT("Unable to find specified cipher \"");
		errorMsg.Append(wxString(config->cipherIface.name().c_str(), *wxConvCurrent));
		errorMsg.Append(wxT("\""));
		return false;
    }

	CipherKey userKey = config->getUserKey(std::string(oldPassword.mb_str()), "");

	// decode volume key using user key -- at this point we detect an incorrect
	// password if the key checksum does not match (causing readKey to fail).
	CipherKey volumeKey = cipher->readKey( config->getKeyData(), userKey );

	if(!volumeKey)
	{
		errorMsg = wxT("Invalid old password");
		return false;
	}

	userKey.reset();
	// reinitialize salt and iteration count
	config->kdfIterations = 0; // generate new

	userKey = config->getUserKey(std::string(newPassword.mb_str()), "");

	// re-encode the volume key using the new user key and write it out..
	bool isOK = false;
	if(userKey)
	{
		int encodedKeySize = cipher->encodedKeySize();
		unsigned char *keyBuf = new unsigned char[ encodedKeySize ];

		// encode volume key with new user key
		cipher->writeKey( volumeKey, keyBuf, userKey );
		userKey.reset();

		config->assignKeyData( keyBuf, encodedKeySize );
		delete [] keyBuf;

		if(saveConfig( cfgType, rootDir, config, useExternalConfigFile, externalConfigFileNameStr ))
		{
			// password modified -- changes volume key of filesystem..
			errorMsg = wxT("Volume Key successfully updated.");
			isOK = true;
		}
		else
		{
			errorMsg = wxT("Error saving modified config file.");
		}
	}
	else
	{
		errorMsg = wxT("Error creating key.");
	}

	volumeKey.reset();

	return isOK;
}

static bool exportFile(const boost::shared_ptr<EncFS_Root> &rootInfo, 
	std::string encfsName, std::string targetName)
{
	boost::shared_ptr<FileNode> node = 
		rootInfo->root->lookupNode( encfsName.c_str(), "EncFSMP" );
	if(!node)
		return false;

	efs_stat st;
	if(node->getAttr(&st, 0) != 0)
		return false;

	if(node->open(O_RDONLY) < 0)
		return false;

	int fd = fs_layer::creat(targetName.c_str(), st.st_mode);

	const int blockSize = 512;
	int blocks = (node->getSize() + blockSize-1) / blockSize;
	boost::scoped_array<unsigned char> buf(new unsigned char[blockSize]);

	for(int i = 0; i < blocks; i++)
	{
		int writeRet = 0;
		ssize_t readBytes = node->read(i * blockSize, buf.get(), blockSize);
		if(readBytes > 0)
			writeRet = fs_layer::write(fd, buf.get(), static_cast<unsigned int>(readBytes));

		if(writeRet < 0)
		{
			fs_layer::close(fd);
			return false;
		}
	}
	fs_layer::close(fd);

	return true;
}

static bool exportDir(const boost::shared_ptr<EncFS_Root> &rootInfo, 
	std::string volumeDir, std::string destDir)
{
	// Create destination directory with the same permissions as original
	{
		efs_stat st;
		boost::shared_ptr<FileNode> dirNode = 
			rootInfo->root->lookupNode( volumeDir.c_str(), "EncFSMP" );
		if(dirNode->getAttr(&st, 0))
			return false;

		fs_layer::mkdir(destDir.c_str(), st.st_mode);
	}

	// Traverse directory
	DirTraverse dt = rootInfo->root->openDir(volumeDir.c_str());
	if(dt.valid())
	{
		std::string name = dt.nextPlaintextName();
		while(!name.empty())
		{
			// Recurse to subdirectories
			if(name != "." && name != "..")
			{
				std::string plainPath = volumeDir + name;
				std::string cpath = rootInfo->root->cipherPath(plainPath.c_str());
				std::string destName = destDir + name;

				bool retVal = true;
				efs_stat stBuf;
				if( !fs_layer::lstat( cpath.c_str(), &stBuf ))
				{
					if( S_ISDIR( stBuf.st_mode ) )
					{
						// Recursive call
						exportDir(rootInfo, (plainPath + '/'), 
							destName + '/');
					}
					else if( S_ISREG( stBuf.st_mode ) )
					{
						retVal = exportFile(rootInfo, plainPath, destName);
					}
				}
				else
				{
					retVal = false;
				}

				if(!retVal)
					return retVal;
			}

			name = dt.nextPlaintextName();
		}
	}
	return true;
}

bool EncFSUtilities::exportEncFS(const wxString &encFSPath,
	const wxString &externalConfigFileName, bool useExternalConfigFile,
	const wxString &password, const wxString &exportPath, wxString &errorMsg)
{
	std::string rootDir = wxStringToEncFSPath(encFSPath);
	std::ostringstream ostr;

	shared_ptr<EncFS_Opts> opts( new EncFS_Opts() );
	opts->rootDir = rootDir;
	opts->createIfNotFound = false;
	opts->checkKey = false;
	opts->passwordProgram = std::string(password.mb_str());
	opts->externalConfigFileName = EncFSUtilities::wxStringToEncFSFile(externalConfigFileName);
	opts->useExternalConfigFile = useExternalConfigFile;
	RootPtr rootInfo = initFS( NULL, opts, ostr );

	if(!rootInfo)
	{
		if(ostr.str().length() == 0)
			ostr << "No encrypted filesystem found";

		std::string errMsg1 = ostr.str();
		errorMsg = wxString(errMsg1.c_str(), *wxConvCurrent);
		return false;
	}

	std::string destDir = wxStringToEncFSPath(exportPath);

	return exportDir(rootInfo, "/", destDir);
}

std::string EncFSUtilities::wxStringToEncFSPath(const wxString &path)
{
	std::wstring pathUTF16(path.c_str());
#if defined(EFS_WIN32)
	// Replace all \ with /
	//boost::replace_all(pathUTF16, L"\\", L"/");
	for(size_t i = 0; i < pathUTF16.length(); i++)
	{
		if(pathUTF16[i] == L'\\')
			pathUTF16[i] = L'/';
	}
#endif

	// Make sure path has trailing separator
	wxString separators = wxFileName::GetPathSeparators(wxPATH_NATIVE);
	wchar_t lastChar = pathUTF16[pathUTF16.size() - 1];
	if(separators.Find(lastChar) == wxNOT_FOUND)
		pathUTF16.push_back((wchar_t)(wxFileName::GetPathSeparator(wxPATH_UNIX)));

	std::string pathUTF8 = boost::locale::conv::utf_to_utf<char>(pathUTF16.c_str());

	return pathUTF8;
}

std::string EncFSUtilities::wxStringToEncFSFile(const wxString &path)
{
	std::wstring pathUTF16(path.c_str());
#if defined(EFS_WIN32)
	// Replace all \ with /
	//boost::replace_all(pathUTF16, L"\\", L"/");
	for(size_t i = 0; i < pathUTF16.length(); i++)
	{
		if(pathUTF16[i] == L'\\')
			pathUTF16[i] = L'/';
	}
#endif

	std::string pathUTF8 = boost::locale::conv::utf_to_utf<char>(pathUTF16.c_str());

	return pathUTF8;
}