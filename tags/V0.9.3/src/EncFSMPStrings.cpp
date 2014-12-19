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
#include "EncFSMPStrings.h"


std::wstring EncFSMPStrings::formatterName_(L"encFSMP");
std::string EncFSMPStrings::formatterName8_("encFSMP");

wxString EncFSMPStrings::configAppName_(wxT("EncFSMP"));
wxString EncFSMPStrings::configOrganizationName_(wxT("hiesti.ch"));
wxString EncFSMPStrings::configMountListPath_(wxT("/MountList"));

wxString EncFSMPStrings::configNameKey_(wxT("Name"));
wxString EncFSMPStrings::configEncFSPathKey_(wxT("EncFSPath"));
wxString EncFSMPStrings::configDriveLetterKey_(wxT("DriveLetter"));
wxString EncFSMPStrings::configPasswordKey_(wxT("Password"));
wxString EncFSMPStrings::configIsWorldWritableKey_(wxT("IsWorldWritable"));
wxString EncFSMPStrings::configIsSystemVisibleKey_(wxT("IsSystemVisible"));
wxString EncFSMPStrings::configWindowDimensions_(wxT("WindowDimensions"));
wxString EncFSMPStrings::configColumnWidths_(wxT("ColumnWidths"));
wxString EncFSMPStrings::configMinimizeToTray_(wxT("MinimizeToTray"));
wxString EncFSMPStrings::configDisableUnmountDialogOnExit_(wxT("DisableUnmountDialogOnExit"));
wxString EncFSMPStrings::configShowErrorLogOnErr_(wxT("ShowErrorLogOnErr"));
