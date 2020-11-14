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



#ifndef RY_SABRINA_AREA_DEBUG_H
#define RY_SABRINA_AREA_DEBUG_H



#include "egs_mirror.h"
#include "area_geometry.h"
#include "game_share/bmp4image.h"



class CEntityBase;
class CSabrinaAreaDebug;
extern CSabrinaAreaDebug AreaDebug;

/**
 * debug system for sabrina effect
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CSabrinaAreaDebug
{
public:

	inline bool doIDump(){return Dump;}

	void init(const TDataSetRow & targetRow, float radius = 100.0f, uint pixelPerMeter = 5 );

	void dumpBomb( CEntityBase * caster,const  std::vector<CEntityBase*> &entities, float radius );
	void dumpCone( CEntityBase * caster, const std::vector<CEntityBase*> & entities, const CAreaQuad<sint32> & cone );
	void dumpChain( CEntityBase * caster, const std::vector<CEntityBase*> & entities);
	

private:

	inline sint getXPix( sint32 x );
	inline sint getYPix( sint32 y );

	inline void drawEntity( CEntityBase * entity, uint color );


	bool setPoint(sint x , sint y, uint color)
	{
		if ( x >= 0 && y >= 0 && x < (sint)Lines[0].size() && y < (sint)Lines.size() )
		{
			Lines[y][x] = color;
			return true;
		}
		return false;
	}

	void drawLine( sint startX, sint startY, sint endX, sint endY, uint color )
	{
		if (startX > endX )
		{
			sint buf = endX;
			endX = startX;
			startX = buf;
			
			buf = endY;
			endY = startY;
			startY = buf;
		}
		float startXf = (float)startX;
		float startYf = (float)startY;
		float endXf = (float)endX;
		float endYf = (float)endY;

		// horizontal case
		if ( endX == startX )
		{
			for ( sint i = startY; i <= endY ; i++ )
			{
				setPoint(startX,i,color);
			}
		}
		else
		{
			float a = (  startYf - endYf ) / (startXf - endXf);
			float b =  endYf - a * endXf;

			
			if ( a > 1.0f || a < -1.0f )
			{
				if (startY > endY )
				{
					sint buf = endX;
					endX = startX;
					startX = buf;
					
					buf = endY;
					endY = startY;
					startY = buf;
				}
				b = - b/a;
				a = 1 / a;
				
				sint h = endY - startY;
				for ( sint i = startY; i <= endY ; i++ )
				{
					sint x = sint( a * i + b );
					setPoint(x,i,color);
				}
			}
			else
			{
				for ( sint i = startX; i <= endX ; i++ )
				{
					sint y = sint( a * i + b );
					setPoint(i,y,color);
				}
			}
		}
	}
	void drawCircle( uint x, uint y, uint radius, uint color )
	{
		uint rsq = radius * radius;

		sint size = (sint)( double(radius) * sqrt(2.0)/2.0 );
		for ( sint col = 0 ; col <= size ; col++ )
		{
			sint row = (sint) sqrt( double(rsq) - double(col) * double(col));
			//  0 < theta 45
			setPoint(col + x,row + y,color);
			// sym around y = x
			setPoint(row + y,col + x,color);

			// sym around y = 0
			setPoint(col + x,-row + y,color);
			setPoint(row + y,-col + x,color);

			// sym around x = 0
			setPoint(-col + x,row + y,color);
			setPoint(-row + y,col + x,color);
			setPoint(-col + x,-row + y,color);
			setPoint(-row + y,-col + x,color);
		}
		setPoint(-(sint)radius + x, y ,color);
		setPoint( radius -1 + x, y ,color);
	}


	void dump(CEntityBase * caster,const  std::vector<CEntityBase*> &entities);	


	uint PixelPerMeter;
	sint Center;
	float ViewRadius;
	CEntityBase* Target;
	
	CTGAImage Img;
	std::vector< std::vector<uint16> > Lines;

	bool Dump;
	

	static uint16 FileId;


};



#endif // RY_SABRINA_AREA_DEBUG_H

/* End of sabrina_area_debug.h */
