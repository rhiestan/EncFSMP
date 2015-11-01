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

#ifndef OPENEXISTINGFSDIALOG_H
#define OPENEXISTINGFSDIALOG_H

class MountList;

#include "EncFSMPMainFrameBase.h"

class OpenExistingFSDialog: public OpenExistingFSDialogBase
{
public:
	OpenExistingFSDialog(wxWindow *parent);
	virtual ~OpenExistingFSDialog();

	void setMountList(MountList *pMountList);
	void setEditMode(const wxString &mountName,
		const wxString &encFSPath,
		bool useExternalConfigFile, wxString externalConfigFileName,
		wchar_t driveLetter, bool isLocalDrive,
		const wxString &password, bool isWorldWritable,
		bool cachingEnabled);

	bool getUseExternalConfigFile() const { return useExternalConfigFile_; }
	wxString getExternalConfigFileName() const { return externalConfigFileName_; }
	wchar_t getDriveLetter() const { return driveLetter_; }
	wxString getEncFSPath() const { return encFSPath_; }
	bool getIsLocalDrive() const { return isLocalDrive_; }
	bool getCachingEnabled() const { return cachingEnabled_; }

protected:
	virtual void OnInitDialog( wxInitDialogEvent& event );
	virtual void OnUseExternalConfigFileCheckBox( wxCommandEvent& event );
	virtual void OnStorePasswordCheckBox( wxCommandEvent& event );
	virtual void OnCancelButton( wxCommandEvent& event );
	virtual void OnOKButton( wxCommandEvent& event );

private:
	MountList *pMountList_;

	bool editMode_;
	bool useExternalConfigFile_;
	wxString externalConfigFileName_;
	wchar_t driveLetter_;
	wxString encFSPath_;
	bool isLocalDrive_;
	bool cachingEnabled_;

	int autoChoice_, noneChoice_;

	DECLARE_EVENT_TABLE()
};


#endif
