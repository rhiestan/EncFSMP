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

#ifndef ENCFSUTILITIES_H
#define ENCFSUTILITIES_H

class EncFSUtilities
{
public:
	EncFSUtilities();
	virtual ~EncFSUtilities();

	static bool createEncFS(const wxString &encFSPath, const wxString &password,
		const wxString &externalConfigFileName, bool useExternalConfigFile,
		const wxString &cipherAlgorithm, long cipherKeySize, long cipherBlockSize,
		const wxString &nameEncoding, long keyDerivationDuration,
		bool perBlockHMAC, bool uniqueIV, bool chainedIV, bool externalIV);

	struct EncFSInfo
	{
		wxString configVersionString;
		wxString cipherAlgorithm;
		long cipherKeySize, cipherBlockSize;
		wxString nameEncoding;
		long keyDerivationIterations, saltSize;
		bool perBlockHMAC, uniqueIV, chainedIV, externalIV, allowHoles;
	};

	static bool getEncFSInfo(const wxString &encFSPath, const wxString &externalConfigFileName,
		bool useExternalConfigFile, EncFSInfo &info);

	static bool changePassword(const wxString &encFSPath,
		const wxString &externalConfigFileName, bool useExternalConfigFile,
		const wxString &oldPassword, const wxString &newPassword,
		wxString &errorMsg);

	static bool exportEncFS(const wxString &encFSPath,
		const wxString &externalConfigFileName, bool useExternalConfigFile,
		const wxString &password, const wxString &exportPath, wxString &errorMsg);

	static std::string wxStringToEncFSPath(const wxString &path);

	static std::string wxStringToEncFSFile(const wxString &fn);
};

#endif
