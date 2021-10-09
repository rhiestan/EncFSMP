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

#include "EncFSMPTestApp.h"
#include "TestBigFile.h"
#include "TestOpenFileTwice.h"
#include "TestManyFiles.h"

#if defined(_WIN32)
#include "TestFileWin32.h"
#endif

#include <wx/cmdline.h>
#include <wx/stdpaths.h>
#include <wx/txtstrm.h>

#include "version.h"

 // EncFS
#if wxCHECK_VERSION(2, 9, 0) && defined(_WIN32)
#define HAVE_MODE_T		// Workaround for double defined mode_t on Windows
#endif
#include "config.h"

 // OpenSSL
#include "openssl/ssl.h"

 // boost
#include <boost/version.hpp>
#include "gtest/gtest.h"

class TestParameters
{
public:
	static wxString testPath_;
	static boost::filesystem::path testPathBoost_;
};
wxString TestParameters::testPath_;
boost::filesystem::path TestParameters::testPathBoost_;


// Main program equivalent, creating windows and returning main app frame
bool EncFSMPTestApp::OnInit()
{
	wxInitAllImageHandlers();

	::testing::InitGoogleTest(&argc, (wchar_t **)argv);

	// Parse command line
	static const wxCmdLineEntryDesc cmdLineDesc[] =
	{
		{ wxCMD_LINE_PARAM, "test dir", "test directory", "Test directory", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
		{ wxCMD_LINE_NONE }
	};

	wxCmdLineParser parser(argc, argv);
	parser.SetDesc(cmdLineDesc);

	wxString testpath("Z:\\");
	if(parser.Parse(true) == 0)
	{
		// Parse file names given via command line
		size_t ct = parser.GetParamCount();
		for(size_t i = 0; i < ct; i++)
		{
			testpath = parser.GetParam(i);
		}
	}
	else
	{
		std::cout << "Command line parser returned 0" << std::endl;
		return false;
	}

	TestParameters::testPath_ = testpath;
#if defined(_WIN32)
	TestParameters::testPathBoost_ = TestParameters::testPath_.wc_str();
#else
	TestParameters::testPathBoost_ = TestParameters::testPath_.mb_str();
#endif

	// Print out library versions
	std::cout << "Built with: " ENCFSMP_COMPILER " " ENCFSMP_COMPILER_VERSION << std::endl;
	std::cout << "EncFS " EFS_STRINGIFY(VERSION) << std::endl;
	std::cout << OPENSSL_VERSION_TEXT << std::endl;

	wxString boostVersion;
	boostVersion.Printf(wxT("%d.%d.%d"), BOOST_VERSION / 100000, BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100);
	std::cout << "boost " << boostVersion.ToStdString().c_str() << std::endl;

	return true;
}

TEST(FilesTest, TestBigFile)
{
	EXPECT_TRUE(TestBigFile::runTest(TestParameters::testPathBoost_));
}
TEST(FilesTest, TestOpenFileTwice)
{
	EXPECT_TRUE(TestOpenFileTwice::runTest(TestParameters::testPathBoost_));
}
TEST(FilesTest, TestManyFiles)
{
	EXPECT_TRUE(TestManyFiles::runTest(TestParameters::testPathBoost_));
}

#if defined(_WIN32)
TEST(Win32Test, TestPFMRegression)
{
	EXPECT_TRUE(TestFileWin32::checkPFMRegrTest(TestParameters::testPathBoost_));
}
TEST(Win32Test, TestSupersede)
{
	EXPECT_TRUE(TestFileWin32::supersedeTest(TestParameters::testPathBoost_));
}

#endif

int EncFSMPTestApp::OnRun()
{
	return RUN_ALL_TESTS();
}

int EncFSMPTestApp::OnExit()
{
	return 0;
}

IMPLEMENT_APP_CONSOLE(EncFSMPTestApp)
//IMPLEMENT_APP(EncFSMPTestApp)
