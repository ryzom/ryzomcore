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



#ifndef RYZOM_STABLE_H
#define RYZOM_STABLE_H

#include "game_share/continent.h"
#include "server_share/respawn_point_type.h"
#include "server_share/entity_state.h"

/**
 * CStable
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2003
 * \Manage Stables for pet animals
 */
class CStable
{
	NL_INSTANCE_COUNTER_DECL(CStable);
public:

	struct TStableData
	{
		std::string StableName;
		CONTINENT::TContinent Continent;
		sint32 StableExitX;
		sint32 StableExitY;
		sint32 StableExitZ;
		float Theta;
	};

	typedef std::map< uint16, TStableData > TStableContainer;
	
	// initialize 
	static CStable* getInstance() 
	{
		if(_Instance == NULL)
		{
			_Instance = new CStable();
		}
		return _Instance;
	}
	
	/// constructor
	CStable(); 

	/// destructor
	virtual ~CStable();

	/// add a stable
	void addStable( const std::string& stableName, uint16 placeId, const std::string& continent, float x, float y, float z, float theta );

	/// return stable entry point coordinate
	bool getStableData( uint16 placeId, TStableData& stableData ) const;

	/// return true if name corresponding to a stable
	inline bool isStable( uint16 placeId ) const { return _Stables.find( placeId ) != _Stables.end(); }

private:
	// singleton instance
	static CStable * _Instance;
	
	// re-spawn name and datas
	TStableContainer _Stables;
};
#endif // RYZOM_STABLE_H

/* End of file stable.h */





















