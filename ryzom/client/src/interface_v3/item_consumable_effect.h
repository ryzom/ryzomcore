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

#ifndef _ITEM_CONSUMABLE_EFFECT_H
#define _ITEM_CONSUMABLE_EFFECT_H

#include "inventory_manager.h"

/**
 * Helper class to manage consumable effects on items
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2005
 */
class CItemConsumableEffectHelper
{
public:
	// Singleton access
	static CItemConsumableEffectHelper* getInstance();

	// Fill itemText with consumable effects from item sheet
	void getItemConsumableEffectText(const CItemSheet *pIS, ucstring &itemText, sint32 itemQuality);

private:
	CItemConsumableEffectHelper() {}
	//CItemConsumableEffectHelper(const CItemConsumableEffectHelper&);
};

#endif // _ITEM_CONSUMABLE_EFFECT_H
