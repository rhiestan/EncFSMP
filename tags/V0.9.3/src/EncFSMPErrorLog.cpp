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
#include "EncFSMPErrorLog.h"

const wxEventType errorLogEventType = wxNewEventType();

enum
{
	ID_NEW_ERROR_LOG_ENTRY = 3000
};

EncFSMPErrorLog::EncFSMPErrorLog(wxWindow* parent)
	: EncFSMPErrorLogBase(parent), isWindowShown_(false),
	showErrorLogOnErr_(true), mutex_()
{
}

EncFSMPErrorLog::~EncFSMPErrorLog()
{
}

bool EncFSMPErrorLog::getShowErrorLogOnErr() const
{
	return showErrorLogOnErr_;
}

void EncFSMPErrorLog::setShowErrorLogOnErr(bool showErrorLogOnErr)
{
	showErrorLogOnErr_ = showErrorLogOnErr;
}

void EncFSMPErrorLog::toggleVisibleState()
{
	isWindowShown_ = !isWindowShown_;
	this->Show(isWindowShown_);
}

bool EncFSMPErrorLog::isWindowShown()
{
	return isWindowShown_;
}

/**
 * Add text to the log window.
 *
 * Since this method can be called from all threads, the
 * text control itself can't be accessed directly. Instead, 
 * - Store text in wxString variable
 * - Send message to self
 * - Add stored text to text ctrl in OnNewErrorLogEntry
 */
void EncFSMPErrorLog::addText(const wxString &text)
{
	{
		wxMutexLocker lock(mutex_);
		errorText_.Append(text);
	}

#if wxCHECK_VERSION(2, 9, 0)
	wxCommandEvent* newEvent = new wxCommandEvent(errorLogEventType, ID_NEW_ERROR_LOG_ENTRY);
	wxQueueEvent(this, newEvent);
#else
	wxCommandEvent newEvent(errorLogEventType, ID_NEW_MOUNT_EVENT);
	AddPendingEvent(newEvent);
#endif
}

void EncFSMPErrorLog::OnClose( wxCloseEvent& event )
{
	isWindowShown_ = false;
	this->Hide();
}

void EncFSMPErrorLog::OnClearErrorListButton( wxCommandEvent& event )
{
	pErrorListTextCtrl_->Clear();
}

void EncFSMPErrorLog::OnNewErrorLogEntry( wxCommandEvent& event )
{
	wxString textLocalCopy;
	{
		wxMutexLocker lock(mutex_);
		textLocalCopy = errorText_;
		errorText_.Clear();
	}
	if(!textLocalCopy.empty())
	{
		(*pErrorListTextCtrl_) << textLocalCopy;

		if(showErrorLogOnErr_
			&& !isWindowShown_)
		{
			toggleVisibleState();
		}
	}
}

BEGIN_EVENT_TABLE( EncFSMPErrorLog, EncFSMPErrorLogBase )
	EVT_COMMAND(ID_NEW_ERROR_LOG_ENTRY, errorLogEventType, EncFSMPErrorLog::OnNewErrorLogEntry)
END_EVENT_TABLE()
