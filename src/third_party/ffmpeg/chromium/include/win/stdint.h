/* ----------------------------------------------------------------------------
 * NOTICE:
 *      Copyright (c) 2013. Bloomberg Finance L.P.
 *
 *      Permission is hereby granted, free of charge, to any person obtaining a
 *      copy of this software and associated documentation files (the
 *      "Software"), to deal in the Software without restriction, including
 *      without limitation the rights to use, copy, modify, merge, publish,
 *      distribute, sublicense, and/or sell copies of the Software, and to
 *      permit persons to whom the Software is furnished to do so, subject to
 *      the following conditions:
 *
 *      The above copyright notice and this permission notice shall be included
 *      in all copies or substantial portions of the Software.
 *
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *      OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *      MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *      IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *      CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *      TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ------------------------------------------------------------------------- */

/* This file is provided by Bloomberg L.P. to allow ffmpeg to build in
 * VS2008 with the Windows 7.1 SDK.
 */

#ifndef INCLUDED_BB_STDINT
#define INCLUDED_BB_STDINT

typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

typedef long long intmax_t;
typedef unsigned long long uintmax_t;

#define INT8_MIN (-127-1)
#define INT8_MAX (127)
#define UINT8_MAX (255)
#define INT8_C(n) ((int8_t)n)
#define UINT8_C(n) ((uint8_t)n)

#define INT16_MIN (-32767-1)
#define INT16_MAX (32767)
#define UINT16_MAX (65535)
#define INT16_C(n) ((int16_t)n)
#define UINT16_C(n) ((uint16_t)n)

#define INT32_MIN (-2147483647-1)
#define INT32_MAX (2147483647)
#define UINT32_MAX (4294967295U)
#define INT32_C(n) (n ## L)
#define UINT32_C(n) (n ## UL)

#define INT64_MIN (-9223372036854775807LL-1LL)
#define INT64_MAX (9223372036854775807LL)
#define UINT64_MAX (18446744073709551615ULL)
#define INT64_C(n) (n ## LL)
#define UINT64_C(n) (n ## ULL)

#endif  /* INCLUDED_BB_STDINT */

