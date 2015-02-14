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

// Nel misc
#include "nel/misc/command.h"
#include "nel/misc/variable.h"

#include "nel/misc/aabbox.h"
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/array_2d.h"
// TMP TMP
#include "nel/misc/bitmap.h"


#include "nel/misc/algo.h"


// Nel pacs
#include "nel/pacs/u_collision_desc.h"
#include "nel/pacs/u_global_position.h"
#include "nel/pacs/u_global_retriever.h"
#include "nel/pacs/u_move_container.h"
#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/u_retriever_bank.h"
#include "nel/pacs/u_primitive_block.h"

#include "nel/pacs/global_retriever.h"


// Nel Ligo
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive.h"


// Server share
#include "game_share/bmp4image.h"
#include "server_share/continent_container.h"


// AI share
#include "ai_share/world_map.h"
#include "ai_share/ai_spawn_commands.h"


// STL
#include <vector>
#include <map>


using namespace std;
using namespace NLMISC;
using namespace NLPACS;
using namespace RYAI_MAP_CRUNCH;
using namespace NLLIGO;

extern CLigoConfig LigoConfig;

//CAISpawnCtrl *CAISpawnCtrl::_instance=NULL;

string	EvaluatedPos;

/// The start point for each continent
multimap<string, CVectorD>		StartPoints;
multimap<string, string>		PrimFiles;
CVectorD						DefaultStartPoint = CVectorD::Null;

/// The output path
string							OutputPath = string("./");
vector<string>					PacsPrimPath;
vector<string>					LookupPath;
vector<string>					LookupNoRecursePath;
bool							PathInitialized = false;

CWorldMap						StaticWorldMap;
uint							Verbose = 1;
CVectorD						BoxMin, BoxMax;


namespace RYPACSCRUNCH
{

class	CPacsCruncher
{
public:

	CContinentContainer			_Continents;

	URetrieverBank				*_Bank;
	UGlobalRetriever			*_Retriever;
	UMoveContainer				*_Container;
	UMovePrimitive				*_Primitive;
	UMovePrimitive				*_Primitive3;
	UMovePrimitive				*_Primitive5;

	CWorldMap					_WorldMap;

	deque<UGlobalPosition>		_Positions1;
	deque<UGlobalPosition>		_Positions3;
	deque<UGlobalPosition>		_Positions5;

	CVectorD					_BMin;
	CVectorD					_BMax;
	CVectorD					_BSize;

	uint32						_RetrieverWidth;
	uint32						_RetrieverHeight;
	uint32						_RetrieverArea;

	typedef std::map<std::string, NLPACS::UPrimitiveBlock*>		TPacsPrimMap;
	static TPacsPrimMap			_PacsPrimMap;

protected:

	CSlot	getSurfaceAfterMove(UGlobalPosition &pos, const CMapPosition &newPosition, uint maxGabarit)
	{
		CVectorD		target = newPosition.toVectorD()+CVectorD(0.1, 0.1, 0.0);
		CVectorD		motion = target - _Retriever->getDoubleGlobalPosition(pos);

		motion.z = 0.0;

		UMovePrimitive	*prims[3] = { _Primitive, _Primitive3, _Primitive5 };
		sint	gabarit;
		CSlot	slot;

		for (gabarit=maxGabarit; gabarit>=0; --gabarit)
		{
			UMovePrimitive	*prim = prims[gabarit];

			prim->setGlobalPosition(pos, 0);
			prim->move(motion, 0);
			//_Container->evalNCPrimitiveCollision(1.0, prim, 0);
			_Container->evalCollision(1.0, 0);

			UGlobalPosition newPos;
			prim->getGlobalPosition(newPos, 0);

			// DEBUG HERE
			//
			CVectorD	dpos = _Retriever->getDoubleGlobalPosition(pos);
			CVectorD	dnpos = _Retriever->getDoubleGlobalPosition(newPos);

			//bool	test = (static_cast<NLPACS::CGlobalRetriever*>(_Retriever))->testPosition(newPos);
			bool test=true;

			NLPACS::CGlobalRetriever* gRetriever = static_cast<NLPACS::CGlobalRetriever*>(_Retriever);

			if (newPos.InstanceId < 0 || newPos.InstanceId >= (sint)gRetriever->getInstances().size())
				test=false;

			const CRetrieverInstance & instance = gRetriever->getInstances()[newPos.InstanceId];

			if (!instance.getBBox().include(newPos.LocalPosition.Estimation + instance.getOrigin()))
				test=false;
			else
			{
				const CLocalRetriever & lRetriever = gRetriever->getRetriever(instance.getRetrieverId());
				
				if (!lRetriever.isLoaded())
					test=false;
				else if (newPos.LocalPosition.Surface < 0 || newPos.LocalPosition.Surface >= (sint)lRetriever.getSurfaces().size())
				{
					nlwarning("can't test inexistant surface %d", newPos.LocalPosition.Surface);
					test=false;
				}
				else if (fabs(newPos.LocalPosition.Estimation.x) >= 256.0 || fabs(newPos.LocalPosition.Estimation.y) >= 256.0)
					test=false;
			}

			if (!test)
			{
				if (Verbose)
					nlinfo("Move from pos(%d,%d,%f,%f/%f,%f) motion(%f,%f) -> pos(%d,%d,%f,%f/%f,%f) %s", pos.InstanceId, pos.LocalPosition.Surface, pos.LocalPosition.Estimation.x, pos.LocalPosition.Estimation.y, dpos.x, dpos.y, motion.x, motion.y, newPos.InstanceId, newPos.LocalPosition.Surface, newPos.LocalPosition.Estimation.x, newPos.LocalPosition.Estimation.y, dnpos.x, dnpos.y, test ? "succeded" : "failed");
				return	slot;
			}
			//
			//

			CVectorD	dmotion = dpos+motion-dnpos;
			dmotion.z = 0.0;

			// if the move was not accomplished successfully return '3' as "direction blocked"
			if ( dmotion.norm() > 1.0e-2 )
				continue;

			CMapPosition	checkPosition(_Retriever->getDoubleGlobalPosition(newPos));
			nlassert(newPosition == checkPosition);

			CUnitSlot	surfId(newPos);
			TCellUnit	&slots = _WorldMap.getCellUnit(newPosition);

			// locate a slot to stick the surface in
			bool	newSlot = false;
			for (slot=CSlot(0); slot.isValid(); ++slot) 
			{
				CUnitSlot&	unitSlot=slots[slot.slot()];
				if (!unitSlot.used())
				{
					unitSlot = surfId;
					unitSlot.setHeight((uint)floor(dnpos.z/2.0 + 0.5));
					unitSlot.setInterior(_Retriever->isInterior(newPos));
					float	waterHeight;
					unitSlot.setWater(_Retriever->isWaterPosition(newPos, waterHeight));
					unitSlot.setGabarit(gabarit);
					if (gabarit==0)			_Positions1.push_back(newPos);
					else if (gabarit==1)	_Positions3.push_back(newPos);
					else					_Positions5.push_back(newPos);
					newSlot = true;
					break;
				}
				else if (unitSlot.hasSameSurface(surfId))
				{
					break;
				}
			}

			break;
		}
		return	slot;
	}

	void	readPrimitive(IPrimitive *primitive, const std::string &insertAt)
	{
		if (dynamic_cast<CPrimZone*>(primitive) != NULL)
		{
			CPrimZone	*prim = static_cast<CPrimZone*>(primitive);
			uint	i;
			for (i=0; i<prim->VPoints.size(); ++i)
			{
				StartPoints.insert(multimap<string, CVectorD>::value_type(insertAt, prim->VPoints[i]));
			}
		}
		else if (dynamic_cast<CPrimPoint*>(primitive) != NULL)
		{
			CPrimPoint	*prim = static_cast<CPrimPoint*>(primitive);
			StartPoints.insert(multimap<string, CVectorD>::value_type(insertAt, prim->Point));
		}
		else if (dynamic_cast<CPrimPath*>(primitive) != NULL)
		{
			CPrimPath	*prim = static_cast<CPrimPath*>(primitive);
			uint	i;
			for (i=0; i<prim->VPoints.size(); ++i)
			{
				StartPoints.insert(multimap<string, CVectorD>::value_type(insertAt, prim->VPoints[i]));
			}
		}

		// parse children
		uint	i;
		for (i=0; i<primitive->getNumChildren(); ++i)
		{
			IPrimitive	*child;

			if (!primitive->getChild(child, i))
				continue;

			readPrimitive(child, insertAt);
		}
	}


	void	readStartupPrims(const std::string &continent)
	{
		pair<multimap<string, string>::iterator, multimap<string, string>::iterator>	rng;
		rng = PrimFiles.equal_range(continent);

		multimap<string, string>::iterator	it;
		for (it=rng.first; it!=rng.second; ++it)
		{
			string	prim = (*it).second;

			CIFile		f(CPath::lookup(prim));
			CIXml		xml;

			CPrimitives	prims;

			// load xml file
			xml.init(f);
			nlinfo("Loaded prim file '%s'", prim.c_str());

			// read nodes
			if (!prims.read(xml.getRootNode(), prim.c_str(), LigoConfig))
			{
				nlwarning("Can't use primitive file '%s', xml parse error",  prim.c_str());
				continue;
			}
		}
	}



public:

	CPacsCruncher() : _Bank(NULL), _Retriever(NULL), _Container(NULL)
	{
	}

	void	initPackedSheets()
	{
		if (!PathInitialized)
		{
			uint	i;
			for (i=0; i<PacsPrimPath.size(); ++i)
				_Continents.initPacsPrim(PacsPrimPath[i]);
			for (i=0; i<LookupPath.size(); ++i)
				CPath::addSearchPath(LookupPath[i], true, false);
			for (i=0; i<LookupNoRecursePath.size(); ++i)
				CPath::addSearchPath(LookupNoRecursePath[i], false, false);

			PathInitialized = true;
		}

		_Continents.init(0, 0, 32.0f, 2, "./", 32.0);
	}

	void	init(const std::string &name)
	{
		initPackedSheets();
		_Continents.loadContinent(name, name, 0, true);

		_Bank = _Continents.getRetrieverBank(0);
		_Retriever = _Continents.getRetriever(0);
		_Container = _Continents.getMoveContainer(0);
	}

	void	release()
	{
		_Continents.removeContinent(0);
		_Container = NULL;
		_Retriever = NULL;
		_Bank = NULL;
	}

	string	secToString(sint sec)
	{
		return toString(sec/60)+"m"+toString(sec%60)+"s";
	}

