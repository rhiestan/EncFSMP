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

#include "PFMMonitorThread.h"
#include "EncFSMPMainFrame.h"
#include "EncFSMPStrings.h"

#include <map>
#include <string>
#include <stdint.h>

// FSR API types and FsrMalloc_i. All calls go through PFMProxy, which is the
// only translation unit that links against fsrapi.dll (see PFMProxy.cpp).
#include "PFMProxy.h"

// State we remember per mount between two polls of the mount list.
struct MountState
{
	MountState(): statusFlags(0), driveLetter(L' ') { }

	int statusFlags;
	std::wstring mountSourceName;
	std::wstring mountPoint;
	wchar_t driveLetter;
};

PFMMonitorThread::PFMMonitorThread()
	: wxThread(wxTHREAD_JOINABLE),
	mutex_(), sendEvents_(true)
{
}

PFMMonitorThread::~PFMMonitorThread()
{
}

bool PFMMonitorThread::startThread()
{
	wxThreadError err = this->Create();
	if(err != wxTHREAD_NO_ERROR)
		return false;
	err = this->Run();

	return (err == wxTHREAD_NO_ERROR);
}

void PFMMonitorThread::stopThread()
{
	if(this->IsRunning())
	{
		// Entry() checks TestDestroy() once per poll interval.
		Delete();
	}
}

void PFMMonitorThread::sendEvents(bool sendEvents)
{
	wxMutexLocker lock(mutex_);
	sendEvents_ = sendEvents;
}

wxThread::ExitCode PFMMonitorThread::Entry()
{
	// Interval between mount list queries, in milliseconds. The FSR mount list
	// change pin (FsrRetainMountListChangePin) is not usable here: it is a
	// one-shot callback pin, its poll() never reports a signalled state, and a
	// handler attached with pin_i_attach() fires exactly once and then has to be
	// re-attached, which would still need a full list query to close the race.
	// So the list is polled and compared, which is what the old PFM
	// implementation effectively did as well.
	const int pollIntervalMSecs = 250;

	// Get the main frame window
	EncFSMPMainFrame *pMainFrame = NULL;
	wxWindow *pTopWindow = wxTheApp->GetTopWindow();
	if(pTopWindow != NULL)
	{
		pMainFrame = dynamic_cast<EncFSMPMainFrame *>(pTopWindow);
	}

	// State of the mounts belonging to our formatter, by mount id.
	std::map<int, MountState> knownMounts;

	while(!TestDestroy())
	{
		wxThread::Sleep(pollIntervalMSecs);
		if(TestDestroy())
			break;

		bool sendEvents = true;
		{
			wxMutexLocker lock(mutex_);
			sendEvents = sendEvents_;
		}

		if(!sendEvents)
			continue;

		// Snapshot the current mount list.
		FsrMalloc_i fsrMalloc;
		FsrMountInfo** miList = NULL;
		PT_SIZE_T miCount = PFMProxy::getInstance().fsrMountGetList(&fsrMalloc, &miList, NULL, NULL);

		std::map<int, MountState> currentMounts;

		for(PT_SIZE_T i = 0; i < miCount; ++i)
		{
			FsrMountInfo *mi = miList[i];
			if(mi == NULL)
				continue;

			// Only process mounts belonging to our formatter.
			if(mi->formatterName == NULL
				|| std::string(mi->formatterName) != EncFSMPStrings::formatterName8_)
				continue;

			MountState state;
			state.statusFlags = mi->statusFlags;
			state.mountSourceName = mi->mountSourceName ?
				wxString(mi->mountSourceName, wxConvUTF8).ToStdWstring() : std::wstring();
			state.mountPoint = mi->mountPoint ?
				wxString(mi->mountPoint, wxConvUTF8).ToStdWstring() : std::wstring();
			state.driveLetter = mi->driveLetter ? static_cast<wchar_t>(mi->driveLetter) : L' ';
#if !defined(EFS_WIN32)
			state.driveLetter = L' ';
#endif
			currentMounts[mi->mountId] = state;

			// Report new mounts and status changes.
			std::map<int, MountState>::const_iterator it = knownMounts.find(mi->mountId);
			bool changed = (it == knownMounts.end()) || (it->second.statusFlags != state.statusFlags);

			if(changed && pMainFrame != NULL
				&& (state.statusFlags & (fsrStatusFlagReady | fsrStatusFlagDisconnected | fsrStatusFlagClosed)) != 0)
			{
				bool isMountEvent = ((state.statusFlags & (fsrStatusFlagDisconnected | fsrStatusFlagClosed)) == 0);
				pMainFrame->addNewMountEvent(isMountEvent, false, state.mountSourceName,
					state.driveLetter, state.mountPoint);
			}
		}

		// A mount that disappears from the list is gone for good. FSR usually
		// removes the entry outright instead of first publishing a disconnected
		// or closed status, so without this the entry would stay "Mounted".
		for(std::map<int, MountState>::const_iterator it = knownMounts.begin();
			it != knownMounts.end(); ++it)
		{
			if(currentMounts.find(it->first) == currentMounts.end() && pMainFrame != NULL)
			{
				pMainFrame->addNewMountEvent(false, false, it->second.mountSourceName,
					it->second.driveLetter, it->second.mountPoint);
			}
		}

		knownMounts.swap(currentMounts);
	}

	return 0;
}
