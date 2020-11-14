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



template	<class T>	class CAIVision;


#ifndef RYAI_VISION_H
#define RYAI_VISION_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"

#include "game_share/player_vision_delta.h"
#include "game_share/ryzom_entity_id.h"
#include "ai_entity_matrix.h"
#include "time_interface.h"
#include "ai_instance.h"

#include <map>
#include <vector>
#include <string>


//--------------------------------------------------------------------------
// The class
//--------------------------------------------------------------------------

template	<class T>
class CAIVision
{
public:
	//----------------------------------------------------------------------
	// default ctor
	CAIVision()	//	: _lastUpdate(CTimeInterface::gameCycle())	{}
	{}
	virtual ~CAIVision()		{}

	template <class VectorClass> 
	void updateBotsAndPlayers(CAIInstance *aii, const VectorClass &xy,uint32 playerRadiusInMeters,uint32 botRadiusInMeters)
	{
//		_lastUpdate=CTimeInterface::gameCycle();
		updatePlayers(aii, xy, playerRadiusInMeters);
		updateBots(aii, xy, botRadiusInMeters);
	}

	void clear ()
	{
		_bots.clear();
		_players.clear();
		_botsVisionRadius = 0;
		_playersVisionRadius = 0;
	}

	//----------------------------------------------------------------------
	// player and bot buffer accessors
	const std::vector<NLMISC::CDbgPtr<T> > &bots()		const { return _bots; }
	const std::vector<NLMISC::CDbgPtr<T> > &players()	const { return _players; }

	const CAIVector&	getBotsVisionCenter() const	{ return _botsVisionCenter; }
	uint32				getBotsVisionRadius() const	{ return _botsVisionRadius; }
	const CAIVector&	getPlayersVisionCenter() const	{ return _playersVisionCenter; }
	uint32				getPlayersVisionRadius() const	{ return _playersVisionRadius; }

	//----------------------------------------------------------------------
	// an stl-like iterator	for iterating through entities in vision
	class iterator
	{
	public:
		iterator(): _vect(NULL), _vision(NULL) {}
		iterator(const CAIVision *vision): _vision(vision) 
		{
			if (!vision->players().empty())
			{
				_vect=&vision->players();
				_it=vision->players().begin();
			}
			else if (!vision->bots().empty())
			{
				_vect=&vision->bots();
				_it=vision->bots().begin();
			}
			else
				_vect=NULL;
		}
		T &operator*() const
		{
			#ifdef NL_DEBUG
			nlassert(_vect!=NULL);
			#endif
			const NLMISC::CDbgPtr<T>	&dbgRef=*_it;
			T&	objRef=*dbgRef;
			return	objRef;
		}
		T *operator->() const
		{
			#ifdef NL_DEBUG
			nlassert(_vect!=NULL);
			#endif
			const NLMISC::CDbgPtr<T>	&dbgRef=*_it;
			T*	objPtr=(T*)dbgRef;
			
			return	objPtr;
		}
		iterator operator++()
		{
			#ifdef NL_DEBUG
			nlassert(_vect!=NULL);
			#endif

			// try to just move the iterator forwards in the current vector
			// if not at end of vector then return (success)
			++_it;
			if (_it!=_vect->end())
				return *this;

			// we've reached the end of the current vector so check wehether we were
			// scanning the player vector - if so switch to the bot vector
			if (_vect==&(_vision->players()) && !_vision->bots().empty())
			{
				_vect=&_vision->bots();
				_it=_vision->bots().begin();
				return *this;
			}

			// we've at the end of both player and bot vectors so flag 'end' condition
			_vect=NULL;
			return *this;
		}

		bool operator!=( const iterator &cmp ) const
		{
			// assume that:
			// - if _vect and cmp._vect don't match then no match
			// - if _vect and cmp._vect are both NULL we have a match
			// - otherwise match depends on iterators
			return ( _vect!=cmp._vect || (_vect!=NULL && _it != cmp._it));
		}

	private:
		const CAIVision *_vision;
		const std::vector<NLMISC::CDbgPtr<T> > *_vect; 
		typename std::vector<NLMISC::CDbgPtr<T> >::const_iterator _it;
	};

	//----------------------------------------------------------------------
	// stl-like begin() and end()
	iterator begin() 
	{
		return iterator(this);
	}
	iterator end() 
	{
		return iterator();
	}

//	const	uint32	&getLastUpdate	()	const
//	{
//		return	_lastUpdate;
//	}

private:

	inline CAIVector getPosition(const class RYAI_MAP_CRUNCH::CWorldPosition & xy)
	{
		return xy.toAIVector();
	}

	inline CAIVector getPosition(const class CAIVector   & xy)
	{
		return xy;
	}



	template <class VectorClass> 
		void updateBots(CAIInstance *aii, const VectorClass &xy,uint32 botRadiusInMeters)
	{
		H_AUTO(VisionUpdateBots);

		_botsVisionCenter = xy;
		_botsVisionRadius = botRadiusInMeters;
		
		const CAIEntityMatrixIteratorTblLinear *tbl;
		typename CAIEntityMatrix<T>::CEntityIteratorLinear it;
		
		_bots.clear();
		if (botRadiusInMeters==0)
			return;
		tbl = CAIS::instance().bestLinearMatrixIteratorTbl(botRadiusInMeters);

		CAIVector aiVectorXy = getPosition(xy);

 		for (it = aii->botMatrix().beginEntities(tbl,xy); !it.end(); ++it)
		{
			CAIEntityPhysical const* phys = const_cast<T*>(&*it)->getSpawnObj();
			if (phys && phys->aipos().quickDistTo(aiVectorXy) < botRadiusInMeters)
			{
				_bots.push_back(const_cast<T*>(&*it));
			}
		}
	}
	
	template <class VectorClass> 
		void updatePlayers(CAIInstance *aii, const VectorClass &xy,uint32 playerRadiusInMeters)
	{
		H_AUTO(VisionUpdatePlayers);
		
		_playersVisionCenter = xy;
		_playersVisionRadius = playerRadiusInMeters;
		
		const CAIEntityMatrixIteratorTblLinear *tbl;
		typename CAIEntityMatrix<T>::CEntityIteratorLinear it;
		
		_players.clear();
		if (playerRadiusInMeters==0)
			return;
		tbl = CAIS::instance().bestLinearMatrixIteratorTbl(playerRadiusInMeters);
		CAIVector aiVectorXy = getPosition(xy);

		for (it = aii->playerMatrix().beginEntities(tbl,xy); !it.end(); ++it)
		{
			CAIEntityPhysical const* phys = const_cast<T*>(&*it)->getSpawnObj();

			if (phys && phys->aipos().quickDistTo(aiVectorXy) < playerRadiusInMeters)
			{
				_players.push_back(const_cast<T*>(&*it));
			}
		}
	}
	
	//----------------------------------------------------------------------
	// private data

	CAIVector		_botsVisionCenter;
	uint32			_botsVisionRadius;
	std::vector<NLMISC::CDbgPtr<T> >	_bots;

	CAIVector		_playersVisionCenter;
	uint32			_playersVisionRadius;
	std::vector<NLMISC::CDbgPtr<T> >	_players;

//	uint32			_lastUpdate;
};

#endif
