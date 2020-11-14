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



#ifndef RYAI_COORD_H
#define RYAI_COORD_H

#include "nel/misc/types_nl.h"


//--------------------------------------------------------------------------
// The class
//--------------------------------------------------------------------------

// A coordinate with orientation and surface id

class CAICoord  
{
public:
	//----------------------------------------------------------------------------
	// Constants
	
	enum CAIConstants
	{
		UNITS_PER_METER = 1000
	};

	//----------------------------------------------------------------------------
	// ctors

	explicit	CAICoord(): _coord(0) {}
	explicit	CAICoord(double d): _coord((sint32)(d*UNITS_PER_METER))	{}
	CAICoord(const CAICoord &coord): _coord(coord._coord) {}
	//CAICoord(CAICoord &aicoord):  _coord(aicoord._coord) {}

	//----------------------------------------------------------------------------
	// convertions to other types

//	(StepH) still now, we have no fpu acceleration :( so its quickest to do a simple int div .. waiting DM response.
	inline double asDouble() const		{ return (double)_coord/(double)UNITS_PER_METER; }
	inline sint32 asInt() const			{ return _coord; }
	inline sint32 asIntMeters() const	{ return _coord>>10; }
	inline sint32 asInt16Meters() const	{ return _coord>>14; }

	//----------------------------------------------------------------------------
	// direct operations

  CAICoord	operator-() const
  {
	CAICoord c(-_coord,true);
	return c;
  }

	//----------------------------------------------------------------------------
	// maths with CAICoord

	const CAICoord &operator=(const CAICoord &other)		{ _coord=other._coord; return *this; }
	const CAICoord &operator+=(const CAICoord &other)		{ _coord+=other._coord; return *this; }
	const CAICoord &operator-=(const CAICoord &other)		{ _coord-=other._coord; return *this; }
	const CAICoord &operator*=(const CAICoord &other)		{ _coord*=other._coord; return *this; }		
	const CAICoord &operator/=(const CAICoord &other)		{ _coord/=other._coord; return *this; }
	
	const CAICoord operator+(const CAICoord &other) const	{ CAICoord c(*this); c+=other; return c; }
	const CAICoord operator-(const CAICoord &other) const	{ CAICoord c(*this); c-=other; return c; }
	const CAICoord operator*(const CAICoord &other) const	{ CAICoord c(*this); c*=other; return c; }
	const CAICoord operator/(const CAICoord &other) const	{ CAICoord c(*this); c/=other; return c; }

	//----------------------------------------------------------------------------
	// maths with double

	const CAICoord &operator=(double d)						{ *this=CAICoord(d); return *this; }
	const CAICoord &operator+=(double d)					{ *this+=CAICoord(d); return *this; }
	const CAICoord &operator-=(double d)					{ *this-=CAICoord(d); return *this; }
	const CAICoord &operator*=(double d)					{ *this=CAICoord(asDouble()*d); return *this; }
	const CAICoord &operator/=(double d)					{ *this=CAICoord(asDouble()*(1.0/d)); return *this; }

	const CAICoord operator+(double d) const				{ CAICoord c(*this); c+=d; return c; }
	const CAICoord operator-(double d) const				{ CAICoord c(*this); c-=d; return c; }
	const CAICoord operator*(double d) const				{ CAICoord c(*this); c*=d; return c; }
	const CAICoord operator/(double d) const				{ CAICoord c(*this); c/=d; return c; }

	//----------------------------------------------------------------------------
	// maths with int

	const CAICoord &operator*=(int i)						{ _coord*=i; return *this; }
	const CAICoord &operator/=(int i)						{ _coord/=i; return *this; }

	const CAICoord operator*(int i) const					{ CAICoord c(*this); c*=i; return c; }
	const CAICoord operator/(int i) const					{ CAICoord c(*this); c/=i; return c; }


	//----------------------------------------------------------------------------
	// compareson with CAICoord

	bool operator==(const CAICoord &other) const			{ return _coord == other._coord; }
	bool operator!=(const CAICoord &other) const			{ return _coord != other._coord; }
	bool operator>=(const CAICoord &other) const			{ return _coord >= other._coord; }
	bool operator<=(const CAICoord &other) const			{ return _coord <= other._coord; }
	bool operator> (const CAICoord &other) const			{ return _coord >  other._coord; }
	bool operator< (const CAICoord &other) const			{ return _coord <  other._coord; }

	//----------------------------------------------------------------------------
	// compareson with double

	bool operator==(double d) const { return _coord == CAICoord(d)._coord; }
	bool operator!=(double d) const { return _coord != CAICoord(d)._coord; }
	bool operator>=(double d) const { return _coord >= CAICoord(d)._coord; }
	bool operator<=(double d) const { return _coord <= CAICoord(d)._coord; }
	bool operator> (double d) const { return _coord >  CAICoord(d)._coord; }
	bool operator< (double d) const { return _coord <  CAICoord(d)._coord; }

	//----------------------------------------------------------------------------
	// compareson with int

	bool operator==(int d) const { return _coord == CAICoord(d)._coord; }
	bool operator!=(int d) const { return _coord != CAICoord(d)._coord; }
	bool operator>=(int d) const { return _coord >= CAICoord(d)._coord; }
	bool operator<=(int d) const { return _coord <= CAICoord(d)._coord; }
	bool operator> (int d) const { return _coord >  CAICoord(d)._coord; }
	bool operator< (int d) const { return _coord <  CAICoord(d)._coord; }

	//----------------------------------------------------------------------------
	// serial.

	void serial(NLMISC::IStream &i)				
	{
		i.serial(_coord);
	}

	//----------------------------------------------------------------------------
	// toString

	inline std::string toString() const 
	{
		return NLMISC::toString("%.2f",asDouble());
	}

	operator sint32() const
	{
		return _coord;
	}

private:
  CAICoord(int coord, bool priv):_coord(coord) {}
	/*double _coord;*/
	sint32 _coord;		// the coordinate is now mapped directly onto the mirror value
};

#endif

