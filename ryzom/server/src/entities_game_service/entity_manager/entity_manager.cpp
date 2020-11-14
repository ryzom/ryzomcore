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

#include "entity_manager/entity_manager.h"
#include "entity_manager/entity_base.h"

//Nel georges
#include "nel/georges/u_form.h"

//game share
#include "game_share/send_chat.h"

#include "entity_manager/entity_callbacks.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "creature_manager/creature_manager.h"

// egs

#include "entities_game_service/world_instances.h"

using namespace NLMISC;
using namespace std;
using namespace NLGEORGES;
using namespace NLNET;

extern CPlayerManager PlayerManager;
extern CCreatureManager CreatureManager;

//vector< CEntityBaseManager::SSuccessXpLine > CEntityBaseManager::_SuccessXpTable;
//CEntityBaseManager::TSuccessTable CEntityBaseManager::_SuccessTable;


//---------------------------------------------------
// CAIInfosOnEntityMsgImp callback
//---------------------------------------------------
void CAIInfosOnEntityMsgImp::callback (const std::string &name, NLNET::TServiceId  id)
{
	// get Entity
	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(EntityRowId);
	if (!entity)
	{
		nlwarning("<CAIInfosOnEntityMsgImp> received infos on unknown entity");
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// log infos
	egs_eminfo("Infos on entity %s", entity->getId().toString().c_str());
	for (uint i = 0 ; i < Infos.size() ; ++i)
	{
		egs_eminfo("%s", Infos[i].c_str());
	}

	//////////////////////////////////////////////////////////////////////////
	// send infos to client
	CEntityBase *player = CEntityBaseManager::getEntityBasePtr(AskerRowID);

	if (player)
	{
		for (uint i = 0 ; i < Infos.size() ; ++i)
		{
			chatToPlayer(player->getId(), Infos[i]);
		}
	}

	// egs specific infos
	string strTemp = NLMISC::toString("HitPoints : %d / %d", entity->currentHp(), entity->maxHp());
	egs_eminfo("%s",strTemp.c_str());
	if (player)
		chatToPlayer(player->getId(), strTemp);
	
	strTemp = NLMISC::toString("isDead : %s ", entity->isDead()?"Yes":"No");
	egs_eminfo("%s",strTemp.c_str());
	if (player)
		chatToPlayer(player->getId(), strTemp);	

	strTemp = NLMISC::toString("ActionFlag : %u ", entity->getActionFlag());
	egs_eminfo("%s",strTemp.c_str());
	if (player)
		chatToPlayer(player->getId(), strTemp);
}





void CQueryEgsImp::callback (const std::string &name, NLNET::TServiceId id)
{

	// the first parameter is a 'literal' that is used by the sender to identified the message
	// the second parameter is the name of the function that is query eg "IsItemInBag"
	std::string funName;	
	if (Params.size() < 2)
	{
		nlwarning("Error in CQueryEgsImpl sub function not defined");
		return;
	}
	funName = Params[1];	
	CQueryEgs::TFunEnum funId = getFunEnum( funName);
	if ( funId == CQueryEgs::Undef)
	{
		nlwarning("Error in CQueryEgsImpl sub function not unknown '%s'", funName.c_str());
		return;
	}
	
	CCharacter* character = 0;

	// the third parameter is the entity id of the player from which we query information
	if (Params.size() < 3)
	{
		nlwarning("Error in CQueryEgsImpl playerId not defined for function '%s'", funName.c_str() );
		return;
	}	
	std::string playerIdStr = Params[2];
	NLMISC::CEntityId playerId;
	playerId.fromString(playerIdStr.c_str());
	character = PlayerManager.getChar( playerId );
	if (!character)
	{
		return;
	}

	NLMISC::CSheetId sheet;
	if (funId == CQueryEgs::IsInInventory || funId == CQueryEgs::KnowBrick )
	{
		if (Params.size() < 4)
		{
			nlwarning("Error in CQueryEgsImpl item not defined for function '%s'", funName.c_str() );
			return;
		}
		sheet = NLMISC::CSheetId ( Params[3]);
		if ( sheet == NLMISC::CSheetId::Unknown)
		{
			nlwarning("Error in CQueryEgsImpl item '%s' not defined  in sheetid.bin for function '%s'", Params[3].c_str(), funName.c_str() );
			return;
		}

	}
	NLMISC::CEntityId entityId;
	if (funId == CQueryEgs::Target)
	{
		if (Params.size() < 4)
		{
			nlwarning("Error in CQueryEgsImpl item not defined for function '%s'", funName.c_str() );
			return;
		}
		entityId.fromString(Params[3].c_str());		
		if ( entityId == NLMISC::CEntityId::Unknown)
		{
			nlwarning("Error in CQueryEgsImpl bot '%s' seem to no be define '%s'", Params[3].c_str(), funName.c_str() );
			return;
		}

	}

	// use the Id of the function query the wanted information
	std::string ret;
	bool error = false;
	switch (funId)
	{
		
	

		case CQueryEgs::Name: ret =  character->getName().toUtf8(); break;								
		case CQueryEgs::Hp: ret = NLMISC::toString("%d", character->getCurrentScore(SCORES::hit_points)); break;
		case CQueryEgs::Sap: ret =	NLMISC::toString("%d", character->getCurrentScore(SCORES::sap)); break;		
		case CQueryEgs::Stamina: ret =	NLMISC::toString("%d", character->getCurrentScore(SCORES::stamina)); break;		
		case CQueryEgs::Focus: ret = NLMISC::toString("%d", character->getCurrentScore(SCORES::focus)); break;
		// Max Score
		case CQueryEgs::MaxHp: ret = NLMISC::toString("%d", character->getMaxScore(SCORES::hit_points)); break;
		case CQueryEgs::MaxSap: ret = NLMISC::toString("%d", character->getMaxScore(SCORES::sap)); break;
		case CQueryEgs::MaxStamina: ret = NLMISC::toString("%d", character->getMaxScore(SCORES::stamina)); break;
		case CQueryEgs::MaxFocus: ret = NLMISC::toString("%d", character->getMaxScore(SCORES::focus)); break;
		// Score  / MaxScore
		case CQueryEgs::RatioHp: { sint32 val = character->getMaxScore(SCORES::hit_points); ret = NLMISC::toString("%d", val? 100 * character->getCurrentScore(SCORES::hit_points) / val:0);} break;
		case CQueryEgs::RatioSap: { sint32 val = character->getMaxScore(SCORES::sap); ret = NLMISC::toString("%d", val? 100 * character->getCurrentScore(SCORES::sap) / val:0);} break;
		case CQueryEgs::RatioStamina: { sint32 val = character->getMaxScore(SCORES::stamina); ret = NLMISC::toString("%d", val? 100 * character->getCurrentScore(SCORES::stamina) / val:0);} break;
		case CQueryEgs::RatioFocus: { sint32 val = character->getMaxScore(SCORES::focus); ret = NLMISC::toString("%d", val? 100 * character->getCurrentScore(SCORES::focus) / val:0);} break;
		
		// Skills
		case CQueryEgs::BestSkillLevel: 
		{
			SKILLS::ESkills skill = character->getBestSkill();
			sint32 value = character->getBestSkillValue(skill);
			ret = NLMISC::toString("%d", value);
			break;
		}

		case CQueryEgs::Target:
		{
			ret = NLMISC::toString("%d", character->getTarget() == entityId);
			break;
		}
		
		//KnowBrick (use sheet parameter)
		case CQueryEgs::KnowBrick:
		{	// verify if a brick identified by its sheet id is known by the player	
			const std::set<NLMISC::CSheetId> & knownBricks = character->getKnownBricks();
			if (knownBricks.find(sheet) != knownBricks.end())
			{
				ret = "1";
			}
			else 
			{
				ret = "0";
			}		
			break;

		}

		//IsInInventory (use sheet parameter)
		case CQueryEgs::IsInInventory:
		{

			bool equipement = false;
			bool bags = false;
			// verify if an item identified by its sheet id is in the inventory of the player
			// return 0 if not in inventory
			// return 1 if in equipment / handling
			// return 2 if in bags / mektoub
			// return 3 if in equipement/handling *AND*  bags / mektoub
		
			const uint sizeInv = INVENTORIES::NUM_INVENTORY;
			for ( uint i = 0; i < sizeInv ; ++i )
			{
				CInventoryPtr childSrc = character->getInventory(INVENTORIES::TInventory(i));
				if ( childSrc != NULL)
				{				
					for ( uint j = 0; j < childSrc->getSlotCount(); j++ )
					{
						CGameItemPtr item = childSrc->getItem(j);
						if (item != NULL && item->getSheetId() == sheet)
						{
							if (i == INVENTORIES::handling  || i == INVENTORIES::equipment)
							{
								equipement = true;
								// go out of loop
								i = sizeInv;
								j = childSrc->getSlotCount();
								break;
							}
							else if (i == INVENTORIES::bag || ( INVENTORIES::pet_animal1 <= i && i < INVENTORIES::max_pet_animal)  )
							{
								bags = true;
							}
						}
					}
				}
			}
			if (equipement){ ret = "1"; }
			else if (bags){ ret = "2"; }
			else { ret = "0";}						
			break;
		}

		default: 
		{
			nlwarning("Error in CQueryEgsImpl function '%s' is not implemented yet", funName.c_str() );
			return;
		}	
	}

	// Launch an userevent back to ais (kind of callback)
	CUserEventMsg msg;
	msg.InstanceNumber = InstanceId;
	msg.GrpAlias = GroupAlias;
	msg.EventId = EventId;
	msg.Params.push_back( Params[0]); // first parameter is a msg specific identifier	
	msg.Params.push_back( ret );// second parameter is  the return of the function
	msg.Params.insert(msg.Params.begin() + 2, Params.begin() +1, Params.end() ); // then other parameters
	CWorldInstances::instance().msgToAIInstance(msg.InstanceNumber, msg);
	
		

}
//---------------------------------------------------
// CEntityBaseManager : constructor
//
//---------------------------------------------------
CEntityBaseManager::CEntityBaseManager()
{
}

//---------------------------------------------------
// Add callback for entity management :
//
//---------------------------------------------------
void CEntityBaseManager::addEntityCallback()
{
	//array of callback items
	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
		{ "SET_VALUE",				cbSetValue				},
		{ "MODIFY_VALUE",			cbModifyValue			},
		{ "TARGET",					cbTarget				},
		{ "FIGHTING_TARGET",		cbFightingTarget		},
		
		{ "ENTITY_POS",				cbEntityPos				},

//		{ "EVENT_REPORTS",			cbActionReport			},

		{ "SET_MODE",				cbChangeMode			},
		{ "SET_BEHAVIOUR",			cbChangeBehaviour		},
		
		//{ "DISENGAGE",				cbDisengage				},
		{ "ENGAGE",					cbEngage				},
		{ "DEFAULT_ATTACK",			cbDefaultAttack			},

		{ "STUN",					cbStun					},
		{ "WAKE",					cbWake					},
//		{ "GOD_MODE",				cbGodMode				},

		{ "SRC_SPWN_VLD_R",			cbRcvValidateSourceSpawnReply },
		
		{ "PLAYER_UNREACHABLE",			cbPlayerUnreachable }
	};
	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );
}


