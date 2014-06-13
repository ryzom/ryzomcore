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

#ifndef NLMISC_VECTORF_MATH_H
#define NLMISC_VECTORF_MATH_H
#include "types_nl.h"

// System includes
#ifdef NL_HAS_SSE2
#	include <xmmintrin.h>
#	include <emmintrin.h>
#endif
#include <math.h>

// STL includes

// NLMISC includes
#include "types_nl.h"

// Project includes

#ifdef min
#	undef min
#endif
#ifdef max
#	undef max
#endif

namespace NLMISC {

NL_FORCE_INLINE CVector1F add(const CVector1F &l, const CVector1F &r)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = _mm_add_ss(l.mm, r.mm);
	return m;
#else
	CVector1F a;
	a.x = l.x + r.x;
	return a;
#endif
}

NL_FORCE_INLINE CVector2F add(const CVector2F &l, const CVector2F &r)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_add_ps(l.mm, r.mm);
	return m;
#else
	CVector2F a;
	a.x = l.x + r.x;
	a.y = l.y + r.y;
	return a;
#endif
}

NL_FORCE_INLINE CVector3F add(const CVector3F &l, const CVector3F &r)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_add_ps(l.mm, r.mm);
	return m;
#else
	CVector3F a;
	a.x = l.x + r.x;
	a.y = l.y + r.y;
	a.z = l.z + r.z;
	return a;
#endif
}

NL_FORCE_INLINE CVector4F add(const CVector4F &l, const CVector4F &r)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_add_ps(l.mm, r.mm);
	return m;
#else
	CVector4F a;
	a.x = l.x + r.x;
	a.y = l.y + r.y;
	a.z = l.z + r.z;
	a.w = l.w + r.w;
	return a;
#endif
}





NL_FORCE_INLINE CVector1F sub(const CVector1F &l, const CVector1F &r)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = _mm_sub_ss(l.mm, r.mm);
	return m;
#else
	CVector1F a;
	a.x = l.x - r.x;
	return a;
#endif
}

NL_FORCE_INLINE CVector2F sub(const CVector2F &l, const CVector2F &r)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_sub_ps(l.mm, r.mm);
	return m;
#else
	CVector2F a;
	a.x = l.x - r.x;
	a.y = l.y - r.y;
	return a;
#endif
}

NL_FORCE_INLINE CVector3F sub(const CVector3F &l, const CVector3F &r)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_sub_ps(l.mm, r.mm);
	return m;
#else
	CVector3F a;
	a.x = l.x - r.x;
	a.y = l.y - r.y;
	a.z = l.z - r.z;
	return a;
#endif
}

NL_FORCE_INLINE CVector4F sub(const CVector4F &l, const CVector4F &r)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_sub_ps(l.mm, r.mm);
	return m;
#else
	CVector4F a;
	a.x = l.x - r.x;
	a.y = l.y - r.y;
	a.z = l.z - r.z;
	a.w = l.w - r.w;
	return a;
#endif
}





NL_FORCE_INLINE CVector1F mul(const CVector1F &l, const CVector1F &r)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = _mm_mul_ss(l.mm, r.mm);
	return m;
#else
	CVector1F a;
	a.x = l.x * r.x;
	return a;
#endif
}

NL_FORCE_INLINE CVector2F mul(const CVector2F &l, const CVector2F &r)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_mul_ps(l.mm, r.mm);
	return m;
#else
	CVector2F a;
	a.x = l.x * r.x;
	a.y = l.y * r.y;
	return a;
#endif
}

NL_FORCE_INLINE CVector3F mul(const CVector3F &l, const CVector3F &r)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_mul_ps(l.mm, r.mm);
	return m;
#else
	CVector3F a;
	a.x = l.x * r.x;
	a.y = l.y * r.y;
	a.z = l.z * r.z;
	return a;
#endif
}

NL_FORCE_INLINE CVector4F mul(const CVector4F &l, const CVector4F &r)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_mul_ps(l.mm, r.mm);
	return m;
#else
	CVector4F a;
	a.x = l.x * r.x;
	a.y = l.y * r.y;
	a.z = l.z * r.z;
	a.w = l.w * r.w;
	return a;
#endif
}





NL_FORCE_INLINE CVector1F div(const CVector1F &l, const CVector1F &r)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = _mm_div_ss(l.mm, r.mm);
	return m;
