/*

Copyright (c) 2010-2014, Jan BOON
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef NLMISC_VECTORF_SWIZZLE_H
#define NLMISC_VECTORF_SWIZZLE_H
#include "types_nl.h"

// System includes
#ifdef NL_HAS_SSE2
#	include <xmmintrin.h>
#	include <emmintrin.h>
#endif

// STL includes

// NLMISC includes

// Project includes

namespace NLMISC {

NL_FORCE_INLINE CVector1F x(const CVector1F &v)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = v.mm;
	return m;
#else
	CVector1F s;
	s.x = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector1F x(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = v.mm;
	return m;
#else
	CVector1F s;
	s.x = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector1F x(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = v.mm;
	return m;
#else
	CVector1F s;
	s.x = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector1F x(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = v.mm;
	return m;
#else
	CVector1F s;
	s.x = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F xx(const CVector1F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 0));
	return m;
#else
	CVector2F s;
	s.x = v.x;
	s.y = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F xx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 0));
	return m;
#else
	CVector2F s;
	s.x = v.x;
	s.y = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F xx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 0));
	return m;
#else
	CVector2F s;
	s.x = v.x;
	s.y = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F xx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 0));
	return m;
#else
	CVector2F s;
	s.x = v.x;
	s.y = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xxx(const CVector1F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xxx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xxx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxxx(const CVector1F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxxx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxxx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxxy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxxy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxxz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xxy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xxy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxyx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxyx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxyy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxyy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxyz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xxz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxzx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxzy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxzz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 0, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xxww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 0, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.x;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F xy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = v.mm;
	return m;
#else
	CVector2F s;
	s.x = v.x;
	s.y = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F xy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = v.mm;
	return m;
#else
	CVector2F s;
	s.x = v.x;
	s.y = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F xy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = v.mm;
	return m;
#else
	CVector2F s;
	s.x = v.x;
	s.y = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xyx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xyx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyxx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyxx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyxy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyxy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyxz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xyy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xyy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyyx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyyx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyyy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyyy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyyz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xyz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = v.mm;
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = v.mm;
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyzx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyzy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyzz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = v.mm;
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 1, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xywx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xywy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xywz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xyww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 1, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.y;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F xz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 0));
	return m;
#else
	CVector2F s;
	s.x = v.x;
	s.y = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F xz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 0));
	return m;
#else
	CVector2F s;
	s.x = v.x;
	s.y = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xzx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 2, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 2, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzxx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzxy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzxz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xzy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 2, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 2, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzyx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzyy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzyz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xzz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzzx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzzy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzzz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 2, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xzww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 2, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.z;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F xw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 3, 0));
	return m;
#else
	CVector2F s;
	s.x = v.x;
	s.y = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 3, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 3, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 3, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F xww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 3, 0));
	return m;
#else
	CVector3F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F xwww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 3, 0));
	return m;
#else
	CVector4F s;
	s.x = v.x;
	s.y = v.w;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector1F y(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 1));
	return m;
#else
	CVector1F s;
	s.x = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector1F y(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 1));
	return m;
#else
	CVector1F s;
	s.x = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector1F y(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 1));
	return m;
#else
	CVector1F s;
	s.x = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F yx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 1));
	return m;
#else
	CVector2F s;
	s.x = v.y;
	s.y = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F yx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 1));
	return m;
#else
	CVector2F s;
	s.x = v.y;
	s.y = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F yx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 1));
	return m;
#else
	CVector2F s;
	s.x = v.y;
	s.y = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yxx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yxx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxxx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxxx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxxy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxxy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxxz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yxy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yxy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxyx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxyx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxyy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxyy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxyz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yxz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxzx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxzy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxzz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 0, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yxww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 0, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.x;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F yy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 1));
	return m;
#else
	CVector2F s;
	s.x = v.y;
	s.y = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F yy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 1));
	return m;
#else
	CVector2F s;
	s.x = v.y;
	s.y = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F yy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 1));
	return m;
#else
	CVector2F s;
	s.x = v.y;
	s.y = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yyx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yyx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyxx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyxx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyxy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyxy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyxz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yyy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yyy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyyx(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyyx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyyy(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyyy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyyz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yyz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyzx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyzy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyzz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 1, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yywx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yywy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yywz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yyww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 1, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.y;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F yz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 1));
	return m;
#else
	CVector2F s;
	s.x = v.y;
	s.y = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F yz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 1));
	return m;
#else
	CVector2F s;
	s.x = v.y;
	s.y = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yzx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 2, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 2, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzxx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzxy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzxz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yzy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 2, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 2, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzyx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzyy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzyz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yzz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzzx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzzy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzzz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 2, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F yzww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 2, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.z;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F yw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 3, 1));
	return m;
#else
	CVector2F s;
	s.x = v.y;
	s.y = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F ywx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 3, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F ywy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 3, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F ywz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 3, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F yww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 3, 1));
	return m;
#else
	CVector3F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F ywww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 3, 1));
	return m;
#else
	CVector4F s;
	s.x = v.y;
	s.y = v.w;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector1F z(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 2));
	return m;
#else
	CVector1F s;
	s.x = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector1F z(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 2));
	return m;
#else
	CVector1F s;
	s.x = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F zx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 2));
	return m;
#else
	CVector2F s;
	s.x = v.z;
	s.y = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F zx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 2));
	return m;
#else
	CVector2F s;
	s.x = v.z;
	s.y = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zxx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxxx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxxy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxxz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zxy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxyx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxyy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxyz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zxz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxzx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxzy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxzz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 0, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zxww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 0, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.x;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F zy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 2));
	return m;
#else
	CVector2F s;
	s.x = v.z;
	s.y = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F zy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 2));
	return m;
#else
	CVector2F s;
	s.x = v.z;
	s.y = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zyx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyxx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyxy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyxz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zyy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyyx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyyy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyyz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zyz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyzx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyzy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyzz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 1, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zywx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zywy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zywz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zyww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 1, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.y;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F zz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 2));
	return m;
#else
	CVector2F s;
	s.x = v.z;
	s.y = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F zz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 2));
	return m;
#else
	CVector2F s;
	s.x = v.z;
	s.y = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zzx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 2, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 2, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzxx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzxy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzxz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zzy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 2, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 2, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzyx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzyy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzyz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zzz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzzx(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzzy(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzzz(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 2, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zzww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 2, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.z;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F zw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 3, 2));
	return m;
#else
	CVector2F s;
	s.x = v.z;
	s.y = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 3, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 3, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 3, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F zww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 3, 2));
	return m;
#else
	CVector3F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F zwww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 3, 2));
	return m;
#else
	CVector4F s;
	s.x = v.z;
	s.y = v.w;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector1F w(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 3));
	return m;
#else
	CVector1F s;
	s.x = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F wx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 3));
	return m;
#else
	CVector2F s;
	s.x = v.w;
	s.y = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 0, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wxww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 0, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.x;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F wy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 3));
	return m;
#else
	CVector2F s;
	s.x = v.w;
	s.y = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 1, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wywx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wywy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wywz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wyww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 1, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.y;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F wz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 3));
	return m;
#else
	CVector2F s;
	s.x = v.w;
	s.y = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 2, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 2, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 2, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wzww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 2, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.z;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F ww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 3, 3));
	return m;
#else
	CVector2F s;
	s.x = v.w;
	s.y = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 3, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwxx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 0, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.x;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwxy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 0, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.x;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwxz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 0, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.x;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwxw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.x;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 3, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwyx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 1, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.y;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwyy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.y;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwyz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 1, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.y;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwyw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.y;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F wwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 3, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwzx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 2, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.z;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwzy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 2, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.z;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwzz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.z;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwzw(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 2, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.z;
	s.w = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F www(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 3, 3));
	return m;
#else
	CVector3F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwwx(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(0, 3, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.w;
	s.w = v.x;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwwy(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 3, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.w;
	s.w = v.y;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwwz(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 3, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.w;
	s.w = v.z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F wwww(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 3, 3));
	return m;
#else
	CVector4F s;
	s.x = v.w;
	s.y = v.w;
	s.z = v.w;
	s.w = v.w;
	return s;
#endif
}

} /* namespace NLMISC */

#endif /* #ifndef NLMISC_VECTORF_SWIZZLE_H */

/* end of file */