	//
	void	crunch(const std::string &name, const CAABBox *constraintBox = NULL, const string &outputsuffix = "", const CVector *startPoint=NULL)
	{
//		nlassert(false);

		// setup pacs
		bool	keep = true;
		if (_Bank == NULL && _Retriever == NULL && _Container == NULL)
		{
			init(name);
			keep = false;
		}

		CAABBox		rBox = _Retriever->getBBox();
		nlinfo("%s rbox: %.1f,%.1f - %.1f,%.1f", name.c_str(), rBox.getMin().x, rBox.getMin().y, rBox.getMax().x, rBox.getMax().y);

		// compute new box
		CAABBox		useBox;
		if (constraintBox)
		{
			CVector	vmin, vmax;
			vmin.maxof(constraintBox->getMin(), rBox.getMin());
			vmax.minof(constraintBox->getMax(), rBox.getMax());
			useBox.setMinMax(vmin, vmax);
		}
		else
		{
			useBox = rBox;
		}

		useBox.setMinMax(CVector((float)floor(useBox.getMin().x), (float)floor(useBox.getMin().y), (float)floor(useBox.getMin().z)),
						 CVector((float)ceil(useBox.getMax().x), (float)ceil(useBox.getMax().y), (float)ceil(useBox.getMax().z)));

		nlinfo("Use Box: (%.3f,%.3f)-(%.3f,%.3f)", useBox.getMin().x, useBox.getMin().y, useBox.getMax().x, useBox.getMax().y);

		// setup the grid of surface ids per location
		_RetrieverWidth = (uint32)(useBox.getSize().x+1.0);
		_RetrieverHeight = (uint32)(useBox.getSize().y+1.0);
		_RetrieverArea = _RetrieverWidth*_RetrieverHeight;

		// clear the world map object
		_WorldMap.clear();

		// setup the UMovePrimitive
		_Primitive = _Container->addCollisionablePrimitive(0, 1);
		_Primitive->setPrimitiveType( UMovePrimitive::_2DOrientedCylinder );
		_Primitive->setReactionType( UMovePrimitive::Stop );
		_Primitive->setTriggerType((UMovePrimitive::TTrigger)UMovePrimitive::NotATrigger);
		_Primitive->setCollisionMask( 0xffffffff );
		_Primitive->setOcclusionMask( 0x00000000 );
		_Primitive->setObstacle( true );
		_Primitive->setAbsorbtion( 0 );
		_Primitive->setHeight( 6.0f );
		_Primitive->setRadius( 0.5f );

		_Primitive3 = _Container->addCollisionablePrimitive(0, 1);
		_Primitive3->setPrimitiveType( UMovePrimitive::_2DOrientedCylinder );
		_Primitive3->setReactionType( UMovePrimitive::Stop );
		_Primitive3->setTriggerType((UMovePrimitive::TTrigger)UMovePrimitive::NotATrigger);
		_Primitive3->setCollisionMask( 0xffffffff );
		_Primitive3->setOcclusionMask( 0x00000000 );
		_Primitive3->setObstacle( true );
		_Primitive3->setAbsorbtion( 0 );
		_Primitive3->setHeight( 6.0f );
		_Primitive3->setRadius( 1.5f );

		_Primitive5 = _Container->addCollisionablePrimitive(0, 1);
		_Primitive5->setPrimitiveType( UMovePrimitive::_2DOrientedCylinder );
		_Primitive5->setReactionType( UMovePrimitive::Stop );
		_Primitive5->setTriggerType((UMovePrimitive::TTrigger)UMovePrimitive::NotATrigger);
		_Primitive5->setCollisionMask( 0xffffffff );
		_Primitive5->setOcclusionMask( 0x00000000 );
		_Primitive5->setObstacle( true );
		_Primitive5->setAbsorbtion( 0 );
		_Primitive5->setHeight( 6.0f );
		_Primitive5->setRadius( 2.5f );

		_BMin = useBox.getMin();
		_BMax = useBox.getMax();

		if (BoxMin != CVectorD::Null)
		{
			if (BoxMin.x > _BMin.x)	_BMin.x = BoxMin.x;
			if (BoxMin.x > _BMin.x)	_BMin.x = BoxMin.x;
		}

		if (BoxMax != CVectorD::Null)
		{
			if (BoxMax.x < _BMax.x)	_BMax.x = BoxMax.x;
			if (BoxMax.x < _BMax.x)	_BMax.x = BoxMax.x;
		}

		_BSize = _BMax-_BMin;

		// setup a start position for the primitive
		CVectorD	zoneCentre = useBox.getCenter();
		zoneCentre.z = 0.0;

		vector<CVectorD>	startPoints;
		pair<multimap<string, CVectorD>::iterator, multimap<string, CVectorD>::iterator>	range = StartPoints.equal_range(name);
		multimap<string, CVectorD>::iterator	itsp;
		for (itsp=range.first; itsp!=range.second; ++itsp)
			startPoints.push_back((*itsp).second);

		if (startPoint != NULL)
		{
			startPoints.push_back(*startPoint);
			startPoints.back().z = 0.0;
		}

		if (DefaultStartPoint != CVectorD::Null)
		{
			startPoints.push_back(DefaultStartPoint);
			startPoints.back().z = 0.0;
		}

		if (startPoints.empty())
		{
			startPoints.push_back(zoneCentre);
		}


		// init time counter
		CGlobalRetriever	*cgr = (CGlobalRetriever*)_Retriever;

		uint	sz = 0;
		uint	ic;
		for (ic = 0; ic<cgr->getInstances().size(); ++ic)
		{
			const CRetrieverInstance	&inst = cgr->getInstance(ic);
			sz += (uint)(inst.getBBox().getHalfSize().x*inst.getBBox().getHalfSize().y*0.6f)*4;
		}

		nlinfo("Estimated %d iterations", sz);
		uint count=0;

		while (!startPoints.empty())
		{
			zoneCentre = startPoints.back();
			startPoints.pop_back();

			CMapPosition	firstIndex = CMapPosition(zoneCentre);
			zoneCentre = firstIndex.toVectorD();

			nlinfo("Start scan at %.1f,%.1f", zoneCentre.x, zoneCentre.y);

			_Positions5.clear();
			_Positions3.clear();
			_Positions1.clear();

			_Primitive->setGlobalPosition(zoneCentre, 0);
			_Container->evalCollision(1.0, 0);
			_Primitive3->setGlobalPosition(zoneCentre, 0);
			_Container->evalCollision(1.0, 0);
			_Primitive5->setGlobalPosition(zoneCentre, 0);
			_Container->evalCollision(1.0, 0);

			// setup and intialise the positions to visit stack
			_Positions5.push_back(UGlobalPosition());
			_Primitive->getGlobalPosition(_Positions5[0], 0);

			{
				TCellUnit	&slots = _WorldMap.getCellUnit(firstIndex);
				CUnitSlot	surfId(_Positions5[0]);

				// locate a slot to stick the surface in
				bool	newSlot = true;
				CSlot	slot;
				for (slot=CSlot(0); slot.isValid(); ++slot) 
				{
					CUnitSlot&	unitSlot=slots[slot.slot()];

					if (unitSlot.hasSameSurface(surfId))
					{
						newSlot = false;
						break;
					}
				}

				if (!newSlot)
					continue;

				CWorldPosition	worldPosition(_WorldMap.getWorldPositionGeneration(firstIndex,CSlot(0)));
				_WorldMap.getUnitSlot(worldPosition) = CUnitSlot(_Positions5[0]);
				_WorldMap.getUnitSlot(worldPosition).setGabarit(2);
			}

			uint	xmaxreached = 0,
					xminreached = 0,
					ymaxreached = 0,
					yminreached = 0;

			TTime	startTime = CTime::getLocalTime();

			uint	countGabarits[3] = { 0, 0, 0 };

			while (!_Positions5.empty() || !_Positions3.empty() || !_Positions1.empty())
			{
				// pop the next postion to test off the stack
				UGlobalPosition		refPos;

				if (!_Positions5.empty())
				{
					refPos = _Positions5.front();
					_Positions5.pop_front();
				}
				else if (!_Positions3.empty())
				{
					refPos = _Positions3.front();
					_Positions3.pop_front();
				}
				else
				{
					refPos = _Positions1.front();
					_Positions1.pop_front();
				}

				if (!(count&0xfff))
				{
					TTime				itime = CTime::getLocalTime();

					UGlobalPosition		nextPos = refPos;
					CVectorD			dpos = _Retriever->getDoubleGlobalPosition(nextPos);

					double				persec = (double)count*1000.0 / (double)(itime-startTime);
					sint				toGo = sz-count;
					double				ttoGo = (double)toGo/persec;
					sint				elapsed = (sint)((itime-startTime)/1000);

					nlinfo("crunchPacsMap: %d iterations (%d,%d,%d) (%dpct, estimated %s, %s to go) - start at (%d,%d,%f,%f-%f,%f)", count, countGabarits[0], countGabarits[1], countGabarits[2], count*100/sz, secToString(elapsed+(sint)ttoGo).c_str(), secToString((sint)ttoGo).c_str(), nextPos.InstanceId, nextPos.LocalPosition.Surface, nextPos.LocalPosition.Estimation.x, nextPos.LocalPosition.Estimation.y, dpos.x, dpos.y);
				}
				++count;

				if (!(count&0xffff) && count>0)
				{
					//buildBMP("temp_"+name+toString(count));
				}

				CVectorD			pos = _Retriever->getDoubleGlobalPosition(refPos);
//				EvaluatedPos = toString(refPos.InstanceId)+","+toString(refPos.LocalPosition.Surface)+":"+toString(pos.x)+","+toString(pos.y)+","+toString(pos.z);
/*
				CVectorD	d = pos - CVectorD(1045.0, -6086.0, 0.0);
				d.z = 0.0f;
				if (d.norm() < 0.5f)
					nlinfo("Bla");
*/
				CMapPosition		refIndex(pos);

				TCellUnit			&slots = _WorldMap.getCellUnit(refIndex);
				uint				slot;

				CUnitSlot			refSlot(refPos);

				for (slot=0; slot<3 && !slots[slot].hasSameSurface(refSlot); ++slot)
					;

				if (slot >= 3)
				{
					nlwarning("Failed to find point in surface slots");
					continue;
				}

				uint				gabarit = slots[slot].gabarit();

				nlassert(gabarit <= 2);
				++countGabarits[gabarit];


				// check out eastern cell and OR east move into neighbour grid
				if (pos.x + 1.0 <= _BMax.x)
					slots[slot].cellLink().setESlot(getSurfaceAfterMove(refPos, refIndex.getStepE(), gabarit));
				else
					++xmaxreached;

				// check out westtern cell and OR west move into neighbour grid
				if (pos.x - 1.0 >= _BMin.x)
					slots[slot].cellLink().setWSlot(getSurfaceAfterMove(refPos, refIndex.getStepW(), gabarit));
				else
					++xminreached;

				// check out southern cell and OR south move into neighbour grid
				if (pos.y + 1.0 <= _BMax.y)
					slots[slot].cellLink().setNSlot(getSurfaceAfterMove(refPos, refIndex.getStepN(), gabarit));
				else
					++ymaxreached;

				// check out northern cell and OR north move into neighbour grid
				if (pos.y - 1.0 >= _BMin.y)
					slots[slot].cellLink().setSSlot(getSurfaceAfterMove(refPos, refIndex.getStepS(), gabarit));
				else
					++yminreached;
			}
			nlinfo("crunchPacsMap: performed %d iterations",count);
		}


		if (count > 1)
		{
			//filter();
			//checkMap(_WorldMap, true);

			COFile		f(OutputPath+name+".wmap");

			f.serial(_WorldMap);
		}

		// HOUSEKEEPING
		_Container->removePrimitive(_Primitive);
		_Container->removePrimitive(_Primitive3);
		_Container->removePrimitive(_Primitive5);

		if (!keep)
		{
			release();
		}

		_WorldMap.clear();
	}

	void	buildGabarit(const string &name, uint gabarit)
	{
		nlinfo("building gabarit %d map...", gabarit);

		_WorldMap.clear();
		CIFile	f(OutputPath+name+".wmap");
		f.serial(_WorldMap);

		checkMap(_WorldMap);
		filter();
		checkMap(_WorldMap, true);

		CMapPosition	min, max;
		_WorldMap.getBounds(min, max);

		CMapPosition	scan, scanline;

		for (scan = min; scan.y() != max.y(); scan = scan.stepCell(0, 1))
		{
			for (scanline = scan; scanline.x()	!= max.x(); scanline = scanline.stepCell(1, 0))
			{
				//scanline = CMapPosition(0x0380, 0x2650);

				if (!_WorldMap.exist(scanline))
					continue;

				const CComputeCell	*cc = const_cast<const CWorldMap&>(_WorldMap).getComputeCellCst(scanline);

				uint	slot, i, j;

				for (i=0; i<16; ++i)
				{
					for (j=0; j<16; ++j)
					{
						CMapPosition	pos=scanline;
						pos.setUnitId(j,i);

						TCellUnit	&slots = _WorldMap.getCellUnit(pos);

						for (slot=0; slot<3; ++slot)
						{
							CUnitSlot	&unitSlot=slots[slot];

							// if slot is used but not accessible for my gabarit, reset it
							if (unitSlot.getCellLink().used() && unitSlot.gabarit()<gabarit)
							{
								CSlot	currentSlot=unitSlot.getCellLink().ESlot();
								if (currentSlot.isValid())
									_WorldMap.getUnitSlot(_WorldMap.getWorldPosition(pos.getStepE(), currentSlot)).cellLink().setWSlot(CSlot());

								currentSlot=unitSlot.getCellLink().WSlot();
								if (currentSlot.isValid())
									_WorldMap.getUnitSlot(_WorldMap.getWorldPosition(pos.getStepW(), currentSlot)).cellLink().setESlot(CSlot());

								currentSlot=unitSlot.getCellLink().NSlot();
								if (currentSlot.isValid())
									_WorldMap.getUnitSlot(_WorldMap.getWorldPosition(pos.getStepN(), currentSlot)).cellLink().setSSlot(CSlot());

								currentSlot=unitSlot.getCellLink().SSlot();
								if (currentSlot.isValid())
									_WorldMap.getUnitSlot(_WorldMap.getWorldPosition(pos.getStepS(), currentSlot)).cellLink().setNSlot(CSlot());

								unitSlot.reset();
							}
						}
					}
				}
			}
		}

		checkMap(_WorldMap);

		COFile		of(OutputPath+name+"_"+toString(gabarit)+".wmap");
		of.serial(_WorldMap);
	}

