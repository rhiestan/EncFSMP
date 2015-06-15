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

#ifndef MOUNTLIST_H
#define MOUNTLIST_H

#include <list>

/**
 * Data-only class for holding information about a mount.
 */
class MountEntry
{
public:

	enum MountState
	{
		MSNotMounted = 0, MSPending, MSMounted
	};

	MountEntry()
		: isWorldWritable_(false), isLocalDrive_(true),
		mountState_(MSNotMounted)
	{ }
	MountEntry(const MountEntry &o) { copy(o); }
	virtual ~MountEntry() { }
	MountEntry &copy(const MountEntry &o)
	{
		name_ = o.name_;
		encFSPath_ = o.encFSPath_;
		driveLetter_ = o.driveLetter_;
		assignedDriveLetter_ = o.assignedDriveLetter_;
		password_ = o.password_;
		assignedMountPoint_ = o.assignedMountPoint_;
		volatilePassword_ = o.volatilePassword_;
		isWorldWritable_ = o.isWorldWritable_;
		isLocalDrive_ = o.isLocalDrive_;
		mountState_ = o.mountState_;
		return *this;
	}
	MountEntry & operator=(const MountEntry & o)
	{
		return copy(o);
	}

	wxString name_, encFSPath_, driveLetter_, assignedDriveLetter_, password_;
	wxString assignedMountPoint_, volatilePassword_;	// Not persistent attributes
	bool isWorldWritable_, isLocalDrive_;
	MountState mountState_;
};


class MountList
{
public:
	MountList();
	virtual ~MountList();

	bool addMount(wxString name, wxString encFSPath, wxString driveLetter,
		wxString password, bool isWorldWritable, bool isLocalDrive, bool isMounted);

	std::list<MountEntry> &getList() { return mountEntries_; }

	MountEntry *findEntryByName(wxString name);

	bool storeToConfig();
	bool loadFromConfig();

private:
	std::list<MountEntry> mountEntries_;
};

#endif
