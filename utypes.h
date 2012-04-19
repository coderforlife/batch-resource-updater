// Adapted from the uSTL library:
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <stddef.h>
typedef SSIZE_T ssize_t;
#if _MSC_VER >= 1600
#include <stdint.h>
#else
#include <yvals.h>
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
#ifndef WCHAR_MIN
#define WCHAR_MIN	0x0000
#define WCHAR_MAX	0xffff
#endif
#endif
#include <limits.h>
#include <float.h>
#include <sys/types.h>

typedef size_t		uoff_t;			///< A type for storing offsets into blocks measured by size_t.
typedef uint32_t	hashvalue_t;	///< Value type returned by the hash functions.
typedef size_t		streamsize;		///< Size of stream data
typedef uoff_t		streamoff;		///< Offset into a stream
