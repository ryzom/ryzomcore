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

#ifndef NL_UV_H
#define NL_UV_H

#include "types_nl.h"
#include "stream.h"


namespace NLMISC
{


// ***************************************************************************
/**
 * 2d UV.
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
class	CUV
{
public:
	float	U,V;

public:
	CUV() {}
	CUV(float u, float v) : U(u), V(v) {}
	// modify uv's
	void set(float u, float v) { U = u; V = v; }
	// bin operators.
	CUV	operator+(const CUV &v) const
		{ return CUV(U+v.U, V+v.V);}
	// binary -
	CUV	operator-(const CUV &v) const
		{ return CUV(U-v.U, V-v.V);}
	// unary -
	CUV operator-() const
		{ return CUV(-U, -V); }
	// = operators.
	CUV	&operator*=(float f)
		{ U*=f;V*=f; return *this;}
	CUV	&operator+=(const CUV &v)
		{ U+=v.U;V+=v.V; return *this;}
	CUV	&operator-=(const CUV &v)
		{ U-=v.U;V-=v.V; return *this;}
	/// This operator is here just for map/set insertion (no meaning). comparison order is U,V.
	bool	operator<(const CUV &o) const
	{
		if(U!=o.U)
			return U<o.U;
		return V<o.V;
	}
	bool	operator==(const CUV &c) const {return (U==c.U) && (V==c.V);}
	bool	operator!=(const CUV &c) const {return !(*this==c);}

	void	serial(NLMISC::IStream &f)	{f.serial(U,V);}
};


inline CUV operator * (float f, const CUV &uv)
{
	return CUV(uv.U * f, uv.V * f);
}


inline CUV operator * (const CUV &uv, float f)
{
	return f * uv;
}

// blend (faster version than the generic version found in algo.h)
inline CUV blend(const CUV &uv0, const CUV &uv1, float lambda)
{
	float invLambda = 1.f - lambda;
	return CUV(invLambda * uv0.U + lambda * uv1.U,
		       invLambda * uv0.V + lambda * uv1.V);

}



// ***************************************************************************
/**
 * 3d UV.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CUVW
{
public:
	float	U,V,W;

public:
	CUVW() {}
	CUVW(float u, float v, float w) : U(u), V(v), W(w) {}
	// bin operators.
	CUVW	operator+(const CUVW &v) const
		{ return CUVW(U+v.U, V+v.V, W+v.W);}
	CUVW	operator-(const CUVW &v) const
		{ return CUVW(U-v.U, V-v.V, W-v.W);}
	CUVW	operator*(float f) const
		{ return CUVW(U*f, V*f, W*f);}
	// = operators.
	CUVW	&operator*=(float f)
		{ U*=f;V*=f; W*=f; return *this;}
	CUVW	&operator+=(const CUVW &v)
		{ U+=v.U;V+=v.V; W+=v.W; return *this;}
	CUVW	&operator-=(const CUVW &v)
		{ U-=v.U;V-=v.V; W-=v.W; return *this;}
	/// This operator is here just for map/set insertion (no meaning). comparison order is U,V.
	bool	operator<(const CUVW &o) const
	{
		if(U!=o.U)
			return U<o.U;
		if(V!=o.V)
			return V<o.V;
		return W<o.W;
	}
	bool	operator==(const CUVW &c) const {return (U==c.U) && (V==c.V) && (W==c.W);}
	bool	operator!=(const CUVW &c) const {return !(*this==c);}

	void	serial(NLMISC::IStream &f)	{f.serial(U,V,W);}

	// Convert to a standard UV. The W coordinate is lost of course..
	operator CUV() const { return CUV(U, V); }
};


} // NLMISC


#endif // NL_UV_H

/* End of uv.h */
