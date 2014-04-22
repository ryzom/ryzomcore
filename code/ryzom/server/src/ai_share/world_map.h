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



#ifndef NL_WORLD_MAP_H
#define NL_WORLD_MAP_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/stream.h"

#include "nel/pacs/u_global_position.h"
#include "nel/misc/vectord.h"

#include "16x16_layer.h"

#include "ai_coord.h"
#include "ai_vector.h"
#include "ai_types.h"

class CFollowPath;

namespace RYPACSCRUNCH
{

class CPacsCruncher;

}

namespace RYAI_MAP_CRUNCH
{

enum TAStarFlag
{
	Nothing			= 0,
	Interior		= 1,
	Water			= 2,
	NoGo			= 4,
	WaterAndNogo	= 6,
	GroundFlags		= WaterAndNogo
};

const std::string& toString(TAStarFlag flag);
TAStarFlag toAStarFlag(const std::string& str);

uint const WorldMapGridSize = 256;
double const WorldGridResolution = 1.;
NLMISC::CVectorD const WorldStartOffset = NLMISC::CVectorD(0., 0., 0.);

typedef sint TLevel;


//////////////////////////////////////////////////////////////////////////////

/**
 * Slot for the 3 map layer (corresponding to 3 heights of deplacement)
 * \author Stephane le Dorze
 * \author Nevrax France
 * \date 2003
 */
class CSlot
{
public:
	enum
	{
		INVALID_SLOT = 3
	};
	
public:
	explicit CSlot();
	explicit CSlot(uint slot);
	
	bool isValid() const { return _Slot!=INVALID_SLOT; }
	void setSlot(CSlot const& slot) { _Slot = slot._Slot; }
	
	bool operator ==(CSlot const& slot) const { return _Slot==slot._Slot; }
	bool operator !=(CSlot const& slot) const { return _Slot!=slot._Slot; }
	
	bool operator >(CSlot const& slot) const { return _Slot>slot._Slot; }
	bool operator <(CSlot const& slot) const { return _Slot<slot._Slot; }
	
	bool operator >=(CSlot const& slot) const { return _Slot>=slot._Slot; }
	bool operator <=(CSlot const& slot) const { return _Slot<=slot._Slot; }
	
	CSlot const& operator ++();
	