	void	buildCrunchedMap(const string &name)
	{
		nlinfo("building crunched map...");

		_WorldMap.clear();
		CIFile	f(OutputPath+name+".wmap");
		f.serial(_WorldMap);

		checkMap(_WorldMap);

		buildTopologies();
		checkMap(_WorldMap);

		nlinfo("Build map...");

		CMapPosition	min, max;
		_WorldMap.getBounds(min, max);

		CMapPosition	scan, scanline;

		for (scan = min; scan.y() != max.y(); scan = scan.stepCell(0, 1))
		{
			for (scanline = scan; scanline.x() != max.x(); scanline = scanline.stepCell(1, 0))
			{
				//scanline = CMapPosition(0x1731, 0xe2f8);

				const CComputeCell	*cell = const_cast<const CWorldMap&>(_WorldMap).getComputeCellCst(scanline);

				CRootCell	*newCell = NULL;

				if (cell == NULL)
					continue;

				CMapPosition	pos;

				uint	i, j;
				bool	failed = false;
				bool	white = true;
				for (i=0; i<16 && !failed; ++i)
				{
//					char	buff[17];
					for (j=0; j<16 && !failed; ++j)
					{
						bool	sl0 = cell->isSlotUsed(pos, CSlot(0));
						bool	sl1 = cell->isSlotUsed(pos, CSlot(1));
						bool	sl2 = cell->isSlotUsed(pos, CSlot(2));

						pos = scanline;
						pos.setUnitId(j,i);
						const TCellUnit	&cunit = cell->getCellUnitCst(pos);
						uint32	used = cell->maxUsedSlot(pos);
						if (used>=1)
							failed = true;
						else if (used>0)
							white = false;

						if (white && cunit[0].getCellLink().getLinks() != 0)
							white = false;

						if (!failed && !white && cunit[0].getCellLink().used())
						{
							if ((!cunit[0].getCellLink().isNSlotValid() && _WorldMap.isSlotUsed(pos.getStepN(), CSlot(0))) ||
								(!cunit[0].getCellLink().isSSlotValid() && _WorldMap.isSlotUsed(pos.getStepS(), CSlot(0))) ||
								(!cunit[0].getCellLink().isESlotValid() && _WorldMap.isSlotUsed(pos.getStepE(), CSlot(0))) ||
								(!cunit[0].getCellLink().isWSlotValid() && _WorldMap.isSlotUsed(pos.getStepW(), CSlot(0))))
								failed = true;
						}

						if (sl0 && (cunit[0].interior() /*|| cunit[0].water()*/ || cunit[0].topology() != 0))
							white = false;
					}

//					buff[16] = '\0';
//					nlinfo("Dump: %s", buff);
				}

				uint32	used;

				pos = scanline.getStepS();
				for (i=0; i<16 && !failed; ++i, pos = pos.getStepE())
					if ((used = _WorldMap.maxUsedSlot(pos)) >= 1)
						failed = true;
					else if (used != 0)
						white = false;

				pos = scanline.getStepW();
				for (i=0; i<16 && !failed; ++i, pos = pos.getStepN())
					if ((used = _WorldMap.maxUsedSlot(pos)) >= 1)
						failed = true;
					else if (used != 0)
						white = false;

				pos = scanline.stepCell(0, 1);
				for (i=0; i<16 && !failed; ++i, pos = pos.getStepE())
					if ((used = _WorldMap.maxUsedSlot(pos)) >= 1)
						failed = true;
					else if (used != 0)
						white = false;

				pos = scanline.stepCell(1, 0);
				for (i=0; i<16 && !failed; ++i, pos = pos.getStepN())
					if ((used = _WorldMap.maxUsedSlot(pos)) >= 1)
						failed = true;
					else if (used != 0)
						white = false;

				if (!failed && white)
				{
					// replace the compute cell by a white cell
					CWhiteCell	*wcell = new CWhiteCell(_WorldMap);
					wcell->setTopologiesNodes(cell->getTopologiesNodes());

					// build height map
					CFull16x16Layer		*heightMap = new CFull16x16Layer();
					for (i=0; i<16; ++i)
						for (j=0; j<16; ++j)
							heightMap->set(i, j, cell->getHeight(_WorldMap.getWorldPositionGeneration(CMapPosition(j, i),CSlot(0))));

					wcell->setHeightMap(I16x16Layer::compress(heightMap, 0x7fffffff));

					newCell = wcell;

//					_WorldMap.setRootCell(scanline, wcell);
				}
				else if (!failed)
				{
					// replace the compute cell by a single layer cell
					CSingleLayerCell	*slcell = new CSingleLayerCell(_WorldMap);

					uint	maxtopo = 0;

					for (i=0; i<16; ++i)
					{
						for (j=0; j<16; ++j)
						{
							pos = scanline;
							pos.setUnitId(j,i);							

							sint	maxs = cell->maxUsedSlot(pos);
							nlassert(maxs == 0 || maxs == -1);
							slcell->setPos(pos, cell->isSlotUsed(pos, CSlot(0)));

							uint	topology=cell->getTopology(_WorldMap.getWorldPositionGeneration(pos,CSlot(0)));
							if (topology>maxtopo)
								maxtopo=topology;
						}

					}

					if (maxtopo == 0)
					{
						slcell->setTopologies(NULL);
					}
					else if (maxtopo == 1)
					{
						C1Bit16x16Layer		*layer = new C1Bit16x16Layer();

						for (i=0; i<16; ++i)
						{
							for (j=0; j<16; ++j)
							{
								pos = scanline;
								pos.setUnitId(j,i);								

								layer->set(i, j, cell->getTopology(_WorldMap.getWorldPositionGeneration(pos,CSlot(0))));
							}
						}

						slcell->setTopologies(layer);
					}
					else
					{
						C8Bits16x16Layer	*layer = new C8Bits16x16Layer();

						for (i=0; i<16; ++i)
						{
							for (j=0; j<16; ++j)
							{
								pos = scanline;
								pos.setUnitId(j,i);

								layer->set(i, j, cell->getTopology(_WorldMap.getWorldPositionGeneration(pos,CSlot(0))));
							}
						}

						slcell->setTopologies(layer);
					}

					pos = scanline.getStepS();
					for (i=0; i<16; ++i, pos = pos.getStepE())
					{
						sint	maxs = cell->maxUsedSlot(pos);
						nlassert(maxs == 0 || maxs == -1);
						slcell->setSLink(i, _WorldMap.maxUsedSlot(pos) == 0);
					}

					pos = scanline.getStepW();
					for (i=0; i<16 && !failed; ++i, pos = pos.getStepN())
					{
						sint	maxs = cell->maxUsedSlot(pos);
						nlassert(maxs == 0 || maxs == -1);
						slcell->setWLink(i, _WorldMap.maxUsedSlot(pos) == 0);
					}

					pos = scanline.stepCell(0, 1);
					for (i=0; i<16 && !failed; ++i, pos = pos.getStepE())
					{
						sint	maxs = cell->maxUsedSlot(pos);
						nlassert(maxs == 0 || maxs == -1);
						slcell->setNLink(i, _WorldMap.maxUsedSlot(pos) == 0);
					}

					pos = scanline.stepCell(1, 0);
					for (i=0; i<16 && !failed; ++i, pos = pos.getStepN())
					{
						sint	maxs = cell->maxUsedSlot(pos);
						nlassert(maxs == 0 || maxs == -1);
						slcell->setELink(i, _WorldMap.maxUsedSlot(pos) == 0);
					}

					slcell->setTopologiesNodes(cell->getTopologiesNodes());

					// build height map
					CFull16x16Layer		*heightMap = new CFull16x16Layer();
					for (i=0; i<16; ++i)
						for (j=0; j<16; ++j)
							heightMap->set(i, j, slcell->isSlotUsed(CMapPosition(j, i), CSlot(0)) ? cell->getHeight(_WorldMap.getWorldPositionGeneration(CMapPosition(j,i),CSlot(0))) : 
																									0x7fffffff);

					slcell->setHeightMap(I16x16Layer::compress(heightMap, 0x7fffffff));

					newCell = slcell;

//					_WorldMap.setRootCell(scanline, slcell);
				}
				else
				{
					// build multi layer

					CMultiLayerCell		*mlcell = new CMultiLayerCell(_WorldMap);

					uint32	slot;

					for (i=0; i<16; ++i)
					{
						for (j=0; j<16; ++j)
						{
							pos = scanline;
							pos.setUnitId(j,i);							

							for (slot=0; slot<3; ++slot)
							{
								CWorldPosition	wp = _WorldMap.getWorldPositionGeneration(pos,CSlot(slot));
								const CUnitSlot	&uslot = cell->getUnitSlotCst(wp);
								if (uslot.getCellLink().used())
								{
									mlcell->setLinks(wp, uslot.getCellLink());
									mlcell->setTopology(wp, uslot.topology());
								}
							}
						}
					}

					// build height map
					CFull16x16Layer		*heightMap[3] = { NULL, NULL, NULL };
					for (i=0; i<16; ++i)
					{
						for (j=0; j<16; ++j)
						{
							for (slot=0; slot<3; ++slot)
							{
								CFull16x16Layer*	heightMapPt=heightMap[slot];
								
								if (mlcell->isSlotUsed(CMapPosition(j, i), CSlot(slot)))
								{
									if (!heightMapPt)
									{
										heightMapPt=new CFull16x16Layer();
										heightMap[slot]=heightMapPt;
									}
									nlassert(heightMapPt);
									heightMapPt->set(i, j, cell->getHeight(_WorldMap.getWorldPositionGeneration(CMapPosition(j, i), CSlot(slot))));
								}
								else
								{
									if (heightMapPt)
										heightMapPt->set(i, j, 0x7fffffff);
								}
							}
						}
					}
					
					for (slot=0; slot<3; ++slot)
						if (heightMap[slot] != NULL)
							mlcell->setHeightMap(CSlot(slot), I16x16Layer::compress(heightMap[slot], 0x7fffffff));

					mlcell->setTopologiesNodes(cell->getTopologiesNodes());

					newCell = mlcell;
				}

				nlassert(newCell != NULL);

				pos = scanline;
				uint32	slot;
				for (i=0; i<16; ++i)
				{
					for (j=0; j<16; ++j)
					{
						pos.setUnitId(j, i);

						for (slot=0; slot<3; ++slot)
						{
							CWorldPosition	wpos = _WorldMap.getWorldPositionGeneration(pos, CSlot(slot));
							bool	sused = cell->isSlotUsed(pos, CSlot(slot)),
									nsused = newCell->isSlotUsed(pos, CSlot(slot));
							nlassert(sused == nsused);

							if (cell->isSlotUsed(pos, CSlot(slot)))
							{
								CCellLinkage	lnk=cell->getCellLink(wpos),
												nlnk=newCell->getCellLink(wpos);
								uint			top=cell->getTopology(wpos),
												ntop=newCell->getTopology(wpos);
								if (failed || !white)
									nlassert(cell->getCellLink(wpos).getLinks() == newCell->getCellLink(wpos).getLinks());
								nlassert(cell->getHeight(wpos) == newCell->getHeight(wpos));
								nlassert(cell->getTopology(wpos) == newCell->getTopology(wpos));
							}
						}
					}
				}

				_WorldMap.setRootCell(scanline, newCell);
			}
		}

		checkMap(_WorldMap);

		buildMotionLayers();

//		nlassert(false);
		COFile	out(OutputPath+name+".cwmap2");
		out.serial(_WorldMap);
	}