#else
	CVector1F a;
	a.x = l.x / r.x;
	return a;
#endif
}

NL_FORCE_INLINE CVector2F div(const CVector2F &l, const CVector2F &r)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_div_ps(l.mm, r.mm);
	return m;
#else
	CVector2F a;
	a.x = l.x / r.x;
	a.y = l.y / r.y;
	return a;
#endif
}

NL_FORCE_INLINE CVector3F div(const CVector3F &l, const CVector3F &r)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_div_ps(l.mm, r.mm);
	return m;
#else
	CVector3F a;
	a.x = l.x / r.x;
	a.y = l.y / r.y;
	a.z = l.z / r.z;
	return a;
#endif
}

NL_FORCE_INLINE CVector4F div(const CVector4F &l, const CVector4F &r)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_div_ps(l.mm, r.mm);
	return m;
#else
	CVector4F a;
	a.x = l.x / r.x;
	a.y = l.y / r.y;
	a.z = l.z / r.z;
	a.w = l.w / r.w;
	return a;
#endif
}





NL_FORCE_INLINE CVector1F sqrtF(const CVector1F &v)
{
#if (NL_HAS_SSE2 && NL_HAS_SSE2)
	CVector1F s;
	s.mm = _mm_sqrt_ss(v.mm);
	return s;
#else
	CVector1F s;
	s.x = sqrtf(v.x);
	return s;
#endif
}

NL_FORCE_INLINE CVector2F sqrtF(const CVector2F &v)
{
#if (NL_HAS_SSE2 && NL_HAS_SSE2)
	CVector2F s;
	s.mm = _mm_sqrt_ps(v.mm);
	return s;
#else
	CVector2F s;
	s.x = sqrtf(v.x);
	s.y = sqrtf(v.y);
	return s;
#endif
}

NL_FORCE_INLINE CVector3F sqrtF(const CVector3F &v)
{
#if (NL_HAS_SSE2 && NL_HAS_SSE2)
	CVector3F s;
	s.mm = _mm_sqrt_ps(v.mm);
	return s;
#else
	CVector3F s;
	s.x = sqrtf(v.x);
	s.y = sqrtf(v.y);
	s.z = sqrtf(v.z);
	return s;
#endif
}

NL_FORCE_INLINE CVector4F sqrtF(const CVector4F &v)
{
#if (NL_HAS_SSE2 && NL_HAS_SSE2)
	CVector4F s;
	s.mm = _mm_sqrt_ps(v.mm);
	return s;
#else
	CVector4F s;
	s.x = sqrtf(v.x);
	s.y = sqrtf(v.y);
	s.z = sqrtf(v.z);
	s.w = sqrtf(v.w);
	return s;
#endif
}



NL_FORCE_INLINE CVector1F accumulate(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	return add(x(v), y(v));
#else
	CVector1F result;
	result.x = v.x + v.y;
	return result;
#endif
}

NL_FORCE_INLINE CVector1F accumulate(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	return add(add(x(v), y(v)), z(v));
#else
	CVector1F result;
	result.x = v.x + v.y + v.z;
	return result;
#endif
}

NL_FORCE_INLINE CVector1F accumulate(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	return accumulate(add(xy(v), zw(v)));
#else
	CVector1F result;
	result.x = v.x + v.y + v.z + v.w;
	return result;
#endif
}



NL_FORCE_INLINE CVector3F cross(const CVector3F &l, const CVector3F &r)
{
	return sub(mul(yzx(l), zxy(r)), mul(zxy(l), yzx(r)));
}

	

NL_FORCE_INLINE CVector1F dot(const CVector2F &l, const CVector2F &r)
{
	CVector2F mult = mul(l, r);
	CVector1F result = accumulate(mult);
	return result;
}

NL_FORCE_INLINE CVector1F dot(const CVector3F &l, const CVector3F &r)
{
	CVector3F mult = mul(l, r);
	CVector1F result = accumulate(mult);
	return result;
}

NL_FORCE_INLINE CVector2F dotSplat(const CVector2F &l, const CVector2F &r)
{
	return xx(dot(l, r));
}

NL_FORCE_INLINE CVector3F dotSplat(const CVector3F &l, const CVector3F &r)
{
	return xxx(dot(l, r));
}