	uint8 slot() const { return _Slot; }

private:
	uint8 _Slot;
};

inline
CSlot::CSlot()
: _Slot(INVALID_SLOT)
{
}

inline
CSlot::CSlot(uint slot)
: _Slot(slot)
{
#ifdef NL_DEBUG
	nlassert(slot>=0 && slot<=3);
#endif
}

inline
CSlot const& CSlot::operator ++()
{
	++_Slot;
#ifdef NL_DEBUG
	nlassert(_Slot<=INVALID_SLOT);
#endif
	return *this;
}

//////////////////////////////////////////////////////////////////////////////

/**
 * Compressed link set for the 4 cardinal directions
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CCellLinkage
// pass CCellLinkage parameters by references. (anyway) and remove validation checks. (big update to come).
{
public:
	enum
	{
		SouthSlotOffset = 0,
		SouthSlotMask = 0x03,
		EastSlotOffset = 2,
		EastSlotMask = 0x0c,
		NorthSlotOffset = 4,
		NorthSlotMask = 0x30,
		WestSlotOffset = 6,
		WestSlotMask = 0xc0,
	};
	
public:
	CCellLinkage(uint8 links = 0xff);
	
	CSlot SSlot() const { return CSlot((_Links & SouthSlotMask)  >> SouthSlotOffset); }
	CSlot ESlot() const { return CSlot((_Links & EastSlotMask)   >> EastSlotOffset); }
	CSlot NSlot() const { return CSlot((_Links & NorthSlotMask)  >> NorthSlotOffset); }
	CSlot WSlot() const { return CSlot((_Links & WestSlotMask)   >> WestSlotOffset); }
	
	bool isSSlotValid() const { return (_Links&SouthSlotMask)!=SouthSlotMask; }
	bool isESlotValid() const { return (_Links&EastSlotMask)!=EastSlotMask; }
	bool isNSlotValid() const { return (_Links&NorthSlotMask)!=NorthSlotMask; }
	bool isWSlotValid() const { return (_Links&WestSlotMask)!=WestSlotMask; }
	
	void setSSlot(CSlot const& slot);
	void setESlot(CSlot const& slot);
	void setNSlot(CSlot const& slot);
	void setWSlot(CSlot const& slot);
	
	CCellLinkage& operator |=(uint8 links);
	
	bool used() const { return _Links!=0xff; }
	
	uint8 getLinks() const { return _Links; }
	
	void serial(NLMISC::IStream &f);
	
private:
	uint8 _Links;
};

inline
CCellLinkage::CCellLinkage(uint8 links)
: _Links(links)
{
}

inline
void CCellLinkage::setSSlot(CSlot const& slot)
{
	_Links = (_Links & (~SouthSlotMask)) + (slot.slot() << SouthSlotOffset);
}

inline
void CCellLinkage::setESlot(CSlot const& slot)
{
	_Links = (_Links & (~EastSlotMask)) + (slot.slot() << EastSlotOffset);
}

inline
void CCellLinkage::setNSlot(CSlot const& slot)
{
	_Links = (_Links & (~NorthSlotMask)) + (slot.slot() << NorthSlotOffset);
}

inline
void CCellLinkage::setWSlot(CSlot const& slot)
{
	_Links = (_Links & (~WestSlotMask)) + (slot.slot() << WestSlotOffset);
}

inline
CCellLinkage& CCellLinkage::operator |=(uint8 links)
{
	_Links |= links;
	return *this;
}

inline
void CCellLinkage::serial(NLMISC::IStream &f)
{
	f.serial(_Links);
}

//////////////////////////////////////////////////////////////////////////////

class CDirection
{
	friend class CWorldMap;
public:
	enum TDeltaDirection
	{
		HALF_TURN_LEFT	= 1,	//	45 degres
		HALF_TURN_RIGHT	= -1,	//	-45 degres
		TURN_LEFT		= 2,	//	90 degres
		TURN_RIGHT		= -2,	//	-90 degres
		HALF_TURN		= 4		//	180 degres
	};
	
	enum TDirection
	{
		E	= 0,
		NE	= 1,
		N	= 2,
		NW	= 3,
		W	= 4,
		SW	= 5,
		S	= 6,
		SE	= 7,
		UNDEFINED = 8
	};
	
	enum TMotifDirection
	{
		SHIFT_E  = 1,
		SHIFT_NE = 2,
		SHIFT_N  = 4,
		SHIFT_NW = 8,
		SHIFT_W  = 16,
		SHIFT_SW = 32,
		SHIFT_S  = 64,
		SHIFT_SE = 128,
		SHIFT_UNDEFINED = 256 // be aware, if you use a byte, it makes 0.
	};
	
	enum TCost
	{
		NO_COST		= 0,
		ORTHO_COST  = 2,
		DIAG_COST   = 3,
		MAX_COST	= 3
	};
	
public:
	explicit CDirection();
	CDirection(CDirection const& dir);
	explicit CDirection(TDirection dir);
	
	/// @name user interface assuming the y inversion (sorry).
	//@{
	explicit CDirection(int deltax, int deltay, bool toCalculate);
	
	//	initialisation with a direction depending on deltas (-1,0 or 1)
	explicit CDirection(int deltax, int deltay);
	
	//	initialisation with a direction depending on deltas (-1,0 or 1)
	explicit CDirection(CAngle const& angle);
	
	CAngle getAngle();
	
	sint dx() const;
	sint dy() const;
	void dxdy(int& dx, int& dy);
	//@}
	
	bool isValid() { return _value!=UNDEFINED; }
	
	// add a number of 45 turn
	void addStep(TDeltaDirection step);
	
	NLMISC::CVector2f getDirFloat() const;
	
	TDirection getVal() const;
	
	TMotifDirection getShift() const;
	
	bool operator !=(TDirection dir) const;
	
	bool operator != (CDirection const& dir) const;
	
	struct CDirectionData
	{
		sint dx;
		sint dy;
		TCost weight;
	};
	static CDirectionData const directionDatas[];
	static TDirection const table[];
	
	// used in pacs (nothing to do here but i haven't enought time to rewrite much code)
	uint32 getWeight();
	
	static void getDirectionAround(CAngle const& Angle, CDirection& Dir0, CDirection& Dir1);
	
private:
	void setDxDy(int deltax, int deltay);
	
	TDirection _value;
};

inline
CDirection::CDirection()
: _value(UNDEFINED)
{
}

inline
CDirection::CDirection(CDirection const& dir)
: _value(dir._value)
{
}

inline
CDirection::CDirection(TDirection dir)
: _value(dir)
{
#ifdef NL_DEBUG
	nlassert(dir>=E	&&	dir<=UNDEFINED);
#endif
}

inline
CDirection::CDirection(int deltax, int deltay, bool toCalculate)
{
#ifdef NL_DEBUG
	nlassert(deltax!=0 || deltay!=0);
#endif
	uint absDeltax = abs(deltax);
	uint absDeltay = abs(deltay);
	
	if (deltax!=0 && deltay!=0)
	{
		float ratio = (float)((float)absDeltax/(float)absDeltay);
		if (ratio>=0.4141 && ratio<=2.4145)	// cos(PI/8)/sin(PI/8) ou sin(PI/8)/cos(PI/8)
		{
			setDxDy(deltax>0?1:-1, deltay>0?1:-1);
			return;
		}
	}
	
	if (absDeltax>absDeltay)
		setDxDy(deltax>0?1:-1, 0);
	else
		setDxDy(0, deltay>0?1:-1);
}

inline
CDirection::CDirection(int deltax, int deltay)
{
	setDxDy(deltax, deltay);
}

inline
CDirection::CDirection(CAngle const& angle)
{
	_value = (TDirection)((((sint32)angle.asRawSint16()+65536+4096)/8192)&7);	//	add 4096 to have a correct angle magnet.
}

inline
CAngle CDirection::getAngle()
{
#ifdef NL_DEBUG
	nlassert(isValid());
#endif
	return CAngle(_value*8192);
}

inline
sint CDirection::dx() const
{
	return directionDatas[_value].dx;
}

inline
sint CDirection::dy() const
{
	return directionDatas[_value].dy;
}

inline
void CDirection::dxdy(int& dx, int& dy)
{
	CDirectionData const& direction = directionDatas[_value];
	dx = direction.dx;
	dy = direction.dy;
}

inline
void CDirection::addStep(TDeltaDirection step)
{
#ifdef NL_DEBUG
	nlassert(isValid());
#endif
	_value = (TDirection)((_value+step)&7);
}

inline
NLMISC::CVector2f CDirection::getDirFloat() const
{
	return NLMISC::CVector2f((float)dx(), (float)dy());
}

inline
CDirection::TDirection CDirection::getVal() const
{
	return _value;
}

inline
CDirection::TMotifDirection CDirection::getShift() const
{
	return (TMotifDirection)(1<<_value);
}

inline
bool CDirection::operator !=(TDirection dir) const
{
	return _value!=dir;
}

inline
bool CDirection::operator != (CDirection const& dir) const
{
	return _value!=dir._value;
}

inline
uint32 CDirection::getWeight()
{
	return directionDatas[_value].weight;
}

inline
void CDirection::getDirectionAround(CAngle const& Angle, CDirection& Dir0, CDirection& Dir1)
{
	sint value = Angle.asRawSint16()&(~16383);	//8191);
	Dir0 = CDirection(CAngle(value));
	value += 16384;	//8192;
	Dir1 = CDirection(CAngle(value));
}

inline
void CDirection::setDxDy(int deltax, int deltay)
{
#ifdef NL_DEBUG
	nlassert( deltax>=-1 && deltax<=1 && deltay>=-1 && deltay<=1);
#endif
	_value = table[1+deltax+(1+deltay)*3];
#ifdef NL_DEBUG
	nlassert(dx()==deltax && dy()==deltay);
#endif
}

//////////////////////////////////////////////////////////////////////////////

/**
 * A map of accessible positions near a given position
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CNeighbourhood
{
public:
	/// One bit per direction
	bool isValid(CDirection::TDirection dir) const;
	bool isValid(CDirection const& dir) const;
	void set(CDirection::TDirection dir);
	CNeighbourhood();
	
private:
	CNeighbourhood(uint32 neighb);
	
	/// 8 bits for 8 directions
	uint32	Neighb;
};

inline
bool CNeighbourhood::isValid(CDirection::TDirection dir) const
{
	return (Neighb&CDirection(dir).getShift())!=0;
}

inline
bool CNeighbourhood::isValid(CDirection const& dir) const
{
	return (Neighb&dir.getShift())!=0;
}

inline
void CNeighbourhood::set(CDirection::TDirection dir)
{
	Neighb|=CDirection(dir).getShift();
}

inline
CNeighbourhood::CNeighbourhood()
: Neighb(0)
{
}

inline
CNeighbourhood::CNeighbourhood(uint32 neighb)
: Neighb(neighb)
{
}

//////////////////////////////////////////////////////////////////////////////

/**
 * A slot, defining pacs position and linkage to other slots
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CUnitSlot
{
public:
	// 12 bits  0-11 : instance id
	// 12 bits 12-23 : surface id
	// 2 bits  24-25 : north link
	// 2 bits  26-27 : east link
	// 2 bits  28-29 : south link
	// 2 bits  30-31 : west link
	enum
	{
		InstanceIdOffset = 0,
		InstanceIdMask = 0x00000fff,
		InstanceIdMax = 4096,
		
		SurfaceIdOffset = 12,
		SurfaceIdMask = 0x00fff000,
		SurfaceIdMax = 4096,
		
		SlotOffset = 24,
		SlotMask = 0xff000000
	};
	
	// 6 bits  0-5 topology
	// 2 bits  6-7 gabarit
	enum
	{
		TopologyMask = 0x3f,
		GabaritMask = 0xc0,
		GabaritShift = 6,
	};
	
	// 1 bit   0 interior
	// 1 bit   1 water
	// 1 bit   2 nogo
	// 13 bits 3-15 height
	enum
	{
		InteriorMask = 0x0001,
		WaterMask = 0x0002,
		NoGoMask = 0x0004,
		HeightMask = 0xfff8,
		HeightShift = 3,
	};
	
public:
	/// @name Constructors
	//@{
	explicit CUnitSlot();
	explicit CUnitSlot(uint instanceId, uint surfaceId);
	explicit CUnitSlot(NLPACS::UGlobalPosition const& pos);
	//@}
	
	// Selectors
	uint instanceId() const { return (_Id&InstanceIdMask) >> InstanceIdOffset; }
	uint surfaceId () const { return (_Id&SurfaceIdMask)  >> SurfaceIdOffset; }
	uint height    () const { return _Height >> HeightShift; }
	
	CCellLinkage& cellLink() { return *((CCellLinkage*)(((char*)&_Id)+3)); }
	CCellLinkage getCellLink() const { return *((CCellLinkage*)(((char*)&_Id)+3)); }
	uint topology() const	{ return (_Topology & TopologyMask); }
	uint gabarit() const	{ return (_Topology & GabaritMask) >> GabaritShift; }
	bool interior() const	{ return (_Height & InteriorMask) != 0; }
	bool water() const		{ return (_Height & WaterMask) != 0; }
	bool nogo() const		{ return (_Height & NoGoMask) != 0; }
	
	/// @name Mutators
	//@{
	void setInstanceId(uint instanceId);
	void setSurface(uint surfaceId);
	void setTopology(uint topology);
	void setGabarit(uint gabarit);
	void setHeight(sint height);
	void setInterior(bool interior);
	void setWater(bool water);
	void setNoGo(bool nogo);
	void reset();
	//@}
	
	bool used() const;
	bool hasSameSurface(const CUnitSlot &slot)	const;
	
	void serial(NLMISC::IStream &f);
	
private:
	// _Id is divided this way
	// bits  0-11 : instance id
	// bits 12-23 : surface id
	// bits 24-25 : north link
	// bits 26-27 : east link
	// bits 28-29 : south link
	// bits 30-31 : west link
	uint32	_Id;
	
	// 6 bits 0-5 topology
	// 2 bits 6-7 gabarit
	uint8	_Topology;
	
	// 1 bit 0 interior
	// 1 bit 1 water
	// 1 bit 2 nogo
	// 13 bits 3-15 height
	sint16	_Height;
};
// a cell is composed of units of 3 slots
typedef CUnitSlot TCellUnit[3];

inline
CUnitSlot::CUnitSlot()
: _Id(0xffffffff)
, _Topology(0)
, _Height(0)
{
}

inline
CUnitSlot::CUnitSlot(uint instanceId, uint surfaceId)
: _Topology(0)
, _Height(0)
{
#ifdef NL_DEBUG
	nlassert(instanceId<InstanceIdMax);
	nlassert(surfaceId<SurfaceIdMax);
#endif
	_Id = SlotMask + (instanceId << InstanceIdOffset) + (surfaceId << SurfaceIdOffset);
}

inline
CUnitSlot::CUnitSlot(NLPACS::UGlobalPosition const& pos)
: _Topology(0)
, _Height(0)
{
#ifdef NL_DEBUG
	nlassert(pos.InstanceId < InstanceIdMax);
	nlassert(pos.LocalPosition.Surface < SurfaceIdMax);
#endif
	_Id=SlotMask+(pos.InstanceId<<InstanceIdOffset)+(pos.LocalPosition.Surface<<SurfaceIdOffset);
}

inline
void CUnitSlot::setInstanceId(uint instanceId)
{
#ifdef NL_DEBUG
	nlassert(instanceId < InstanceIdMax);
#endif
	_Id = (_Id & (~InstanceIdMask)) + (instanceId << InstanceIdOffset);
}

inline
void CUnitSlot::setSurface(uint surfaceId)
{
#ifdef NL_DEBUG
	nlassert(surfaceId < SurfaceIdMax);
#endif
	_Id = (_Id & (~SurfaceIdMask)) + (surfaceId << SurfaceIdOffset);
}

inline
void CUnitSlot::setTopology(uint topology)
{
#ifdef NL_DEBUG
	nlassert(topology < 63);
#endif
	_Topology = (_Topology & (~TopologyMask)) + (uint8)topology;
}

inline
void CUnitSlot::setGabarit(uint gabarit)
{
#ifdef NL_DEBUG
	nlassert(gabarit < 4);
#endif
	_Topology = (_Topology & (~GabaritMask)) | (uint8)(gabarit << GabaritShift);
}

inline
void CUnitSlot::setHeight(sint height)
{
#ifdef NL_DEBUG
	nlassert(height >= -4096 && height <= 4095);
#endif
	_Height = (_Height & (~HeightMask)) | (sint16)(height << HeightShift);
}

inline
void CUnitSlot::setInterior(bool interior)
{
	if (interior)
		_Height	|=	InteriorMask;
	else
		_Height	&=	~InteriorMask;
}

inline
void CUnitSlot::setWater(bool water)
{
	if (water)
		_Height |= WaterMask;
	else
		_Height	&=	~WaterMask;
}

inline
void CUnitSlot::setNoGo(bool nogo)
{
	if (nogo)
		_Height |= NoGoMask;
	else
		_Height &= ~NoGoMask;
}

inline
void CUnitSlot::reset()
{
	_Id = 0xffffffff;
	_Topology = 0;
	_Height = 0;
}

inline
bool CUnitSlot::used() const
{
	return _Id != 0xffffffff;
}

inline
bool CUnitSlot::hasSameSurface(const CUnitSlot &slot)	const
{
	return ((slot._Id ^ _Id) & (~SlotMask)) == 0;
}

inline
void CUnitSlot::serial(NLMISC::IStream &f)
{
	f.serial(_Id);
	f.serial(_Topology);
	f.serial(_Height);
}

//////////////////////////////////////////////////////////////////////////////

class CMapPosition;

typedef sint32 TMapCoordType;

/**
 * A simple 1 axis coordinate
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CMapCoord
{
	friend class CMapPosition;
public:
	explicit CMapCoord(uint scellId, uint cellId, uint unitId);
	
private:
	explicit CMapCoord() : c(0) { }
	explicit CMapCoord(CAICoord const& aiCoord) : c((sint)(floor(aiCoord.asDouble()+0.5))) { }
	explicit CMapCoord(sint cc) : c(cc) { }
	
	uint cellIdMask(uint val) const;
	uint fullCellIdMask(uint val) const;
	
	uint toUnitId(uint val) const;
	uint toCellId(uint val) const;
	uint toSuperCellId(uint val) const;
	
	uint unitIdToUInt(uint val) const;
	uint cellIdToUInt(uint val) const;
	uint superCellIdToUInt(uint val) const;
	uint fullCellIdToUInt(uint val) const;
	
	TMapCoordType& cRef();
	
public:
	uint getUnitId		() const { return unitIdToUInt(c); }
	uint getCellId		() const { return cellIdToUInt(c); }
	uint getSuperCellId	() const { return superCellIdToUInt(c); }
	uint getFullCellId	() const { return fullCellIdToUInt(c); }
	
	void setUnitId(uint id) { c = fullCellIdMask(c)	| toUnitId(id); }
	void setCellId(uint id) { c = cellIdMask(c)		| toCellId(id); }
	
	bool operator ==(CMapCoord const& cc) const { return c == cc.c; }
	bool operator !=(CMapCoord const& cc) const { return c != cc.c; }
	CMapCoord operator -(CMapCoord const& cc) const { return CMapCoord(c-cc.c); }
	
	TMapCoordType const& get() const { return c; }
	
private:
	// coordinate
	TMapCoordType c;
};

inline
CMapCoord::CMapCoord(uint scellId, uint cellId, uint unitId)
: c(toSuperCellId(scellId) + toCellId(cellId) + toUnitId(unitId))
{
#ifdef NL_DEBUG
	nlassert(scellId < 256);
#endif
}

inline
uint CMapCoord::cellIdMask(uint val) const
{
	return val&0xff0f;
}

inline
uint CMapCoord::fullCellIdMask(uint val) const
{
	return val&0x0fffffff0;
}

inline
uint CMapCoord::toUnitId(uint val) const
{
	return val&0x0f;
}

inline
uint CMapCoord::toCellId(uint val) const
{
	return (val<<4)&0x0f0;
}

inline
uint CMapCoord::toSuperCellId(uint val) const
{
	return (val<<8)&0x0ffff;
}

inline
uint CMapCoord::unitIdToUInt(uint val) const
{
	return val&0x0f;
}

inline
uint CMapCoord::cellIdToUInt(uint val) const
{
	return (val>>4)&0xf;
}	

inline
uint CMapCoord::superCellIdToUInt(uint val) const
{
	return (val>>8)&0x0ff;
}

inline
uint CMapCoord::fullCellIdToUInt(uint val) const
{
	return (val>>4)&0x0fff;
}

inline
TMapCoordType& CMapCoord::cRef()
{
	return c;
}

//////////////////////////////////////////////////////////////////////////////

/**
 * A map position, consisting of 2 axis coordinates
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CMapPosition
{
	friend class RYPACSCRUNCH::CPacsCruncher;
public:
	CMapPosition() { }
	CMapPosition(sint x, sint y);
	CMapPosition(CMapCoord const& x, CMapCoord const& y);
	CMapPosition(CMapPosition const& pos);
	CMapPosition(NLMISC::CVectorD const& pos);
	CMapPosition(CAIVector const& pos);
	CMapPosition(CAICoord const& x, CAICoord const& y);
	
	NLMISC::CVectorD toVectorD() const;
	NLMISC::CVector2d toVector2d() const;
	CAIVector toAIVector() const;
	
	std::string toString() const;
	
	void setUnitId(uint const xId, uint const yId);
	void setNullUnitId();
	
	CMapPosition stepCell(sint dx, sint dy) const;
	
	bool hasSameFullCellId(CMapPosition const& other) const;
	
	CMapPosition operator -(CMapPosition const& other) const;
	bool operator ==(CMapPosition const& cc) const;
	bool operator !=(CMapPosition const& cc) const;
	
	CMapCoord const& xCoord() const { return _x; }
	CMapCoord const& yCoord() const { return _y; }
	
	TMapCoordType const& x() const { return _x.get(); }
	TMapCoordType const& y() const { return _y.get(); }	
	
	uint32 superCellFastIndex() const;
	uint32 rootCellFastIndex() const;
	uint32 cellUnitFastIndex() const;
	
	CMapPosition step(sint dx, sint dy) const;
	
	CMapPosition getStepS() const { return step(0, -1); }
	CMapPosition getStepN() const { return step(0, +1); }
	CMapPosition getStepE() const { return step(+1, 0); }
	CMapPosition getStepW() const { return step(-1, 0); }
protected:

	// return if we have changed of RootCell ..
	bool stepS() { _y.cRef()-=1; return ((_y.cRef()&0xf)==0xf); }
	bool stepN() { _y.cRef()+=1; return ((_y.cRef()&0xf)==0x0); }
	bool stepW() { _x.cRef()-=1; return ((_x.cRef()&0xf)==0xf); }
	bool stepE() { _x.cRef()+=1; return ((_x.cRef()&0xf)==0x0); }
	
private:
  	CMapCoord _x;
	CMapCoord _y;
};

inline
CMapPosition::CMapPosition(sint x, sint y)
: _x(CMapCoord(x))
, _y(CMapCoord(y))
{
}

inline
CMapPosition::CMapPosition(CMapCoord const& x, CMapCoord const& y)
: _x(x)
, _y(y)
{
}

inline
CMapPosition::CMapPosition(CMapPosition const& pos)
: _x(pos._x)
, _y(pos._y)
{
}

inline
CMapPosition::CMapPosition(NLMISC::CVectorD const& pos)
: _x((uint)floor((pos.x - WorldStartOffset.x)/WorldGridResolution + 0.5))
, _y((uint)floor((pos.y - WorldStartOffset.y)/WorldGridResolution + 0.5))
{
}

inline
CMapPosition::CMapPosition(CAIVector const& pos)
: _x(pos.x())
, _y(pos.y())
{
}

inline
CMapPosition::CMapPosition(CAICoord const& x, CAICoord const& y)
: _x(x)
, _y(y)
{
}

inline
NLMISC::CVectorD CMapPosition::toVectorD() const
{
	return NLMISC::CVectorD(WorldStartOffset.x + _x.get()*WorldGridResolution, WorldStartOffset.y + _y.get()*WorldGridResolution, 0.0);
}

inline
NLMISC::CVector2d CMapPosition::toVector2d() const
{
	return NLMISC::CVector2d(WorldStartOffset.x + _x.get()*WorldGridResolution, WorldStartOffset.y + _y.get()*WorldGridResolution);
}

inline
CAIVector CMapPosition::toAIVector() const
{
	return CAIVector(WorldStartOffset.x + _x.get()*WorldGridResolution, WorldStartOffset.y + _y.get()*WorldGridResolution);
}

inline
std::string CMapPosition::toString() const
{
	return toAIVector().toString();
}

inline
void CMapPosition::setUnitId(uint const xId, uint const yId)
{
	_x.setUnitId(xId);
	_y.setUnitId(yId);
}

inline
void CMapPosition::setNullUnitId()
{
  _x.setUnitId(0);
  _y.setUnitId(0);
}

inline
CMapPosition CMapPosition::stepCell(sint dx, sint dy) const
{
	CMapPosition newMapPos(xCoord().get()+(dx<<4),yCoord().get()+(dy<<4));
	newMapPos.setNullUnitId();		
	return newMapPos;
}

inline
bool CMapPosition::hasSameFullCellId(CMapPosition const& other) const
{
	return ( _x.getFullCellId()==other._x.getFullCellId() && _y.getFullCellId()==other._y.getFullCellId() ); 
}

inline
CMapPosition CMapPosition::operator -(CMapPosition const& other) const
{	
	return CMapPosition(_x-other._x,_y-other._y);	
}

inline
bool CMapPosition::operator ==(CMapPosition const& cc) const
{
	return _x==cc._x && _y==cc._y;
}

inline
bool CMapPosition::operator !=(CMapPosition const& cc) const
{
	return _x!=cc._x || _y!=cc._y;
}

inline
uint32 CMapPosition::superCellFastIndex() const
{
	return ((((uint)y())&(0x0ff<<8)) + xCoord().getSuperCellId());
}

inline
uint32 CMapPosition::rootCellFastIndex() const
{
	return ((((uint)y())&0x0f<<4) + xCoord().getCellId());
}

inline
uint32 CMapPosition::cellUnitFastIndex() const
{
	return ((yCoord().getUnitId()<<4) + xCoord().getUnitId());
}

inline
CMapPosition CMapPosition::step(sint dx, sint dy) const
{
	return CMapPosition(_x.get() + dx, _y.get() + dy);
}

//////////////////////////////////////////////////////////////////////////////

/**
 * Motion map interface
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CGridDirectionLayer
: public I16x16Layer
{
public:
	CDirection getDirection(int y, int x) const;
	CDirection getDirection(CMapPosition const& pos) const;
	
	void setDirection(int y, int x, CDirection dir);
};

inline
CDirection CGridDirectionLayer::getDirection(int y, int x) const
{
	return CDirection((CDirection::TDirection)get(y, x));
}

inline
CDirection CGridDirectionLayer::getDirection(CMapPosition const& pos) const
{
	return CDirection((CDirection::TDirection)get(pos.yCoord().getUnitId(), pos.xCoord().getUnitId()));
}

inline
void CGridDirectionLayer::setDirection(int y, int x, CDirection dir)
{
	this->set((uint)y, (uint)x, (sint)dir.getVal());
}

//////////////////////////////////////////////////////////////////////////////

class CDirectionLayer
{
public:
	CDirectionLayer();
	
	void serial(NLMISC::IStream& f);
	
	CGridDirectionLayer* getGridLayer(int y, int x);
	void setGridLayer(int y, int x, I16x16Layer* layer);
	
	void dump();
	
private:
	// Grid[1][1] is central cell
	I16x16Layer	*Grid[3][3];
};

inline
CDirectionLayer::CDirectionLayer()
{
	uint i, j;
	for (i=0; i<3; ++i)
		for (j=0; j<3; ++j)
			Grid[i][j] = NULL;
}

inline
CGridDirectionLayer* CDirectionLayer::getGridLayer(int y, int x)
{
	return static_cast<CGridDirectionLayer*>(Grid[y][x]);
}	

inline
void CDirectionLayer::setGridLayer(int y, int x, I16x16Layer* layer)
{
	Grid[y][x] = layer;
}

inline
void CDirectionLayer::dump()
{
	char output[16*3][16*3];
	for (sint i=2; i>=0; --i)
		for (sint j=0; j<3; ++j)
			for (sint y=15; y>=0; --y)
				for (sint x=0; x<16; ++x)
	{
		CGridDirectionLayer* gridDirectionLayer = getGridLayer(i,j);
		CDirection motion;
		
		if (gridDirectionLayer)
			motion = gridDirectionLayer->getDirection(y,x);
		
		char c = ' ';

		switch (motion.getVal())
		{
		case CDirection::N:
			c = '^';
			break;
		case CDirection::S:
			c = 'v';
			break;
		case CDirection::E:
			c = '>';
			break;
		case CDirection::W:
			c = '<';
			break;
		case CDirection::NE:
			c = '7';
			break;
		case CDirection::SW:
			c = 'L';
			break;
		case CDirection::NW:
			c = 'r';
			break;
		case CDirection::SE:
			c = '\\';
			break;
//		case 255:
//			c = ' ';
//			break;
		default:
			c = 'o';
			break;
		}
		output[y+i*16][x+j*16] = c;
	}
	
	char op[256];
	nlinfo("    \t0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF");
	
	for (sint i=47; i>=0; --i)
	{
		sint j;
		for (j=0; j<48; ++j)
			op[j] = output[i][j];				
		op[j] = '\0';
		nlinfo("%04X\t%s", (i&15), op);
	}
}

//////////////////////////////////////////////////////////////////////////////

class CDirectionMap
{
public:
	CDirectionLayer* Layers[3];
	
	CDirectionMap();
	
	void serial(NLMISC::IStream& f);
	
	void dump();
};

inline
CDirectionMap::CDirectionMap()
{
	Layers[0] = NULL;
	Layers[1] = NULL;
	Layers[2] = NULL;
}

inline
void CDirectionMap::dump()
{
	for (uint i=0; i<3; ++i)
	{
		nlinfo("Layer %d", i);
		if (Layers[i] != NULL)
			Layers[i]->dump();
		else
			nlinfo("Empty");
	}
}

//////////////////////////////////////////////////////////////////////////////

class CWorldMap;
class CRootCell;
class CWorldPosition;

class CTopology
{
public:

	/// A topology Id, representing cell and topology
	class TTopologyId
	{
	public:
		enum
		{
			UNDEFINED_TOPOLOGY	=	0xffffffff
		};
		
	public:
		explicit TTopologyId();
		explicit TTopologyId(CMapPosition const& pos, uint topology);
		// For test only
		explicit TTopologyId(uint32 id);
		
		bool haveSameCellPos(TTopologyId const& other) const;
		
		CMapPosition getMapPosition() const;
		uint getTopologyIndex() const;
		
		bool isValid() const;
		
		bool operator <(TTopologyId const& other) const;
		bool operator ==(TTopologyId const& other) const;
		bool operator !=(TTopologyId const& other) const;
		
		void serial(NLMISC::IStream& f);
		
		uint32 getVal() const;
		
	private:
		uint32 _value;
	};
	
	class TTopologyRef
	: public TTopologyId
	{
		friend class CWorldPosition;
	public:
		TTopologyRef(CWorldPosition const& pos);
		TTopologyRef();
		explicit TTopologyRef(TTopologyId const& id, CRootCell* rootCell);
		CTopology const& getCstTopologyNode() const;
		void setRootCell(CRootCell* rootCell);
		
		void serial(NLMISC::IStream& f);
		CRootCell const* getRootCell() const;
		
	private:
		TTopologyRef(CWorldPosition const& pos, CRootCell const* rootCell);
		CRootCell const* _RootCell;
	};
	
	/// A link to a neighbour, including mean distance to it
	class CNeighbourLink
	{
	public:
		explicit CNeighbourLink();
		explicit CNeighbourLink(TTopologyRef const& ref, float distance);
		CNeighbourLink(CNeighbourLink const& other);
		
		void serial(NLMISC::IStream& f);
		
		TTopologyRef const& getTopologyRef() const;
		
		void updateTopologyRef(CWorldMap* worldMapPtr);
		
		float getDistance() const;
		
	private:
		TTopologyRef _Ref;
		float _Distance;
	};
	
	void updateTopologyRef(CWorldMap* worldMapPtr);
	
	bool isInInterior() const { return (Flags & Interior) != 0; }
	bool isInWater() const { return (Flags & Water) != 0; }
	bool isInNogo() const { return (Flags & NoGo) != 0; }
	
	TAStarFlag getFlags() const { return (TAStarFlag)Flags; }
	
	uint32 getMasterTopo(TAStarFlag const& flags) const;
	
	uint32 getMasterTopo(bool allowWater, bool allowNoGo) const;
	
	uint32& getMasterTopoRef(bool allowWater, bool allowNoGo);
	
	bool isCompatible(bool allowWater, bool allowNoGo) const;
	
	/// The Id of this topology
	TTopologyId	 Id;
	/// The direction map to access this topology
	CDirectionMap* DirectionMap;
	/// The neighbour topologies that have access to this topology
	std::vector<CNeighbourLink>	Neighbours;
	/// Flags of the topology
	uint16 Flags;			// why doing this (uint16), its not align so no gain in memory and it causes access cost !
	/// Master topologies
	uint32 MasterTopL;		// only landscape
	uint32 MasterTopLW;	// landscape and water
	uint32 MasterTopLN;	// landscape and nogo
	uint32 MasterTopLNW;	// landscape and water and nogo
	
	/// Topology center, for A* purpose
	NLMISC::CVector Position;
	
	CTopology();
	
	void serial(NLMISC::IStream& f);
};

inline
CTopology::TTopologyId::TTopologyId()
: _value(UNDEFINED_TOPOLOGY)
{
}

inline
CTopology::TTopologyId::TTopologyId(CMapPosition const& pos, uint topology)
{
#ifdef NL_DEBUG
	nlassert(topology < 256);
#endif
	_value = (pos.yCoord().getFullCellId() << 20) + (pos.xCoord().getFullCellId() << 8) + topology;
}

inline
CTopology::TTopologyId::TTopologyId(uint32 id)
{
	_value = id;
}

inline
bool CTopology::TTopologyId::haveSameCellPos(TTopologyId const& other) const
{
	return ((other._value^_value)&0x0ffffff00)==0;
}

inline
CMapPosition CTopology::TTopologyId::getMapPosition() const
{
	return CMapPosition((_value&0x000fff00) >> (8-4),(_value&0xfff00000) >> (20-4));
}

inline
uint CTopology::TTopologyId::getTopologyIndex() const
{
	return _value&0x000000ff;
}

inline
bool CTopology::TTopologyId::isValid() const
{
	return _value!=UNDEFINED_TOPOLOGY;
}

inline
bool CTopology::TTopologyId::operator <(TTopologyId const& other) const
{
	return _value<other._value;
}

inline
bool CTopology::TTopologyId::operator ==(TTopologyId const& other) const
{
	return _value==other._value;
}

inline
bool CTopology::TTopologyId::operator !=(TTopologyId const& other) const
{
	return _value!=other._value;
}

inline
void CTopology::TTopologyId::serial(NLMISC::IStream& f)
{
	f.serial(_value);
}

inline
uint32 CTopology::TTopologyId::getVal() const
{
	return _value;
}

inline
CTopology::TTopologyRef::TTopologyRef()
: TTopologyId()
, _RootCell(NULL)
{
}

inline
CTopology::TTopologyRef::TTopologyRef(TTopologyId const& id, CRootCell* rootCell)
: TTopologyId(id)
{
	setRootCell(rootCell);
}

inline
void CTopology::TTopologyRef::setRootCell(CRootCell* rootCell)
{
	_RootCell = rootCell;
}

inline
void CTopology::TTopologyRef::serial(NLMISC::IStream& f)
{
	TTopologyId::serial(f);
}

inline
CRootCell const* CTopology::TTopologyRef::getRootCell() const
{
	return _RootCell;
}

inline
CTopology::CNeighbourLink::CNeighbourLink()
: _Ref()
{
}

inline
CTopology::CNeighbourLink::CNeighbourLink(TTopologyRef const& ref, float distance)
: _Ref(ref)
, _Distance(distance)
{
}

inline
CTopology::CNeighbourLink::CNeighbourLink(CNeighbourLink const& other)
: _Ref(other._Ref)
,	_Distance(other._Distance)
{
}

inline
void CTopology::CNeighbourLink::serial(NLMISC::IStream& f)
{
	f.serial(_Ref, _Distance);
}

inline
CTopology::TTopologyRef const& CTopology::CNeighbourLink::getTopologyRef() const
{
	return _Ref;
}

inline
float CTopology::CNeighbourLink::getDistance() const
{
	return _Distance;
}

inline
void CTopology::updateTopologyRef(CWorldMap* worldMapPtr)
{
	std::vector<CNeighbourLink>::iterator it=Neighbours.begin(), itEnd=Neighbours.end();
	while (it!=itEnd)
	{
		(*it).updateTopologyRef(worldMapPtr);
		++it;
	}
}

inline
uint32 CTopology::getMasterTopo(TAStarFlag const& flags) const
{
	switch (flags&WaterAndNogo)
	{
	case Nothing:
	default:
		return MasterTopL;
	case Water:
		return MasterTopLW;
	case NoGo:
		return MasterTopLN;
	case WaterAndNogo:
		return MasterTopLNW;
	}
}

inline
uint32 CTopology::getMasterTopo(bool allowWater, bool allowNoGo) const
{
	if (allowWater)
	{
		if (allowNoGo)
			return MasterTopLNW;
		return MasterTopLW;
	}
	if (allowNoGo)
		return MasterTopLN;
	return MasterTopL;
}

inline
uint32& CTopology::getMasterTopoRef(bool allowWater, bool allowNoGo)
{
	if (allowWater)
	{
		if (allowNoGo)
			return MasterTopLNW;
		return MasterTopLW;
	}
	if (allowNoGo)
		return MasterTopLN;
	return MasterTopL;
}

inline
bool CTopology::isCompatible(bool allowWater, bool allowNoGo) const
{
	return (allowWater || !isInWater()) && (allowNoGo || !isInNogo());
}

inline
CTopology::CTopology()
: Id()
, DirectionMap(NULL)
, Flags(Nothing)
, MasterTopL(TTopologyId::UNDEFINED_TOPOLOGY)
, MasterTopLW(TTopologyId::UNDEFINED_TOPOLOGY)
, MasterTopLN(TTopologyId::UNDEFINED_TOPOLOGY)
, MasterTopLNW(TTopologyId::UNDEFINED_TOPOLOGY)
{
}

// convert a 2 characters string to uint16
#ifdef NL_LITTLE_ENDIAN
#	define NELID16(x) (uint16((x[0] << 8) | (x[1])))
#else
#	define NELID16(x) (uint16((x[1] << 8) | (x[0])))
#endif



inline
void CTopology::serial(NLMISC::IStream& f)
{
	
	uint version = 0;
	
	uint16	check = NELID16("Tp");
	f.serial(check);
	
	if (check != NELID16("TP"))
	{
		nlassert(check == NELID16("Tp"));
		version = f.serialVersion(3);
	}
	
	f.serial(Id);
	
	if (f.isReading())
	{
		delete DirectionMap;
		DirectionMap = NULL;
	}
	
	bool	present = (DirectionMap != NULL);
	f.serial(present);
	
	if (present)
	{
		if (f.isReading())
			DirectionMap = new CDirectionMap();
		
		f.serial(*DirectionMap);
	}
	
	f.serialCont(Neighbours);
	
	f.serial(Position);
	
	if (version >= 3)
	{
		f.serial(Flags);
		
		f.serial(MasterTopL);
		f.serial(MasterTopLW);
		f.serial(MasterTopLN);
		f.serial(MasterTopLNW);
	}
	else
	{
		nlassert(f.isReading());
		
		Flags = Nothing;
		
		if (version >= 1)
		{
			bool	interior;
			f.serial(interior);
			if	(interior)
				Flags |= Interior;
		}
		
		if (version >= 2)
		{
			bool	water;
			f.serial(water);
			if	(water)
				Flags |= Water;
		}
		
		MasterTopL = 0;
		MasterTopLW = 0;
		MasterTopLN = 0;
		MasterTopLNW = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////

/**
 * CWorldPosition, position referenced by a map position and a slot
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CWorldPosition
: public CMapPosition
, public CSlot
{
	friend class CWorldMap;
	friend class RYPACSCRUNCH::CPacsCruncher;
public:
	explicit CWorldPosition();
	explicit CWorldPosition(sint x, sint y);
	
	// resumes that the RootCell is valid;
	CRootCell const* getRootCell() const { return _RootCell; }
	
	/// In millimeters.
	sint32 getMetricHeight() const;
	sint32 h() const { return getMetricHeight(); }
	
	CCellLinkage const& getCellLinkage() const { return _cellLinkage; }
	
	CTopology::TTopologyRef getTopologyRef() const;
	
	bool isInInterior() const;
	
	CTopology const& getTopologyNode() const;
	
	TAStarFlag getFlags() const;
	
	/// Get a vector position from a world position
	NLMISC::CVectorD getPosition() const;
	
	bool operator ==(CWorldPosition const& cc) const { return CSlot::operator ==(cc) && CMapPosition::operator ==(cc); }
	bool operator !=(CWorldPosition const& cc) const { return CSlot::operator !=(cc) || CMapPosition::operator !=(cc); }
	
private:
	CWorldPosition getPosS() const;
	CWorldPosition getPosN() const;
	CWorldPosition getPosE() const;
	CWorldPosition getPosW() const;
	
	void setPosS(CWorldPosition& pos) const;
	void setPosN(CWorldPosition& pos) const;
	void setPosE(CWorldPosition& pos) const;
	void setPosW(CWorldPosition& pos) const;
	
	bool moveS();
	bool moveN();
	bool moveE();
	bool moveW();
	
	void stepS();
	void stepN();
	void stepE();
	void stepW();
	
	CRootCell const* _RootCell;
	CCellLinkage _cellLinkage;
	
	explicit CWorldPosition(CRootCell const* cell, CMapPosition const& pos, CSlot const& slot);
	explicit CWorldPosition(CRootCell const* cell, CMapPosition const& pos, CSlot const& slot, bool generationOnly);
};

inline
CWorldPosition::CWorldPosition()
: CMapPosition()
, CSlot()
, _RootCell(NULL)
, _cellLinkage(0)
{
}

inline
CWorldPosition::CWorldPosition(sint x, sint y)
: CMapPosition(x, y)
, CSlot()
, _RootCell(NULL)
, _cellLinkage(0)
{
}

inline
CTopology::TTopologyRef CWorldPosition::getTopologyRef() const
{
	return CTopology::TTopologyRef(*this, getRootCell());
}

inline
bool CWorldPosition::isInInterior() const
{
	return getTopologyRef().getCstTopologyNode().isInInterior();
}

inline
CTopology const& CWorldPosition::getTopologyNode() const
{
	return getTopologyRef().getCstTopologyNode();
}

inline
TAStarFlag CWorldPosition::getFlags() const
{
	return getTopologyRef().getCstTopologyNode().getFlags();	//Flags;
}

//////////////////////////////////////////////////////////////////////////////

class CCompatibleResult
{
public:	
	CCompatibleResult(TAStarFlag movementFlags = Interior, uint32 choosenMasterTopo = ~0);
	void set(TAStarFlag movementFlags, uint32 choosenMasterTopo);
	void setValid(bool valid = true);
	TAStarFlag const& movementFlags() const;
	uint32 const& choosenMasterTopo() const;
	bool const& isValid() const;
	
private:
	TAStarFlag _MovementFlags;
	uint32 _ChoosenMasterTopo;
	bool _Valid;
};

inline
CCompatibleResult::CCompatibleResult(TAStarFlag movementFlags, uint32 choosenMasterTopo)
: _MovementFlags(movementFlags)
, _ChoosenMasterTopo(choosenMasterTopo)
, _Valid(false)
{
}

inline
void CCompatibleResult::set(TAStarFlag movementFlags, uint32 choosenMasterTopo)
{
	_MovementFlags = movementFlags;
	_ChoosenMasterTopo = choosenMasterTopo;
}

inline
void CCompatibleResult::setValid(bool valid)
{
	_Valid = valid;
}

inline
TAStarFlag const& CCompatibleResult::movementFlags() const
{
	return _MovementFlags;
}

inline
uint32 const& CCompatibleResult::choosenMasterTopo() const
{
	return _ChoosenMasterTopo;
}

inline
bool const& CCompatibleResult::isValid() const
{
	return _Valid;
}

//////////////////////////////////////////////////////////////////////////////

/**
@relates CWorldPosition
*/
void areCompatiblesWithoutStartRestriction(CWorldPosition const& startPos, CWorldPosition const& endPos, TAStarFlag const& denyflags, CCompatibleResult& res, bool allowStartRestriction = false);

//////////////////////////////////////////////////////////////////////////////

/**
 * CRootCell, interface for all 16x16 cells in world map
 * derivated into CComputeCell (for map computation only), into CSingleLayerCell and CMultiLayerCell
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CRootCell
{
public:
	enum TCellType
	{
		Root = 0,
		Compute,
		White,
		SingleLayer,
		MultiLayer
	};
	
public:
	CRootCell(CWorldMap const& worldMapPtr);
	
	virtual ~CRootCell() { }
	
	void setFlag(uint32 const flag) { _flag = flag; }
	uint32 getFlag() const { return _flag; }
	
	sint32 getMetricHeight(CWorldPosition const& wpos) const;
	
	CWorldMap const& getWorldMapPtr() const { return _WorldMapPtr; }
	
	// mutator (only for build purpose)
	CTopology& getTopologyNode(uint topology);
	CTopology const& getCstTopologyNode(uint topology) const;
	
	CTopology::TTopologyId getTopologyId(CWorldPosition const& wpos) const;
	
	CTopology::TTopologyRef	getTopologyRef(const CWorldPosition&pos) const;
	
	CTopology const& getCstTopologyNode(CTopology::TTopologyId const& id) const;
	
	std::vector<CTopology>& getTopologiesNodes() { return _TopologiesNodes; }
	std::vector<CTopology> const& getTopologiesNodes() const { return _TopologiesNodes; }	
	void setTopologiesNodes(std::vector<CTopology> const& nodes) { _TopologiesNodes = nodes; }
	
	CDirectionMap const* getDirectionMap(uint topology) const;
	void setDirectionMap(CDirectionMap* map, uint topology);
	
	void updateTopologyRef(CWorldMap* worldMap);
	
	CWorldPosition const& getWorldPosition(uint ind) const;
	
	void setWorldPosition(CWorldPosition const& pos, uint ind);
	
	virtual CCellLinkage getCellLink(CWorldPosition const& wpos) const = 0;
	
	virtual uint32 nbUsedSlots(CMapPosition const& pos) const = 0;
	virtual sint32 maxUsedSlot(CMapPosition const& pos) const = 0;
	virtual bool isSlotUsed(CMapPosition const& pos, CSlot const& slot) const = 0;
	
	virtual uint getTopology(CWorldPosition const& wpos) const = 0;
	
	virtual sint getHeight(CWorldPosition const& wpos) const = 0;
	
	// clear height map
	virtual void clearHeightMap() = 0;
	
	// serial
	virtual void serial(NLMISC::IStream& f) = 0;
	
	// load a cell
	static CRootCell* load(NLMISC::IStream& f, CWorldMap const& worldMap);
	
	// save a cell
	static void save(NLMISC::IStream& f, CRootCell* cell);
	
private:
	CWorldMap const& _WorldMapPtr;	//	his owner map;
	std::vector<CTopology> _TopologiesNodes;
	CWorldPosition _WorldPosition[4]; // 4 random positions for each slots.
	uint32 _flag;
};

inline
CRootCell::CRootCell(CWorldMap const& worldMapPtr)
: _WorldMapPtr(worldMapPtr)
{
	_flag = 0;
}

inline
sint32 CRootCell::getMetricHeight(CWorldPosition const& wpos) const
{
	// check if wpos valid
	if (!wpos.isValid())
		return 0;
	
	sint32 const z = (getHeight(wpos)*2000 - 1000)&(~3);
	uint const top = getTopology(wpos);
	if (top<_TopologiesNodes.size() && _TopologiesNodes[top].isInInterior())
		return z+2;
	return z;
}

inline
CTopology& CRootCell::getTopologyNode(uint topology)
{
	if (topology >= _TopologiesNodes.size())
		_TopologiesNodes.resize(topology+1);
	
	return _TopologiesNodes[topology];
}

inline
CTopology const& CRootCell::getCstTopologyNode(uint topology) const
{
#if !FINAL_VERSION
	nlassert(topology < _TopologiesNodes.size());
#endif
	return _TopologiesNodes[topology];
}

inline
CTopology::TTopologyId CRootCell::getTopologyId(CWorldPosition const& wpos) const
{		
	return CTopology::TTopologyId(wpos, getTopology(wpos));
}

inline
CTopology::TTopologyRef	CRootCell::getTopologyRef(const CWorldPosition&pos) const
{
#ifdef NL_DEBUG
	nlassert(pos.getRootCell()!=this);
#endif
	return	CTopology::TTopologyRef(pos);
}

inline
CTopology const& CRootCell::getCstTopologyNode(CTopology::TTopologyId const& id) const
{
	return getCstTopologyNode(id.getTopologyIndex());
}

inline
CDirectionMap const* CRootCell::getDirectionMap(uint topology) const
{
	return (topology<_TopologiesNodes.size()) ? getCstTopologyNode(topology).DirectionMap : NULL;
}

inline
void CRootCell::setDirectionMap(CDirectionMap* map, uint topology)
{
	getTopologyNode(topology).DirectionMap = map;
}

inline
void CRootCell::updateTopologyRef(CWorldMap* worldMap)
{
	std::vector<CTopology>::iterator it=_TopologiesNodes.begin(), itEnd=_TopologiesNodes.end();
	while (it!=itEnd)
	{
		(*it).updateTopologyRef(worldMap);
		++it;
	}
}

inline
CWorldPosition const& CRootCell::getWorldPosition(uint ind) const
{
#ifdef NL_DEBUG
	nlassert(ind<4);
#endif
	return _WorldPosition[ind];
}

inline
void CRootCell::setWorldPosition(CWorldPosition const& pos, uint ind)
{
#ifdef NL_DEBUG
	nlassert(ind<4);
#endif
	_WorldPosition[ind]	= pos;
}

//////////////////////////////////////////////////////////////////////////////

/**
 * A computed 16x16 cell
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CComputeCell
: public CRootCell
{
public:
	CComputeCell(CWorldMap const& worldMapPtr);
	
	CCellLinkage getCellLink(CWorldPosition const& wpos) const;
	
	uint32 nbUsedSlots(CMapPosition const& pos) const;
	
	sint32 maxUsedSlot(CMapPosition const& pos) const;
	
	bool isSlotUsed(CMapPosition const& pos, CSlot const& slot) const;
	
	uint getTopology(CWorldPosition const& wpos) const;
	
	sint getHeight(CWorldPosition const& wpos) const;
	
	TCellUnit const& getCellUnitCst(CMapPosition const& pos) const;
	
	CUnitSlot const& getUnitSlotCst(CWorldPosition const& wpos) const;
	
	TCellUnit& getCellUnit(CMapPosition const& pos);
	
	CUnitSlot& getUnitSlot(CWorldPosition const& wpos);
	
	void serial(NLMISC::IStream& f);

	void clearHeightMap() { }
	
private:
	TCellUnit const& getCellUnitCst(CWorldPosition const& wpos) const;
	
	// a cell is a grid of 16x16 units
	TCellUnit _Grid[16*16];
};

inline
CComputeCell::CComputeCell(CWorldMap const& worldMapPtr)
: CRootCell(worldMapPtr)
{
}

inline
CCellLinkage CComputeCell::getCellLink(CWorldPosition const& wpos) const
{
	return getUnitSlotCst(wpos).getCellLink();
}

inline
uint32 CComputeCell::nbUsedSlots(CMapPosition const& pos) const
{
	TCellUnit const& cellUnit = getCellUnitCst(pos);
	uint32 count = 0;
	for (uint slot=0; slot<3; ++slot)
		if (cellUnit[slot].getCellLink().used())
			++count;
	return count;
}

inline
sint32 CComputeCell::maxUsedSlot(CMapPosition const& pos) const
{
	TCellUnit const& cellUnit = getCellUnitCst(pos);
	sint32 maxs = -1;
	for (uint slot=0; slot<3; ++slot)
		if (cellUnit[slot].getCellLink().used())
			maxs = slot;
	return maxs;
}

inline
bool CComputeCell::isSlotUsed(CMapPosition const& pos, CSlot const& slot) const
{
	TCellUnit const& cellUnit = getCellUnitCst(pos);
	return slot.isValid() ? cellUnit[slot.slot()].getCellLink().used() : false;
}

inline
uint CComputeCell::getTopology(CWorldPosition const& wpos) const
{
	return getUnitSlotCst(wpos).topology();
}

inline
sint CComputeCell::getHeight(CWorldPosition const& wpos) const
{
	return getUnitSlotCst(wpos).height();
}

inline
TCellUnit const& CComputeCell::getCellUnitCst(CMapPosition const& pos) const
{
	return _Grid[pos.cellUnitFastIndex()];
}

inline
CUnitSlot const& CComputeCell::getUnitSlotCst(CWorldPosition const& wpos) const
{
#ifdef NL_DEBUG
	nlassert(wpos.isValid());
#endif
	return getCellUnitCst(wpos)[wpos.slot()];
}

inline
TCellUnit& CComputeCell::getCellUnit(CMapPosition const& pos)
{
	return _Grid[pos.cellUnitFastIndex()];
}

inline
CUnitSlot& CComputeCell::getUnitSlot(CWorldPosition const& wpos)
{
#ifdef NL_DEBUG
	nlassert(wpos.isValid());
#endif
	return getCellUnit(wpos)[wpos.slot()];
}

inline
TCellUnit const& CComputeCell::getCellUnitCst(CWorldPosition const& wpos) const
{
	return _Grid[wpos.cellUnitFastIndex()];
	}

//////////////////////////////////////////////////////////////////////////////

/**
 * A Single layer 16x16 cell that has not walk constraint (map is completely white)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CWhiteCell
: public CRootCell
{
public:
	CWhiteCell(CWorldMap const& worldMapPtr);
	
	CCellLinkage getCellLink(CWorldPosition const& wpos) const;
	
	uint32 nbUsedSlots(CMapPosition const& pos) const;
	
	sint32 maxUsedSlot(CMapPosition const& pos) const;
	
	bool isSlotUsed(CMapPosition const& pos, CSlot const& slot) const;
	
	uint getTopology(CWorldPosition const& wpos) const;
	
	sint getHeight(CWorldPosition const& wpos) const;
	
	void serial(NLMISC::IStream& f);
	
	void clearHeightMap();
	
	void setHeightMap(I16x16Layer* heightMap);
	
private:	
	I16x16Layer* _HeightMap;
};

inline
CWhiteCell::CWhiteCell(CWorldMap const& worldMapPtr)
: CRootCell(worldMapPtr), _HeightMap(NULL)
{
}

inline
CCellLinkage CWhiteCell::getCellLink(CWorldPosition const& wpos) const
{
	return CCellLinkage(0);
}

inline
uint32 CWhiteCell::nbUsedSlots(CMapPosition const& pos) const
{
	return 1;
}

inline
sint32 CWhiteCell::maxUsedSlot(CMapPosition const& pos) const
{
	return 0;
}

inline
bool CWhiteCell::isSlotUsed(CMapPosition const& pos, CSlot const& slot) const
{
	return slot.slot() == 0;
}

inline
uint CWhiteCell::getTopology(CWorldPosition const& wpos) const
{
	return 0;
}

inline
sint CWhiteCell::getHeight(CWorldPosition const& wpos) const
{
	return (_HeightMap != NULL) ? _HeightMap->get(wpos.yCoord().getUnitId(), wpos.xCoord().getUnitId()) : 0;
}

inline
void CWhiteCell::serial(NLMISC::IStream& f)
{
	f.serialCheck(NELID16("WC"));
	if (f.isReading())
		_HeightMap = I16x16Layer::load(f);
	else
		I16x16Layer::save(f, _HeightMap);
}

inline
void CWhiteCell::clearHeightMap()
{
	if (_HeightMap)
		delete _HeightMap;
	_HeightMap = NULL;
}

inline
void CWhiteCell::setHeightMap(I16x16Layer* heightMap)
{
	_HeightMap = heightMap;
}

//////////////////////////////////////////////////////////////////////////////

/**
 * A Single layer 16x16 cell
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CSingleLayerCell
: public CRootCell
{
protected:
	bool testPos(uint i, uint j) const { return (_Map[i] & _MaskMap[j]) != 0; }
	void setPos(uint i, uint j, bool p)	{ _Map[i] = (p ? (_Map[i] | _MaskMap[j]) : (_Map[i] & (~_MaskMap[j]))); }
	
public:
	CSingleLayerCell(CWorldMap const& worldMapPtr);
	
	CCellLinkage getCellLink(CWorldPosition const& wpos) const;
	
	uint32 nbUsedSlots(CMapPosition const& pos) const;
	
	sint32 maxUsedSlot(CMapPosition const& pos) const;
	
	bool isSlotUsed(CMapPosition const& pos, CSlot const& slot) const;
	
	uint getTopology(CWorldPosition const& wpos) const;
	
	sint getHeight(CWorldPosition const& wpos) const;
	
	bool testPos(CMapPosition const& pos) const;
	
	void setPos(CMapPosition const& pos, bool p);
	
	void setSLink(uint i, bool p) { _SLinks = p ? (_SLinks | _MaskMap[i]) : (_SLinks & (~_MaskMap[i])); }
	void setNLink(uint i, bool p) { _NLinks = p ? (_NLinks | _MaskMap[i]) : (_NLinks & (~_MaskMap[i])); }
	void setELink(uint i, bool p) { _ELinks = p ? (_ELinks | _MaskMap[i]) : (_ELinks & (~_MaskMap[i])); }
	void setWLink(uint i, bool p) { _WLinks = p ? (_WLinks | _MaskMap[i]) : (_WLinks & (~_MaskMap[i])); }
	
	void setTopologies(I16x16Layer* topology);
	
	void setHeightMap(I16x16Layer* heightMap);
	
	virtual void serial(NLMISC::IStream& f);
	
	void clearHeightMap();
	
private:
	/// The map of accessible positions
	uint16 _Map[16];
	/// The north links, true if north position is accessible
	uint16 _SLinks;
	uint16 _NLinks;
	uint16 _ELinks;
	uint16 _WLinks;
	
	// topology layer
	I16x16Layer* _Topologies;
	
	// height map
	I16x16Layer* _HeightMap;
	
	static uint16 _MaskMap[16];
	static bool _Initialized;	
};

inline
CSingleLayerCell::CSingleLayerCell(CWorldMap const& worldMapPtr)
: CRootCell(worldMapPtr)
{
	for (uint i=0; i<16; ++i)
		_Map[i] = 0;
	
	_SLinks = 0;
	_NLinks = 0;
	_ELinks = 0;
	_WLinks = 0;
	_Topologies = NULL;
	_HeightMap = NULL;
	
	if (!_Initialized)
	{
		for (uint i=0; i<16; ++i)
			_MaskMap[i] = (1 << (15-i));
		_Initialized = true;
	}
}

inline
CCellLinkage CSingleLayerCell::getCellLink(CWorldPosition const& wpos) const
{
	uint xi = wpos.xCoord().getUnitId();
	uint yi = wpos.yCoord().getUnitId();
	
	CCellLinkage l(0);
	
	l |= ((xi > 0  && testPos(yi, xi-1)) || (xi == 0  && (_WLinks&_MaskMap[yi]))) ? 0 : CCellLinkage::WestSlotMask;
	l |= ((xi < 15 && testPos(yi, xi+1)) || (xi == 15 && (_ELinks&_MaskMap[yi]))) ? 0 : CCellLinkage::EastSlotMask;
	l |= ((yi > 0  && testPos(yi-1, xi)) || (yi == 0  && (_SLinks&_MaskMap[xi]))) ? 0 : CCellLinkage::SouthSlotMask;
	l |= ((yi < 15 && testPos(yi+1, xi)) || (yi == 15 && (_NLinks&_MaskMap[xi]))) ? 0 : CCellLinkage::NorthSlotMask;
	
	return l;
}

inline
uint32 CSingleLayerCell::nbUsedSlots(CMapPosition const& pos) const
{
	return testPos(pos) ? 1 : 0;
}

inline
sint32 CSingleLayerCell::maxUsedSlot(CMapPosition const& pos) const
{
	return testPos(pos) ? 0 : -1;
}

inline
bool CSingleLayerCell::isSlotUsed(CMapPosition const& pos, CSlot const& slot) const
{
	return slot.slot() == 0 && testPos(pos);
}

inline
uint CSingleLayerCell::getTopology(CWorldPosition const& wpos) const
{
	return (_Topologies != NULL) ? _Topologies->get(wpos.yCoord().getUnitId(), wpos.xCoord().getUnitId()) : 0;
}

inline
sint CSingleLayerCell::getHeight(CWorldPosition const& wpos) const
{
	return (_HeightMap != NULL) ? _HeightMap->get(wpos.yCoord().getUnitId(), wpos.xCoord().getUnitId()) : 0;
}

inline
bool CSingleLayerCell::testPos(CMapPosition const& pos) const
{
	return testPos(pos.yCoord().getUnitId(), pos.xCoord().getUnitId());
}

inline
void CSingleLayerCell::setPos(CMapPosition const& pos, bool p)
{
	setPos(pos.yCoord().getUnitId(), pos.xCoord().getUnitId(), p);
}

inline
void CSingleLayerCell::setTopologies(I16x16Layer* topology)
{
	delete _Topologies;
	_Topologies = topology;
}

inline
void CSingleLayerCell::setHeightMap(I16x16Layer* heightMap)
{
	if (_HeightMap)	
		delete _HeightMap;
	_HeightMap = heightMap;
}

inline
void CSingleLayerCell::clearHeightMap()
{
	delete _HeightMap;
	_HeightMap = NULL;
}

//////////////////////////////////////////////////////////////////////////////

/**
 * A Multiple layer 16x16 cell (maximum 3 layers)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CMultiLayerCell
: public CRootCell
{
protected:
	class CCellLayer
	{
		friend class CMultiLayerCell; // for build features.
	public:
		CCellLinkage const& getCellLinkage(CMapPosition const& pos) const;
		uint8 getTopology(CMapPosition const& pos) const;
		I16x16Layer* getHeightMap() { return _HeightMap; }
		
	private:
		CCellLinkage& cellLinkage(CMapPosition const& pos);
		
	private:
		CCellLinkage _Layer[16*16];
		uint8 _Topology[16*16];
		I16x16Layer* _HeightMap;
	};
	
public:
	CMultiLayerCell(CWorldMap const& worldMapPtr);
	virtual ~CMultiLayerCell();
	
	CCellLinkage getCellLink(CWorldPosition const& wpos) const;
	uint32 nbUsedSlots(CMapPosition const& pos) const;
	sint32 maxUsedSlot(CMapPosition const& pos) const;
	bool isSlotUsed(CMapPosition const& pos, CSlot const& slot) const;
	uint getTopology(CWorldPosition const& wpos) const;
	sint getHeight(CWorldPosition const& wpos) const;
	
	virtual void serial(NLMISC::IStream& f);
	
	void setLinks(CWorldPosition const& wpos, CCellLinkage links);
	void setTopology(CWorldPosition const& wpos, uint topology);
	void setHeightMap(CSlot const& slot, I16x16Layer* heightMap);
	void clearHeightMap();
	
private:
	// each layer is allocated in memory
	// the 3 layers available
	CCellLayer* _Layers[3];
};

inline
CCellLinkage const& CMultiLayerCell::CCellLayer::getCellLinkage(CMapPosition const& pos) const
{
	return _Layer[pos.cellUnitFastIndex()];	//yCoord().getUnitId()][pos.xCoord().getUnitId()];
}

inline
uint8 CMultiLayerCell::CCellLayer::getTopology(CMapPosition const& pos) const
{
	return _Topology[pos.cellUnitFastIndex()];	//yCoord().getUnitId()][pos.xCoord().getUnitId()];
}

inline
CCellLinkage& CMultiLayerCell::CCellLayer::cellLinkage(CMapPosition const& pos)
{
	return _Layer[pos.cellUnitFastIndex()];	//yCoord().getUnitId()][pos.xCoord().getUnitId()];
}

inline
CMultiLayerCell::CMultiLayerCell(CWorldMap const& worldMapPtr)
: CRootCell(worldMapPtr)
{
	_Layers[0] = NULL;
	_Layers[1] = NULL;
	_Layers[2] = NULL;
}

inline
CMultiLayerCell::~CMultiLayerCell()
{
	delete [] _Layers[0];
	delete [] _Layers[1];
	delete [] _Layers[2];
}

inline
CCellLinkage CMultiLayerCell::getCellLink(CWorldPosition const& wpos) const
{
#ifdef NL_DEBUG
	nlassert(wpos.isValid());
#endif
	CCellLayer*	cellLayerPt=_Layers[wpos.slot()];	
	return (!cellLayerPt) ? CCellLinkage() : cellLayerPt->getCellLinkage(wpos);
}

inline
uint32 CMultiLayerCell::nbUsedSlots(CMapPosition const& pos) const
{
	uint32 ns = 0;
	for (uint32	slot=0; slot<3; ++slot)
	{
		CCellLayer*	cellLayerPt=_Layers[slot];
		if (cellLayerPt && cellLayerPt->cellLinkage(pos).used())
			++ns;
	}
	return	ns;
}

inline
sint32 CMultiLayerCell::maxUsedSlot(CMapPosition const& pos) const
{
	sint32 maxs = -1;
	for (uint32	slot=0; slot<3; ++slot)
	{
		CCellLayer*	cellLayerPt=_Layers[slot];
		if (cellLayerPt && cellLayerPt->cellLinkage(pos).used())
			maxs = slot;
	}
	return maxs;
}

inline
bool CMultiLayerCell::isSlotUsed(CMapPosition const& pos, CSlot const& slot) const
{
	if (!slot.isValid())
		return false;
	CCellLayer*	cellLayerPt = _Layers[slot.slot()];
	return (cellLayerPt && cellLayerPt->cellLinkage(pos).used());
}

inline
uint CMultiLayerCell::getTopology(CWorldPosition const& wpos) const
{
#ifdef NL_DEBUG
	nlassert(wpos.isValid());
#endif
	CCellLayer*	cellLayerPt = _Layers[wpos.slot()];
	return (!cellLayerPt) ? 0 : cellLayerPt->getTopology(wpos);
}

inline
sint CMultiLayerCell::getHeight(CWorldPosition const& wpos) const
{
#ifdef NL_DEBUG
	nlassert(wpos.isValid());
#endif
	CCellLayer*	cellLayerPt = _Layers[wpos.slot()];
	return (!cellLayerPt || !cellLayerPt->getHeightMap()) ? 0 : cellLayerPt->getHeightMap()->get(wpos.yCoord().getUnitId(), wpos.xCoord().getUnitId());
}

inline
void CMultiLayerCell::setLinks(CWorldPosition const& wpos, CCellLinkage links)
{
#ifdef NL_DEBUG
	nlassert(wpos.isValid());
#endif
	CCellLayer*	cellLayerPt = _Layers[wpos.slot()];
	if (!cellLayerPt)
	{
		cellLayerPt = new CCellLayer();
		_Layers[wpos.slot()] = cellLayerPt;
		
		for (uint i=0; i<16*16; ++i)
			cellLayerPt->_Topology[i] = 0;
		cellLayerPt->_HeightMap = NULL;
	}
#ifdef NL_DEBUG
	nlassert(cellLayerPt);
#endif
	cellLayerPt->cellLinkage(wpos)=links;
}

inline
void CMultiLayerCell::setTopology(CWorldPosition const& wpos, uint topology)
{
#ifdef NL_DEBUG
	nlassert(topology < 256);		
	nlassert(wpos.isValid());
#endif
	CCellLayer*	cellLayerPt = _Layers[wpos.slot()];
	
#ifdef NL_DEBUG
	nlassert(cellLayerPt);
#endif
	cellLayerPt->_Topology[wpos.cellUnitFastIndex()]=(uint8)topology;
}

inline
void CMultiLayerCell::setHeightMap(CSlot const& slot, I16x16Layer* heightMap)
{
#ifdef NL_DEBUG
	nlassert(slot.isValid());
#endif
	CCellLayer*	cellLayerPt = _Layers[slot.slot()];
	
#ifdef NL_DEBUG
	nlassert(cellLayerPt);
#endif
	if (cellLayerPt->_HeightMap)
		delete cellLayerPt->_HeightMap;
	cellLayerPt->_HeightMap = heightMap;
}

inline
void CMultiLayerCell::clearHeightMap()
{
	for (CSlot i(0); i.isValid(); ++i)
	{
		CCellLayer*	cellLayerPt = _Layers[i.slot()];
		if (cellLayerPt)
		{
			delete cellLayerPt->_HeightMap;
			cellLayerPt->_HeightMap=NULL;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

/**
 * A CSuperCell of basically 16x16 CCells
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CSuperCell
{
public:
	CSuperCell(CWorldMap const& worldMap);
	virtual ~CSuperCell();
	
	// get a const cell (might be NULL)
	CRootCell const* getRootCellCst(CMapPosition const& pos) const;
	// get a const cell (might be NULL)
	CRootCell* getRootCell(CMapPosition const& pos);
	
	void updateTopologyRef(CWorldMap* worldMap);
	void countCells(uint& compute, uint& white, uint& simple, uint& multi, uint& other) const;
	
	// set a cell
	void setRootCell(CMapPosition const& pos, CRootCell* cell);
	
	//
	// compute cell selectors
	//
	
	// get a cell
	CComputeCell* getComputeCell(CMapPosition const& pos);
	// get a const cell
	CComputeCell const* getComputeCell(CMapPosition const& pos) const;
	
	// get a slot in a cell
	CUnitSlot getUnitSlot(CWorldPosition const& wpos) const;
	// get a slot in super cell
	CUnitSlot& getUnitSlot(CWorldPosition const& wpos);
	TCellUnit& getCellUnit(CMapPosition const& pos);
	
	void serial(NLMISC::IStream& f);
	
	void markRootCell(uint32 const flag) const;
	
private:
	// a super cell is composed of 16x16 pointers to cells, initially NULL
	CRootCell* _Grid[16*16];
	CWorldMap const& _WorldMap;
};

inline
CSuperCell::CSuperCell(CWorldMap const& worldMap)
: _WorldMap(worldMap)
{
	for (uint i=0;i<16*16;i++)
		_Grid[i] = NULL;
}

inline
CSuperCell::~CSuperCell()
{
	for (uint i=0;i<16*16;i++)
		if (_Grid[i])
			delete _Grid[i];
}

inline
CRootCell const* CSuperCell::getRootCellCst(CMapPosition const& pos) const
{
	return _Grid[pos.rootCellFastIndex()];
}

inline
CRootCell* CSuperCell::getRootCell(CMapPosition const& pos)
{
	return _Grid[pos.rootCellFastIndex()];
}

inline
void CSuperCell::setRootCell(CMapPosition const& pos, CRootCell* cell)
{
	CRootCell*& rootCell = _Grid[pos.rootCellFastIndex()];
	if (rootCell==cell)
		return;
	delete rootCell;
	rootCell = cell;
}

inline
CComputeCell* CSuperCell::getComputeCell(CMapPosition const& pos)
{
	CRootCell*& rootCell = _Grid[pos.rootCellFastIndex()];
	if (!rootCell)
		rootCell = new CComputeCell(_WorldMap);
	
#ifdef NL_DEBUG
	nlassert(rootCell);
	nlassert(dynamic_cast<CComputeCell*>(rootCell));
#endif
	return static_cast<CComputeCell*>(rootCell);
}

inline
CComputeCell const* CSuperCell::getComputeCell(CMapPosition const& pos) const
{
	CRootCell const* const& rootCell = _Grid[pos.rootCellFastIndex()];
	if (!rootCell)
		return NULL;
#ifdef NL_DEBUG
	nlassert(dynamic_cast<CComputeCell const*>(rootCell));
#endif
	return static_cast<CComputeCell const*>(rootCell);
}

inline
CUnitSlot CSuperCell::getUnitSlot(CWorldPosition const& wpos) const
{
	CComputeCell const* cell = getComputeCell(wpos);
	return !cell ? CUnitSlot() : cell->getUnitSlotCst(wpos);
}

inline
CUnitSlot& CSuperCell::getUnitSlot(CWorldPosition const& wpos)
{
	CComputeCell* cell = getComputeCell(wpos);
	return cell->getUnitSlot(wpos);
}

inline
TCellUnit& CSuperCell::getCellUnit(CMapPosition const& pos)
{
	CComputeCell* cell = getComputeCell(pos);
	return cell->getCellUnit(pos);
}

inline
void CSuperCell::markRootCell(uint32 const flag) const
{
	for	(uint32 i=0; i<256; ++i)
	{
		if (_Grid[i])
			_Grid[i]->setFlag(flag);
	}
}

//////////////////////////////////////////////////////////////////////////////

/**
 * The A* Path structure, computed by the WorldMap
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CAStarPath
{
	friend class CWorldMap;
public:
	std::vector<CTopology::TTopologyRef> const& topologiesPath() const { return _TopologiesPath; }
	std::vector<CTopology::TTopologyRef>& topologiesPathForCalc() { return _TopologiesPath; }
	
	CWorldPosition const& getStartPos() const { return _Start; }
	void setStartPos(CWorldPosition const& pos) { _Start = pos; }
	
	CWorldPosition const& getEndPos() const { return _End; }
	void setEndPos(CWorldPosition const& pos) { _End = pos; }
	
private:
	std::vector<CTopology::TTopologyRef> _TopologiesPath;
	CWorldPosition _Start;
	CWorldPosition _End;
};

//////////////////////////////////////////////////////////////////////////////

/**
 * The Inside A* Path structure, computed by the WorldMap
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CInsideAStarPath	
{
	friend class CWorldMap;
public:
	CWorldPosition const& getStartPos() const { return _Start; }
	void setStartPos(CWorldPosition const& pos) { _Start = pos; }
	
	CWorldPosition const& getEndPos() const { return _End; }
	void setEndPos(CWorldPosition const& pos) { _End = pos; }	
	std::vector<CDirection>& getStepPathForCalc() { return _StepPath; }
	
private:
	CWorldPosition			_Start;
	CWorldPosition			_End;
	std::vector<CDirection>	_StepPath;
};

//////////////////////////////////////////////////////////////////////////////

/**
 * A template Heap implementation
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
template <typename T, typename V>
class CHeap
{
public:
	typedef std::pair<T, V> THeapNode;
	
public:
	CHeap();
	
	/// clear Heap
	void clear() { _Heap.resize(1); }
	
	/// test heap is empty or not
	bool empty() const { return _Heap.size() <= 1; }
	
	/// push a value in heap and sort it
	void push(T key, V const& value);
	
	/// pop the minimal value of the heap
	V pop();
	
	void backwardLeveling(uint index);
	
	void forwardLeveling(uint index);
	
	/// auto check of the internal heap state
	void check();
	
	uint32 size() { return _Heap.size(); }
	
	std::vector<THeapNode>& heapAsVector() { return _Heap; }
	
private:
	std::vector<THeapNode> _Heap;
};

template <typename T, typename V>
CHeap<T, V>::CHeap()
{
	// node 0 unused (daniel's trick)
	clear();
}

template <typename T, typename V>
void CHeap<T, V>::push(T key, V const& value)
{
	_Heap.push_back(THeapNode(key, value));
	
	backwardLeveling((uint)_Heap.size()-1);
}

template <typename T, typename V>
V CHeap<T, V>::pop()
{
#ifdef NL_DEBUG
	nlassert(_Heap.size() > 1);
#endif
	
	// return best value
	V ret = _Heap[1].second;
	
	// if only 1 node in heap, pop it and leave
	if (_Heap.size() == 2)
	{
		_Heap.pop_back();
		return ret;
	}
	
	// for more than 1 object, copy last at top and pop last
	_Heap[1] = _Heap.back();
	_Heap.pop_back();
	
	forwardLeveling	(1);
	return ret;
}

template <typename T, typename V>
void CHeap<T, V>::backwardLeveling(uint index)
{
	while (index != 1 && _Heap[index].first < _Heap[index/2].first)
	{
		swap(_Heap[index], _Heap[index/2]);
		index /= 2;
	}
}

template <typename T, typename V>
void CHeap<T, V>::forwardLeveling(uint index)
{
	while (true)
	{
		uint min_index = 2*index;
		
		// if object has no child, leave
		if (min_index > _Heap.size()-1)
			break;
		
		// choose left or right child
		if (min_index+1 <= _Heap.size()-1 && _Heap[min_index].first>_Heap[min_index+1].first)
			++min_index;
		
		// if swap needed, swap and step one more, else leave
		if (_Heap[index].first > _Heap[min_index].first)
		{
			swap(_Heap[index], _Heap[min_index]);
			index = min_index;
		}
		else
		{
			break;
		}
	}
}

template <typename T, typename V>
void CHeap<T, V>::check()
{
	for (uint index=1; index<=_Heap.size()-1; ++index)
	{
		if (2*index <= _Heap.size()-1)
			nlassert(_Heap[index].first <= _Heap[2*index].first);
		if (2*index+1 <= _Heap.size()-1)
			nlassert(_Heap[index].first <= _Heap[2*index+1].first);
	}
}

//////////////////////////////////////////////////////////////////////////////

/**
 * The world mapping
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */

class CWorldMap
{
	friend class RYPACSCRUNCH::CPacsCruncher;
	friend class CWorldPosition;
	friend class CTopology::CNeighbourLink;
public:
	CWorldMap();
	~CWorldMap();
	
