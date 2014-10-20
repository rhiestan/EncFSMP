// The following code is partly taken from the example source code CppUACSelfElevation.cpp
// Small changes by Roman Hiestand:
// - removed goto
// - removed exceptions
// - bool instead of BOOL
// The method getShieldIcon() is (c) Roman Hiestand

/**
Copyright (c) Microsoft Corporation.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "CommonIncludes.h"

#include "Win32Utils.h"


bool Win32Utils::hasUAC()
{
	// Windows Vista and later
	OSVERSIONINFO osver = { sizeof(osver) };
	if (GetVersionEx(&osver) && osver.dwMajorVersion >= 6)
		return true;

	return false;
}

bool Win32Utils::isRunAsAdmin()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(
		&NtAuthority, 
		2, 
		SECURITY_BUILTIN_DOMAIN_RID, 
		DOMAIN_ALIAS_RID_ADMINS, 
		0, 0, 0, 0, 0, 0, 
		&pAdministratorsGroup))
	{
		dwError = GetLastError();
	}

	if (ERROR_SUCCESS == dwError)
	{
		// Determine whether the SID of administrators group is enabled in 
		// the primary access token of the process.
		if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
		{
			dwError = GetLastError();
		}
	}

	// Centralized cleanup for all allocated resources.
	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	if (ERROR_SUCCESS != dwError)
	{
		return false;
	}

	return (fIsRunAsAdmin != 0);
}

/**
 * Restart the current process as admin.
 *
 * This method asks the user to run the current process with administrator rights.
 * If the user allows, the process is restarted with admin rights, the method returns true.
 * In that case, immediately close the program.
 * If the user does not allow, the method returns false.
 */
bool Win32Utils::restartAsAdmin(HWND hWnd)
{
	bool isOK = false;
	wchar_t szPath[MAX_PATH];
	if (GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		// Launch itself as administrator.
		SHELLEXECUTEINFO sei = { sizeof(sei) };
		sei.lpVerb = L"runas";
		sei.lpFile = szPath;
		sei.hwnd = hWnd;
		sei.nShow = SW_NORMAL;

		if (!ShellExecuteEx(&sei))
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_CANCELLED)
			{
				// The user refused the elevation.
				// Do nothing ...
			}
		}
		else
		{
			//EndDialog(hWnd, TRUE);  // Quit itself
			isOK = true;
		}
	}

	return isOK;
}

// For other compilers than Microsoft Visual Studio, define EFS_DEFINE_STOCKICONINFO on command line if necessary
#if defined(_MSC_VER)
#	if (NTDDI_VERSION < NTDDI_VISTA)
#		define EFS_DEFINE_STOCKICONINFO
#	endif
#endif

// Since _WIN32_WINNT is set to 0x0502 (WinXP SP2) in CommonIncludes.h, the following defines/typedefs were not made:
#if defined(EFS_DEFINE_STOCKICONINFO)
typedef struct _SHSTOCKICONINFO
{
    DWORD cbSize;
    HICON hIcon;
    int   iSysImageIndex;
    int   iIcon;
    WCHAR szPath[MAX_PATH];
} SHSTOCKICONINFO;

typedef enum SHSTOCKICONID { 
  SIID_SHIELD             = 77
} SHSTOCKICONID;

#define SHGSI_ICON              0x0100
#define SHGSI_SMALLICON         0x01
#endif

/**
 * Get the shield icon for UAC.
 *
 * This method returns the small shield icon for UAC
 * on Vista and above. To be able to run this app also on
 * previous Windows versions, the function pointer to
 * SHGetStockIconInfo is obtained via GetProcAddress
 * (i.e. no link-time dependency).
 */
bool Win32Utils::getShieldIcon(wxBitmap &shieldBM)
{
	HMODULE	hShellDLL;
	HRESULT	hr;
	bool isOK = true;

	HRESULT	(CALLBACK* pfnSHGetStockIconInfo)(SHSTOCKICONID siid, UINT uFlags, SHSTOCKICONINFO *psii);

	hShellDLL = ::LoadLibraryW(L"shell32.dll");
	if(hShellDLL == NULL)
		return false;

	(*(FARPROC*)&pfnSHGetStockIconInfo) = ::GetProcAddress(hShellDLL, "SHGetStockIconInfo");
	if(pfnSHGetStockIconInfo)
	{
		SHSTOCKICONINFO stockInfo;
		stockInfo.cbSize = sizeof(SHSTOCKICONINFO);
		hr = pfnSHGetStockIconInfo(SIID_SHIELD, SHGSI_ICON | SHGSI_SMALLICON, &stockInfo);
		if(hr == S_OK)
		{
			wxIcon uacShieldIcon;
#if wxCHECK_VERSION(2, 9, 0)
			uacShieldIcon.CreateFromHICON(stockInfo.hIcon);
#endif
			shieldBM.CopyFromIcon(uacShieldIcon);
			::DestroyIcon(stockInfo.hIcon);
		}
		else
			isOK = false;

	}
	else
		isOK = false;

	::FreeLibrary(hShellDLL);

	return isOK;
}
