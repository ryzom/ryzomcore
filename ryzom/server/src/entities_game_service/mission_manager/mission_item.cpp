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
#include "mission_item.h"
#include "mission_manager/mission_parser.h"
#include "mission_log.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/algo.h"
#include "player_manager/character.h"
#include "game_item_manager/game_item_manager.h"

#include "egs_sheets/egs_sheets.h"
#include "egs_sheets/egs_static_rolemaster_phrase.h"
#include "server_share/log_item_gen.h"

using namespace std;
using namespace NLMISC;

string CMissionItem::hex_decode(const string & str)
{
	string output;
	for (size_t i=0; i<(str.length()-1); i+=2)
	{
		char c1 = str[i], c2 = str[i+1];
		char buffer[3] = { c1, c2, '\0' };
		char c = (char)strtol(buffer, NULL, 16);
		output.push_back(c);
	}
	return output;
}

bool CMissionItem::buildFromScript(const std::vector<std::string> & script) {
	if (script.size() < 3)
	{
		MISLOG("syntax error usage : '<sheetid>:<quality>:<drop=0|1>:(<phraseid>|(<param>=<value>[,*]))");
		return false;
	}

	// get the sheet
	_SheetId = CSheetId(script[0] + ".sitem");
	if (_SheetId == CSheetId::Unknown)
	{
		MISLOG("Invalid sitem sheet '%s'", (script[0]+".sitem").c_str());
		return false;
	}

	NLMISC::fromString(script[1], _Quality);

	_NoDrop = script[2] == "0";

	if (script.size() > 3) {
		vector<string> vars;
		NLMISC::splitString(script[3], ",", vars);

		for (uint i = 0; i < vars.size(); i++)
		{
			vector<string> args;
			CMissionParser::tokenizeString(vars[i], "=", args);
			if (args.size() == 2)
			{
				// 2 params means that it is an item property
				if(!nlstricmp(args[0], "CustomName"))
					_CustomName.fromUtf8(hex_decode(args[1]));
				else if(!nlstricmp(args[0], "CustomText"))
					_CustomText.fromUtf8(hex_decode(args[1]));
				else if(!nlstricmp(args[0], "RequiredFaction"))
					_RequiredFaction = args[1];
				else if(!nlstricmp(args[0], "RequiredPowo"))
					_RequiredPowo = args[1];
				else
				{
					float value;
					NLMISC::fromString(args[1], value);
					if(!nlstricmp(args[0], "Durability"))
						_Params.Durability = value;
					else if(!nlstricmp(args[0], "Weight"))
						_Params.Weight = value;
					else if(!nlstricmp(args[0], "SapLoad"))
						_Params.SapLoad = value;
					else if(!nlstricmp(args[0], "Dmg"))
						_Params.Dmg = value;
					else if(!nlstricmp(args[0], "Speed"))
						_Params.Speed = value;
					else if(!nlstricmp(args[0], "Range"))
						_Params.Range = value;
					else if(!nlstricmp(args[0], "DodgeModifier"))
						_Params.DodgeModifier = value;
					else if(!nlstricmp(args[0], "ParryModifier"))
						_Params.ParryModifier = value;
					else if(!nlstricmp(args[0], "AdversaryDodgeModifier"))
						_Params.AdversaryDodgeModifier = value;
					else if(!nlstricmp(args[0], "AdversaryParryModifier"))
						_Params.AdversaryParryModifier = value;
					else if(!nlstricmp(args[0], "ProtectionFactor"))
						_Params.ProtectionFactor = value;
					else if(!nlstricmp(args[0], "MaxSlashingProtection"))
						_Params.MaxSlashingProtection = value;
					else if(!nlstricmp(args[0], "MaxBluntProtection"))
						_Params.MaxBluntProtection = value;
					else if(!nlstricmp(args[0], "MaxPiercingProtection"))
						_Params.MaxPiercingProtection = value;
					else if(!nlstricmp(args[0], "HpBuff"))
						_Params.HpBuff = value;
					else if(!nlstricmp(args[0], "SapBuff"))
						_Params.SapBuff = value;
					else if(!nlstricmp(args[0], "StaBuff"))
						_Params.StaBuff = value;
					else if(!nlstricmp(args[0], "FocusBuff"))
						_Params.FocusBuff = value;
					else if(!nlstricmp(args[0], "Color"))
					{
						uint8 color;
						NLMISC::fromString(args[1], color);
						if( color <= 7 )
						{
							_Params.Color[color] = 1;
						}
					}
					else if(!nlstricmp(args[0], "AcidProtection"))
						_Params.AcidProtectionFactor = value;
					else if(!nlstricmp(args[0], "ColdProtection"))
						_Params.ColdProtectionFactor = value;
					else if(!nlstricmp(args[0], "FireProtection"))
						_Params.FireProtectionFactor = value;
					else if(!nlstricmp(args[0], "RotProtection"))
						_Params.RotProtectionFactor = value;
					else if(!nlstricmp(args[0], "ShockWaveProtection"))
						_Params.ShockWaveProtectionFactor = value;
					else if(!nlstricmp(args[0], "PoisonProtection"))
						_Params.PoisonProtectionFactor = value;
					else if(!nlstricmp(args[0], "ElectricityProtection"))
						_Params.ElectricityProtectionFactor = value;
					else if (!nlstricmp(args[0], "Phrase"))
					{
						_SPhraseId = CSheetId (args[0] + ".sphrase");
						if ( _SPhraseId == CSheetId::Unknown )
						{
							MISLOG("Invalid sheet '%s.sphrase'", args[0].c_str());
							return false;
						}
					}
					else
					{
						RESISTANCE_TYPE::TResistanceType resistance = RESISTANCE_TYPE::fromString(args[0]);
						switch( resistance )
						{
						case RESISTANCE_TYPE::Desert:
							_Params.DesertResistanceFactor = value;
							break;
						case RESISTANCE_TYPE::Forest:
							_Params.ForestResistanceFactor = value;
							break;
						case RESISTANCE_TYPE::Lacustre:
							_Params.LacustreResistanceFactor = value;
							break;
						case RESISTANCE_TYPE::Jungle:
							_Params.JungleResistanceFactor = value;
							break;
						case RESISTANCE_TYPE::PrimaryRoot:
							_Params.PrimaryRootResistanceFactor = value;
							break;
						default:
							MISLOG("Invalid param '%s'", args[0].c_str());
							return false;
						}
					}
				}
			}
			else if (args.size() == 1)
			{
				_PhraseId = args[0];
			}
			else
			{
				MISLOG("Invalid property defined with 0 or more than 2 params");
				return false;
			}
		}
	}

	return true;
}// CMissionItem::buildFromScript


