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
#include "EncFSMPTaskBarIcon.h"
#include "EncFSMPMainFrame.h"

enum
{
	ID_TASK_BAR_RESTORE = 3000,
	ID_TASK_BAR_UNMOUNT_ALL_QUIT
};

EncFSMPTaskBarIcon::EncFSMPTaskBarIcon() : wxTaskBarIcon()
{
}

EncFSMPTaskBarIcon::~EncFSMPTaskBarIcon()
{
}

void EncFSMPTaskBarIcon::setMainFrame(EncFSMPMainFrame *pMainFrame)
{
	pMainFrame_ = pMainFrame;
}

wxMenu* EncFSMPTaskBarIcon::CreatePopupMenu()
{
	wxMenu *pPopupMenu = new wxMenu(wxT("EncFSMP"));
	pPopupMenu->Append(ID_TASK_BAR_RESTORE, wxT("Restore"));
	pPopupMenu->Append(ID_TASK_BAR_UNMOUNT_ALL_QUIT, wxT("Unmount all and Quit"));
	return pPopupMenu;
}

void EncFSMPTaskBarIcon::OnMenuItemRestore( wxCommandEvent& event )
{
	if(pMainFrame_ != NULL)
	{
		pMainFrame_->Show(true);
#if defined(EFS_MACOSX)
		wxIconizeEvent dummyEvent(0, false);
		dummyEvent.SetEventObject(this);
		pMainFrame_->OnMainFrameIconize(dummyEvent);
#else
		pMainFrame_->Iconize(false);
#endif
	}
}

void EncFSMPTaskBarIcon::OnUnmountAllAndQuit( wxCommandEvent& event )
{
	if(pMainFrame_ != NULL)
	{
		pMainFrame_->unmountAllAndQuit();
	}
}

void EncFSMPTaskBarIcon::OnTaskBarIconDoubleClick( wxTaskBarIconEvent &evt )
{
	wxCommandEvent dummyEvent;
	OnMenuItemRestore(dummyEvent);
}

BEGIN_EVENT_TABLE( EncFSMPTaskBarIcon, wxTaskBarIcon )
	EVT_MENU( ID_TASK_BAR_RESTORE, EncFSMPTaskBarIcon::OnMenuItemRestore )
	EVT_MENU( ID_TASK_BAR_UNMOUNT_ALL_QUIT, EncFSMPTaskBarIcon::OnUnmountAllAndQuit )
	EVT_TASKBAR_LEFT_DCLICK( EncFSMPTaskBarIcon::OnTaskBarIconDoubleClick )
END_EVENT_TABLE()
