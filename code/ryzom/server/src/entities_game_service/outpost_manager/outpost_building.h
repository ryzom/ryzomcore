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

#ifndef RY_OUTPOST_BUILDING_H
#define RY_OUTPOST_BUILDING_H

class CStaticOutpostBuilding;
class COutpost;

#include "creature_manager/creature.h"

/**
 * This class groups all types of buildings (it is like a building slot)
 * \author Matthieu Besson
 * \author Nevrax France
 * \date september 2005
 */
class COutpostBuilding
{
public:

	DECLARE_PERSISTENCE_METHODS

	COutpostBuilding(COutpost *parent, TAIAlias alias, const NLMISC::CVector &center, NLMISC::CSheetId defaultSheet);

	bool construct(NLMISC::CSheetId sheetid);

	bool destroy();

	void update(uint32 currentTime);

	/// get the alias (unique for each building)
	TAIAlias getAlias() const { return _Alias; }

	/// get the default sheet of the building
	NLMISC::CSheetId getDefaultSheet() const { return _DefaultSheet; }

	/// get the current sheet of the building
	NLMISC::CSheetId getCurrentSheet() const { return _CurrentSheet; }

	/// get the static data (hold by the sheet)
	const CStaticOutpostBuilding *getStaticData() const { return _StaticData; }

	/// Called when ais has decided to spawn the corresponding bot
	void onSpawned(CCreature *pBuilding);

	// Called when the ais is ready and running
	void onAISUp();

	const COutpost *getParent() const { return _Parent; }

	bool isConstructing() const { return _Constructing; }

	/// get a one line description string (for debug)
	std::string toString() const;

	/// called only by the command : outpostAccelerateConstruction
	void setConstructionTime(uint32 nNbSeconds, uint32 nCurrentTime);

private:

	// Never construct a building like this
	COutpostBuilding() { }

	void changeShape(const NLMISC::CSheetId &botId, const std::string &name);

	void postLoad();

private:
	/// @name Primitive data (static)
	//@{
	COutpost				*_Parent;
	
	TAIAlias				_Alias;
	
	// Position in the world
	NLMISC::CVector			_Center;

	// Default sheet of the building
	NLMISC::CSheetId		_DefaultSheet;
	//@}

	/// @name Sheet data (static)
	//@{
	// Static data of the building
	const CStaticOutpostBuilding *_StaticData;
	//@}
	
	/// @name Persistent data
	//@{
	// Date of the last constructed or the beginning of construction
	uint32					_BuiltDate;
	
	// Is there a construction in progress ?
	bool					_Constructing;
	
	// Currently spawned object
	NLMISC::CSheetId		_CurrentSheet;
	
	/// Used for driller
	//@{
	float	_MPLeft;
	uint32	_MPLastTime;
	//@}
	//@}
	
	/// @name Dynamic data
	//@{
	/// Used BotObject
	NLMISC::CRefPtr<CCreature>	_BotObject;
	//@}
};

#endif
