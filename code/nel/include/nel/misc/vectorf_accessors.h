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

#ifndef NLMISC_VECTORF_ACCESSORS_H
#define NLMISC_VECTORF_ACCESSORS_H
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

NL_FORCE_INLINE float getX(const CVector1F &v)
{
#ifdef NL_HAS_SSE2
	return _mm_cvtss_f32(v.mm);
#else
	return v.x;
#endif
}

NL_FORCE_INLINE float getX(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	return _mm_cvtss_f32(v.mm);
#else
	return v.x;
#endif
}

NL_FORCE_INLINE float getY(const CVector2F &v)
{
#ifdef NL_HAS_SSE2
	CVector2F g;
	g.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 1, 1));
	return _mm_cvtss_f32(g.mm);
#else
	return v.y;
#endif
}

NL_FORCE_INLINE float getX(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	return _mm_cvtss_f32(v.mm);
#else
	return v.x;
#endif
}

NL_FORCE_INLINE float getY(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F g;
	g.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 1, 1));
	return _mm_cvtss_f32(g.mm);
#else
	return v.y;
#endif
}

NL_FORCE_INLINE float getZ(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F g;
	g.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 2, 2));
	return _mm_cvtss_f32(g.mm);
#else
	return v.z;
#endif
}

NL_FORCE_INLINE float getX(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	return _mm_cvtss_f32(v.mm);
#else
	return v.x;
#endif
}

NL_FORCE_INLINE float getY(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F g;
	g.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(1, 1, 1, 1));
	return _mm_cvtss_f32(g.mm);
#else
	return v.y;
#endif
}

NL_FORCE_INLINE float getZ(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F g;
	g.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(2, 2, 2, 2));
	return _mm_cvtss_f32(g.mm);
#else
	return v.z;
#endif
}

NL_FORCE_INLINE float getW(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F g;
	g.mm = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 3, 3, 3));
	return _mm_cvtss_f32(g.mm);
#else
	return v.w;
#endif
}

NL_FORCE_INLINE CVector2F setX(const CVector2F &src, const float x)
{
#ifdef NL_HAS_SSE2
	CVector1F v1x;
	v1x.mm = _mm_set_ss(x);
	CVector2F result;
	result.mm = _mm_move_ss(src.mm, v1x.mm);
	return result;
#else
	CVector2F result = src;
	result.x = x;
	return result;
#endif
}

NL_FORCE_INLINE CVector3F setX(const CVector3F &src, const float x)
{
#ifdef NL_HAS_SSE2
	CVector1F v1x;
	v1x.mm = _mm_set_ss(x);
	CVector3F result;
	result.mm = _mm_move_ss(src.mm, v1x.mm);
	return result;
#else
	CVector3F result = src;
	result.x = x;
	return result;
#endif
}

NL_FORCE_INLINE CVector4F setX(const CVector4F &src, const float x)
{
#ifdef NL_HAS_SSE2
	CVector1F v1x;
	v1x.mm = _mm_set_ss(x);
	CVector4F result;
	result.mm = _mm_move_ss(src.mm, v1x.mm);
	return result;
#else
	CVector4F result = src;
	result.x = x;
	return result;
#endif
}



NL_FORCE_INLINE CVector2F setY(const CVector2F &src, const float y)
{
#ifdef NL_HAS_SSE2
	return yx(setX(yx(src), y));
#else
	CVector2F result = src;
	result.y = y;
	return result;
#endif
}

NL_FORCE_INLINE CVector3F setY(const CVector3F &src, const float y)
{
#ifdef NL_HAS_SSE2
	return yxz(setX(yxz(src), y));
#else
	CVector3F result = src;
	result.y = y;
	return result;
#endif
}

NL_FORCE_INLINE CVector4F setY(const CVector4F &src, const float y)
{
#ifdef NL_HAS_SSE2
	return yxzw(setX(yxzw(src), y));
#else
	CVector4F result = src;
	result.y = y;
	return result;
#endif
}



NL_FORCE_INLINE CVector3F setZ(const CVector3F &src, const float z)
{
#ifdef NL_HAS_SSE2
	return zyx(setX(zyx(src), z));
#else
	CVector3F result = src;
	result.z = z;
	return result;
#endif
}

NL_FORCE_INLINE CVector4F setZ(const CVector4F &src, const float z)
{
#ifdef NL_HAS_SSE2
	return zyxw(setX(zyxw(src), z));
#else
	CVector4F result = src;
	result.z = z;
	return result;
#endif
}



NL_FORCE_INLINE CVector4F setW(const CVector4F &src, const float w)
{
#ifdef NL_HAS_SSE2
	return wyzx(setX(wyzx(src), w));
#else
	CVector4F result = src;
	result.w = w;
	return result;
#endif
}



