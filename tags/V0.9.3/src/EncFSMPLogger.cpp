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
#include "EncFSMPLogger.h"
#include "EncFSMPErrorLog.h"

#if wxCHECK_VERSION(2, 9, 0) && defined(_WIN32)
#define HAVE_MODE_T		// Workaround for double defined mode_t on Windows
#endif
#include "fs_layer.h"

// rlog
#include "rlog/Error.h"

// Static member
EncFSMPLogger EncFSMPLogger::instance_;

EncFSMPLogger::EncFSMPLogger()
	: pEncFSMPErrorLog_(NULL)
{
}

EncFSMPLogger::~EncFSMPLogger()
{
}

void EncFSMPLogger::setErrorLog(EncFSMPErrorLog *pEncFSMPErrorLog)
{
	instance_.pEncFSMPErrorLog_ = pEncFSMPErrorLog;
}

void EncFSMPLogger::log(const std::wstring &errStr, const std::string &fn, rlog::Error *pErr)
{
	if(instance_.pEncFSMPErrorLog_ != NULL)
	{
		wxString text(errStr);
		if(!fn.empty())
		{
			boost::filesystem::path path(fs_layer::stringToFSPath(fn));
			wxString pathWx(path.wstring());
			if(!text.empty())
				text.Append(wxT(", "));
			text.Append(wxT("file: "));
			text.Append(pathWx);
		}
		if(pErr != NULL)
		{
			text.Append(wxT(", details: "));
			text.Append(wxString(pErr->what(), *wxConvCurrent));
		}
		text.Append(wxT("\n"));
		instance_.pEncFSMPErrorLog_->addText(text);
	}
}