	void	checkMap(CWorldMap &wmap, bool fix=false)
	{
		nlinfo("Checking Wmap link integrity...");

		CMapPosition	min, max;
		_WorldMap.getBounds(min, max);

		CMapPosition	posy;

		for (posy=min; posy.y()<max.y(); posy=posy.step(0, 1))
		{
			CMapPosition	posx;
			for (posx=posy; posx.x()<max.x(); posx=posx.step(1, 0))
			{
				uint	s;
				for (s=0; s<3; ++s)
				{
					CWorldPosition	wpos = wmap.getSafeWorldPosition(posx, CSlot(s));

					if (!wpos.isValid())
						continue;

					CWorldPosition	test;

					//
					if (wpos.getCellLinkage().isNSlotValid())
					{
						CSlot	slot = wpos.getCellLinkage().NSlot();
						test = wmap.getSafeWorldPosition(CMapPosition(wpos).getStepN(), slot);
						bool	failed = true;
						if (!test.isValid())
						{
							//nlwarning("Check: invalid N link at (%04X,%04X,%d)", wpos.x(), wpos.y(), s);
						}
						else if (!test.getCellLinkage().isSSlotValid())
						{
							//nlwarning("Check: N slot of (%04X,%04X,%d) has invalid S link", wpos.x(), wpos.y(), s);
							if (fix)
							{
								wmap.getUnitSlot(test).cellLink().setSSlot(CSlot(s));
								failed = false;
							}
						}
						else if (test.getCellLinkage().SSlot().slot() != s)
						{
							//nlwarning("Check: N slot of (%04X,%04X,%d) points to another slot at S", wpos.x(), wpos.y(), s);
						}
						else
						{
							failed = false;
						}
						if (failed && fix)
							wmap.resetUnitSlotNLink(wpos);
					}

					//
					if (wpos.getCellLinkage().isSSlotValid())
					{
						CSlot	slot = wpos.getCellLinkage().SSlot();
						test = wmap.getSafeWorldPosition(CMapPosition(wpos).getStepS(), slot);
						bool	failed = true;
						if (!test.isValid())
						{
							//nlwarning("Check: invalid S link at (%04X,%04X,%d)", wpos.x(), wpos.y(), s);
						}
						else if (!test.getCellLinkage().isNSlotValid())
						{
							//nlwarning("Check: S slot of (%04X,%04X,%d) has invalid N link", wpos.x(), wpos.y(), s);
							if (fix)
							{
								wmap.getUnitSlot(test).cellLink().setNSlot(CSlot(s));
								failed = false;
							}
						}
						else if (test.getCellLinkage().NSlot().slot() != s)
						{
							//nlwarning("Check: S slot of (%04X,%04X,%d) points to another slot at N", wpos.x(), wpos.y(), s);
						}
						else
						{
							failed = false;
						}
						if (failed && fix)
							wmap.resetUnitSlotSLink(wpos);
					}

					//
					if (wpos.getCellLinkage().isESlotValid())
					{
						CSlot	slot = wpos.getCellLinkage().ESlot();
						test = wmap.getSafeWorldPosition(CMapPosition(wpos).getStepE(), slot);
						bool	failed = true;
						if (!test.isValid())
						{
							//nlwarning("Check: invalid E link at (%04X,%04X,%d)", wpos.x(), wpos.y(), s);
						}
						else if (!test.getCellLinkage().isWSlotValid())
						{
							//nlwarning("Check: E slot of (%04X,%04X,%d) has invalid W link", wpos.x(), wpos.y(), s);
							if (fix)
							{
								wmap.getUnitSlot(test).cellLink().setWSlot(CSlot(s));
								failed = false;
							}
						}
						else if (test.getCellLinkage().WSlot().slot() != s)
						{
							//nlwarning("Check: E slot of (%04X,%04X,%d) points to another slot at W", wpos.x(), wpos.y(), s);
						}
						else
						{
							failed = false;
						}
						if (failed && fix)
							wmap.resetUnitSlotELink(wpos);
					}

					//
					if (wpos.getCellLinkage().isWSlotValid())
					{
						CSlot	slot = wpos.getCellLinkage().WSlot();
						test = wmap.getSafeWorldPosition(CMapPosition(wpos).getStepW(), slot);
						bool	failed = true;
						if (!test.isValid())
						{
							//nlwarning("Check: invalid W link at (%04X,%04X,%d)", wpos.x(), wpos.y(), s);
						}
						else if (!test.getCellLinkage().isESlotValid())
						{
							//nlwarning("Check: W slot of (%04X,%04X,%d) has invalid E link", wpos.x(), wpos.y(), s);
							if (fix)
							{
								wmap.getUnitSlot(test).cellLink().setESlot(CSlot(s));
								failed = false;
							}
						}
						else if (test.getCellLinkage().ESlot().slot() != s)
						{
							//nlwarning("Check: W slot of (%04X,%04X,%d) points to another slot at E", wpos.x(), wpos.y(), s);
						}
						else
						{
							failed = false;
						}
						if (failed && fix)
							wmap.resetUnitSlotWLink(wpos);
					}
				}
			}
		}
	}

	//
	void	clearHeightMap(const string &name, uint gabarit)
	{
		nlinfo("clearing height map for '%s'", (name+"_"+toString(gabarit)+".cwmap2").c_str());

//		nlassert(false);
		_WorldMap.clear();
		CIFile	fi(OutputPath+name+"_"+toString(gabarit)+".cwmap2");
		fi.serial(_WorldMap);
		fi.close();

		_WorldMap.clearHeightMap();

		COFile	fo(OutputPath+name+"_"+toString(gabarit)+".cwmap2");
		fo.serial(_WorldMap);
		fo.close();
	}

	//
	void	filter()
	{
		nlinfo("filtering...");

		CMapPosition	min, max;
		_WorldMap.getBounds(min, max);

		CMapPosition	scanpos = min;
		uint			x, y;

		uint	scanWidth = max.x()-min.x();
		uint	scanHeight = max.y()-min.y();

		for (y=0; y<scanHeight; ++y)
		{
			CMapPosition	pos(scanpos);

			for (x=0; x<scanWidth; ++x)
			{
				if (_WorldMap.exist(pos))
				{
					TCellUnit			&slots = _WorldMap.getCellUnit(pos);

					if (slots[0].used() && slots[1].used() && !slots[2].used() && abs(sint(slots[0].height())-sint(slots[1].height()))<2)
					{
						CMapPosition	np;
						if ((!_WorldMap.exist(np = pos.getStepS()) || _WorldMap.nbUsedSlots(np) <= 1) &&
							(!_WorldMap.exist(np = pos.getStepN()) || _WorldMap.nbUsedSlots(np) <= 1) &&
							(!_WorldMap.exist(np = pos.getStepE()) || _WorldMap.nbUsedSlots(np) <= 1) &&
							(!_WorldMap.exist(np = pos.getStepW()) || _WorldMap.nbUsedSlots(np) <= 1))
						{
							slots[1].reset();							
							_WorldMap.getUnitSlot(_WorldMap.getWorldPosition(pos.getStepW(),CSlot(0))).cellLink().setESlot(CSlot(0));
							_WorldMap.getUnitSlot(_WorldMap.getWorldPosition(pos.getStepE(),CSlot(0))).cellLink().setWSlot(CSlot(0));
							_WorldMap.getUnitSlot(_WorldMap.getWorldPosition(pos.getStepS(),CSlot(0))).cellLink().setNSlot(CSlot(0));
							_WorldMap.getUnitSlot(_WorldMap.getWorldPosition(pos.getStepN(),CSlot(0))).cellLink().setSSlot(CSlot(0));
						}

					}
				}
				pos = pos.getStepE();
			}
			scanpos = scanpos.getStepN();
		}
	}

