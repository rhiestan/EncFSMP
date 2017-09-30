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
#include "EncFSMPIPCWin.h"
#include "EncFSMPMainFrame.h"
#include "EncFSMPStrings.h"

// Remark: Functionality is disabled with wxWidgets < 2.9.0

#include <wx/ipc.h>
#if wxCHECK_VERSION(2, 9, 0)
#include <wx/base64.h>
#else
#pragma message ("Command-line functionality is disabled with wxWidgets < 2.9.0")
#endif
#include <wx/tokenzr.h>

class EncFSMPIPC_Server;

/**
 * Connection class for Windows.
 *
 * Used for both server and client.
 */
class EncFSMPIPC_Connection: public wxConnection
{
public:
	EncFSMPIPC_Connection() : wxConnection(), pServer_(NULL)
	{
	}
	EncFSMPIPC_Connection(EncFSMPIPC_Server *pServer)
		: wxConnection(),
		pServer_(pServer)
	{
	}
	virtual ~EncFSMPIPC_Connection()
	{
	}

	virtual bool OnExec(const wxString &topic, const wxString &data)
	{
		if(topic.IsSameAs(EncFSMPStrings::commandDDETopic_))
		{
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
			return true;
		}
		return false;
	}
	virtual bool OnDisconnect()
	{
		delete this;
		return true;
	}

private:
	EncFSMPIPC_Server *pServer_;
};

/**
 * Server class for Windows.
 */
class EncFSMPIPC_Server: public wxServer
{
public:
	EncFSMPIPC_Server() : wxServer()
	{
	}
	virtual ~EncFSMPIPC_Server()
	{
	}

	virtual wxConnectionBase *OnAcceptConnection(const wxString& topic)
	{
		if(topic.IsSameAs(EncFSMPStrings::commandDDETopic_))
		{
			pConnection_ = new EncFSMPIPC_Connection(this);
			return pConnection_;
		}

		// Ignore unknown topics
		return NULL;
	}

private:
	EncFSMPIPC_Connection *pConnection_;
};

/**
 * Client class.
 */
class EncFSMPIPC_Client: public wxClient
{
public:
	EncFSMPIPC_Client(): wxClient()
	{
	}
	virtual ~EncFSMPIPC_Client()
	{
	}
	virtual wxConnectionBase* OnMakeConnection()
	{
		return new EncFSMPIPC_Connection();
	}
};

EncFSMPIPC_Server *EncFSMPIPC::pServer_ = NULL;

void EncFSMPIPC::initialize()
{
	if(pServer_ == NULL)
	{
		pServer_ = new EncFSMPIPC_Server();
		if(!pServer_->Create(EncFSMPStrings::commandDDEServerName_))
		{
			// Failed
			delete pServer_;
			pServer_ = NULL;
		}
	}
}

void EncFSMPIPC::cleanup()
{
	if(pServer_ != NULL)
	{
		delete pServer_;
		pServer_ = NULL;
	}
}

void EncFSMPIPC::sendCommand(const wxString &command,
		const wxString &mountName,
		const wxString &password)
{
	EncFSMPIPC_Client *pClient = new EncFSMPIPC_Client();
	wxConnectionBase *pConnection = pClient->MakeConnection(wxT("localhost"),
		EncFSMPStrings::commandDDEServerName_, EncFSMPStrings::commandDDETopic_);
	if(pConnection != NULL)
	{
		wxString marshalledArgs = marshalArguments(command, mountName, password);
		pConnection->Execute(marshalledArgs);
		pConnection->Disconnect();
		delete pConnection;
	}
	delete pClient;
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
		{
			command = dec;
		}
		else if(i == 1)
			mountName = dec;
		else if(i == 2)
			password = dec;
#endif
		i++;
	}

	return (i >= 1 && i < 4);
}
