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
#include "game_share/utils.h"
#include "world_map.h"

//extern bool simulateBug(int bugId);

using namespace std;
using namespace NLMISC;

namespace RYAI_MAP_CRUNCH
{

NL_BEGIN_STRING_CONVERSION_TABLE (TAStarFlag)
	NL_STRING_CONVERSION_TABLE_ENTRY(Nothing)
	NL_STRING_CONVERSION_TABLE_ENTRY(Interior)
	NL_STRING_CONVERSION_TABLE_ENTRY(Water)
	NL_STRING_CONVERSION_TABLE_ENTRY(NoGo)
	NL_STRING_CONVERSION_TABLE_ENTRY(WaterAndNogo)
	NL_STRING_CONVERSION_TABLE_ENTRY(GroundFlags)
NL_END_STRING_CONVERSION_TABLE(TAStarFlag, AStarFlagConversion, Nothing)

const std::string& toString(TAStarFlag flag)
{
	return AStarFlagConversion.toString(flag);
}

TAStarFlag toAStarFlag(const std::string& str)
{
	return AStarFlagConversion.fromString(str);
}


//////////////////////////////////////////////////////////////////////////////
// Helper classes and data                                                  //
//////////////////////////////////////////////////////////////////////////////

const struct CDirection::CDirectionData CDirection::directionDatas[] =
{
	{ +1,  0, ORTHO_COST},
	{ +1, +1, DIAG_COST},
	{  0, +1, ORTHO_COST},
	{ -1, +1, DIAG_COST},
	{ -1,  0, ORTHO_COST},
	{ -1, -1, DIAG_COST},
	{  0, -1, ORTHO_COST},
	{ +1, -1, DIAG_COST},
	{  0,  0, NO_COST}
};

//	Enum vals ..
//	5 6 7
//	4 8 0
//	3 2 1
const CDirection::TDirection	CDirection::table[]	=
{
	CDirection::SW, CDirection::S,			CDirection::SE,
	CDirection::W,  CDirection::UNDEFINED,	CDirection::E,
	CDirection::NW, CDirection::N,			CDirection::NE
};

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

class CABaseStarNode
{
public:
	CABaseStarNode(uint father, float distance, bool open);

	void setOpen(bool open) { _Open = open; }
	bool isOpened() const { return _Open; }
	float getDistance() const { return _Distance; }
	uint getFather() const { return _Father; }

private:
	uint  _Father; ///< Parent node in the path from the start position
	float _Distance;
	bool  _Open; ///< Is the node in the OPEN or CLOSED set?
};

inline
CABaseStarNode::CABaseStarNode(uint father, float distance, bool open)
: _Father(father)
, _Distance(distance)
, _Open(open)
{
}

//////////////////////////////////////////////////////////////////////////////
// CAStarHeapNode                                                           //
//////////////////////////////////////////////////////////////////////////////

class CAStarHeapNode : public CABaseStarNode
{
public:
	explicit CAStarHeapNode(CTopology::TTopologyRef Ref, uint Father, float Distance, bool Open);

	CTopology::TTopologyRef const& getRef() const { return _Ref; }

private:
	CTopology::TTopologyRef	_Ref;
};

inline
CAStarHeapNode::CAStarHeapNode(CTopology::TTopologyRef Ref, uint Father, float Distance, bool Open)
: CABaseStarNode(Father, Distance, Open)
, _Ref(Ref)
{
}

//////////////////////////////////////////////////////////////////////////////
// CAStarNode                                                               //
//////////////////////////////////////////////////////////////////////////////

class CAStarNode
{
public:
	CAStarNode() { }
	explicit CAStarNode(CWorldPosition const& pos) :
		_x(pos.xCoord().getUnitId()) , _y(pos.yCoord().getUnitId()), _slot(pos.slot())
		{ }

	CAStarNode(const CAStarNode & other) :
		_x(other._x), _y(other._y), _slot(other._slot)
		{ }

	void updateMapPosition(CMapPosition& _mapPos) const;

	bool operator==(CAStarNode const& other) const;
	bool operator!=(CAStarNode const& other) const;
	bool operator<(CAStarNode const& other) const;

