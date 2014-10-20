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

#include "EncFSMPMainFrame.h"
#include "PFMProxy.h"
#include "PFMHandlerThread.h"
#include "OpenExistingFSDialog.h"
#include "CreateNewEncFSDialog.h"
#include "ChangePasswordDialog.h"
#include "EncFSUtilities.h"
#include "EncFSMPStrings.h"
#include "EncFSMPTaskBarIcon.h"

#include <wx/dirdlg.h>
#include <wx/busyinfo.h>
#include <wx/config.h>

#if defined(__WXMAC__) || defined(__WXOSX__) || defined(__WXOSX_COCOA__)
#	include "osx/MacMenuWorkaroundBridge.h"
#endif

#if defined(EFS_WIN32)
#	include "Win32Utils.h"
#	include <Shellapi.h>
#endif

// EncFS
#if wxCHECK_VERSION(2, 9, 0) && defined(_WIN32)
#define HAVE_MODE_T		// Workaround for double defined mode_t on Windows
#endif
#include "config.h"

// OpenSSL
#include "openssl/ssl.h"

// boost
#include <boost/version.hpp>

#include <sstream>
#include <memory>

enum
{
	ID_TIMER = 2000,
	ID_NEW_MOUNT_EVENT,
	ID_ENCFS_MOUNT_ERROR_EVENT,
	ID_UAC,
	ID_MINIMIZETOTRAYMENUITEM,
	ID_CTXMOUNT,
	ID_CTXREMOVE,
	ID_CTXEDIT,
	ID_CTXBROWSE,
	ID_CTXSHOWINFO,
	ID_CTXCHANGEPASSWD,
	ID_CTXEXPORT
};

const wxEventType myCustomEventType = wxNewEventType();

EncFSMPMainFrame::EncFSMPMainFrame(wxWindow* parent)
	: EncFSMPMainFrameBase(parent),
	aTimer_(this, ID_TIMER), minimizeToTray_(false),
	pTaskBarIcon_(NULL), pMountsListPopupMenu_(NULL),
	firstTimeOnTimer_(false), isRunningAsAdmin_(false)
{
	pMountsListCtrl_->ClearAll();
	pMountsListCtrl_->InsertColumn(0, wxT("Mounted"));
	pMountsListCtrl_->InsertColumn(1, wxT("Name"));
	pMountsListCtrl_->InsertColumn(2, wxT("EncFS path"));
#if defined(EFS_WIN32)
	pMountsListCtrl_->InsertColumn(3, wxT("Drive"));
#endif

	SetIcon(getIcon());

#if defined(EFS_WIN32)
	if(Win32Utils::hasUAC())
	{
		if(!Win32Utils::isRunAsAdmin())
		{
			wxBitmap uacShieldBM;
			wxMenuItem *pUACMenuItem = new wxMenuItem( pToolsMenu_, ID_UAC,
				wxString( wxT("Restart with admin rights") ) , wxEmptyString, wxITEM_NORMAL );
			if(Win32Utils::getShieldIcon(uacShieldBM))
				pUACMenuItem->SetBitmap(uacShieldBM);
			pToolsMenu_->Insert( 0, pUACMenuItem );
		}
		else
		{
			// Add "(Admin)" to window title
			SetTitle(GetTitle() + wxString(wxT(" (Admin)")));
			isRunningAsAdmin_ = true;
		}
	}
#endif

#if wxCHECK_VERSION(2, 9, 0)
	if(wxTaskBarIcon::IsAvailable())
#endif
	{
#if defined(EFS_MACOSX)
		pMinimizeToTrayMenuItem_ = pToolsMenu_->Append(ID_MINIMIZETOTRAYMENUITEM,
			wxT("Minimize to menu icon"), wxEmptyString, wxITEM_NORMAL);
#else
		pMinimizeToTrayMenuItem_ = pToolsMenu_->Append(ID_MINIMIZETOTRAYMENUITEM,
			wxT("Minimize to tray"), wxEmptyString, wxITEM_CHECK);
#endif
	}

	// Workaround for OS X: Hide empty menus
#if defined(__WXMAC__) || defined(__WXOSX__) || defined(__WXOSX_COCOA__)
	wxMenuItem *tempItem = pToolsMenu_->Append(ID_INFOBUTTON, wxT("Temp item"));
	CocoaBridge::hideEmptyMenus();
	pToolsMenu_->Remove(tempItem);
#endif

	loadWindowLayoutFromConfig();
	mountList_.loadFromConfig();
	updateMountListCtrl();
	updateButtonStates();

	aTimer_.Start(50);
}

EncFSMPMainFrame::~EncFSMPMainFrame()
{
	aTimer_.Stop();
	if(pTaskBarIcon_ != NULL)
	{
		delete pTaskBarIcon_;
		pTaskBarIcon_ = NULL;
	}
	if(pMountsListPopupMenu_ != NULL)
	{
		delete pMountsListPopupMenu_;
		pMountsListPopupMenu_ = NULL;
	}
}

/**
 * This method gets called by the PFMMonitorThread in case of an mount/unmount event.
 */