	// serial -- beware, this method merges, use clear() before to load only one world map
	void serial(NLMISC::IStream& f);
	
	/// \name User Interface, path finding and complex moves
	// @{
	/// Finds an A* path
	bool findAStarPath(CWorldPosition const& start, CWorldPosition const& end, std::vector<CTopology::TTopologyRef>& path, TAStarFlag denyflags = Nothing) const;
	
	/// Finds an A* path
	bool findAStarPath(CTopology::TTopologyId const& start, CTopology::TTopologyId const& end, CAStarPath& path, TAStarFlag denyflags = Nothing) const;
	
	/// Finds an A* inside a topoly
	bool findInsideAStarPath(CWorldPosition const& start, CWorldPosition const& end, std::vector<CDirection>& stepPath, TAStarFlag denyflags = Nothing) const;
		
	/// Moves to given topology, returns false if failed
	bool moveTowards(CWorldPosition& pos, CTopology::TTopologyRef const& topology) const;
	
	/// Moves according a to a given path, returns false if failed
	bool move(CWorldPosition& pos, CAStarPath& path, uint& currentstep) const;
			
	/// Moves from a position to another
	bool move(CWorldPosition& pos, CMapPosition const& end, TAStarFlag const denyFlags) const;
	// @}
	
	bool customCheckDiagMove(CWorldPosition const& pos, CDirection const& direction, RYAI_MAP_CRUNCH::TAStarFlag denyFlags) const;
		
