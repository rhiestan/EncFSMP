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
#include "version.h"
#include "efs_config.h"

#include <iostream>

#include "EncFSMPApp.h"
#include "EncFSMPMainFrame.h"
#include "EncFSMPLogger.h"
#include <wx/cmdline.h>
#include <wx/snglinst.h>

#include "PFMProxy.h"
#include "OpenSSLProxy.h"

#include "easylogging++.h"

#if defined(EFS_WIN32)
#	include "EncFSMPIPCWin.h"
#else
#	include "EncFSMPIPCPosix.h"
#endif

#if defined(_MSC_VER)
#define ENCFSMP_MINIDUMP
#endif

#if defined(ENCFSMP_MINIDUMP)
// Taken from here:
// https://stackoverflow.com/questions/5028781/how-to-write-a-sample-code-that-will-crash-and-produce-dump-file
// Obviously only works on Windows

#include <Windows.h>
#include <Dbghelp.h>

void make_minidump(EXCEPTION_POINTERS* e)
{
    auto hDbgHelp = LoadLibraryA("dbghelp");
    if(hDbgHelp == nullptr)
        return;
    auto pMiniDumpWriteDump = (decltype(&MiniDumpWriteDump))GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
    if(pMiniDumpWriteDump == nullptr)
        return;

    char name[MAX_PATH];
    {
        auto nameEnd = name + GetModuleFileNameA(GetModuleHandleA(0), name, MAX_PATH);
        SYSTEMTIME t;
        GetSystemTime(&t);
        wsprintfA(nameEnd - strlen(".exe"),
            "_%4d%02d%02d_%02d%02d%02d.dmp",
            t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    }

    auto hFile = CreateFileA(name, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if(hFile == INVALID_HANDLE_VALUE)
        return;

    MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
    exceptionInfo.ThreadId = GetCurrentThreadId();
    exceptionInfo.ExceptionPointers = e;
    exceptionInfo.ClientPointers = FALSE;

    auto dumped = pMiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory),
        e ? &exceptionInfo : nullptr,
        nullptr,
        nullptr);

    CloseHandle(hFile);

    return;
}

LONG CALLBACK unhandled_handler(EXCEPTION_POINTERS* e)
{
    make_minidump(e);
    return EXCEPTION_CONTINUE_SEARCH;
}

#endif

wxBEGIN_EVENT_TABLE(EncFSMPApp, wxApp)
	EVT_END_SESSION(EncFSMPApp::OnEndSession)
	EVT_QUERY_END_SESSION(EncFSMPApp::OnQueryEndSession)
wxEND_EVENT_TABLE()

// Main program equivalent, creating windows and returning main app frame
bool EncFSMPApp::OnInit()
{
#if defined(ENCFSMP_MINIDUMP)
	SetUnhandledExceptionFilter(unhandled_handler);
#endif

	wxString appName(wxT(ENCFSMP_NAME));
#if wxCHECK_VERSION(2, 9, 0)
	SetAppDisplayName(appName);
#endif
	appName.Replace(wxT(" "), wxEmptyString, true);
	SetAppName(appName);

	// Set logging configuration
	START_EASYLOGGINGPP(argc, argv);
	EncFSMPLogger::setup();

	// Parse command line
	static const wxCmdLineEntryDesc cmdLineDesc[] =
	{
#if wxCHECK_VERSION(2, 9, 0)
		{ wxCMD_LINE_PARAM, "Command", "Command", "Command to run", wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY },
		{ wxCMD_LINE_OPTION, "m", "mount", "The name of the EncFS mount", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
		{ wxCMD_LINE_OPTION, "p", "password", "The password", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
#else
		{ wxCMD_LINE_PARAM, wxT("Command"), wxT("Command"), wxT("Command to run"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY },
		{ wxCMD_LINE_OPTION, wxT("m"), wxT("mount"), wxT("The name of the EncFS mount"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
		{ wxCMD_LINE_OPTION, wxT("p"), wxT("password"), wxT("The password"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
#endif
		{ wxCMD_LINE_NONE }
	};

	wxCmdLineParser parser(argc, argv);
	parser.SetDesc(cmdLineDesc);
	
	// Only parse command line if there is at least one argument
	wxString command, mountName, password;
	if(argc > 1)
	{
		if(parser.Parse() == 0)
		{
			wxString param;
			command = parser.GetParam();
			if(parser.Found(wxT("m"), &param))
				mountName = param;
			if(parser.Found(wxT("p"), &param))
				password = param;

			if(password == wxT("-"))
			{
				// Read password from cin
				std::string password_cstr;
				std::cin >> password_cstr;
				password = wxString(password_cstr.c_str(), *wxConvCurrent);
			}
		}
		else
			return false;
	}

	// Make sure only one instance is running
#if wxCHECK_VERSION(2, 9, 0)
	pSingleInstanceChecker_ = new wxSingleInstanceChecker();
#else
	const wxString appname = wxString::Format(wxT(ENCFSMP_NAME) wxT("-%s"), wxGetUserId().c_str());
    pSingleInstanceChecker_ = new wxSingleInstanceChecker(appname);
#endif

	if(pSingleInstanceChecker_->IsAnotherRunning())
	{
		if(!command.IsEmpty())
		{
			EncFSMPIPC::sendCommand(command, mountName, password);
			return false;
		}
		else
		{
			wxMessageBox(wxT("Another instance of " ) wxT(ENCFSMP_NAME) wxT(" is already running."),
				wxT(ENCFSMP_NAME), wxOK | wxICON_HAND);
			delete pSingleInstanceChecker_;
			pSingleInstanceChecker_ = NULL;
			return false;
		}
	}

	wxInitAllImageHandlers();

	PFMProxy::getInstance().initialize();
	OpenSSLProxy::initialize();
	EncFSMPIPC::initialize();

	pMainFrame_ = new EncFSMPMainFrame(NULL);

	// Show the frame
	pMainFrame_->Show(true);

	SetTopWindow(pMainFrame_);

	// Monitor thread needs top window
	if(!PFMProxy::getInstance().startMonitorThread())
	{
		wxMessageBox(wxT("Creating a PFM monitor failed. Please reinstall Pismo File Mount."),
			wxT(ENCFSMP_NAME), wxOK | wxICON_ERROR);
		return false;
	}

	if(!command.IsEmpty() && !mountName.IsEmpty())
		pMainFrame_->sendCommand(command, mountName, password);

	return true;
}

int EncFSMPApp::OnExit()
{
	EncFSMPIPC::cleanup();
	OpenSSLProxy::uninitialize();
	PFMProxy::getInstance().uninitialize();

	delete pSingleInstanceChecker_;

	return 0;
}

/**
 * Gets called when a session ends (logout, shutdown etc.).
 */
void EncFSMPApp::OnEndSession(wxCloseEvent& event)
{
	pMainFrame_->unmountAll();

	EncFSMPIPC::cleanup();
	OpenSSLProxy::uninitialize();
	PFMProxy::getInstance().uninitialize();

	// Call default handler (which quits the program)
	wxApp::OnEndSession(event);
}

/**
 * Gets called when a session is about to end (logout, shutdown etc.).
 *
 * The default behaviour is to send a normal close message to the main
 * window, which is indistinguishable from the regular close.
 */
void EncFSMPApp::OnQueryEndSession(wxCloseEvent& event)
{
	pMainFrame_->unmountAll();
	pMainFrame_->Close();
}

IMPLEMENT_APP(EncFSMPApp)