void EncFSMPMainFrame::addNewMountEvent(bool isMountEvent,
	const std::wstring &mountName, wchar_t driveLetter, const std::wstring &uncName)
{
	{
		wxMutexLocker lock(mountEventsMutex_);
		MountEvent evt;
		evt.isMountEvent_ = isMountEvent;
		evt.mountName_ = mountName;
		evt.driveLetter_ = driveLetter;
		evt.uncName_ = uncName;
		mountEvents_.push_back(evt);
	}

#if wxCHECK_VERSION(2, 9, 0)
	wxCommandEvent* newEvent = new wxCommandEvent(myCustomEventType, ID_NEW_MOUNT_EVENT);
	wxQueueEvent(this, newEvent);
#else
	wxCommandEvent newEvent(myCustomEventType, ID_NEW_MOUNT_EVENT);
	AddPendingEvent(newEvent);
#endif
}

/**
 * This method gets called by the PFMHandlerThread in case of an error.
 */
void EncFSMPMainFrame::reportEncFSError(const wxString &error, const wxString &mountName)
{
	{
		wxMutexLocker lock(encFSErrorMutex_);
		encFSErrorList_.push_back(error);
	}

	std::wstring mountNameStdStr(mountName.c_str());
	addNewMountEvent(false, mountNameStdStr, L' ', L"");

#if wxCHECK_VERSION(2, 9, 0)
	wxCommandEvent* newEvent = new wxCommandEvent(myCustomEventType, ID_ENCFS_MOUNT_ERROR_EVENT);
	wxQueueEvent(this, newEvent);
#else
	wxCommandEvent newEvent(myCustomEventType, ID_ENCFS_MOUNT_ERROR_EVENT);
	AddPendingEvent(newEvent);
#endif
}

void EncFSMPMainFrame::unmountAllAndQuit()
{
	// Unmount
	std::list<MountEntry> &mountList = mountList_.getList();
	std::list<MountEntry>::iterator iter = mountList.begin();
	while(iter != mountList.end())
	{
		MountEntry &cur = (*iter);
		if(cur.mountState_ == MountEntry::MSMounted)
		{
			PFMProxy::getInstance().unmount(cur.name_);
			cur.mountState_ = MountEntry::MSNotMounted;	// Make sure OnMainFrameClose does not ask again...
		}

		iter++;
	}

	Close();
}

void EncFSMPMainFrame::OnMainFrameClose( wxCloseEvent& evt )
{
	// Check whether there are active or pending mounts
	int mountCount = 0, pendingCount = 0;
	std::list<MountEntry> &mountList = mountList_.getList();
	std::list<MountEntry>::iterator iter = mountList.begin();
	while(iter != mountList.end())
	{
		MountEntry &cur = (*iter);
		if(cur.mountState_ == MountEntry::MSMounted)
			mountCount++;
		else if(cur.mountState_ == MountEntry::MSPending)
			pendingCount++;

		iter++;
	}
	if(pendingCount > 0)
	{
		wxString msgText = wxString::Format(wxT("There are still %d pending mounts.\n")
			wxT("Are you sure you want to exit the program?\nData loss may occur!"),
			pendingCount);

		int retVal = wxMessageBox(msgText, wxT("Confirm"), wxYES_NO | wxCANCEL, this);
		if(retVal == wxCANCEL
			|| retVal == wxNO)
		{
			evt.Veto();
			return;
		}
	}
	if(mountCount > 0)
	{
		wxString msgText = wxString::Format(wxT("There are still %d active mounts.\n")
			wxT("Are you sure you want to exit the program?"),
			mountCount);

		int retVal = wxMessageBox(msgText, wxT("Confirm"), wxYES_NO | wxCANCEL, this);
		if(retVal == wxCANCEL
			|| retVal == wxNO)
		{
			evt.Veto();
			return;
		}
	}

	// Point of no return
	PFMProxy::getInstance().closingDown();

	// Unmount
	iter = mountList.begin();
	while(iter != mountList.end())
	{
		MountEntry &cur = (*iter);
		if(cur.mountState_ == MountEntry::MSMounted)
		{
			PFMProxy::getInstance().unmount(cur.name_);
		}

		iter++;
	}

	saveWindowLayoutToConfig();

	aTimer_.Stop();
	Destroy();
}

void EncFSMPMainFrame::OnMainFrameIconize( wxIconizeEvent& event )
{
	if(
#if wxCHECK_VERSION(2, 9, 0)
		wxTaskBarIcon::IsAvailable() &&
#endif
		minimizeToTray_)
	{
		if(event.IsIconized())
		{
			// Hide window, remove from task bar
			this->Hide();
			// Add tray icon
			if(pTaskBarIcon_ == NULL)
			{
				pTaskBarIcon_ = new EncFSMPTaskBarIcon();
				wxIcon icon = getIcon();
#if defined(EFS_MACOSX)
				// On OS X, menu icons (NSStatusMenus) should be black and white
				wxBitmap iconBM;
				iconBM.CopyFromIcon(icon);
				wxImage img = iconBM.ConvertToImage();
				wxImage imgGrey = img.ConvertToGreyscale();
				wxBitmap iconBMGrey(imgGrey);
				icon.CopyFromBitmap(iconBMGrey);
#endif
				pTaskBarIcon_->SetIcon(icon, EncFSMPStrings::configAppName_);
				pTaskBarIcon_->setMainFrame(this);
			}
		}
		else
		{
			// Show window, add to task bar
#if wxCHECK_VERSION(2, 9, 0)
			pTaskBarIcon_->Destroy();
#endif
			pTaskBarIcon_ = NULL;
		}
	}
}

