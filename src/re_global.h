#ifndef RE_GENERAL_H
#define RE_GENERAL_H

// define OS_XXX to know which platform is it.

#if (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#   define OS_WIN
#   define OS_WIN64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#   define OS_WIN
#   define OS_WIN32
#elif defined(__MWERKS__) && defined(__INTEL__)
#   define OS_WIN
#   define OS_WIN32
#elif __APPLE__
#   include "TargetConditionals.h"
#   if TARGET_IPHONE_SIMULATOR
         // iOS Simulator
#   elif TARGET_OS_IPHONE
        // iOS device
#   elif TARGET_OS_MAC
        // Other kinds of Mac OS
#   else
#       error "Unknown Apple platform"
#   endif
#elif defined(__linux__) || defined(__linux)
#   define OS_UNIX
#   define OS_LINUX
#elif defined(__USLC__) // all SCO platforms + UDK or OUDK
#   define OS_UNIX
#elif defined(__svr4__) && defined(i386) // Open UNIX 8 + GCC
#   define OS_UNIX
#elif __unix__ // all unices not caught above
#   define OS_UNIX
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown Platform"
#endif

//// @TODO include file to get int 32, int 64, signed, unsigned etc.

// @TODO write the typedef

typedef signed long long   s64;
typedef signed int         s32;
typedef signed short       s16;
typedef signed char        s8;

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

// @TEMP @TODO use a better struct than std::string
#include <string>
typedef std::string utf8_string;

#include <vector>
#include <array>

template <class T>
using darray = std::vector<T>; // darray = (allocated) dynamic array, stretchy array


template <class T, std::size_t N>
using array = std::array<T, N>; // fixed size array


template <class T>
using fdarray = std::vector<T>; // Allocated fixed sized array @TODO implement a std::dynarray like,


// Check the definition of functions below in re_general.xxx.h

inline static void g_get_absolute_file_path(const char* filepath, char* abs_filepath);

inline static bool g_is_absolute_path(const char* filepath);




#ifdef OS_WIN

#include "re_global.win.h"

#endif // OS_WIN

#ifdef OS_UNIX

#include "re_global.unix.h"

#endif



#endif // RE_GENERAL_H
