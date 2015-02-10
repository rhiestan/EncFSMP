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

#ifndef EncFSMPStrings_H
#define EncFSMPStrings_H

#include <wx/string.h>

class EncFSMPStrings
{
public:
	const static std::wstring formatterName_;
	const static std::string formatterName8_;

	const static wxString configAppName_;
	const static wxString configOrganizationName_;
	const static wxString configMountListPath_;
	const static wxString configNameKey_;
	const static wxString configEncFSPathKey_;
	const static wxString configDriveLetterKey_;
	const static wxString configPasswordKey_;
	const static wxString configIsWorldWritableKey_;
	const static wxString configIsSystemVisibleKey_;
	const static wxString configWindowDimensions_;
	const static wxString configColumnWidths_;
	const static wxString configMinimizeToTray_;
	const static wxString configDisableUnmountDialogOnExit_;
	const static wxString configShowErrorLogOnErr_;
	const static wxString configSavePasswordsInRAM_;

	const static wxString commandMount_;
	const static wxString commandUnmount_;
	const static wxString commandDDETopic_;
	const static wxString commandDDEServerName_;

private:
	EncFSMPStrings() { }
	virtual ~EncFSMPStrings() { }
};

#endif
