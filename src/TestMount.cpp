/**
* Copyright (C) 2026 Roman Hiestand
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

#include "TestMount.h"
#include "PFMProxy.h"
#include "EncFSMPStrings.h"
#include "EncFSUtilities.h"

#if wxCHECK_VERSION(2, 9, 0) && defined(_WIN32)
#define HAVE_MODE_T		// Workaround for double defined mode_t on Windows
#endif

#include "pfm_layer.h"

// libencfs
#include "FileUtils.h"
#include "Context.h"

#include <boost/filesystem.hpp>

namespace
{
	const wxString testMountName_(wxT("EncFSMPTest"));
	// Not a real secret: only ever used for a throwaway, temporary volume
	// created and destroyed within a single test run.
	const wxString testPassword_(wxT("EncFSMPTest-9f3c7ad1"));

	boost::filesystem::path volumePath_;
	encfs::RootPtr rootFS_;
	std::unique_ptr<encfs::EncFS_Context> context_;
	std::ostringstream mountOstr_;
	bool mounted_ = false;

	// Runs PFMLayer::startFS(), which blocks until unmount or driver
	// disconnect. Mirrors PFMHandlerThread::Entry(), minus the GUI
	// error-reporting callback (mountOstr_ carries any error message instead).
	class TestMountThread : public wxThread
	{
	public:
		explicit TestMountThread(wchar_t driveLetter)
			: wxThread(wxTHREAD_JOINABLE), driveLetter_(driveLetter)
		{
		}

	protected:
		virtual wxThread::ExitCode Entry() wxOVERRIDE
		{
			PFMLayer pfm;
			pfm.startFS(rootFS_, testMountName_.c_str(), driveLetter_,
				false, false, true, false, mountOstr_);
			return 0;
		}

	private:
		wchar_t driveLetter_;
	};

	TestMountThread *pMountThread_ = NULL;

	// Polls the FSR mount list for our mount to reach fsrStatusFlagReady, the
	// same signal PFMMonitorThread uses in the real app. The FSR mount-list
	// change pin can't be used instead: see PFMMonitorThread.cpp for why.
	bool waitForMountReady(int timeoutMSecs)
	{
		const int pollIntervalMSecs = 100;
		for(int waited = 0; waited < timeoutMSecs; waited += pollIntervalMSecs)
		{
			FsrMalloc_i fsrMalloc;
			FsrMountInfo** miList = NULL;
			PT_SIZE_T miCount = PFMProxy::getInstance().fsrMountGetList(&fsrMalloc, &miList, NULL, NULL);

			for(PT_SIZE_T i = 0; i < miCount; ++i)
			{
				FsrMountInfo *mi = miList[i];
				if(mi == NULL || mi->formatterName == NULL
					|| std::string(mi->formatterName) != EncFSMPStrings::formatterName8_
					|| mi->mountSourceName == NULL)
					continue;

				// Same normalization unmount() uses to match a mount by name.
				wxString sourceNameStr(mi->mountSourceName, wxConvUTF8);
				if(wxFileName(sourceNameStr).GetFullName() != testMountName_)
					continue;

				if((mi->statusFlags & fsrStatusFlagReady) != 0)
					return true;
			}

			wxMilliSleep(pollIntervalMSecs);
		}
		return false;
	}
}

bool TestMount::mount(const wxString &mountPath, wxString &errorMsg)
{
	if(!PFMProxy::getInstance().isPFMPresent())
	{
		errorMsg = wxT("Pismo File System Redirector is not available");
		return false;
	}

	wchar_t driveLetter = !mountPath.IsEmpty() ? static_cast<wchar_t>(mountPath[0]) : L'Z';

	volumePath_ = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	boost::system::error_code ec;
	if(!boost::filesystem::create_directories(volumePath_, ec))
	{
		errorMsg = wxT("Unable to create a temporary directory for the test volume");
		return false;
	}

	wxString encFSPath(volumePath_.wstring().c_str());
	// Small key derivation duration to keep startup fast; never use this for
	// a real volume.
	if(!EncFSUtilities::createEncFS(encFSPath, testPassword_,
		wxEmptyString, false,
		wxT("AES"), 256, 4096, wxT("Block"), 10,
		false, true, true, false))
	{
		errorMsg = wxT("Unable to create the temporary test volume");
		boost::filesystem::remove_all(volumePath_, ec);
		return false;
	}

	std::shared_ptr<encfs::EncFS_Opts> opts(new encfs::EncFS_Opts());
	opts->rootDir = EncFSUtilities::wxStringToEncFSPath(encFSPath);
	opts->createIfNotFound = false;
	opts->checkKey = true;
	opts->passwordProgram = std::string(testPassword_.mb_str());	// Abusing this parameter, see EncFSUtilities.cpp
	opts->useExternalConfigFile = false;

	context_.reset(new encfs::EncFS_Context());
	rootFS_ = initFS(context_.get(), opts, mountOstr_);
	if(!rootFS_)
	{
		errorMsg = wxT("Unable to initialize the test volume: ")
			+ wxString(mountOstr_.str().c_str(), *wxConvCurrent);
		context_.reset();
		boost::filesystem::remove_all(volumePath_, ec);
		return false;
	}

	pMountThread_ = new TestMountThread(driveLetter);
	if(pMountThread_->Create() != wxTHREAD_NO_ERROR
		|| pMountThread_->Run() != wxTHREAD_NO_ERROR)
	{
		errorMsg = wxT("Unable to start the mount thread");
		delete pMountThread_;
		pMountThread_ = NULL;
		rootFS_.reset();
		context_.reset();
		boost::filesystem::remove_all(volumePath_, ec);
		return false;
	}

	if(!waitForMountReady(20000))
	{
		errorMsg = wxT("Timed out waiting for the test volume to mount");
		unmount();
		return false;
	}

	mounted_ = true;
	return true;
}

void TestMount::unmount()
{
	if(mounted_)
	{
		// Causes the blocked PFMLayer::startFS() call in the mount thread to
		// return.
		PFMProxy::getInstance().unmount(testMountName_);
		mounted_ = false;
	}

	if(pMountThread_ != NULL)
	{
		pMountThread_->Wait();
		delete pMountThread_;
		pMountThread_ = NULL;
	}

	// Only safe to tear down now that the mount thread (which reads rootFS_
	// for as long as it runs) has actually finished.
	rootFS_.reset();
	context_.reset();

	if(!volumePath_.empty())
	{
		boost::system::error_code ec;
		boost::filesystem::remove_all(volumePath_, ec);
		volumePath_.clear();
	}
}
