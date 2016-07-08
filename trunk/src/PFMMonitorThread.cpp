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
#include "PFMProxy.h"
#include "EncFSMPMainFrame.h"
#include "EncFSMPStrings.h"

#if defined(_WIN32)
#	ifndef _INC_WINDOWS
#		define _INC_WINDOWS
#	endif
#endif

#include <stdint.h>

// Pismo File Mount
#include "pfmapi.h"

PFMMonitorThread::PFMMonitorThread()
	: wxThread(wxTHREAD_JOINABLE),
	pfmMonitor_(NULL), mutex_(), sendEvents_(true)
{
}

PFMMonitorThread::~PFMMonitorThread()
{
	if(pfmMonitor_ != NULL)
		pfmMonitor_->Release();
}

bool PFMMonitorThread::startThread()
{
	if(!PFMProxy::getInstance().isPFMPresent())
		return false;
	PfmApi *pfmApi = PFMProxy::getInstance().getPfmApi();
	pfmMonitor_ = NULL;
	pfmApi->MountMonitorFactory(&pfmMonitor_);

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
		pfmMonitor_->Cancel();
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
	const int timeoutMSecs = 50;
	long long startChangeInstance = 0, nextChangeInstance = 0;
	PfmApi *pfmApi = PFMProxy::getInstance().getPfmApi();

	// Get the main frame window
	EncFSMPMainFrame *pMainFrame = NULL;
	wxWindow *pTopWindow = wxTheApp->GetTopWindow();
	if(pTopWindow != NULL)
	{
		pMainFrame = dynamic_cast<EncFSMPMainFrame *>(pTopWindow);
	}

	while(!TestDestroy())
	{
		int retVal = pfmMonitor_->Wait(nextChangeInstance, timeoutMSecs);
		bool sendEvents = true;
		{
			wxMutexLocker lock(mutex_);
			sendEvents = sendEvents_;
		}

		if(sendEvents)
		{
			PfmIterator *iter = NULL;
			pfmApi->MountIterate(startChangeInstance, &nextChangeInstance, &iter);

			if(iter != NULL)
			{
				long long curChangeInstance = 0;
				int mountId = iter->Next(&curChangeInstance);
				while(mountId > 0)
				{
					if(curChangeInstance >= startChangeInstance)
					{
						PfmMount *curMount = NULL;
						int err = pfmApi->MountIdOpen(mountId, &curMount);

						if(err == 0 && curMount != NULL)
						{
							int statusFlags = curMount->GetStatusFlags();

							std::wstring formatterName(curMount->GetFormatterName());
							std::wstring fileName(curMount->GetMountSourceName());
							wchar_t driveLetter = curMount->GetDriveLetter();
#if !defined(EFS_WIN32)
							driveLetter = L' ';
#endif
							std::wstring ownerName(curMount->GetOwnerName());
							std::wstring ownerId(curMount->GetOwnerId());
							std::wstring mountPoint(curMount->GetMountPoint());

							if(formatterName == EncFSMPStrings::formatterName_)
							{
								if((statusFlags & (pfmStatusFlagReady | pfmStatusFlagDisconnected | pfmStatusFlagClosed)) != 0)
								{
									bool isMountEvent = ((statusFlags & (pfmStatusFlagDisconnected | pfmStatusFlagClosed)) == 0);
									pMainFrame->addNewMountEvent(isMountEvent, false, fileName, driveLetter, mountPoint);
								}
							}

							curMount->Release();
						}
					}
					mountId = iter->Next(&curChangeInstance);
				}

				iter->Release();
				startChangeInstance = nextChangeInstance;
			}
			else
			{
				// Workaround for problem, shouldn't happen
#if defined(EFS_WIN32)
				Sleep(50);
#else
				struct timespec tm;
				tm.tv_sec = 0;
				tm.tv_nsec = 50000000L;
				nanosleep(&tm, NULL);
#endif
			}
		}
	}

	return 0;
}
