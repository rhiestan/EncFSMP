// The following code is partly taken from the example source code CppUACSelfElevation.cpp
// Small changes by Roman Hiestand

/**
Copyright (c) Microsoft Corporation.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
*/
#ifndef WIN32UTILS_H
#define WIN32UTILS_H

class Win32Utils
{
public:

	static bool hasUAC();
	static bool isRunAsAdmin();
	static bool restartAsAdmin(HWND hWnd);
	static bool getShieldIcon(wxBitmap &shieldBM);

private:
	Win32Utils() { }
	virtual ~Win32Utils() { }
};

#endif
