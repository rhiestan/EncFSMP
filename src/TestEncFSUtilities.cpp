/**
* Copyright (C) 2026 Roman Hiestand
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

// EncFSUtilities creates/inspects/reconfigures an encfs volume directly on
// disk (it is what CreateNewEncFSDialog/EncFSMPMainFrame call before a volume
// is ever mounted), so it can be tested without a mounted drive.

#include "CommonIncludes.h"

#include "doctest/doctest.h"

#include "EncFSUtilities.h"

#include <boost/filesystem.hpp>

TEST_SUITE("UnitTests")
{
	TEST_CASE("EncFSUtilities: create, inspect, change password and export a volume")
	{
		// Kept tiny so key derivation calibrates almost instantly; never use a
		// value this low for a real volume.
		const long keyDerivationDurationMs = 10;

		boost::system::error_code ec;
		boost::filesystem::path volumePath = boost::filesystem::temp_directory_path()
			/ boost::filesystem::unique_path();
		REQUIRE(boost::filesystem::create_directories(volumePath, ec));

		wxString encFSPath(volumePath.wstring().c_str());
		wxString password(wxT("correct horse battery staple"));

		bool created = EncFSUtilities::createEncFS(encFSPath, password,
			wxEmptyString, false,
			wxT("AES"), 256, 4096, wxT("Block"), keyDerivationDurationMs,
			false, true, true, false);
		REQUIRE(created);

		SUBCASE("getEncFSInfo reads back what was written")
		{
			EncFSUtilities::EncFSInfo info;
			bool ok = EncFSUtilities::getEncFSInfo(encFSPath, wxEmptyString, false, info);
			CHECK(ok);
			// cipherAlgorithm/nameEncoding come back as encfs's internal versioned
			// interface names (e.g. "ssl/aes", "nameio/block"), not the "AES"/
			// "Block" identifiers passed into createEncFS - only check that the
			// requested algorithms were actually found and marked supported.
			CHECK_FALSE(info.cipherAlgorithm.Contains(wxT("NOT supported")));
			CHECK(info.cipherKeySize == 256);
			CHECK(info.cipherBlockSize == 4096);
			CHECK_FALSE(info.nameEncoding.Contains(wxT("NOT supported")));
			CHECK(info.keyDerivationIterations > 0);
			CHECK(info.saltSize > 0);
			CHECK_FALSE(info.perBlockHMAC);
			CHECK(info.uniqueIV);
			CHECK(info.chainedIV);
			CHECK_FALSE(info.externalIV);
		}

		SUBCASE("changePassword rejects the wrong old password")
		{
			wxString errorMsg;
			bool ok = EncFSUtilities::changePassword(encFSPath, wxEmptyString, false,
				wxT("wrong password"), wxT("new password"), errorMsg);
			CHECK_FALSE(ok);
			CHECK(errorMsg == wxT("Invalid old password"));
		}

		SUBCASE("changePassword accepts the correct old password, and the new one then works too")
		{
			wxString errorMsg;
			bool ok = EncFSUtilities::changePassword(encFSPath, wxEmptyString, false,
				password, wxT("new password"), errorMsg);
			CHECK(ok);

			// getEncFSInfo doesn't take a password at all, so the only way to
			// prove the new password was actually persisted is to use it.
			wxString errorMsg2;
			bool ok2 = EncFSUtilities::changePassword(encFSPath, wxEmptyString, false,
				wxT("new password"), wxT("newer password"), errorMsg2);
			CHECK(ok2);
		}

		SUBCASE("exportEncFS extracts an (empty) volume to a plain directory")
		{
			boost::filesystem::path exportPath = boost::filesystem::temp_directory_path()
				/ boost::filesystem::unique_path();

			wxString exportPathWx(exportPath.wstring().c_str());
			wxString errorMsg;
			bool ok = EncFSUtilities::exportEncFS(encFSPath, wxEmptyString, false,
				password, exportPathWx, errorMsg);
			CHECK(ok);
			CHECK(boost::filesystem::is_directory(exportPath));

			boost::filesystem::remove_all(exportPath, ec);
		}

		boost::filesystem::remove_all(volumePath, ec);
	}
}