void EncFSMPMainFrame::OnExitMenuItem( wxCommandEvent& WXUNUSED(event) )
{
	Close();
}

void EncFSMPMainFrame::OnExportMenuItem( wxCommandEvent& event )
{
	MountEntry *pMountEntry = getSelectedMount();
	if(pMountEntry != NULL)
	{
		// Choose export directory
		wxDirDialog dlg(this, wxT("Please choose the export path:"));
		if(dlg.ShowModal() != wxID_OK)
			return;

		wxString exportPath = dlg.GetPath();

		// Enter password, if necessary
		wxString password = pMountEntry->password_;
		if(password.Length() == 0)
		{
			wxPasswordEntryDialog dlg(this, wxT("Please enter the password:"));
			int retVal = dlg.ShowModal();
			if(retVal == wxID_CANCEL)
				return;
			password = dlg.GetValue();
		}

		// TODO: Show progress bar, start separate thread?
		wxString errorMsg;
		bool isOK = true;
		{
			wxWindowDisabler disableAll;
			wxBusyInfo wait(wxT("Exporting EncFS files..."));
			isOK = EncFSUtilities::exportEncFS(pMountEntry->encFSPath_,
				password, exportPath, errorMsg);
		}
		if(isOK)
		{
			wxMessageBox(wxT("Exporting EncFS finished."), wxT("Export EncFS"),
				wxICON_INFORMATION | wxOK, this);
		}
		else
		{
			wxMessageBox(errorMsg,
				wxT("Export EncFS"), wxICON_ERROR | wxOK, this);
		}

	}
}

void EncFSMPMainFrame::OnAboutMenuItem( wxCommandEvent& WXUNUSED(event) )
{
	wxAboutDialogInfo info;
	info.SetName(wxT(ENCFSMP_NAME) wxT(" ") wxT(ENCFSMP_ARCHITECTURE_STRING));
	info.SetVersion(wxString(ENCFSMP_VERSION_STRING, *wxConvCurrent));
	info.SetIcon(getIcon());

	wxString descrString;
	descrString = wxT("Mount EncFS folders - multiplatform style.\n\n");
	descrString.Append(wxT("Built with: ") wxT(ENCFSMP_COMPILER) wxT(" ") wxT(ENCFSMP_COMPILER_VERSION) wxT("\n"));
	descrString.Append(wxT("Linked with:\n - ") wxVERSION_STRING wxT("\n - "));
	descrString.Append(wxT("EncFS ") wxT(EFS_STRINGIFY(VERSION)) wxT("\n - "));
	descrString.Append(wxString(wxT(OPENSSL_VERSION_TEXT)));

	wxString boostVersion;
	boostVersion.Printf(wxT("%d.%d.%d"), BOOST_VERSION / 100000, BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100);
	descrString.Append(wxT("\n - boost "));
	descrString.Append(boostVersion);
	descrString.Append(wxT("\n\n"));

	if(PFMProxy::getInstance().isPFMPresent())
		descrString.Append(wxString(wxT("Pismo File Mount version found: "))
			+ wxString(PFMProxy::getInstance().getPFMVersionString().c_str(), *wxConvCurrent) );
	else
		descrString.Append(wxT("No Pismo File Mount installation found"));
	
	descrString.Append(wxT("\nIcon (C) by Oxygen\n"));

	info.SetDescription(descrString);
	info.SetCopyright(wxT("(C) ") wxT(ENCFSMP_COPYRIGHT_YEAR) wxT(" ") wxT(ENCFSMP_COPYRIGHT_NAME));

	wxAboutBox(info);
}

void EncFSMPMainFrame::OnCreateMountButton( wxCommandEvent& event )
{
	CreateNewEncFSDialog dlg(this);
	dlg.setMountList(&mountList_);
	if(dlg.ShowModal() == wxID_OK)
	{
		if(!EncFSUtilities::createEncFS(dlg.getEncFSPath(),
			dlg.password_, dlg.getCipherAlgorithm(), dlg.getCipherKeySize(),
			dlg.getCipherBlockSize(), dlg.getNameEncoding(),
			dlg.getKeyDerivationDuration(), dlg.perBlockHMAC_,
			dlg.uniqueIV_, dlg.chainedIV_, dlg.externalIV_))
		{
			wxMessageBox(wxT("Creation of new EncFS failed"), wxT("Error"),
				wxOK | wxICON_ERROR);
			return;
		}

		wxString password;
		if(dlg.storePassword_)
			password = dlg.password_;
		mountList_.addMount(dlg.mountName_, dlg.getEncFSPath(), dlg.getDriveLetter(),
			password, dlg.worldWritable_, dlg.systemVisible_, false);

		mountList_.storeToConfig();

		updateMountListCtrl();
	}

}

