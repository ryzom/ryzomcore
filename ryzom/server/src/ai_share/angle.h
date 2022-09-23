// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef RY_ANGLE_H
#define RY_ANGLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2d.h"

#include <string>

class CAngle
{
public:
	// enum for pseudo contante
	enum
	{
		// the pi value
		PI = 0x8000
	};

	CAngle(): _data(0) {}

	inline	CAngle(const	CAngle &other)	:	_data(other._data)	{}
	inline	CAngle(const	sint16 &val)	:	_data(val)	{}
	inline	CAngle(const	sint32 &val)	:	_data((sint16)val)	{}
	inline	CAngle(const	uint16 &val)	:	_data((sint16)val)	{}
	inline	CAngle(const	uint32 &val)	:	_data((sint16)val)	{}          
	inline	CAngle(const	double &val)
	{
		// This code correct a nasty bug :
		//	GCC and MSVC don't handle the cast from a float
		//	into a 16 bit signed interger when the 
		//	float value overflow inside the integer.
		//	It seeems that GCC bound the value to 0x8000 (-32768) and 0x7fff (32767)
		//	but MSVC just truncate the exedent bits (as if it perform a float
		//	to 32 bit interger conversion first).
		// NB : Fundamentaly, the problem is that it is bad to cast something with an overflow... ;)
		sint32 a = sint32(val*(PI/NLMISC::Pi));
		while (a >= PI)
			a-=2*PI;
		while (a < -PI)
			a+=2*PI;

		_data = sint16(a);
	}

#ifdef NL_COMP_VC6
	inline	CAngle(const	sint &val)	:	_data((sint16)val)	{}
	inline	CAngle(const	uint &val)	:	_data((sint16)val)	{}
#endif

	template <class C> bool operator>(const C &other) const		{ return _data>CAngle(other)._data; }
	template <class C> bool operator<(const C &other) const		{ return _data<CAngle(other)._data; }
	template <class C> bool operator>=(const C &other) const	{ return _data>=CAngle(other)._data; }
	template <class C> bool operator<=(const C &other) const	{ return _data<=CAngle(other)._data; }
	template <class C> bool operator==(const C &other) const	{ return _data==CAngle(other)._data; }
	template <class C> bool operator!=(const C &other) const	{ return _data!=CAngle(other)._data; }

	template <class C> CAngle &operator=(const C &val)			{ _data=CAngle(val)._data; return *this; }
	template <class C> CAngle &operator+=(const C &val)			{ _data+=CAngle(val)._data; return *this; }
	template <class C> CAngle &operator-=(const C &val)			{ _data-=CAngle(val)._data; return *this; }
	template <class C> CAngle &operator*=(const C &val)			{ _data*=val; return *this; }
	template <class C> CAngle &operator/=(const C &val)			{ _data/=val; return *this; }

	template <class C> CAngle operator+(const C &val)			{ return CAngle(_data+CAngle(val)._data); }
	template <class C> CAngle operator-(const C &val)			{ return CAngle(_data-CAngle(val)._data); }
	template <class C> CAngle operator*(const C &val)			{ return CAngle(sint16(_data*val)); }

	CAngle operator/(const double &val)	const					{ return CAngle((sint16)(_data/val)); }
	float  operator/(const CAngle &val)	const					{ return (float)_data/(float)val._data; }
													
	CAngle &operator-()											{ _data=-_data; return *this; }

	float  asRadians() const									{ return (float)((float)_data*(NLMISC::Pi/PI)); }
	sint16 asRawSint16() const									{ return _data; }
	int    asDegrees() const									{ return (360*(uint16)_data+PI)/(2*PI); }

	const char* asCompassHeading()								{ return "NOT IMPLEMENTED YET"; }
	NLMISC::CVector2d asVector2d() const						{ float	radians=asRadians();return NLMISC::CVector2d(cos(radians), sin(radians)); }

	inline static const CAngle &pi()							{ static CAngle p(PI); return p; } 

private:
	sint16 _data;
};


//-----------------------------------------------
// computeShortestAngle :
// Compute the angle between a source and a destination using the shortest way.
// \param from : angle between -Pi and Pi;
// \param to : angle between -Pi and Pi;
//-----------------------------------------------
inline	double computeShortestAngle(double from, double to)
{
	double difAngle = to - from;
	if	(difAngle<-NLMISC::Pi)		// Angle in the wrong direction.
		return	difAngle+2.0*NLMISC::Pi;
	else
	{
		if	(difAngle>NLMISC::Pi)	// Angle in the wrong direction.
			return	difAngle-2.0*NLMISC::Pi;
		else
			return difAngle;
	}	
}// computeShortestAngle //			  

#endif

