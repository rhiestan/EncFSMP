EncFSMP project
===============

This is the git project for EncFSMP, the tool to mount encfs folders on Windows and Mac OS.

## How to compile ##

This is a guide to compile EncFSMP using Visual Studio 2015 Community edition. All tools and libraries are available for free on the internet.
In order to complete these steps, you need some experience with C++, CMake and Visual Studio. Unfortunately, I am not able to give support to build EncFSMP due to my limited spare time.

- Please go to my project "Build scripts" ([link](https://github.com/rhiestan/build-scripts)) and download the latest release.
- Follow the instructions in the README of this project to compile zlib, jpeg, png, tiff, bzip2, boost, openssl, wxwidgets, gtest (in this order). The other libraries are not required for EncFSMP.
- Download the latest PFM developer kit from the [PismoTech homepage](http://pismotec.com/download/) and extract it.
- Download and extract the latest source release of EncFSMP
- Copy the file `build_msvc2015_x64_sb_templ.bat` to `build_msvc2015_x64_sb.bat` and adjust the path `SB_SCRIPT_PATH` to the path where you extracted the build scripts, and adjust the path `PFM_ROOT` to the  include folder in the path where you extracted the PFM SDK.
- Run the batch file by double-clicking, it should create a new folder with several files. Double click on EncFSMP.sln.



## Pull requests ##

I will accept pull requests, as long as they fulfill the following requirements:

- High code quality, tested
- Multiplatform-capable (no Windows API, for example)
- Similar code style to rest of the code

Please make pull requests only against develop branch.

I need help in the following areas, in this order:

- Bug fixes (see tickets at SourceForge, [link](https://sourceforge.net/p/encfsmp/tickets/))
- Test cases (see EncFSMPTestApp.cpp)
- New features


## Version history ##

###Version 0.10.7, 02.01.2017:
- Unmount in case of Windows shutdown (bug #56)
- Update OpenSSL to 1.0.2j, boost to 1.62.0

###Version 0.10.6, 01.08.2016:
- Bug fix (Windows only): Mounts without drive letter are now mounted again unter C:\Volumes
- Fixed bug #66: For directories with more than ~1050 files, not all files were displayed
- The password dialog now also shows the mount name


###Version 0.10.5, 08.07.2016:
- Implemented pattern to upgrade from opening a file read-only to read-and-write (should fix bugs with seemingly read-only files)
- Updated OpenSSL to 1.0.2h, boost to 1.61.0. OpenSSL is now compiled to use assembly language and AES instructions, if present
- Updated PFM to 1.0.0.183, using new API


###Version 0.10.4, 17.04.2016:
- Fixed another bug with read-only files, fixes error reports like "...RawFileIO.cpp:257 -- true == canWrite"
- Properly implemented Access method, should fix ticket #55 (and maybe other problems on OS X)
- Updated OpenSSL to 1.0.2g
- Updated PFM to 1.0.0.181 (should fix ticket #50 and #53)


###Version 0.10.3, 22.02.2016:
- Fixed bug with opening files as read-only
- Updated OpenSSL to 1.0.2f, boost to 1.60.0


###Version 0.10.2, 07.01.2016:
- Fixed directory listing of directories with more than 1100 entries (ticket #43)


###Version 0.10.1, 27.12.2015:
- Date bug fixed in 32 bit version (ticket #40)
- Updated PFM to 1.0.0.180 (fixes ticket #34)
- Updated OpenSSL to 1.0.2e


###Version 0.10.0, 01.11.2015:
- Added support for external encfs-configs
- Improved speed with optional cache and better data structures


###Version 0.9.9, 02.08.2015:
- Updated OpenSSL to 1.0.2d
- Updated Pismo File Mount to 1.0.0.178

###Version 0.9.8, 15.06.2015:
- Updated OpenSSL to 1.0.2c
- Fixed bug with handling last write time, fixes ticket #25
- Improved speed while reading directories on Windows
- Windows: Added option to mount drives as Local or Network drives
- Windows: Added option not to mount a new volume using a drive letter (only under C:\Volumes)

###Version 0.9.7, 08.05.2015:
- Updated Pismo File Mount to 1.0.0.177
- Windows: Removed "System visible" flag for mounts and "Restart as Admin" menu
- Fixed ticket #13 and #16: Mounted Volumes are now local under Windows
- Fixed ticket #14: Improved handling of read-only files
- Fixed ticket #15: Dialogs now also work on high-DPI devices


###Version 0.9.6, 20.03.2015
- Updated OpenSSL to 1.0.2a
- Fixed bug in acquiring a mutex in MemoryPool, see ticket #12 for details


### Version 0.9.5, 16.03.2015:
- Support for files > 2 GB
- Fixed possible problem in creating new directories with wrong permissions


### Version 0.9.4, 10.02.2015:
- Updated Pismo File Mount to 1.0.0.173
- Updated OpenSSL to 1.0.2
- Passwords can now be saved in RAM (while EncFSMP is running)
- Mount/unmount via command line
- Improved speed of listing a directory


### Version 0.9.3, 19.12.2014:
- Added possibility to disable unmount confirmation dialog on exit
- Improved error handling, added error log window
- Fixed bug: encfs folders in paths with Unicode characters are now supported
- Fixed bug: Deletion of directories sometimes didn't work
- Files with name .encfs* are hidden only in root directory
- Updated libpng to 1.6.15 and boost to 1.57.0

Thanks for A. Večeřa for reporting the bugs!

### Version 0.9.2, 20.10.2014:
- Added context menu in mounts list
- Updated wxWidgets to 3.0.2 and OpenSSL to 1.0.1j
- Minor code changes, EncFSMP can now be compiled against wxWidgets < 2.9.0

### Version 0.9.1, 10.10.2014:
- Fixed a problem where EncFS on Linux was not able to open EncFS folders created with EncFSMP
- Added feature "Minimize to tray" (Windows)/"Minimize to menu icon" (Mac OS X)
- Dialogs for creating new/opening existing EncFS folders slightly improved

### Version 0.9, 06.10.2014:

Initial release (beta status)


System requirements
-------------------

For Windows 32 bit executables:

* Windows (tested with Windows XP and up)

For Windows 64 bit executables:

* Windows 64 bit
* 64-bit capable CPU

For the OS X executable:

* OS X 10.9 (Maverick)
Previous versions (from 10.6 onwards) should work too, but are untested.
