cmake_minimum_required(VERSION 3.20)
project(AISLADO VERSION 0.1.0 LANGUAGES CXX)

# =============================================================================
# Global settings
# =============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)                # -std=c++20, not gnu++20
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)        # compile_commands.json for clangd / VS Code
set(CMAKE_CXX_VISIBILITY_PRESET hidden)      # same export behaviour on every platform
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(ENGINE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# Everything lands in bin/ and you run the testbed from there (like the series).
# NOTE: with multi-config generators (Visual Studio) CMake adds bin/Debug etc.
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})   # .exe and .dll (Windows)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})   # .so (Linux)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # import libs

# MinGW: bake the GCC runtime into our binaries so they run outside an MSYS shell
# (otherwise you get "libstdc++-6.dll was not found").
# Caveat: engine.dll and testbed.exe then each carry their own copy of the
# runtime, so never pass STL objects (std::string, std::vector...) across the
# DLL boundary. Use your own types / plain structs in public engine headers.
if(MINGW)
    add_link_options(-static-libgcc -static-libstdc++)
endif()

# =============================================================================
# Dependencies
# =============================================================================
find_package(Vulkan REQUIRED COMPONENTS glslc)   # needs the LunarG Vulkan SDK
find_package(Threads REQUIRED)

if(UNIX AND NOT APPLE)
    # Linux window layer (XCB + xkbcommon).
    # Ubuntu: sudo apt install libxcb1-dev libx11-xcb-dev libxkbcommon-dev
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb x11-xcb xkbcommon)
endif()

# =============================================================================
# Per-target warning / language flags
# =============================================================================
function(engine_apply_flags target)
    if(MSVC)  # true for cl and clang-cl
        target_compile_options(${target} PRIVATE /W4 /permissive- /EHs-c- /GR-)
        target_compile_definitions(${target} PRIVATE _HAS_EXCEPTIONS=0 _CRT_SECURE_NO_WARNINGS)
        if(ENGINE_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Wshadow
            -fno-exceptions -fno-rtti)
        if(ENGINE_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# =============================================================================
# engine  (shared library)   ->  engine/include, engine/src
# =============================================================================
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/engine/src/*.cpp)
if(NOT ENGINE_SOURCES)
    message(FATAL_ERROR "No .cpp files found in engine/src - create at least one.")
endif()

add_library(engine SHARED ${ENGINE_SOURCES})
source_group(TREE ${CMAKE_SOURCE_DIR}/engine FILES ${ENGINE_SOURCES})

# Generates <build>/generated/engine/engine_export.h defining ENGINE_API.
# Put ENGINE_API on anything the game / testbed must see:
#   #include <engine/engine_export.h>
#   ENGINE_API void engine_run();
include(GenerateExportHeader)
generate_export_header(engine
    EXPORT_MACRO_NAME ENGINE_API
    EXPORT_FILE_NAME  ${CMAKE_BINARY_DIR}/generated/engine/engine_export.h)

target_include_directories(engine
    PUBLIC  ${CMAKE_SOURCE_DIR}/engine/include
            ${CMAKE_BINARY_DIR}/generated
    PRIVATE ${CMAKE_SOURCE_DIR}/engine/src
            ${CMAKE_SOURCE_DIR}/third_party)

target_compile_definitions(engine PUBLIC
    $<$<CONFIG:Debug>:ENGINE_DEBUG=1>
    $<$<NOT:$<CONFIG:Debug>>:ENGINE_RELEASE=1>)

target_link_libraries(engine PRIVATE Vulkan::Vulkan Threads::Threads ${CMAKE_DL_LIBS})

if(WIN32)
    # ENGINE_PLATFORM_* macros come from engine/include/engine/platform.h
    target_compile_definitions(engine
        PRIVATE VK_USE_PLATFORM_WIN32_KHR NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(engine PRIVATE user32 gdi32)
    set_target_properties(engine PROPERTIES PREFIX "")   # engine.dll, not libengine.dll
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(engine
        PRIVATE VK_USE_PLATFORM_XCB_KHR)
    target_link_libraries(engine PRIVATE PkgConfig::XCB)
endif()

engine_apply_flags(engine)

# =============================================================================
# testbed_game  (shared library, the hot-reloadable "game code")
# =============================================================================
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/testbed/game/*.cpp)
if(NOT GAME_SOURCES)
    message(FATAL_ERROR "No .cpp files found in testbed/game - create at least one.")
endif()

add_library(testbed_game SHARED ${GAME_SOURCES})
source_group(TREE ${CMAKE_SOURCE_DIR}/testbed/game FILES ${GAME_SOURCES})

generate_export_header(testbed_game
    EXPORT_MACRO_NAME GAME_API
    EXPORT_FILE_NAME  ${CMAKE_BINARY_DIR}/generated/testbed/game_export.h)

target_include_directories(testbed_game PRIVATE
    ${CMAKE_SOURCE_DIR}/testbed/game
    ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(testbed_game PRIVATE engine)
if(WIN32)
    set_target_properties(testbed_game PROPERTIES PREFIX "")
endif()
engine_apply_flags(testbed_game)

# =============================================================================
# testbed  (executable)  ->  loads testbed_game at runtime, drives the engine
# =============================================================================
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/testbed/app/*.cpp)
if(NOT APP_SOURCES)
    message(FATAL_ERROR "No .cpp files found in testbed/app - create at least one.")
endif()

add_executable(testbed ${APP_SOURCES})
source_group(TREE ${CMAKE_SOURCE_DIR}/testbed/app FILES ${APP_SOURCES})
target_include_directories(testbed PRIVATE ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(testbed PRIVATE engine)
engine_apply_flags(testbed)

# The exe does not link the game lib (it loads it at runtime for hot reload),
# so tell CMake to build it first.
add_dependencies(testbed testbed_game)

# =============================================================================
# Shaders: assets/shaders/*.{vert,frag,comp,geom}  ->  bin/assets/shaders/*.spv
# Rebuilt automatically whenever a .vert/.frag changes.
# =============================================================================
file(GLOB_RECURSE SHADER_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/assets/shaders/*.vert
    ${CMAKE_SOURCE_DIR}/assets/shaders/*.frag
    ${CMAKE_SOURCE_DIR}/assets/shaders/*.comp
    ${CMAKE_SOURCE_DIR}/assets/shaders/*.geom)

set(SPV_FILES "")
foreach(shader ${SHADER_SOURCES})
    file(RELATIVE_PATH shader_rel ${CMAKE_SOURCE_DIR}/assets/shaders ${shader})
    set(spv ${OUTPUT_DIR}/assets/shaders/${shader_rel}.spv)
    get_filename_component(spv_dir ${spv} DIRECTORY)
    add_custom_command(
        OUTPUT  ${spv}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${spv_dir}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${shader} -o ${spv}
        DEPENDS ${shader}
        COMMENT "glslc ${shader_rel}"
        VERBATIM)
    list(APPEND SPV_FILES ${spv})
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_FILES})
add_dependencies(testbed shaders)

# =============================================================================
message(STATUS "Compiler : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build    : ${CMAKE_BUILD_TYPE}")
message(STATUS "Vulkan   : ${Vulkan_LIBRARY}")
message(STATUS "glslc    : ${Vulkan_GLSLC_EXECUTABLE}")
message(STATUS "Shaders  : ${SHADER_SOURCES}")
