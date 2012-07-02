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

#ifndef CHARACTER_STATISTICS_RECORDER_H
#define CHARACTER_STATISTICS_RECORDER_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// game share
#include "game_share/ryzom_entity_id.h"
#include "game_share/character_summary.h"
#include "game_share/gender.h"
#include "game_share/persistent_data.h"

//-----------------------------------------------------------------------------
// advanced class declarations
//-----------------------------------------------------------------------------

class CCharacter;


//-----------------------------------------------------------------------------
// class CCharaterStatisticsRecorderRecord
//-----------------------------------------------------------------------------

class CCharaterStatisticsRecorderRecord
{
public:
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	CCharaterStatisticsRecorderRecord();
	~CCharaterStatisticsRecorderRecord();

	void clear();
	void build(const CCharaterStatisticsRecorderRecord& last, CCharacter* character);
	
private:
	uint32 _TicksInGame;
	uint64 _TimeInGame;
	uint32 _TotalXP;
};


//-----------------------------------------------------------------------------
// class CCharaterStatisticsRecorderContainer
//-----------------------------------------------------------------------------

class CCharaterStatisticsRecorderContainer
{
public:
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	CCharaterStatisticsRecorderContainer();
	~CCharaterStatisticsRecorderContainer();

	void clear();
	void add(CCharacter* character);
	
private:
	uint32 _Counter;
	std::vector<CCharaterStatisticsRecorderRecord> _Sessions1;
	std::vector<CCharaterStatisticsRecorderRecord> _Sessions10;
	std::vector<CCharaterStatisticsRecorderRecord> _Sessions100;
};


//-----------------------------------------------------------------------------
#endif