CGameItemPtr CMissionItem::createItemInTempInv(CCharacter * user, uint16 quantity)
{
	TLogContext_Item_Mission logContext(user->getId());

	nlassert(user);
	CGameItemPtr item = createItem(quantity);
	if (item == NULL)
	{
		nlwarning("<CMissionItem createItem> could not create item sheet'%s' for '%s'",_SheetId.toString().c_str(), user->getId().toString().c_str());
		return NULL;
	}

	bool res = user->addItemToInventory(INVENTORIES::temporary, item, false); // no autostack because we need to return the item
	if (res)
		return item;

	item.deleteItem();
	return NULL;
}// CMissionItem::createItem


CGameItemPtr CMissionItem::createItem(uint16 quantity)
{
	CGameItemPtr item = GameItemManager.createInGameItem(_Quality, quantity, _SheetId, CEntityId::Unknown, &_PhraseId);
	if (item == NULL)
	{
		nlwarning("<CMissionItem createItem> could not create mission item");
		return NULL;
	}

	setItemParam(item);
	return item;
}// CMissionItem::createItem

void CMissionItem::setItemParam(CGameItemPtr item)
{
	const CStaticRolemasterPhrase * phrase = NULL;
	if ( _SPhraseId != CSheetId::Unknown )
	{
		phrase = CSheets::getSRolemasterPhrase(_SPhraseId);
		if (phrase == NULL)
		{
			nlwarning("<CMissionItem setItemParam> Invalid sheet %s",_SPhraseId.toString().c_str());
			return;
		}
	}
//	if ( item->getSheetId() == CSheetId("stack.sitem") )
//	{
//		const uint size = item->getChildren().size();
//		for (uint i = 0; i < size; i++ )
//		{
//			if ( item->getChildren()[i] != NULL )
//			{
//				if( phrase != 0 )
//					item->applyEnchantment(phrase->Bricks);
//				item->getChildren()[i]->setCraftParameters(_Params);
//				if ( _NoDrop )
//				{
//					item->getChildren()[i]->destroyable(false);
//					item->getChildren()[i]->dropable(false);
//				}
//			}
//		}
//	}
//	else
	{
		if( phrase != 0 )
			item->applyEnchantment(phrase->Bricks);
		item->setCraftParameters(_Params);
		if (_NoDrop)
		{
			item->destroyable(false);
			item->dropable(false);
		}

		if (!_CustomText.empty())
			item->setCustomText(_CustomText);

		if (!_CustomName.empty())
			item->setCustomName(_CustomName);

		if (!_RequiredFaction.empty())
			item->setRequiredFaction(_RequiredFaction);

		if (!_RequiredPowo.empty())
			item->setRequiredPowo(_RequiredPowo);
	}
}// CMissionItem::setItemParam



