	// clean
	void clear();
	
	void setFlagOnPosAndRadius(CMapPosition const& pos, float radius, uint32 flag);
	
	//	to remove.
	CNeighbourhood neighbours(CWorldPosition const& wpos) const;
	
	/// Moves in neighbourhood
	bool move(CWorldPosition& pos, CDirection const& direction) const;
	
	/// Moves in neighbourhood (corner moves don't avoid collision)
	bool moveSecure(CWorldPosition& pos, CDirection const& direction, uint16 maskFlags = 0xffff) const;
	
	/// Moves in neighbourhood with diag test on both sides
	bool moveDiagTestBothSide(CWorldPosition& pos, CDirection const& direction) const;
	
	CTopology::TTopologyId getTopologyId(CWorldPosition const& wpos) const;
	
	// LastRemoved
	CTopology const& getTopologyNode(CTopology::TTopologyId const& id) const;
	
	CTopology& getTopologyNode(CTopology::TTopologyId const& id);
	
	CGridDirectionLayer const* getGridDirectionLayer(CWorldPosition const& pos, CTopology::TTopologyRef const& topologyRef) const;
	
	/// Get CWorldPosition from a CMapPosition and a CSlot
	CWorldPosition getWorldPosition(CMapPosition const& mapPos, CSlot const& slot) const;
	
