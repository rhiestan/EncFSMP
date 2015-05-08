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

#include "PFMProxy.h"
#include "PFMMonitorThread.h"

#if defined(_WIN32)
#	ifndef _INC_WINDOWS
#		define _INC_WINDOWS
#	endif
#endif


// Pismo File Mount
#include "pfmapi.h"
#include "pfmmarshaller.h"

PFMProxy PFMProxy::instance_;

PFMProxy::PFMProxy():
	isInitialized_(false), isPFMPresent_(false),
	pfmApi_(NULL), pPFMMonitorThread_(NULL)
{
}

PFMProxy::~PFMProxy()
{
}

void PFMProxy::initialize()
{
	if(!isInitialized_)
	{
		pfmApi_ = NULL;
		int err = PfmApiFactory(&pfmApi_);

		if(pfmApi_ != NULL)
		{
			isPFMPresent_ = true;
			const char *versionStr = pfmApi_->Version();
			pfmVersionString_ = std::string(versionStr);
		}

		isInitialized_ = true;
	}
}

void PFMProxy::startMonitorThread()
{
	if(!isPFMPresent_)
		return;
	if(pPFMMonitorThread_ != NULL)
		return;

	pPFMMonitorThread_ = new PFMMonitorThread();
	if(!pPFMMonitorThread_->startThread())
	{
		delete pPFMMonitorThread_;
		pPFMMonitorThread_ = NULL;
	}
}

void PFMProxy::closingDown()
{
	// Tell the monitor thread to stop sending events when application is closing down
	if(pPFMMonitorThread_ != NULL)
		pPFMMonitorThread_->sendEvents(false);
}

void PFMProxy::uninitialize()
{
	if(pPFMMonitorThread_ != NULL)
	{
		pPFMMonitorThread_->stopThread();
		delete pPFMMonitorThread_;
		pPFMMonitorThread_ = NULL;
	}

	if(pfmApi_ != NULL)
		pfmApi_->Release();
	pfmApi_ = NULL;

	PfmApiUnload();

	isInitialized_ = false;
}

bool PFMProxy::unmount(const wxString &mountName)
{
	bool isOK = false;
	PfmIterator *iter = NULL;
	long long nextChangeInstance = 0;
	pfmApi_->MountIterate(0, &nextChangeInstance, &iter);

	long long curChangeInstance = 0;
	int mountId = iter->Next(&curChangeInstance);
	while(mountId > 0)
	{
		PfmMount *curMount = NULL;
		int err = pfmApi_->MountIdOpen(mountId, &curMount);
		if(err == 0 && curMount != NULL)
		{
			// Remove any paths in front of name
			wxFileName nameFN(curMount->GetMountSourceName());
			wxString correctedName = nameFN.GetFullName();

			if(correctedName == mountName)
			{
				curMount->Unmount(0);
				isOK = true;
			}

			curMount->Release();
		}

		mountId = iter->Next(&curChangeInstance);
	}
	iter->Release();

	return isOK;
}
