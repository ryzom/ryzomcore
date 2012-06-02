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



#include "stdpch.h"

//--------------------------------------------------------------------------
// the singleton instance
CAIWorldMap CAIWorldMap::_this;

// the no-go surface neighbour
CAIMapSurfaceNeighbour CAIMapSurfaceNeighbour::NoGo(true);


//--------------------------------------------------------------------------
// ctor - this is a singleton so prohibit public instantiation
CAIWorldMap::CAIWorldMap() 
{
	memset(_map,0,sizeof(_map));
}


//--------------------------------------------------------------------------
/*
	The following routine uses a classic line draw algorithm to approximate the 
	Movement path required to perform requested move. 
	A better implementation would cater for overlap between entity collision radius
	and neighbouring surfaces
*/
bool CAIWorldMap::tryMoveStatic(CAIWorldMap::SMove &move)
//bool CAIWorldMap::tryMoveStatic(CAIEntityPhysical *entity,sint dx,sint dy,double &result)
{
#if 0
	// ignore millimeter part of dx & dy and calculate magnitude of moves
	sint xdist=abs(dx)>>10;
	sint ydist=abs(dy)>>10;
	dx>>=10;
	dy>>=10;

	// setup x axis and y axis movement directions
	sint xstep=(dx>>31)*2+1;	// ie +/- 1
	sint ystep=(dy>>31)*2+1;	// ie +/- 1

	// setup starting point for movement
	CAIMapSurface *surface=entity->surface();
	sint x0=entity->x();
	sint y0=entity->y();

	sint x,y;

	if (xdist>ydist)
	{
		// movement with dominant dx
		sint x1=x0+dx;
		sint yerr=xdist/2;
		y=y0;
		for (x=x0;x!=x1;x+=xstep)
		{
			// consider vertical move
			if ((yerr+=ydist)>=xdist)
			{
				yerr-=xdist;
				// move vertical
				if ((surface=surface->getNextSurfaceY(x,y,ystep))==NULL)
				{
					result=double(x-x0)/double(dx);
					return false;
				}
				y+=ystep;
			}
			// move horizontal
			if ((surface=surface->getNextSurfaceX(x,y,xstep))==NULL)
			{
				result=double(x-x0)/double(dx);
				return false;
			}
		}
		result=1.0;
		return true;
	}
	else
	{
		// movement with dominant dy
		sint y1=y0+dy;
		sint xerr=ydist/2;
		x=x0;
		for (y=y0;y!=y1;y+=ystep)
		{
			// consider horizontal move
			if ((xerr+=xdist)>=ydist)
			{
				xerr-=ydist;
				// move horizontal
				if ((surface=surface->getNextSurfaceX(x,y,xstep))==NULL)
				{
					result=double(y-y0)/double(dy);
					return false;
				}
				x+=xstep;
			}
			// move vertical
			if ((surface=surface->getNextSurfaceY(x,y,ystep))==NULL)
			{
				result=double(y-y0)/double(dy);
				return false;
			}
		}
		result=1.0;
		return true;
	}
#endif
	return true;
}

//--------------------------------------------------------------------------
/*
The following routine will check for collisions between a single moving objects
and its neighbours. The neighbours are all considered to be static for the purpose
of this test...
*/
bool CAIWorldMap::tryMoveDynamic(CAIWorldMap::SMove &move)
{
#if 0

	// coordinates here should be in 8.8bit fixed (meters) = 6.25cm precision 
	// the following collision tests should only be performed
	// for close neighbours <100m distance

	// generate a movement vector for our entity
	sint32 a=dy;
	sint32 b=-dy;
	sint32 c=y*dx-x*dy;	//Ax0+By0+C=0 ... C=-Ax0-By0=-x0dy+y0dx
	uint32 mag2=(a*a+b*b);

	// for each possible collision partner
	{
		// if (close to move vector) (using square distances)
		if ( (a*other->x+b*other->y+c)*(a*other->x+b*other->y+c) <= (r+other->r)*(r+other->r)*mag2 )
		{
			// calculate collision time
			//if time < _moveProportion
				// _moveProportion = time
		}
	}
#endif
	return true;
}


//--------------------------------------------------------------------------
// linking cells into the map and unking them

void CAIWorldMap::setMapCell(CAICoord cx, CAICoord cy,CAIMapCell *cell)
{
	uint x=(uint)cx.asInt();
	uint y=(uint)-cy.asInt();
	uint x0=x&((1<<10)-1);	x>>=10;	uint y0=y&((1<<10)-1);	y>>=10;	// millimeter
	uint x1=x&((1<<4)-1);	x>>=4;	uint y1=y&((1<<4)-1);	y>>=4;	// 16x16 meter cells
	uint x2=x&((1<<4)-1);	x>>=4;	uint y2=y&((1<<4)-1);	y>>=4;	// 16x16 cell map segments
	uint x3=x&0xff;					uint y3=y&0xff;					// 256x256 grid of map segments (64km x 64km) 

	// if this is a 'dropMapCell' and the related map segment doesn't exist then nothing to do so return
	if (_this._map[y3][x3]==NULL && cell==NULL)
		return;

	// make sure the map segment for the cell exists (create it if it doesn't)
	if (_this._map[y3][x3]==NULL)
	{
		_this._map[y3][x3]=(TMapSegment *)malloc(sizeof(TMapSegment));
		memset(_this._map[y3][x3],0,sizeof(*_this._map[y3][x3]));
	}

	// the following assert is an assert 'cos it means there's a big bad memory leak!
	nlassert(_this._map[y3][x3][y2][x2]==NULL);
	
	_this._map[y3][x3][y2][x2]=cell;
}

void CAIWorldMap::dropMapCell(CAICoord x, CAICoord y)
{
	setMapCell(x,y,NULL);
}


//--------------------------------------------------------------------------
// The CAIMapCell ctor & dtor
//--------------------------------------------------------------------------

CAIMapCell::CAIMapCell(CAICoord x0,CAICoord y0): _x0(x0), _y0(y0) 
{
	CAIWorldMap::setMapCell(_x0,_y0,this);
}

CAIMapCell::~CAIMapCell() 
{
	CAIWorldMap::dropMapCell(_x0,_y0);
}

	
//--------------------------------------------------------------------------

NLMISC_COMMAND(dummyWorldMap,"","")
{
	NL_ALLOC_CONTEXT(AIDWM);
	if(args.size()!=1)
		return false;

	for (CAICoord j=4096.0;j>(4096.0+512.0);j+=16.0)
		for (CAICoord i=4096.0;i<(4096.0+512.0);i+=16.0)
		{
			new CAIMapCellSimple(i,j);
		}

	return true;
}