	/**
	 * Get CWorldPosition from a CMapPosition and a TLevel
	 * Assumes that 0 is the highest level at the given CMapPosition, and greater the level is, lower the height is.
	 */
	CWorldPosition getWorldPosition(CMapPosition const& mapPos, TLevel level) const;
	
	// do not initialise a bot with the position & the world position (or u must assume that pos have no fraction).
	bool setWorldPosition(AITYPES::TVerticalPos verticalPos, CWorldPosition& wpos, CAIVector const& pos, CRootCell const* originCell = NULL) const;
	
	// Alternate setWorldPosition(), with real z instead of TVerticalPos, in millimeters
	bool setWorldPosition(sint32 z, CWorldPosition& wpos, CAIVector const& pos, CRootCell const* originCell = NULL) const;
	// Double version isn't tested !!! Verify it's the same than above sint32 version !
	bool setWorldPosition(double z, CWorldPosition& wpos, CAIVector const& pos, CRootCell const* originCell = NULL) const;
	
	CTopology::TTopologyRef	getTopologyRef(CTopology::TTopologyId const& id) const;
	
	void buildMasterTopo(bool allowWater, bool allowNogo);
	
	// checks motion layers
	void checkMotionLayer();
	
	void countSuperTopo();
	
	CRootCell const* getRootCellCst(CMapPosition const& pos) const;
	
	CSuperCell const* getSuperCellCst(CMapPosition const& pos) const;
	
	void getBounds(CMapPosition& min, CMapPosition& max);

	CWorldPosition getSafeWorldPosition(CMapPosition const& mapPos, CSlot const& slot) const;

protected:
	CWorldPosition getWorldPositionGeneration(CMapPosition const& mapPos, CSlot const& slot) const;
	
	
	/// Get a world position from a vector position
	template <class	T>
	CWorldPosition getWorldPosition(T const& pos) const
	{
		CSlot slot;
		CMapPosition const mapPos(pos);
		
		CRootCell const* cell = getRootCellCst(mapPos);
		if (cell)
		{
			double bd = 1.0e10;
			
			// find best slot
			for (uint32	s=0; s<3; ++s)
			{
				CSlot const sslot = CSlot(s);
				if (!cell->isSlotUsed(mapPos, sslot))
					continue;
				double const sh = cell->getHeight(CWorldPosition(cell,mapPos,sslot))*2.0 + 1.0;
				if (fabs(sh) < bd)
				{
					bd = fabs(sh);
					slot=sslot;
				}
			}
		}
		return CWorldPosition(cell,mapPos,slot);
	}
	
	
	//	only for generation.
	CSuperCell* getSuperCell(CMapPosition const& pos);
	
	CComputeCell* getComputeCell(CMapPosition const& pos);
	
	CRootCell* getRootCell(CMapPosition const& pos);
	
	bool exist(CMapPosition const& pos) const;
	
	uint32 nbUsedSlots(CMapPosition const& pos) const;
	
	sint32 maxUsedSlot(CMapPosition const& pos) const;
	
	bool isSlotUsed(CMapPosition const& pos, CSlot slot) const;
	
	//	used in build process.
	uint getTopology(CWorldPosition const& wpos) const;
	
	void countCells(uint& compute, uint& white, uint& simple, uint& multi, uint& other) const;
	
	// mutators
	void setRootCell(CMapPosition const& pos, CRootCell* cell);
	
	// clear height map
	void clearHeightMap();
	
	//
	// compute cell selectors
	//
	
	TCellUnit& getCellUnit(CMapPosition const& pos);
	
	CUnitSlot& getUnitSlot(CWorldPosition const& wpos);
	
	CComputeCell const* getComputeCellCst(CMapPosition const& pos) const;
	
	void resetUnitSlotNLink(CWorldPosition const& wpos) { getUnitSlot(wpos).cellLink().setNSlot(CSlot()); }
	void resetUnitSlotSLink(CWorldPosition const& wpos) { getUnitSlot(wpos).cellLink().setSSlot(CSlot()); }
	void resetUnitSlotWLink(CWorldPosition const& wpos) { getUnitSlot(wpos).cellLink().setWSlot(CSlot()); }
	void resetUnitSlotELink(CWorldPosition const& wpos) { getUnitSlot(wpos).cellLink().setESlot(CSlot()); }
	