	//
	void	buildTopologies()
	{
//		nlassert(false);		
		nlinfo("building topologies...");


		CMapPosition	min, max;
		_WorldMap.getBounds(min, max);

		CMapPosition	scan, scanline;

		uint			maxtopo = 0;
		vector<uint>	toposcount;

		struct	CTopoGrid
		{
			sint	getTopo(const	CSlot	&slot)	const
			{
				return	topos[slot.slot()];
			}
			sint	topos[3];
			bool	testGrid;
		};
		CTopoGrid	toposGridList[16][16];

		for (scan = min; scan.y() != max.y(); scan = scan.stepCell(0, 1))
		{
			for (scanline = scan; scanline.x() != max.x(); scanline = scanline.stepCell(1, 0))
			{
				if (!_WorldMap.exist(scanline))
					continue;

				//scanline = CMapPosition(0x02e0, 0xd440);

				CComputeCell	*cell =	_WorldMap.getComputeCell(scanline);

				{
					uint	i;
					for (i=0; i<16*16; ++i)
					{
						toposGridList[0][i].topos[0]	=
						toposGridList[0][i].topos[1]	=
						toposGridList[0][i].topos[2]	=	-1;
					}
				}

				uint	i = 0;	// current topo

				CMapPosition	sp = scanline;
				sp.setUnitId(0,0);

				uint	spx, spy, spslot;

				for (spy=0; spy<16; ++spy)
				{
					for (spx=0; spx<16; ++spx)
					{
						sp.setUnitId(spx, spy);

						for (spslot=0; spslot<3; ++spslot)
						{
							if (!cell->isSlotUsed(sp, CSlot(spslot)))
								continue;

							CTopoGrid	&topoGrid=toposGridList[sp.yCoord().getUnitId()][sp.xCoord().getUnitId()];
							sint		&curTopos=topoGrid.topos[spslot];

							if (curTopos<0)
							{

								{	//	-- clean flood fill table --
									uint	gi, gj;
									for (gi=0; gi<16; ++gi)
										for (gj=0; gj<16; ++gj)
											toposGridList[gi][gj].testGrid=false;
								}	//	----------------------------

								topoGrid.testGrid=true;
								curTopos=i;
								
								CUnitSlot	&unitslot = _WorldMap.getUnitSlot(_WorldMap.getWorldPosition(sp,CSlot(spslot)));

								unitslot.setTopology(i);

								bool		interior = unitslot.interior();
								bool		water = unitslot.water();
								bool		nogo = unitslot.nogo();

								CVector		totalPos(0.0f, 0.0f, 0.0f);
								uint		totalElm = 0;

								vector<CWorldPosition>	stack;
								stack.push_back(_WorldMap.getWorldPosition(sp, CSlot(spslot)));

								bool		topoHasNeighb = false;

								while (!stack.empty())
								{
									CWorldPosition	wp(stack.back());
									stack.pop_back();

									totalPos+=CVector(wp.toVectorD());
									++totalElm;
									
									static const CDirection::TDirection	dirs[] =
									{
										CDirection::W,
										CDirection::E,
										CDirection::S,
										CDirection::N
									};


									for (uint	dir=0; dir<4; ++dir)
									{
										const	CDirection	direction=CDirection(dirs[dir]);
										CWorldPosition	tm(wp);
										uint	x = tm.xCoord().getUnitId()+direction.dx();
										uint	y = tm.yCoord().getUnitId()+direction.dy();

										//InfoLog->displayRaw("Test move from %4X,%4X,%d to %4X,%4X", tm.x(), tm.y(), tm.slot(), tm.x()+direction.dx(), tm.y()+direction.dy());

										// store if move succeded
										bool	mvres = _WorldMap.move(tm, direction);

/*
										bool	test1 = false;
										bool	test2 = false;
										bool	test3 = false;
										bool	test4 = false;
										bool	test5 = false;
										bool	test6 = false;
										bool	test7 = false;

										// check move is on same cell, and position hasn't been visited (nor in same topo or in another)
										// and moves in same kind of floor
										if (   (test1 = ((x&0xf0)==0))
											&& (test2 = ((y&0xf0)==0))
											&& (test3 = (!toposGridList[y][x].testGrid))
											&& (test4 = (mvres))
											&& (test5 = (toposGridList[y][x].topos[tm.slot()]<0))
											&& (test6 = (_WorldMap.getUnitSlot(tm).interior() == interior))
											&& (test7 = (_WorldMap.getUnitSlot(tm).water() == water)))
*/

										if (		((x&0xf0)==0)
												&&	((y&0xf0)==0)
												&&	(!toposGridList[y][x].testGrid)
												&&	(mvres)
												&&	(toposGridList[y][x].topos[tm.slot()]<0)
												&&	(_WorldMap.getUnitSlot(tm).interior() == interior)
												&&	(_WorldMap.getUnitSlot(tm).water() == water)
												&&	(_WorldMap.getUnitSlot(tm).nogo() == nogo)			)
										{
											toposGridList[y][x].testGrid=true;
											toposGridList[y][x].topos[tm.slot()] = i;
											_WorldMap.getUnitSlot(tm).setTopology(i);
											stack.push_back(tm);
											// fakes move result so if on same topo, doesn't add a neighbour
											mvres = false;

											//InfoLog->displayRawNL(" SUCCESS");
										}
										else
										{
											//InfoLog->displayRawNL(" FAILED (test1=%d test2=%d test3=%d test4=%d test5=%d test6=%d test7=%d)", test1, test2, test3, test4, test5, test6, test7);
										}

										if (mvres)
											topoHasNeighb = true;
									}
								}

								if (topoHasNeighb)
								{
									CTopology	&topology = cell->getTopologyNode(i);
									topology.Id = CTopology::TTopologyId(scanline, i);
									topology.Position = totalPos/(float)totalElm;
									topology.Flags = (interior ? Interior : 0) | (water ? Water : 0) | (nogo ? NoGo : 0);

									//nlinfo("Topology %08X %04X", topology.Id.getVal(), topology.Flags);

									++i;
								}
								else
								{
									nlinfo("Unactivated topo %d in cell (%4X,%4X)",i, scanline.x()&0xffff, scanline.y()&0xffff);
									uint	i, j, s;
									for (i=0; i<16; ++i)
									{
										for (j=0; j<16; ++j)
										{
											for (s=0; s<3; ++s)
											{
												if (toposGridList[i][j].topos[s] == (sint)i)
												{
													CMapPosition	errp = scanline;
													errp.setUnitId(j, i);
													_WorldMap.resetUnitSlot(_WorldMap.getWorldPosition(errp, CSlot(s)));
													toposGridList[i][j].topos[s] = 0xffff;
												}
											}
										}
									}
								}
							}					
						}
					}
				}

				if (i>50)
					nlstop;

				if (i>maxtopo)
					maxtopo = i;

				if (i >= toposcount.size())
					toposcount.resize(i+1);

				toposcount[i]++;
			}
		}

		nlinfo("Found %d topologies maximum", maxtopo);
		uint	i;
		for (i=0; i<toposcount.size(); ++i)
			nlinfo("%d cells of %d topologies", toposcount[i], i);
	}

	class	CMotionTrace
	{
	public:
		CMotionTrace()	:	dx(0), dy(0), distance(0xffffffff), flags(0)
		{
		}
		sint8	dx;
		sint8	dy;
		uint32	distance;
		uint8	flags;
	};

	//
	void	buildMotionLayers()
	{
		nlinfo("building motion layers...");

		CMapPosition	min, max;
		_WorldMap.getBounds(min, max);

		CMapPosition	scan, scanline;

		uint	compute = 0, white = 0, simple = 0, multi = 0, other = 0;
		_WorldMap.countCells(compute, white, simple, multi, other);
		uint	total = compute+white+simple+multi+other;

		uint	compCells = 0;
		TTime	startTime = CTime::getLocalTime();

		//
		//min = CMapPosition(0x3a40, 0x8330);
		//max = CMapPosition(0x3a50, 0x8340);

		for (scan = min; scan.y() != max.y(); scan = scan.stepCell(0, 1))
		{
			for (scanline = scan; scanline.x() != max.x(); scanline = scanline.stepCell(1, 0))
			{
				if (CTime::getLocalTime() - startTime > 5000)
				{
					startTime = CTime::getLocalTime();
					nlinfo("buildMotionLayers: %d pct done...", compCells*100/total);
				}

				//
				//scanline = CMapPosition(0x3A40, 0x8330);

				if (!_WorldMap.exist(scanline))
					continue;

				//if (scanline.x() == 4436 && scanline.y() == -4240)
				//	nlstop;

				++compCells;

				CRootCell	*cell = _WorldMap.getRootCell(scanline);
				uint		topo = 0;
				uint		maxtopo = 0;
				const	CMapPosition	mincell = scanline.stepCell(-1, -1),
										maxcell = scanline.stepCell( 2,  2);

				while (true)
				{
					CMotionTrace	fillGrid[16*3][16*3][3];

					bool	found = false;
					uint	stx = 0, sty = 0;
					uint32	stsl=0;
					CMapPosition	sp = scanline;
					const	CMapPosition		soffset = sp.stepCell(-1, -1);

					set<CTopology::TTopologyRef>	visited;

					const	CTopology::TTopologyId	startTId(scanline, topo);

					for (sty=0; sty<16; ++sty)
					{
						for (stx=0; stx<16; ++stx)
						{
							sp.setUnitId(stx,sty);

							for (stsl=0; stsl<3; ++stsl)
							{
								if (!cell->isSlotUsed(sp, CSlot(stsl)))
									continue;

								CWorldPosition	stwp = _WorldMap.getWorldPosition(sp, CSlot(stsl));
								const uint		topology=cell->getTopology(stwp);

								if (topology>maxtopo && topology<0x80)
									maxtopo=topology;

								// find a point in the current topo
								if	(topology==topo && fillGrid[sty+16][stx+16][stsl].distance == 0xffffffff)
								{
									// this point exists, is not marked in fill grid and belongs to the current topo
									found = true;

									//
									// HERE: use multimap instead of vector so the position stack is always sorted, and thus
									//       the flood fill should be minimal (Dijkstra like graph route)
									//
									//vector<CWorldPosition>	stack;
									multimap<sint, CWorldPosition>	stacks[5];

									// mark start point
									CMotionTrace	&first = fillGrid[sp.y()-soffset.y()][sp.x()-soffset.x()][stsl];
									first.dx = 0;
									first.dy = 0;
									first.distance = 0;
									const CTopology&	topNode = stwp.getTopologyRef().getCstTopologyNode();
									first.flags = (topNode.isInWater() ? 1 : 0) + (topNode.isInNogo() ? 2 : 0);

									// push first position
									stacks[first.flags].insert(make_pair<sint, CWorldPosition>(first.flags, stwp));

									while (true)
									{
										uint	stack;
										for (stack=0; stack<5; ++stack)
											if (!stacks[stack].empty())
												break;
										if (stack == 5)
											break;

										CWorldPosition	next((*(stacks[stack].begin())).second);
										stacks[stack].erase(stacks[stack].begin());

										const CTopology&	nextNode = next.getTopologyRef().getCstTopologyNode();
										uint8				nextflags = (nextNode.isInWater() ? 1 : 0) + (nextNode.isInNogo() ? 2 : 0);

										CTopology::TTopologyRef	nextTId(next);

										sint	cdist = fillGrid[next.y()-soffset.y()][next.x()-soffset.x()][next.slot()].distance;
																				
										if	(	nextTId != startTId
											&&	cdist<=CDirection::MAX_COST) // check if it is a touching topology
											visited.insert(nextTId);

										uint	dir;
										for (dir=0; dir<8; ++dir)
										{
											CWorldPosition	tmp(next);

											CDirection	direction((CDirection::TDirection)dir);

											if	(_WorldMap.moveSecure(tmp,direction))
											{
												// don't move more than 1 cell
												if	(	tmp.x() <	mincell.x()
													||	tmp.x() >=	maxcell.x()
													||	tmp.y() <	mincell.y()
													||	tmp.y() >=	maxcell.y())
													continue;

												// check we can move back
												CDirection	opp = direction;
												opp.addStep(CDirection::HALF_TURN);
												CWorldPosition	checkBackMove(tmp);
												if (!_WorldMap.move(checkBackMove, opp))
												{
													nlwarning("Can't move back from pos (%d,%d,%d) to pos (%d,%d,%d)",
																next.x(), next.y(), next.slot(),
																tmp.x(), tmp.y(), tmp.slot());
													continue;
												}
												else if (checkBackMove != next)
												{

													if (Verbose)
														nlwarning("Reverse move from pos (%d,%d,%d) to pos (%d,%d,%d) gave a different path",
																	next.x(), next.y(), next.slot(),
																	tmp.x(), tmp.y(), tmp.slot());
													continue;
												}

												const CTopology&	tmpNode = tmp.getTopologyRef().getCstTopologyNode();
												uint8				tmpflags = (tmpNode.isInWater() ? 1 : 0) + (tmpNode.isInNogo() ? 2 : 0);
/*
												if (tmpNode.Id != nextNode.Id)
													nlinfo("tamere");

												if (tmpflags != 0)
													nlinfo("blabla");
*/

												uint				ndist = (_WorldMap.getTopology(tmp)==topo && tmp.hasSameFullCellId(sp) ) ? 0 : cdist+direction.getWeight();

												CMotionTrace&		motionTrace	= fillGrid[tmp.y()-soffset.y()][tmp.x()-soffset.x()][tmp.slot()];

												uint				forbid = nextflags & (~tmpflags);
												if ((motionTrace.distance == 0xffffffff) ||
													(forbid == 0 && motionTrace.distance > ndist))
												{
													/*nlinfo("stack=%d ndist=%d move from (%04X,%04X,%d - topo=%08X,flags=%d) to (%04X,%04X,%d - topo=%08X,flags=%d) - %d %d %d %d",
																		stack, ndist,
																		next.x(), next.y(), next.slot(), nextNode.Id.getVal(), nextflags,
																		tmp.x(), tmp.y(), tmp.slot(), tmpNode.Id.getVal(), tmpflags,
																		stacks[0].size(), stacks[1].size(), stacks[2].size(), stacks[3].size());*/

													motionTrace.distance = ndist;
													if (ndist == 0)
													{
														motionTrace.dx =
														motionTrace.dy =	0;
													}
													else
													{
														motionTrace.dx=-direction.dx();
														motionTrace.dy=-direction.dy();
													}
													if (tmp.getTopologyNode().Id == topNode.Id)
													{
														stacks[0].insert(make_pair<sint, CWorldPosition>(ndist, tmp));
													}
													else
													{
														stacks[tmpflags+1].insert(make_pair<sint, CWorldPosition>(ndist, tmp));
													}
												}										
											}
										}
									}
								}
							}
						}
					}

					if (found)
					{
						// generate direction map for this topo
						CDirectionMap	*dmap = new CDirectionMap();

						uint	x, y, s;
						for (s=0; s<3; ++s)
						{
							CDirectionLayer*		directionLayer=dmap->Layers[s];

							for (y=0; y<16*3; ++y)
							{
//								char	output[256];
								for (x=0; x<16*3; ++x)
								{
//									static const char	ht[]= "0123456789ABCDEF";
									CMotionTrace	&motionTrace=fillGrid[y][x][s];
									
//									output[x] = ht[motionTrace.distance&15];
									
									if (motionTrace.distance != 0xffffffff)
									{
										//	create CDirectionLayer if it do not exists
										if	(!directionLayer)
										{
											directionLayer	= new CDirectionLayer();
											nlassert(directionLayer!=NULL);
											dmap->Layers[s] = directionLayer;
										}							

										CGridDirectionLayer*	dirLayer=directionLayer->getGridLayer(y>>4,x>>4);

										//	create CGridDirectionLayer if it do not exists
										if (!dirLayer)
										{
											directionLayer->setGridLayer(y>>4,x>>4,	new C4Bits16x16Layer());
											dirLayer=directionLayer->getGridLayer(y>>4,x>>4);
											nlassert(dirLayer);
										}
										dirLayer->setDirection(y&15, x&15,	(motionTrace.dx == 0 && motionTrace.dy == 0 ? CDirection(CDirection::UNDEFINED) : CDirection(motionTrace.dx,motionTrace.dy)));
									}

								}
//								output[x] = '\0';
//								nlinfo("%s", output);
							}

						}

						for (s=0; s<3; ++s)
						{
							CDirectionLayer*	directionLayer=dmap->Layers[s];
							
							if	(!directionLayer)
								continue;

							for (y=0; y<3; ++y)
							{
								for (x=0; x<3; ++x)
								{
									CGridDirectionLayer	*layer=directionLayer->getGridLayer(y,x);

									if (layer==NULL)
										continue;

									uint	i, j;
									uint	cmotion = 15;
									bool	different = false;

									for (i=0; i<16 && !different; ++i)
									{
										for (j=0; j<16 && !different; ++j)
										{
											uint	nm = layer->get(i, j);
											if (cmotion != 15 && nm != 15 && nm != cmotion)
											{
												different = true;
											}
											else if (nm != 15)
											{
												cmotion = layer->get(i, j);
											}

										}

									}

									if (!different)
									{
										CWhite16x16Layer	*nlayer = new CWhite16x16Layer();
										
										nlayer->set	(0, 0, cmotion);

										delete	directionLayer->getGridLayer(y,x);
										directionLayer->setGridLayer(y,x,nlayer);
									}

								}

							}

						}

//						dmap->dump();

						cell->setDirectionMap(dmap, topo);

						CTopology	&topology = cell->getTopologyNode(topo);
						
						topology.Id = CTopology::TTopologyId(scanline, topo);
						topology.Neighbours.clear();
						
						set<CTopology::TTopologyRef>::iterator	it;
						for (it=visited.begin(); it!=visited.end(); ++it)
						{
							CTopology::TTopologyRef	topId(*it);
							const CTopology			&node = topId.getCstTopologyNode();	//	_WorldMap.getTopologyNode(topId)
							float					Norm=(topology.Position-node.Position).norm();
							topology.Neighbours.push_back(CTopology::CNeighbourLink(topId, Norm));
						}
					}
					else if (Verbose)
					{
						nlwarning("Topo %d not found, probably not accessible, left unchecked", topo);
					}

					++topo;

					if (topo > maxtopo)
						break;
				}
			}
		}

		for (scan = min; scan.y() != max.y(); scan = scan.stepCell(0, 1))
		{
			for (scanline = scan; scanline.x() != max.x(); scanline = scanline.stepCell(1, 0))
			{
				if (!_WorldMap.exist(scanline))
					continue;

				CRootCell	*cell = _WorldMap.getRootCell(scanline);
				if (cell == NULL)
					continue;

				vector<CTopology>	&tops = cell->getTopologiesNodes();
				uint	i, j;

				// for all topologies in cell
				for (i=0; i<tops.size(); ++i)
				{
					CTopology	&node = tops[i];
					vector<CTopology::CNeighbourLink>::iterator	it;

					// look for all neighbours
					for (it=node.Neighbours.begin(); it!=node.Neighbours.end();)
					{
						// and check that it is a neighbour of the neighbour
						const CTopology	&neighb = (*it).getTopologyRef().getCstTopologyNode();
						for (j=0; j<neighb.Neighbours.size(); ++j)
							if (neighb.Neighbours[j].getTopologyRef() == node.Id)
								break;

						if (j == neighb.Neighbours.size())
						{
							if (Verbose)
								nlwarning("Non bijective link between topology %08X and topology %08X, fixed", node.Id.getVal(), neighb.Id.getVal());
							it  = node.Neighbours.erase(it);
						}
						else
							++it;
					}
				}
			}
		}

		_WorldMap.checkMotionLayer();
		_WorldMap.countSuperTopo();

		_WorldMap.buildMasterTopo(false, false);
		_WorldMap.buildMasterTopo(true, false);
		_WorldMap.buildMasterTopo(false, true);
		_WorldMap.buildMasterTopo(true, true);
	}


