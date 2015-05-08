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

#include "MountList.h"
#include "EncFSMPStrings.h"

#include <wx/config.h>
#include <memory>

MountList::MountList()
{
}

MountList::~MountList()
{
}

bool MountList::addMount(wxString name, wxString encFSPath, wxString driveLetter,
	wxString password, bool isWorldWritable, bool isMounted)
{
	wxString correctedName = name;

	// Remove any paths in front of name
	wxFileName nameFN(correctedName);
	correctedName = nameFN.GetFullName();

	// Check for double entry
	if(findEntryByName(correctedName) != NULL)
		return false;

	MountEntry newEntry;
	newEntry.name_ = correctedName;
	newEntry.encFSPath_ = encFSPath;
	newEntry.driveLetter_ = driveLetter;
	newEntry.password_ = password;
	newEntry.isWorldWritable_ = isWorldWritable;
	newEntry.mountState_ = (isMounted ? MountEntry::MSMounted : MountEntry::MSNotMounted);

	mountEntries_.push_back(newEntry);

	return true;
}

MountEntry *MountList::findEntryByName(wxString name)
{
	wxString correctedName = name;
	
	// Remove any paths in front of name
	wxFileName nameFN(correctedName);
	correctedName = nameFN.GetFullName();

	std::list<MountEntry>::iterator iter = mountEntries_.begin();
	while(iter != mountEntries_.end())
	{
		MountEntry &cur = (*iter);

		if(cur.name_ == correctedName)
			return &cur;

		iter++;
	}

	return NULL;
}


bool MountList::storeToConfig()
{
	std::auto_ptr<wxConfig> config(new wxConfig(EncFSMPStrings::configAppName_, EncFSMPStrings::configOrganizationName_));
	config->DeleteGroup(EncFSMPStrings::configMountListPath_);
	config->SetPath(EncFSMPStrings::configMountListPath_);

	int i = 0;
	std::list<MountEntry>::iterator iter = mountEntries_.begin();
	while(iter != mountEntries_.end())
	{
		MountEntry &cur = (*iter);

		wxString grpName = wxString::Format(wxT("MountEntry%d"), i);
		config->SetPath(grpName);

		if(!config->Write(EncFSMPStrings::configNameKey_, cur.name_))
			return false;
		config->Write(EncFSMPStrings::configEncFSPathKey_, cur.encFSPath_);
		config->Write(EncFSMPStrings::configDriveLetterKey_, cur.driveLetter_);
		if(cur.password_.Length() > 0)
			config->Write(EncFSMPStrings::configPasswordKey_, cur.password_);
		config->Write(EncFSMPStrings::configIsWorldWritableKey_, cur.isWorldWritable_);

		config->SetPath(wxT(".."));

		iter++;
		i++;
	}

	return true;
}

bool MountList::loadFromConfig()
{
	long index = 0;
	wxString str;

	std::auto_ptr<wxConfig> config(new wxConfig(EncFSMPStrings::configAppName_, EncFSMPStrings::configOrganizationName_));
	config->SetPath(EncFSMPStrings::configMountListPath_);
	bool isOK = config->GetFirstGroup(str, index);
	while(isOK)
	{
		config->SetPath(str);
		MountEntry cur;

		if(!config->Read(EncFSMPStrings::configNameKey_, &cur.name_))
			return false;
		if(!config->Read(EncFSMPStrings::configEncFSPathKey_, &cur.encFSPath_))
			return false;
		config->Read(EncFSMPStrings::configDriveLetterKey_, &cur.driveLetter_);
		config->Read(EncFSMPStrings::configPasswordKey_, &cur.password_);
		config->Read(EncFSMPStrings::configIsWorldWritableKey_, &cur.isWorldWritable_);

		cur.assignedDriveLetter_ = wxEmptyString;
		cur.mountState_ = MountEntry::MSNotMounted;

		mountEntries_.push_back(cur);

		config->SetPath(wxT(".."));
		isOK = config->GetNextGroup(str, index);
	}

	return true;
}
