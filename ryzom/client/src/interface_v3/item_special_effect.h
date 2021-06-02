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

#ifndef NL_ITEM_SPECIAL_EFFECT_H
#define NL_ITEM_SPECIAL_EFFECT_H

#include "inventory_manager.h"

/**
 * Helper class to manage special effects on items
 * \author Fabien Houlmann
 * \author Nevrax France
 * \date 2005
 */
class CItemSpecialEffectHelper
{
public:
	// Singleton access
	static CItemSpecialEffectHelper* getInstance();

	// Fill itemText with special effects from item sheet
	void getItemSpecialEffectText(const CItemSheet *pIS, ucstring &itemText);

	// Register a new item special effect
	void registerItemSpecialEffect(const std::string &name);

private:
	CItemSpecialEffectHelper();
	CItemSpecialEffectHelper(const CItemSpecialEffectHelper&);

	// Get UI text with values filled from 'effect' string
	ucstring getEffect(const std::string &effect, bool &first);

	// Map effects name with parameters
	typedef std::vector<std::string> stringVector;
	std::map<NLMISC::CSString, stringVector> effectMap;
};

#endif // NL_ITEM_SPECIAL_EFFECT_H