void EncFSMPMainFrame::OnOpenExistingEncFSButton( wxCommandEvent& event )
{
	OpenExistingFSDialog dlg(this);
	dlg.setMountList(&mountList_);
	if(dlg.ShowModal() == wxID_OK)
	{
		// Check whether path is EncFS path
		EncFSUtilities::EncFSInfo info;
		bool encFSFound = EncFSUtilities::getEncFSInfo(dlg.getEncFSPath(), info);
		if(!encFSFound)
		{
			wxString msg(wxT("No supported encrypted filesystem found.\n"));
			if(!info.configVersionString.empty())
				msg.Append(info.configVersionString);
			wxMessageBox(msg, wxT("Open existing EncFS failed"), wxOK | wxICON_ERROR);
			return;
		}
		wxString password;
		if(dlg.storePassword_)
			password = dlg.password_;

		mountList_.addMount(dlg.mountName_, dlg.getEncFSPath(),
			wxString(dlg.getDriveLetter()), password,
			dlg.worldWritable_, dlg.systemVisible_, false);

		mountList_.storeToConfig();
		updateMountListCtrl();
	}
}

void EncFSMPMainFrame::OnMountsListColClick( wxListEvent& event )
{
}

void EncFSMPMainFrame::OnMountsListItemDeselected( wxListEvent& event )
{
	updateButtonStates();
}

void EncFSMPMainFrame::OnMountsListItemRightClick( wxListEvent& event )
{
	if(pMountsListPopupMenu_ != NULL)
	{
		delete pMountsListPopupMenu_;
		pMountsListPopupMenu_ = NULL;
	}

	bool isPFMPresent = PFMProxy::getInstance().isPFMPresent();
	MountEntry *pMountEntry = getSelectedMount();
/*
	ID_CTXMOUNT,
	ID_CTXREMOVE,
	ID_CTXEDIT,
	ID_CTXBROWSE,
	ID_CTXSHOWINFO,
	ID_CTXCHANGEPASSWD,
	ID_CTXEXPORT
*/

	// Don't show popup menu if no selected entry was found or mount is pending
	if(pMountEntry == NULL
		|| pMountEntry->mountState_ == MountEntry::MSPending)
		return;

	pMountsListPopupMenu_ = new wxMenu();
	if(pMountEntry->mountState_ == MountEntry::MSMounted)
	{
		if(isPFMPresent)
			pMountsListPopupMenu_->Append(ID_CTXMOUNT, wxT("Unmount"));
		pMountsListPopupMenu_->Append(ID_CTXBROWSE, wxT("Browse"));
	}
	else
	{
		if(isPFMPresent)
			pMountsListPopupMenu_->Append(ID_CTXMOUNT, wxT("Mount"));
		pMountsListPopupMenu_->Append(ID_CTXREMOVE, wxT("Remove"));
		pMountsListPopupMenu_->Append(ID_CTXEDIT, wxT("Edit"));
		pMountsListPopupMenu_->Append(ID_CTXSHOWINFO, wxT("Show Info"));
		pMountsListPopupMenu_->Append(ID_CTXCHANGEPASSWD, wxT("Change password"));
		pMountsListPopupMenu_->Append(ID_CTXEXPORT, wxT("Export"));
	}

	PopupMenu(pMountsListPopupMenu_);
}

void EncFSMPMainFrame::OnMountsListItemSelected( wxListEvent& event )
{
	updateButtonStates();
}

void EncFSMPMainFrame::OnMountsListKeyDown( wxListEvent& event )
{
	int keyCode = event.GetKeyCode();
	if(keyCode == WXK_DELETE || keyCode == WXK_BACK)
	{
		// Delete selected item
		MountEntry *pMountEntry = getSelectedMount();
		if(pMountEntry != NULL)
		{
			std::list<MountEntry> &mountList = mountList_.getList();
			std::list<MountEntry>::iterator iter = mountList.begin();
			while(iter != mountList.end())
			{
				MountEntry &cur = (*iter);
				if(pMountEntry->name_ == cur.name_
					&& pMountEntry->mountState_ == MountEntry::MSNotMounted)
				{
					mountList.erase(iter);
					mountList_.storeToConfig();
					updateMountListCtrl();
					return;
				}
				iter++;
			}
		}
	}
}

