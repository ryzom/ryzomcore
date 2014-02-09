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

#ifndef NL_QUAT_H
#define NL_QUAT_H

#include "types_nl.h"
#include "vector.h"
#include "stream.h"

#include <cmath>

namespace	NLMISC
{


// ***************************************************************************
const	double	QuatEpsilon= 0.000001;



// ***************************************************************************
/**
 * An AngleAxis.
 * \author Antoine Viau.
 * \author Nevrax France
 * \date 2000
 */
struct	CAngleAxis
{
	CVector		Axis;		/// an axis.
	float		Angle;		/// angle in radians.

	CAngleAxis() {}
	CAngleAxis(const CVector &axis, float ang) : Axis(axis), Angle(ang) {}

	/// serial.
	void	serial(IStream &f)
	{
		f.serial(Axis);
		f.serial(Angle);
	}
};


// ***************************************************************************
/**
 * A Template quaternion. Use CQuat and CQuatD.
 * \author Antoine Viau.
 * \author Nevrax France
 * \date 2000
 */
template <class T> class CQuatT
{
public:
	T x,y,z,w;


public:

	/// \name Object
	// @{
	CQuatT() : x((T)0.0),y((T)0.0),z((T)0.0),w((T)1.0) {}
	CQuatT(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
	/// ctor of a UNIT quaternion, from an angle axis.
	CQuatT(const CVector &axis, float angle) {setAngleAxis(axis, angle);}
	/// ctor of a UNIT quaternion, from an angle axis.
	CQuatT(const CAngleAxis &aa) {setAngleAxis(aa);}
	// @}


	/// \name Sets.
	// @{
	void	set(T X, T Y, T Z, T W)		{x= X; y= Y; z= Z; w= W;}
	// @}

	/// \name Comparison
	// @{
	bool	operator==(const CQuatT& a) const		{return (x==a.x && y==a.y && z==a.z && w==a.w);}
	bool	equal(const CQuatT& a, float epsilon = 1E-6f) const;
	void	identity()						{x = y = z = 0.0f ;	w = 1.0f; }
	bool	isIdentity() const				{return (x==0.0f && y==0.0f && z==0.0f && w==1.0f);}
	// @}

	/// \name 4D vector operations.
	// @{
	CQuatT&	operator+=(const CQuatT&o)		{x+=o.x; y+=o.y; z+=o.z; w+=o.w; return *this;}
	CQuatT&	operator-=(const CQuatT&o)		{x-=o.x; y-=o.y; z-=o.z; w-=o.w; return *this;}
	CQuatT&	operator*=(T f)					{x*=f;y*=f;z*=f;w*=f; return *this;}
	CQuatT&	operator/=(T f)					{double oof= 1.0/f; x=(T)(x*oof); y=(T)(y*oof); z= (T)(z*oof); w=(T)(w*oof); return *this;}
	CQuatT	operator+(const CQuatT&o) const	{return CQuatT(x+o.x,y+o.y,z+o.z,w+o.w);}
	CQuatT	operator-(const CQuatT&o) const	{return CQuatT(x-o.x,y-o.y,z-o.z,w-o.w);}
	CQuatT	operator*(T f) const			{return CQuatT(x*f,y*f,z*f,w*f);}
	CQuatT	operator/(T f) const			{double oof= 1.0/f; return CQuatT(x*oof,y*oof,z*oof,w*oof);}
	CQuatT	operator-() const				{return(CQuatT(-x,-y,-z,-w)); }
	CQuatT	operator+() const				{return *this; }
	/// return the square of the norm of the 4D vector.
	T		sqrnorm() const					{return (x*x + y*y + z*z + w*w);}
	/// return the norm of the 4D vector.
	T		norm() const					{return (T)sqrt(sqrnorm());}
	/// Normalize the quaternion.
	void	normalize();
	/// Return the quaternion normalized.
	CQuatT	normed() const					{CQuatT	ret= *this; ret.normalize(); return ret;}
	// @}


	/// \name Basic Quaternions operations.
	// @{
	/// Quaternion multiplication/composition.
	CQuatT	operator*(const CQuatT&) const;
	CQuatT&	operator*=(const CQuatT&);
	/// Invert this quaternion. If normalized, conjugate is faster and does same thing.
	void	invert();
	/// return the quaternion inverted.
	CQuatT	inverted() const				{CQuatT	ret= *this; ret.invert(); return ret;}
	/// return the conjugate of this quaternion.
	CQuatT	conjugate() const				{return CQuatT(-x, -y, -z, w);}
	// @}


	/// \name To/From other orientation.
	// @{
	/// Return the equivalent Unit axis of this quaternion.
	CVector	getAxis() const {CVector ret((float)x,(float)y,(float)z); return ret.normed();}
	/// Return the equivalent angle of this quaternion. (in radian).
	float	getAngle() const {return (float)(2*acos(w/norm()));}
	/// Return the equivalent Unit  AngleAxis of this quaternion.
	CAngleAxis	getAngleAxis() const {return CAngleAxis(getAxis(), getAngle());}

	/// Build a UNIT quaternion from an AngleAxis.
	void	setAngleAxis(const CVector &axis, float angle);
	/// Build a UNIT quaternion from an AngleAxis.
	void	setAngleAxis(const CAngleAxis &angAxis) {setAngleAxis(angAxis.Axis, angAxis.Angle);}
	// @}


	/// \name Misc.
	// @{
	/// compute logn quaternion.
	CQuatT	log();
	/// compute quaternion exponent.
	CQuatT	exp();
	/// ensure that *this and q are on same side of hypersphere, ie dotProduct(*this,q) is >0, modifying this if necessary.
	void	makeClosest(const CQuatT &o);
	/// serial.
	void	serial(IStream &f)
	{
		f.serial(x,y,z,w);
	}
	// @}


public:

	/// \name Quaternions static functions.
	// @{
	/// Return the dotProduct of 2 quaternions.
	static	T		dotProduct(const CQuatT<T> &q0, const CQuatT<T> &q1);
	/** Quaternion spherical linear interpolation. when t==0, ret==q0, when t==1, ret==q1.
	 * No hemisphere correction is made.
	 */
	static	CQuatT	slerp(const CQuatT<T>& q0, const CQuatT<T>& q1, float t);
	/** Quaternion Quadratic spherical linear interpolation. when t==0, ret==q0, when t==1, ret==q1.
	 * No hemisphere correction is made.
	 */
	static	CQuatT	squad(const CQuatT<T>& q0, const CQuatT<T>& tgtQ0, const CQuatT<T>& tgtQ1, const CQuatT<T>& q1, float t);
	/** Quaternion Quadratic spherical linear interpolation, with multi revision support.
	 */
	static	CQuatT	squadrev(const CAngleAxis &rot, const CQuatT<T>& q0, const CQuatT<T>& tgtQ0, const CQuatT<T>& tgtQ1, const CQuatT<T>& q1, float t);

	/// compute lnDiff of q0.inverted()*q1.
	static	CQuatT	lnDif(const CQuatT &q0, const CQuatT &q1);

	// @}


};



// ***************************************************************************


/// \name Quaternions functions.
// @{

/// f*quat operator
template <class T>
inline	CQuatT<T>	operator*(T f, const CQuatT<T> &o) {return o*f;}

// @}



// ***************************************************************************
// ***************************************************************************
// Template implementation.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
template <class T>
inline bool	CQuatT<T>::equal(const CQuatT<T>& a, float epsilon) const
{
	if (fabs(x-a.x)<epsilon &&
		fabs(y-a.y)<epsilon &&
		fabs(z-a.z)<epsilon &&
		fabs(w-a.w)<epsilon )
	{
		return true;
	}
	return false;
}


// ***************************************************************************
template <class T>
inline CQuatT<T>	CQuatT<T>::operator*(const CQuatT<T>& o) const
{
	// wres= ww' - v.v'
	// vres= wv' + w'v + v^v' ]
	return	CQuatT<T>(
					(w*o.x) +(x*o.w) + (y*o.z)-(z*o.y),
					(w*o.y) +(y*o.w) + (z*o.x)-(x*o.z),
					(w*o.z) +(z*o.w) + (x*o.y)-(y*o.x),
					(w*o.w)-(x*o.x)-(y*o.y)-(z*o.z) );

}

// ***************************************************************************
template <class T>
inline CQuatT<T>&	CQuatT<T>::operator*=(const CQuatT<T>& o)
{
	*this= *this * o;
	return *this;
}


// ***************************************************************************
template <class T>
inline void	CQuatT<T>::invert()
{
	// Must invert the norm.
	T	f= sqrnorm();
	if(f!=0)
	{
		*this/=f;
	}

	*this= conjugate();
}


// ***************************************************************************
template <class T>
inline void	CQuatT<T>::normalize()
{
	T	f= norm();
	if(f==0)
		identity();
	else
	{
		*this/=f;
	}
}


// ***************************************************************************
template <class T>
inline void	CQuatT<T>::setAngleAxis(const CVector &axis, float angle)
{
	CVector		v= axis;
	v.normalize();
	double	ca= cos(angle/2);
	double	sa= sin(angle/2);
	x= (T)(v.x*sa);
	y= (T)(v.y*sa);
	z= (T)(v.z*sa);
	w= (T)(ca);
}


// ***************************************************************************
template <class T>
T	CQuatT<T>::dotProduct(const CQuatT<T> &q0, const CQuatT<T> &q1)
{
	return q0.x*q1.x + q0.y*q1.y + q0.z*q1.z + q0.w*q1.w;
}


// ***************************************************************************
template <class T>
CQuatT<T>	CQuatT<T>::slerp(const CQuatT<T>& q0, const CQuatT<T>& q1, float t)
{
	// omega is the 4D angle between q0 and q1.
	double	omega, cosom,sinom;
	T	factq0= 1;
	T	s0,s1;

	cosom = CQuatT<T>::dotProduct(q0, q1);

	// Make q0 and q1 on the same hemisphere.
	/*if(cosom<0)
	{
		cosom= -cosom;
		factq0= -1;
	}*/
	// ????

	if ( cosom < 1.0 - NLMISC::QuatEpsilon)
	{
		omega = acos(cosom);
		sinom = sin(omega);
		s0 = (T) (sin((1.0f - t)*omega) / sinom);
		s1 = (T) (sin(t*omega) / sinom);
	}
	else
	{	// q0 and q1 are nearly the same => sinom nearly 0. We can't slerp.
		// just linear interpolate.
		s0 = (T)(1.0 - t);
		s1 = t;
	}

	return	q0*(factq0*s0) + q1*s1;

}


// ***************************************************************************
template <class T>
CQuatT<T>	CQuatT<T>::squad(const CQuatT<T>& q0, const CQuatT<T>& tgtQ0, const CQuatT<T>& tgtQ1, const CQuatT<T>& q1, float t)
{
	return CQuatT<T>::slerp(
		CQuatT<T>::slerp(q0, q1, t),
		CQuatT<T>::slerp(tgtQ0, tgtQ1, t),
		2.f*(1.f-t)*t);
}


// ***************************************************************************
template <class T>
CQuatT<T>	CQuatT<T>::squadrev(const CAngleAxis &rot, const CQuatT<T>& q0, const CQuatT<T>& tgtQ0, const CQuatT<T>& tgtQ1, const CQuatT<T>& q1, float t)
{
	float s,v;
	float omega = rot.Angle* 0.5f;
	float nrevs = 0.0f;
	CQuatT<T>	ret,qaxis,pp,qq;

	// just one rev?
	//==============
	if (omega<Pi-QuatEpsilon)
	{
		ret = CQuatT<T>::squad(q0,tgtQ0,tgtQ1,q1,t);
		return ret;
	}


	// multirev.
	//==============

	// rotation of 180deg around rot.Axis.  (=> sin(a/2)==sin(Pi/2)==1, and c(a/2)=0).
	qaxis.set(rot.Axis.x, rot.Axis.y, rot.Axis.z, 0);

	// the number of revisions (float!)
	nrevs= (float)(omega/Pi);
	// Angle>2Pi. squad from 0 to Pi, slerp from Pi to Angle-Pi, squad from Angle-Pi to Angle.
	s = t*2*nrevs;


	// So for s, squad from 0 to 1, slerp from 1 to 2*nrevs-1, squad from 2*nrevs-1 to 2*nrevs.
	if (s < 1.0f)
	{
		// first part.
		pp = q0*qaxis;
		ret = CQuatT<T>::squad(q0,tgtQ0,pp,pp,s);
	}
	else
	{
		v = s - (2.0f*nrevs - 1.0f);
		if( v <= 0.0f)
		{
			// middle part
			while (s >= 2.0f) s -= 2.0f;
			pp = q0*qaxis;
			// s vary from 1 to 2. This is still correct for slerp().
			ret = CQuatT<T>::slerp(q0,pp,s);
		}
		else
		{
			// Last part.
			qq = - q1*qaxis;
			ret= CQuatT<T>::squad(qq,qq,tgtQ1,q1,v);
		}
	}

	return ret;
}



// ***************************************************************************
template <class T>
CQuatT<T>	CQuatT<T>::log()
{
	double	len;
	len = sqrt (x*x + y*y + z*z);

	if (len < QuatEpsilon)
		return CQuatT<T>(0.f, 0.f, 0.f, 0.f);
	else
	{
		double div = (float) acos (w) / len;
		return CQuatT<T>( (T)(x*div), (T)(y*div), (T)(z*div), 0.f);
	}

}


// ***************************************************************************
template <class T>
CQuatT<T>	CQuatT<T>::exp()
{
	double	len;
	len = sqrt (x*x + y*y + z*z);

	if (len < QuatEpsilon)
		return CQuatT<T>(0.f, 0.f, 0.f, 1.f);
	else
	{
		double len1 = sin(len) / len;
		return CQuatT<T>( (T)(x*len1), (T)(y*len1), (T)(z*len1), (T)cos(len));
	}
}


// ***************************************************************************
template <class T>
CQuatT<T>	CQuatT<T>::lnDif(const CQuatT<T> &q0, const CQuatT<T> &q1)
{
	CQuatT<T>	dif = q0.inverted()*q1;
	dif.normalize();

	return dif.log();
}


// ***************************************************************************
template <class T>
void	CQuatT<T>::makeClosest(const CQuatT<T> &o)
{
	if( dotProduct(*this, o) < 0 )
		*this= -(*this);
}



// ***************************************************************************
// ***************************************************************************
// CQuat/CQuatD
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
/**
 * A float quaternion.
 * \author Antoine Viau.
 * \author Nevrax France
 * \date 2000
 */
class	CQuat : public CQuatT<float>
{
public:
	static const CQuat		Identity;

	/// \name Object
	// @{
	CQuat	&operator=(const CQuatT<float> &o) {x=o.x; y=o.y; z=o.z; w=o.w; return *this;}
	CQuat(const CQuatT<float> &o) : CQuatT<float>(o) {}
	CQuat() {}
	CQuat(float X, float Y, float Z, float W) : CQuatT<float>(X,Y,Z,W) {}
	/// ctor of a UNIT quaternion, from an angle axis.
	CQuat(const CVector &axis, float angle) : CQuatT<float>(axis, angle) {}
	/// ctor of a UNIT quaternion, from an angle axis.
	CQuat(const CAngleAxis &aa) : CQuatT<float>(aa) {}
	// @}

};


// ***************************************************************************
/**
 * A double quaternion.
 * \author Antoine Viau.
 * \author Nevrax France
 * \date 2000
 */
class	CQuatD : public CQuatT<double>
{
public:
	static const CQuatD		Identity;

	/// \name Object
	// @{
	CQuatD	&operator=(const CQuatT<double> &o) {x=o.x; y=o.y; z=o.z; w=o.w; return *this;}
	CQuatD(const CQuatT<double> &o) : CQuatT<double>(o) {}
	CQuatD() {}
	CQuatD(double X, double Y, double Z, double W) : CQuatT<double>(X,Y,Z,W) {}
	/// ctor of a UNIT quaternion, from an angle axis.
	CQuatD(const CVector &axis, float angle) : CQuatT<double>(axis, angle) {}
	/// ctor of a UNIT quaternion, from an angle axis.
	CQuatD(const CAngleAxis &aa) : CQuatT<double>(aa) {}
	// @}


	/// \name CQuat conversion.
	// @{
	CQuatD(const CQuat &o) {x=o.x; y=o.y; z=o.z; w=o.w;}
	CQuatD	&operator=(const CQuatT<float> &o) {x=o.x; y=o.y; z=o.z; w=o.w; return *this;}
	operator	CQuat() const {return CQuat((float)x, (float)y, (float)z, (float)w);}
	// @}

};





} // NLMISC

#endif // NL_QUAT_H

