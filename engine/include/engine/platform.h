#pragma once

/**
 * @file platform.h
 * @brief Compile-time platform detection.
 *
 * This header is the single place that decides which platform is being built.
 * Do NOT also define these from CMake.
 *
 * Primary platform macros (exactly one branch is taken):
 *   ENGINE_PLATFORM_WINDOWS            Windows (64-bit only)
 *   ENGINE_PLATFORM_LINUX              Linux (Android also defines this)
 *   ENGINE_PLATFORM_ANDROID            Android (on top of LINUX)
 *   ENGINE_PLATFORM_MACOS              macOS
 *   ENGINE_PLATFORM_IOS                iOS / iPadOS / tvOS / watchOS
 *   ENGINE_PLATFORM_IOS_SIMULATOR      iOS family simulator (on top of IOS)
 *   ENGINE_PLATFORM_UNIX               Any other Unix (BSDs, Solaris...)
 *
 * Group macros (for code shared between several platforms):
 *   ENGINE_PLATFORM_APPLE              macOS or iOS family
 *   ENGINE_PLATFORM_POSIX              Linux, Android, Apple, other Unix
 *   ENGINE_PLATFORM_MOBILE             Android or iOS family
 *   ENGINE_PLATFORM_DESKTOP            everything else
 *
 * Also defined:
 *   ENGINE_PLATFORM_NAME               human readable string, e.g. "Windows"
 *
 * Use with #ifdef, not #if. An undefined macro inside #if silently becomes 0,
 * which hides typos.
 *
 *   #ifdef ENGINE_PLATFORM_WINDOWS
 *       // Win32 code
 *   #endif
 */

// -----------------------------------------------------------------------------
// Primary platform
// -----------------------------------------------------------------------------
#if defined(_WIN32)
// Windows. _WIN32 is defined by MSVC, clang-cl and MinGW, for 32- and 64-bit.
#    define ENGINE_PLATFORM_WINDOWS 1
#    define ENGINE_PLATFORM_NAME "Windows"
#    if !defined(_WIN64)
#        error "A 64-bit build is required on Windows!"
#    endif

#elif defined(__linux__) || defined(__gnu_linux__)
// Linux. Android compilers define __linux__ too, so check Android inside here.
#    define ENGINE_PLATFORM_LINUX 1
#    if defined(__ANDROID__)
#        define ENGINE_PLATFORM_ANDROID 1
#        define ENGINE_PLATFORM_NAME "Android"
#    else
#        define ENGINE_PLATFORM_NAME "Linux"
#    endif

#elif defined(__APPLE__)
// Apple platforms. Checked before generic Unix.
#    include <TargetConditionals.h>
#    define ENGINE_PLATFORM_APPLE 1
// TARGET_OS_MAC is also 1 on iOS, so the iOS family must be tested first.
#    if TARGET_OS_IPHONE
#        define ENGINE_PLATFORM_IOS 1
#        define ENGINE_PLATFORM_NAME "iOS"
#        if TARGET_OS_SIMULATOR || TARGET_IPHONE_SIMULATOR
#            define ENGINE_PLATFORM_IOS_SIMULATOR 1
#        endif
#    elif TARGET_OS_MAC
#        define ENGINE_PLATFORM_MACOS 1
#        define ENGINE_PLATFORM_NAME "macOS"
#    else
#        error "Unknown Apple platform!"
#    endif

#elif defined(__unix__) || defined(__unix)
// Catch-all for other Unix systems (FreeBSD, OpenBSD, Solaris...).
#    define ENGINE_PLATFORM_UNIX 1
#    define ENGINE_PLATFORM_NAME "Unix"

#else
#    error "Unknown platform!"
#endif

// -----------------------------------------------------------------------------
// Platform groups
// -----------------------------------------------------------------------------
#if defined(ENGINE_PLATFORM_LINUX) || defined(ENGINE_PLATFORM_APPLE) || defined(ENGINE_PLATFORM_UNIX)
#    define ENGINE_PLATFORM_POSIX 1
#endif

#if defined(ENGINE_PLATFORM_ANDROID) || defined(ENGINE_PLATFORM_IOS)
#    define ENGINE_PLATFORM_MOBILE 1
#else
#    define ENGINE_PLATFORM_DESKTOP 1
#endif

// -----------------------------------------------------------------------------
// Sanity check for every platform: the engine assumes 64-bit pointers.
// -----------------------------------------------------------------------------
static_assert(sizeof(void*) == 8, "A 64-bit build is required.");
