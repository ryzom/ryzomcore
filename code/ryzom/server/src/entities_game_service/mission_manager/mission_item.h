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



#ifndef RY_MISSION_ITEM_H
#define RY_MISSION_ITEM_H

#include "game_share/string_manager_sender.h"
#include "game_item_manager/game_item.h"

class CCharacter;
/**
 * class used to described a mission item
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CMissionItem
{
public:
	/// build this class from a script. Return true on success, and add the item chat param to the parameter list
	bool buildFromScript( const std::vector<std::string> & script, std::vector< std::pair< std::string, STRING_MANAGER::TParamType > > & chatParams, std::string & varName );
	/// create an ingame item from this class and put it in user temp inventory
	CGameItemPtr createItemInTempInv(CCharacter * user, uint16 quantity);
	/// create an item from the mission item data
	CGameItemPtr createItem(uint16 quantity);
	/// return the quality
	uint16 getQuality() { return _Quality; }
	/// return the quality
	const NLMISC::CSheetId & getSheetId() { return _SheetId; }
private:
	// set a created item parameter
	void setItemParam(CGameItemPtr item);

	
	/// sheet describing the item type
	NLMISC::CSheetId		_SheetId;
	/// parameters of the item
	CCraftParameters		_Params;
	/// quality of the item
	uint16					_Quality;
	/// phrase id of the item ( its name in most cases )
	std::string				_PhraseId;
	/// true if the item cant be dropped
	bool					_NoDrop;
	/// sheet of the phrase
	NLMISC::CSheetId		_SPhraseId;
};


#endif // RY_MISSION_ITEM_H

/* End of mission_item.h */
