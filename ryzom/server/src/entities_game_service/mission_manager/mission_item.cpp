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

bool CMissionItem::buildFromScript( const std::vector<std::string> & script, std::vector< std::pair< std::string, STRING_MANAGER::TParamType > > & chatParams, std::string & varName)
{
	_NoDrop = false;
	if( script.size() < 4 || script.size() > 7)
	{
		MISLOG("syntax error usage : '%s:<item_name> : <brick_craft_plan> : <req_skill> : +[ [<property>|<resist>] <value> ; | <action> ;] [ : <phrase_id>] [ : nodrop]",script[0].c_str() );
		return false;
	}
	string val;
	bool ret = true;

	// get the variable name
	varName = CMissionParser::getNoBlankString( script[1] );
	// get the sheet
	_SheetId = CSheetId( CMissionParser::getNoBlankString( script[2] ) + ".sitem" );
	if  ( _SheetId == CSheetId::Unknown )
	{
		MISLOG("Invalid sitem sheet '%s'", (CMissionParser::getNoBlankString( script[1] ) + ".sitem").c_str());
		ret = false;
	}
	NLMISC::fromString(script[3], _Quality);

	for ( uint i = 4; i < script.size(); i++)
	{
		if ( !nlstricmp( "nodrop", script[i] ) )
			_NoDrop = true;
		else
		{
			vector<string> vars;
			NLMISC::splitString( script[i], ";",vars );

			bool propFound = false;
			for ( uint i = 0; i < vars.size(); i++ )
			{	
				vector<string> args;
				CMissionParser::tokenizeString( vars[i], " \t",args );
				if ( args.size() == 2 )
				{
					// 2 params means that it is an item property
					if( !nlstricmp(args[0],"Durability" ) )
						_Params.Durability = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"Weight" ) )
						_Params.Weight = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"SapLoad" ) )
						_Params.SapLoad = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"Dmg" ) )
						_Params.Dmg = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"Speed" ) )
						_Params.Speed = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"Range" ) )
						_Params.Range = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"DodgeModifier" ) )
						_Params.DodgeModifier = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"ParryModifier" ) )
						_Params.ParryModifier = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"AdversaryDodgeModifier" ) )
						_Params.AdversaryDodgeModifier = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"AdversaryParryModifier" ) )
						_Params.AdversaryParryModifier = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"ProtectionFactor" ) )
						_Params.ProtectionFactor = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"MaxSlashingProtection" ) )
						_Params.MaxSlashingProtection = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"MaxBluntProtection" ) )
						_Params.MaxBluntProtection = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"MaxPiercingProtection" ) )
						_Params.MaxPiercingProtection = (float)atof(args[1].c_str());
					
					else if( !nlstricmp(args[0],"HpBuff" ) )
						NLMISC::fromString(args[1], _Params.HpBuff);
					else if( !nlstricmp(args[0],"SapBuff" ) )
						NLMISC::fromString(args[1], _Params.SapBuff);
					else if( !nlstricmp(args[0],"StaBuff" ) )
						NLMISC::fromString(args[1], _Params.StaBuff);
					else if( !nlstricmp(args[0],"FocusBuff" ) )
						NLMISC::fromString(args[1], _Params.FocusBuff);
					else if( !nlstricmp(args[0],"Color" ) )
					{
						uint8 color;
						NLMISC::fromString(args[1], color);
						if( color <= 7 )
						{
							_Params.Color[color] = 1;
						}
					}
					else if( !nlstricmp(args[0],"AcidProtection") )
						_Params.AcidProtectionFactor = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"ColdProtection") )
						_Params.ColdProtectionFactor = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"FireProtection") )
						_Params.FireProtectionFactor = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"RotProtection") )
						_Params.RotProtectionFactor = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"ShockWaveProtection") )
						_Params.ShockWaveProtectionFactor = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"PoisonProtection") )
						_Params.PoisonProtectionFactor = (float)atof(args[1].c_str());
					else if( !nlstricmp(args[0],"ElectricityProtection") )
						_Params.ElectricityProtectionFactor = (float)atof(args[1].c_str());
					else
					{
						RESISTANCE_TYPE::TResistanceType resistance = RESISTANCE_TYPE::fromString( args[1] );
						switch( resistance )
						{
						case RESISTANCE_TYPE::Desert:
							_Params.DesertResistanceFactor = 1.0f;
							break;
						case RESISTANCE_TYPE::Forest:
							_Params.ForestResistanceFactor = 1.0f;
							break;
						case RESISTANCE_TYPE::Lacustre:
							_Params.LacustreResistanceFactor = 1.0f;
							break;
						case RESISTANCE_TYPE::Jungle:
							_Params.JungleResistanceFactor = 1.0f;
							break;
						case RESISTANCE_TYPE::PrimaryRoot:
							_Params.PrimaryRootResistanceFactor = 1.0f;
							break;
						default:
							MISLOG("Invalid param '%s'", args[0].c_str());
							ret = false;
						}
					}
					propFound = true;
				}
				else if ( args.size() > 2 )
				{
					MISLOG("Invalid property defined with more than 2 params");
					ret = false;
				}
				else if ( args.size() == 1 && i != 0 )
				{
					CSheetId enchantId = CSheetId( CMissionParser::getNoBlankString( args[0] ) + ".sphrase" );
					if (enchantId == CSheetId::Unknown)
					{
						MISLOG("Invalid enchantement '%s.sphrase'", args[0].c_str());
						ret = false;
					}
					else
					{
						_SPhraseId = CSheetId (args[0] + ".sphrase");
						if ( _SPhraseId == CSheetId::Unknown )
						{
							MISLOG("Invalid sheet '%s.sphrase'", args[0].c_str());
							ret = false;
						}
					}
				}
				else if ( !propFound )
					_PhraseId = CMissionParser::getNoBlankString( args[0] );
			}
			///\todo : actions
		}
	}
	return true;
}// CMissionItem::buildFromScript


CGameItemPtr CMissionItem::createItemInTempInv(CCharacter * user, uint16 quantity)
{
	TLogContext_Item_Mission	logContext(user->getId());

	nlassert(user);
	CGameItemPtr item = createItem(quantity);
	if ( item == NULL )
	{
		nlwarning("<CMissionItem createItem> could not create item sheet'%s' for '%s'",_SheetId.toString().c_str(), user->getId().toString().c_str());
		return NULL;
	}
	if (user->addItemToInventory(INVENTORIES::temporary, item, false)) // no autostack because we need to return the item
		return item;
	return NULL;
}// CMissionItem::createItem


CGameItemPtr CMissionItem::createItem(uint16 quantity)
{
	CGameItemPtr item = GameItemManager.createInGameItem(_Quality,quantity,_SheetId,CEntityId::Unknown,&_PhraseId);
	if ( item == NULL )
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
	}
}// CMissionItem::setItemParam



