	//
	void	buildBMP(const string &name)
	{
//		nlassert(false);
		nlinfo("building bitmap...");

		uint	compute = 0, white = 0, simple = 0, multi = 0, other = 0;
		_WorldMap.countCells(compute, white, simple, multi, other);
		uint	total = compute+white+simple+multi+other;

		nlinfo("%d cells : compute=%d, white=%d, simple=%d, multi=%d, other=%d", total, compute, white, simple, multi, other);

		nlinfo("Build bmp...");

		CMapPosition	min, max;
		_WorldMap.getBounds(min, max);

		uint	scanWidth = max.x()-min.x();
		uint	scanHeight = max.y()-min.y();

		uint	imageWidth = (scanWidth+15)&~15;
		uint	imageHeight = (scanHeight);

		CBMP4Image<2,2>::SHdr		imageHdr(imageWidth, imageHeight);
		CBMP4Image<2,2>::SPalette	imagePalette;

		FILE	*outf = fopen((OutputPath+name+".bmp").c_str(),"wb");

		if (outf == NULL)
			return;

		uint16	BM = 0x4D42;
		imagePalette.setupForCol();
		fwrite((void *)&BM, 1, sizeof(BM), outf);
		fwrite((void *)&imageHdr, 1, sizeof(imageHdr), outf);
		fwrite((void *)&imagePalette, 1, sizeof(imagePalette), outf);

		uint8	*lineBuffer = new uint8[imageWidth/2];
		memset(lineBuffer, 255, imageWidth/2);

		CMapPosition	scanpos(min.x(),min.y());
		uint			x, y;

		const CWorldMap	*wmap = &_WorldMap;

		for (y=0; y<scanHeight; ++y)
		{
			uint8			*linePtr = lineBuffer;
			uint8			pointBuffer;

			CMapPosition	pos(scanpos);

			for (x=0; x<scanWidth; ++x)
			{
				uint8	color = 0;
				sint32	colorHM = -32768;
				uint32	slot=0;

				const CRootCell	*cell = wmap->getRootCellCst(pos);

				if (cell != NULL)
				{
					uint	ns;
					for (ns=0; ns<3; ++ns)
					{
						CWorldPosition	wp = _WorldMap.getSafeWorldPosition(pos, CSlot(ns));

						if (!wp.isValid())
							continue;

						++slot;

						CCellLinkage	links = cell->getCellLink(wp);
						sint32			height = cell->getHeight(wp);
						uint8			cl = 0;

						if (!links.isESlotValid())	++cl;
						if (!links.isWSlotValid())	++cl;
						if (!links.isSSlotValid())	++cl;
						if (!links.isNSlotValid())	++cl;

						if (cl > color)
							color = cl;

						if (height > colorHM)
							colorHM = height;
					}
				}

				pointBuffer = ((pointBuffer<<4) | (color+((uint8)slot<<2)));

				if (x & 1)
				{
					*(linePtr++) = pointBuffer;
				}

				pos = pos.getStepE();
			}
			scanpos = scanpos.getStepN();

			fwrite((void*)lineBuffer, 1, imageWidth/2, outf);
		}

		fclose(outf);
		delete [] lineBuffer;
	}



	//
	void	buildBitMap(const string &name)
	{
//		nlassert(false);
		nlinfo("building bitmap...");

		_WorldMap.clear();
		string	ext = CFile::getExtension(name);
		if (ext == "")
			ext = "cwmap2";
		CIFile	f(OutputPath+CFile::getFilenameWithoutExtension(name)+"."+ext);
		f.serial(_WorldMap);

		uint	compute = 0, white = 0, simple = 0, multi = 0, other = 0;
		_WorldMap.countCells(compute, white, simple, multi, other);
		uint	total = compute+white+simple+multi+other;

		nlinfo("%d cells : compute=%d, white=%d, simple=%d, multi=%d, other=%d", total, compute, white, simple, multi, other);

		nlinfo("Build bmp...");

		CMapPosition	min, max;
		_WorldMap.getBounds(min, max);

		uint	scanWidth = max.x()-min.x();
		uint	scanHeight = max.y()-min.y();

		uint	imageWidth = (scanWidth+15)&~15;
		uint	imageHeight = (scanHeight);

		CBMP4Image<2,2>::SHdr		imageHdr(imageWidth, imageHeight);
		CBMP4Image<2,2>::SPalette	imagePalette;

		FILE	*outf = fopen((OutputPath+name+".bmp").c_str(),"wb");
		FILE	*outfh = fopen((OutputPath+name+"_hm.bmp").c_str(),"wb");

		if (outf == NULL)
			return;

		uint16	BM = 0x4D42;
		imagePalette.setupForCol();
		fwrite((void *)&BM, 1, sizeof(BM), outf);
		fwrite((void *)&imageHdr, 1, sizeof(imageHdr), outf);
		fwrite((void *)&imagePalette, 1, sizeof(imagePalette), outf);

		imagePalette.setupHueCircle();
		fwrite((void *)&BM, 1, sizeof(BM), outfh);
		fwrite((void *)&imageHdr, 1, sizeof(imageHdr), outfh);
		fwrite((void *)&imagePalette, 1, sizeof(imagePalette), outfh);

		uint8	*lineBuffer = new uint8[imageWidth/2];
		memset(lineBuffer, 255, imageWidth/2);

		uint8	*lineBufferHM = new uint8[imageWidth/2];
		memset(lineBufferHM, 255, imageWidth/2);
		
		CMapPosition	scanpos(min.x(),min.y());
		uint			x, y;

		const CWorldMap	*wmap = &_WorldMap;

		CTGAImage		tgaImage;

		tgaImage.setup((uint16)imageWidth, (uint16)imageHeight, OutputPath+name+".tga", (uint16)min.x(), (uint16)-max.y());
		tgaImage.setupForCol();

		for (y=0; y<scanHeight; ++y)
		{
			uint8			*linePtr = lineBuffer;
			uint8			*linePtrHM = lineBufferHM;
			uint8			pointBuffer = 0;
			uint8			pointBufferHM = 0;

			CMapPosition	pos(scanpos);

			for (x=0; x<scanWidth; ++x)
			{
				uint8	color = 0;
				sint32	colorHM = -32768;
				uint32	slot=0;

				const CRootCell	*cell = wmap->getRootCellCst(pos);

				if (cell != NULL)
				{
					uint	ns;
					for (ns=0; ns<3; ++ns)
					{
						CWorldPosition	wp = _WorldMap.getSafeWorldPosition(pos, CSlot(ns));

						if (!wp.isValid())
							continue;

						++slot;

						CCellLinkage	links = cell->getCellLink(wp);
						sint32			height = cell->getHeight(wp);
						uint8			cl = 0;

						if (!links.isESlotValid())	++cl;
						if (!links.isWSlotValid())	++cl;
						if (!links.isSSlotValid())	++cl;
						if (!links.isNSlotValid())	++cl;

						if (cl > color)
							color = cl;

						if (height > colorHM)
							colorHM = height;
					}
				}

				tgaImage.set(x, color+((uint8)slot<<2));

				pointBuffer = ((pointBuffer<<4) | (color+((uint8)slot<<2)));
				pointBufferHM = ((pointBufferHM<<4) | (colorHM & 0xf));

				if (x & 1)
				{
					*(linePtr++) = pointBuffer;
					*(linePtrHM++) = pointBufferHM;
				}

				pos = pos.getStepE();
			}
			scanpos = scanpos.getStepN();

			fwrite((void*)lineBuffer, 1, imageWidth/2, outf);
			fwrite((void*)lineBufferHM, 1, imageWidth/2, outfh);

			tgaImage.writeLine();
		}

		fclose(outf);
		fclose(outfh);
		delete [] lineBuffer;
		delete [] lineBufferHM;

		_WorldMap.clear();
	}


