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
#include "EncFSMPStrings.h"


const std::wstring EncFSMPStrings::formatterName_(L"encFSMP");
const std::string EncFSMPStrings::formatterName8_("encFSMP");
 
const wxString EncFSMPStrings::configAppName_(wxT("EncFSMP"));
const wxString EncFSMPStrings::configOrganizationName_(wxT("hiesti.ch"));
const wxString EncFSMPStrings::configMountListPath_(wxT("/MountList"));
 
const wxString EncFSMPStrings::configNameKey_(wxT("Name"));
const wxString EncFSMPStrings::configEncFSPathKey_(wxT("EncFSPath"));
const wxString EncFSMPStrings::configDriveLetterKey_(wxT("DriveLetter"));
const wxString EncFSMPStrings::configPasswordKey_(wxT("Password"));
const wxString EncFSMPStrings::configIsWorldWritableKey_(wxT("IsWorldWritable"));
const wxString EncFSMPStrings::configIsLocalDriveKey_(wxT("IsLocalDrive"));
const wxString EncFSMPStrings::configWindowDimensions_(wxT("WindowDimensions"));
const wxString EncFSMPStrings::configColumnWidths_(wxT("ColumnWidths"));
const wxString EncFSMPStrings::configMinimizeToTray_(wxT("MinimizeToTray"));
const wxString EncFSMPStrings::configDisableUnmountDialogOnExit_(wxT("DisableUnmountDialogOnExit"));
const wxString EncFSMPStrings::configShowErrorLogOnErr_(wxT("ShowErrorLogOnErr"));
const wxString EncFSMPStrings::configSavePasswordsInRAM_(wxT("SavePasswordsInRAM"));

const wxString EncFSMPStrings::commandMount_(wxT("mount"));
const wxString EncFSMPStrings::commandUnmount_(wxT("unmount"));
const wxString EncFSMPStrings::commandDDETopic_(wxT("EncFSMP_Command"));
const wxString EncFSMPStrings::commandDDEServerName_(wxT("EncFSMP_DDEServer"));