void EncFSMPMainFrame::OnMountButton( wxCommandEvent& event )
{
	MountEntry *pMountEntry = getSelectedMount();
	if(pMountEntry != NULL)
	{
		if(pMountEntry->mountState_ == MountEntry::MSMounted)
		{
			PFMProxy::getInstance().unmount(pMountEntry->name_);
			pMountEntry->mountState_ = MountEntry::MSPending;
		}
		else if(pMountEntry->mountState_ == MountEntry::MSNotMounted)
		{
			// Mount: Start a PFMHandlerThread
#if defined(EFS_WIN32)
			if(Win32Utils::hasUAC()
				&& pMountEntry->isSystemVisible_
				&& !isRunningAsAdmin_)
			{
				wxMessageBox(wxT("Mounts with the System Visible Flag set require admin rights."),
					wxT("Admin rights required"), wxOK | wxICON_ERROR);
				return;
			}
#endif
			wxString password = pMountEntry->password_;
			if(password.length() == 0)
			{
				wxPasswordEntryDialog dlg(this, wxT("Please enter the password:"));
				int retVal = dlg.ShowModal();
				if(retVal == wxID_CANCEL)
					return;
				password = dlg.GetValue();
			}
			bool isWorldWritable = pMountEntry->isWorldWritable_;
			bool isSystemVisible = pMountEntry->isSystemVisible_;
#if defined(EFS_WIN32)
			// We need to world writable flag so that all users (not only the admin user) can see the mount and change the files in it
			if(Win32Utils::hasUAC() && isRunningAsAdmin_)
			{
				isWorldWritable = true;
				isSystemVisible = true;
			}
#endif
			PFMHandlerThread *pPFMHandlerThread = new PFMHandlerThread();
			pPFMHandlerThread->setParameters(pMountEntry->name_,
				pMountEntry->encFSPath_, pMountEntry->driveLetter_,
				password, isWorldWritable,
				false, isSystemVisible);

			pPFMHandlerThread->Create();
			pPFMHandlerThread->Run();
			pMountEntry->mountState_ = MountEntry::MSPending;
		}

		updateMountListCtrl();
	}
}

void EncFSMPMainFrame::OnRemoveButton( wxCommandEvent& event )
{
	MountEntry *pMountEntry = getSelectedMount();
	if(pMountEntry != NULL)
	{
		std::list<MountEntry> &mountList = mountList_.getList();
		std::list<MountEntry>::iterator iter = mountList.begin();
		while(iter != mountList.end())
		{
			MountEntry &cur = (*iter);
			if(pMountEntry->name_ == cur.name_)
			{
				mountList.erase(iter);
				mountList_.storeToConfig();
				updateMountListCtrl();
				return;
			}
			iter++;
		}
	}
}

void EncFSMPMainFrame::OnEditButton( wxCommandEvent& event )
{
	MountEntry *pMountEntry = getSelectedMount();
	if(pMountEntry != NULL)
	{
		OpenExistingFSDialog dlg(this);
		dlg.setMountList(&mountList_);
		dlg.setEditMode(pMountEntry->name_,
			pMountEntry->encFSPath_, pMountEntry->driveLetter_[0],
			pMountEntry->password_, pMountEntry->isWorldWritable_,
			pMountEntry->isSystemVisible_);
		if(dlg.ShowModal() == wxID_OK)
		{
			wxString password;
			if(dlg.storePassword_)
				password = dlg.password_;
			else
				password = wxEmptyString;

			pMountEntry->driveLetter_ = wxString(dlg.getDriveLetter());
			pMountEntry->password_ = password;
			pMountEntry->isWorldWritable_ = dlg.worldWritable_;
			pMountEntry->isSystemVisible_ = dlg.systemVisible_;

			mountList_.storeToConfig();
			updateMountListCtrl();
		}
	}
}

void EncFSMPMainFrame::OnBrowseButton( wxCommandEvent& event )
{
	MountEntry *pMountEntry = getSelectedMount();
	if(pMountEntry != NULL)
	{
#if defined(EFS_WIN32)
		wxString path = pMountEntry->assignedDriveLetter_;
		path.Append(wxT(":\\"));
		
		if(!wxFileName::DirExists(path))
			return;

		wxString winDir = wxGetOSDirectory();
		wxFileName fn(winDir, wxT("Explorer.exe"));
		if(fn.FileExists())
		{
			wxString cmd;
			cmd = fn.GetFullPath();
			cmd.Append(wxT(" "));
			cmd.Append(path);
			wxExecute(cmd, wxEXEC_ASYNC);
		}
		else
		{
			wxMessageBox(wxT("Explorer.exe not found!"),
				wxT("Serious error"), wxICON_ERROR | wxOK, this);
		}
#elif defined(EFS_MACOSX)
		wxString path = pMountEntry->assignedUncName_;
		wxString cmd(wxT("/usr/bin/open \""));
		cmd.Append(path);
		cmd.Append(wxT("\""));
		wxExecute(cmd, wxEXEC_ASYNC);
#endif
	}
}

