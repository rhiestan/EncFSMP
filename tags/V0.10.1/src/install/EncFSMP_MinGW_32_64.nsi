; EncFSMP.nsi
;
; This is an installer script for the Nullsoft Installer System (NSIS).
; See http://nsis.sourceforge.net/
;
;--------------------------------

; Use Modern UI 2
!include MUI2.nsh

!include "x64.nsh"
!include "LogicLib.nsh"

;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\hiesti.ch\EncFSMP" ""

!searchparse /file ..\version.h "#define ENCFSMP_VERSION_MAJOR " VER_MAJOR
!searchparse /file ..\version.h "#define ENCFSMP_VERSION_MINOR " VER_MINOR
!searchparse /file ..\version.h "#define ENCFSMP_VERSION_BUILD " VER_BUILD
!searchparse /file ..\version.h '#define ENCFSMP_NAME "' APP_NAME '"'
!searchparse /file ..\version.h '#define ENCFSMP_COPYRIGHT_NAME "' COMPANY_NAME '"'
!searchparse /file ..\version.h '#define ENCFSMP_COPYRIGHT_YEAR "' COPYRIGHT_YEAR '"'

; The name of the installer
Name "EncFS MP ${VER_MAJOR}.${VER_MINOR}.${VER_BUILD}"

; The Installer to create
OutFile "..\..\EncFSMP_${VER_MAJOR}.${VER_MINOR}.${VER_BUILD}_Setup.exe"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

XPStyle on

; Set best compression
SetCompressor lzma

; Set file and version information in the installer itself
VIAddVersionKey "ProductName" "EncFSMP_Setup"
VIAddVersionKey "Comments" "Installer for ${APP_NAME}"
VIAddVersionKey "CompanyName" "${COMPANY_NAME}"
VIAddVersionKey "LegalCopyright" "Copyright (C) ${COPYRIGHT_YEAR} ${COMPANY_NAME}"
VIAddVersionKey "FileDescription" "Installer for ${APP_NAME}"
VIAddVersionKey "FileVersion" "${VER_MAJOR}.${VER_MINOR}.${VER_BUILD}"
VIProductVersion "${VER_MAJOR}.${VER_MINOR}.${VER_BUILD}.0"

; Do not automatically jump to the finish page, to allow the user to check the install log.
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp"

Var StartMenuFolder

;--------------------------------

; Pages

!insertmacro MUI_PAGE_LICENSE ..\licenses\Copyright.txt
!insertmacro MUI_PAGE_DIRECTORY

;Start Menu Folder Page Configuration
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\hiesti.ch\EncFSMP" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "EncFSMP"
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH


!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------

; The section for the main program, 32 bit version
Section "Main program 32" Section_Main32

	SectionIn 1

	; Set output path to the installation directory.
	SetOutPath $INSTDIR

	; Store install path in registry
	WriteRegStr HKCU "Software\hiesti.ch\EncFSMP" "" $INSTDIR

	; Put files there
	File ..\..\build_mingw32_msys\EncFSMP.exe
	WriteUninstaller "$INSTDIR\Uninstall.exe"

	; Insert links in start menu
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
		CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
		CreateShortCut "$SMPROGRAMS\$StartMenuFolder\EncFSMP.lnk" "$INSTDIR\EncFSMP.exe"
		CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  	!insertmacro MUI_STARTMENU_WRITE_END

	; Write uninstall routine and some additional info into registry
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "DisplayName" "EncFS MP"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "UninstallString" '"$INSTDIR\Uninstall.exe"'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "QuietUninstallString" "$\"$INSTDIR\Uninstall.exe$\" /S"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "InstallLocation" "$\"$INSTDIR$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "DisplayIcon" "$\"$INSTDIR\EncFSMP.exe$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "Publisher" "${COMPANY_NAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "DisplayVersion" "${VER_MAJOR}.${VER_MINOR}.${VER_BUILD}"
	WriteRegDword HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "NoModify" "1"
	WriteRegDword HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "NoRepair" "1"

SectionEnd

; The section for the main program, 32 bit version
Section "Main program 64" Section_Main64

	SectionIn 1

	; Set output path to the installation directory.
	SetOutPath $INSTDIR

	; Store install path in registry
	WriteRegStr HKCU "Software\hiesti.ch\EncFSMP" "" $INSTDIR

	; Put files there
	File ..\..\build_mingw64_msys\EncFSMP.exe
	WriteUninstaller "$INSTDIR\Uninstall.exe"

	; Insert links in start menu
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
		CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
		CreateShortCut "$SMPROGRAMS\$StartMenuFolder\EncFSMP.lnk" "$INSTDIR\EncFSMP.exe"
		CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  	!insertmacro MUI_STARTMENU_WRITE_END

	; Write uninstall routine and some additional info into registry
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "DisplayName" "EncFS MP"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "UninstallString" '"$INSTDIR\Uninstall.exe"'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "QuietUninstallString" "$\"$INSTDIR\Uninstall.exe$\" /S"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "InstallLocation" "$\"$INSTDIR$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "DisplayIcon" "$\"$INSTDIR\EncFSMP.exe$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "Publisher" "${COMPANY_NAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "DisplayVersion" "${VER_MAJOR}.${VER_MINOR}.${VER_BUILD}"
	WriteRegDword HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "NoModify" "1"
	WriteRegDword HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP" "NoRepair" "1"

SectionEnd

Section "Pismo File Mount" Section_PFM

	SectionIn 1
	
	SetOutPath $TEMP
	
	IfFileExists "$TEMP\pfm-180-encfsmp-win.exe" ErrorPFMFileExists
	DetailPrint "Installing Pismo File Mount"
	File "res\pfm-180-encfsmp-win.exe"
	nsExec::ExecToLog '"$TEMP\pfm-180-encfsmp-win.exe" install'
	Delete "$TEMP\pfm-180-encfsmp-win.exe"

	Return

ErrorPFMFileExists:
	MessageBox MB_OK|MB_ICONEXCLAMATION "Error while extracting Pismo File Mount installation package: File already exists"
SectionEnd


Function .onInit
	${If} ${RunningX64}
		StrCpy $INSTDIR "$PROGRAMFILES64\EncFSMP"
		SetRegView 64
		SectionSetFlags ${Section_Main64} 17
		SectionSetFlags ${Section_Main32} 16
	${Else}
		StrCpy $INSTDIR "$PROGRAMFILES32\EncFSMP"
		SectionSetFlags ${Section_Main64} 16
		SectionSetFlags ${Section_Main32} 17
	${EndIf}
	SectionSetFlags ${Section_PFM} 17
	!insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Section "Uninstall" Uninstall
	RMDir /r "$INSTDIR\licenses"
	Delete "$INSTDIR\Uninstall.exe"
	Delete "$INSTDIR\EncFSMP.exe"
	RMDir "$INSTDIR"

	!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
	Delete "$SMPROGRAMS\$StartMenuFolder\Licenses.lnk"
	Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
	Delete "$SMPROGRAMS\$StartMenuFolder\EncFSMP.lnk"
	RMDir "$SMPROGRAMS\$StartMenuFolder"
	DeleteRegKey /ifempty HKCU "Software\hiesti.ch\EncFSMP"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EncFSMP"

;	MessageBox MB_YESNO "Do you want to uninstall Pismo File Mount as well?" IDNO NoUninstallPFM
	nsExec::ExecToLog '"$WINDIR\pfm.exe" uninstall $WINDIR\pfm-license-encfsmp.txt'

NoUninstallPFM:

SectionEnd

Function un.onInit
	${If} ${RunningX64}
		SetRegView 64
	${EndIf}
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd
