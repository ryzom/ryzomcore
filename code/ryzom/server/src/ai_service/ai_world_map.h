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



#ifndef RYAI_WORLD_MAP_H
#define RYAI_WORLD_MAP_H

#include "nel/misc/types_nl.h"
#include "ai_share/ai_entity_physical_list_link.h"
#include "ai_route.h"

/*
	This class implements the data set and related methods for a continent.
	Continent data includes collision maps and so on.
*/

//--------------------------------------------------------------------------
// Advanced class refferences
//--------------------------------------------------------------------------

class CAIWorldMap;
class CAIMapSurface;
class CAIMapCell;


//--------------------------------------------------------------------------
// The map surface classes
//--------------------------------------------------------------------------

class CAIMapSurfaceNeighbour
{
public:
	inline CAIMapSurfaceNeighbour();
	inline CAIMapSurfaceNeighbour(bool isNoGo);	// constructor for NoGo surface
	inline CAIMapSurface *surface() { return _surface; }

private:
	// identification of the surface in question
	NLMISC::CDbgPtr<CAIMapSurface>	_surface;
	
	// info for high level of pathfinding algorithms
	uint16 _distance;	// distance value for pathfinding algorithms
	bool _sameRegion;	// true if routes lead through cells of same region

	// map of orientations to take to get to the surface
	unsigned char _direction[16*16/2];	// /2 'cos 4bits/ pixel

	static CAIMapSurfaceNeighbour NoGo; // a reserved neighbour that is pointed at for nogo areas
};

class CAIMapSurface
{
public:
	inline CAIMapSurface();
	inline CAIMapSurface(bool isNoGo);	// constructor for NoGo surface

//	inline void linkEntity(CEntityListLink<CAIEntityPhysical> &l);
//	inline CAIEntityPhysical *getFirstEntity();
	
	inline const CAIMapCell *getCell();

	inline CAIMapSurface *getNextSurfaceX(CAICoord x,CAICoord y,CAICoord xstep);
	inline CAIMapSurface *getNextSurfaceY(CAICoord x,CAICoord y,CAICoord ystep);

private:
	std::vector<NLMISC::CDbgPtr<CAIMapSurfaceNeighbour> > _neighbours;	// entry 0 points to self

	uint _regionId;	// the region that this surface is contained in
	uint _h;		// the value to send to the client to represent height
	CEntityListLink<CAIEntityPhysical> _entityList;	// list of entities in surface
	NLMISC::CDbgPtr<CAIMapCell> _cell;	// the 16x16 cell that the surface belongs to

	// for neighbourhood tables:
	// hi nibble +ve boundary, lo nibble -ve boundary 
	// value = 0..f (index into _neighbours vector)
	uint8 _xNeighbourhood[16][16];
	uint8 _yNeighbourhood[16][16];
};


//--------------------------------------------------------------------------
// The map cell classes
//--------------------------------------------------------------------------

// the general interface for map cell implementations
class CAIMapCell
{
public:
	CAIMapCell(const CAICoord &x0,const CAICoord &y0);
	virtual ~CAIMapCell();

	virtual CAIMapSurface *getSurface(const CAICoord &x,const CAICoord &y,uint h) const =0;
	virtual CAIMapSurface *getNextSurface(const CAICoord &x,const CAICoord &y,uint h,const	CAngle &direction) const =0;

protected:
	const CAICoord _x0, _y0;
};

// a simple map cell - all one surface
class CAIMapCellSimple: public CAIMapCell
{
public:
	CAIMapCellSimple(const CAICoord &x0,const CAICoord &y0): CAIMapCell(x0,y0) 
	{
	}
	virtual CAIMapSurface *getSurface(const CAICoord &x,const CAICoord &y,uint h)	const
	{
		return const_cast<CAIMapSurface *>(&_surface);
	}
	virtual CAIMapSurface *getNextSurface(const CAICoord &x,const CAICoord &y,uint h,CAngle direction) const
	{
		nlwarning("*** getNextSurface() NOT IMPLEMENTED YET ***");
		return NULL;
	}

private:
	CAIMapSurface _surface;
	bool noGo[16][16];
};


//--------------------------------------------------------------------------
// The CAIWorldMap singleton class
//--------------------------------------------------------------------------

class CAIWorldMap
{
public:
	//------------------------------------------------------------------
	// collision evaluation for entity movement
	struct SMove
	{
		SMove(const CAIPos &startPos,const CAIVector &move,uint32 radius)
		{
			_startPos=startPos;
			_move=move;
			_radius=radius;
			_moveProportion=1.0f;
		}
		CAIVector	_move;
		CAIPos		_startPos;
		uint32		_radius;
		float		_moveProportion;	// = 1.0 if move not blocked
	};
	static bool tryMoveStatic(SMove &move);	// returns true if move not hindered
	static bool tryMoveDynamic(SMove &move);	// returns true if move not hindered

	//------------------------------------------------------------------
	// looking up a cell or surface by coordinates
	inline static CAIMapCell *getMapCell(CAICoord x,CAICoord y);
	inline static CAIMapSurface *getMapSurface(CAICoord x, CAICoord y, uint h);

