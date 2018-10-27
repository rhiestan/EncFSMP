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
#include "EncFSMPLogger.h"
#include "EncFSMPErrorLog.h"

#if wxCHECK_VERSION(2, 9, 0) && defined(_WIN32)
#define HAVE_MODE_T		// Workaround for double defined mode_t on Windows
#endif
#include "fs_layer.h"

#include "Error.h"

#include "easylogging++.h"

#include <codecvt>

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

void EncFSMPLogger::log(const std::wstring &errStr, const std::string &fn, encfs::Error *pErr)
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

class EncFSMPLogDispatcher : public el::LogDispatchCallback
{
protected:
	void handle(const el::LogDispatchData* data) noexcept override
	{
		m_data = data;
		// Dispatch using default log builder of logger
		dispatch(m_data->logMessage()->logger()->logBuilder()->build(m_data->logMessage(),
			m_data->dispatchAction() == el::base::DispatchAction::NormalLog));
	}

private:
	const el::LogDispatchData* m_data;
	void dispatch(el::base::type::string_t&& logLine) noexcept
	{
		std::string fn;
		EncFSMPLogger::log(logLine, fn, nullptr);
	}
};

void EncFSMPLogger::setup()
{
	el::Configurations defaultConf;
	defaultConf.setToDefault();
	defaultConf.set(el::Level::Global, el::ConfigurationType::ToFile, "false");
	defaultConf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "false");
	el::Loggers::setDefaultConfigurations(defaultConf, true);

	el::Logger* defaultLogger = el::Loggers::getLogger("default");

	encfs::initLogging(false, false);

	el::Helpers::installLogDispatchCallback<EncFSMPLogDispatcher>("EncFSMPLogDispatcher");

	EncFSMPLogDispatcher* dispatcher = el::Helpers::logDispatchCallback<EncFSMPLogDispatcher>("EncFSMPLogDispatcher");
	dispatcher->setEnabled(true);
}
