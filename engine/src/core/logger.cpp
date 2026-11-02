#include <engine/core/logger.h>

#include <engine/core/asserts.h>

#include <cstdarg>
#include <cstddef>
#include <cstdio>

#if defined(ENGINE_PLATFORM_WINDOWS)
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <windows.h>
#endif

namespace engine {

namespace {

// Longest message body we format in one go. Longer messages are truncated.
constexpr size_t MESSAGE_BUFFER_SIZE = 4096;

struct LevelStyle {
    const char* prefix;  // text printed before the message
    const char* ansi;    // ANSI colour code (Linux, macOS, ...)
    unsigned short win;  // Windows console text attribute
};

// Indexed by static_cast<u8>(LogLevel). Keep in the same order as the enum.
constexpr LevelStyle LEVEL_STYLES[] = {
    {"[FATAL]: ", "0;41", 64},  // white on red
    {"[ERROR]: ", "1;31", 4},   // red
    {"[WARN]:  ", "1;33", 6},   // yellow
    {"[INFO]:  ", "1;32", 2},   // green
    {"[DEBUG]: ", "1;34", 1},   // blue
    {"[TRACE]: ", "1;30", 8},   // grey
};
static_assert(sizeof(LEVEL_STYLES) / sizeof(LEVEL_STYLES[0]) == 6, "LEVEL_STYLES must have one entry per LogLevel.");

// Writes one finished line to the console in the colour for its level.
// Fatal and error go to stderr, everything else to stdout.
// TODO: move this into the platform layer once it exists.
void console_write(const char* line, LogLevel level) {
    const LevelStyle& style = LEVEL_STYLES[static_cast<u8>(level)];
    const bool is_error = level <= LogLevel::Error;
    std::FILE* stream = is_error ? stderr : stdout;

#if defined(ENGINE_PLATFORM_WINDOWS)
    HANDLE console = GetStdHandle(is_error ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO original;
    // Fails when output is redirected to a file or pipe; then we skip the colours.
    const bool is_console = GetConsoleScreenBufferInfo(console, &original) != 0;
    if (is_console) {
        SetConsoleTextAttribute(console, style.win);
    }
    std::fputs(line, stream);
    std::fflush(stream);
    if (is_console) {
        SetConsoleTextAttribute(console, original.wAttributes);
    }
    // Also show it in the debugger's output window.
    OutputDebugStringA(line);
#else
    std::fprintf(stream, "\033[%sm%s\033[0m", style.ansi, line);
    std::fflush(stream);
#endif
}

}  // namespace

bool logger_initialize() {
    // TODO: create a log file here (later episode).
    return true;
}

void logger_shutdown() {
    // TODO: flush and close the log file here.
}

void log_output(LogLevel level, const char* format, ...) {
    // Not thread-safe yet. Add a lock once the job system exists.
    char body[MESSAGE_BUFFER_SIZE];

    va_list args;
    va_start(args, format);
    std::vsnprintf(body, sizeof(body), format, args);
    va_end(args);

    char line[MESSAGE_BUFFER_SIZE + 32];
    std::snprintf(line, sizeof(line), "%s%s\n", LEVEL_STYLES[static_cast<u8>(level)].prefix, body);

    console_write(line, level);
}

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) {
    log_output(LogLevel::Fatal, "Assertion failure: %s, message: '%s', in file: %s, line: %d", expression, message, file, line);
}

}  // namespace engine
