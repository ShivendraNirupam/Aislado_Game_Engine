#pragma once

/**
 * @file logger.h
 * @brief Leveled, coloured console logging.
 *
 * Use the macros, printf-style:
 *
 *   ELOG_FATAL("Out of memory: %u bytes", size);
 *   ELOG_ERROR("Failed to load '%s'", path);
 *   ELOG_WARN("Texture %s is not power of two", name);
 *   ELOG_INFO("Window created: %ux%u", w, h);
 *   ELOG_DEBUG("Frame time: %f", dt);      // compiled out in release
 *   ELOG_TRACE("Entering update()");       // compiled out in release
 *
 * The first argument must be a string literal so the compiler can check the
 * format specifiers against the arguments (GCC / Clang).
 */

#include <engine/defines.h>
#include <engine/engine_export.h>

// Which levels are compiled in. Debug and trace are disabled in release builds
// (CMake defines ENGINE_RELEASE for every build type except Debug).
#define ELOG_WARN_ENABLED 1
#define ELOG_INFO_ENABLED 1
#ifdef ENGINE_RELEASE
#    define ELOG_DEBUG_ENABLED 0
#    define ELOG_TRACE_ENABLED 0
#else
#    define ELOG_DEBUG_ENABLED 1
#    define ELOG_TRACE_ENABLED 1
#endif

// Lets GCC / Clang check printf format strings. On MinGW the logger uses the
// GNU-compatible printf family, so tell the compiler to check against that.
#if defined(__MINGW32__)
#    define ENGINE_PRINTF_FORMAT(fmt_index, args_index) __attribute__((format(gnu_printf, fmt_index, args_index)))
#elif defined(__GNUC__) || defined(__clang__)
#    define ENGINE_PRINTF_FORMAT(fmt_index, args_index) __attribute__((format(printf, fmt_index, args_index)))
#else
#    define ENGINE_PRINTF_FORMAT(fmt_index, args_index)
#endif

namespace engine {

/** @brief Log severity, from most to least severe. */
enum class LogLevel : u8 {
    Fatal = 0,
    Error = 1,
    Warn = 2,
    Info = 3,
    Debug = 4,
    Trace = 5
};

/**
 * @brief Starts the logging system. Call once at startup.
 * @return true on success.
 */
ENGINE_API bool logger_initialize();

/** @brief Shuts the logging system down. Call once at exit. */
ENGINE_API void logger_shutdown();

/**
 * @brief Formats and prints a message at the given level. Prefer the ELOG_* macros.
 * @param level Severity of the message.
 * @param format printf-style format string.
 */
ENGINE_API void log_output(LogLevel level, const char* format, ...) ENGINE_PRINTF_FORMAT(2, 3);

}  // namespace engine

// Fatal and error are always on.
#define ELOG_FATAL(...) ::engine::log_output(::engine::LogLevel::Fatal, __VA_ARGS__)
#define ELOG_ERROR(...) ::engine::log_output(::engine::LogLevel::Error, __VA_ARGS__)

#if ELOG_WARN_ENABLED == 1
#    define ELOG_WARN(...) ::engine::log_output(::engine::LogLevel::Warn, __VA_ARGS__)
#else
#    define ELOG_WARN(...) ((void)0)
#endif

#if ELOG_INFO_ENABLED == 1
#    define ELOG_INFO(...) ::engine::log_output(::engine::LogLevel::Info, __VA_ARGS__)
#else
#    define ELOG_INFO(...) ((void)0)
#endif

#if ELOG_DEBUG_ENABLED == 1
#    define ELOG_DEBUG(...) ::engine::log_output(::engine::LogLevel::Debug, __VA_ARGS__)
#else
#    define ELOG_DEBUG(...) ((void)0)
#endif

#if ELOG_TRACE_ENABLED == 1
#    define ELOG_TRACE(...) ::engine::log_output(::engine::LogLevel::Trace, __VA_ARGS__)
#else
#    define ELOG_TRACE(...) ((void)0)
#endif