	CSlot const& slot() const { return _slot; }

private:
	uint8 _x;
	uint8 _y;
	CSlot _slot;
};

inline
void CAStarNode::updateMapPosition(CMapPosition& _mapPos) const
{
	_mapPos.setUnitId(_x, _y);
}

inline
bool CAStarNode::operator==(CAStarNode const& other) const
{
	return _x==other._x && _y==other._y && _slot==other._slot;
}

inline
bool CAStarNode::operator!=(CAStarNode const& other) const
{
	return _x!=other._x || _y!=other._y || _slot!=other._slot;
}

inline
bool CAStarNode::operator<(CAStarNode const& other) const
{
	if (_x!=other._x)
		return _x<other._x;
	if (_y!=other._y)
		return _y<other._y;
	return _slot<other._slot;
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

class	CInsideAStarHeapNode	:	public	CABaseStarNode
{
public:
	friend	class	CAStarNode;

	explicit	CInsideAStarHeapNode(const	CAStarNode	&node, uint Father, CDirection Direction, float Distance, bool Open) : CABaseStarNode(Father,Distance,Open), _Direction(Direction), _Node(node)
	{
	}

	inline	const	CDirection	&getDirection()	const
	{
		return	_Direction;
	}

	inline	const	CAStarNode	&getNode()	const
	{
		return	_Node;
	}

private:
	CDirection	_Direction;
	CAStarNode	_Node;
};


//////////////////////////////////////////////////////////////////////////////
// CDirectionLayer                                                          //
//////////////////////////////////////////////////////////////////////////////

void CDirectionLayer::serial(NLMISC::IStream& f)
{
	uint i, j;
	for (i=0; i<3; ++i)
	{
		for (j=0; j<3; ++j)
		{
			if (f.isReading())
			{
				delete Grid[i][j];
				Grid[i][j] = NULL;
			}

			bool	present = (Grid[i][j] != NULL);
			f.serial(present);

			if (present)
			{
				if (f.isReading())
					Grid[i][j] =	I16x16Layer::load(f);
				else
					I16x16Layer::save(f, Grid[i][j]);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// CDirectionMap                                                            //
//////////////////////////////////////////////////////////////////////////////

void CDirectionMap::serial(NLMISC::IStream& f)
{
	uint i;
	for (i=0; i<3; ++i)
	{
		if (f.isReading())
		{
			delete Layers[i];
			Layers[i] = NULL;
		}

		bool	present = (Layers[i] != NULL);
		f.serial(present);

		if (present)
		{
			if (f.isReading())
				Layers[i] = new CDirectionLayer();
			f.serial(*Layers[i]);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// CRootCell                                                                //
//////////////////////////////////////////////////////////////////////////////

CRootCell* CRootCell::load(NLMISC::IStream& f, CWorldMap const& worldMap)
{
	TCellType	type = Compute;
	CRootCell	*result = NULL;

	f.serialEnum(type);

	switch (type)
	{
	case Compute:
		result = new CComputeCell(worldMap);
		static_cast<CComputeCell*>(result)->serial(f);
		break;
	case White:
		result = new CWhiteCell(worldMap);
		static_cast<CWhiteCell*>(result)->serial(f);
		break;
	case SingleLayer:
		result = new CSingleLayerCell(worldMap);
		static_cast<CSingleLayerCell*>(result)->serial(f);
		break;
	case MultiLayer:
		result = new CMultiLayerCell(worldMap);
		static_cast<CMultiLayerCell*>(result)->serial(f);
		break;
	default:
		nlassert(false);
		nlwarning("Unknown type of cell %d to load, abort", type);
		return result;
		break;
	}

	//	allow us to optimize access.
	f.serialCont(result->_TopologiesNodes);

	return result;
}

void CRootCell::save(NLMISC::IStream& f, CRootCell* cell)
{
	if (dynamic_cast<CComputeCell*>(cell) != NULL)
	{
		TCellType	type = Compute;
		f.serialEnum(type);
		static_cast<CComputeCell*>(cell)->serial(f);
	}
	else if (dynamic_cast<CWhiteCell*>(cell) != NULL)
	{
		TCellType	type = White;
		f.serialEnum(type);
		static_cast<CWhiteCell*>(cell)->serial(f);
	}
	else if (dynamic_cast<CSingleLayerCell*>(cell) != NULL)
	{
		TCellType	type = SingleLayer;
		f.serialEnum(type);
		static_cast<CSingleLayerCell*>(cell)->serial(f);
	}
	else if (dynamic_cast<CMultiLayerCell*>(cell) != NULL)
	{
		TCellType	type = MultiLayer;
		f.serialEnum(type);
		static_cast<CMultiLayerCell*>(cell)->serial(f);
	}
	else
	{
		nlassert(false);
		nlwarning("Unknown type of cell to save, abort");
		return;
	}

	f.serialCont(cell->_TopologiesNodes);
}

//////////////////////////////////////////////////////////////////////////////
// CComputeCell                                                             //
//////////////////////////////////////////////////////////////////////////////

void CComputeCell::serial(NLMISC::IStream& f)
{
	// Version
	// 0: initial version
	uint	version = f.serialVersion(0);

	for (uint32 i=0; i<16*16; ++i)
		for (uint32 k=0; k<3; ++k)
			f.serial(_Grid[i][k]);
}

//////////////////////////////////////////////////////////////////////////////
// CSingleLayerCell                                                         //
//////////////////////////////////////////////////////////////////////////////

bool	CSingleLayerCell::_Initialized = false;
uint16	CSingleLayerCell::_MaskMap[16];

void CSingleLayerCell::serial(NLMISC::IStream& f)
{
	f.serialCheck((uint16)'SL');

	uint i;
	for (i=0; i<16; ++i)
		f.serial(_Map[i]);

	f.serial(_SLinks);
	f.serial(_NLinks);
	f.serial(_ELinks);
	f.serial(_WLinks);

	if (f.isReading())
	{
		delete _Topologies;
		delete _HeightMap;
		_Topologies = I16x16Layer::load(f);
		_HeightMap = I16x16Layer::load(f);
	}
	else
	{
		I16x16Layer::save(f, _Topologies);
		I16x16Layer::save(f, _HeightMap);
	}
}

//////////////////////////////////////////////////////////////////////////////
// CMultiLayerCell                                                          //
//////////////////////////////////////////////////////////////////////////////

void CMultiLayerCell::serial(NLMISC::IStream& f)
{
	f.serialCheck((uint16)'ML');

	uint slot;

	for (slot=0; slot<3; ++slot)
	{
		// delete layer if any previously
		if (f.isReading())
		{
			if (_Layers[slot] != NULL)
				delete _Layers[slot]->_HeightMap;
			delete _Layers[slot];
			_Layers[slot] = NULL;
		}

		bool	present = (_Layers[slot] != NULL);
		f.serial(present);

		if (present)
		{
			if (f.isReading())
			{
				_Layers[slot] = new CCellLayer();
				_Layers[slot]->_HeightMap = I16x16Layer::load(f);
			}
			else
			{
				I16x16Layer::save(f, _Layers[slot]->_HeightMap);
			}

			nlassert(_Layers[slot] != NULL);

			for (uint32 i=0; i<16*16; ++i)
			{
				f.serial(_Layers[slot]->_Layer[i]);
				f.serial(_Layers[slot]->_Topology[i]);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// CSuperCell                                                               //
//////////////////////////////////////////////////////////////////////////////

void CSuperCell::serial(NLMISC::IStream& f)
{
	// Version
	// 0: initial version
	uint	version = f.serialVersion(0);

	if (f.isReading())
	{
		for (uint32 i=0; i<16*16; ++i)
		{
			bool present;
			f.serial(present);

			if (_Grid[i] != NULL)
				delete _Grid[i];

			if (present)
				_Grid[i] = CRootCell::load(f,_WorldMap);
		}
	}
	else
	{
		for (uint32 i=0; i<16*16; ++i)
		{
			bool present = (_Grid[i] != NULL);
			f.serial(present);

			if (present)
				CRootCell::save(f, _Grid[i]);
		}
	}
}

void CSuperCell::updateTopologyRef(CWorldMap* worldMap)
{
	for (uint32 i=0; i<16*16; ++i)
	{
		if (_Grid[i])
			_Grid[i]->updateTopologyRef	(worldMap);
	}
}

void CSuperCell::countCells(uint& compute, uint& white, uint& simple, uint& multi, uint& other) const
{
	for (uint32 i=0; i<16*16; ++i)
	{
		if (!_Grid[i])
			continue;
		if (dynamic_cast<const CWhiteCell*>(_Grid[i]) != NULL)
			++white;
		else if (dynamic_cast<const CSingleLayerCell*>(_Grid[i]) != NULL)
			++simple;
		else if (dynamic_cast<const CComputeCell*>(_Grid[i]) != NULL)
			++compute;
		else if (dynamic_cast<const CMultiLayerCell*>(_Grid[i]) != NULL)
			++multi;
		else
			++other;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CWorldMap                                                                //
//////////////////////////////////////////////////////////////////////////////

void CWorldMap::getBounds(CMapPosition& min, CMapPosition& max)
{
	uint	i, j;
	uint	mini = 256, maxi = 0, minj = 256, maxj = 0;

	for (i=0; i<256; ++i)
	{
		for (j=0; j<256; ++j)
		{
			if (_GridFastAccess[i*256+j])
			{
				if (i < mini)	mini = i;
				if (i > maxi)	maxi = i;
				if (j < minj)	minj = j;
				if (j > maxj)	maxj = j;
			}
		}
	}

	min	=	CMapPosition(CMapCoord(minj, 0, 0),		CMapCoord(mini, 0, 0)	);
	max	=	CMapPosition(CMapCoord(maxj+1, 0, 0),	CMapCoord(maxi+1, 0, 0)	);
}

void	CWorldMap::clear()
{
	for	(uint	i=0;i<65536;i++)
	{
		if (_GridFastAccess[i])
		{
			delete	_GridFastAccess[i];
			_GridFastAccess[i]=NULL;
		}

	}

}


void	CWorldMap::serial(NLMISC::IStream &f)
{
	f.serialCheck(NELID("WMAP"));

	// Version
	// 0: initial version
	uint	version = f.serialVersion(0);

	if (f.isReading())
	{
		uint32 i;
		for	(i=0;i<65536;i++)
		{
			bool	present;
			f.serial(present);

			if (present)
			{
				CSuperCell	*scell = _GridFastAccess[i];
				if (!scell)
					_GridFastAccess[i]	=	scell	=new	CSuperCell(*this);
				f.serial(*scell);
			}

		}

		//	made to update RootCell pointers in TTopologyRef ..
		for	(i=0;i<65536;i++)
		{
			CSuperCell	*scell = _GridFastAccess[i];
			if (scell)
				scell->updateTopologyRef	(this);
		}

		//	made to calculate some random pos ..
		{
			CMapPosition	min, max;
			getBounds(min, max);
			CMapPosition	scan, scanline;
			NLMISC::CRandom	random;

			for (scan = min; scan.y() != max.y(); scan = scan.stepCell(0, 1))
			{
				for (scanline = scan; scanline.x()	!= max.x(); scanline = scanline.stepCell(1, 0))
				{
					CRootCell	*rootCell=getRootCell(scanline);

					if (!rootCell)
						continue;

					CMapPosition	pos(scanline.x(),0xffff0000|scanline.y());

					uint	ind=0;
					uint	maxTries=256;

					for (;ind<4 && maxTries>0;maxTries--)
					{
						CWorldPosition	wpos;

						uint	i=	uint32(random.rand()) & 0xf;
						uint	j=	uint32(random.rand()) & 0xf;
#ifdef NL_DEBUG
						nlassert(i<16 && j<16);
#endif
						pos.setUnitId(i,j);
						CAIVector	vecPos=CAIVector(pos);
						if (setWorldPosition	(AITYPES::vp_auto, wpos, vecPos))
						{
#ifdef NL_DEBUG
							nlassert(wpos.getRootCell()==rootCell);
							nlassert(wpos.y()<=0 && wpos.x()>=0);
#endif
							rootCell->setWorldPosition(wpos, ind);
							ind++;
						}

					}

					// if we have found some valid positions but not all, fill the array with the last pos found.
					if (ind<4 && ind>0)
					{
						while (ind<4)
						{
							rootCell->setWorldPosition(rootCell->getWorldPosition(ind-1), ind);
							ind++;
						}

					}

				}

			}

		}

	}
	else
	{
		for	(uint32	i=0;i<65536;i++)
		{
			bool	present = (_GridFastAccess[i]!=NULL);
			f.serial(present);

			if (present)
			{
				//nldebug("Save SuperCell %d/%d", i, j);
				f.serial(*(_GridFastAccess[i]));
			}

		}

	}

}

void	CWorldMap::setFlagOnPosAndRadius(const	CMapPosition	&pos,float	radius,	uint32	flag)
{
	float	minx=pos.x()-radius;
	float	maxx=pos.x()+radius;

	float	miny=pos.y()-radius;
	float	maxy=pos.y()+radius;

	const	float	radius2=radius*radius;

	for (float	ty=miny;ty<=maxy;ty++)
	{
		const	float	dy=ty-pos.y();
		for (float	tx=minx;tx<=maxx;tx++)
		{
			const	float	dx=tx-pos.x();

			if ((dy*dy+dx*dx)>radius2)
				continue;

			CRootCell	*rootCell=getRootCell(CMapPosition((int)tx,(int)ty));
			if (!rootCell)
				continue;
			rootCell->setFlag(flag);
		}

	}

}

void	CWorldMap::countCells(uint &compute, uint &white, uint &simple, uint &multi, uint &other) const
{
	for (uint32 i=0; i<65536; ++i)
	{
		if (_GridFastAccess[i])
			_GridFastAccess[i]->countCells(compute, white, simple, multi, other);
	}

//		uint	i, j;
//		for (i=0; i<256; ++i)
//			for (j=0; j<256; ++j)
//				if (_Grid[i][j] != NULL)
//					_Grid[i][j]->countCells(compute, white, simple, multi, other);
}

	//
	CNeighbourhood	CWorldMap::neighbours(const CWorldPosition &wpos) const
	{
		CNeighbourhood			neighbs;
		const	CCellLinkage	lnks=wpos.getCellLinkage();

		if (lnks.isSSlotValid())
		{
			neighbs.set(CDirection::S);

			const	CCellLinkage	&nlnk = wpos.getPosS().getCellLinkage();	//CWorldPosition(wpos.getPosS(),lnks.NSlot()));
			if (nlnk.isESlotValid())
				neighbs.set(CDirection::SE);
			if (nlnk.isWSlotValid())
				neighbs.set(CDirection::SW);
		}

		if (lnks.isNSlotValid())
		{
			neighbs.set(CDirection::N);

			const	CCellLinkage	&slnk = wpos.getPosN().getCellLinkage();
			if (slnk.isESlotValid())
				neighbs.set(CDirection::NE);
			if (slnk.isWSlotValid())
				neighbs.set(CDirection::NW);
		}

		if (lnks.isESlotValid())
		{
			neighbs.set(CDirection::E);

			const	CCellLinkage	&elnk = wpos.getPosE().getCellLinkage();
			if (elnk.isSSlotValid())
				neighbs.set(CDirection::SE);
			if (elnk.isNSlotValid())
				neighbs.set(CDirection::NE);
		}

		if (lnks.isWSlotValid())
		{
			neighbs.set(CDirection::W);

			const	CCellLinkage	&wlnk = wpos.getPosW().getCellLinkage();
			if (wlnk.isSSlotValid())
				neighbs.set(CDirection::SW);
			if (wlnk.isNSlotValid())
				neighbs.set(CDirection::NW);
		}
		return	neighbs;
	}


	//
	bool	CWorldMap::customCheckDiagMove(const CWorldPosition &pos, const	CDirection	&direction, TAStarFlag denyFlags) const
	{
		H_AUTO(AI_WorldMap_move);
		// get straight links
		const	CCellLinkage	&lnk = pos.getCellLinkage();

		switch	(direction.getVal())
		{
		case	CDirection::SE:
			{
				{
					CWorldPosition	tmpPos(pos);
					if (!tmpPos.getCellLinkage().isSSlotValid())
						return	false;

					tmpPos=tmpPos.getPosS();
					if ((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
						return	false;

					if (!tmpPos.getCellLinkage().isESlotValid())
						return	false;

					tmpPos=tmpPos.getPosE();
					if	((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
						return	false;
				}

				{
					CWorldPosition	tmpPos(pos);
					if (!tmpPos.getCellLinkage().isESlotValid())
						return	false;

					tmpPos=tmpPos.getPosE();
					if ((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
						return	false;

					if (!tmpPos.getCellLinkage().isSSlotValid())
						return	false;

//					tmpPos=tmpPos.getPosS();
//					if	((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
//						return	false;
				}
				return	true;
			}

		case	CDirection::NE:
			{
				{
					CWorldPosition	tmpPos(pos);
					if (!tmpPos.getCellLinkage().isNSlotValid())
						return	false;

					tmpPos=tmpPos.getPosN();
					if ((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
						return	false;

					if (!tmpPos.getCellLinkage().isESlotValid())
						return	false;

					tmpPos=tmpPos.getPosE();
					if	((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
						return	false;
				}

				{
					CWorldPosition	tmpPos(pos);
					if (!tmpPos.getCellLinkage().isESlotValid())
						return	false;

					tmpPos=tmpPos.getPosE();
					if ((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
						return	false;

					if (!tmpPos.getCellLinkage().isNSlotValid())
						return	false;

//					tmpPos=tmpPos.getPosN();
//					if	((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
//						return	false;
				}
				return	true;
			}
		case	CDirection::NW:
			{
				{
					CWorldPosition	tmpPos(pos);
					if (!tmpPos.getCellLinkage().isNSlotValid())
						return	false;

					tmpPos=tmpPos.getPosN();
					if ((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
						return	false;

					if (!tmpPos.getCellLinkage().isWSlotValid())
						return	false;

					tmpPos=tmpPos.getPosW();
					if	((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
						return	false;
				}

				{
					CWorldPosition	tmpPos(pos);
					if (!tmpPos.getCellLinkage().isWSlotValid())
						return	false;

					tmpPos=tmpPos.getPosW();
					if ((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
						return	false;

					if (!tmpPos.getCellLinkage().isNSlotValid())
						return	false;

//					tmpPos=tmpPos.getPosN();
//					if	((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
//						return	false;
				}
				return	true;
			}
		case	CDirection::SW:
			{
				{
					CWorldPosition	tmpPos(pos);
					if (!tmpPos.getCellLinkage().isSSlotValid())
						return	false;

					tmpPos=tmpPos.getPosS();
					if ((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
						return	false;

					if (!tmpPos.getCellLinkage().isWSlotValid())
						return	false;

					tmpPos=tmpPos.getPosW();
					if	((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
						return	false;
				}

				{
					CWorldPosition	tmpPos(pos);
					if (!tmpPos.getCellLinkage().isWSlotValid())
						return	false;

					tmpPos=tmpPos.getPosW();
					if ((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
						return	false;

					if (!tmpPos.getCellLinkage().isSSlotValid())
						return	false;

//					tmpPos=tmpPos.getPosS();
//					if	((tmpPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)
//						return	false;
				}
				return	true;
			}
			break;
		default:
			break;

		}
		return	false;
	}

/*
	bool	CWorldMap::customCheckDiagMove(const CWorldPosition &pos, const	CDirection	&direction, TAStarFlag denyFlags) const
	{
		H_AUTO(AI_WorldMap_move);
		// get straight links
		const	CCellLinkage	&lnk = pos.getCellLinkage();

		switch	(direction.getVal())
		{
		case	CDirection::SE:
			{
				if (!lnk.isSSlotValid())
					return	false;
				if (!lnk.isESlotValid())
					return	false;

				return	(	pos.getPosS().getCellLinkage().isESlotValid()
					||	pos.getPosE().getCellLinkage().isSSlotValid());
			}

		case	CDirection::NE:
			{
				if (!lnk.isNSlotValid())
					return	false;
				if (!lnk.isESlotValid())
					return	false;

				return	(	pos.getPosN().getCellLinkage().isESlotValid()
					||	pos.getPosE().getCellLinkage().isNSlotValid());
			}
		case	CDirection::NW:
			{
				if (!lnk.isNSlotValid())
					return	false;
				if (!lnk.isWSlotValid())
					return	false;

				return	(	pos.getPosN().getCellLinkage().isWSlotValid()
					||	pos.getPosW().getCellLinkage().isNSlotValid());
			}
		case	CDirection::SW:
			{
				if (!lnk.isSSlotValid())
					return	false;
				if (!lnk.isWSlotValid())
					return	false;

				return	(	pos.getPosS().getCellLinkage().isWSlotValid()
					||	pos.getPosW().getCellLinkage().isSSlotValid());
			}

		}
		return	false;
	}
*/

	//
	bool	CWorldMap::move(CWorldPosition &pos, const	CDirection	&direction) const
	{
		H_AUTO(AI_WorldMap_move);
		// get straight links
		const	CCellLinkage	&lnk = pos.getCellLinkage();

		switch	(direction.getVal())
		{
		case	CDirection::S:
			{
				if (lnk.isSSlotValid())
				{
					pos.stepS();
					return	true;
				}
				return false;
			}
		case	CDirection::SE:
			{
				if (lnk.isSSlotValid())
				{
					const	CWorldPosition	temp(pos.getPosS());
					if (temp.getCellLinkage().isESlotValid())
					{
						temp.setPosE(pos);
						return	true;
					}
				}

				if (lnk.isESlotValid())
				{
					const	CWorldPosition	temp(pos.getPosE());
					if (temp.getCellLinkage().isSSlotValid())
					{
						temp.setPosS(pos);
						return	true;
					}
				}
				return	false;
			}
		case	CDirection::E:
			{
				if (lnk.isESlotValid())
				{
					pos.stepE();
					return	true;
				}
				return false;
			}
		case	CDirection::NE:
			{
				if (lnk.isESlotValid())
				{
					const	CWorldPosition	temp(pos.getPosE());
					if (temp.getCellLinkage().isNSlotValid())
					{
						temp.setPosN(pos);
						return	true;
					}

				}

				if (lnk.isNSlotValid())
				{
					const	CWorldPosition	temp(pos.getPosN());
					if (temp.getCellLinkage().isESlotValid())
					{
						temp.setPosE(pos);
						return	true;
					}
				}

				return	false;
			}
		case	CDirection::N:
			{
				if (lnk.isNSlotValid())
				{
					pos.stepN();
					return	true;
				}
				return false;
			}
		case	CDirection::NW:
			{
				if (lnk.isWSlotValid())
				{
					const	CWorldPosition	temp(pos.getPosW());
					if (temp.getCellLinkage().isNSlotValid())
					{
						temp.setPosN(pos);
						return	true;
					}
				}

				if (lnk.isNSlotValid())
				{
					const	CWorldPosition	temp(pos.getPosN());
					if (temp.getCellLinkage().isWSlotValid())
					{
						temp.setPosW(pos);
						return	true;
					}
				}

				return	false;
			}
		case	CDirection::W:
			{
				if (lnk.isWSlotValid())
				{
					pos.stepW();
					return	true;
				}
				return false;
			}
		case	CDirection::SW:
			{
				if (lnk.isSSlotValid())
				{
					const	CWorldPosition	temp(pos.getPosS());
					if (temp.getCellLinkage().isWSlotValid())
					{
						temp.setPosW(pos);
						return	true;
					}
				}

				if (lnk.isWSlotValid())
				{
					const	CWorldPosition	temp(pos.getPosW());
					if (temp.getCellLinkage().isSSlotValid())
					{
						temp.setPosS(pos);
						return	true;
					}

				}
				return	false;
			}
		default:
			break;

		}
		return	false;
	}

	//
	bool	CWorldMap::moveSecure(CWorldPosition &pos, const CDirection	&direction, uint16 maskFlags) const
	{
		H_AUTO(AI_WorldMap_moveSecure);
		// get straight links

		uint16	pflags = pos.getFlags();

		switch	(direction.getVal())
		{
		case	CDirection::S:
			{
				return pos.moveS();
			}
		case	CDirection::SE:
			{
				CWorldPosition	p1(pos), p2(pos);
				if (p1.moveS() && (((p1.getFlags()^pflags)&maskFlags) == 0) && p1.moveE() && (((p1.getFlags()^pflags)&maskFlags) == 0) &&
					p2.moveE() && (((p2.getFlags()^pflags)&maskFlags) == 0) && p2.moveS() && (((p2.getFlags()^pflags)&maskFlags) == 0) &&
					p1 == p2)
				{
					pos = p1;
					return true;
				}
				break;
			}
		case	CDirection::E:
			{
				return pos.moveE();
			}
		case	CDirection::NE:
			{
				CWorldPosition	p1(pos), p2(pos);
				if (p1.moveN() && (((p1.getFlags()^pflags)&maskFlags) == 0) && p1.moveE() && (((p1.getFlags()^pflags)&maskFlags) == 0) &&
					p2.moveE() && (((p2.getFlags()^pflags)&maskFlags) == 0) && p2.moveN() && (((p2.getFlags()^pflags)&maskFlags) == 0) &&
					p1 == p2)
				{
					pos = p1;
					return true;
				}
				break;
			}
		case	CDirection::N:
			{
				return pos.moveN();
			}
		case	CDirection::NW:
			{
				CWorldPosition	p1(pos), p2(pos);
				if (p1.moveN() && (((p1.getFlags()^pflags)&maskFlags) == 0) && p1.moveW() && (((p1.getFlags()^pflags)&maskFlags) == 0) &&
					p2.moveW() && (((p2.getFlags()^pflags)&maskFlags) == 0) && p2.moveN() && (((p2.getFlags()^pflags)&maskFlags) == 0) &&
					p1 == p2)
				{
					pos = p1;
					return true;
				}
				break;
			}
		case	CDirection::W:
			{
				return pos.moveW();
			}
		case	CDirection::SW:
			{
				CWorldPosition	p1(pos), p2(pos);
				if (p1.moveS() && (((p1.getFlags()^pflags)&maskFlags) == 0) && p1.moveW() && (((p1.getFlags()^pflags)&maskFlags) == 0) &&
					p2.moveW() && (((p2.getFlags()^pflags)&maskFlags) == 0) && p2.moveS() && (((p2.getFlags()^pflags)&maskFlags) == 0) &&
					p1 == p2)
				{
					pos = p1;
					return true;
				}
				break;
			}
		default:
			break;
		}
		return	false;
	}

	//
	bool	CWorldMap::moveDiagTestBothSide(CWorldPosition &pos, const	CDirection	&direction) const
	{
		H_AUTO(AI_WorldMap_move);
		// get straight links
		const	CCellLinkage	&lnk = pos.getCellLinkage();

		switch	(direction.getVal())
		{
		case	CDirection::S:
			{
				if (lnk.isSSlotValid())
				{
					pos.stepS();
					return	true;
				}
				return false;
			}
		case	CDirection::SE:
			{
				if (!lnk.isSSlotValid() || !lnk.isESlotValid())
					return	false;

				CSlot	eSlot=pos.getPosS().getCellLinkage().ESlot();
				if (!eSlot.isValid())
					return	false;

				const	CWorldPosition	temp(pos.getPosE());
				if (temp.getCellLinkage().SSlot()!=eSlot)
					return	false;

				temp.setPosS(pos);
				return	true;
			}
		case	CDirection::E:
			{
				if (lnk.isESlotValid())
				{
					pos.stepE();
					return	true;
				}
				return false;
			}
		case	CDirection::NE:
			{
				if (!lnk.isNSlotValid() || !lnk.isESlotValid())
					return	false;

				CSlot	eSlot=pos.getPosN().getCellLinkage().ESlot();
				if (!eSlot.isValid())
					return	false;

				const	CWorldPosition	temp(pos.getPosE());
				if (temp.getCellLinkage().NSlot()!=eSlot)
					return	false;

				temp.setPosN(pos);
				return	true;
			}
		case	CDirection::N:
			{
				if (lnk.isNSlotValid())
				{
					pos.stepN();
					return	true;
				}
				return false;
			}
		case	CDirection::NW:
			{
				if (!lnk.isNSlotValid() || !lnk.isWSlotValid())
					return	false;

				CSlot	wSlot=pos.getPosN().getCellLinkage().WSlot();
				if (!wSlot.isValid())
					return	false;

				const	CWorldPosition	temp(pos.getPosW());
				if (temp.getCellLinkage().NSlot()!=wSlot)
					return	false;

				temp.setPosN(pos);
				return	true;
			}
		case	CDirection::W:
			{
				if (lnk.isWSlotValid())
				{
					pos.stepW();
					return	true;
				}
				return false;
			}
		case	CDirection::SW:
			{
				if (!lnk.isSSlotValid() || !lnk.isWSlotValid())
					return	false;

				CSlot	wSlot=pos.getPosS().getCellLinkage().WSlot();
				if (!wSlot.isValid())
					return	false;

				const	CWorldPosition	temp(pos.getPosW());
				if (temp.getCellLinkage().SSlot()!=wSlot)
					return	false;

				temp.setPosS(pos);
				return	true;
			}
		default:
			break;
		}
		return	false;
	}


	void	areCompatiblesWithoutStartRestriction(const CWorldPosition &startPos, const CWorldPosition& endPos, const TAStarFlag &denyflags, CCompatibleResult &res, bool allowStartRestriction)
	{
		res.setValid(false);

		if (&startPos == NULL)
		{
			nlwarning("Invalid startPos (NULL)");
			return;
		}

		if (&endPos == NULL)
		{
			nlwarning("Invalid endPos (NULL)");
			return;
		}

		const	CTopology	&startTopoNode=startPos.getTopologyRef().getCstTopologyNode();
		const	CTopology	&endTopoNode=endPos.getTopologyRef().getCstTopologyNode();
		TAStarFlag	startFlag=(TAStarFlag)(startTopoNode.getFlags()&WaterAndNogo);
//		if	(!allowStartRestriction)
			startFlag=Nothing;


		for	(TAStarFlag	possibleFlag=Nothing;possibleFlag<=WaterAndNogo;possibleFlag=(TAStarFlag)(possibleFlag+2))	//	tricky !! -> to replace with a defined list of flags to checks.
		{
			const	uint32	incompatibilityFlags=(possibleFlag&(denyflags&~startFlag))&WaterAndNogo;

			if	(incompatibilityFlags)
				continue;

			const	uint32	startMasterTopo=startTopoNode.getMasterTopo(possibleFlag);
			const	uint32	endMasterTopo=endTopoNode.getMasterTopo(possibleFlag);
			if	(	(startMasterTopo^endMasterTopo)!=0
				||	startMasterTopo == std::numeric_limits<uint32>::max())	// if not same masterTopo or invalid masterTopo then bypass ..
				continue;

			res.set(possibleFlag, startMasterTopo);
			res.setValid();

			if	(((possibleFlag&denyflags)&WaterAndNogo)==0)	//	it was the optimal case ?
				break;
		}

	}

//////////////////////////////////////////////////////////////////////////////
// Path finding                                                             //
//////////////////////////////////////////////////////////////////////////////

//#define CHECK_HEAP
std::map<uint, uint> MapAStarNbSteps;
uint LastAStarNbSteps = 0;

NLMISC_CATEGORISED_COMMAND(ais, dumpAStarSteps, "Dump the distribution of A* number of steps", "")
{
	log.displayNL( "Distribution of the %u nb steps:", MapAStarNbSteps.size() );
	log.displayNL( "NbSteps\tNbOccurrences", MapAStarNbSteps.size() );
	for( std::map<uint, uint>::const_iterator first=MapAStarNbSteps.begin(), last=MapAStarNbSteps.end(); first!=last; ++first )
	{
		log.displayNL( "%u\t%u", first->first, first->second );
	}
	return true;
}

bool CWorldMap::findAStarPath(CWorldPosition const& start, CWorldPosition const& end, std::vector<CTopology::TTopologyRef>& path, TAStarFlag denyflags) const
{
	H_AUTO(findAStarPath1);

	// Clear destination path
	path.clear();

	// Check start position validity
	if (!start.isValid())
	{
		_LastFASPReason = FASPR_INVALID_START_POS;
		return false;
	}
	// Check end position validity
	if (!end.isValid())
	{
		_LastFASPReason = FASPR_INVALID_END_POS;
		return false;
	}

	// Get start and end topologies
	CTopology::TTopologyRef	startTopo = start.getTopologyRef();
	CTopology::TTopologyRef	endTopo = end.getTopologyRef();

	// Get associated topology nodes
	CTopology const& startTopoNode = startTopo.getCstTopologyNode();
	CTopology const& endTopoNode = endTopo.getCstTopologyNode();

	// Check start point
	if (!startTopo.isValid())
	{
		_LastFASPReason = FASPR_INVALID_START_TOPO;
		return	false;
	}
	// Check end point
	if (!endTopo.isValid())
	{
		_LastFASPReason = FASPR_INVALID_END_TOPO;
		return	false;
	}

	// Check compatibility of start and end points depending on flags to avoid
	RYAI_MAP_CRUNCH::CCompatibleResult res;
	areCompatiblesWithoutStartRestriction(start, end, denyflags, res, true);
	if (!res.isValid())
	{
		_LastFASPReason = FASPR_INCOMPATIBLE_POSITIONS;
		return	false;
	}

	// Get flags to use to compute the path
	TAStarFlag movementFlags = res.movementFlags();
	// Get the master topology inside which to compute the path (reminder: no path between different master topo)
	uint32 choosenMasterTopo=res.choosenMasterTopo();

	// A list of A* nodes
	vector<CAStarHeapNode> nodes;
	// List of visited topologies, with associated node in 'nodes' vector
	map<CTopology::TTopologyId, uint> visited;

	// The heap used to store A* nodes
	// :TODO: Check if STL heap is not better suited, or if another data structure would be more useful in AIS
	CHeap<float, uint> heap;

	// Get end position
	CVector const& endPoint = endTopo.getCstTopologyNode().Position;

	// Create a heap node for the start point
	CAStarHeapNode hnode(startTopo, 0xffffffff, 0.0f, true);
	// Push it in the node list
	nodes.push_back(hnode);
	// Take it as first father
	uint father = (uint)nodes.size()-1;

	// Add start topology to visited nodes (father holds start topo node index for the moment)
	visited.insert(make_pair<CTopology::TTopologyId,uint>(startTopo, father));
	// Push start node in the heap with a zero cost
	heap.push(0.0f, father);

	// Boolean to notify that end point has been reached
	bool found = false;

#ifdef CHECK_HEAP
	static uint32 maxHeap = 65535;
	static uint32 maxHeapMeasure = 0;
#endif

	uint nbHeapSteps = 0;
	while (!heap.empty())
	{
	#ifdef CHECK_HEAP
		if (heap.size()>maxHeap) // if too much calculs, not found (to remove when debugged).
			break;
	#endif

		++nbHeapSteps;

		// Get best node (popping it)
		father = std::numeric_limits<uint>::max(); // :TODO: Remove that useless statement (since do while first loop ALWAYS overwrite it)
		do
		{
			father = heap.pop();
		}
		while (!nodes[father].isOpened() && !heap.empty());

		if (father == std::numeric_limits<uint>::max())
			break;

		// Mark current node as closed
		hnode.setOpen(false);

		// Make best node the current one
		hnode = nodes[father];

		// Get the current node itself
		CTopology::TTopologyRef const& current = hnode.getRef();

		// If we reached the end node, stop search
		if (current==endTopo)
		{
			found=true;
			break;
		}

		// Get current node topology
		CTopology const& ctp = current.getCstTopologyNode();

		// Get g(n) for current node
		float dist = hnode.getDistance();

		// Examine each neighbour of the current node
		for (vector<CTopology::CNeighbourLink>::const_iterator it=ctp.Neighbours.begin(), itEnd=ctp.Neighbours.end();it!=itEnd;++it)
		{
			// Get the neighbour topology node
			CTopology::CNeighbourLink const& neighbourLink = (*it);
			CTopology::TTopologyRef const& next = neighbourLink.getTopologyRef();

			// If it's not in the same master topo skip it
			if	(next.getCstTopologyNode().getMasterTopo(movementFlags)!=choosenMasterTopo)
				continue;

			// Compute neighbour node g(n)
			float distance = dist + neighbourLink.getDistance();

			uint child;
			// Check if node has already been visited
			map<CTopology::TTopologyId, uint>::iterator	itv = visited.find(next);
			if (itv!=visited.end())
			{
				// Assume child is that node
				child = (*itv).second;
				// If that node's previous distance is better than the new one skip it
				if (nodes[child].getDistance() <= distance)
					continue;

				// Close the old node
				nodes[child].setOpen(false);
				// Remove it from visited
				visited.erase(itv);
			}
			// Create a new node for that cell
			child = (uint)nodes.size();
			nodes.push_back(CAStarHeapNode(next, father, distance, true));
			// Compute h(n) as an euclidian distance heuristic
			float heuristic = (endPoint-next.getCstTopologyNode().Position).norm();
			// Add node to heap with a computed f(n)=g(n)+h(n)
			heap.push(distance + heuristic, child);
			// Add node to visited
			visited.insert(make_pair<CTopology::TTopologyId,uint>(next, child));
		}
	}

#ifdef	CHECK_HEAP
	if (heap.size()>maxHeapMeasure)
	{
		maxHeapMeasure=heap.size();
	}
#endif

	++MapAStarNbSteps[nbHeapSteps];
	LastAStarNbSteps = nbHeapSteps;

#ifdef NL_DEBUG
	nlassert(found);
#else
	if (!found)
	{
		nlwarning("(!!Appeler StepH!!)Path not found from %s : %d to %s : %d", start.toString().c_str(), start.slot(), end.toString().c_str(), end.slot());
	}
#endif

	// If not found, return error
	if (!found)
	{
		_LastFASPReason = FASPR_NOT_FOUND;
		return false;
	}

	// Backtrack path
	while (father != 0xffffffff)
	{
		CAStarHeapNode const& node = nodes[father];
		path.push_back(node.getRef());
		father = node.getFather();
	}

	// Reverse path container
	std::reverse(path.begin(), path.end());

	_LastFASPReason = FASPR_NO_ERROR;
	return true;
}

// Finds an A* path
bool	CWorldMap::findAStarPath(const CTopology::TTopologyId &start, const CTopology::TTopologyId &end, CAStarPath &path, TAStarFlag denyflags) const
{
	H_AUTO(findAStarPath2)
	path._TopologiesPath.clear();

	CTopology::TTopologyRef	startTopo = getTopologyRef(start);
	CTopology::TTopologyRef	endTopo = getTopologyRef(end);

	// if not found start point or end point, abort
	if	(!startTopo.isValid() || !endTopo.isValid())
		return	false;

	vector<CAStarHeapNode>		nodes;
	map<CTopology::TTopologyId, uint>	visited;	// topology + node index in vector
	CHeap<float, uint>			heap;

	const	CVector	&endPoint =	endTopo.getCstTopologyNode().Position;

	// add current to heap
	CAStarHeapNode	hnode(startTopo,0xffffffff,0.0f,true);

	nodes.push_back(hnode);
	uint	father = (uint)nodes.size()-1;

	// add current to visited nodes
	visited.insert(make_pair<CTopology::TTopologyId,uint>(startTopo, father));
	heap.push(0.0f, father);

	bool	found=false;

	while (!heap.empty())
	{
		// pop best node
		father = heap.pop();
		hnode = nodes[father];

		const	CTopology::TTopologyRef		&current	=	hnode.getRef();

		// if reached end node, leave
		if (current==endTopo)
		{
			found=true;
			break;
		}

		const CTopology	&ctp = current.getCstTopologyNode();
		float			dist = hnode.getDistance();

		for (uint	i=0; i<ctp.Neighbours.size(); ++i)
		{
			const	CTopology::CNeighbourLink	&neightBourgLink=ctp.Neighbours[i];

			const	CTopology::TTopologyRef	&next=neightBourgLink.getTopologyRef();

			const CTopology	&ntp = next.getCstTopologyNode();

			// don't examine not accessible nodes
			if ((ntp.Flags & denyflags) != 0)
				continue;

			// compute actual node distance
			float	distance = dist + neightBourgLink.getDistance();

			// if node is open or closed and previous visit has shorter path, skip node
			map<CTopology::TTopologyId, uint>::iterator	itv = visited.find(next);
			if (itv != visited.end() && nodes[(*itv).second].getDistance() <= distance)
				continue;

			// compute heuristic
			float	heuristic = distance + (endPoint-ntp.Position).norm();

			// setup node
			CAStarHeapNode	cnode(next,father,distance,true);
			uint			child;

			// setup node
			if (itv == visited.end())
			{
				// if node is not open nor closed, create an entry
				child = (uint)nodes.size();
				nodes.push_back(cnode);
			}
			else
			{
				// else recover previous entry -- reopen node
				child = (*itv).second;
				nodes[child]=cnode;
			}

			// add node to visited and to heap
			heap.push(heuristic, child);
			visited.insert(make_pair<CTopology::TTopologyId,uint>(next, child));
		}
	}

	// if not found, return error
	if (!found)
		return	false;

	// backtrack path
	while (father != 0xffffffff)
	{
		const	CAStarHeapNode&	node=nodes[father];

		path._TopologiesPath.push_back(node.getRef());
		father = node.getFather();
	}

	// reverse path
	reverse(path._TopologiesPath.begin(), path._TopologiesPath.end());

	return true;
}


//	This whole routine MUST be optimized !! Its slow .. (to much).
bool CWorldMap::findInsideAStarPath(CWorldPosition const& start, CWorldPosition const& end, std::vector<CDirection>& stepPath, TAStarFlag denyflags) const
{
	H_AUTO(findInsideAStarPath);

	// Check start and end position validity
	if (!start.isValid())
	{
		_LastFIASPReason = FIASPR_INVALID_START_POS;
		return	false;
	}
	if (!end.isValid())
	{
		_LastFIASPReason = FIASPR_INVALID_END_POS;
		return	false;
	}
	// Verify that they are in the same topology
	if (start.getTopologyRef()!=end.getTopologyRef())
	{
		_LastFIASPReason = FIASPR_DIFFERENT_TOPO;
		return false;
	}

	// A list of A* nodes
	vector<CInsideAStarHeapNode> nodes;
	// List of visited nodes, with associated node index in 'nodes' vector
	map<CAStarNode, uint> visited;
	// The heap used to store A* nodes
	// :TODO: Check if STL heap is not better suited, or if another data structure would be more useful in AIS
	CHeap<float, uint> heap;

	// Get end point
	CVector endPoint = end.toVectorD();
	// Build a node for the end point
	CAStarNode endId(end);
	// Build a node for the start point
	CAStarNode startNode(start);

	// Create a heap node for the start point and push it in the node list
	nodes.push_back(CInsideAStarHeapNode(startNode, 0xffffffff, CDirection(), 0.f, true));
	// Take it as first father
	uint father = (uint)nodes.size()-1;

	// Add start node to visited nodes (father holds start node index for the moment)
	visited.insert(make_pair<CAStarNode, uint>(startNode, father));
	// Push start node in the heap with a zero cost
	heap.push(0.0f, father);

	// Boolean to notify that end point has been reached
	bool found = false;

	while (!heap.empty())
	{
		// Get best node (popping it)
		father = heap.pop();
		// Get associated heap node
		CInsideAStarHeapNode& hnode = nodes[father];
		// Get associated A* node
		CAStarNode const& current = hnode.getNode();

		// If we reached the end node, stop search
		if (current==endId)
		{
			found=true;
			break;
		}

		// Get g(n) for current node
		float dist = hnode.getDistance();

		// Get current node slot
		CSlot slot = current.slot();
		// Compute a map position
		CMapPosition pos = start; // The map has the same cell than the start pos (coz in same topo)
		current.updateMapPosition(pos); // Just update the unit id

		// For each neighbour (8 directions)
		CNeighbourhood neighbourhood = neighbours(getWorldPosition(pos,slot));
		for (uint i=0; i<8; ++i)
		{
			// Compute a CDirection
			CDirection dir((CDirection::TDirection)i);

			// If neighbour in that direction is not valid skip it
			if (!neighbourhood.isValid(dir))
				continue;

			// :TODO: Continue documentation
			// If we cannot move in that direction skip it
			CWorldPosition	mv(getWorldPosition(pos, slot));
			if (!moveDiagTestBothSide(mv, dir))
				continue;

			// If that new point is not in the same cell skip it
			if (!mv.hasSameFullCellId(start))
				continue;

			// If that point's flags are not compatible skip it
			if ((denyflags & mv.getTopologyRef().getCstTopologyNode().getFlags()) != 0)
				continue;

			// Build an A* node
			CAStarNode next(mv);

			// Compute g(n) (diagonal)
			float distance = dist + (((i & 1) != 0) ? 1.4142f : 1.0f);

			// If node has already been visited and previous distance was better skip it
			map<CAStarNode, uint>::iterator	itv = visited.find(next);
			if (	itv != visited.end()
				&&	nodes[(*itv).second].getDistance() <= distance	)
				continue;

			uint child;
			// If node has already been visited update it
			if (itv!=visited.end())
			{
				child = (*itv).second;
				nodes[child] = CInsideAStarHeapNode(next, father, dir, distance, true);
			}
			// Else create a new node
			else
			{
				child = (uint)nodes.size();
				nodes.push_back(CInsideAStarHeapNode(next, father, dir, distance, true));
			}
			// Compute h(n) as an euclidian distance heuristic
			float heuristic = (endPoint-mv.toVectorD()).norm();
			// Add node to heap with a computed f(n)=g(n)+h(n)
			heap.push(distance + heuristic, child);
			// Add node to visited
			visited.insert(make_pair<CAStarNode, uint>(next, child));
		}
	}

#ifdef NL_DEBUG
	nlassert(found);
#endif
	// If not found, return error
	if (!found)
	{
		_LastFIASPReason = FIASPR_NOT_FOUND;
		return false;
	}

	stepPath.clear();

	// Backtrack path
	while (father != 0xffffffff)
	{
		if (nodes[father].getFather() != 0xffffffff)
			stepPath.push_back(nodes[father].getDirection());
		father = nodes[father].getFather();
	}

	// Reverse path container
	std::reverse(stepPath.begin(), stepPath.end());

	_LastFIASPReason = FIASPR_NO_ERROR;
	return true;
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

bool CWorldMap::moveTowards(CWorldPosition& pos, CTopology::TTopologyRef const& topology) const
{
	CGridDirectionLayer const* layer = getGridDirectionLayer(pos, topology);
	if (!layer)
		return	false;

	CDirection	motion=layer->getDirection(pos);
	if (!motion.isValid())
		return	false;

	return	move(pos,motion);
}

// Moves according a to a given path, returns false if failed
bool	CWorldMap::move(CWorldPosition &pos, CAStarPath &path, uint &currentstep) const
{
	if	(currentstep >= path._TopologiesPath.size())
		return	false;

	CTopology::TTopologyRef	cid(pos);

	if	(!cid.isValid())
		return false;

	if	(cid==path._TopologiesPath[currentstep])
	{
		++currentstep;
		if	(currentstep==path._TopologiesPath.size())
			return false;
	}
	return	moveTowards(pos, path._TopologiesPath[currentstep]);
}


// Moves from a position to another
bool CWorldMap::move(CWorldPosition& pos, CMapPosition const& end, TAStarFlag const denyFlags) const
{
	CWorldPosition	tempPos(pos);
	BOMB_IF((tempPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0, "Error in CWorldMap::mode, invalid flag "<<RYAI_MAP_CRUNCH::toString(tempPos.getTopologyRef().getCstTopologyNode().getFlags())
		<<"on world pos "<<pos.toString()<<" while going to map pos "<<end.toString()<<" with denyflags "<<RYAI_MAP_CRUNCH::toString(denyFlags), return false);
	//	not optimum but it will be rewrite for each specialized rootcell type.
	const	sint32	x0	=	pos.x();
	const	sint32	y0	=	pos.y();

	const	sint32	deltax	=	end.x() - x0;
	const	sint32	deltay	=	end.y() - y0;

	const	sint32	d	=	std::max(abs(deltax), abs(deltay));

	for (sint32 i=1; i<=d; ++i)
	{
		const	sint	dx = x0 + (deltax*i)/d - pos.x();
		const	sint	dy = y0 + (deltay*i)/d - pos.y();


		if	(	!move(tempPos, CDirection(dx,dy))
			||	(tempPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlags)!=0)	//	Arghh !!
			return	false;
		pos=tempPos;
	}
	return true;
}

// Clears height map
void CWorldMap::clearHeightMap()
{
	CMapPosition	min, max;
	getBounds(min, max);

	CMapPosition	scan, scanline;

	for (scan = min; scan.yCoord() != max.yCoord(); scan = scan.stepCell(0, 1))
	{
		for (scanline = scan; scanline.x() != max.x(); scanline = scanline.stepCell(1, 0))
		{
			CRootCell*	rootCell=getRootCell(scanline);
			if (!rootCell)
				continue;
			rootCell->clearHeightMap();
		}
	}
}

// checks motion layers
void	CWorldMap::checkMotionLayer()
{
	CMapPosition	min, max;
	getBounds(min, max);

	uint	compute = 0, white = 0, simple = 0, multi = 0, other = 0;
	countCells(compute, white, simple, multi, other);
	uint	total = compute+white+simple+multi+other;
	uint	compCells = 0;

	CMapPosition	scan, scanline;
	CTimeEstimator	timeest(total);

	for (scan = min; scan.yCoord() != max.yCoord(); scan = scan.stepCell(0, 1))
	{
		for (scanline = scan; scanline.x() != max.x(); scanline = scanline.stepCell(1, 0))
		{

			CRootCell*	rootCell=getRootCell(scanline);
			if (!rootCell)
				continue;

			timeest.step("checkMotionLayer");

			vector<CTopology>					&topologies = rootCell->getTopologiesNodes();

			// move from any point to the current topology
			uint	i;
			for (i=0; i<topologies.size(); ++i)
			{
				CTopology							&topology = topologies[i];
				CTopology::TTopologyRef				tref = getTopologyRef(topology.Id);
				set<CTopology::TTopologyId>			neighbours;

				//
				uint16								toflags = topology.Flags;

				uint	j;
				for (j=0; j<topology.Neighbours.size(); ++j)
					neighbours.insert(topology.Neighbours[j].getTopologyRef());

				sint	x, y, slot;

				bool	failed = false;

				for (y=-16; y<32; ++y)
				{
					for (x=-16; x<32; ++x)
					{
						CMapPosition	cpos(scanline.x()+x, scanline.y()+y);

						for (slot=0; slot<3; ++slot)
						{
							// build world pos and test it is valid
							CWorldPosition	cwpos = getSafeWorldPosition(cpos, CSlot(slot));
							if (!cwpos.isValid())
								continue;

							// if current topo is not a neighbour of checked topo, go to the next pos
							CTopology::TTopologyId	ctopoId = getTopologyId(cwpos);
							if (neighbours.find(ctopoId) == neighbours.end())
								continue;

							//
							CTopology	&starttopo = getTopologyNode(ctopoId);
							uint16		fromflags = starttopo.Flags;

							cwpos = getWorldPosition(cpos, CSlot(slot));

							bool	found = false;
							uint32	numSteps = 0;

							do
							{
								if (getTopologyId(cwpos) == tref)
								{
									found = true;
									break;
								}

								CTopology	&ctopo = getTopologyNode(getTopologyId(cwpos));
								uint16		cflags = ctopo.Flags;

								uint16		allow = ((~fromflags) & (~toflags) & cflags);

								if (allow != 0)
								{
									nlwarning("Unallowed move from (%04X,%04X,%d-topo=%08X) to topo %08X, fromflags=%04X, toflags=%04X, cflags=%04X", cpos.x(), cpos.y(), slot, ctopoId.getVal(), tref.getVal(), fromflags, toflags, cflags);
									failed = true;
								}
							}
							while (moveTowards(cwpos, tref) && cwpos.isValid() && numSteps++ < 128);

							if (!found)
							{
								nlwarning("Failed to go from (%d,%d,%d-topo=%08X) to topo %08X", cpos.x(), cpos.y(), slot, ctopoId.getVal(), tref.getVal());
							}

						}

					}

				}

				if (failed)
				{
					nlinfo("---- scanline = %04X %04X topo = %d ----", scanline.x(), scanline.y(), i);
					topology.DirectionMap->dump();
				}

			}

		}

	}

}

//
void	CWorldMap::buildMasterTopo(bool allowWater, bool allowNogo)
{
	nlinfo("buildMasterTopo");

	CMapPosition	min, max;
	getBounds(min, max);

	CMapPosition	scan, scanline;

	uint			masterTopo = 0;
	uint			totalTopos = 0;

	for (scan = min; scan.y() != max.y(); scan = scan.stepCell(0, 1))
	{
		for (scanline = scan; scanline.x() != max.x(); scanline = scanline.stepCell(1, 0))
		{
			CRootCell		*cell = getRootCell(scanline);
			if (cell == NULL)
				continue;

			vector<CTopology>	&topos = cell->getTopologiesNodes();

			uint	i;
			for (i=0; i<topos.size(); ++i)
			{
				CTopology&				tp = topos[i];

				++totalTopos;

				if (!tp.isCompatible(allowWater, allowNogo) ||
					tp.getMasterTopo(allowWater, allowNogo) != CTopology::TTopologyId::UNDEFINED_TOPOLOGY)
					continue;

				set<CTopology::TTopologyId>	tovisit;

				// set mastertopo id
				tp.getMasterTopoRef(allowWater, allowNogo) = masterTopo;
				tovisit.insert(tp.Id);

				while (!tovisit.empty())
				{
					CTopology&	t = getTopologyNode(*(tovisit.begin()));
					tovisit.erase(tovisit.begin());

					uint	j;
					for (j=0; j<t.Neighbours.size(); ++j)
					{
						CTopology&	neighb = getTopologyNode(t.Neighbours[j].getTopologyRef());

						// can go to the neighbour and neighb not visited yet ?
						if (!neighb.isCompatible(allowWater, allowNogo) ||
							neighb.getMasterTopo(allowWater, allowNogo) != CTopology::TTopologyId::UNDEFINED_TOPOLOGY)
							continue;

						// set mastertopo id
						neighb.getMasterTopoRef(allowWater, allowNogo) = masterTopo;
						tovisit.insert(neighb.Id);
					}
				}
				++masterTopo;
			}
		}
	}
	nlinfo("Built %d master topologies (%d topos tested)", masterTopo, totalTopos);
}

//
void	CWorldMap::countSuperTopo()
{
	nlinfo("Count super topologies");

	CMapPosition	min, max;
	getBounds(min, max);

	CMapPosition	scan, scanline;

	uint			superTopo = 0;
	set<CTopology::TTopologyId>		visited;

	uint			totalTopos = 0;

	for (scan = min; scan.y() != max.y(); scan = scan.stepCell(0, 1))
	{
		for (scanline = scan; scanline.x() != max.x(); scanline = scanline.stepCell(1, 0))
		{
			const CRootCell		*cell = getRootCellCst(scanline);
			if (cell == NULL)
				continue;

			const vector<CTopology>	&topos = cell->getTopologiesNodes();

			uint	i;
			for (i=0; i<topos.size(); ++i)
			{
				CTopology::TTopologyId	topoid = topos[i].Id;

				++totalTopos;

				if (visited.find(topoid) == visited.end())
				{
					set<CTopology::TTopologyId>	tovisit;

					tovisit.insert(topoid);
					visited.insert(topoid);

					while (!tovisit.empty())
					{
						CTopology::TTopologyId	id = *(tovisit.begin());
						tovisit.erase(tovisit.begin());

						const CTopology	&t = getTopologyNode(id);

						uint	j;
						for (j=0; j<t.Neighbours.size(); ++j)
						{
							CTopology::TTopologyId	neighb = t.Neighbours[j].getTopologyRef();
							if (visited.find(neighb) == visited.end())
							{
								visited.insert(neighb);
								tovisit.insert(neighb);
							}

						}

					}
					++superTopo;
				}

			}

		}

	}
	nlinfo("Found %d super topologies (%d topos tested, %d topos visited)", superTopo, totalTopos, visited.size());
}

bool	CWorldMap::setWorldPosition(sint32 z, CWorldPosition	&wpos,	const CAIVector	&pos, const CRootCell	*originCell) const
{
	CSlot	slot;
	const	CMapPosition	mapPos(pos);

	const CRootCell	*cell = originCell ? originCell : getRootCellCst(mapPos);
	if (!cell)
	{
		return false;
	}

	sint32 minDistZ = INT_MAX;
	CSlot bestSlot;
	sint32 bestZ = 0;
	// Find best slot
	for (uint32	s=0; s<3; ++s)
	{
		if (!cell->isSlotUsed(mapPos, CSlot(s)))
			continue;

		CSlot	sslot=CSlot(s);
		sint32 sh = cell->getMetricHeight(CWorldPosition(cell, mapPos, sslot));
		sint32 dist = z-sh;
		dist = dist<0?-dist:dist;
		if (dist < minDistZ)
		{
			nlassert(dist>=0);
			minDistZ = dist;
			bestZ = sh;
			bestSlot = sslot;
		}
	}
	if (!bestSlot.isValid())
	{
		wpos = CWorldPosition(cell,mapPos,bestSlot,true);
		return false;
	}
	wpos = CWorldPosition(cell,mapPos,bestSlot);
//	if (simulateBug(5))
//	{
//		if (minDistZ > 2000)
//		{
//			return false;
//		}
//	}
//	else
	{
		// :KLUDGE: Water hack
		// If error is too big and player is either not in water or under slot (ie not floating above slot at surface)
		if (minDistZ > 2000 && ((wpos.getFlags()&RYAI_MAP_CRUNCH::Water)==0 || z<bestZ))
		{
			return false;
		}
	}
	return true;
}

bool	CWorldMap::setWorldPosition( double z, CWorldPosition	&wpos,	const CAIVector	&pos, const CRootCell	*originCell) const
{
	nlassert(false && "Double version of setWorldPosition isn't tested !!! Verify it's the same than above sint32 version !");
	CSlot	slot;
	const	CMapPosition	mapPos(pos);

	const CRootCell	*cell = originCell ? originCell : getRootCellCst(mapPos);
	if (!cell)
	{
		return false;
	}

	double minDistZ = DBL_MAX;
	CSlot bestSlot;
	double bestZ;
	// find best slot
	for (uint32	s=0; s<3; ++s)
	{
		if (!cell->isSlotUsed(mapPos, CSlot(s)))
			continue;

		CSlot	sslot=CSlot(s);
		double sh = ((double)cell->getMetricHeight(CWorldPosition(cell, mapPos, sslot)))/1000.0;
		double dist = fabs(z-sh);
		if (dist < minDistZ)
		{
			nlassert(dist>=0);
			minDistZ = dist;
			bestZ = sh;
			bestSlot = sslot;
		}
	}

	if (!bestSlot.isValid())
	{
		wpos=CWorldPosition(cell,mapPos,bestSlot,true);
		return	false;
	}
	if (minDistZ > 2.000)
	{
	//	nldebug("Setting a WorldPosition too far from specified z: x=%d y=%d z=%f slotz=%f", pos.x(), pos.y(), z, bestZ);
		wpos = CWorldPosition(cell,mapPos,bestSlot,true);
		return false;
	}
	wpos=CWorldPosition(cell,mapPos,bestSlot);
	return	true;
}

}

