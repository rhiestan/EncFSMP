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

#include "OpenExistingFSDialog.h"
#include "MountList.h"

OpenExistingFSDialog::OpenExistingFSDialog(wxWindow *parent)
	: OpenExistingFSDialogBase(parent),
	pMountList_(NULL), editMode_(false), driveLetter_(L'?')
{
}

OpenExistingFSDialog::~OpenExistingFSDialog()
{
}

void OpenExistingFSDialog::setMountList(MountList *pMountList)
{
	pMountList_ = pMountList;
}

void OpenExistingFSDialog::setEditMode(const wxString &mountName,
	const wxString &encFSPath, wchar_t driveLetter,
	const wxString &password, bool isWorldWritable,
	bool isSystemVisible)
{
	editMode_ = true;
	mountName_ = mountName;
	pMountNameTextCtrl_->SetValue(mountName);
	encFSPath_ = encFSPath;
	pEncFSPathDirPicker_->SetPath(encFSPath);
	driveLetter_ = driveLetter;
	if(password.Length() > 0)
	{
		storePassword_ = true;
		pStorePasswordCheckBox_->SetValue(true);
		pPasswordTextCtrl_->SetValue(password);
		pPasswordRetypeTextCtrl_->SetValue(password);
		password_ = passwordRetype_ = password;
	}
	else
	{
		storePassword_ = false;
		pStorePasswordCheckBox_->SetValue(false);
	}
	worldWritable_ = isWorldWritable;
	pWorldWritableCheckBox_->SetValue(isWorldWritable);
	systemVisible_ = isSystemVisible;
	pSystemVisibleCheckBox_->SetValue(isSystemVisible);
}

void OpenExistingFSDialog::OnInitDialog( wxInitDialogEvent& event )
{
	if(editMode_)
	{
		pMountNameTextCtrl_->Enable(false);
		pEncFSPathDirPicker_->Enable(false);
#if defined(__WXOSX__)
		// Workaround for different behaviour of OS X port
		pEncFSPathDirPicker_->GetTextCtrl()->Disable();
		pEncFSPathDirPicker_->GetPickerCtrl()->Disable();
#endif
		wxCommandEvent dummyEvent;
		OnStorePasswordCheckBox(dummyEvent);

		this->SetTitle(wxT("Edit mount"));
	}
	else
	{
		storePassword_ = false;
		pStorePasswordCheckBox_->SetValue(false);
		pPasswordTextCtrl_->Enable(false);
		pPasswordRetypeTextCtrl_->Enable(false);
	}

#if defined(EFS_WIN32)
	pDriveLetterChoice_->Clear();
	int autoItem = pDriveLetterChoice_->Append(wxT("Auto"));

	// Find out unused drive letters
	DWORD usedLogicalDrives = GetLogicalDrives();
	wxCSConv conv(wxT("C"));	// Use "C" locale
	for(char i = 'A'; i <= 'Z'; i++)
	{
		if((usedLogicalDrives & 0x01) == 0)
		{
			// current drive letter is unused -> add it to combo box
			size_t outLen;
			wxString driveLetter(conv.cMB2WC(&i, 1, &outLen));
			driveLetter.Append(wxT(":"));
			pDriveLetterChoice_->Append(driveLetter);
		}
		usedLogicalDrives >>= 1;
	}
	if(editMode_ && driveLetter_ != L'?')
		pDriveLetterChoice_->SetStringSelection(wxString(driveLetter_) + wxT(":"));
	else
		pDriveLetterChoice_->Select(autoItem);

	pWorldWritableStaticText_->Hide();
	pWorldWritableCheckBox_->Hide();
	pFlexGridSizer_->AddGrowableRow(6);

/*
	wxArrayString volumes = wxFSVolume::GetVolumes();
	for(int i = 0; i < volumes.size(); i++)
	{
		wxString curVol = volumes[i];
		wxFSVolume vol(curVol);
		if(vol.IsOk())
		{
			wxString displayName = vol.GetDisplayName();
			wxString name = vol.GetName();
			wxFSVolumeFlags flags = static_cast<wxFSVolumeFlags>(vol.GetFlags());

			wxFSVolumeKind kind = vol.GetKind();
			if(kind == wxFS_VOL_DISK)
			{
				int jj = 27;
			}
		}
	}
	*/
#else
	pDriveLetterStaticText_->Hide();
	pDriveLetterChoice_->Hide();
	pSystemVisibleStaticText_->Hide();
	pSystemVisibleCheckBox_->Hide();
	pFlexGridSizer_->AddGrowableRow(5);
	pFlexGridSizer_->AddGrowableRow(7);
#endif

	Layout();
}

void OpenExistingFSDialog::OnStorePasswordCheckBox( wxCommandEvent& event )
{
	bool doEnable = false;
	if(pStorePasswordCheckBox_->GetValue())
		doEnable = true;

	pPasswordTextCtrl_->Enable(doEnable);
	pPasswordRetypeTextCtrl_->Enable(doEnable);
}

void OpenExistingFSDialog::OnCancelButton( wxCommandEvent& event )
{
	EndModal(wxID_CANCEL);
}

void OpenExistingFSDialog::OnOKButton( wxCommandEvent& event )
{
	if (Validate() && TransferDataFromWindow())
	{
		if(!editMode_)
		{
			// Make sure mountName_ is unique
			if(pMountList_->findEntryByName(mountName_) != NULL)
			{
				wxMessageBox(wxT("The mount name is not unique! Please choose another name."),
					wxT("Error"), wxOK | wxICON_ERROR);
				return;
			}
			encFSPath_ = pEncFSPathDirPicker_->GetPath();
			if(!wxFileName::DirExists(encFSPath_))
			{
				wxMessageBox(wxT("EncFS path does not exist! Please enter a valid path"),
					wxT("Error"), wxOK | wxICON_ERROR);
				return;
			}
		}

#if defined(EFS_WIN32)
		if(pDriveLetterChoice_->GetSelection() == 0)
		{
			driveLetter_ = L'?';	// Auto
		}
		else
		{
			wxString selectedDriveLetter = pDriveLetterChoice_->GetString(pDriveLetterChoice_->GetSelection());
			driveLetter_ = selectedDriveLetter.c_str()[0];
		}
#else
		driveLetter_ = L'?';
#endif

		if(storePassword_)
		{
			if(password_.Length() == 0)
			{
				wxMessageBox(wxT("Empty password is not allowed!"),
					wxT("Error"), wxOK | wxICON_ERROR);
				return;
			}
			if(password_ != passwordRetype_)
			{
				wxMessageBox(wxT("The retyped password is not equal to the first one!"),
					wxT("Error"), wxOK | wxICON_ERROR);
				return;
			}
		}
		else
		{
			password_ = wxEmptyString;
			passwordRetype_ = wxEmptyString;
		}

		EndModal(wxID_OK);
	}
}


BEGIN_EVENT_TABLE( OpenExistingFSDialog, OpenExistingFSDialogBase )
END_EVENT_TABLE()
