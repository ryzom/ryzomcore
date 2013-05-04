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
#include "item_consumable_effect.h"
#include "nel/misc/algo.h"
#include "nel/misc/i18n.h"
#include "../sheet_manager.h"

using namespace std;
using namespace NLMISC;


CItemConsumableEffectHelper* CItemConsumableEffectHelper::getInstance()
{
	// Singleton
	static CItemConsumableEffectHelper* instance = NULL;
	if (instance == NULL)
		instance = new CItemConsumableEffectHelper;
	return instance;
}

void CItemConsumableEffectHelper::getItemConsumableEffectText(const CItemSheet *pIS, ucstring &itemText, sint32 itemQuality)
{
	// check if some effects are present on this item
	ucstring effects("");

	uint i;
	for( i=0; i<pIS->Consumable.Properties.size(); ++i )
	{
		CSString eff = pIS->Consumable.Properties[i];

		if (eff.empty())
			continue;

		// Get name id of effect
		CSString name = toUpper(eff.splitTo(':', true));

		// Extract parameters from sheet
		vector<CSString> params;
		CSString param = eff.splitTo(':', true);
		while (param != "")
		{
			params.push_back(param);
			param = eff.splitTo(':', true);
		}

		if( name == "SP_CHG_CHARAC" )
		{
			CHARACTERISTICS::TCharacteristics charac = CHARACTERISTICS::toCharacteristic(params[0]);
			string characUIId = "uiCaracId" + toString((uint8)charac);

			double param1, param2;
			fromString(params[1].c_str(), param1);
			fromString(params[2].c_str(), param2);
			sint32 bonus = (uint32)(param1 * itemQuality + param2);

			uint32 timeInSec;
			fromString(params[3].c_str(), timeInSec);

			ucstring result;

			if (bonus >= 0)
				result = CI18N::get("uiItemConsumableEffectUpCharac");
			else
				result = CI18N::get("uiItemConsumableEffectDownCharac");

			strFindReplace(result, "%charac", CI18N::get(characUIId));
			strFindReplace(result, "%bonus", toString(bonus));
			strFindReplace(result, "%minutes", toString(timeInSec/60));
			strFindReplace(result, "%secondes", toString(timeInSec%60));

			effects += result;
			effects += "\n";
		}

		if ( name == "SP_LIFE_AURA" )
		{
			
			uint16 regenMod;
			fromString(params[0].c_str(), regenMod);
			uint32 duration;
			fromString(params[1].c_str(), duration);
			uint32 radius;
			fromString(params[2].c_str(), radius);
			uint32 targetDisableTime;
			fromString(params[3].c_str(), targetDisableTime);
			uint32 userDisableTime;
			fromString(params[4].c_str(), userDisableTime);

			ucstring result = CI18N::get("uiItemConsumableEffectLifeAura");
			strFindReplace(result, "%modifier", toString(regenMod));
			strFindReplace(result, "%minutes", toString(duration/60));
			strFindReplace(result, "%secondes", toString(duration%60));
			strFindReplace(result, "%radius", toString(radius));
			strFindReplace(result, "%targetDisableTime", toString(targetDisableTime));
			strFindReplace(result, "%userDisableTime", toString(userDisableTime));

			effects += result;
			effects += "\n";
		}

		if ( name == "SP_LIFE_AURA2" )
		{
			
			uint16 regenMod;
			fromString(params[0].c_str(), regenMod);
			uint32 bonus = regenMod * itemQuality;
			uint32 duration;
			fromString(params[1].c_str(), duration);
			uint32 radius;
			fromString(params[2].c_str(), radius);
			uint32 targetDisableTime;
			fromString(params[3].c_str(), targetDisableTime);
			uint32 userDisableTime;
			fromString(params[4].c_str(), userDisableTime);

			ucstring result = CI18N::get("uiItemConsumableEffectLifeAura");
			strFindReplace(result, "%modifier", toString(bonus));
			strFindReplace(result, "%minutes", toString(duration/60));
			strFindReplace(result, "%secondes", toString(duration%60));
			strFindReplace(result, "%radius", toString(radius));
			strFindReplace(result, "%targetDisableTime", "0");
			strFindReplace(result, "%userDisableTime", "0");

			effects += result;
			effects += "\n";
		}

		if ( name == "SP_STAMINA_AURA" )
		{
			
			uint16 regenMod;
			fromString(params[0].c_str(), regenMod);
			uint32 duration;
			fromString(params[1].c_str(), duration);
			uint32 radius;
			fromString(params[2].c_str(), radius);
			uint32 targetDisableTime;
			fromString(params[3].c_str(), targetDisableTime);
			uint32 userDisableTime;
			fromString(params[4].c_str(), userDisableTime);

			ucstring result = CI18N::get("uiItemConsumableEffectStaminaAura");
			strFindReplace(result, "%modifier", toString(regenMod));
			strFindReplace(result, "%minutes", toString(duration/60));
			strFindReplace(result, "%secondes", toString(duration%60));
			strFindReplace(result, "%radius", toString(radius));
			strFindReplace(result, "%targetDisableTime", toString(targetDisableTime));
			strFindReplace(result, "%userDisableTime", toString(userDisableTime));

			effects += result;
			effects += "\n";
		}


		if ( name == "SP_STAMINA_AURA2" )
		{
			
			uint16 regenMod;
			fromString(params[0].c_str(), regenMod);
			uint32 bonus = regenMod * itemQuality;
			uint32 duration;
			fromString(params[1].c_str(), duration);
			uint32 radius;
			fromString(params[2].c_str(), radius);
			uint32 targetDisableTime;
			fromString(params[3].c_str(), targetDisableTime);
			uint32 userDisableTime;
			fromString(params[4].c_str(), userDisableTime);

			ucstring result = CI18N::get("uiItemConsumableEffectStaminaAura");
			strFindReplace(result, "%modifier", toString(bonus));
			strFindReplace(result, "%minutes", toString(duration/60));
			strFindReplace(result, "%secondes", toString(duration%60));
			strFindReplace(result, "%radius", toString(radius));
			strFindReplace(result, "%targetDisableTime", "0");
			strFindReplace(result, "%userDisableTime", "0");

			effects += result;
			effects += "\n";
		}

		if ( name == "SP_SAP_AURA" )
		{
			
			uint16 regenMod;
			fromString(params[0].c_str(), regenMod);
			uint32 duration;
			fromString(params[1].c_str(), duration);
			uint32 radius;
			fromString(params[2].c_str(), radius);
			uint32 targetDisableTime;
			fromString(params[3].c_str(), targetDisableTime);
			uint32 userDisableTime;
			fromString(params[4].c_str(), userDisableTime);

			ucstring result = CI18N::get("uiItemConsumableEffectSapAura");
			strFindReplace(result, "%modifier", toString(regenMod));
			strFindReplace(result, "%minutes", toString(duration/60));
			strFindReplace(result, "%secondes", toString(duration%60));
			strFindReplace(result, "%radius", toString(radius));
			strFindReplace(result, "%targetDisableTime", toString(targetDisableTime));
			strFindReplace(result, "%userDisableTime", toString(userDisableTime));

			effects += result;
			effects += "\n";
		}

		if ( name == "SP_SAP_AURA2" )
		{
			
			uint16 regenMod;
			fromString(params[0].c_str(), regenMod);
			uint32 bonus = regenMod * itemQuality;
			uint32 duration;
			fromString(params[1].c_str(), duration);
			uint32 radius;
			fromString(params[2].c_str(), radius);
			uint32 targetDisableTime;
			fromString(params[3].c_str(), targetDisableTime);
			uint32 userDisableTime;
			fromString(params[4].c_str(), userDisableTime);

			ucstring result = CI18N::get("uiItemConsumableEffectSapAura");
			strFindReplace(result, "%modifier", toString(bonus));
			strFindReplace(result, "%minutes", toString(duration/60));
			strFindReplace(result, "%secondes", toString(duration%60));
			strFindReplace(result, "%radius", toString(radius));
			strFindReplace(result, "%targetDisableTime", "0");
			strFindReplace(result, "%userDisableTime", "0");

			effects += result;
			effects += "\n";
		}

		// skill modifier consumables
		//---------------------------
		ucstring result("");
		uint8 paramIdx = 0;
		if( name == "SP_MOD_DEFENSE" )
		{
			if( toLower(params[0]) == "dodge" )
				result = CI18N::get("uiItemConsumableEffectModDodgeSuccess");
			else
			if( toLower(params[0]) == "parry" )
				result = CI18N::get("uiItemConsumableEffectModParrySuccess");
			else
				result = CI18N::get("uiItemConsumableEffectModDefenseSuccess");
			paramIdx++;
		}
		if( name == "SP_MOD_MELEE_SUCCESS" )
		{
			result = CI18N::get("uiItemConsumableEffectModMeleeSuccess");
		}
		if( name == "SP_MOD_RANGE_SUCCESS" )
		{
			result = CI18N::get("uiItemConsumableEffectModRangeSuccess");
		}
		if( name == "SP_MOD_MAGIC_SUCCESS" )
		{
			result = CI18N::get("uiItemConsumableEffectModMagicSuccess");
		}
		if( name == "SP_MOD_CRAFT_SUCCESS" )
		{
			result = CI18N::get("uiItemConsumableEffectModCraftSuccess");
		}
		if( name == "SP_MOD_FORAGE_SUCCESS" )
		{
			if( toLower(params[0]) == "commonecosystem" )
				result = CI18N::get("uiItemConsumableEffectModForageSuccess");
			if( toLower(params[0]) == "desert" )
				result = CI18N::get("uiItemConsumableEffectModDesertForageSuccess");
			if( toLower(params[0]) == "forest" )
				result = CI18N::get("uiItemConsumableEffectModForestForageSuccess");
			if( toLower(params[0]) == "lacustre" )
				result = CI18N::get("uiItemConsumableEffectModLacustreForageSuccess");
			if( toLower(params[0]) == "jungle" )
				result = CI18N::get("uiItemConsumableEffectModJungleForageSuccess");
			if( toLower(params[0]) == "primaryroot" )
				result = CI18N::get("uiItemConsumableEffectModPrimaryRootForageSuccess");
			paramIdx++;
		}
		if( !result.empty() )
		{
			double param1, param2;
			fromString(params[paramIdx], param1);
			fromString(params[paramIdx+1], param2);
			sint32 modifier = (sint32)(param1 * itemQuality + param2);
			if( modifier < 0 )
				strFindReplace(result, "%modifier", "@{E42F}-"+toString(modifier)+"@{FFFF}");
			else
				strFindReplace(result, "%modifier", "@{0F0F}"+toString(modifier)+"@{FFFF}");

			uint32 timeInSec;
			fromString(params[paramIdx+2], timeInSec);
			strFindReplace(result, "%minutes", toString(timeInSec/60));
			strFindReplace(result, "%secondes", toString(timeInSec%60));

			effects += result;
			effects += "\n";
		}
	}

	// Display consumable effects info.
	strFindReplace(itemText, "%consumable_effects", effects);
}

