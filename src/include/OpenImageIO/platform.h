/*
  Copyright 2014 Larry Gritz and the other authors and contributors.
  All Rights Reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the software's owners nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  (This is the Modified BSD License)
*/


/////////////////////////////////////////////////////////////////////////
/// @file  platform.h
///
/// @brief Platform-related macros.
/////////////////////////////////////////////////////////////////////////


#pragma once

// Make sure all platforms have the explicit sized integer types
#if defined(_MSC_VER) && _MSC_VER < 1600
   typedef __int8  int8_t;
   typedef __int16 int16_t;
   typedef __int32 int32_t;
   typedef __int64 int64_t;
   typedef unsigned __int8  uint8_t;
   typedef unsigned __int16 uint16_t;
# ifndef _UINT64_T
   typedef unsigned __int32 uint32_t;
   typedef unsigned __int64 uint64_t;
#  define _UINT32_T
#  define _UINT64_T
# endif
#else
#  ifndef __STDC_LIMIT_MACROS
#    define __STDC_LIMIT_MACROS  /* needed for some defs in stdint.h */
#  endif
#  include <stdint.h>
#endif

#if defined(__FreeBSD__)
#include <sys/param.h>
#endif

#ifdef __MINGW32__
#include <malloc.h> // for alloca
#endif



// Detect if we're C++11
#if (__cplusplus >= 201103L)
#define OIIO_CPLUSPLUS11 1
#endif



/// allocates memory, equivalent of C99 type var_name[size]
#define OIIO_ALLOCA(type, size) ((type*)alloca((size) * sizeof (type)))

/// Deprecated (for namespace pollution reasons)
#define ALLOCA(type, size) ((type*)alloca((size) * sizeof (type)))


// Define a macro that can be used for memory alignment.
// I think that in a future world of C++1x compatibility, all these can
// be replaced with [[ align(size) ]].
#if defined (_MSC_VER)
#  define OIIO_ALIGN(size) __declspec(align(size))
#elif defined (__GNUC__)
#  define OIIO_ALIGN(size) __attribute__((aligned(size)))
#elif defined (__INTEL_COMPILER)
#  define OIIO_ALIGN(size) __declspec(align((size)))
#else
#  error "Don't know how to define OIIO_ALIGN"
#endif

// Cache line size is 64 on all modern x86 CPUs. If this changes or we
// anticipate ports to other architectures, we'll need to change this.
#define OIIO_CACHE_LINE_SIZE 64

// Align the next declaration to be on its own cache line
#define OIIO_CACHE_ALIGN OIIO_ALIGN(OIIO_CACHE_LINE_SIZE)



// gcc defines a special intrinsic to use in conditionals that can speed
// up extremely performance-critical spots if the conditional usually
// (or rarely) is true.  You use it by replacing
//     if (x) ...
// with
//     if (OIIO_LIKELY(x)) ...     // if you think x will usually be true
// or
//     if (OIIO_UNLIKELY(x)) ...   // if you think x will rarely be true
// Caveat: Programmers are notoriously bad at guessing this, so it
// should be used only with thorough benchmarking.
#ifdef __GNUC__
#define OIIO_LIKELY(x)   (__builtin_expect((x), 1))
#define OIIO_UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define OIIO_LIKELY(x)   (x)
#define OIIO_UNLIKELY(x) (x)
#endif


#if defined(__GNUC__) || defined(__clang__)
#  define OIIO_FORCEINLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#  define OIIO_FORCEINLINE __forceinline
#else
#  define OIIO_FORCEINLINE inline
#endif


#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER) || defined(__INTEL_COMPILER)
#  define OIIO_RESTRICT __restrict
#else
#  define OIIO_RESTRICT 
#endif



// Try to deduce endianness
#if (defined(_WIN32) || defined(__i386__) || defined(__x86_64__))
#  ifndef __LITTLE_ENDIAN__
#    define __LITTLE_ENDIAN__ 1
#    undef __BIG_ENDIAN__
#  endif
#endif


namespace {   // anon

/// Return true if the architecture we are running on is little endian
OIIO_FORCEINLINE bool littleendian (void)
{
#if defined(__BIG_ENDIAN__)
    return false;
#elif defined(__LITTLE_ENDIAN__)
    return true;
#else
    // Otherwise, do something quick to compute it
    int i = 1;
    return *((char *) &i);
#endif
}


/// Return true if the architecture we are running on is big endian
OIIO_FORCEINLINE bool bigendian (void)
{
    return ! littleendian();
}

} // end anon namespace

