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



#ifndef RY_AREA_GEOMETRY_H
#define RY_AREA_GEOMETRY_H



template<class T>
struct CAreaCoords
{
	CAreaCoords(){}
	CAreaCoords(T x, T y)
		:X(x),Y(y){}
	T X;
	T Y;

	
	CAreaCoords<T> &operator*=(float scalar)
	{ 
		X = T ( scalar * X);
		Y = T ( scalar * Y);
		return *this;
	}

};

template<class T>
class CAreaQuad
{
public:
	bool contains( CAreaCoords<T> point )const
	{
		uint32 nNbIntersection = 0;
		for (uint i = 0; i < 4; ++i)
		{
			const CAreaCoords<T> &p1 = Vertices[i];
			const CAreaCoords<T> &p2 = Vertices[(i+1)%4];
			
			if ( p1.Y - point.Y < T(0) && p2.Y-point.Y  < T(0) )
				continue;
			if ( p1.Y-point.Y  > T(0) && p2.Y-point.Y > T(0) )
				continue;
			T xinter = (sint32)( p1.X + (p2.X-p1.X) * ( double(point.Y - p1.Y) / double( p2.Y - p1.Y  ) ) );
			if (xinter > point.X)
				++nNbIntersection;
		}
		if ((nNbIntersection&1) == 1) // odd intersections so the vertex is inside
			return true;
		else
			return false;
	}
	CAreaCoords<T> Vertices [4];
};

template<class T>
class CAreaRect
{
public:
	///from a quad : build the bounding rect
	CAreaRect<T>( const CAreaQuad<T> & quad )
	{
		Point.X = quad.Vertices[0].X;
		Point.Y = quad.Vertices[0].Y;
		CAreaCoords<T> topRight(quad.Vertices[0].X,quad.Vertices[0].Y);

		for ( uint i = 1; i < 3; i++ )
		{
			if ( Point.X > quad.Vertices[i].X )
				Point.X = quad.Vertices[i].X;
			else if ( topRight.X < quad.Vertices[i].X )
				topRight.X = quad.Vertices[i].X;

			if ( Point.Y > quad.Vertices[i].Y )
				Point.Y = quad.Vertices[i].Y;
			else if ( topRight.Y < quad.Vertices[i].Y )
				topRight.Y = quad.Vertices[i].Y;
		}
		Width = topRight.X - Point.X;
		Height = topRight.Y - Point.Y;
	}

	CAreaCoords<T> center()
	{
		return CAreaCoords<T> ( Point.X + Width / 2,  Point.Y + Height / 2 );
	}

	/// bottom left point
	CAreaCoords<T> Point;
	///width
	T Width;
	T Height;
};



#endif // RY_AREA_GEOMETRY_H

/* End of area_geometry.h */