	// build a height map of sint16. Full path should be given for the input cw_map2
	void	buildHeightMap16(const string &name)
	{
//		nlassert(false);
		nlinfo("building heightmap for %s", name.c_str());

		_WorldMap.clear();
		string	ext = CFile::getExtension(name);
		if (ext == "")
			ext = "cw_map2";
		
		CIFile	f(CFile::getPath(name) +  CFile::getFilenameWithoutExtension(name)+"."+ext);
		f.serial(_WorldMap);			

		uint	compute = 0, white = 0, simple = 0, multi = 0, other = 0;
		_WorldMap.countCells(compute, white, simple, multi, other);
		uint	total = compute+white+simple+multi+other;

		nlinfo("%d cells : compute=%d, white=%d, simple=%d, multi=%d, other=%d", total, compute, white, simple, multi, other);

		nlinfo("Build bmp...");

		CMapPosition	min, max;
		_WorldMap.getBounds(min, max);

		uint	scanWidth = max.x()-min.x();
		uint	scanHeight = max.y()-min.y();

				
		
		CMapPosition	scanpos(min.x(),min.y());
		uint			x, y;

		const CWorldMap	*wmap = &_WorldMap;
		
		CArray2D<sint16> heightMap;
		heightMap.init(scanWidth, scanHeight);
		
		sint16 *dest = &heightMap(0, 0);

		for (y=0; y<scanHeight; ++y)
		{			
			CMapPosition	pos(scanpos);

			for (x=0; x<scanWidth; ++x)
			{				
				sint16	height = 0x7fff;

				const CRootCell	*cell = wmap->getRootCellCst(pos);

				if (cell != NULL)
				{
					uint	ns;
					for (ns=0; ns<3; ++ns)
					{
						CWorldPosition	wp = _WorldMap.getSafeWorldPosition(pos, CSlot(ns));
						if (!wp.isValid())
							continue;						
						sint32			newHeight = cell->getHeight(wp);												
						if (newHeight < (sint16) height)
							height = (sint16) newHeight;
					}
				}

				*dest ++ = height;
				pos = pos.getStepE();
			}
			scanpos = scanpos.getStepN();			
		}		
		// write result
		COFile output(OutputPath + CFile::getFilenameWithoutExtension(name)+".cw_height");		
		sint32 xmin = (sint32) min.x();
		sint32 xmax = (sint32) max.x();
		sint32 ymin = (sint32) (sint16) min.y();
		sint32 ymax = (sint32) (sint16) max.y();
		output.serialCheck(NELID("OBSI"));
		output.serial(xmin);
		output.serial(xmax);
		output.serial(ymin);
		output.serial(ymax);
		output.serial(heightMap);
		// TMP TMP
		/*
		CBitmap tgaHM;
		tgaHM.resize(heightMap.getWidth(), heightMap.getHeight());
		sint16 hMax = -32768;
		sint16 hMin= 32767;
		uint numPix = heightMap.getWidth() * heightMap.getHeight();
		for(CArray2D<sint16>::iterator it = heightMap.begin(); it != heightMap.end(); ++it)	
		{
			if (*it != 32767)
			{
				hMax = std::max(hMax, *it);
				hMin = std::min(hMin, *it);
			}
		}
		
		float scale = 255.f / favoid0((float) (hMax - hMin));
		float bias = (float) - hMin;
		CRGBA *destCol = (CRGBA *) &tgaHM.getPixels()[0];
		for(CArray2D<sint16>::iterator it = heightMap.begin(); it != heightMap.end(); ++it)
		{
			if (*it == 0x7fff)
			{
				*destCol++ = CRGBA::Magenta;
			}
			else
			{
				float height = scale * ((*it) + bias);
				clamp(height, 0.f, 255.f);
				*destCol++ = CRGBA((uint8) height, (uint8) height, (uint8) height);
			}
		}	
		//
		COFile tgaOut("d:\\tmp\\whole_world_height_map.tga");
		tgaHM.writeTGA(tgaOut, 0, true);
		*/
		//
		_WorldMap.clear();
	}



	//
	void	findPath(const string &name, CVectorD start, CVectorD end)
	{
		_WorldMap.clear();
		CIFile	f(OutputPath+name+".cwmap2");
		f.serial(_WorldMap);

		CAStarPath	path;
		path.setStartPos(_WorldMap.getWorldPosition(start));
		path.setEndPos(_WorldMap.getWorldPosition(end));
		_WorldMap.findAStarPath(path.getStartPos(), path.getEndPos(), path.topologiesPathForCalc());

		uint	step=0;

		CWorldPosition	cpos(_WorldMap.getWorldPosition(start));
		while (_WorldMap.move(cpos, path, step))
		{
			CVectorD		dpos = cpos.getPosition();	//	_WorldMap.getPosition(cpos)
			CMapPosition	mpos = cpos;
			nldebug("Move: (%.1f,%.1f,%.1f)/(%04x,%04x)", dpos.x, dpos.y, dpos.z, mpos.x(), mpos.y());
		}

		_WorldMap.clear();
	}

	//
	void	findInsidePath(const string &name, CVectorD start, CVectorD end)
	{
		_WorldMap.clear();
		CIFile	f(OutputPath+name+".cwmap2");
		f.serial(_WorldMap);

		CInsideAStarPath	path;
		path.setStartPos(_WorldMap.getWorldPosition(start));
		path.setEndPos(_WorldMap.getWorldPosition(end));
		_WorldMap.findInsideAStarPath(path.getStartPos(), path.getEndPos(), path.getStepPathForCalc());

		uint	step=0;
/*
		CWorldPosition	cpos = _WorldMap.getWorldPosition(start);
		while (_WorldMap.move(cpos, path, step))
		{
			CVectorD		dpos = _WorldMap.getWorldPosition(cpos);
			CMapPosition	mpos = cpos.getMapPosition();
			nldebug("Move: (%.1f,%.1f,%.1f)/(%04x,%04x)", dpos.x, dpos.y, dpos.z, mpos.x.c, mpos.y.c);
		}
*/
		_WorldMap.clear();
	}

