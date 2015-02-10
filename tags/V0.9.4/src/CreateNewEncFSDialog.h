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

#ifndef CREATENEWENCFSDIALOG_H
#define CREATENEWENCFSDIALOG_H

class MountList;

#include "EncFSMPMainFrameBase.h"

class CreateNewEncFSDialog: public CreateNewEncFSDialogBase
{
public:
	CreateNewEncFSDialog(wxWindow* parent);
	virtual ~CreateNewEncFSDialog();

	void setMountList(MountList *pMountList);

	wchar_t getDriveLetter() const { return driveLetter_; }
	wxString getEncFSPath() const { return encFSPath_; }
	wxString getCipherAlgorithm() const { return cipherAlgorithm_; }
	long getCipherKeySize() const { return cipherKeySize_; }
	long getCipherBlockSize() const { return cipherBlockSize_; }
	wxString getNameEncoding() const { return nameEncoding_; }
	long getKeyDerivationDuration() const { return keyDerivationDuration_; }

protected:
	virtual void OnEncFSConfigurationRadioBox( wxCommandEvent& event );
	virtual void OnCipherAlgorithmChoice( wxCommandEvent& event );
	virtual void OnCancel( wxCommandEvent& event );
	virtual void OnOK( wxCommandEvent& event );

private:
	MountList *pMountList_;

	wchar_t driveLetter_;
	wxString encFSPath_;
	wxString cipherAlgorithm_;
	long cipherKeySize_;
	long cipherBlockSize_;
	wxString nameEncoding_;
	long keyDerivationDuration_;

	DECLARE_EVENT_TABLE()
};

#endif
