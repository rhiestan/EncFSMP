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

#include "CommonIncludes.h"
#include "version.h"

#include "EncFSMPApp.h"
#include "EncFSMPMainFrame.h"
#include <wx/cmdline.h>
#include <wx/snglinst.h>

#include "PFMProxy.h"
#include "OpenSSLProxy.h"

// Main program equivalent, creating windows and returning main app frame
bool EncFSMPApp::OnInit()
{
	wxString appName(wxT(ENCFSMP_NAME));
#if wxCHECK_VERSION(2, 9, 0)
	SetAppDisplayName(appName);
#endif
	appName.Replace(wxT(" "), wxEmptyString, true);
	SetAppName(appName);

	// Make sure only one instance is running
#if wxCHECK_VERSION(2, 9, 0)
	pSingleInstanceChecker_ = new wxSingleInstanceChecker();
#else
	const wxString appname = wxString::Format(wxT(ENCFSMP_NAME) wxT("-%s"), wxGetUserId().c_str());
    pSingleInstanceChecker_ = new wxSingleInstanceChecker(appname);
#endif

	if(pSingleInstanceChecker_->IsAnotherRunning())
	{
		wxMessageBox(wxT("Another instance of " ) wxT(ENCFSMP_NAME) wxT(" is already running."),
			wxT(ENCFSMP_NAME), wxOK | wxICON_HAND);
		delete pSingleInstanceChecker_;
		pSingleInstanceChecker_ = NULL;
		return false;
	}

	wxInitAllImageHandlers();

	PFMProxy::getInstance().initialize();
	OpenSSLProxy::initialize();

	pMainFrame_ = new EncFSMPMainFrame(NULL);

	// Show the frame
	pMainFrame_->Show(true);

	SetTopWindow(pMainFrame_);

	// Monitor thread needs top window
	PFMProxy::getInstance().startMonitorThread();

	return true;
}

int EncFSMPApp::OnExit()
{
	OpenSSLProxy::uninitialize();
	PFMProxy::getInstance().uninitialize();

	delete pSingleInstanceChecker_;

	return 0;
}

IMPLEMENT_APP(EncFSMPApp)
