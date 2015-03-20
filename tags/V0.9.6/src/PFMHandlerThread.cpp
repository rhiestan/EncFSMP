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

#include "PFMHandlerThread.h"
#include "PFMProxy.h"
#include "EncFSMPMainFrame.h"
#include "EncFSUtilities.h"

#if wxCHECK_VERSION(2, 9, 0) && defined(_WIN32)
#define HAVE_MODE_T		// Workaround for double defined mode_t on Windows
#endif

#ifndef _INC_WINDOWS
#define _INC_WINDOWS
#endif

#include "pfm_layer.h"
#include "fs_layer.h"

// libencfs
#include "Cipher.h"
#include "DirNode.h"
#include "MemoryPool.h"
#include "Interface.h"
#include "FileUtils.h"
#include "StreamNameIO.h"
#include "BlockNameIO.h"
#include "NullNameIO.h"

// boost
#include <boost/locale.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>

PFMHandlerThread::PFMHandlerThread() : wxThread(wxTHREAD_DETACHED),
	worldWrite_(false), startBrowser_(true), systemVisible_(false)
{
}

PFMHandlerThread::~PFMHandlerThread()
{
}

void PFMHandlerThread::setParameters(const wxString &mountName,
	const wxString &path, const wxString &driveLetter,
	const wxString &password, bool worldWrite, bool startBrowser,
	bool systemVisible)
{
	mountName_ = mountName;
	path_ = path;
	driveLetter_ = driveLetter;
	password_ = password;
	worldWrite_ = worldWrite;
	startBrowser_ = startBrowser;
	systemVisible_ = systemVisible;
}

wxThread::ExitCode PFMHandlerThread::Entry()
{
	RootPtr rootFS;

	wxString errorMsg;
	std::ostringstream ostr;

	// Check whether path_ exists
	wxFileName fnp(path_);
	if(!fnp.DirExists())
	{
		errorMsg = wxT("EncFS path does not exist");
	}
	else
	{
		std::string pathUTF8 = EncFSUtilities::wxStringToEncFSPath(fnp.GetFullPath());
//		std::wstring passwordUTF16 = password_.c_str();
//		std::string passwordUTF8 = boost::locale::conv::utf_to_utf<char>(passwordUTF16.c_str());

		boost::shared_ptr<EncFS_Opts> opts( new EncFS_Opts() );
		opts->rootDir = pathUTF8;
		opts->createIfNotFound = false;
		opts->checkKey = true;					// If this is set to false, mounting never fails even with a wrong password
		opts->passwordProgram = std::string(password_.mb_str());	//passwordUTF8;	// Abusing this parameter here, so that it uses EncFSConfig::getUserKey with password program
		rootFS = initFS( NULL, opts, ostr );

		if(rootFS)
		{
			PFMLayer pfm;
			wchar_t driveLetterW = driveLetter_[0];
			PfmApi *pfmApi = PFMProxy::getInstance().getPfmApi();

			pfm.startFS(rootFS, mountName_.c_str(), pfmApi, driveLetterW,	// Will not return before unmount
				worldWrite_, startBrowser_, systemVisible_, ostr);
		}
		else
		{
			if(ostr.str().length() == 0)
				ostr << "No encrypted filesystem found";
		}

		// Check for errors
		std::string errMsg1 = ostr.str();
		errorMsg = wxString(errMsg1.c_str(), *wxConvCurrent);
	}
	if(errorMsg.Length() > 0)
	{
		// Send error to main frame
		EncFSMPMainFrame *pMainFrame = NULL;
		wxWindow *pTopWindow = wxTheApp->GetTopWindow();
		if(pTopWindow != NULL)
		{
			pMainFrame = dynamic_cast<EncFSMPMainFrame *>(pTopWindow);
		}
		if(pMainFrame != NULL)
		{
			pMainFrame->reportEncFSError(errorMsg, mountName_);
		}
	}

	return 0;
}
