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

// Unfortunately, my version of libencfs is a tiny bit newer than 1.7.4 on Linux.
// This define restores full compatibility
#define EFS_COMPATIBILITY_WORKAROUND 1

#include "CreateNewEncFSDialog.h"
#include "MountList.h"

#if wxCHECK_VERSION(2, 9, 0) && defined(_WIN32)
#define HAVE_MODE_T		// Workaround for double defined mode_t on Windows
#endif

// libencfs
#include "config.h"
#include "FileUtils.h"
#include "ConfigReader.h"
#include "FSConfig.h"

#include "DirNode.h"
#include "Cipher.h"
#include "StreamNameIO.h"
#include "BlockNameIO.h"
#include "NullNameIO.h"
#include "Context.h"


CreateNewEncFSDialog::CreateNewEncFSDialog(wxWindow* parent)
	: CreateNewEncFSDialogBase(parent), pMountList_(NULL),
	driveLetter_(L'?'), useExternalConfigFile_(false), isLocalDrive_(true),
	cachingEnabled_(false), cipherKeySize_(256),
	cipherBlockSize_(4096), keyDerivationDuration_(500),
	autoChoice_(0), noneChoice_(0)
{
	pUseExternalConfigFileCheckBox_->SetValue(useExternalConfigFile_);
	pExternalConfigFileNamePickerCtrl_->Enable(false);
	worldWritable_ = false;
	storePassword_ = false;
	pWorldWritableCheckBox_->SetValue(false);
	pStorePasswordCheckBox_->SetValue(false);
	pLocalDriveCheckBox_->SetValue(true);
	pEnableCachingCheckBox_->SetValue(false);

#if defined(EFS_WIN32)
	pDriveLetterChoice_->Clear();
	autoChoice_ = pDriveLetterChoice_->Append(wxT("Auto"));
	noneChoice_ = pDriveLetterChoice_->Append(wxT("None"));
	pDriveLetterChoice_->Select(autoChoice_);

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

	pWorldWritableStaticText_->Hide();
	pWorldWritableCheckBox_->Hide();
	pFlexGridSizer_->AddGrowableRow(6);
#else
	pDriveLetterStaticText_->Hide();
	pDriveLetterChoice_->Hide();
	pFlexGridSizer_->AddGrowableRow(5);
	pLocalDriveStaticText_->Hide();
	pLocalDriveCheckBox_->Hide();
	pFlexGridSizer_->AddGrowableRow(7);
#endif

	pCipherAlgorithmChoice_->Clear();
	encfs::Cipher::AlgorithmList algorithms = encfs::Cipher::GetAlgorithmList();
	encfs::Cipher::AlgorithmList::const_iterator it;
	for(it = algorithms.begin(); it != algorithms.end(); ++it)
	{
		pCipherAlgorithmChoice_->Append(wxString(it->name.c_str(), *wxConvCurrent));
	}
	pCipherAlgorithmChoice_->Select(0);

	pNameEncodingChoice_->Clear();
	wxString toolTipStr;
	encfs::NameIO::AlgorithmList nmalgorithms = encfs::NameIO::GetAlgorithmList();
	encfs::NameIO::AlgorithmList::const_iterator nmit;
	for(nmit = nmalgorithms.begin(); nmit != nmalgorithms.end(); ++nmit)
	{
#if defined(EFS_COMPATIBILITY_WORKAROUND)
		if(nmit->name == std::string("Block32"))
			continue;
#endif
		if(toolTipStr.Length() > 0)
			toolTipStr.Append(wxT("\n"));
		pNameEncodingChoice_->Append(wxString(nmit->name.c_str(), *wxConvCurrent));
		toolTipStr.Append(wxString(nmit->name.c_str(), *wxConvCurrent)
			+ wxT(": ") + wxString(nmit->description.c_str(), *wxConvCurrent));
	}
	pNameEncodingChoice_->Select(0);
	pNameEncodingChoice_->SetToolTip(toolTipStr);

	pKeyDerivationDurationChoice_->Clear();
	wxArrayString timeStrings;
	for(int i = 500; i <= 3000; i+= 500)
	{
		wxString timeStr = wxString::Format(wxT("%d"), i);
		timeStrings.Add(timeStr);
	}
	pKeyDerivationDurationChoice_->Append(timeStrings);
	pKeyDerivationDurationChoice_->Select(0);

	pEncFSConfigurationRadioButton_->Select(0);
	wxCommandEvent dummyEvent;
	OnEncFSConfigurationRadioBox(dummyEvent);

	Layout();
	Fit();
}

CreateNewEncFSDialog::~CreateNewEncFSDialog()
{
}

void CreateNewEncFSDialog::setMountList(MountList *pMountList)
{
	pMountList_ = pMountList;
}