//---------------------------------------------------
// GPMS connexion
//---------------------------------------------------
void CEntityBaseManager::gpmsConnexion()
{
}

//---------------------------------------------------
// loadSuccessXpTable : load table of success probability and xp gains
//
//---------------------------------------------------
/*void CEntityBaseManager::loadSuccessXpTable( const string& tableName )
{
	UFormLoader *loader = UFormLoader::createLoader ();

	CSmartPtr<UForm> form = loader->loadForm (tableName.c_str());
	if ( !form)
	{
		nlwarning("<CEntityBaseManager::loadSuccessXpTable> Failed to load the form %s", tableName.c_str() );
		return;
	}

	// Get the root node, always exist
    UFormElm &root = form->getRootNode ();

	const UFormElm *array = NULL;
	if (root.getNodeByName (&array, "Chances") && array)
    {
		 // Get array size
        uint size;
		array->getArraySize (size);
		
		nlassertex( size == NB_DELTA_LVL, ( "Success table must be have %d raw of delta level, but nb raw reading is %d, check file %s", NB_DELTA_LVL, size, tableName.c_str() ) );

		_SuccessXpTable.resize( size );

		string difficultyAppreciation;
        // Get a array value
        for (uint i=0; i<size; ++i)
        {
			const UFormElm *line = NULL;
			if ( array->getArrayNode( &line, i) && line)
			{
				line->getValueByName( _SuccessXpTable[ i ].RelativeLevel, "RelativeLevel" );
				line->getValueByName( _SuccessXpTable[ i ].SuccessProbability, "SuccessChances" );
				line->getValueByName( _SuccessXpTable[ i ].XpGain, "XpGain" );
				line->getValueByName( difficultyAppreciation, "DifficultyAppreciation" );
				_SuccessXpTable[ i ].DifficultyAppreciation = SENTENCE_APPRAISAL::toAppraisal( difficultyAppreciation );
			}
        }
	}

	if( ! root.getValueByName( _SuccessTable.MaxSuccess, "Max Success" ) )
	{
		nlwarning("<CEntityBaseManager::loadSuccessXpTable> Can't found field 'Max Success' in form %s", tableName.c_str() );
	}
	if( ! root.getValueByName( _SuccessTable.FadeSuccess, "Fade Value" ) )
	{
		nlwarning("<CEntityBaseManager::loadSuccessXpTable> Can't found field 'Fade Value' in form %s", tableName.c_str() );
	}
	if( ! root.getValueByName( _SuccessTable.FadeRoll, "Fade Roll" ) )
	{
		nlwarning("<CEntityBaseManager::loadSuccessXpTable> Can't found field 'Fade Roll' in form %s", tableName.c_str() );
	}

	if( ! root.getValueByName( _SuccessTable.CraftFullSuccessRole, "Craft Full Success Roll" ) )
	{
		nlwarning("<CEntityBaseManager::loadSuccessXpTable> Can't found field 'Craft Full Success Roll' in form %s", tableName.c_str() );
	}
	if( ! root.getValueByName( _SuccessTable.CraftMinSuccess, "Craft Min Success" ) )
	{
		nlwarning("<CEntityBaseManager::loadSuccessXpTable> Can't found field 'Craft Min Success' in form %s", tableName.c_str() );
	}
	if( ! root.getValueByName( _SuccessTable.CraftMinSuccessRole, "Craft Min Success Roll" ) )
	{
		nlwarning("<CEntityBaseManager::loadSuccessXpTable> Can't found field 'Craft Min Success Roll' in form %s", tableName.c_str() );
	}
}

*/
//---------------------------------------------------
// getEntityPtr : return CEntityBase ptr on Id corresponding entity
//
//---------------------------------------------------
/*A*/CEntityBase * CEntityBaseManager::getEntityBasePtr( const CEntityId& id )
{
	if( id.getType() == RYZOMID::player )
	{
		return PlayerManager.getChar( id );
	}
	else
	{
		return CreatureManager.getCreature( id );
	}
}

