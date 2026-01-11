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



#ifndef NL_WORLD_CONTAINER_H
#define NL_WORLD_CONTAINER_H

#include "ai_share/world_map.h"
/*
#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"

#include <string>
*/
/**
 * The whole World Container, manager for collision and pathfinding for AI
 * This class is not a singleton.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CWorldContainer
{
protected:

	/// List of merged continents
	static std::vector<std::string>		_ContinentNames;

	/// Three world maps, one for each type of creature (1, 3 and 5 meters width)
	static RYAI_MAP_CRUNCH::CWorldMap	_WorldMaps;

public:
	static	inline	const	RYAI_MAP_CRUNCH::CWorldMap&	getWorldMap()
	{
		return	_WorldMaps;
	}

	static	inline	RYAI_MAP_CRUNCH::CWorldMap&	getWorldMapNoCst()
	{
		return	_WorldMaps;
	}
	
	/// Constructor
	CWorldContainer();

	/// Clear up
	static void	clear();

	/// \name Continent management
	// @{

	/// Merge a continent, like "fyros" or "matis"
	static void	loadContinent(const std::string &name);
	/// Get the list of merged continents
	static std::vector<std::string> &getContinentList() {return _ContinentNames; }

	// @}

//	static	uint32	getMapSize(double physicalRadius)
//	{
//		return	0;
////		if (physicalRadius<=0.5)
////			return	0;
////		if (physicalRadius>1.5)
////			return	2;
////		return	1;
//	}

	class CPosValidator
	{
	public:
		CPosValidator()				{}
		virtual	~CPosValidator()	{}
		virtual	bool	check(const RYAI_MAP_CRUNCH::CWorldPosition& wpos) const=0;
	protected:		
	private:
	};
	class CPosValidatorDefault
		:public	CPosValidator
	{
	public:
		CPosValidatorDefault()		{}
		virtual	~CPosValidatorDefault()		{}
		virtual	bool	check(const RYAI_MAP_CRUNCH::CWorldPosition& wpos) const
		{	return	true;	}
	protected:		
	private:
	};

	template	<class T>
	static	bool	calcNearestWPosFromPosAnRadius	(const	AITYPES::TVerticalPos &verticalPos, const	RYAI_MAP_CRUNCH::CWorldMap	&worldMap, RYAI_MAP_CRUNCH::CWorldPosition& wpos, const T &pos, float radius, uint32 tries, const	CPosValidator	&validator)
	{
		H_AUTO(CalcNearestWPos)
		bool	foundValidPlace=false;
		uint32	incTries=0;
		
		while (	!foundValidPlace
			&&	incTries<tries)
		{
			double	rad=(radius*incTries)/tries;
			CAIVector	rndPos(pos.x()+rad*cos((double)incTries),pos.y()+rad*sin((double)incTries));
			foundValidPlace=worldMap.setWorldPosition(verticalPos, wpos,rndPos);
			if	(foundValidPlace)
				foundValidPlace=validator.check(wpos);
			incTries++;
		}
		return	incTries<tries;
	}
	
	template	<class T>
	static	bool	calcNearestWPosFromPosAnRadius	(const	AITYPES::TVerticalPos &verticalPos, RYAI_MAP_CRUNCH::CWorldPosition& wpos, const T &pos, float radius, uint32 tries, const	CPosValidator	&validator)
	{
		return	calcNearestWPosFromPosAnRadius(verticalPos, getWorldMap(), wpos, pos, radius, tries, validator);
	}
	
	template	<class T>
	static	bool	calcNearestWPosFromPosAnRadius	(sint verticalPos, const	RYAI_MAP_CRUNCH::CWorldMap	&worldMap, RYAI_MAP_CRUNCH::CWorldPosition& wpos, const T &pos, float radius, uint32 tries, const	CPosValidator	&validator)
	{
		H_AUTO(CalcNearestWPos)
		bool	foundValidPlace=false;
		uint32	incTries=0;
		
		while (	!foundValidPlace
			&&	incTries<tries)
		{
			double	rad=(radius*incTries)/tries;
			CAIVector	rndPos(pos.x()+rad*cos((double)incTries),pos.y()+rad*sin((double)incTries));
			foundValidPlace=worldMap.setWorldPosition((sint32)verticalPos, wpos,rndPos);
			if	(foundValidPlace)
				foundValidPlace=validator.check(wpos);
			incTries++;
		}
		return	incTries<tries;
	}
	
	template	<class T>
	static	bool	calcNearestWPosFromPosAnRadius	(sint verticalPos, RYAI_MAP_CRUNCH::CWorldPosition& wpos, const T &pos, float radius, uint32 tries, const	CPosValidator	&validator)
	{
		return	calcNearestWPosFromPosAnRadius(verticalPos, getWorldMap(), wpos, pos, radius, tries, validator);
	}
	
	
};


#endif // NL_WORLD_CONTAINER_H

/* End of world_container.h */
