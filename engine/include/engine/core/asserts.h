#pragma once

/**
 * @file asserts.h
 * @brief Runtime assertion macros.
 *
 *   ENGINE_ASSERT(expr)              fails if expr is false
 *   ENGINE_ASSERT_MSG(expr, "text")  same, with an extra message
 *   ENGINE_ASSERT_DEBUG(expr)        only checked in Debug builds (use for costly checks)
 *
 * On failure the expression, file and line are logged as FATAL and the
 * debugger is triggered. To compile every assertion out, define
 * ENGINE_DISABLE_ASSERTIONS (for example in a shipping build).
 */

#include <engine/defines.h>
#include <engine/engine_export.h>

#ifndef ENGINE_DISABLE_ASSERTIONS
#    define ENGINE_ASSERTIONS_ENABLED 1
#endif

// Break into the debugger. If none is attached the program just crashes,
// which is what you want for a failed assertion.
#if defined(_MSC_VER)  // also true for clang-cl
#    include <intrin.h>
#    define ENGINE_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__)
#    define ENGINE_DEBUG_BREAK() __builtin_debugtrap()
#else
#    define ENGINE_DEBUG_BREAK() __builtin_trap()
#endif

namespace engine {

/**
 * @brief Logs a failed assertion as FATAL. Called by the macros below, not by you.
 * @param expression The stringified expression that was false.
 * @param message An optional extra message ("" if none).
 * @param file The source file (__FILE__).
 * @param line The line number (__LINE__).
 */
ENGINE_API void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line);

}  // namespace engine

#ifdef ENGINE_ASSERTIONS_ENABLED

#    define ENGINE_ASSERT(expr)                                                       \
        do {                                                                          \
            if (!(expr)) {                                                            \
                ::engine::report_assertion_failure(#expr, "", __FILE__, __LINE__);    \
                ENGINE_DEBUG_BREAK();                                                 \
            }                                                                         \
        } while (0)

#    define ENGINE_ASSERT_MSG(expr, message)                                          \
        do {                                                                          \
            if (!(expr)) {                                                            \
                ::engine::report_assertion_failure(#expr, message, __FILE__, __LINE__); \
                ENGINE_DEBUG_BREAK();                                                 \
            }                                                                         \
        } while (0)

#    ifdef ENGINE_RELEASE
#        define ENGINE_ASSERT_DEBUG(expr) do {} while (0)
#    else
#        define ENGINE_ASSERT_DEBUG(expr) ENGINE_ASSERT(expr)
#    endif

#else

#    define ENGINE_ASSERT(expr) do {} while (0)
#    define ENGINE_ASSERT_MSG(expr, message) do {} while (0)
#    define ENGINE_ASSERT_DEBUG(expr) do {} while (0)

#endif