void CreateNewEncFSDialog::OnUseExternalConfigFileCheckBox( wxCommandEvent& event )
{
	bool isChecked = event.IsChecked();
	if(isChecked)
	{
		pExternalConfigFileNamePickerCtrl_->Enable(true);
	}
	else
	{
		pExternalConfigFileNamePickerCtrl_->Enable(false);
	}
}

void CreateNewEncFSDialog::OnEncFSConfigurationRadioBox( wxCommandEvent& event )
{
	bool enableDetailWidgets = false;
	int selection = pEncFSConfigurationRadioButton_->GetSelection();
	if(selection == 0)
	{
		// Standard configuration: AES, 192, 1024, Block, false, true, true, false, 500
		pCipherAlgorithmChoice_->SetStringSelection(wxT("AES"));

		wxCommandEvent dummyEvent;
		OnCipherAlgorithmChoice(dummyEvent);

		pCipherKeysizeChoice_->SetStringSelection(wxT("192"));
		pCipherBlocksizeChoice_->SetStringSelection(wxT("1024"));
		pNameEncodingChoice_->SetStringSelection(wxT("Block"));
		pPerBlockHMACCheckBox_->SetValue(false);
		pUniqueIVCheckBox_->SetValue(true);
		pChainedIVCheckBox_->SetValue(true);
		pExternalIVCheckBox_->SetValue(false);
		pKeyDerivationDurationChoice_->SetStringSelection(wxT("500"));
	}
	else if(selection == 1)
	{
		// Cloud-friendly paranoia configuration: AES, 256, 1024, Block, true, true, true, false, 3000
		pCipherAlgorithmChoice_->SetStringSelection(wxT("AES"));

		wxCommandEvent dummyEvent;
		OnCipherAlgorithmChoice(dummyEvent);

		pCipherKeysizeChoice_->SetStringSelection(wxT("256"));
		pCipherBlocksizeChoice_->SetStringSelection(wxT("1024"));
		pNameEncodingChoice_->SetStringSelection(wxT("Block"));
		pPerBlockHMACCheckBox_->SetValue(true);
		pUniqueIVCheckBox_->SetValue(true);
		pChainedIVCheckBox_->SetValue(true);
		pExternalIVCheckBox_->SetValue(false);
		pKeyDerivationDurationChoice_->SetStringSelection(wxT("3000"));
	}
	else if(selection == 2)
	{
		// Paranoia configuration: AES, 256, 1024, Block, true, true, true, true, 3000
		pCipherAlgorithmChoice_->SetStringSelection(wxT("AES"));

		wxCommandEvent dummyEvent;
		OnCipherAlgorithmChoice(dummyEvent);

		pCipherKeysizeChoice_->SetStringSelection(wxT("256"));
		pCipherBlocksizeChoice_->SetStringSelection(wxT("1024"));
		pNameEncodingChoice_->SetStringSelection(wxT("Block"));
		pPerBlockHMACCheckBox_->SetValue(true);
		pUniqueIVCheckBox_->SetValue(true);
		pChainedIVCheckBox_->SetValue(true);
		pExternalIVCheckBox_->SetValue(true);
		pKeyDerivationDurationChoice_->SetStringSelection(wxT("3000"));
	}
	else
	{
		// Expert configuration
		enableDetailWidgets = true;
	}

	pCipherAlgorithmChoice_->Enable(enableDetailWidgets);
	pCipherKeysizeChoice_->Enable(enableDetailWidgets);
	pCipherBlocksizeChoice_->Enable(enableDetailWidgets);
	pNameEncodingChoice_->Enable(enableDetailWidgets);
	pPerBlockHMACCheckBox_->Enable(enableDetailWidgets);
	pUniqueIVCheckBox_->Enable(enableDetailWidgets);
	pChainedIVCheckBox_->Enable(enableDetailWidgets);
	pExternalIVCheckBox_->Enable(enableDetailWidgets);
	pKeyDerivationDurationChoice_->Enable(enableDetailWidgets);
}