NL_FORCE_INLINE CVector1F lengthSq(const CVector2F &v)
{
	return dot(v, v);
}

NL_FORCE_INLINE CVector1F lengthSq(const CVector3F &v)
{
	return dot(v, v);
}

NL_FORCE_INLINE CVector2F lengthSqSplat(const CVector2F &v)
{
	return dotSplat(v, v);
}

NL_FORCE_INLINE CVector3F lengthSqSplat(const CVector3F &v)
{
	return dotSplat(v, v);
}

NL_FORCE_INLINE CVector1F length(const CVector2F &v)
{
	return sqrtF(lengthSq(v));
}

NL_FORCE_INLINE CVector1F length(const CVector3F &v)
{
	return sqrtF(lengthSq(v));
}

NL_FORCE_INLINE CVector2F lengthSplat(const CVector2F &v)
{
	return xx(length(v));
}

NL_FORCE_INLINE CVector3F lengthSplat(const CVector3F &v)
{
	return xxx(length(v));
}


NL_FORCE_INLINE CVector2F normalize(const CVector2F &v)
{
	return div(v, lengthSplat(v));
}

NL_FORCE_INLINE CVector3F normalize(const CVector3F &v)
{
	return div(v, lengthSplat(v));
}

NL_FORCE_INLINE CVector1F minF(const CVector1F &l, const CVector1F &r)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = _mm_min_ss(l.mm, r.mm);
	return m;
#else
	CVector1F m;
	m.x = min(l.x, r.x);
	return m;
#endif
}

NL_FORCE_INLINE CVector1F maxF(const CVector1F &l, const CVector1F &r)
{
#ifdef NL_HAS_SSE2
	CVector1F m;
	m.mm = _mm_max_ss(l.mm, r.mm);
	return m;
#else
	CVector1F m;
	m.x = max(l.x, r.x);
	return m;
#endif
}

NL_FORCE_INLINE CVector2F minF(const CVector2F &l, const CVector2F &r)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_min_ps(l.mm, r.mm);
	return m;
#else
	CVector2F m;
	m.x = min(l.x, r.x);
	m.y = min(l.y, r.y);
	return m;
#endif
}

NL_FORCE_INLINE CVector2F maxF(const CVector2F &l, const CVector2F &r)
{
#ifdef NL_HAS_SSE2
	CVector2F m;
	m.mm = _mm_max_ps(l.mm, r.mm);
	return m;
#else
	CVector2F m;
	m.x = max(l.x, r.x);
	m.y = max(l.y, r.y);
	return m;
#endif
}

NL_FORCE_INLINE CVector3F minF(const CVector3F &l, const CVector3F &r)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_min_ps(l.mm, r.mm);
	return m;
#else
	CVector3F m;
	m.x = min(l.x, r.x);
	m.y = min(l.y, r.y);
	m.z = min(l.z, r.z);
	return m;
#endif
}

NL_FORCE_INLINE CVector3F maxF(const CVector3F &l, const CVector3F &r)
{
#ifdef NL_HAS_SSE2
	CVector3F m;
	m.mm = _mm_max_ps(l.mm, r.mm);
	return m;
#else
	CVector3F m;
	m.x = max(l.x, r.x);
	m.y = max(l.y, r.y);
	m.z = max(l.z, r.z);
	return m;
#endif
}


NL_FORCE_INLINE CVector4F minF(const CVector4F &l, const CVector4F &r)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_min_ps(l.mm, r.mm);
	return m;
#else
	CVector4F m;
	m.x = min(l.x, r.x);
	m.y = min(l.y, r.y);
	m.z = min(l.z, r.z);
	m.w = min(l.w, r.w);
	return m;
#endif
}

NL_FORCE_INLINE CVector4F maxF(const CVector4F &l, const CVector4F &r)
{
#ifdef NL_HAS_SSE2
	CVector4F m;
	m.mm = _mm_max_ps(l.mm, r.mm);
	return m;
#else
	CVector4F m;
	m.x = max(l.x, r.x);
	m.y = max(l.y, r.y);
	m.z = max(l.z, r.z);
	m.w = max(l.w, r.w);
	return m;
#endif
}


