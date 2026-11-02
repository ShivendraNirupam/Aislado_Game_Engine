#pragma once

/**
 * @file defines.h
 * @brief Fixed-size type aliases shared by the whole engine.
 */

#include <cstdint>
#include <engine/platform.h>

/** @brief Unsigned 8-bit integer */
using u8 = uint8_t;
/** @brief Unsigned 16-bit integer */
using u16 = uint16_t;
/** @brief Unsigned 32-bit integer */
using u32 = uint32_t;
/** @brief Unsigned 64-bit integer */
using u64 = uint64_t;

/** @brief Signed 8-bit integer */
using i8 = int8_t;
/** @brief Signed 16-bit integer */
using i16 = int16_t;
/** @brief Signed 32-bit integer */
using i32 = int32_t;
/** @brief Signed 64-bit integer */
using i64 = int64_t;

/** @brief 32-bit floating point number */
using f32 = float;
/** @brief 64-bit floating point number */
using f64 = double;

static_assert(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
static_assert(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
static_assert(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
static_assert(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

static_assert(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
static_assert(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
static_assert(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
static_assert(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

static_assert(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
static_assert(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");
