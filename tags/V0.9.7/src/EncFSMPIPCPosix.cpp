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
#include "EncFSMPIPCPosix.h"
#include "EncFSMPMainFrame.h"

// Remark: Functionality is disabled with wxWidgets < 2.9.0

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/base64.h>
#else
#pragma message ("Command-line functionality is disabled with wxWidgets < 2.9.0")
#endif
#include <wx/tokenzr.h>
#include <wx/stdpaths.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/**
 * This class defines the server.
 *
 * It is a detached thread, reading from the pipe in an endless loop.
 */
class EncFSMPIPC_Server: public wxThread
{
public:
	EncFSMPIPC_Server(): wxThread(wxTHREAD_DETACHED)
	{
	}
	virtual ~EncFSMPIPC_Server()
	{
	}
	
protected:
	wxThread::ExitCode Entry()
	{
		bool finished = false;
		while (!TestDestroy() && !finished)
		{
			int fd = open(EncFSMPIPC::getFIFOFilename().mb_str(), O_RDONLY);
			if(fd >= 0)
			{
				char buf[1024];
				ssize_t numread = read(fd, buf, 1024);
				close(fd);
				
				if(numread > 0)
				{
					wxString data(buf, *wxConvCurrent, numread);

					// Get main frame
					wxWindow *pTopWindow = wxTheApp->GetTopWindow();
					if(pTopWindow != NULL)
					{
						EncFSMPMainFrame *pMainFrame = dynamic_cast<EncFSMPMainFrame*>(pTopWindow);
						if(pMainFrame != NULL)
						{
							pMainFrame->sendCommand(data);
						}
					}
				}
					
			}
			else
			{
				if(errno == ENOENT)		//File does not exist
					finished = true;
			}
			// Avoid busy loop, wait a few ms
			wxThread::Sleep(100);
		}
		
		return (wxThread::ExitCode)0; // success
	}

};

EncFSMPIPC_Server *EncFSMPIPC::pServer_ = NULL;
wxString EncFSMPIPC::fifoFilename_;

void EncFSMPIPC::initialize()
{
	// Create FIFO
	wxFileName fn(getFIFOFilename());
	fn.SetFullName(wxEmptyString);
	if(!fn.DirExists())
		fn.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	
	if(!wxFileName::FileExists(getFIFOFilename()))
	{
		if(mkfifo(getFIFOFilename().mb_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == 0)
		{
		}
	}
	
	pServer_ = new EncFSMPIPC_Server();
	pServer_->Create();
	pServer_->Run();
}

void EncFSMPIPC::cleanup()
{
	if(wxFileName::Exists(getFIFOFilename()))
		wxRemoveFile(getFIFOFilename());
}

void EncFSMPIPC::sendCommand(const wxString &command,
	const wxString &mountName,
	const wxString &password)
{
	wxString marshalledArgs = marshalArguments(command, mountName, password);
	
	int fd = open(getFIFOFilename().mb_str(), O_WRONLY);
	if(fd >= 0)
	{
		std::string args(marshalledArgs.mb_str());
		write(fd, args.c_str(), args.size());
		close(fd);
	}
}

wxString EncFSMPIPC::marshalArguments(const wxString &command,
	const wxString &mountName,
	const wxString &password)
{
	std::wstring cmdWStr(command.wc_str(), command.Length());
	std::wstring mountNameWStr(mountName.wc_str(), mountName.Length());
	std::wstring passwordWStr(password.wc_str(), password.Length());

	wxString args;
#if wxCHECK_VERSION(2, 9, 0)
	args = wxBase64Encode((const void *)cmdWStr.c_str(), cmdWStr.size() * sizeof(wchar_t));
	args.Append(wxT(" "));
	args.Append(wxBase64Encode((const void *)mountNameWStr.c_str(), mountNameWStr.size() * sizeof(wchar_t)));
	if(!passwordWStr.empty())
	{
		args.Append(wxT(" "));
		args.Append(wxBase64Encode((const void *)passwordWStr.c_str(), passwordWStr.size() * sizeof(wchar_t)));
	}
#endif
	return args;
}

bool EncFSMPIPC::unmarshalArguments(const wxString &args,
	wxString &command,
	wxString &mountName,
	wxString &password)
{
	wxStringTokenizer tokenizer(args, wxT(" "));

	command.Empty();
	mountName.Empty();
	password.Empty();

	int i = 0;
	while(tokenizer.HasMoreTokens())
	{
		wxString token = tokenizer.GetNextToken();
#if wxCHECK_VERSION(2, 9, 0)
		wxMemoryBuffer decMB = wxBase64Decode(token);
		wchar_t *ptr = reinterpret_cast<wchar_t*>(decMB.GetData());
		wxString dec(ptr, ptr + decMB.GetDataLen()/sizeof(wchar_t));
		if(i == 0)
			command = dec;
		else if(i == 1)
			mountName = dec;
		else if(i == 2)
			password = dec;
#endif
		i++;
	}

	return (i >= 2 && i < 4);
}

wxString EncFSMPIPC::getFIFOFilename()
{
	if(fifoFilename_.IsEmpty())
	{
		wxString dataDir = wxStandardPaths::Get().GetUserDataDir();
		wxString fifoFilename(wxT("ipc_fifo"));
		wxFileName fn(dataDir, fifoFilename);
		fifoFilename_ = fn.GetFullPath();
	}
	return fifoFilename_;
}