void EncFSMPMainFrame::OnInfoButton( wxCommandEvent& event )
{
	MountEntry *pMountEntry = getSelectedMount();
	if(pMountEntry != NULL)
	{
		wxString path = pMountEntry->encFSPath_;
		EncFSUtilities::EncFSInfo info;
		bool retVal = EncFSUtilities::getEncFSInfo(path, info);
		if(retVal)
		{
			ShowEncFSInfoDialog dlg(this);
			dlg.mountName_ = pMountEntry->name_;
			dlg.encFSPath_ = pMountEntry->encFSPath_;
			dlg.configVersion_ = info.configVersionString;
			dlg.cipherAlgorithm_ = info.cipherAlgorithm;
			dlg.cipherKeySize_ = wxString::Format(wxT("%i"), info.cipherKeySize);
			dlg.cipherBlocksize_ = wxString::Format(wxT("%i"), info.cipherBlockSize);
			dlg.nameEncoding_ = info.nameEncoding;
			dlg.keyDerivIterations_ = wxString::Format(wxT("%i"), info.keyDerivationIterations);
			dlg.saltSize_ = wxString::Format(wxT("%i"), info.saltSize);
			dlg.perBlockHMAC_ = info.perBlockHMAC;
			dlg.pPerBlockHMACCheckBox_->Enable(false);
			dlg.uniqueIV_ = info.uniqueIV;
			dlg.pUniqueIVCheckBox_->Enable(false);
			dlg.chainedIV_ = info.chainedIV;
			dlg.pChainedIVCheckBox_->Enable(false);
			dlg.externalIV_ = info.externalIV;
			dlg.pExternalIVCheckBox_->Enable(false);
			dlg.ShowModal();
		}
		else
		{
			wxMessageBox(info.configVersionString,
				wxT("Show EncFS info"), wxICON_ERROR | wxOK, this);
		}
	}
}

void EncFSMPMainFrame::OnChangePasswordButton( wxCommandEvent& event )
{
	MountEntry *pMountEntry = getSelectedMount();
	if(pMountEntry != NULL)
	{
		wxString path = pMountEntry->encFSPath_;

		ChangePasswordDialog dlg(this);
		if(dlg.ShowModal() == wxID_OK)
		{
			wxString errorMsg;
			bool isOK = EncFSUtilities::changePassword(path, dlg.oldPassword_,
				dlg.newPassword_, errorMsg);
			if(isOK)
			{
				wxMessageBox(errorMsg,
					wxT("Change password successful"), wxICON_INFORMATION | wxOK, this);
			}
			else
			{
				wxMessageBox(errorMsg,
					wxT("Change password error"), wxICON_ERROR | wxOK, this);
			}

			if(dlg.storeNewPassword_)
				pMountEntry->password_ = dlg.newPassword_;
			else
				pMountEntry->password_ = wxEmptyString;

			mountList_.storeToConfig();
		}
	}
}

void EncFSMPMainFrame::OnTimer( wxTimerEvent & WXUNUSED(event) )
{
	if(firstTimeOnTimer_)
	{
		firstTimeOnTimer_ = false;

		updateButtonStates();
	}
}

void EncFSMPMainFrame::OnMountEvent( wxCommandEvent &event )
{
	wxMutexLocker lock(mountEventsMutex_);

	bool updateListCtrl = false;
	while(!mountEvents_.empty())
	{
		const MountEvent &evt = mountEvents_.front();

		// Update mount list
		MountEntry *pMountEntry = mountList_.findEntryByName(evt.mountName_);
		if(pMountEntry != NULL)
		{
			pMountEntry->assignedDriveLetter_ = wxString(evt.driveLetter_);
			pMountEntry->assignedUncName_ = wxString(evt.uncName_.c_str());
			if(evt.isMountEvent_)
				pMountEntry->mountState_ = MountEntry::MSMounted;
			else
				pMountEntry->mountState_ = MountEntry::MSNotMounted;
			updateListCtrl = true;
		}

		mountEvents_.pop_front();
	}


	if(updateListCtrl)
	{
		updateMountListCtrl();
		updateButtonStates();
	}
}

void EncFSMPMainFrame::OnEncFSMountErrorEvent( wxCommandEvent &event )
{
	std::list<wxString> encFSErrorListCopy;
	{
		// Copy the list locally, so that the mutex is only locked for a short time
		wxMutexLocker lock(encFSErrorMutex_);
		encFSErrorListCopy = encFSErrorList_;
		encFSErrorList_.clear();
	}

	std::list<wxString>::iterator iter = encFSErrorListCopy.begin();
	while(iter != encFSErrorListCopy.end())
	{
		wxString &errMsg = (*iter);
		wxMessageBox(errMsg, wxT("EncFS error"), wxOK | wxICON_ERROR);
		iter++;
	}
}

void EncFSMPMainFrame::OnUAC( wxCommandEvent& event )
{
#if defined(EFS_WIN32)
	if(Win32Utils::restartAsAdmin((HWND)this->GetHWND()))
		Close();
#endif
}

void EncFSMPMainFrame::OnMinimizeToTrayMenuItem( wxCommandEvent& event )
{
#if defined(EFS_MACOSX)
	wxIconizeEvent dummyEvent(0, true);
	dummyEvent.SetEventObject(this);
	OnMainFrameIconize(dummyEvent);
#else
	minimizeToTray_ = (event.GetInt() != 0);
	saveWindowLayoutToConfig();
#endif
}

void EncFSMPMainFrame::OnContextMenuMount( wxCommandEvent& event )
{
	OnMountButton(event);
}

void EncFSMPMainFrame::OnContextMenuRemove( wxCommandEvent& event )
{
	OnRemoveButton(event);
}