NL_FORCE_INLINE CVector3F average(const CVector3F &v0, const CVector3F &v1)
{
	CVector3F divider = splat3F(1.0f / 2.0f);
	return mul(add(v0, v1), divider);
}

NL_FORCE_INLINE CVector3F average(const CVector3F &v0, const CVector3F &v1, const CVector3F &v2)
{
	CVector3F divider = splat3F(1.0f / 3.0f);
	return mul(add(add(v0, v1), v2), divider);
}


NL_FORCE_INLINE CVector3F average(const CVector3F &v0, const CVector3F &v1, const CVector3F &v2, const CVector3F &v3)
{
	CVector3F divider = splat3F(1.0f / 4.0f);
	return mul(add(add(add(v0, v1), v2), v3), divider);
}



NL_FORCE_INLINE CVector4F average(const CVector4F &v0, const CVector4F &v1)
{
	CVector4F divider = splat4F(1.0f / 2.0f);
	return mul(add(v0, v1), divider);
}

NL_FORCE_INLINE CVector4F average(const CVector4F &v0, const CVector4F &v1, const CVector4F &v2)
{
	CVector4F divider = splat4F(1.0f / 3.0f);
	return mul(add(add(v0, v1), v2), divider);
}

NL_FORCE_INLINE CVector4F average(const CVector4F &v0, const CVector4F &v1, const CVector4F &v2, const CVector4F &v3)
{
	CVector4F divider = splat4F(1.0f / 4.0f);
	return mul(add(add(add(v0, v1), v2), v3), divider);
}



NL_FORCE_INLINE CVector3F lerp(const CVector3F &l, const CVector3F &r, const CVector3F &a)
{
	CVector3F one = splat3F(1.0f);
	CVector3F lmul = mul(l, sub(one, a));
	CVector3F rmul = mul(r, a);
	return add(lmul, rmul);
}

NL_FORCE_INLINE CVector3F lerp(const CVector3F &l, const CVector3F &r, const float a)
{
	CVector3F va = splat3F(a);
	return lerp(l, r, va);
}

NL_FORCE_INLINE CVector4F lerp(const CVector4F &l, const CVector4F &r, const CVector4F &a)
{
	CVector4F one = splat4F(1.0f);
	CVector4F lmul = mul(l, sub(one, a));
	CVector4F rmul = mul(r, a);
	return add(lmul, rmul);
}

NL_FORCE_INLINE CVector4F lerp(const CVector4F &l, const CVector4F &r, const float a)
{
	CVector4F va = splat4F(a);
	return lerp(l, r, va);
} 


/*
/// Add all the elements inside one float and splat them
// accumulate()
NL_FORCE_INLINE CVector4F accumulateSplat(const CVector4F &v)
{
#ifdef NL_HAS_SSE3
	CVector4F result;
	result.mm = _mm_hadd_ps(v.mm, v.mm);
	result.mm = _mm_hadd_ps(result.mm, result.mm);
	return result;
#else
	CVector4F result;
	result.x = v.x + v.y + v.z + v.w;
	result.y = result.x;
	result.z = result.x;
	result.w = result.x;
	return result;
#endif
}
*/

NL_ASSIMILATE_BI_FUNCTION(CVector3F, add, const CVector3F &)

NL_ASSIMILATE_BI_FUNCTION(CVector2F, minF, const CVector2F &)
NL_ASSIMILATE_BI_FUNCTION(CVector2F, maxF, const CVector2F &)

NL_ASSIMILATE_BI_FUNCTION(CVector3F, minF, const CVector3F &)
NL_ASSIMILATE_BI_FUNCTION(CVector3F, maxF, const CVector3F &)

NL_ASSIMILATE_BI_FUNCTION(CVector4F, minF, const CVector4F &)
NL_ASSIMILATE_BI_FUNCTION(CVector4F, maxF, const CVector4F &)


NL_CLAMP_USING_MIN_MAX(CVector1F, const CVector1F &)
NL_CLAMP_USING_MIN_MAX(CVector2F, const CVector2F &)
NL_CLAMP_USING_MIN_MAX(CVector3F, const CVector3F &)
NL_CLAMP_USING_MIN_MAX(CVector4F, const CVector4F &)


} /* namespace NLMISC */

#endif /* #ifndef NLMISC_VECTORF_MATH_H */

/* end of file */