	//------------------------------------------------------------------
	// linking cells into the map and unlinking them
	static void setMapCell(CAICoord x, CAICoord y,CAIMapCell *cell);
	static void dropMapCell(CAICoord x, CAICoord y);

private:
	//------------------------------------------------------------------
	// ctor - this is a singleton so prohibit public instantiation
	CAIWorldMap(); 

	//------------------------------------------------------------------
	// data structures
	// 256x256 array of 16x16 arrays of MapCell(16mx16m) = 64km x 64km
	typedef CAIMapCell *TMapSegment[16*16];
	TMapSegment *_map[256][256];

	//------------------------------------------------------------------
	// the singleton instance
	static CAIWorldMap _this;
};


//--------------------------------------------------------------------------
// The CAIWorldMap inlines
//--------------------------------------------------------------------------

inline CAIMapCell *CAIWorldMap::getMapCell(CAICoord cx,CAICoord cy)
{
	uint x=(uint)cx.asInt();
	uint y=(uint)-cy.asInt();
	uint x0=x&((1<<10)-1);	x>>=10;	uint y0=y&((1<<10)-1);	y>>=10;	// millimeter
	uint x1=x&((1<<4)-1);	x>>=4;	uint y1=y&((1<<4)-1);	y>>=4;	// 16x16 meter cells
	uint x2=x&((1<<4)-1);	x>>=4;	uint y2=y&((1<<4)-1);	y>>=4;	// 16x16 cell map segments
	uint x3=x&0xff;					uint y3=y&0xff;					// 256x256 grid of map segments (64km x 64km) 

	if (_this._map[y3][x3]==NULL)
		return NULL;
	return _this._map[y3][x3][y2][x2];
}

inline CAIMapSurface *CAIWorldMap::getMapSurface(CAICoord cx, CAICoord cy, uint h)
{
	uint x=(uint)cx.asInt();
	uint y=(uint)-cy.asInt();
	uint x0=x&((1<<10)-1);	x>>=10;	uint y0=y&((1<<10)-1);	y>>=10;	// millimeter
	uint x1=x&((1<<4)-1);	x>>=4;	uint y1=y&((1<<4)-1);	y>>=4;	// 16x16 meter cells
	uint x2=x&((1<<4)-1);	x>>=4;	uint y2=y&((1<<4)-1);	y>>=4;	// 16x16 cell map segments
	uint x3=x&0xff;					uint y3=y&0xff;					// 256x256 grid of map segments (64km x 64km) 

	if (_this._map[y3][x3]==NULL || _this._map[y3][x3][y2][x2]==NULL)
		return NULL;
	return _this._map[y3][x3][y2][x2]->getSurface(cx,cy,h);
}


//--------------------------------------------------------------------------
// The CAIWorldMapNeighbour inlines
//--------------------------------------------------------------------------

inline CAIMapSurfaceNeighbour::CAIMapSurfaceNeighbour(bool isNoGo)	// constructor for NoGo surface
{
	nlassert(isNoGo=true);	// by definition this code can't be executed with isNoGo false

	_surface=NULL;
	_distance=(uint16)-1;
	//bool _sameRegion=false;
	memset(_direction,0,sizeof(_direction));
}


//--------------------------------------------------------------------------
// The CAIMapSurface inlines
//--------------------------------------------------------------------------

inline CAIMapSurface::CAIMapSurface(): _entityList(NULL)
{
	_regionId=~0;
	_cell=NULL;
	_h=0;
}

//inline void CAIMapSurface::linkEntity(CEntityListLink<CAIEntityPhysical> &l)
//{
//	_entityList.link(l);
//}
//
//inline CAIEntityPhysical *CAIMapSurface::getFirstEntity()
//{
//	return _entityList.next()->entity();
//}

inline const CAIMapCell *CAIMapSurface::getCell()
{
	return _cell;
}

inline CAIMapSurface *CAIMapSurface::getNextSurfaceX(CAICoord x,CAICoord y,CAICoord xstep)
{
	// convert xy to table offset
	uint xidx=(x.asInt()>>10)&0xf;
	uint yidx=(y.asInt()>>10)&0xf;

	// lookup the neighbour code
	uint8 neighbourcode=_xNeighbourhood[xidx][yidx];

	// extract the desired nibble of the neighbour code
	if (xstep>0)
		neighbourcode>>=4;
	else
		neighbourcode&=0xf;

	// return the surface id of the given neighbour
	return _neighbours[neighbourcode]->surface();
}

inline CAIMapSurface *CAIMapSurface::getNextSurfaceY(CAICoord x,CAICoord y,CAICoord ystep)
{
	// convert xy to table offset
	uint xidx=(x.asInt()>>10)&0xf;
	uint yidx=(y.asInt()>>10)&0xf;

	// lookup the neighbour code
	uint8 neighbourcode=_yNeighbourhood[xidx][yidx];

	// extract the desired nibble of the neighbour code
	if (ystep>0)
		neighbourcode>>=4;
	else
		neighbourcode&=0xf;

	// return the surface id of the given neighbour
	return _neighbours[neighbourcode]->surface();
}


//--------------------------------------------------------------------------

#endif