void EncFSMPMainFrame::OnContextMenuEdit( wxCommandEvent& event )
{
	OnEditButton(event);
}

void EncFSMPMainFrame::OnContextMenuBrowse( wxCommandEvent& event )
{
	OnBrowseButton(event);
}

void EncFSMPMainFrame::OnContextMenuShowInfo( wxCommandEvent& event )
{
	OnInfoButton(event);
}

void EncFSMPMainFrame::OnContextMenuChangePassword( wxCommandEvent& event )
{
	OnChangePasswordButton(event);
}

void EncFSMPMainFrame::OnContextMenuExport( wxCommandEvent& event )
{
	OnExportMenuItem(event);
}


// Icon: From resource on Win32, from PNG otherwise
#if !defined(EFS_WIN32)
#include "res/png/encfsmp_png.h"
#endif

wxIcon EncFSMPMainFrame::getIcon()
{
#if defined(EFS_WIN32)
	wxIcon icon(wxT("aaaaaaaa"));
#else
	wxMemoryInputStream istream(encfsmp_png, sizeof(encfsmp_png));
	wxImage myimage_img(istream, wxBITMAP_TYPE_PNG);
	wxBitmap myimage_bmp(myimage_img);
	wxIcon icon;
	icon.CopyFromBitmap(myimage_bmp);
#endif

	return icon;
}

void EncFSMPMainFrame::updateMountListCtrl()
{
	std::list<MountEntry> &mountList = mountList_.getList();
	int listCtrlItemCount = pMountsListCtrl_->GetItemCount();
	int mountsCount = static_cast<int>(mountList.size());

	std::list<MountEntry>::iterator iter = mountList.begin();
	//while(iter != mountList.end())
	for(int i = 0; i < mountsCount; i++, iter++)
	{
		MountEntry &curEntry = (*iter);
		wxString mountedString(wxT("No"));
		if(curEntry.mountState_ == MountEntry::MSMounted)
			mountedString = wxT("Yes");
		else if(curEntry.mountState_ == MountEntry::MSPending)
			mountedString = wxT("Pending");

		if(i >= listCtrlItemCount)
		{
			pMountsListCtrl_->InsertItem(i, mountedString);
		}
		else
		{
			pMountsListCtrl_->SetItem(i, 0, mountedString);
		}
		pMountsListCtrl_->SetItem(i, 1, curEntry.name_);
		pMountsListCtrl_->SetItem(i, 2, curEntry.encFSPath_);
#if defined(EFS_WIN32)
		wxString driveLetterStr(wxT("Auto"));
		if(curEntry.mountState_ == MountEntry::MSMounted)
			driveLetterStr = curEntry.assignedDriveLetter_ + wxT(":");
		else if(curEntry.driveLetter_ != wxT("?"))
			driveLetterStr = curEntry.driveLetter_ + wxT(":");
		pMountsListCtrl_->SetItem(i, 3, driveLetterStr);
#endif

		//iter++;
	}

	// Remove superfluous entries in list control
	for(int i = mountsCount; i < listCtrlItemCount; i++)
	{
		pMountsListCtrl_->DeleteItem(i);
	}
}

MountEntry *EncFSMPMainFrame::getSelectedMount()
{
	int selectedItem = -1;
	int stateMask = wxLIST_STATE_SELECTED;
	int itemCount = pMountsListCtrl_->GetItemCount();
	for(int i = 0; i < itemCount; i++)
	{
		int itemState = pMountsListCtrl_->GetItemState(i, stateMask);
		if(itemState != 0)
		{
			selectedItem = i;
		}
	}

	if(selectedItem >= 0)
	{
#if wxCHECK_VERSION(2, 9, 0)
		wxString selectedMountName = pMountsListCtrl_->GetItemText(selectedItem, 1);
#else
		wxListItem info;
		info.SetMask(wxLIST_MASK_TEXT);
		info.SetId(selectedItem);
		info.SetColumn(1);
		wxString selectedMountName;

		if(pMountsListCtrl_->GetItem(info))
			selectedMountName = info.GetText();
#endif
		MountEntry *pMountEntry = mountList_.findEntryByName(selectedMountName);
		return pMountEntry;
	}

	return NULL;
}

