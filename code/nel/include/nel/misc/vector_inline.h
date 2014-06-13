// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef NL_VECTOR_INLINE_H
#define NL_VECTOR_INLINE_H


#include "types_nl.h"
#include "common.h"


namespace	NLMISC
{


// ============================================================================================
// Base Maths.
inline	CVector	&CVector::operator+=(const CVector &v)
{
#ifdef USE_SSE2
	mm = _mm_add_ps(mm, v.mm);
#else
	x+=v.x;
	y+=v.y;
	z+=v.z;
#endif
	return *this;
}
inline	CVector	&CVector::operator-=(const CVector &v)
{
#ifdef USE_SSE2
	mm = _mm_sub_ps(mm, v.mm);
#else
	x-=v.x;
	y-=v.y;
	z-=v.z;
#endif
	return *this;
}
inline	CVector	&CVector::operator*=(float f)
{
#ifdef USE_SSE2
	mm = _mm_mul_ps(mm, _mm_set1_ps(f));
#else
	x*=f;
	y*=f;
	z*=f;
#endif
	return *this;
}
inline	CVector	&CVector::operator/=(float f)
{
#ifdef USE_SSE2
	mm = _mm_div_ps(mm, _mm_set1_ps(f));
	return *this;
#else
	return *this*= (1.0f/f);
#endif
}
inline	CVector	CVector::operator+(const CVector &v) const
{
#ifdef USE_SSE2
	CVector res;
	res.mm = _mm_add_ps(mm, v.mm);
	return res;
#else
	CVector	ret(x+v.x, y+v.y, z+v.z);
	return ret;
#endif
}
inline	CVector	CVector::operator-(const CVector &v) const
{
#ifdef USE_SSE2
	CVector res;
	res.mm = _mm_sub_ps(mm, v.mm);
	return res;
#else
	CVector	ret(x-v.x, y-v.y, z-v.z);
	return ret;
#endif
}
inline	CVector	CVector::operator*(float f) const
{
#ifdef USE_SSE2
	CVector res;
	res.mm = _mm_mul_ps(mm, _mm_set1_ps(f));
	return res;
#else
	CVector	ret(x*f, y*f, z*f);
	return ret;
#endif
}
inline	CVector	CVector::operator/(float f) const
{
#ifdef USE_SSE2
	CVector res;
	res.mm = _mm_div_ps(mm, _mm_set1_ps(f));
	return res;
#else
	return *this*(1.0f/f);
#endif
}
inline	CVector	CVector::operator-() const
{
#ifdef USE_SSE2
	CVector res;
	res.mm = _mm_mul_ps(mm, _mm_set1_ps(-1.0f));
	return res;
#else
	return CVector(-x,-y,-z);
#endif
}
inline CVector	operator*(float f, const CVector &v)
{
#ifdef USE_SSE2
	CVector res;
	res.mm = _mm_mul_ps(v.mm, _mm_set1_ps(f));
	return res;
#else
	CVector	ret(v.x*f, v.y*f, v.z*f);
	return ret;
#endif
}

#ifdef USE_SSE2
inline __m128 dotsplat(const __m128 &l, const __m128 &r)
{
	// TODO: _mm_hadd_ps SSE3

	__m128 mult = _mm_mul_ps(l, r);
	__m128 vx = _mm_shuffle_ps(mult, mult, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 vy = _mm_shuffle_ps(mult, mult, _MM_SHUFFLE(1, 1, 1, 1));
	__m128 vz = _mm_shuffle_ps(mult, mult, _MM_SHUFFLE(2, 2, 2, 2));
	__m128 result = _mm_add_ps(_mm_add_ps(vx, vy), vz);
	return result;
}
#endif

// ============================================================================================
// Advanced Maths.
inline	float	CVector::operator*(const CVector &v) const
{
#ifdef USE_SSE2
	return _mm_cvtss_f32(dotsplat(mm, v.mm));
#else
	return x*v.x + y*v.y + z*v.z;
#endif
}
inline	CVector	CVector::operator^(const CVector &v) const
{
#ifdef USE_SSE2
	CVector res;
	__m128 l = _mm_shuffle_ps(mm, mm, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 r = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 mul1 = _mm_mul_ps(l, r);
	l = _mm_shuffle_ps(mm, mm, _MM_SHUFFLE(3, 1, 0, 2));
	r = _mm_shuffle_ps(v.mm, v.mm, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 mul2 = _mm_mul_ps(l, r);
	res.mm = _mm_sub_ps(mul1, mul2);
	return res;
#else
	CVector	ret;

	ret.x= y*v.z - z*v.y;
	ret.y= z*v.x - x*v.z;
	ret.z= x*v.y - y*v.x;

	return ret;
#endif
}
inline	float	CVector::sqrnorm() const
{
#ifdef USE_SSE2
	return _mm_cvtss_f32(dotsplat(mm, mm));
#else
	return (float)(x*x + y*y + z*z);
#endif
}
inline	float	CVector::norm() const
{
#ifdef USE_SSE2
	return sqrt(sqrnorm());
#else
	return (float)sqrt(x*x + y*y + z*z);
#endif
}
inline	void	CVector::normalize()
{
#ifdef USE_SSE2
	__m128 normsplat = _mm_sqrt_ps(dotsplat(mm, mm));
	if (_mm_cvtss_f32(normsplat))
		mm = _mm_div_ps(mm, normsplat);
#else
	float	n=norm();
	if(n)
		*this/=n;
#endif
}
inline	CVector	CVector::normed() const
{
#ifdef USE_SSE2
	CVector res;
	__m128 normsplat = _mm_sqrt_ps(dotsplat(mm, mm));
	if (_mm_cvtss_f32(normsplat))
		res.mm = _mm_div_ps(mm, normsplat);
	return res;
#else
	CVector	ret;
	ret= *this;
	ret.normalize();
	return ret;
#endif
}


// ============================================================================================
// Misc.
inline	void	CVector::set(float _x, float _y, float _z)
{
#ifdef USE_SSE2
	mm = _mm_setr_ps(_x, _y, _z, 0.0f);
#else
	x=_x; y=_y; z=_z;
#endif
}
inline	bool	CVector::operator==(const CVector &v) const
{
#ifdef USE_SSE2
	return (_mm_movemask_ps(_mm_cmpeq_ps(mm, v.mm)) & 0x07) == 0x07;
#else
	return x==v.x && y==v.y && z==v.z;
#endif
}
inline	bool	CVector::operator!=(const CVector &v) const
{
#ifdef USE_SSE2
	return (_mm_movemask_ps(_mm_cmpneq_ps(mm, v.mm)) & 0x07) != 0;
#else
	return !(*this==v);
#endif
}
inline	bool	CVector::isNull() const
{
#ifdef USE_SSE2
	return (_mm_movemask_ps(_mm_cmpeq_ps(mm, _mm_setzero_ps())) & 0x07) == 0x07;
#else
	return *this==CVector::Null;
#endif
}
inline	bool	CVector::operator<(const CVector &v) const
{
	if(x!=v.x)
		return x<v.x;
	if(y!=v.y)
		return y<v.y;
	return z<v.z;
}

inline	void	CVector::cartesianToSpheric(float &r, float &theta,float &phi) const
{
	CVector v;

	r= norm();
	v= normed();

	// phi E [-PI/2 et PI/2]
	clamp(v.z, -1.0f, 1.0f);
	phi= (float)asin(v.z);

	// theta [-PI,PI]
	theta= (float)atan2(v.y,v.x);
}
inline	void	CVector::sphericToCartesian(float r, float theta,float phi)
{
	double	ct= cos(theta);
	double	st= sin(theta);
	double	cp= cos(phi);
	double	sp= sin(phi);

	x= (float)(r*ct*cp);
	y= (float)(r*st*cp);
	z= (float)(r*sp);
}
inline	void	CVector::minof(const CVector &a, const CVector &b)
{
#ifdef USE_SSE2
	mm = _mm_min_ps(a.mm, b.mm);
#else
	x= std::min(a.x, b.x);
	y= std::min(a.y, b.y);
	z= std::min(a.z, b.z);
#endif
}
inline	void	CVector::maxof(const CVector &a, const CVector &b)
{
#ifdef USE_SSE2
	mm = _mm_max_ps(a.mm, b.mm);
#else
	x= std::max(a.x, b.x);
	y= std::max(a.y, b.y);
	z= std::max(a.z, b.z);
#endif
}
inline	void	CVector::serial(IStream &f)
{
	f.serial(x,y,z);
}


}


#endif