	void resetUnitSlot(CWorldPosition const& wpos);
	
private:
	CSuperCell* _GridFastAccess[256*256];
	
public:
	/// \name Path finding related error handling
	// @{
	enum TFindAStarPathReason
	{
		FASPR_NO_REASON = -1,
		FASPR_NO_ERROR,
		FASPR_INVALID_START_POS,
		FASPR_INVALID_END_POS,
		FASPR_INVALID_START_TOPO,
		FASPR_INVALID_END_TOPO,
		FASPR_INCOMPATIBLE_POSITIONS,
		FASPR_NOT_FOUND
	};
	mutable TFindAStarPathReason _LastFASPReason;
	static std::string toString(TFindAStarPathReason reason);
	enum TFindInsideAStarPathReason
	{
		FIASPR_NO_REASON = -1,
		FIASPR_NO_ERROR,
		FIASPR_INVALID_START_POS,
		FIASPR_INVALID_END_POS,
		FIASPR_DIFFERENT_TOPO,
		FIASPR_NOT_FOUND
	};
	mutable TFindInsideAStarPathReason _LastFIASPReason;
	static std::string toString(TFindInsideAStarPathReason reason);
	// @}
};

inline
CWorldPosition::CWorldPosition(const CRootCell *cell, const CMapPosition &pos, const CSlot &slot) : CMapPosition(pos), CSlot(slot), _RootCell(cell)
{
	_cellLinkage=_RootCell->getCellLink(*this);
}

inline
CWorldPosition::CWorldPosition(const CRootCell *cell, const CMapPosition &pos, const CSlot &slot,bool generationOnly) : CMapPosition(pos), CSlot(slot), _RootCell(cell)
{
}

inline
CWorldPosition CWorldPosition::getPosS() const
{
	CWorldPosition wp(*this);
	wp.stepS();
	return wp;
}

inline
CWorldPosition CWorldPosition::getPosN() const
{
	CWorldPosition wp(*this);
	wp.stepN();
	return wp;
}

inline
CWorldPosition CWorldPosition::getPosE() const
{
	CWorldPosition wp(*this);
	wp.stepE();
	return wp;
}

inline
CWorldPosition CWorldPosition::getPosW() const
{
	CWorldPosition wp(*this);
	wp.stepW();
	return wp;
}

inline
void CWorldPosition::setPosS(CWorldPosition& pos) const
{
	pos = *this;
	pos.stepS();
}

inline
void CWorldPosition::setPosN(CWorldPosition& pos) const
{
	pos = *this;
	pos.stepN();
}

inline
void CWorldPosition::setPosE(CWorldPosition& pos) const
{
	pos = *this;
	pos.stepE();
}

inline
void CWorldPosition::setPosW(CWorldPosition& pos) const
{
	pos = *this;
	pos.stepW();
}

inline
void CWorldPosition::stepS()
{
#ifdef NL_DEBUG
	nlassert(_RootCell);
#endif
	setSlot(_cellLinkage.SSlot());
#ifdef NL_DEBUG
	nlassert(CSlot::isValid());
#endif
	
	if (CMapPosition::stepS())	//	check if we have to recalculate the RootCell ..
	{
		_RootCell = getRootCell()->getWorldMapPtr().getRootCellCst(*this);	// obtain the new _RootCell pointer
	}
	_cellLinkage = _RootCell->getCellLink(*this);
}

inline
void CWorldPosition::stepN()
{
#ifdef NL_DEBUG
	nlassert(_RootCell);
#endif
	setSlot(_cellLinkage.NSlot());
#ifdef NL_DEBUG
	nlassert(CSlot::isValid());
#endif
	
	if (CMapPosition::stepN())	//	check if we have to recalculate the RootCell ..
	{
		_RootCell = getRootCell()->getWorldMapPtr().getRootCellCst(*this);	// obtain the new _RootCell pointer
	}
	_cellLinkage = _RootCell->getCellLink(*this);
}

inline
void CWorldPosition::stepE()
{
#ifdef NL_DEBUG
	nlassert(_RootCell);
#endif
	setSlot(_cellLinkage.ESlot());
#ifdef NL_DEBUG
	nlassert(CSlot::isValid());
#endif
	
	if (CMapPosition::stepE())	//	check if we have to recalculate the RootCell ..
	{
		_RootCell = getRootCell()->getWorldMapPtr().getRootCellCst(*this);	// obtain the new _RootCell pointer
	}
	_cellLinkage = _RootCell->getCellLink(*this);
}

inline
void CWorldPosition::stepW()
{
#ifdef NL_DEBUG
	nlassert(_RootCell);
#endif
	setSlot(_cellLinkage.WSlot());
#ifdef NL_DEBUG
	nlassert(CSlot::isValid());
#endif
	
	if (CMapPosition::stepW())	//	check if we have to recalculate the RootCell ..
	{
		_RootCell = getRootCell()->getWorldMapPtr().getRootCellCst(*this);	// obtain the new _RootCell pointer
	}
	_cellLinkage = _RootCell->getCellLink(*this);
}

inline
bool CWorldPosition::moveS()
{
	if (getCellLinkage().isSSlotValid())
	{
		stepS();
		return true;
	}
	return false;
}

inline
bool CWorldPosition::moveN()
{
	if (getCellLinkage().isNSlotValid())
	{
		stepN();
		return true;
	}
	return false;
}

inline
bool CWorldPosition::moveE()
{
	if (getCellLinkage().isESlotValid())
	{
		stepE();
		return true;
	}
	return false;
}

inline
bool CWorldPosition::moveW()
{
	if (getCellLinkage().isWSlotValid())
	{
		stepW();
		return true;
	}
	return false;
}

inline
NLMISC::CVectorD CWorldPosition::getPosition() const
{
	NLMISC::CVectorD ret = toVectorD();
	CRootCell const* cell = getRootCell();
	if (cell)
		ret.z = cell->getHeight(*this)*2.0 + 1.0;
	return ret;
}

inline
sint32 CWorldPosition::getMetricHeight() const
{
	if (_RootCell)
		return _RootCell->getMetricHeight(*this);
	return 0;
}

inline
CTopology const& CTopology::TTopologyRef::getCstTopologyNode() const
{
#ifdef NL_DEBUG
	nlassert(_RootCell);
#endif
	return _RootCell->getCstTopologyNode(getTopologyIndex());
}

inline
CTopology::TTopologyRef::TTopologyRef(CWorldPosition const& pos, CRootCell const* rootCell)
: TTopologyId(pos, rootCell->getTopology(pos))
, _RootCell(rootCell)
{
#ifdef NL_DEBUG
	nlassert(rootCell);
#endif
	// TESTTOREMOVE TODO
#if !FINAL_VERSION
	nlassert(&_RootCell->getCstTopologyNode(getTopologyIndex())!=NULL);
#endif
}

inline
CTopology::TTopologyRef::TTopologyRef(CWorldPosition const& pos)
: TTopologyId(pos, pos.getRootCell()->getTopology(pos))
, _RootCell(pos.getRootCell())
{
	// TESTTOREMOVE TODO
#if !FINAL_VERSION
	nlassert(&_RootCell->getCstTopologyNode(getTopologyIndex())!=NULL);
#endif
}

inline
void CTopology::CNeighbourLink::updateTopologyRef(CWorldMap* worldMapPtr)
{
	_Ref.setRootCell(worldMapPtr->getRootCell(_Ref.getMapPosition()));
	// TESTTOREMOVE TODO
#if !FINAL_VERSION
	nlassert(&_Ref.getCstTopologyNode()!=NULL);
#endif
}

inline
CWorldMap::CWorldMap()
{
	for	(uint i=0; i<65536; ++i)
		_GridFastAccess[i]=NULL;
}

inline
CWorldMap::~CWorldMap()
{
	for	(uint i=0; i<65536; ++i)
		if (_GridFastAccess[i])
			delete _GridFastAccess[i];
}

inline
CTopology::TTopologyId CWorldMap::getTopologyId(CWorldPosition const& wpos) const
{
	CRootCell const* cell = getRootCellCst(wpos);
	return !cell ? CTopology::TTopologyId() : CTopology::TTopologyId(wpos, cell->getTopology(wpos));
}

inline
CTopology const& CWorldMap::getTopologyNode(CTopology::TTopologyId const& id) const
{
	CMapPosition pos = id.getMapPosition();
	uint topo = id.getTopologyIndex();
	
	CRootCell const* cell = getRootCellCst(pos);
#ifdef NL_DEBUG
	nlassert(cell);
#endif
	return cell->getCstTopologyNode(topo);
}

inline
CTopology& CWorldMap::getTopologyNode(CTopology::TTopologyId const& id)
{
	CMapPosition pos = id.getMapPosition();
	uint topo = id.getTopologyIndex();
	
	CRootCell* cell = getRootCell(pos);
#ifdef NL_DEBUG
	nlassert(cell);
#endif
	return cell->getTopologyNode(topo);
}

inline
CGridDirectionLayer const* CWorldMap::getGridDirectionLayer(CWorldPosition const& pos, CTopology::TTopologyRef const& topologyRef) const
{
	if (!pos.isValid())
		return NULL;
	
	CMapPosition tpos = topologyRef.getMapPosition();
	
	sint dx = pos.xCoord().getFullCellId()-tpos.xCoord().getFullCellId();
	sint dy = pos.yCoord().getFullCellId()-tpos.yCoord().getFullCellId();
	
	if (abs(dx) > 1 || abs(dy) > 1)
		return NULL;
	
	CTopology const& node = topologyRef.getCstTopologyNode();
	
	if (!node.DirectionMap)
		return	NULL;
	
	CDirectionLayer* directionLayer = node.DirectionMap->Layers[pos.slot()];
	if (!directionLayer)
		return NULL;
	
	return directionLayer->getGridLayer(dy+1,dx+1);
}

inline
CWorldPosition CWorldMap::getWorldPosition(CMapPosition const& mapPos, CSlot const& slot) const
{
	return CWorldPosition(getRootCellCst(mapPos), mapPos, slot);
}

inline
CWorldPosition CWorldMap::getWorldPosition(CMapPosition const& mapPos, TLevel level) const
{
	CRootCell const* cell = getRootCellCst(mapPos);
	
	std::vector<sint32>	slots;
	slots.reserve(4);
	
	// go through all slots and get their height
	for (uint s=0; s<3; ++s)
	{
		CSlot slot(s);
		
		if (!cell->isSlotUsed(mapPos, slot))
			continue;
		
		CWorldPosition wPos = CWorldPosition(cell, mapPos, slot);
		slots.push_back((cell->getHeight(wPos) << 2) + s);
	}
	
	// sort them, from the lowest to the heightest
	std::sort(slots.begin(), slots.end());
	
	// get heightest slot
	level = (RYAI_MAP_CRUNCH::TLevel)(slots.size()-1) - level;
	
	// if slot exists, return it or invalid position
	return (level < 0 && level >= (sint)slots.size()) ? CWorldPosition() : CWorldPosition(cell, mapPos, CSlot(slots[level]&3));
}

inline
bool CWorldMap::setWorldPosition(AITYPES::TVerticalPos verticalPos, CWorldPosition& wpos, CAIVector const& pos, CRootCell const* originCell) const
{
	CSlot slot;
	CMapPosition const mapPos(pos);
	
	CRootCell const* cell = originCell ? originCell : getRootCellCst(mapPos);
	if (!cell)
	{
		wpos = CWorldPosition(cell, mapPos, slot, true);
		// addon (not agree but no choice).
		return false;
	}
	
	std::map<double, CSlot>	orderedSlots;
	// find best slot
	for (uint32	s=0; s<3; ++s)
	{
		if (!cell->isSlotUsed(mapPos, CSlot(s)))
			continue;
		
		CSlot const sslot = CSlot(s);
		double const sh = cell->getHeight(CWorldPosition(cell,mapPos,sslot));
		orderedSlots.insert(std::make_pair(sh, sslot));
	}
	
	if (!orderedSlots.empty())
	{
		if (verticalPos == AITYPES::vp_auto)
		{
			double h = orderedSlots.rbegin()->first;
			slot = orderedSlots.rbegin()->second;
		}
		else if (verticalPos == AITYPES::vp_upper)
		{
			double h = orderedSlots.rbegin()->first;
			slot = orderedSlots.rbegin()->second;
		}
		else if (verticalPos == AITYPES::vp_lower)
		{
			double h = orderedSlots.begin()->first;
			slot = orderedSlots.begin()->second;
		}
		else if (verticalPos == AITYPES::vp_middle)
		{
			if (orderedSlots.size() > 1)
			{
				orderedSlots.erase(orderedSlots.rbegin()->first);
				double h = orderedSlots.rbegin()->first;
				slot = orderedSlots.rbegin()->second;
			}
			else
			{
				double h = orderedSlots.begin()->first;
				slot = orderedSlots.begin()->second;
			}
		}
		else
		{
			nlwarning("invalid vertical pos type !");
		}
	}
	
	if (!slot.isValid())
	{
		wpos = CWorldPosition(cell,mapPos,slot,true);
		return false;
	}
	
	wpos = CWorldPosition(cell,mapPos,slot);
	return true;
}

inline
CTopology::TTopologyRef	CWorldMap::getTopologyRef(CTopology::TTopologyId const& id) const
{
	return CTopology::TTopologyRef(id, const_cast<CRootCell*>(getRootCellCst(id.getMapPosition())));
}

inline
CRootCell const* CWorldMap::getRootCellCst(CMapPosition const& pos) const
{
	CSuperCell const* scell = getSuperCellCst(pos);
	return !scell ? NULL : scell->getRootCellCst(pos);
}

inline
CSuperCell const* CWorldMap::getSuperCellCst(CMapPosition const& pos) const
{
	return _GridFastAccess[pos.superCellFastIndex()];
}

inline
CWorldPosition CWorldMap::getWorldPositionGeneration(CMapPosition const& mapPos, CSlot const& slot) const
{
	return CWorldPosition(getRootCellCst(mapPos),mapPos,slot,true);	//	without assuring cellLink is valid .. :(
}

inline
CWorldPosition CWorldMap::getSafeWorldPosition(CMapPosition const& mapPos, CSlot const& slot) const
{
	CRootCell const* cell = getRootCellCst(mapPos); 
	return (cell && cell->isSlotUsed(mapPos, slot)) ? CWorldPosition(getRootCellCst(mapPos),mapPos,slot) : CWorldPosition();	//	without assuring cellLink is valid .. :(
}

inline
CSuperCell* CWorldMap::getSuperCell(CMapPosition const& pos)
{
	CSuperCell*	superCellPtr = _GridFastAccess[pos.superCellFastIndex()];
	
	if (!superCellPtr)
		_GridFastAccess[pos.superCellFastIndex()] = superCellPtr=new CSuperCell(*this);
	
#ifdef NL_DEBUG
	nlassert(superCellPtr);
#endif
	return superCellPtr;
}

inline
CComputeCell* CWorldMap::getComputeCell(CMapPosition const& pos)
{
	return getSuperCell(pos)->getComputeCell(pos);
}

inline
CRootCell* CWorldMap::getRootCell(CMapPosition const& pos)
{
	CSuperCell* scell = getSuperCell(pos);
	return !scell ? NULL : scell->getRootCell(pos);
}

inline
bool CWorldMap::exist(CMapPosition const& pos) const
{
	return getRootCellCst(pos)!=NULL;
}

inline
uint32 CWorldMap::nbUsedSlots(CMapPosition const& pos) const
{
	CRootCell const* cell = getRootCellCst(pos);
	return !cell ? 0:cell->nbUsedSlots(pos);
}

inline
sint32 CWorldMap::maxUsedSlot(CMapPosition const& pos) const
{
	CRootCell const* cell = getRootCellCst(pos);
	return !cell ? -1 : cell->maxUsedSlot(pos);
}

inline
bool CWorldMap::isSlotUsed(CMapPosition const& pos, CSlot slot) const
{
	CRootCell const* cell = getRootCellCst(pos);
	return !cell ? false : cell->isSlotUsed(pos, slot);
}

inline
uint CWorldMap::getTopology(CWorldPosition const& wpos) const
{
	CRootCell const* cell = wpos.getRootCell();	//getRootCellCst(wpos);
	return !cell ? 0 : cell->getTopology(wpos);
}

inline
void CWorldMap::setRootCell(CMapPosition const& pos, CRootCell* cell)
{
	getSuperCell(pos)->setRootCell(pos, cell);
}

inline
TCellUnit& CWorldMap::getCellUnit(CMapPosition const& pos)
{
	return getComputeCell(pos)->getCellUnit(pos);
}

inline
CUnitSlot& CWorldMap::getUnitSlot(CWorldPosition const& wpos)
{
	return getComputeCell(wpos)->getUnitSlot(wpos);
}

inline
CComputeCell const* CWorldMap::getComputeCellCst(CMapPosition const& pos) const
{
	CSuperCell const* scell = getSuperCellCst(pos);
	return !scell ? NULL : scell->getComputeCell(pos);
}

inline
void CWorldMap::resetUnitSlot(CWorldPosition const& wpos)
{
	CUnitSlot& us = getUnitSlot(wpos);
	
	if (us.getCellLink().isNSlotValid()) resetUnitSlotSLink(wpos.getPosN());
	if (us.getCellLink().isSSlotValid()) resetUnitSlotNLink(wpos.getPosS());
	if (us.getCellLink().isWSlotValid()) resetUnitSlotELink(wpos.getPosW());
	if (us.getCellLink().isESlotValid()) resetUnitSlotWLink(wpos.getPosE());
	
	us.reset();
}

inline
std::string CWorldMap::toString(TFindAStarPathReason reason)
{
	switch (reason)
	{
	case FASPR_NO_ERROR:
		return "FASPR_NO_ERROR";
	case FASPR_INVALID_START_POS:
		return "FASPR_INVALID_START_POS";
	case FASPR_INVALID_END_POS:
		return "FASPR_INVALID_END_POS";
	case FASPR_INVALID_START_TOPO:
		return "FASPR_INVALID_START_TOPO";
	case FASPR_INVALID_END_TOPO:
		return "FASPR_INVALID_END_TOPO";
	case FASPR_INCOMPATIBLE_POSITIONS:
		return "FASPR_INCOMPATIBLE_POSITIONS";
	case FASPR_NOT_FOUND:
		return "FIASPR_NOT_FOUND";
	default:
		return "UnknownReason(" + NLMISC::toString((int)reason) + ")";
	}
}

inline
std::string CWorldMap::toString(TFindInsideAStarPathReason reason)
{
	switch (reason)
	{
	case FIASPR_NO_ERROR:
		return "FIASPR_NO_ERROR";
	case FIASPR_INVALID_START_POS:
		return "FIASPR_INVALID_START_POS";
	case FIASPR_INVALID_END_POS:
		return "FIASPR_INVALID_END_POS";
	case FIASPR_DIFFERENT_TOPO:
		return "FIASPR_DIFFERENT_TOPO";
	case FIASPR_NOT_FOUND:
		return "FIASPR_NOT_FOUND";
	default:
		return "UnknownReason(" + NLMISC::toString((int)reason) + ")";
	}
}

}

//////////////////////////////////////////////////////////////////////////////

class CTimeEstimator
{
public:
	CTimeEstimator(uint total);
	
	void step(char const* str);
	
private:
	NLMISC::TTime	_StartTime;
	NLMISC::TTime	_LastTime;
	
	uint			_TotalCounter;
	uint			_CurrentCounter;
	uint			_LastCounter;
	
	double			_AverageSpeed;
	
	std::string	secToString(sint sec);
};

inline
CTimeEstimator::CTimeEstimator(uint total)
: _TotalCounter(total)
, _CurrentCounter(0)
, _LastCounter(0)
, _AverageSpeed(-1.0)
{
	_StartTime = NLMISC::CTime::getLocalTime();
	_LastTime = _StartTime;
}

inline
void CTimeEstimator::step(char const* str)
{
	++_CurrentCounter;
	
	NLMISC::TTime currentTime = NLMISC::CTime::getLocalTime();
	
	if (currentTime-_LastTime > 2000)
	{
		NLMISC::TTime	dt = currentTime-_LastTime;
		double			speed = 1000.0*(_CurrentCounter-_LastCounter)/dt;
		
		_AverageSpeed = (_AverageSpeed < 0.0 ? speed : _AverageSpeed*0.98 + speed*0.02);
		
		uint			remaining = _TotalCounter-_CurrentCounter;
		
		NLMISC::TTime	ellapsedTime = currentTime-_StartTime;
		NLMISC::TTime	remainingTime = (NLMISC::TTime)(remaining*1000/_AverageSpeed);
		
		double			percent = 100.0*_CurrentCounter/_TotalCounter;
		uint			maxpercent = (percent > 100.0 ? 100 : (uint)percent);
		uint			linesize = maxpercent/5;
		
		nlinfo("%s: %.1f%% [ellapsed:%s, togo:%s, total:%s]", str, percent, secToString((uint)(ellapsedTime/1000)).c_str(), secToString((uint)(remainingTime/1000)).c_str(), secToString((uint)((remainingTime+ellapsedTime)/1000)).c_str());
		
		_LastCounter = _CurrentCounter;
		_LastTime = currentTime;
	}
}

inline
std::string	CTimeEstimator::secToString(sint sec)
{
	return NLMISC::toString(sec/60)+"m"+NLMISC::toString(sec%60)+"s";
}

#endif