void EncFSMPMainFrame::updateButtonStates()
{
	bool isPFMPresent = PFMProxy::getInstance().isPFMPresent();
	MountEntry *pMountEntry = getSelectedMount();
	if(pMountEntry != NULL)
	{
		if(pMountEntry->mountState_ == MountEntry::MSMounted)
		{
			pMountButton_->Enable(isPFMPresent);
			pMountButton_->SetLabel(wxT("Unmount"));
			pRemoveButton_->Enable(false);
			pEditButton_->Enable(false);
			pBrowseButton_->Enable(isPFMPresent);
			pInfoButton_->Enable(false);
			pChangePasswordButton_->Enable(false);
			pExportMenuItem_->Enable(false);
		}
		else if(pMountEntry->mountState_ == MountEntry::MSNotMounted)
		{
			pMountButton_->Enable(isPFMPresent);
			pMountButton_->SetLabel(wxT("Mount"));
			pRemoveButton_->Enable(true);
			pEditButton_->Enable(true);
			pBrowseButton_->Enable(false);
			pInfoButton_->Enable(true);
			pChangePasswordButton_->Enable(true);
			pExportMenuItem_->Enable(true);
		}
		else	// pending
		{
			pMountButton_->Enable(false);
			pMountButton_->SetLabel(wxT("Unmount"));
			pRemoveButton_->Enable(false);
			pEditButton_->Enable(false);
			pBrowseButton_->Enable(false);
			pInfoButton_->Enable(false);
			pChangePasswordButton_->Enable(false);
			pExportMenuItem_->Enable(false);
		}
	}
	else
	{
		pMountButton_->Enable(false);
		pRemoveButton_->Enable(false);
		pEditButton_->Enable(false);
		pBrowseButton_->Enable(false);
		pInfoButton_->Enable(false);
		pChangePasswordButton_->Enable(false);
		pExportMenuItem_->Enable(false);
	}
}

void EncFSMPMainFrame::saveWindowLayoutToConfig()
{
	std::auto_ptr<wxConfig> config(new wxConfig(EncFSMPStrings::configAppName_,
		EncFSMPStrings::configOrganizationName_));

	{
		wxPoint pt = GetPosition();
		wxSize sz = GetSize();

		std::wostringstream ostr;
		ostr << pt.x << L" " << pt.y << L" " << sz.x << L" " << sz.y;
		wxString dimensions(ostr.str().c_str());

		config->Write(EncFSMPStrings::configWindowDimensions_, dimensions);
	}

	{
		std::wostringstream ostr;
		int columnCount = pMountsListCtrl_->GetColumnCount();
		for(int i = 0; i < columnCount; i++)
		{
			int width = pMountsListCtrl_->GetColumnWidth(i);
			ostr << width << " ";
		}
		wxString columnWidths(ostr.str().c_str());

		config->Write(EncFSMPStrings::configColumnWidths_, columnWidths);
	}

	config->Write(EncFSMPStrings::configMinimizeToTray_, minimizeToTray_);
}

void EncFSMPMainFrame::loadWindowLayoutFromConfig()
{
	long index = 0;
	wxString str;

	std::auto_ptr<wxConfig> config(new wxConfig(EncFSMPStrings::configAppName_,
		EncFSMPStrings::configOrganizationName_));
	config->SetPath(wxT("/"));

	wxString dimensions, columnWidths;
	if(config->Read(EncFSMPStrings::configWindowDimensions_, &dimensions))
	{
		wxPoint pt;
		wxSize sz;
		std::wistringstream istr(std::wstring(dimensions.c_str()));
		istr >> pt.x >> pt.y >> sz.x >> sz.y;
		SetSize(sz);
		SetPosition(pt);
	}
	if(config->Read(EncFSMPStrings::configColumnWidths_, &columnWidths))
	{
		std::wistringstream istr(std::wstring(columnWidths.c_str()));
		int columnCount = pMountsListCtrl_->GetColumnCount();
		for(int i = 0; i < columnCount; i++)
		{
			int width = 0;
			istr >> width;
			pMountsListCtrl_->SetColumnWidth(i, width);
		}
	}

	if(!config->Read(EncFSMPStrings::configMinimizeToTray_, &minimizeToTray_))
		minimizeToTray_ = false;		// Default is false
#if !defined(EFS_MACOSX)
	pMinimizeToTrayMenuItem_->Check(minimizeToTray_);
#endif
}

BEGIN_EVENT_TABLE( EncFSMPMainFrame, EncFSMPMainFrameBase )
	EVT_COMMAND(ID_NEW_MOUNT_EVENT, myCustomEventType, EncFSMPMainFrame::OnMountEvent)
	EVT_COMMAND(ID_ENCFS_MOUNT_ERROR_EVENT, myCustomEventType, EncFSMPMainFrame::OnEncFSMountErrorEvent)
	EVT_MENU( ID_UAC, EncFSMPMainFrame::OnUAC )
	EVT_TIMER(ID_TIMER, EncFSMPMainFrame::OnTimer)
	EVT_MENU( ID_MINIMIZETOTRAYMENUITEM, EncFSMPMainFrame::OnMinimizeToTrayMenuItem )
	EVT_MENU( ID_CTXMOUNT, EncFSMPMainFrame::OnContextMenuMount )
	EVT_MENU( ID_CTXREMOVE, EncFSMPMainFrame::OnContextMenuRemove )
	EVT_MENU( ID_CTXEDIT, EncFSMPMainFrame::OnContextMenuEdit )
	EVT_MENU( ID_CTXBROWSE, EncFSMPMainFrame::OnContextMenuBrowse )
	EVT_MENU( ID_CTXSHOWINFO, EncFSMPMainFrame::OnContextMenuShowInfo )
	EVT_MENU( ID_CTXCHANGEPASSWD, EncFSMPMainFrame::OnContextMenuChangePassword )
	EVT_MENU( ID_CTXEXPORT, EncFSMPMainFrame::OnContextMenuExport )
END_EVENT_TABLE()
