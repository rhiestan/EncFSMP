#ifndef VERSION_H
#define VERSION_H

#define EFS_STRINGIFY2(x) #x
#define EFS_STRINGIFY(x) EFS_STRINGIFY2(x)

#define ENCFSMP_VERSION_MAJOR 0
#define ENCFSMP_VERSION_MINOR 9
#define ENCFSMP_VERSION_BUILD 4

#define ENCFSMP_VERSION_STRING EFS_STRINGIFY(ENCFSMP_VERSION_MAJOR) \
	"." EFS_STRINGIFY(ENCFSMP_VERSION_MINOR) \
	"." EFS_STRINGIFY(ENCFSMP_VERSION_BUILD)

#define ENCFSMP_NAME "EncFS MP"
#define ENCFSMP_COPYRIGHT_NAME "Roman Hiestand"
#define ENCFSMP_COPYRIGHT_YEAR "2015"

// The check for __INTEL_COMPILER needs to be before _MSC_VER and __GNUG__, as
// the Intel compiler defines _MSC_VER and __GNUG__ as well
#if defined(__INTEL_COMPILER)
#	define ENCFSMP_COMPILER "Intel C++"
#	define ENCFSMP_COMPILER_VERSION EFS_STRINGIFY(__INTEL_COMPILER)
#elif defined(_MSC_VER)
#	define ENCFSMP_COMPILER "Visual C++"
#	define ENCFSMP_COMPILER_VERSION EFS_STRINGIFY(_MSC_FULL_VER)
#elif defined(__clang__)
#	define ENCFSMP_COMPILER "Clang"
#	define ENCFSMP_COMPILER_VERSION EFS_STRINGIFY(__clang_major__) "." EFS_STRINGIFY(__clang_minor__) "." EFS_STRINGIFY(__clang_patchlevel__)
#elif defined(__GNUG__)
#	define ENCFSMP_COMPILER "GCC"
#	if defined(__GNUC_PATCHLEVEL__)
#		define ENCFSMP_COMPILER_VERSION EFS_STRINGIFY(__GNUC__) "." EFS_STRINGIFY(__GNUC_MINOR__) "." EFS_STRINGIFY(__GNUC_PATCHLEVEL__)
#	else
#		define ENCFSMP_COMPILER_VERSION EFS_STRINGIFY(__GNUC__) "." EFS_STRINGIFY(__GNUC_MINOR__)
#	endif
#else
#	define ENCFSMP_COMPILER "Unknown compiler"
#	define ENCFSMP_COMPILER_VERSION "Unknown version"
#endif

// Architecture
#if defined(_M_X64) || defined(_M_AMD64) || defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#define ENCFSMP_ARCHITECTURE_X64
#define ENCFSMP_ARCHITECTURE_STRING "x64"
#elif defined(_M_ARM) || defined(__arm__) || defined(_ARM)
#define ENCFSMP_ARCHITECTURE_ARM
#define ENCFSMP_ARCHITECTURE_STRING "ARM"
#elif defined(_M_IX86) || defined(__i386) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(_X86_)
#define ENCFSMP_ARCHITECTURE_X86
#define ENCFSMP_ARCHITECTURE_STRING "x86"
#elif defined(_M_IA64) || defined(__ia64__) || defined(_IA64) || defined(__IA64__) || defined(__ia64)
#define ENCFSMP_ARCHITECTURE_ITANIUM
#define ENCFSMP_ARCHITECTURE_STRING "Itanium"
#elif defined(_M_PPC) || defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__ppc64__)
#define ENCFSMP_ARCHITECTURE_PPC
#define ENCFSMP_ARCHITECTURE_STRING "PPC"
#elif defined(__sparc__) || defined(__sparc)
#define ENCFSMP_ARCHITECTURE_SPARC
#define ENCFSMP_ARCHITECTURE_STRING "SPARC"
#endif

#endif