NL_FORCE_INLINE CVector2F splat2F(const float v)
{
#ifdef NL_HAS_SSE2
	CVector2F s;
	s.mm = _mm_set_ps1(v);
	return s;
#else
	CVector2F s;
	s.x = v;
	s.y = v;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F splat3F(const float v)
{
#ifdef NL_HAS_SSE2
	CVector3F s;
	s.mm = _mm_set_ps1(v);
	return s;
#else
	CVector3F s;
	s.x = v;
	s.y = v;
	s.z = v;
	return s;
#endif
}



NL_FORCE_INLINE CVector1F set1F(const float x)
{
#ifdef NL_HAS_SSE2
	CVector1F s;
	s.mm = _mm_set_ss(x);
	return s;
#else
	CVector1F s;
	s.x = x;
	return s;
#endif
}

NL_FORCE_INLINE CVector2F set2F(const float x, const float y)
{
#ifdef NL_HAS_SSE2
	CVector2F s;
	s.mm = _mm_setr_ps(x, y, 0.0f, 0.0f);
	return s;
#else
	CVector2F s;
	s.x = x;
	s.y = y;
	return s;
#endif
}

NL_FORCE_INLINE CVector3F set3F(const CVector1F &x, const CVector1F &y, const CVector1F &z)
{
	// todo: optimize
#ifdef NL_HAS_SSE2
	CVector3F s;
	s.mm = _mm_setr_ps(getX(x), getX(y), getX(z), 0.0f);
	return s;
#else
	CVector3F s;
	s.x = getX(x);
	s.y = getX(y);
	s.z = getX(z);
	return s;
#endif
}

NL_FORCE_INLINE CVector3F set3F(const float x, const float y, const float z)
{
#ifdef NL_HAS_SSE2
	CVector3F s;
	s.mm = _mm_setr_ps(x, y, z, 0.0f);
	return s;
#else
	CVector3F s;
	s.x = x;
	s.y = y;
	s.z = z;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F set4F(const CVector2F &xy)
{
#ifdef NL_HAS_SSE2
	CVector4F s;
	s.mm = xy.mm; // z, w "undefined"
	return s;
#else
	CVector4F s;
	s.x = getX(xy);
	s.y = getY(xy);
	// s.z = 0.0f;
	// s.w = 0.0f;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F set4F(const CVector3F &xyz)
{
#ifdef NL_HAS_SSE2
	CVector4F s;
	s.mm = xyz.mm; // w "undefined"
	return s;
#else
	CVector4F s;
	s.x = getX(xyz);
	s.y = getY(xyz);
	s.z = getZ(xyz);
	// s.w = 0.0f;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F set4F(const CVector3F &xyz, const float w)
{
#ifdef NL_HAS_SSE2
	CVector4F s;
	s.mm = xyz.mm;
	s = setW(s, w);
	return s;
#else
	CVector4F s;
	s.x = getX(xyz);
	s.y = getY(xyz);
	s.z = getZ(xyz);
	s.w = w;
	return s;
#endif
}

NL_FORCE_INLINE CVector4F set4F(const float x, const float y, const float z, const float w)
{
#ifdef NL_HAS_SSE2
	CVector4F s;
	s.mm = _mm_setr_ps(x, y, z, w);
	return s;
#else
	CVector4F s;
	s.x = x;
	s.y = y;
	s.z = z;
	s.w = w;
	return s;
#endif
}

NL_FORCE_INLINE CVector1F zero1F()
{
#ifdef NL_HAS_SSE2
	CVector1F z;
	z.mm = _mm_setzero_ps();
	return z;
#else
	CVector1F z;
	z.x = 0.0f;
	return z;
#endif
}

NL_FORCE_INLINE CVector2F zero2F()
{
#ifdef NL_HAS_SSE2
	CVector2F z;
	z.mm = _mm_setzero_ps();
	return z;
#else
	CVector2F z;
	z.x = 0.0f;
	z.y = 0.0f;
	return z;
#endif
}

NL_FORCE_INLINE CVector3F zero3F()
{
#ifdef NL_HAS_SSE2
	CVector3F z;
	z.mm = _mm_setzero_ps();
	return z;
#else
	CVector3F z;
	z.x = 0.0f;
	z.y = 0.0f;
	z.z = 0.0f;
	return z;
#endif
}


NL_FORCE_INLINE CVector4F zero4F()
{
#ifdef NL_HAS_SSE2
	CVector4F z;
	z.mm = _mm_setzero_ps();
	return z;
#else
	CVector4F z;
	z.x = 0.0f;
	z.y = 0.0f;
	z.z = 0.0f;
	z.w = 0.0f;
	return z;
#endif
}

NL_FORCE_INLINE CVector4F splat4F(const float v)
{
#ifdef NL_HAS_SSE2
	CVector4F s;
	s.mm = _mm_set_ps1(v);
	return s;
#else
	CVector4F s;
	s.x = v;
	s.y = v;
	s.z = v;
	s.w = v;
	return s;
#endif
}


NL_FORCE_INLINE CVector4F to4F(const CVector3F &v)
{
#ifdef NL_HAS_SSE2
	CVector4F r;
	r.mm = v.mm;
	return r;
#else
	CVector4F dst;
	dst.x = v.x;
	dst.y = v.y;
	dst.z = v.z;
	dst.w = 0.0f;
	return dst;
#endif
}


NL_FORCE_INLINE CVector3F to3F(const CVector4F &v)
{
#ifdef NL_HAS_SSE2
	CVector3F r;
	r.mm = v.mm;
	return r;
#else
	CVector3F dst;
	dst.x = v.x;
	dst.y = v.y;
	dst.z = v.z;
	return dst;
#endif
}



NL_FORCE_INLINE void swapX(CVector1F &left, CVector1F &right)
{
#ifdef NL_HAS_SSE2
	CVector1F cache = left;
	left.mm = _mm_move_ss(left.mm, right.mm);
	right.mm = _mm_move_ss(right.mm, cache.mm);
#else
	float x = getX(left);
	left = set1F(getX(right));
	right = set1F(x);
#endif
}

NL_FORCE_INLINE void swapX(CVector2F &left, CVector2F &right)
{
#ifdef NL_HAS_SSE2
	CVector2F cache = left;
	left.mm = _mm_move_ss(left.mm, right.mm);
	right.mm = _mm_move_ss(right.mm, cache.mm);
#else
	float x = getX(left);
	left = setX(left, getX(right));
	right = setX(right, x);
#endif
}

NL_FORCE_INLINE void swapX(CVector3F &left, CVector3F &right)
{
#ifdef NL_HAS_SSE2
	CVector3F cache = left;
	left.mm = _mm_move_ss(left.mm, right.mm);
	right.mm = _mm_move_ss(right.mm, cache.mm);
#else
	float x = getX(left);
	left = setX(left, getX(right));
	right = setX(right, x);
#endif
}

NL_FORCE_INLINE void swapX(CVector4F &left, CVector4F &right)
{
#ifdef NL_HAS_SSE2
	CVector4F cache = left;
	left.mm = _mm_move_ss(left.mm, right.mm);
	right.mm = _mm_move_ss(right.mm, cache.mm);
#else
	float x = getX(left);
	left = setX(left, getX(right));
	right = setX(right, x);
#endif
}


NL_FORCE_INLINE void swapY(CVector2F &left, CVector2F &right)
{
#ifdef NL_HAS_SSE2
	left = yx(left);
	right = yx(right);
	CVector2F cache = left;
	left.mm = _mm_move_ss(left.mm, right.mm);
	right.mm = _mm_move_ss(right.mm, cache.mm);
	left = yx(left);
	right = yx(right);
#else
	float y = getY(left);
	left = setY(left, getY(right));
	right = setY(right, y);
#endif
}

NL_FORCE_INLINE void swapY(CVector3F &left, CVector3F &right)
{
#ifdef NL_HAS_SSE2
	left = yxz(left);
	right = yxz(right);
	CVector3F cache = left;
	left.mm = _mm_move_ss(left.mm, right.mm);
	right.mm = _mm_move_ss(right.mm, cache.mm);
	left = yxz(left);
	right = yxz(right);
#else
	float y = getY(left);
	left = setY(left, getY(right));
	right = setY(right, y);
#endif
}

NL_FORCE_INLINE void swapY(CVector4F &left, CVector4F &right)
{
#ifdef NL_HAS_SSE2
	left = yxzw(left);
	right = yxzw(right);
	CVector4F cache = left;
	left.mm = _mm_move_ss(left.mm, right.mm);
	right.mm = _mm_move_ss(right.mm, cache.mm);
	left = yxzw(left);
	right = yxzw(right);
#else
	float y = getY(left);
	left = setY(left, getY(right));
	right = setY(right, y);
#endif
}


NL_FORCE_INLINE void swapZ(CVector3F &left, CVector3F &right)
{
#ifdef NL_HAS_SSE2
	left = zyx(left);
	right = zyx(right);
	CVector3F cache = left;
	left.mm = _mm_move_ss(left.mm, right.mm);
	right.mm = _mm_move_ss(right.mm, cache.mm);
	left = zyx(left);
	right = zyx(right);
#else
	float z = getZ(left);
	left = setZ(left, getZ(right));
	right = setZ(right, z);
#endif
}

NL_FORCE_INLINE void swapZ(CVector4F &left, CVector4F &right)
{
#ifdef NL_HAS_SSE2
	left = zyxw(left);
	right = zyxw(right);
	CVector4F cache = left;
	left.mm = _mm_move_ss(left.mm, right.mm);
	right.mm = _mm_move_ss(right.mm, cache.mm);
	left = zyxw(left);
	right = zyxw(right);
#else
	float z = getZ(left);
	left = setZ(left, getZ(right));
	right = setZ(right, z);
#endif
}


NL_FORCE_INLINE void swapW(CVector4F &left, CVector4F &right)
{
#ifdef NL_HAS_SSE2
	left = wyzx(left);
	right = wyzx(right);
	CVector4F cache = left;
	left.mm = _mm_move_ss(left.mm, right.mm);
	right.mm = _mm_move_ss(right.mm, cache.mm);
	left = wyzx(left);
	right = wyzx(right);
#else
	float w = getW(left);
	left = setW(left, getW(right));
	right = setW(right, w);
#endif
}


} /* namespace NLMISC */

#endif /* #ifndef NLMISC_VECTORF_ACCESSORS_H */

/* end of file */