void CreateNewEncFSDialog::OnCipherAlgorithmChoice( wxCommandEvent& event )
{
	wxString selectedCipherAlgorithm = pCipherAlgorithmChoice_->GetString( pCipherAlgorithmChoice_->GetSelection() );

	pCipherKeysizeChoice_->Clear();
	pCipherBlocksizeChoice_->Clear();

	encfs::Cipher::AlgorithmList algorithms = encfs::Cipher::GetAlgorithmList();
	encfs::Cipher::AlgorithmList::const_iterator it;
	int optNum = 1;
	for(it = algorithms.begin(); it != algorithms.end(); ++it, ++optNum)
	{
		wxString algoName(it->name.c_str(), *wxConvCurrent);
		if(algoName == selectedCipherAlgorithm)
		{
			wxArrayString keySizes;
			int keySizeMin = it->keyLength.min();
			int keySizeMax = it->keyLength.max();
			int keySizeInc = it->keyLength.inc();
			for(int keySize = keySizeMin; keySize <= keySizeMax; keySize += keySizeInc)
			{
				wxString keySizeStr = wxString::Format(wxT("%d"), keySize);
				keySizes.Add(keySizeStr);
			}
			pCipherKeysizeChoice_->Append(keySizes);
			pCipherKeysizeChoice_->Select(keySizes.GetCount() - 1);

			wxArrayString blockSizes;
			int blockSizeMin = it->blockSize.min();
			int blockSizeMax = it->blockSize.max();
			int blockSizeInc = it->blockSize.inc();
			for(int blockSize = blockSizeMin; blockSize <= blockSizeMax; blockSize += blockSizeInc)
			{
				wxString blockSizeStr = wxString::Format(wxT("%d"), blockSize);
				blockSizes.Add(blockSizeStr);
			}
			pCipherBlocksizeChoice_->Append(blockSizes);
			pCipherBlocksizeChoice_->Select(blockSizes.GetCount() - 1);
		}
	}
}

void CreateNewEncFSDialog::OnCancel( wxCommandEvent& event )
{
	EndModal(wxID_CANCEL);
}

void CreateNewEncFSDialog::OnOK( wxCommandEvent& event )
{
	if (Validate() && TransferDataFromWindow())
	{
		// Make sure mountName_ is not empty
		if(mountName_.IsEmpty())
		{
			wxMessageBox(wxT("The mount name must not be empty!"),
				wxT("Error"), wxOK | wxICON_ERROR);
			return;
		}

		// Make sure mountName_ is unique
		if(pMountList_->findEntryByName(mountName_) != NULL)
		{
			wxMessageBox(wxT("The mount name is not unique! Please choose another name."),
				wxT("Error"), wxOK | wxICON_ERROR);
			return;
		}

#if defined(EFS_WIN32)
		if(pDriveLetterChoice_->GetSelection() == autoChoice_)
		{
			driveLetter_ = L'?';	// Auto
		}
		else if(pDriveLetterChoice_->GetSelection() == noneChoice_)
		{
			driveLetter_ = L'-';	// None (treated by EncFSMP, not PFM)
		}
		else
		{
			wxString selectedDriveLetter = pDriveLetterChoice_->GetStringSelection();
			driveLetter_ = selectedDriveLetter.c_str()[0];
		}
#else
		driveLetter_ = L'?';
#endif

		encFSPath_ = pEncFSPathDirPicker_->GetPath();
		if(!wxFileName::DirExists(encFSPath_))
		{
			wxMessageBox(wxT("EncFS path does not exist! Please enter a valid path"),
				wxT("Error"), wxOK | wxICON_ERROR);
			return;
		}
		wxDir encDir(encFSPath_);
		if(encDir.HasFiles()
			|| encDir.HasSubDirs())
		{
			wxMessageBox(wxT("EncFS path is not empty! Please select an empty path"),
				wxT("Error"), wxOK | wxICON_ERROR);
			return;
		}

		useExternalConfigFile_ = pUseExternalConfigFileCheckBox_->GetValue();
		if(useExternalConfigFile_)
		{
			externalConfigFileName_ = pExternalConfigFileNamePickerCtrl_->GetPath();
			wxFileName externalConfigFN(externalConfigFileName_);
			if(externalConfigFN.FileExists())
			{
				wxMessageBox(wxT("External config file already exists!"),
					wxT("Error"), wxOK | wxICON_ERROR);
				return;
			}

			wxFileName externalConfigPath(externalConfigFN);
			externalConfigPath.SetFullName(wxEmptyString);
			if(!externalConfigPath.IsDirWritable())
			{
				wxMessageBox(wxT("Path of external config file is not writable!"),
					wxT("Error"), wxOK | wxICON_ERROR);
				return;
			}
		}

		isLocalDrive_ = pLocalDriveCheckBox_->GetValue();
		cachingEnabled_ = pEnableCachingCheckBox_->GetValue();

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

		cipherAlgorithm_ = pCipherAlgorithmChoice_->GetStringSelection();
		pCipherKeysizeChoice_->GetStringSelection().ToLong(&cipherKeySize_);
		pCipherBlocksizeChoice_->GetStringSelection().ToLong(&cipherBlockSize_);
		nameEncoding_ = pNameEncodingChoice_->GetStringSelection();
		pKeyDerivationDurationChoice_->GetStringSelection().ToLong(&keyDerivationDuration_);

		EndModal(wxID_OK);
	}
}

BEGIN_EVENT_TABLE( CreateNewEncFSDialog, CreateNewEncFSDialogBase )
END_EVENT_TABLE()
