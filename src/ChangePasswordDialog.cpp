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
#include "ChangePasswordDialog.h"


ChangePasswordDialog::ChangePasswordDialog(wxWindow *parent)
	: ChangePasswordDialogBase(parent)
{
}

ChangePasswordDialog::~ChangePasswordDialog()
{
}

void ChangePasswordDialog::OnOKButtonClick( wxCommandEvent& event )
{
	if (Validate() && TransferDataFromWindow())
	{
		if(newPassword_.Length() == 0)
		{
			wxMessageBox(wxT("Empty password is not allowed!"),
				wxT("Error"), wxOK | wxICON_ERROR);
			return;
		}
		if(newPassword_ != retypeNewPassword_)
		{
			wxMessageBox(wxT("The retyped new password is not equal to the first one!"),
				wxT("Error"), wxOK | wxICON_ERROR);
			return;
		}

		EndModal(wxID_OK);
	}
}

BEGIN_EVENT_TABLE( ChangePasswordDialog, ChangePasswordDialogBase )
END_EVENT_TABLE()