	//
	void	testMove(const string &name, CVectorD start, CVectorD end)
	{
		_WorldMap.clear();
		CIFile	f(OutputPath+name+".cwmap2");
		f.serial(_WorldMap);

		CWorldPosition	pos(_WorldMap.getWorldPosition(start));
		CMapPosition	towards = _WorldMap.getWorldPosition(end);
		_WorldMap.move	(pos,towards, Nothing);
		_WorldMap.clear	();
	}
};

CPacsCruncher::TPacsPrimMap	CPacsCruncher::_PacsPrimMap;




NLMISC_COMMAND(setStartPoint,"Set the start point for a continent","<continent> <startx> <starty> [startz]")
{
	if (args.size() < 3)
		return false;

	CVectorD	startPoint;

	NLMISC::fromString(args[1], startPoint.x);
	NLMISC::fromString(args[2], startPoint.y);

	if (args.size() < 4)
	{
		startPoint.z = 0.0;
	}
	else
	{
		NLMISC::fromString(args[3], startPoint.z);
	}

	StartPoints.insert(multimap<string, CVectorD>::value_type(args[0], startPoint));

	nlinfo("Set start point (%.1f,%.1f,%.1f) for continent '%s'", startPoint.x, startPoint.y, startPoint.z, args[0].c_str());

	return true;
}

NLMISC_COMMAND(setBoundingBox, "Set the working bounding box", "<minx> <miny> <maxx> <maxy>")
{
	if (args.size() < 4)
		return false;

	NLMISC::fromString(args[0], BoxMin.x);
	NLMISC::fromString(args[1], BoxMin.y);
	NLMISC::fromString(args[2], BoxMax.x);
	NLMISC::fromString(args[3], BoxMax.y);

	return true;
}

NLMISC_COMMAND(setStartPrimFile,"Adds a .primitive file for a continent","<continent> <primitive file>")
{
	if (args.size() < 2)
		return false;

	string	continent = args[0];
	string	primFile = args[1];

	PrimFiles.insert(multimap<string, string>::value_type(continent, primFile));

	nlinfo("Added primitive file %s to continent '%s'", primFile.c_str(), continent.c_str());

	return true;
}

NLMISC_COMMAND(setPacsPrimPath,"the pacs prim path","<path>")
{
	if (args.size() < 1)
		return false;

	PacsPrimPath.push_back(args[0]);

	return true;
}

NLMISC_COMMAND(setDefaultStart,"Set the default start point for all continents","<startx> <starty> [startz]")
{
	if (args.size() < 2)
		return false;

	CVectorD	startPoint;

	NLMISC::fromString(args[0], startPoint.x);
	NLMISC::fromString(args[1], startPoint.y);

	if (args.size() > 2)
	{
		NLMISC::fromString(args[2], startPoint.z);
	}
	else
	{
		startPoint.z = 0.0;
	}

	DefaultStartPoint = startPoint;

	nlinfo("Set default start point (%.1f,%.1f,%.1f)", startPoint.x, startPoint.y, startPoint.z);

	return true;
}


NLMISC_COMMAND(setOutputPath,"Set the output path","<path>")
{
	if (args.size() < 1)
		return false;

	OutputPath = CPath::standardizePath(args[0]);

	return true;
}


NLMISC_COMMAND(pacsCrunch,"Run a test of pacs crunch","<file name root> [<minx> <miny> <maxx> <maxy>]")
{
	if(args.size()<1)
		return false;

	//crunchPacsMap(args[0]);

	CPacsCruncher	pc;

	if (args.size() == 5)
	{
		float	bxmin, bymin, bxmax, bymax;

		bxmin = (float)atof(args[1].c_str());
		bymin = (float)atof(args[2].c_str());
		bxmax = (float)atof(args[3].c_str());
		bymax = (float)atof(args[4].c_str());

		CAABBox	box;

		box.setMinMax(CVector(bxmin, bymin, -10000.0f), CVector(bxmax, bymax, +10000.0f));

		pc.crunch(args[0], &box);
	}
	else
	{
		pc.crunch(args[0]);
	}

	return true;
}

NLMISC_COMMAND(pacsCrunchStart,"Run a test of pacs crunch","<file name root> <startx> <starty> [<minx> <miny> <maxx> <maxy>]")
{
	if(args.size()<3)
		return false;

	//crunchPacsMap(args[0]);


	float	startx, starty;

	startx = (float)atof(args[1].c_str());
	starty = (float)atof(args[2].c_str());

	CVector	start(startx, starty, 0.0f);

	CPacsCruncher	pc;

	if (args.size() == 7)
	{
		float	bxmin, bymin, bxmax, bymax;

		bxmin = (float)atof(args[3].c_str());
		bymin = (float)atof(args[4].c_str());
		bxmax = (float)atof(args[5].c_str());
		bymax = (float)atof(args[6].c_str());

		CAABBox	box;

		box.setMinMax(CVector(bxmin, bymin, -10000.0f), CVector(bxmax, bymax, +10000.0f));

		pc.crunch(args[0], &box, "", &start);
	}
	else
	{
		pc.crunch(args[0], NULL, "", &start);
	}

	return true;
}

NLMISC_COMMAND(pacsCrunchLoop,"Run a serie of tests of pacs crunch","<file name root> <startx> <starty> <loopx> <loopy> [<size>=160]")
{
	if(args.size()<5 || args.size()>6)
		return false;

	CPacsCruncher	pc;

	float	startx, starty;
	uint	loopx, loopy;
	uint	x, y;
	float	size = 160.0f;

	startx = (float)atof(args[1].c_str());
	starty = (float)atof(args[2].c_str());
	NLMISC::fromString(args[3], loopx);
	NLMISC::fromString(args[4], loopy);

	if (args.size() >= 6)
		NLMISC::fromString(args[5], size);

	pc.init(args[0]);

	for (y=0; y<loopy; ++y)
	{
		for (x=0; x<loopx; ++x)
		{
			CAABBox	box;

			box.setMinMax(CVector(startx+x*size, starty+y*size, -10000.0f), CVector(startx+(x+1)*size, starty+(y+1)*size, +10000.0f));

			pc.crunch(args[0], &box, toString(x)+"_"+toString(y));
		}
	}

	pc.release();

	return true;
}

NLMISC_COMMAND(pacsBuildBitmap,"build a bitmap from a world map","<file name root>")
{
	if(args.size()<1)
		return false;

	CPacsCruncher	pc;

	pc.buildBitMap(args[0]);

	return true;
}

NLMISC_COMMAND(pacsBuildHeightMap16, "build a sint16 heightmap from a world map","<file name root>")
{
	if(args.size()<1)
		return false;

	CPacsCruncher	pc;

	pc.buildHeightMap16(args[0]);

	return true;
}

NLMISC_COMMAND(pacsBuildWmap,"build crunched world map from a world map","<file name root>")
{
	if(args.size()<1)
		return false;

	CPacsCruncher	pc;

	pc.buildCrunchedMap(args[0]);

	return true;
}

NLMISC_COMMAND(pacsBuildGabarit,"build gabarit maps from a world map","<file name root>")
{
	if(args.size()<1)
		return false;

	CPacsCruncher	pc;

	uint	i;
	for (i=0; i<3; ++i)
		pc.buildGabarit(args[0], i);

	return true;
}

NLMISC_COMMAND(pacsClearHeightmap,"build gabarit maps from a world map","<file name root>")
{
	if(args.size()<1)
		return false;

	CPacsCruncher	pc;

	uint	i;
	for (i=1; i<3; ++i)
		pc.clearHeightMap(args[0], i);

	return true;
}



NLMISC_COMMAND(testAstar, "test astar", "file startx starty endx endy (m)")
{
	if (args.size() < 4)
		return false;

	CPacsCruncher	pc;
	
	pc.findPath(args[0], CVectorD(atof(args[1].c_str()), atof(args[2].c_str()), 0.0), CVectorD(atof(args[3].c_str()), atof(args[4].c_str()), 0.0));

	return true;
}

NLMISC_COMMAND(testInsideAstar, "test inside astar", "file startx starty endx endy (m)")
{
	if (args.size() < 4)
		return false;

	CPacsCruncher	pc;
	
	pc.findInsidePath(args[0], CVectorD(atof(args[1].c_str()), atof(args[2].c_str()), 0.0), CVectorD(atof(args[3].c_str()), atof(args[4].c_str()), 0.0));

	return true;
}

NLMISC_COMMAND(testLine, "test linear movement", "file startx starty endx endy (m)")
{
	if (args.size() < 4)
		return false;

	CPacsCruncher	pc;
	
	pc.testMove(args[0], CVectorD(atof(args[1].c_str()), atof(args[2].c_str()), 0.0), CVectorD(atof(args[3].c_str()), atof(args[4].c_str()), 0.0));

	return true;
}



//
NLMISC_COMMAND(checkPackedSheets, "checks continents.packed_sheets file", "")
{
	// a simple pc will automatically checks for continents.packed_sheets file
	CPacsCruncher	pc;
	pc.initPackedSheets();

	return true;
}


//
NLMISC_COMMAND(loadWmap, "Loads a worldmap in static world map", "<file>")
{
	if (args.size() < 1)
		return false;

	StaticWorldMap.clear();
	CIFile	f(OutputPath+args[0]);
	f.serial(StaticWorldMap);

	return true;
}

NLMISC_COMMAND(clearWmap, "Clears static world map", "")
{
	StaticWorldMap.clear();

	return true;
}

NLMISC_COMMAND(dumpMotionLayer, "Dumps motion layer around a position", "<x> <y> <z>")
{
/*
	if (args.size() < 3)
		return false;

	CVectorD		vpos;
	CWorldPosition	pos;

	vpos.x = atof(args[0].c_str());
	vpos.y = atof(args[1].c_str());
	vpos.z = atof(args[2].c_str());

	StaticWorldMap.setWorldPosition(pos, CMapPosition(vpos));

	if (!pos.isValid())
		return false;

	CTopology::TTopologyId	topoId = StaticWorldMap.getTopologyId(pos);
	const CTopology			&topo = StaticWorldMap.getTopologyNode(topoId);

	topo.DirectionMap->dump();
*/
	return true;
}

NLMISC_COMMAND(dumpTopo, "Dumps motion layer around a position", "topoId in hexa")
{
	if (args.size() < 1)
		return false;

	uint32	id;

	sscanf(args[0].c_str(), "%x", &id);

	CTopology::TTopologyId	topoId(id);
	const CTopology			&topo = StaticWorldMap.getTopologyNode(topoId);

	topo.DirectionMap->dump();

	uint	i;
	nlinfo("%08X neighbours (%d neighbours)", id, topo.Neighbours.size());
	for (i=0; i<topo.Neighbours.size(); ++i)
		nlinfo("%d: %08X", i, topo.Neighbours[i].getTopologyRef().getVal());

	return true;
}

NLMISC_COMMAND(findTopoPath, "Find path between 2 topologies", "startTopoId endTopoId")
{
	if (args.size() < 2)
		return false;

	uint32	startId, endId;

	sscanf(args[0].c_str(), "%x", &startId);
	sscanf(args[1].c_str(), "%x", &endId);

	CTopology::TTopologyId	startTopoId(startId);
	CTopology::TTopologyId	endTopoId(endId);
	CAStarPath				path;

	StaticWorldMap.findAStarPath(startTopoId, endTopoId, path);

	return true;
}

NLMISC_COMMAND(findPath, "Find path between 2 world positions", "startx starty startslot endx endy endslot")
{
	if (args.size() < 6)
		return false;

	sint32	startx, starty, startslot;
	sint32	endx, endy, endslot;

	NLMISC::fromString(args[0], startx);
	NLMISC::fromString(args[1], starty);
	NLMISC::fromString(args[2], startslot);
	NLMISC::fromString(args[3], endx);
	NLMISC::fromString(args[4], endy);
	NLMISC::fromString(args[5], endslot);

	CWorldPosition	start = StaticWorldMap.getWorldPosition(CMapPosition(startx, starty), CSlot(startslot));
	CWorldPosition	end = StaticWorldMap.getWorldPosition(CMapPosition(endx, endy), CSlot(endslot));
	CAStarPath		path;

	nlinfo("Start: topo %d, i0:%d i1:%d i2:%d i3:%d", 
		start.getTopologyRef().getCstTopologyNode().Id.getVal(),
		start.getTopologyRef().getCstTopologyNode().MasterTopL,
		start.getTopologyRef().getCstTopologyNode().MasterTopLN,
		start.getTopologyRef().getCstTopologyNode().MasterTopLW,
		start.getTopologyRef().getCstTopologyNode().MasterTopLNW);
	nlinfo("End: topo %d, i0:%d i1:%d i2:%d i3:%d", 
		end.getTopologyRef().getCstTopologyNode().Id.getVal(),
		end.getTopologyRef().getCstTopologyNode().MasterTopL,
		end.getTopologyRef().getCstTopologyNode().MasterTopLN,
		end.getTopologyRef().getCstTopologyNode().MasterTopLW,
		end.getTopologyRef().getCstTopologyNode().MasterTopLNW);

	path.setStartPos(start);
	path.setEndPos(end);
	StaticWorldMap.findAStarPath(start, end, path.topologiesPathForCalc(), Water);

	CWorldPosition	current = start;
	uint			currentStep = 0;

	while (StaticWorldMap.move(current, path, currentStep))
		nlinfo("current: x:%-10d y:%-10d topoid:%08X topoflags:%d", current.x(), current.y(), current.getTopologyRef().getCstTopologyNode().Id.getVal(), current.getTopologyRef().getCstTopologyNode().Flags);

	return true;
}



NLMISC_COMMAND(testLinks, "test links at a position", "x y slot")
{
	if (args.size() < 3)
		return false;

	CWorldPosition	pos, mv, back;
	CDirection		dir(CDirection::E);

	sint posx, posy;
	uint slot;

	NLMISC::fromString(args[0], posx); 
	NLMISC::fromString(args[1], posy);
	NLMISC::fromString(args[2], slot);

	pos = StaticWorldMap.getWorldPosition(CMapPosition(posx, posy), CSlot(slot));

	uint	i;
	for (i=0; i<8; ++i)
	{
		CDirection		opp(dir);
		opp.addStep(CDirection::HALF_TURN);
		if (StaticWorldMap.move(mv=pos, dir))
		{
			if (!StaticWorldMap.move(back=mv, opp))
				nlwarning("Link failure at direction %d of (%04X,%04X,%d)", dir.getVal(), pos.x(), pos.y(), pos.slot());
			else if (back != pos)
				nlwarning("Reverse link failure at direction %d of (%04X,%04X,%d)", dir.getVal(), pos.x(), pos.y(), pos.slot());
		}

		dir.addStep(CDirection::HALF_TURN_LEFT);
	}

	return true;
}


NLMISC_COMMAND(checkMotionLayer, "checks motion layer", "")
{
//	StaticWorldMap.checkMotionLayer();
	StaticWorldMap.countSuperTopo();

	return true;
}




NLMISC_COMMAND(getH, "get H", "")
{
	if (args.size() < 3)
		return false;

	sint32	startx, starty, startslot;

	NLMISC::fromString(args[0], startx);
	NLMISC::fromString(args[1], starty);
	NLMISC::fromString(args[2], startslot);

	CWorldPosition	start = StaticWorldMap.getWorldPosition(CMapPosition(startx, starty), CSlot(startslot));

	sint32	h = start.getRootCell()->getMetricHeight(start);

	nlinfo("Pos at (%d,%d,%d), z=%d", start.x(), start.y(), start.slot(), h);

	return true;
}

NLMISC_COMMAND(testPacsMove, "test a pacs move", "<continent> <x> <y> <dx> <dy>")
{
	if (args.size() != 5)
		return false;

	CPacsCruncher pc;

	string name = args[0];

	double x, y, dx, dy;
	NLMISC::fromString(args[1], x);
	NLMISC::fromString(args[2], y);
	NLMISC::fromString(args[3], dx);
	NLMISC::fromString(args[4], dy);

	pc.init(name);

	UMovePrimitive	*primitive = pc._Container->addNonCollisionablePrimitive();
	primitive->setPrimitiveType( UMovePrimitive::_2DOrientedCylinder );
	primitive->setReactionType( UMovePrimitive::Stop );
	primitive->setTriggerType((UMovePrimitive::TTrigger)(UMovePrimitive::EnterTrigger+
								  UMovePrimitive::ExitTrigger) );
	primitive->setCollisionMask( 0xffffffff );
	primitive->setOcclusionMask( 0x00000000 );
	primitive->setObstacle( true );
	primitive->setAbsorbtion( 0 );
	primitive->setHeight( 6.0f );
	primitive->setRadius( 0.5f );

	CVectorD	startPos(x, y, 0);
	primitive->setGlobalPosition(startPos, 0);
	pc._Container->evalCollision(1.0, 0);

	primitive->move(CVectorD(dx, dy, 0.0), 0);
	pc._Container->evalNCPrimitiveCollision(1.0, primitive, 0);

	UGlobalPosition newPos;
	primitive->getGlobalPosition(newPos, 0);

	return true;
}
//

}

NLMISC_VARIABLE(string, EvaluatedPos, "Last evaluated pacs position");
NLMISC_VARIABLE(uint, Verbose, "Verbosity");
