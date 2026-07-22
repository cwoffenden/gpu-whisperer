/**
 * \file defines.h
 * Application-wide constants and macros.
 */
#pragma once

//***************************** Compiler Helpers ******************************/

/**
 * \def __has_feature
 * Dummy \c __has_feature implementation for when not using Clang (in which case
 * all requested features are reported as unimplemented).
 *
 * \param feature compiler feature to query
 */
#ifndef __has_feature
#ifndef __SNC__
#define __has_feature(feature) 0
#endif
#endif

/**
 * \def __has_builtin
 * Dummy \c __has_builtin implementation for when not using Clang (in which case
 * all requested builtins are reported as unimplemented).
 *
 * \param builtin compiler builtin to query
 */
#ifndef __has_builtin
#define __has_builtin(builtin) 0
#endif

/**
 * \def __has_include
 * Dummy \c __has_include implementation for when using compilers not supporting
 * this feature (in which case all requested files are reported as missing).
 *
 * \param header include file to query
 */
#ifndef __has_include
#define __has_include(header) 0
#endif

/**
 * \def GCC_MIN_VER
 * Helper to determine the minimum required GCC version, resolving to \c 1 if
 * the compiler is GCC and the version is equal to or greater than the passed
 * major and minor values (otherwise zero).
 *
 * \param major major version number
 * \param minor minor version number
 */
#ifndef GCC_MIN_VER
#define GCC_MIN_VER(major, minor) (__GNUC__ && ((__GNUC__ > major) || (__GNUC__ == major && __GNUC_MINOR__ >= minor)))
#endif

/**
 * \def CLANG_MIN_VER
 * Helper to determine the minimum required Clang version, resolving to \c 1 if
 * the compiler is Clang and the version is equal to or greater than the passed
 * major and minor values (otherwise zero).
 *
 * \param major major version number
 * \param minor minor version number
 */
#ifndef CLANG_MIN_VER
#define CLANG_MIN_VER(major, minor) (__clang__ && ((__clang_major__ > major) || (__clang_major__ == major && __clang_minor__ >= minor)))
#endif

/*
 * Even though we're not macroing keywords in (VS) compilers that support the
 * feature, the checks that run are being tripped due to keywords being reserved
 * (e.g. VS2012 checks for \n noexcept, reserved but not supported).
 */
#define _ALLOW_KEYWORD_MACROS

/**
 * \def nullptr
 * Null pointer constant (resolves to C++11's \c nullptr where possible).
 *
 * \note Supported in MSCV 2010, Clang 3.0 and GCC 4.6 onwards (falling back to
 * \c NULL for other compilers or versions).
 */
#ifndef nullptr
#if !((_MSC_VER >= 1600) || __has_feature(cxx_nullptr) || (GCC_MIN_VER(4, 6) && __cplusplus >= 201103L))
#define nullptr NULL
#endif
#endif

/**
 * \def _Nonnull
 * Qualifier that marks a pointer as \e never being \c null (see also \c
 * #_Nullable). Valid for both parameters and return values.
 *
 * \note This is a Clang-only feature
 * \sa https://clang.llvm.org/docs/AttributeReference.html#id417
 */
#ifndef _Nonnull
#if !__has_feature(nullability)
#define _Nonnull
#endif
#endif

/**
 * \def _Nullable
 * Qualifier that marks a pointer as possibly being \c null (see also \c
 * #_Nonnull). Valid for both parameters and return values.
 *
 * \note This is a Clang-only feature
 * \sa https://clang.llvm.org/docs/AttributeReference.html#id419
 */
#ifndef _Nullable
#if !__has_feature(nullability)
#define _Nullable
#endif
#endif

/**
 * \def __unused
 * Attribute that marks a function, parameter, variable, etc., as unused,
 * silencing any \c -Wunused warnings.
 *
 * \note This is a Clang 3.9 and GCC 4.9 onwards only feature (here for
 * compatibility with Xcode). Earlier compiler versions may support it with C,
 * but not C++ or Objective-C. Analogous to the C++11 \c [[unused]] attribute.
 *
 * \note It should work in GCC 3.x versions but is warning that the attribute is
 * ignored. Older compilers are picky about the placement, and only after the
 * variable name works everywhere.
 */
#ifndef __unused
#if CLANG_MIN_VER(3, 9) || GCC_MIN_VER(4, 9)
#define __unused __attribute__((unused))
#else
#define __unused
#endif
#endif

/**
 * \def static_assert
 * Compile-time assertion.
 *
 * \note Supported in MSCV 2012, Clang 3.0 and GCC 4.6 onwards (with an empty
 * implementation for other compilers).
 *
 * \param exp expression that must evaluate to \c true
 * \param str error string literal to show if \a exp is \c false (note this is \e not optional, as it is in C++11)
 */
#ifndef static_assert
#if !((_MSC_VER >= 1700) || __has_feature(cxx_static_assert) || (GCC_MIN_VER(4, 6) && __cplusplus >= 201103L))
#define static_assert(exp, str)
#endif
#endif

//*********************************** Types ***********************************/

/*
 * Basic numeric types (e.g. uint8_t, size_t, UINT8_MAX, etc.).
 */
#if __cplusplus < 201103L
#ifndef __STDC_LIMIT_MACROS
#error "-D__STDC_LIMIT_MACROS needs passing to the compiler"
#endif
#endif
#include <stdint.h>
#include <cstddef>

/*
 * Endianness (we're only interested in capturing BE, assuming LE otherwise).
 */
#ifndef XP_BIG_ENDIAN
#if defined(__APPLE__)
#include <machine/endian.h>
#endif
#if (defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN)) || (defined(BYTE_ORDER) && (BYTE_ORDER == BIG_ENDIAN))
#define XP_BIG_ENDIAN
#endif
#endif

/*
 * Older Xcode defines this for debug builds, then library code fails to compile
 * when RTTI is disabled. Setting _GLIBCXX_NO_ASSERTIONS is having no effect.
 */
#if defined(__APPLE__) && defined(_GLIBCXX_DEBUG)
#undef _GLIBCXX_DEBUG
#undef _GLIBCXX_DEBUG_PEDANTIC
#endif
