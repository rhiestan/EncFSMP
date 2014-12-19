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

#ifndef ENCFSMPMAINFRAME_H
#define ENCFSMPMAINFRAME_H

class EncFSMPTaskBarIcon;
class wxMenu;
class EncFSMPErrorLog;

#include <list>
#include "EncFSMPMainFrameBase.h"
#include "MountList.h"

class EncFSMPMainFrame: public EncFSMPMainFrameBase
{
public:
	EncFSMPMainFrame(wxWindow* parent);
	virtual ~EncFSMPMainFrame();

	void addNewMountEvent(bool isMountEvent,
		const std::wstring &mountName, wchar_t driveLetter, const std::wstring &uncName);
	void reportEncFSError(const wxString &error, const wxString &mountName);

	void unmountAllAndQuit();

	virtual void OnMainFrameIconize( wxIconizeEvent& event );

protected:
	virtual void OnMainFrameClose( wxCloseEvent& event );
	virtual void OnExitMenuItem( wxCommandEvent& event );
	virtual void OnExportMenuItem( wxCommandEvent& event );
	virtual void OnShowErrorLogMenuItem( wxCommandEvent& event );
	virtual void OnShowErrorLogMenuItemUpdate( wxUpdateUIEvent& event );
	virtual void OnShowErrorLogOnErrMenuItem( wxCommandEvent& event );
	virtual void OnDisableUnmountDialogOnExitMenuItem( wxCommandEvent& event );
	virtual void OnAboutMenuItem( wxCommandEvent& event );
	virtual void OnCreateMountButton( wxCommandEvent& event );
	virtual void OnOpenExistingEncFSButton( wxCommandEvent& event );
	virtual void OnMountsListColClick( wxListEvent& event );
	virtual void OnMountsListItemDeselected( wxListEvent& event );
	virtual void OnMountsListItemRightClick( wxListEvent& event );
	virtual void OnMountsListItemSelected( wxListEvent& event );
	virtual void OnMountsListKeyDown( wxListEvent& event );
	virtual void OnMountButton( wxCommandEvent& event );
	virtual void OnRemoveButton( wxCommandEvent& event );
	virtual void OnEditButton( wxCommandEvent& event );
	virtual void OnBrowseButton( wxCommandEvent& event );
	virtual void OnInfoButton( wxCommandEvent& event );
	virtual void OnChangePasswordButton( wxCommandEvent& event );

	virtual void OnTimer( wxTimerEvent &event );

	virtual void OnMountEvent( wxCommandEvent &event );
	virtual void OnEncFSMountErrorEvent( wxCommandEvent &event );
	virtual void OnUAC( wxCommandEvent& event );
	virtual void OnMinimizeToTrayMenuItem( wxCommandEvent& event );
	virtual void OnContextMenuMount( wxCommandEvent& event );
	virtual void OnContextMenuRemove( wxCommandEvent& event );
	virtual void OnContextMenuEdit( wxCommandEvent& event );
	virtual void OnContextMenuBrowse( wxCommandEvent& event );
	virtual void OnContextMenuShowInfo( wxCommandEvent& event );
	virtual void OnContextMenuChangePassword( wxCommandEvent& event );
	virtual void OnContextMenuExport( wxCommandEvent& event );

	wxIcon getIcon();
	void updateMountListCtrl();
	MountEntry *getSelectedMount();
	void updateButtonStates();
	void saveWindowLayoutToConfig();
	void loadWindowLayoutFromConfig();

private:
	wxTimer aTimer_;
	wxMenuItem *pMinimizeToTrayMenuItem_;
	bool minimizeToTray_, disableUnmountDialogOnExit_;
	EncFSMPTaskBarIcon *pTaskBarIcon_;
	wxMenu *pMountsListPopupMenu_;

	bool firstTimeOnTimer_;
	bool isRunningAsAdmin_;

	class MountEvent
	{
	public:
		MountEvent() : isMountEvent_(true), driveLetter_(L'A') { }
		MountEvent(const MountEvent &o) { copy(o); }
		virtual ~MountEvent() { }
		MountEvent &copy(const MountEvent &o)
		{
			isMountEvent_ = o.isMountEvent_;
			mountName_ = o.mountName_;
			driveLetter_ = o.driveLetter_;
			uncName_ = o.uncName_;
			return *this;
		}
		MountEvent & operator=(const MountEvent & o)
		{
			return copy(o);
		}

		bool isMountEvent_;
		std::wstring mountName_, uncName_;
		wchar_t driveLetter_;
	};


	wxMutex mountEventsMutex_;
	std::list<MountEvent> mountEvents_;

	wxMutex encFSErrorMutex_;
	std::list<wxString> encFSErrorList_;

	MountList mountList_;

	EncFSMPErrorLog *pEncFSMPErrorLog_;

	DECLARE_EVENT_TABLE()
};

#endif
