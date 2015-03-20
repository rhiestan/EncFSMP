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
		const wxString &encFSPath, wchar_t driveLetter,
		const wxString &password, bool isWorldWritable,
		bool isSystemVisible);

	wchar_t getDriveLetter() const { return driveLetter_; }
	wxString getEncFSPath() const { return encFSPath_; }

protected:
	virtual void OnInitDialog( wxInitDialogEvent& event );
	virtual void OnStorePasswordCheckBox( wxCommandEvent& event );
	virtual void OnCancelButton( wxCommandEvent& event );
	virtual void OnOKButton( wxCommandEvent& event );

private:
	MountList *pMountList_;

	bool editMode_;
	wchar_t driveLetter_;
	wxString encFSPath_;

	DECLARE_EVENT_TABLE()
};


#endif
