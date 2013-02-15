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
#include "item_special_effect.h"
#include "nel/misc/algo.h"
#include "nel/misc/i18n.h"
#include "../sheet_manager.h"

using namespace std;
using namespace NLMISC;

CItemSpecialEffectHelper::CItemSpecialEffectHelper()
{
	// Register special effects
	registerItemSpecialEffect("ISE_FIGHT_ADD_CRITICAL");
	registerItemSpecialEffect("ISE_FIGHT_VAMPIRISM");
	registerItemSpecialEffect("ISE_MAGIC_DIVINE_INTERVENTION");
	registerItemSpecialEffect("ISE_MAGIC_SHOOT_AGAIN");
	registerItemSpecialEffect("ISE_CRAFT_ADD_STAT_BONUS");
	registerItemSpecialEffect("ISE_CRAFT_ADD_LIMIT");
	registerItemSpecialEffect("ISE_FORAGE_ADD_RM");
	registerItemSpecialEffect("ISE_FORAGE_NO_RISK");
}

CItemSpecialEffectHelper* CItemSpecialEffectHelper::getInstance()
{
	// Singleton
	static CItemSpecialEffectHelper* instance = NULL;
	if (instance == NULL)
		instance = new CItemSpecialEffectHelper;
	return instance;
}

void CItemSpecialEffectHelper::registerItemSpecialEffect(const string &name)
{
	// store parameters to be replaced in UIstring
	vector<string> params;

	// get ui string
	ucstring ucs = CI18N::get("uiItemFX_" + name);
	CSString p, s = ucs.toString();

	// locate and store parameters
	// %p : percent
	// %n : integer
	// %r : real
	// %s : string
	p = s.splitTo('%', true);
	while (p.size() > 0 && s.size() > 0)
	{
		if (s[0] == 'p' || s[0] == 'n' || s[0] == 'r' || s[0] == 's')
		{
			string tmp = "%";
			tmp += s[0];
			if (s.size() >=2 && isdigit(s[1]))
				tmp += s[1];
			params.push_back(tmp);
		}
		p = s.splitTo('%', true);
	}

	effectMap.insert(make_pair(name, params));
}

void CItemSpecialEffectHelper::getItemSpecialEffectText(const CItemSheet *pIS, ucstring &itemText)
{
	// check if some effects are present on this item
	bool firstEffect = false;
	ucstring effects;
	effects += getEffect(pIS->getEffect1(), firstEffect);
	effects += getEffect(pIS->getEffect2(), firstEffect);
	effects += getEffect(pIS->getEffect3(), firstEffect);
	effects += getEffect(pIS->getEffect4(), firstEffect);

	if(!effects.empty()) effects += "\n";

	// Display special effects info.
	strFindReplace(itemText, "%special_effects", effects);
}

ucstring CItemSpecialEffectHelper::getEffect(const std::string &effect, bool &first)
{
	ucstring result;
	CSString eff = effect;

	if (eff.empty())
		return result;

	// Get name id of effect
	CSString name = toUpper(eff.splitTo(':', true));

	// Extract parameters from sheet
	vector<CSString> params;
	CSString param = eff.splitTo(':', true);
	while (!param.empty())
	{
		params.push_back(param);
		param = eff.splitTo(':', true);
	}

	// Check number of arguments
	uint n = (uint)(effectMap[name]).size();
	if (params.size() != n)
	{
		nlinfo("Bad arguments for : %s", effect.c_str());
		return result;
	}

	// Get translated ui string
	result = CI18N::get("uiItemFX_" + name);

	// Add endline if it's not the first
	if (!first)
		first = true;
	else
		result = "\n" + result;

	// Replace each parameters with value from sheet
	//nlinfo("name = %s", name.c_str());
	for (uint i=0 ; i<n ; i++)
	{
		string replace, p = effectMap[name][i];
		//nlinfo("param = %s : %s", p.c_str(), params[i].c_str());

		// %p : percent format
		if (p[1] == 'p')
		{
			float f;
			fromString(params[i], f);
			replace = toString("%.1f", f*100);
		}
		// %n : integer format
		else if (p[1] == 'n')
		{
			int a;
			fromString(params[i], a);
			replace = toString("%d", a);
		}
		// %r : real format
		else if (p[1] == 'r')
		{
			float f;
			fromString(params[i], f);
			replace = toString("%.1f", f);
		}
		// %s : string format
		else if (p[1] == 's')
		{
			replace = params[i];
		}
		else
		{
			continue;
		}

		strFindReplace(result, p.c_str(), replace);
	}

	return result;
}
