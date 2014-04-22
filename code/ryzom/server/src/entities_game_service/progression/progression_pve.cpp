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


//XP
	 //
	 // -	On garde les points de dégâts infligés sur chaque créature par chaque team ou joueur sans team (considéré comme une team de 1)
	 //
	 // -	la « team » avec le plus de dégâts est la seule a gagner l’xp de la créature
	 //
	 // -	Tout joueur membre d’une team ayant fait une action ‘de combat’ a moins de X mètres de la créature tuée par son groupe gagne sa part
	 // d’Xp, qu’il soit ou non dans l’aggro list de la créature. L’Xp est réparti équitablement entre les joueurs éligibles.
	 // Chaque créature a une valeur d’XP fixe renseignée dans sa fiche. Cette valeur est multipliée par le facteur d’Xp obtenu par l’écart
	 // de niveau entre la créature (XpLevel) et la plus forte skill utilisée par n’importe quel membre du groupe (magie ou combat, soins compris).
	 // Chaque team a un « poids » égal a 1 + 0.8 par membre au délà de 1 (cette valeur de 0.8 est réglable dans le cfg de l’egs)
	 // Une fois le total d’Xp calculé on divise par le « poids » de la team et on donne a chacun sa part.
	 //
	 //  NB : Si un joueur éligible se trouve a plus de Y mètres de la créature ET qu’il n’est plus dans l’aggro list au moment de la mort de
	 //	 celle-ci il ne gagne pas sa part mais il compte dans la division !!
	 //
	 //	 -	Si un joueur rejoint une team il ‘fusionne’ ses dégâts avec celle-ci.
	 //
	 //	 -	Si un joueur quitte une team il ‘emmène’ une partie des dégâts sur chaque créature pour laquelle il aurait pu gagner de l’XP.
	 //	 Cette partie est tout simplement le total des dégâts de la team divisé par le nb de membres avant son départ.
	 //
	 //	 -	Si tous les membres de la team sortent de l’aggro list d’une créature leurs degats sur celle-ci sont transférés sur le compte d'une
	 //	 créature fictive. Par exemple on a 2 joueurs sans team, l’un fait 99% des degats et meurt et respawn, ses 99% sont transférés à une
	 //	 créature fictive. Le 2ème joueur achève la créature, il a fait 1% des degats, la créature fictive a fait le plus de dégats et donc aucun
	 //	 joueur réel ne gagne d’XP.
	 //
	 //	 - ATTENTION : les gardes et autres npcs sont aussi comptabilisés pour leur degats, et n’ont pas de ‘range’ de validité (par contre
	 //	 ils doivent etre ds l’aggro liste ??). Ce qui veut dire que si une créature est gravement blessée par un garde le joueur qui l’achève
	 //	 ne gagne rien.
	 //
	 // - si un joueur soigne un joueur qui n'est PAS un membre de sa team il ne gagnera pas d'Xp pour cette action


#include "stdpch.h"
#include "progression_pve.h"

#include "entity_manager/entity_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"

#include "game_share/action_nature.h"
#include "game_share/skills.h"
#include "game_share/constants.h"
#include "game_share/utils.h"
#include "game_share/fame.h"

#include "egs_sheets/egs_sheets.h"
#include "entity_manager/entity_base.h"
#include "player_manager/character.h"
#include "creature_manager/creature.h"
#include "creature_manager/creature_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "team_manager/team_manager.h"
#include "pvp_manager/pvp.h"
#include "pvp_manager/pvp_faction_reward_manager/pvp_faction_reward_manager.h"
#include "server_share/r2_variables.h"

#include "egs_globals.h"
#include "server_share/log_item_gen.h"
#include "server_share/log_character_gen.h"

using namespace std;
using namespace NLMISC;

// Variable to alter the progression factor.
extern float				SkillProgressionFactor;
extern CCreatureManager		CreatureManager;
extern const CStaticXpFactorTable *XpFactorTable;
extern CTeamManager			TeamManager;


//----------------------------------------------------------------------------
// CAILostAggroMsgImp::callback creature lost agro
//----------------------------------------------------------------------------
void CAILostAggroMsgImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CCreature *creature = CreatureManager.getCreature( TargetRowId );
	if( creature )
	{
		// forget Xp Gain
		PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->lostAggro( TargetRowId, PlayerRowId);

//		creature->getCreatureOpponent().aggroLost( PlayerRowId );
		creature->removeAggressivenessAgainstPlayerCharacter( PlayerRowId );
	}
}


namespace PROGRESSIONPVE
{


NL_INSTANCE_COUNTER_IMPL(CSkillProgress);
NL_INSTANCE_COUNTER_IMPL(CCharacterActions);
NL_INSTANCE_COUNTER_IMPL(CCharacterProgressionPVE);

// static members of CCharacterProgressionPVE
CCharacterProgressionPVE * CCharacterProgressionPVE::_Instance = NULL;

//----------------------------------------------------------------------------
// dtor
//----------------------------------------------------------------------------
CCharacterProgressionPVE::~CCharacterProgressionPVE()
{
	for( TCharacterActionsContainer::iterator it = _CharacterActions.begin(); it != _CharacterActions.end(); ++it )
	{
		delete (*it).second;
	}
	_CharacterActions.clear();
	delete _Instance;
	_Instance = NULL;
}

//----------------------------------------------------------------------------
// actionReport: game system report an action
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::actionReport( TReportAction& reportAction, bool incActionCounter, bool scaleForNewbies )
{
	H_AUTO(CCharacterProgressionPVE_actionReport);

	CEntityBase * actor = CEntityBaseManager::getEntityBasePtr( reportAction.ActorRowId );
	if( actor == 0 )
	{
		nlwarning("<CCharacterProgressionPVE::actionReport> received report action but can't found actor entity corresponding to mirror raw id %d", reportAction.ActorRowId.getIndex() );
		return;
	}
	CEntityBase * target = CEntityBaseManager::getEntityBasePtr( reportAction.TargetRowId );

	// memorize damage
	if (target && reportAction.Hp > 0)
	{
		addDamage(actor->getId(), target->getId(), reportAction.Hp);
	}

	// used skill will be considered for delta level of the action during a combat
	bool useSkillForDeltaLevel = true;

	switch( reportAction.ActionNature )
	{
		case ACTNATURE::DODGE:
		case ACTNATURE::PARRY:
		case ACTNATURE::SHIELD_USE:
			// no xp gain
			return;

		case ACTNATURE::NEUTRAL:
			// do not consider used skill for delta level
			useSkillForDeltaLevel = false;
			// do not break here

		case ACTNATURE::FIGHT:
			// if no damage done on a fight action, do not consider it, juste returns
			if (reportAction.Hp == 0)
			{
				return;
			}
			// do not break here

		case ACTNATURE::OFFENSIVE_MAGIC:
			if( target )
			{
				offensiveActionReported( reportAction, actor, target, incActionCounter );

				if (actor->getId().getType() == RYZOMID::player)
				{
					CCharacter *playerChar = dynamic_cast<CCharacter*> (actor);
					if (!playerChar)
					{
						nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", actor->getId().toString().c_str());
						return;
					}

					list<TDataSetRow> enabledCreatures;

					if (useSkillForDeltaLevel)
					{
						// if no damage reported yet, report it now to add target creature as team ennemy
						if (reportAction.Hp == 0)
							addDamage(actor->getId(), target->getId(), 0);

						uint16 skillValue = (uint16)playerChar->getSkillBaseValue(reportAction.Skill);
						if (reportAction.SkillLevel > skillValue)
							skillValue = reportAction.SkillLevel;

						// enable xp gain only if attacking creatures which give xp
						CCreature *creature = dynamic_cast<CCreature*> (target);
						if (creature && creature->getForm())
						{
							if ( creature->getForm()->getXPGainOnCreature() != 0 )
							{
								enableXpGainForPlayer(actor->getId(), skillValue, enabledCreatures);
							}
						}
					}
					else
					{
						// enable xp gain only if attacking creatures which give xp
						CCreature *creature = dynamic_cast<CCreature*> (target);
						if (creature && creature->getForm())
						{
							if ( creature->getForm()->getXPGainOnCreature() != 0 )
							{
								enableXpGainForPlayer(actor->getId(), 0, enabledCreatures);
							}
						}
					}

					for (list<TDataSetRow>::iterator it = enabledCreatures.begin() ; it != enabledCreatures.end() ; ++it)
					{
						if (*it == reportAction.TargetRowId)
							continue;

						CEntityBase *creature = CreatureManager.getCreature(*it);
						if (creature)
						{
							TReportAction reportActionBis = reportAction;
							reportActionBis.TargetRowId = (*it);
							//reportActionBis.ItemUsed = NULL;
							offensiveActionReported( reportActionBis, actor, creature, incActionCounter );
						}
					}
				}
			}
			break;
		case ACTNATURE::CURATIVE_MAGIC:
			if( target )
			{
				curativeActionReported( reportAction, actor, target, incActionCounter );
				// no longer need following
				/*if ( actor->getId().getType() == RYZOMID::player)
				{
					CCharacter *playerChar = dynamic_cast<CCharacter*> (actor);
					if (!playerChar)
					{
						nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", actor->getId().toString().c_str());
						return;
					}

					list<TDataSetRow> enabledCreatures;
					enableXpGainForPlayer(actor->getId(), (uint16)playerChar->getSkillBaseValue(reportAction.Skill), enabledCreatures);
					for (list<TDataSetRow>::iterator it = enabledCreatures.begin() ; it != enabledCreatures.end() ; ++it)
					{
						CEntityBase *creature = CreatureManager.getCreature(*it);
						if (creature)
						{
							TReportAction reportActionBis = reportAction;
							reportActionBis.TargetRowId = (*it);
							reportActionBis.ItemUsed = NULL;
							curativeActionReported( reportActionBis, actor, creature );
						}
					}
				}
				*/
			}
			break;
		case ACTNATURE::CRAFT:
		case ACTNATURE::HARVEST:
		case ACTNATURE::SEARCH_MP:
			simpleActionReported( reportAction, actor, scaleForNewbies );
			break;
		default:
			nlwarning("<CCharacterProgressionPVE::actionReport> received action report with ActionNature %s, can't process skill progression", ACTNATURE::toString(reportAction.ActionNature).c_str() );
			return;
	}
}


//----------------------------------------------------------------------------
// forgetXpGain: forget xp gain (after creature lost agro on character or despawned)
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::forgetXpGain( TDataSetRow creature, TDataSetRow offensiveCharacter )
{
	TCharacterActionsContainer::iterator it = _CharacterActions.find( offensiveCharacter );
	if( it != _CharacterActions.end() )
	{
		bool another = (*it).second->forgetXpGain( creature );
		if( another == false )
		{
			delete (*it).second;
			_CharacterActions.erase( it );
		}
	}
}


//----------------------------------------------------------------------------
// creatureDeath: creature is dead, report character perform offensive action against
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::creatureDeath(TDataSetRow creature)
{
	// get damage inflicted on creature
	TCreatureTakenDamageContainer::iterator itCreatureDamage = _CreatureTakenDamage.find(creature);
	if (itCreatureDamage == _CreatureTakenDamage.end())
	{
		// disable loot for players by validating it for creature itself
		CCreature *creaturePtr = CreatureManager.getCreature(creature);
		if (creaturePtr)
		{
			creaturePtr->enableLootRights(creaturePtr->getEntityRowId());
		}
		return;
	}

	CCreatureTakenDamage &creatureTakenDmg = (*itCreatureDamage).second;

	// attribute kills for mission system
	creatureTakenDmg.attributeKillsForMission(creature);

	// get most effective team or single player
	const sint16 index = creatureTakenDmg.getMaxInflictedDamageTeamIndex();

	// max damage have been done by players
	if (index != -1)
	{
		CCreature *creaturePtr = CreatureManager.getCreature(creature);
		if (creaturePtr && creaturePtr->getForm())
		{
			// Auto-quartering of mission items
			uint16 itemLevel = creaturePtr->getForm()->getXPLevel();
			vector<CAutoQuarterItemDescription> matchingItemsForMissions; // point to creaturePtr->getForm()->getItemsForMission()

			// get delta level factor between team and creature
			const uint16 skillRefValue = creatureTakenDmg.PlayerInflictedDamage[index].MaxSkillValue;
			sint32 deltaLevel = sint32(skillRefValue - creaturePtr->getForm()->getXPLevel());

			float factor = CStaticSuccessTable::getXPGain(SUCCESS_TABLE_TYPE::FightPhrase, deltaLevel);

			// dispatch Xp for all validated team members
			if (creatureTakenDmg.PlayerInflictedDamage[index].TeamId != CTEAM::InvalidTeamId)
			{
				// validate loot right for team on this creature
				creaturePtr->enableLootRights(creatureTakenDmg.PlayerInflictedDamage[index].TeamId);

				// get team members list
				CTeam *team = TeamManager.getTeam(creatureTakenDmg.PlayerInflictedDamage[index].TeamId);
				if (team)
				{
					const list<CEntityId> &teamMembers = team->getTeamMembers();

					vector<CTeamMember> &members = creatureTakenDmg.PlayerInflictedDamage[index].TeamMembers;

					// compute XP divisor from nb team members
					const float equivalentXpMembers = 1 + (members.size()-1) * XPTeamMemberDivisorValue;

					for (uint i = 0; i != members.size() ; ++i)
					{
						TDataSetRow playerRowId = TheDataset.getDataSetRow(members[i].Id);
						if (members[i].GainXp)
						{
							float tempFactor = factor;

							// check this player is in aggro list or not too far away
							double distance = PHRASE_UTILITIES::getDistance( playerRowId, creature);
							if ( distance > MaxDistanceForXpGain)
							{
								const std::set<TDataSetRow> &aggrolist = creaturePtr->getAggroList();
								if ( aggrolist.find(playerRowId) == aggrolist.end())
									tempFactor = 0.0f;
							}

							TCharacterActionsContainer::iterator it = _CharacterActions.find( playerRowId );
							if( it != _CharacterActions.end() )
							{
								// Dispatch XP gain
								bool moreXpWaiting = (*it).second->dispatchXpGain( playerRowId, creature, equivalentXpMembers, tempFactor, teamMembers );
								if( moreXpWaiting == false )
								{
									delete (*it).second;
									_CharacterActions.erase( it );
								}

								// Get the mission requirements for auto-quartering
								CCharacter *player = PlayerManager.getChar( playerRowId );
								if ( player )
								{
									player->getMatchingMissionLootRequirements( itemLevel, creaturePtr->getForm()->getItemsForMissions(), matchingItemsForMissions );
								}
							}
						}
					}
				}
			}
			// all xp (and mission auto-quartering) go to a single player
			else
			{
				const TDataSetRow rowId = TheDataset.getDataSetRow(creatureTakenDmg.PlayerInflictedDamage[index].PlayerId);
				creaturePtr->enableLootRights(rowId);

				TCharacterActionsContainer::iterator it = _CharacterActions.find(rowId);
				if( it != _CharacterActions.end() )
				{
					// Enable loot rights for this player alone

					// Display XP gain
					list<CEntityId> teamMembers;
					teamMembers.push_back(creatureTakenDmg.PlayerInflictedDamage[index].PlayerId);
					bool moreXpWaiting = (*it).second->dispatchXpGain( (*it).first, creature, 1, factor, teamMembers);
					if( moreXpWaiting == false )
					{
						delete (*it).second;
						_CharacterActions.erase( it );
					}

					// Get the mission requirements for auto-quartering
					CCharacter *player = PlayerManager.getChar( rowId );
					if ( player )
					{
						player->getMatchingMissionLootRequirements( itemLevel, creaturePtr->getForm()->getItemsForMissions(), matchingItemsForMissions );
					}
				}
			}

			// Finish auto-quartering of mission items
			if ( ! matchingItemsForMissions.empty() )
			{
				// Decide how many mission items will be given away (supports any float average; typically 1.0)
				uint quantityOfMissionItemsToGive = 0;
				vector<uint> quantityPerMatchingMissionItem( matchingItemsForMissions.size(), 0 );
				uint iMaxMatchingMissionItem = (uint)quantityPerMatchingMissionItem.size() - 1;
				quantityOfMissionItemsToGive = (uint)QuarteringQuantityAverageForMissions.get();
				float decimalPart = (uint)QuarteringQuantityAverageForMissions.get() - (float)floor( QuarteringQuantityAverageForMissions.get() );
				quantityOfMissionItemsToGive = RandomGenerator.rand( quantityOfMissionItemsToGive * 2 );
				if ( (decimalPart != 0) && (RandomGenerator.frand( 1.0f ) < decimalPart) )
					++quantityOfMissionItemsToGive;
				for ( uint i=0; i!=quantityOfMissionItemsToGive; ++i )
				{
					++quantityPerMatchingMissionItem[RandomGenerator.rand( iMaxMatchingMissionItem )];
				}

				// Give the mission items to the selected players
				for ( uint i=0; i<=iMaxMatchingMissionItem; ++i )
				{
					if ( quantityPerMatchingMissionItem[i] != 0 )
					{
						// Give item, limiting the quantity according to the quantity required by the mission (the remaining is lost)
						uint16 quantity = min( (uint16)quantityPerMatchingMissionItem[i], matchingItemsForMissions[i].QuantityNeeded );
						uint iMissionItem = (uint)matchingItemsForMissions[i].ItemIndex;
						const CSheetId& itemSheetId = creaturePtr->getForm()->getItemsForMissions()[iMissionItem];
						CCharacter *player = matchingItemsForMissions[i].PlayerNeedingTheItem;
						TLogContext_Item_AutoMissionLoot logContext(player->getId());

						if ( player->createItemInInventory( INVENTORIES::bag, itemLevel, quantity, itemSheetId ) )
						{
							// Send message
							SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
							params[0].Int = (sint32)quantity;
							params[1].SheetId = itemSheetId;
							params[2].Int = (sint32)itemLevel;
							PHRASE_UTILITIES::sendDynamicSystemMessage( player->getEntityRowId(), "AUTO_LOOT_SUCCESS", params );

							// Mission event
							CMissionEventLootRm event( itemSheetId, quantity, itemLevel );
							player->processMissionEvent( event );
						}
					}
				}
			}
		}
	}
	// max damage have been done by npcs or creatures
	else
	{
		// If fictitious entity killed the creature send a message to all players involved to tell em they were not enough implied
		if (creatureTakenDmg.isMaxCreatureInflictedDamageTransfered())
		{
			// Get players list
			std::set<NLMISC::CEntityId> players(creatureTakenDmg.getAllPlayers());
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
			CCreature* creaturePtr = CreatureManager.getCreature(creature);
			if (creaturePtr)
			{
				params[0].setEIdAIAlias( creaturePtr->getId(), CAIAliasTranslator::getInstance()->getAIAlias(creaturePtr->getId()) );
				std::set<NLMISC::CEntityId>::iterator itPlayer, itPlayerEnd = players.end();
				for (itPlayer=players.begin(); itPlayer!=itPlayerEnd; ++itPlayer)
				{
					CCharacter* player = PlayerManager.getChar(*itPlayer);
					if (player)
						PHRASE_UTILITIES::sendDynamicSystemMessage( player->getEntityRowId(), "PROGRESS_MAX_DAMAGE_TRANSFERED", params );
				}
			}
		}

		CCreature *creaturePtr = CreatureManager.getCreature(creature);
		if (creaturePtr)
		{
			// disable loot for players by validating it for creature itself
			creaturePtr->enableLootRights(creaturePtr->getEntityRowId());
		}
	}

	removeCreature(creature);
}


//----------------------------------------------------------------------------
// getProgressionFactor: return xp gain depend on skill and delta level as factor
//----------------------------------------------------------------------------
double CCharacterProgressionPVE::getProgressionFactor( CEntityBase * actor, sint32 deltaLvl, SKILLS::ESkills skill, SUCCESS_TABLE_TYPE::TSuccessTableType tableType, bool scaleForNewbies )
{
	if ( scaleForNewbies )
	{
		// get pointer on static skills tree definition
		CSheetId sheet("skills.skill_tree");
		const CStaticSkillsTree * SkillsTree = CSheets::getSkillsTreeForm( sheet );
		nlassert( SkillsTree );

		if( skill == SKILLS::unknown )
		{
			nlwarning("<PROGRESSION>%s invalid skill. table type = '%s'",actor->getId().toString().c_str(),SUCCESS_TABLE_TYPE::toString(tableType).c_str() );
			return 0.0;
		}

		// calculate the xp points gain
		double XpGain = 0;

		// Found compatible unlocked skill
		sint32 skillval = 0;
		sint skillValue = 0;
		if ( actor->getId().getType() == RYZOMID::player )
		{
			CCharacter * pC = (CCharacter *) actor;
			while( pC->getSkills()._Skills[ skill ].Base == 0 && SkillsTree->SkillsTree[ skill ].ParentSkill != SKILLS::unknown )
			{
				skill = SkillsTree->SkillsTree[ skill ].ParentSkill;
			}
			skillValue = pC->getSkills()._Skills[ skill ].Base + 10;
		}
		else
		{
			const CStaticCreatures * form = actor->getForm();
			if ( !form )
			{
				nlwarning( "<MAGIC>invalid creature form %s in entity %s", actor->getType().toString().c_str(), actor->getId().toString().c_str() );
				return 0.0;
			}
			skillValue = form->getAttackLevel() + 10;
		}
		// Delta level with player skill factor multiplier
		sint32 minimum = min( (sint32)50, (sint32)( skillValue ) );
		if( minimum == 0 )
		{
			minimum = 10;
		}
		deltaLvl *= (sint32)50 / minimum;
	}

	return CStaticSuccessTable::getXPGain(tableType, deltaLvl);
}


//----------------------------------------------------------------------------
// getXpGain: return xp gain depend on skill and delta level
//----------------------------------------------------------------------------
double CCharacterProgressionPVE::getXpGain( CEntityBase * actor, sint32 deltaLvl, SKILLS::ESkills skill, float factor, SUCCESS_TABLE_TYPE::TSuccessTableType tableType, bool scaleForNewbies )
{
	return getProgressionFactor( actor, deltaLvl, skill, tableType, scaleForNewbies ) * SkillProgressionFactor * factor;
}


//----------------------------------------------------------------------------
// offensiveActionReported: process report for a fight action (melee and range combat, all spells makes damage, all spells makes disease and curse)
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::offensiveActionReported( const TReportAction& reportAction, CEntityBase * actor, CEntityBase * target, bool incActionCounter )
{
	if( actor->getId().getType() == RYZOMID::player )
	{
//		if( reportAction.Skill != SKILLS::unknown )
		{
			CCreature * crea = dynamic_cast< CCreature * >( target );
			if( crea != 0 )
			{
				if( crea->isSpire() )
				{
					CPVPFactionRewardManager::getInstance().spireAttacked( (CCharacter *)actor, crea );
				}

				// PJ perform offensive action against PNJ
//				crea->getCreatureOpponent().storeAggressor( reportAction.ActorRowId, reportAction.Hp );
				TCharacterActionsContainer::iterator it = _CharacterActions.find( reportAction.ActorRowId );
				if( it == _CharacterActions.end() )
				{
					CCharacterActions * oc = new CCharacterActions();
					if( oc )
					{
						pair< TCharacterActionsContainer::iterator, bool > insertReturn;
						insertReturn = _CharacterActions.insert( make_pair( reportAction.ActorRowId, oc ) );
						if( insertReturn.second == false )
						{
							nlwarning("<CCharacterProgressionPVE::actionReport> insert new pair<%d, CCharacterActions *> failed, can't continue skill progress process", reportAction.ActorRowId.getIndex() );
							return;
						}
						it = insertReturn.first;
					}
					else
					{
						nlwarning("<CCharacterProgressionPVE::actionReport> can't allocate CCharacterActions instance, memory full ?, can't continue skill progress process");
						return;
					}
				}

				(*it).second->addAction( reportAction.TargetRowId, reportAction.Skill, incActionCounter );
			}
		}

		// PVP hurt
		if (actor->getId().getType() == RYZOMID::player && target->getId().getType() == RYZOMID::player)
		{
			CCharacter * actorChar = dynamic_cast<CCharacter *>(actor);
			CCharacter * targetChar = dynamic_cast<CCharacter *>(target);
			if ( actorChar && targetChar && actorChar->getPVPInterface().isValid() )
				actorChar->getPVPInterface().hurt( targetChar );
		}
	}
/*	else if( target->getId().getType() != RYZOMID::player )
	{
		CCreature * crea = dynamic_cast< CCreature * >( target );
		if( crea != 0 )
		{
			crea->getCreatureOpponent().storeAggressor( reportAction.ActorRowId, reportAction.Hp );
		}
	}
*/
}


//----------------------------------------------------------------------------
// curativeMagicActionReported: process report for a curative (heal) action
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::curativeActionReported( const TReportAction& reportAction, CEntityBase * actor, CEntityBase * target, bool incActionCounter )
{
	BOMB_IF(actor == NULL || target == NULL, "", return);

	// do nothing if target or actor aren't both players
	if (actor->getId().getType() != RYZOMID::player || target->getId().getType() != RYZOMID::player)
		return;

	CCharacter * targetedPlayer = dynamic_cast< CCharacter * >( target );
	if( targetedPlayer == NULL )
		return;

	CCharacter * actorPlayer = dynamic_cast< CCharacter * >( actor );
	if( actorPlayer == NULL )
		return;

	// check actor and target are member or the same team
	if ( (actor == target) || ( (targetedPlayer->getTeamId() != CTEAM::InvalidTeamId) && (targetedPlayer->getTeamId() == actorPlayer->getTeamId()) ) )
	{
		const uint16 teamId = targetedPlayer->getTeamId();
		TTeamsAttackedCreature::iterator itTeam = _TeamsWoundedCreatures.find(teamId);
		if (itTeam == _TeamsWoundedCreatures.end())
		{
			// no creature in combat, do not memorize action
			return;
		}

		// get or create character actions structure
		TCharacterActionsContainer::iterator itActions = _CharacterActions.find( reportAction.ActorRowId );
		if( itActions == _CharacterActions.end() )
		{
			CCharacterActions * oc = new CCharacterActions();
			if( oc )
			{
				pair< TCharacterActionsContainer::iterator, bool > insertReturn;
				insertReturn = _CharacterActions.insert( make_pair( reportAction.ActorRowId, oc ) );
				if( insertReturn.second == false )
				{
					nlwarning("<CCharacterProgressionPVE::actionReport> insert new pair<%d, CCharacterActions *> failed, can't continue skill progress process", reportAction.ActorRowId.getIndex() );
					return;
				}
				itActions = insertReturn.first;
			}
			else
			{
				nlwarning("<CCharacterProgressionPVE::actionReport> can't allocate CCharacterActions instance, memory full ?, can't continue skill progress process");
				return;
			}
		}

		// memorize action for all creatures in combat
		vector<TDataSetRow> &creatures = (*itTeam).second;
		for (uint i = 0 ; i < creatures.size() ; ++i)
		{
			// check distance between player and creature, if too far away, do not enable xp gain
			const double distance = PHRASE_UTILITIES::getDistance(actorPlayer->getEntityRowId(), creatures[i]);
			if (distance > (double)MaxDistanceForXpGain)
				continue;

			TCreatureTakenDamageContainer::iterator itc = _CreatureTakenDamage.find(creatures[i]);
			//nlassert(itc != _CreatureTakenDamage.end());
			BOMB_IF( itc == _CreatureTakenDamage.end(), "team has wounded a creature but creature has no registered damage", return);
			const sint16 index = (*itc).second.getIndexForTeam(teamId);
			//nlassert(index >= 0);
			BOMB_IF( index < 0, "team has wounded a creature but creature has no registered damage from team", return);

			(*itc).second.PlayerInflictedDamage[index].enableXP(actorPlayer->getId(), (uint16)actorPlayer->getSkillBaseValue(reportAction.Skill));

			TCharacterActionsContainer::iterator it = _CharacterActions.find( reportAction.TargetRowId );
			if( it != _CharacterActions.end() )
			{
				// ensure this healer is known as an adversary for this creature  (even for simulation)
//				CCreature *creature = CreatureManager.getCreature( creatures[i] );
//				if (creature)
//					creature->getCreatureOpponent().storeAggressor(reportAction.ActorRowId,0);

				(*itActions).second->addAction( creatures[i], reportAction.Skill, incActionCounter );
			}

		}
	}
}


//----------------------------------------------------------------------------
// simpleActionReported: process report for a simple action (ie action with immediat xp gain directly depends on delta level reported)
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::simpleActionReported( const TReportAction& reportAction, CEntityBase * actor, bool scaleForNewbies )
{
	CCharacter * pj = dynamic_cast< CCharacter * >( actor );
	if( pj )
	{
		TLogContext_Character_SkillProgress logContext(pj->getId());
#ifdef NL_DEBUG
		nldebug("simpleActionReported Delta Skill %d, Skill %s, Factor %1.2f", reportAction.DeltaLvl, SKILLS::toString(reportAction.Skill).c_str(), reportAction.factor );
#endif

		double xpGain = getXpGain( actor, reportAction.DeltaLvl, reportAction.Skill, reportAction.factor, SUCCESS_TABLE_TYPE::actionNatureToTableType(reportAction.ActionNature), scaleForNewbies );
		pj->addXpToSkill( xpGain, SKILLS::toString( reportAction.Skill ) );
	}
}


//----------------------------------------------------------------------------
// clearAllXpForPlayer: remove allocated structure for this character
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::clearAllXpForPlayer( TDataSetRow character, uint16 teamId, bool removeDamageFromTeam )
{
	TCharacterActionsContainer::iterator it = _CharacterActions.find( character );
	if( it != _CharacterActions.end() )
	{
		delete (*it).second;
		_CharacterActions.erase( it );
	}

	clearPlayerDamage( TheDataset.getEntityId(character), teamId, removeDamageFromTeam);
}


//----------------------------------------------------------------------------
// applyArmorWear: apply armor wear
//----------------------------------------------------------------------------
/*void CCharacterProgressionPVE::applyArmorWear( CCharacter * character, const TReportAction & reportAction )
{
	static const uint8 NbArmorSlots = 6;
	static const SLOT_EQUIPMENT::TSlotEquipment armorSlots[NbArmorSlots]=
	{
		SLOT_EQUIPMENT::HEAD,
		SLOT_EQUIPMENT::CHEST,
		SLOT_EQUIPMENT::ARMS,
		SLOT_EQUIPMENT::HANDS,
		SLOT_EQUIPMENT::LEGS,
		SLOT_EQUIPMENT::FEET
	};

	for (uint i = 0; i < NbArmorSlots ; ++i)
	{
		// get armor piece on player
		CGameItemPtr armorPtr = character->getItem(INVENTORIES::equipment,  armorSlots[i]);
		if (armorPtr == NULL)
			continue;

		const CStaticItem *form = armorPtr->getStaticForm();
		if ( form == NULL || form->Family != ITEMFAMILY::ARMOR)
			continue;

		// get the really equipped armor in bag
		bool hand;
		uint16 slotInBag;

		if ( !armorPtr->getSlotImage(hand,slotInBag) )
		{
			nlwarning("Player %s, Armor piece %s has no slot image but is equipped, BUG !", character->getId().toString().c_str(), armorPtr->getSheetId().toString().c_str());
			continue;
		}

		armorPtr = character->getItem(INVENTORIES::bag, slotInBag);
		if (armorPtr == NULL)
		{
			nlwarning("Player %s, Armor piece %s has a slot image but we cannot find item in bag, BUG !", character->getId().toString().c_str(), form->SheetId.toString().c_str());
			continue;
		}

		if (armorPtr->getItemWornState() != ITEM_WORN_STATE::Worned)
		{
			double wear = armorPtr->getWearPerAction();
			armorPtr->removeHp(wear);
		}
		else
		{
			nlwarning("Player armor is worned, destroy it");
		}

		// if armor now worned out, destroy it and send a chat message to player
		if (armorPtr->getItemWornState() == ITEM_WORN_STATE::Worned)
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
			params[0].SheetId = armorPtr->getSheetId();
			PHRASE_UTILITIES::sendDynamicSystemMessage( character->getEntityRowId(), "ITEM_WORNED_DESTROYED", params);

			// NB : if the item is equipped, destroy item also destroy the real item in bag
			character->destroyItem ( INVENTORIES::equipment, armorSlots[i], 1, false  );
		}
	}

} // applyArmorWear //


//----------------------------------------------------------------------------
// applyShieldWear: apply shield wear
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::applyShieldWear( CCharacter * character, const TReportAction & reportAction )
{
	CGameItemPtr shield = character->getLeftHandItem();
	if (shield == NULL)
		return;

	const CStaticItem * form = shield->getStaticForm();

	if (form == NULL || form->Family != ITEMFAMILY::SHIELD)
		return;

	if (shield->getItemWornState() != ITEM_WORN_STATE::Worned)
	{
		double wear = shield->getWearPerAction();
		shield->removeHp(wear);
	}
	else
	{
		nlwarning("Player shield is worned, destroy it");
	}

	// if shield now worned out, destroy it and send a chat message to player
	if (shield->getItemWornState() == ITEM_WORN_STATE::Worned)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
		params[0].SheetId = shield->getSheetId();
		PHRASE_UTILITIES::sendDynamicSystemMessage( character->getEntityRowId(), "ITEM_WORNED_DESTROYED", params);

		// NB : if the item is equipped, destroy item also destroy the real item in bag
		character->destroyItem ( INVENTORIES::handling, INVENTORIES::left, 1, false  );
	}

} // applyShieldWear

*/

//----------------------------------------------------------------------------
// CCharacterProgressionPVE::playerJoinsTeam
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::playerJoinsTeam(const NLMISC::CEntityId &playerId, uint16 teamId)
{
	// get creatures wounded by player
	TEntityAttackedCreature::iterator itPlayer = _PlayersWoundedCreatures.find(playerId);

	// player has inflicted no damage on a creature yet, so nothing to do
	if (itPlayer == _PlayersWoundedCreatures.end())
		return;

	vector<TDataSetRow> &playerAttackedCreatures = (*itPlayer).second;

	TTeamsAttackedCreature::iterator itTeam = _TeamsWoundedCreatures.find(teamId);
	// team has inflicted no damage on a creature yet
	if (itTeam == _TeamsWoundedCreatures.end())
	{
		for (vector<TDataSetRow>::iterator it = playerAttackedCreatures.begin() ; it != playerAttackedCreatures.end() ; ++it)
		{
			// get creature taken damage
			TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(*it);
			//nlassert(itCreature != _CreatureTakenDamage.end());
			BOMB_IF( itCreature == _CreatureTakenDamage.end(), "PROGRESSION_BUG: Player joins team, has damage registered on a creature but creature has no registered damage", return);

			const sint16 index = (*itCreature).second.getIndexForPlayer(playerId);
			BOMB_IF( index < 0, "PROGRESSION_BUG: Player joins team, has damage registered on a creature but creature has no registered damage for this player", return);
			//nlassert(index >= 0);
			(*itCreature).second.PlayerInflictedDamage[index].PlayerId = CEntityId::Unknown;
			(*itCreature).second.PlayerInflictedDamage[index].TeamId = teamId;
		}

		_TeamsWoundedCreatures.insert( make_pair(teamId, playerAttackedCreatures) );
	}
	else
	{
		vector<TDataSetRow> &teamAttackedCreatures = (*itTeam).second;

		for ( uint i = 0 ; i < playerAttackedCreatures.size() ; ++i)
		{
			bool found = false;
			for ( uint j = 0 ; j < teamAttackedCreatures.size() ; ++j)
			{
				if ( playerAttackedCreatures[i] == teamAttackedCreatures[j])
				{
					//get stats on creature
					TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(playerAttackedCreatures[i]);
					BOMB_IF( itCreature == _CreatureTakenDamage.end(), "PROGRESSION_BUG: Player joins team, has damage registered on a creature but creature has no registered damage", return);

					const sint16 indexTeam = (*itCreature).second.getIndexForTeam(teamId);
					//nlassert(indexTeam >= 0);
					BOMB_IF( indexTeam < 0, "PROGRESSION_BUG: Player joins team, team has damage registered on a creature but creature has no registered damage for this team", return);
					const sint16 indexPlayer = (*itCreature).second.getIndexForPlayer(playerId);
					//nlassert(indexPlayer >= 0);
					BOMB_IF( indexPlayer < 0, "PROGRESSION_BUG: Player joins team, has damage registered on a creature but creature has no registered damage for this player", return);

					(*itCreature).second.PlayerInflictedDamage[indexTeam].TotalDamage += (*itCreature).second.PlayerInflictedDamage[indexPlayer].TotalDamage;

					// remove entry for player
					(*itCreature).second.PlayerInflictedDamage.erase( (*itCreature).second.PlayerInflictedDamage.begin() + indexPlayer );

					found = true;
				}
			}

			if (!found)
			{
				TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(playerAttackedCreatures[i]);
				//nlassert(itCreature != _CreatureTakenDamage.end());
				BOMB_IF( itCreature == _CreatureTakenDamage.end(), "PROGRESSION_BUG: Player joins team, has damage registered on a creature but creature has no registered damage", return);

				const sint16 index = (*itCreature).second.getIndexForPlayer(playerId);
				//nlassert(index >= 0);
				BOMB_IF( index < 0, "PROGRESSION_BUG: Player joins team, has damage registered on a creature but creature has no registered damage for this player", return);
				(*itCreature).second.PlayerInflictedDamage[index].PlayerId = CEntityId::Unknown;
				(*itCreature).second.PlayerInflictedDamage[index].TeamId = teamId;

				teamAttackedCreatures.push_back(playerAttackedCreatures[i]);
			}
		}
	}

	_PlayersWoundedCreatures.erase(itPlayer);
}

//----------------------------------------------------------------------------
// CCharacterProgressionPVE::playerLeavesTeam
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::playerLeavesTeam(const NLMISC::CEntityId &playerId, uint16 teamId)
{
	TTeamsAttackedCreature::iterator itTeam = _TeamsWoundedCreatures.find(teamId);
	// team has inflicted no damage on a creature yet
	if (itTeam == _TeamsWoundedCreatures.end())
		return;

	vector<TDataSetRow> &teamAttackedCreatures = (*itTeam).second;
	vector<TDataSetRow> playerAttackedCreatures;

	uint8 teamSize;

	for ( uint i = 0 ; i < teamAttackedCreatures.size() ; ++i)
	{
		TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(teamAttackedCreatures[i]);
		//nlassert(itCreature != _CreatureTakenDamage.end());
		BOMB_IF( itCreature == _CreatureTakenDamage.end(), "PROGRESSION_BUG: Player leaves team,team has damage registered on a creature but creature has no registered damage", return);

		const sint16 index = (*itCreature).second.getIndexForTeam(teamId);
		//nlassert(index >= 0);
		BOMB_IF( index < 0, "PROGRESSION_BUG: Player leaves team, team has damage registered on a creature but creature has no registered damage for this team", return);

		teamSize = 0;
		bool found = false;
		for (uint j = 0 ; j < (*itCreature).second.PlayerInflictedDamage[index].TeamMembers.size() ; ++j)
		{
			if ((*itCreature).second.PlayerInflictedDamage[index].TeamMembers[j].GainXp)
			{
				if ((*itCreature).second.PlayerInflictedDamage[index].TeamMembers[j].Id == playerId)
					found = true;

				++teamSize;
			}
		}

		if (!found)
			continue;

		playerAttackedCreatures.push_back((*itCreature).first);

		CTeamDamage damage(playerId);
		damage.TotalDamage = (*itCreature).second.PlayerInflictedDamage[index].TotalDamage / teamSize;

		(*itCreature).second.PlayerInflictedDamage[index].TotalDamage -= damage.TotalDamage;
		(*itCreature).second.PlayerInflictedDamage[index].removeMember(playerId);

		// vl: do that to fix the 3k XP exploit when you quit a team after almost kill a mob
		damage.MaxSkillValue = (*itCreature).second.PlayerInflictedDamage[index].MaxSkillValue;

		// add damage on this creature for removed player
		(*itCreature).second.PlayerInflictedDamage.push_back(damage);
	}

	_PlayersWoundedCreatures.insert( make_pair(playerId, playerAttackedCreatures) );
}

//----------------------------------------------------------------------------
// CCharacterProgressionPVE::disbandTeam
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::disbandTeam(uint16 teamId, const list<CEntityId> &teamMembers)
{
	/************************************************************************/
	/* Todo : optimisation
	/************************************************************************/
	TTeamsAttackedCreature::iterator itTeam = _TeamsWoundedCreatures.find(teamId);
	// team has inflicted no damage on a creature yet, return
	if (itTeam == _TeamsWoundedCreatures.end())
		return;

	list<CEntityId>::const_iterator it;
	const list<CEntityId>::const_iterator itEnd = teamMembers.end();
	for (it = teamMembers.begin() ; it != itEnd ; ++it)
	{
		playerLeavesTeam(*it, teamId);
	}

	vector<TDataSetRow> &teamAttackedCreatures = (*itTeam).second;

	// clear damage entries on creatures for this team
	for ( uint i = 0 ; i < teamAttackedCreatures.size() ; ++i)
	{
		TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(teamAttackedCreatures[i]);
		BOMB_IF( itCreature == _CreatureTakenDamage.end(), "PROGRESSION_BUG : disband Team,team has damage registered on a creature but creature has no registered damage", return);

		const sint16 index = (*itCreature).second.getIndexForTeam(teamId);
		BOMB_IF( index < 0, "PROGRESSION_BUG: Player leaves team, team has damage registered on a creature but creature has no registered damage for this team", return);

		// clear damage for this team on this creature
		(*itCreature).second.PlayerInflictedDamage.erase( (*itCreature).second.PlayerInflictedDamage.begin() + index );
	}

	// clear team registered damage
	_TeamsWoundedCreatures.erase(itTeam);

/*	vector<TDataSetRow> &teamAttackedCreatures = (*itTeam).second;

	set<CEntityId> teamMembers;

	// parse all creatures attacked by team
	for ( uint i = 0 ; i < teamAttackedCreatures.size() ; ++i)
	{
		TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(teamAttackedCreatures[i]);
		nlassert(itCreature != _CreatureTakenDamage.end());

		const sint16 index = (*itCreature).second.getIndexForTeam(teamId);
		nlassert(index >= 0);

		CTeamDamage &teamDamage = (*itCreature).second.PlayerInflictedDamage[index];

		// get the number of team members that can gain XP on this creature
		uint teamSize = 0;
		for (uint j = 0 ; j < teamDamage.TeamMembers.size() ; ++j)
		{
			if (teamDamage.TeamMembers[j].GainXp)
			{
				++teamSize;
			}
		}

		// compute the part of total damage attributed to each team member
		const uint32 damage = teamDamage.TotalDamage / teamSize;

		// add this part to each team member that can gain XP

		for (uint j = 0 ; j < teamDamage.TeamMembers.size() ; ++j)
		{
			if (teamDamage.TeamMembers[j].GainXp)
			{
				CTeamDamage damagePlayer(teamDamage.TeamMembers[j].Id);
				damagePlayer.TotalDamage = damage;
				(*itCreature).second.PlayerInflictedDamage.push_back(damagePlayer);

				// add this creature to the player attacked creatures
			}
		}

		(*itCreature).second.PlayerInflictedDamage.erase((*itCreature).second.PlayerInflictedDamage.begin()+index);
	}

	for (list<CEntityId>::const_iterator it = teamMembers.begin() ; it != teamMembers.end() ; ++it)
	{
		_PlayersWoundedCreatures.insert( make_pair( (*it), teamAttackedCreatures) );
	}
*/

}

//----------------------------------------------------------------------------
// CCharacterProgressionPVE::clearPlayerDamage
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::clearPlayerDamage(const NLMISC::CEntityId &playerId, uint16 teamId, bool removeDamageFromTeam)
{
	if ( teamId != CTEAM::InvalidTeamId )
	{
		TTeamsAttackedCreature::iterator itTeam = _TeamsWoundedCreatures.find(teamId);
		// team has inflicted no damage on a creature yet
		if (itTeam == _TeamsWoundedCreatures.end())
			return;

		vector<TDataSetRow> &teamAttackedCreatures = (*itTeam).second;

		// get team
		const CTeam *team = TeamManager.getTeam(teamId);
		if (!team)
			return;

		for ( uint i = 0 ; i < teamAttackedCreatures.size() ; )
		{
			TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(teamAttackedCreatures[i]);
			//nlassert(itCreature != _CreatureTakenDamage.end());
			BOMB_IF( itCreature == _CreatureTakenDamage.end(), "PROGRESSION_BUG: clear player damage, remfromteam, found attacked creature with no registered damage ", return);

			const sint16 index = (*itCreature).second.getIndexForTeam(teamId);
			//nlassert(index >= 0);
			BOMB_IF( index < 0, "PROGRESSION_BUG: clear player damage, remfromteam, found attacked creature with no registered damage for this team", return);

			if (removeDamageFromTeam == true)
			{
				uint8 teamSize = (uint8)(*itCreature).second.PlayerInflictedDamage[index].TeamMembers.size();
				(*itCreature).second.PlayerInflictedDamage[index].TotalDamage -= (*itCreature).second.PlayerInflictedDamage[index].TotalDamage / teamSize;
			}

			(*itCreature).second.PlayerInflictedDamage[index].removeMember(playerId);
			// if no members remains, remove entry
			if ( (*itCreature).second.PlayerInflictedDamage[index].TeamMembers.empty() )
			{
				(*itCreature).second.PlayerInflictedDamage[index] = (*itCreature).second.PlayerInflictedDamage.back();
				(*itCreature).second.PlayerInflictedDamage.pop_back();

				teamAttackedCreatures[i] = teamAttackedCreatures.back();
				teamAttackedCreatures.pop_back();
				// do not inc i
			}
			else
			{
				++i;
			}
		}
	}
	else
	{
		TEntityAttackedCreature::iterator itPlayer = _PlayersWoundedCreatures.find(playerId);

		// player has inflicted no damage on a creature yet, so nothing to do
		if (itPlayer == _PlayersWoundedCreatures.end())
			return;

		vector<TDataSetRow> &playerAttackedCreatures = (*itPlayer).second;

		for ( uint i = 0 ; i < playerAttackedCreatures.size() ; ++i)
		{
			TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(playerAttackedCreatures[i]);
			//nlassert(itCreature != _CreatureTakenDamage.end());
			BOMB_IF( itCreature == _CreatureTakenDamage.end(), "PROGRESSION_BUG: clear player damage, found attacked creature with no registered damage ", return);

			const sint16 index = (*itCreature).second.getIndexForPlayer(playerId);
			//nlassert(index >= 0);
			BOMB_IF( index < 0, "PROGRESSION_BUG: clear player damage, found attacked creature with no registered damage for this player", return);
			(*itCreature).second.PlayerInflictedDamage.erase((*itCreature).second.PlayerInflictedDamage.begin() + index);
		}

		_PlayersWoundedCreatures.erase(itPlayer);
	}
}

//----------------------------------------------------------------------------
// CCharacterProgressionPVE::clearCreatureInflictedDamage
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::clearCreatureInflictedDamage(TDataSetRow creature)
{
	if (!creature.isValid() || !TheDataset.isDataSetRowStillValid(creature))
		return;

	const CEntityId creatureId = TheDataset.getEntityId(creature);

	TEntityAttackedCreature::iterator itCreature = _CreaturesWoundedCreatures.find(creatureId);

	// creature has inflicted no damage on a creature yet, so nothing to do
	if (itCreature == _CreaturesWoundedCreatures.end())
		return;

	vector<TDataSetRow> &attackedCreatures = (*itCreature).second;

	for ( uint i = 0 ; i < attackedCreatures.size() ; ++i)
	{
		TCreatureTakenDamageContainer::iterator itTaken = _CreatureTakenDamage.find(attackedCreatures[i]);
		BOMB_IF( itTaken == _CreatureTakenDamage.end(), "PROGRESSION_BUG: clear creature inflicted damage, found attacked creature with no registered damage ", return);

		CCreatureTakenDamage &takenDmg = (*itTaken).second;

		const sint16 index = takenDmg.getIndexForCreature(creatureId);
		BOMB_IF( index < 0, "PROGRESSION_BUG: clear creature inflicted damage, found attacked creature with no registered damage for removed creature", return);

		takenDmg.TotalCreatureInflictedDamage -= takenDmg.CreatureInflictedDamage[index].TotalDamage;

		takenDmg.CreatureInflictedDamage.erase(takenDmg.CreatureInflictedDamage.begin() + index);
	}

	_CreaturesWoundedCreatures.erase(itCreature);
}

//----------------------------------------------------------------------------
// CCharacterProgressionPVE::lostAggro
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::lostAggro(TDataSetRow creature, TDataSetRow player)
{
	if (!TheDataset.isAccessible(player))
		return;

	// forget the xp gain
	forgetXpGain(creature, player);

	CEntityId playerId = TheDataset.getEntityId(player);

	// get creatures wounded by player
	TEntityAttackedCreature::iterator itPlayer = _PlayersWoundedCreatures.find(playerId);

	if (itPlayer != _PlayersWoundedCreatures.end())
	{
		vector<TDataSetRow> &playerAttackedCreatures = (*itPlayer).second;
		for ( uint i = 0 ; i < playerAttackedCreatures.size() ; ++i)
		{
			if (playerAttackedCreatures[i] == creature)
			{
				// clear player damage against this creature
				TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(creature);
				//nlassert(itCreature != _CreatureTakenDamage.end());
				BOMB_IF( itCreature == _CreatureTakenDamage.end(), "PROGRESSION_BUG: player lost aggro, found attacked creature with no registered damage ", return);

				const sint16 index = (*itCreature).second.getIndexForPlayer(playerId);
				//nlassert(index >= 0);
				BOMB_IF( index < 0, "PROGRESSION_BUG: player lost aggro, found attacked creature with no registered damage for this player", return);

				playerAttackedCreatures.erase(playerAttackedCreatures.begin() + i);
			//	(*itCreature).second.PlayerInflictedDamage.erase((*itCreature).second.PlayerInflictedDamage.begin() + index);
				CCharacter *playerChar = PlayerManager.getChar(playerId);
				if (playerChar)
					(*itCreature).second.transferDamageOnFictitiousCreature(playerId, playerChar->getTeamId());
				else
					(*itCreature).second.transferDamageOnFictitiousCreature(playerId, CTEAM::InvalidTeamId);

				return;
			}
		}

		return;
	}

	CCharacter *playerChar = PlayerManager.getChar(playerId);
	if (!playerChar)
		return;

	// get player team
	const uint16 teamId = playerChar->getTeamId();
	if (teamId == CTEAM::InvalidTeamId)
		return;

	TTeamsAttackedCreature::iterator itTeam = _TeamsWoundedCreatures.find(teamId);
	if (itTeam == _TeamsWoundedCreatures.end())
		return;

	// if team members no longer has aggro with this creature, remove all entries for it
	if ( !checkAggroForTeam(teamId, creature) )
	{
		vector<TDataSetRow> &teamAttackedCreatures = (*itTeam).second;
		for ( uint i = 0 ; i < teamAttackedCreatures.size() ; ++i)
		{
			if ( teamAttackedCreatures[i] == creature)
			{
				TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(teamAttackedCreatures[i]);
				//nlassert(itCreature != _CreatureTakenDamage.end());
				BOMB_IF( itCreature == _CreatureTakenDamage.end(), "PROGRESSION_BUG: player lost aggro (is in team), found attacked creature with no registered damage ", return);

				const sint16 index = (*itCreature).second.getIndexForTeam(teamId);
				//nlassert(index >= 0);
				BOMB_IF( index < 0, "PROGRESSION_BUG: player lost aggro (is in team), found attacked creature with no registered damage for this team", return);

				teamAttackedCreatures.erase(teamAttackedCreatures.begin() + i);
			//	(*itCreature).second.PlayerInflictedDamage.erase((*itCreature).second.PlayerInflictedDamage.begin() + index);
				CCharacter *playerChar = PlayerManager.getChar(playerId);
				if (playerChar)
					(*itCreature).second.transferDamageOnFictitiousCreature(playerId, playerChar->getTeamId());
				else
					(*itCreature).second.transferDamageOnFictitiousCreature(playerId, CTEAM::InvalidTeamId);

				return;
			}
		}
	}
}


//----------------------------------------------------------------------------
// CCharacterProgressionPVE::checkAggroForTeam
//----------------------------------------------------------------------------
bool CCharacterProgressionPVE::checkAggroForTeam( uint16 teamId, TDataSetRow creatureRowId )
{
	CTeam *team = TeamManager.getTeam(teamId);
	if (!team)
		return false;

	CCreature *creature = CreatureManager.getCreature(creatureRowId);
	if (!creature)
		return false;

	const std::set<TDataSetRow> &aggrolist = creature->getAggroList();

	const list<CEntityId> &members = team->getTeamMembers();
	for ( list<CEntityId>::const_iterator it = members.begin() ; it != members.end() ; ++it )
	{
		TDataSetRow row = TheDataset.getDataSetRow(*it);
		if ( aggrolist.find(row) != aggrolist.end())
		{
			return true;
		}
	}

	return false;
}


//----------------------------------------------------------------------------
// CCharacterProgressionPVE::addDamage
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::addDamage(const NLMISC::CEntityId &actorId, const NLMISC::CEntityId &targetId, uint32 damage)
{
	// only keep damage inflicted on creature or npc
	if (targetId.getType() == RYZOMID::player)
		return;

	CCreatureTakenDamage *takenDmg = NULL;

	TDataSetRow targetRowId = TheDataset.getDataSetRow(targetId);

	TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(targetRowId);
	if (itCreature != _CreatureTakenDamage.end())
	{
		takenDmg = &((*itCreature).second);
	}
	else
	{
		CCreatureTakenDamage dmg;
		pair<TCreatureTakenDamageContainer::iterator, bool> retPair = _CreatureTakenDamage.insert( make_pair(targetRowId, dmg) );
		if (!retPair.second)
		{
			nlwarning("Failed to insert entry in _CreatureTakenDamage map for entity %s", targetId.toString().c_str());
			return;
		}

		takenDmg = &((*retPair.first).second);
	}

	BOMB_IF(takenDmg==NULL,"(BUG) TakenDmg == NULL",return);

	if (actorId.getType() != RYZOMID::player)
	{
		takenDmg->TotalCreatureInflictedDamage += damage;

		const sint16 index = takenDmg->getIndexForCreature(actorId);
		if (index >= 0)
		{
			takenDmg->CreatureInflictedDamage[index].TotalDamage += damage;
		}
		else
		{
			CCreatureInflictedDamage creatureDamage(actorId);
			creatureDamage.TotalDamage = (float)damage;
			takenDmg->CreatureInflictedDamage.push_back(creatureDamage);

			TEntityAttackedCreature::iterator itCreature = _CreaturesWoundedCreatures.find(actorId);
			if (itCreature != _CreaturesWoundedCreatures.end())
			{
				(*itCreature).second.push_back( targetRowId );
			}
			else
			{
				vector<TDataSetRow> creatures;
				creatures.push_back(targetRowId);
				_CreaturesWoundedCreatures.insert(make_pair(actorId, creatures));
			}
		}
	}
	else
	{
		CCharacter *playerChar = PlayerManager.getChar(actorId);
		if (!playerChar)
			return;

		const uint16 teamId = playerChar->getTeamId();
		if ( teamId != CTEAM::InvalidTeamId)
		{
			// player is in a team
			const sint16 index = takenDmg->getIndexForTeam(teamId);
			if (index >= 0)
			{
				takenDmg->PlayerInflictedDamage[index].TotalDamage += damage;
			}
			else
			{
				CTeamDamage teamDamage(teamId);
				teamDamage.TotalDamage = (float)damage;
				takenDmg->PlayerInflictedDamage.push_back(teamDamage);

				TTeamsAttackedCreature::iterator itTeam = _TeamsWoundedCreatures.find(teamId);
				if (itTeam != _TeamsWoundedCreatures.end())
				{
					(*itTeam).second.push_back( targetRowId );
				}
				else
				{
					vector<TDataSetRow> creatures;
					creatures.push_back(targetRowId);
					itTeam = _TeamsWoundedCreatures.insert(make_pair(teamId, creatures)).first;
					//nlassert(itTeam != _TeamsWoundedCreatures.end());
				}
			}
		}
		else
		{
			// player isn't in a team
			const sint16 index = takenDmg->getIndexForPlayer(actorId);
			if (index >= 0)
			{
				takenDmg->PlayerInflictedDamage[index].TotalDamage += damage;
			}
			else
			{
				CTeamDamage playerDamage(actorId);
				playerDamage.TotalDamage = (float)damage;
				takenDmg->PlayerInflictedDamage.push_back(playerDamage);

				TEntityAttackedCreature::iterator itPlayer = _PlayersWoundedCreatures.find(actorId);
				if (itPlayer != _PlayersWoundedCreatures.end())
				{
					(*itPlayer).second.push_back( targetRowId );
				}
				else
				{
					vector<TDataSetRow> creatures;
					creatures.push_back(targetRowId);
					_PlayersWoundedCreatures.insert(make_pair(actorId, creatures));
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
// CCharacterProgressionPVE::enableXpGain
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::enableXpGainForPlayer(const CEntityId &playerId, uint16 skillValue, list<TDataSetRow> &enabledCreatures)
{
	CCharacter *playerChar = PlayerManager.getChar(playerId);
	if (!playerChar)
		return;

	enabledCreatures.clear();

	const uint16 teamId = playerChar->getTeamId();
	if ( teamId == CTEAM::InvalidTeamId)
	{
		TEntityAttackedCreature::iterator itPlayer = _PlayersWoundedCreatures.find(playerId);
		if (itPlayer != _PlayersWoundedCreatures.end())
		{
			for (uint i = 0 ; i < (*itPlayer).second.size() ; ++i)
			{
				enabledCreatures.push_back((*itPlayer).second[i]);

				TCreatureTakenDamageContainer::iterator itc = _CreatureTakenDamage.find((*itPlayer).second[i]);
				//nlassert(itc != _CreatureTakenDamage.end());
				BOMB_IF( itc == _CreatureTakenDamage.end(), "player has wounded a creature but creature has no registered damage", return);
				const sint16 index = (*itc).second.getIndexForPlayer(playerId);
				//nlassert(index >= 0);
				BOMB_IF( index < 0, "player has wounded a creature but creature has no registered damage from player", return);

				if ( (*itc).second.PlayerInflictedDamage[index].MaxSkillValue < skillValue)
					(*itc).second.PlayerInflictedDamage[index].MaxSkillValue = skillValue;
			}
		}
	}
	else
	{
		TTeamsAttackedCreature::iterator itTeam = _TeamsWoundedCreatures.find(teamId);
		if (itTeam == _TeamsWoundedCreatures.end())
		{
			return;
		}

		vector<TDataSetRow> &creatures = (*itTeam).second;
		for (uint i = 0 ; i < creatures.size() ; ++i)
		{
			// check distance between player and creature, if too far away, do not enable xp gain
			const double distance = PHRASE_UTILITIES::getDistance(playerChar->getEntityRowId(), creatures[i]);
			if (distance > (double)MaxDistanceForXpGain)
				continue;

			TCreatureTakenDamageContainer::iterator itc = _CreatureTakenDamage.find(creatures[i]);
			//nlassert(itc != _CreatureTakenDamage.end());
			BOMB_IF( itc == _CreatureTakenDamage.end(), "team has wounded a creature but creature has no registered damage", return);
			const sint16 index = (*itc).second.getIndexForTeam(teamId);
			//nlassert(index >= 0);
			BOMB_IF( index < 0, "team has wounded a creature but creature has no registered damage from team", return);

			(*itc).second.PlayerInflictedDamage[index].enableXP(playerId, skillValue);

			enabledCreatures.push_back(creatures[i]);
		}
	}
}

//----------------------------------------------------------------------------
// CCharacterProgressionPVE::removeCreature
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::removeXpCreature(TDataSetRow creature)
{
	TCreatureTakenDamageContainer::iterator itCreatureDamage = _CreatureTakenDamage.find(creature);
	if (itCreatureDamage == _CreatureTakenDamage.end())
		return;
	// clear damage inflicted by players and teams on this creature
	const vector<CTeamDamage> &damage = (*itCreatureDamage).second.PlayerInflictedDamage;
	for (uint i = 0 ; i < damage.size() ; ++i )
	{
		if (damage[i].TeamId != CTEAM::InvalidTeamId)
		{
			// forget Xp gain on this creature for whole team
			CTeam *team = TeamManager.getRealTeam(damage[i].TeamId);
			if (team)
			{
				const list<CEntityId> &teamMembers = team->getTeamMembers();

				for ( list<CEntityId>::const_iterator it = teamMembers.begin() ; it != teamMembers.end() ; ++it)
				{
					forgetXpGain(creature, TheDataset.getDataSetRow(*it));
				}
			}
		}
		else
		{
			// forget Xp gain on this creature
			forgetXpGain(creature, TheDataset.getDataSetRow(damage[i].PlayerId));
		}
	}
}

//----------------------------------------------------------------------------
// CCharacterProgressionPVE::removeCreature
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::removeCreature(TDataSetRow creature)
{
	// remove creature inflicted damage
	clearCreatureInflictedDamage(creature);

	// get damage inflicted on creature
	TCreatureTakenDamageContainer::iterator itCreatureDamage = _CreatureTakenDamage.find(creature);
	if (itCreatureDamage == _CreatureTakenDamage.end())
		return;

	// clear damage inflicted by players and teams on this creature
	const vector<CTeamDamage> &damage = (*itCreatureDamage).second.PlayerInflictedDamage;
	for (uint i = 0 ; i < damage.size() ; ++i )
	{
		if (damage[i].TeamId != CTEAM::InvalidTeamId)
		{
			TTeamsAttackedCreature::iterator itTeam = _TeamsWoundedCreatures.find(damage[i].TeamId);

			BOMB_IF( itTeam == _TeamsWoundedCreatures.end(), "creature memorized damage from team but team has no registered damage", return);

			vector<TDataSetRow> &creatures = (*itTeam).second;

			uint j;
			const uint size = (uint)creatures.size();
			for (j = 0  ; j < size ; ++j)
			{
				if (creatures[j] == creature)
				{
					creatures.erase(creatures.begin()+j);
					break;
				}
			}

			BOMB_IF( j >= size, "creature memorized damage from team but team has no registered damage on this creature", return);

			if (creatures.empty())
				_TeamsWoundedCreatures.erase(itTeam);

			// forget Xp gain on this creature for whole team
			CTeam *team = TeamManager.getRealTeam(damage[i].TeamId);
			if (team)
			{
				const list<CEntityId> &teamMembers = team->getTeamMembers();

				for ( list<CEntityId>::const_iterator it = teamMembers.begin() ; it != teamMembers.end() ; ++it)
				{
					forgetXpGain(creature, TheDataset.getDataSetRow(*it));
				}
			}
		}
		else
		{
			TEntityAttackedCreature::iterator itPlayer = _PlayersWoundedCreatures.find(damage[i].PlayerId);

			BOMB_IF( itPlayer == _PlayersWoundedCreatures.end(), "creature memorized damage from player but player has no registered damage", return);
			vector<TDataSetRow> &creatures = (*itPlayer).second;

			uint j;
			const uint size = (uint)creatures.size();
			for (j = 0  ; j < size ; ++j)
			{
				if (creatures[j] == creature)
				{
					creatures.erase(creatures.begin()+j);
					break;
				}
			}

			BOMB_IF( j >= size, "creature memorized damage from player but player has no registered damage on this creature", return);

			if (creatures.empty())
				_PlayersWoundedCreatures.erase(itPlayer);

			// forget Xp gain on this creature
			forgetXpGain(creature, TheDataset.getDataSetRow(damage[i].PlayerId));
		}
	}

	// clear damage inflicted by other creatures on this creature
	vector<CCreatureInflictedDamage> &creatureDamage = (*itCreatureDamage).second.CreatureInflictedDamage;
	for (uint i = 0 ; i < creatureDamage.size() ; ++i )
	{
		// skip fake creature
		if ( creatureDamage[i].CreatureId != CEntityId::Unknown )
		{
			TEntityAttackedCreature::iterator itCreature = _CreaturesWoundedCreatures.find(creatureDamage[i].CreatureId);
			BOMB_IF( itCreature == _CreaturesWoundedCreatures.end(), "creature memorized damage from creature but second creature has no registered damage", return);
			vector<TDataSetRow> &creatures = (*itCreature).second;

			uint j;
			const uint size = (uint)creatures.size();
			for (j = 0  ; j < size ; ++j)
			{
				if (creatures[j] == creature)
				{
					creatures.erase(creatures.begin()+j);
					break;
				}
			}
			BOMB_IF( j >= size, "creature memorized damage from player but player has no registered damage on this creature", return);

			if (creatures.empty())
				_CreaturesWoundedCreatures.erase(itCreature);
		}
	}

	_CreatureTakenDamage.erase(itCreatureDamage);
}


//----------------------------------------------------------------------------
// CCharacterProgressionPVE::removeCreature
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::applyRegenHP(TDataSetRow creature, sint32 regenHP)
{
	// get damage inflicted on creature
	TCreatureTakenDamageContainer::iterator itCreatureDamage = _CreatureTakenDamage.find(creature);
	if (itCreatureDamage == _CreatureTakenDamage.end())
		return;

	(*itCreatureDamage).second.applyRegenHP(regenHP);
}

//----------------------------------------------------------------------------
// CCharacterProgressionPVE::transferDamageOnFictitiousCreature
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::transferDamageOnFictitiousCreature(TDataSetRow playerRowId, uint16 teamId)
{
	if ( !TheDataset.isAccessible(playerRowId) )
		return;

	CEntityId playerId = TheDataset.getEntityId(playerRowId);

	// get creatures wounded by player
	TEntityAttackedCreature::iterator itPlayer = _PlayersWoundedCreatures.find(playerId);
	if (itPlayer != _PlayersWoundedCreatures.end())
	{
		vector<TDataSetRow> &playerAttackedCreatures = (*itPlayer).second;
		uint nbCreatures = (uint)playerAttackedCreatures.size();
		while (nbCreatures > 0)
		{
			// clear player damage against this creature
			TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(playerAttackedCreatures[nbCreatures-1]);
			BOMB_IF( itCreature == _CreatureTakenDamage.end(), "PROGRESSION_BUG: found attacked creature with no registered damage ", return);

			const sint16 index = (*itCreature).second.getIndexForPlayer(playerId);
			BOMB_IF( index < 0, "PROGRESSION_BUG: found attacked creature with no registered damage for this player", return);

			(*itCreature).second.transferDamageOnFictitiousCreature(playerId, teamId);

			playerAttackedCreatures.pop_back();

			--nbCreatures;
		}

		_PlayersWoundedCreatures.erase(itPlayer);
		return;
	}
	// if the player wounded no creature
	CCharacter *playerChar = PlayerManager.getChar(playerId);
	if (!playerChar)
		return;

	// player is in team
	if (teamId == CTEAM::InvalidTeamId)
		return;

	TTeamsAttackedCreature::iterator itTeam = _TeamsWoundedCreatures.find(teamId);
	if (itTeam == _TeamsWoundedCreatures.end())
		return;

	vector<TDataSetRow> &teamAttackedCreatures = (*itTeam).second;
	for ( uint i = 0 ; i < teamAttackedCreatures.size() ; )
	{
		TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(teamAttackedCreatures[i]);
		BOMB_IF( itCreature == _CreatureTakenDamage.end(), "PROGRESSION_BUG: (is in team), found attacked creature with no registered damage ", return);

		const sint16 index = (*itCreature).second.getIndexForTeam(teamId);
		BOMB_IF( index < 0, "PROGRESSION_BUG: (is in team), found attacked creature with no registered damage for this team", return);

		if ( (*itCreature).second.transferDamageOnFictitiousCreature(playerId, teamId) )
		{
			teamAttackedCreatures[i] = teamAttackedCreatures.back();
			teamAttackedCreatures.pop_back();
			//do not inc i
		}
		else
		{
			++i;
		}
	}
}

//----------------------------------------------------------------------------
// CCharacterProgressionPVE::transferDamageOnFictitiousCreature
//----------------------------------------------------------------------------
void CCharacterProgressionPVE::transferDamageOnFictitiousCreature(TDataSetRow playerRowId, uint16 teamId, TDataSetRow creatureRowId)
{
	nlerror("This function is not used, so may be really buggy");

	if ( !TheDataset.isAccessible(playerRowId) )
		return;

	CEntityId playerId = TheDataset.getEntityId(playerRowId);

	// Find creature
	TCreatureTakenDamageContainer::iterator itCreature = _CreatureTakenDamage.find(creatureRowId);
	if (itCreature != _CreatureTakenDamage.end() )
	{
		// Find player in creature list
		const sint16 index = (*itCreature).second.getIndexForPlayer(playerId);
		if (index >= 0)
		{
			// Transfer damage
			(*itCreature).second.transferDamageOnFictitiousCreature(playerId, teamId);
			// Clean player
			// Find player
			TEntityAttackedCreature::iterator itPlayer = _PlayersWoundedCreatures.find(playerId);
			if (itPlayer != _PlayersWoundedCreatures.end())
			{
				// Find creature in player list
				vector<TDataSetRow> &playerAttackedCreatures = (*itPlayer).second;
				vector<TDataSetRow>::iterator it = std::find(playerAttackedCreatures.begin(), playerAttackedCreatures.end(), creatureRowId);
				BOMB_IF(it==playerAttackedCreatures.end(), "Error : creature attacked by player but not found in playerAttackedCreature", return);
				// Erase creature from player list
				playerAttackedCreatures.erase(it);
			}
			return;
		}
	}

	CCharacter *playerChar = PlayerManager.getChar(playerId);
	if (!playerChar)
		return;

	// player is in team
	if (teamId == CTEAM::InvalidTeamId)
		return;

	// remove damage from team to target
	itCreature = _CreatureTakenDamage.find(creatureRowId);
	if (itCreature != _CreatureTakenDamage.end() )
	{
		const sint16 index = (*itCreature).second.getIndexForTeam(teamId);
		if (index >= 0)
		{
			(*itCreature).second.transferDamageOnFictitiousCreature(playerId, teamId);

			TTeamsAttackedCreature::iterator itTeam = _TeamsWoundedCreatures.find(teamId);
			if (itTeam != _TeamsWoundedCreatures.end())
			{
				vector<TDataSetRow> &teamAttackedCreatures = (*itTeam).second;
				vector<TDataSetRow>::iterator it = std::find(teamAttackedCreatures.begin(), teamAttackedCreatures.end(), creatureRowId);
				BOMB_IF(it==teamAttackedCreatures.end(), "Error : creature attacked by team but not found in teamAttackedCreatures", return);
				teamAttackedCreatures.erase(it);
			}

			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------------
// dtor
//----------------------------------------------------------------------------
CCharacterActions::~CCharacterActions()
{
	for( TSkillProgressPerOpponentContainer::iterator it = _SkillProgressPerOpponent.begin(); it != _SkillProgressPerOpponent.end(); ++it )
	{
		delete (*it).second;
	}
	_SkillProgressPerOpponent.clear();
}


//----------------------------------------------------------------------------
// addFightAction: add fight action
//----------------------------------------------------------------------------
void CCharacterActions::addAction( const TDataSetRow& target, SKILLS::ESkills skill, bool incActionCounter)
{
	TSkillProgressPerOpponentContainer::iterator it = _SkillProgressPerOpponent.find( target );
	if( it == _SkillProgressPerOpponent.end() )
	{
		CSkillProgress * ocp = new CSkillProgress();
		if( ocp )
		{
			pair< TSkillProgressPerOpponentContainer::iterator, bool > insertReturn;
			 insertReturn = _SkillProgressPerOpponent.insert( make_pair(target, ocp ) );
			if( insertReturn.second == false )
			{
				nlwarning("<CCharacterActions::addOffensiveAction> insert new pair<%s, COffensiveCharacterPerOpponent *> failed, can't continue skill progress process", target.toString().c_str() );
				return;
			}
			it = insertReturn.first;

			incActionCounter = true;
		}
		else
		{
			nlwarning("<CCharacterActions::addOffensiveAction> can't allocate COffensiveCharacterPerOpponent instance, memory full ?, can't continue skill progress process");
			return;
		}
	}

	if (incActionCounter)
		(*it).second->incNbAction(skill);
}

//----------------------------------------------------------------------------
// addCurativeAction: add curative action
//----------------------------------------------------------------------------
/*void CCharacterActions::addCurativeAction( const TReportAction& reportAction )
{
	// no xp gain if curative action restore no energy
	/*if( reportAction.Hp + reportAction.Sap + reportAction.Sta + reportAction.Focus == 0 )
		return 0;

	uint32 TotalHpDmg = 0;
	uint32 TotalSapDmg = 0;
	uint32 TotalStaDmg = 0;
	uint32 TotalFocusDmg = 0;

	double xpGainForHpRestore = 0.0f;
	double xpGainForSapRestore = 0.0f;
	double xpGainForStaRestore = 0.0f;
	double xpGainForFocusRestore = 0.0f;

	for( TSkillProgressPerOpponentContainer::iterator it = _SkillProgressPerOpponent.begin(); it != _SkillProgressPerOpponent.end(); ++it )
	{
		TotalHpDmg += (*it).second->hp();
		TotalSapDmg += (*it).second->sap();
		TotalStaDmg += (*it).second->sta();
		TotalFocusDmg += (*it).second->focus();
	}

	xpGainForHpRestore = xpGain * reportAction.Hp / ( reportAction.Hp + reportAction.Sap + reportAction.Sta + reportAction.Focus );
	xpGainForSapRestore = xpGain * reportAction.Sap / ( reportAction.Hp + reportAction.Sap + reportAction.Sta + reportAction.Focus );
	xpGainForStaRestore = xpGain * reportAction.Sta / ( reportAction.Hp + reportAction.Sap + reportAction.Sta + reportAction.Focus );
	xpGainForFocusRestore = xpGain * reportAction.Focus / ( reportAction.Hp + reportAction.Sap + reportAction.Sta + reportAction.Focus );

	uint32 Healed;
	double dispatchedXpGain = 0.0f;

	double totalXp = 0.0;

	for( TSkillProgressPerOpponentContainer::iterator it = _SkillProgressPerOpponent.begin(); it != _SkillProgressPerOpponent.end(); ++it )
	{
		// ensure this healer is known as an adversary for this creature  (even for simulation)
		CCreature *creature = CreatureManager.getCreature( (*it).first );
		if (creature)
			creature->getCreatureOpponent().storeAggressor(reportAction.ActorRowId,0);

		totalXp += (*it).second->Curative.addCurativeCharacter( reportAction.ActorRowId, (*it).first, reportAction.Skill, dispatchedXpGain, incActionCounter, simulateOnly );
	}

	return totalXp;
}
*/


//----------------------------------------------------------------------------
// dispatchXpGain: a creature dead dispatch xp gained for it
//----------------------------------------------------------------------------
bool CCharacterActions::dispatchXpGain( TDataSetRow actor, TDataSetRow creatureRowId, float equivalentXpMembers, float xpFactor, const list<CEntityId> &teamMembers )
{
	CCharacter * c = dynamic_cast< CCharacter * >( CEntityBaseManager::getEntityBasePtr( actor ) );
	if( c == 0 )
		return false; // actor is disconnected before creature death ?

	BOMB_IF(equivalentXpMembers < 1.0f, "equivalentXpMembers < 1!", equivalentXpMembers = 1.0f);

	float maxXPGain = 10;
	uint32 faction = CStaticFames::INVALID_FACTION_INDEX;
	bool creatureFameByKillValid = false;
	sint32 creatureFameByKill = 0;
	// get creature
	const CCreature *creature = CreatureManager.getCreature(creatureRowId);
	if (creature)
	{
		const CStaticCreatures* form = creature->getForm();
		if( form != 0 )
		{
			maxXPGain = form->getXPGainOnCreature();
			faction = form->getFaction();
			creatureFameByKillValid = form->getFameByKillValid();
			creatureFameByKill = form->getFameByKill();
		}
		else
		{
			nlwarning("<CCharacterActions::dispatchXpGain> Cannot find form for creature %s", creature->getId().toString().c_str());
			return false;
		}
	}
	else
	{
		nlwarning("<CCharacterActions::dispatchXpGain> Cannot find creature (row id %s)", creatureRowId.toString().c_str());
		return false;
	}

	// just a check to track a possible bug
	if (float(xpFactor * maxXPGain / equivalentXpMembers) > 150)
	{
		nlwarning("Too much Xp ! when killing creature %s, XPGain = %f, max Xp Gain is %f, xpFactor (for delta level) is %f and equivalent xp members is %f",
			creature->getId().toString().c_str(), float(xpFactor * maxXPGain / equivalentXpMembers), maxXPGain, xpFactor, equivalentXpMembers
			);
	}

	// compute xp gain on creature, cap xp gain per player to MaxXPGainPerPlayer
	const float xpGainPerOpponent = min( MaxXPGainPerPlayer.get(), float(xpFactor * maxXPGain / equivalentXpMembers) );

	c->mobKill(creatureRowId);

	TSkillProgressPerOpponentContainer::iterator it = _SkillProgressPerOpponent.find( creatureRowId );
	if( it != _SkillProgressPerOpponent.end() )
	{
		bool gain = (*it).second->applyXp( c, xpGainPerOpponent);
		if( gain )
		{
			// apply fame lost on offensive char
			if (faction != CStaticFames::INVALID_FACTION_INDEX)
			{
				// retreive the dataset row in the fame dataset
				CEntityId eid = TheDataset.getEntityId(actor);
				if (creatureFameByKillValid)
					CFameInterface::getInstance().addFameIndexed(eid, faction, creatureFameByKill, true);
				else
					CFameInterface::getInstance().addFameIndexed(eid, faction, FameByKill, true);

				// We don't inform the client right now, the timer will take care of this
				//character->sendEventForMissionAvailabilityCheck();
			}
		}
		delete (*it).second;
		_SkillProgressPerOpponent.erase( it );
	}
	return (_SkillProgressPerOpponent.begin() != _SkillProgressPerOpponent.end());
}


//----------------------------------------------------------------------------
// forgetXpGain: forget xp gain of one creature
//----------------------------------------------------------------------------
bool CCharacterActions::forgetXpGain( TDataSetRow creature )
{
	TSkillProgressPerOpponentContainer::iterator it = _SkillProgressPerOpponent.find( creature );
	if( it != _SkillProgressPerOpponent.end() )
	{
		delete (*it).second;
		_SkillProgressPerOpponent.erase( it );
	}
	return (_SkillProgressPerOpponent.begin() != _SkillProgressPerOpponent.end());
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------------
// incNbAction: inc action counter for given skill
//----------------------------------------------------------------------------
void CSkillProgress::incNbAction( SKILLS::ESkills skill )
{
	const uint size = (uint)_SkillsProgress.size();

	for( uint i = 0; i < size; ++i )
	{
		if( _SkillsProgress[ i ].Skill == skill )
		{
			++_SkillsProgress[ i ].NbActions;
			return;
		}
	}

	TSkillProgress sp;
	sp.Skill = skill;
	sp.NbActions = 1;
	_SkillsProgress.push_back( sp );
}


//----------------------------------------------------------------------------
// applyXp: add xp to skill
//----------------------------------------------------------------------------
bool CSkillProgress::applyXp( CCharacter * c, float xpGainPerOpponent )
{
	TLogContext_Character_SkillProgress logContext(c->getId());


	// compute total number of action  (BUT skip all 'unknown' skills actions)
	const uint size = (uint)_SkillsProgress.size();
	double totalNbActions = 0;
	for( uint i = 0; i < size; ++i )
	{
		if (_SkillsProgress[i].Skill != SKILLS::unknown)
			totalNbActions += _SkillsProgress[i].NbActions;
	}


	// We use a map to store total gain by skill and a set to store skills with no gain actions
	// The goal is to send a unique xp info message per skill
	set<SKILLS::ESkills> noGain;
	string skillStr;
	bool gain = false;
	sint lastNonNullXPGainIndex = -1;
	vector<double> xpGainVect;
	xpGainVect.resize(size);

	// in the first loop we compute all xp gains and get the index which will tell us when to start sending xp info messages
	for( uint i = 0; i < size; ++i )
	{
		// skip unknown skill actions (enchantment uses)
		if (_SkillsProgress[i].Skill == SKILLS::unknown)
		{
			xpGainVect[i] = 0.0;
			continue;
		}

		// compute part of xp gain to apply on this skill
		double xpGain = _SkillsProgress[ i ].NbActions / totalNbActions * xpGainPerOpponent;

		xpGainVect[i] = xpGain;

		if( xpGain > 0.0 )
		{
			// this index is used to know when we can flush the send xp message buffer
			lastNonNullXPGainIndex = i;
			gain = true;
		}
		else
		{
			noGain.insert( _SkillsProgress[ i ].Skill );
		}
	}
	// here we add xp
	if( lastNonNullXPGainIndex > -1 )
	{
		map<SKILLS::ESkills,CXpProgressInfos> gainBySkill;

		for( uint i = 0; i < size; ++i )
		{
			if( xpGainVect[i] > 0.0 )
			{
				c->addXpToSkillAndBuffer( xpGainVect[i], SKILLS::toString( _SkillsProgress[ i ].Skill ), gainBySkill);
				// log XP gain
				if (PlayerManager.logXPGain( c->getEntityRowId() ) )
				{
					nlinfo("[XPLOG] Player %s gains %f XP in skill %s", c->getId().toString().c_str(),
						xpGainVect[i], SKILLS::toString(_SkillsProgress[ i ].Skill).c_str() );
				}
			}
		}

		// send skill progression messages to client
		map<SKILLS::ESkills,CXpProgressInfos>::iterator itGainSkill;
		uint32 catalyserCount = 0;
		uint32 catalyserLvl = 0;
		uint32 ringCatalyserCount = 0;
		uint32 ringCatalyserLvl = 0;
		for( itGainSkill = gainBySkill.begin(); itGainSkill != gainBySkill.end(); ++itGainSkill )
		{
			if( (*itGainSkill).second.XpBonus > 0 || (*itGainSkill).second.RingXpBonus > 0 )
			{
				SM_STATIC_PARAMS_3(params, STRING_MANAGER::skill, STRING_MANAGER::integer, STRING_MANAGER::integer);
				params[0].Enum = (*itGainSkill).first;
				params[1].Int = max((sint32)1, sint32(100*(*itGainSkill).second.TotalXpGain) );
				params[2].Int = max((sint32)1, sint32(100*((*itGainSkill).second.TotalXpGain - ((*itGainSkill).second.XpBonus+(*itGainSkill).second.RingXpBonus))) );
				PHRASE_UTILITIES::sendDynamicSystemMessage(c->getEntityRowId(), "XP_CATALYSER_PROGRESS_NORMAL_GAIN", params);

				if( (*itGainSkill).second.XpBonus > 0 )
				{
					catalyserCount += (*itGainSkill).second.CatalyserCount;
					catalyserLvl = (*itGainSkill).second.CatalyserLvl;
				}

				if( (*itGainSkill).second.RingXpBonus > 0 )
				{
					ringCatalyserCount += (*itGainSkill).second.RingCatalyserCount;
					ringCatalyserLvl = (*itGainSkill).second.RingCatalyserLvl;
				}
			}
			else
			{
				SM_STATIC_PARAMS_2(params, STRING_MANAGER::skill, STRING_MANAGER::integer);
				params[0].Enum = (*itGainSkill).first;
				params[1].Int = max((sint32)1, sint32(100*(*itGainSkill).second.TotalXpGain) );
				PHRASE_UTILITIES::sendDynamicSystemMessage(c->getEntityRowId(), "PROGRESS_NORMAL_GAIN", params);
			}
		}
		if( catalyserCount > 0 )
		{
			SM_STATIC_PARAMS_2(params, STRING_MANAGER::integer, STRING_MANAGER::integer);
			params[0].Int = catalyserCount;
			params[1].Int = catalyserLvl;
			PHRASE_UTILITIES::sendDynamicSystemMessage(c->getEntityRowId(), "XP_CATALYSER_CONSUME", params);
		}
		if( ringCatalyserCount > 0 )
		{
			SM_STATIC_PARAMS_2(params, STRING_MANAGER::integer, STRING_MANAGER::integer);
			params[0].Int = ringCatalyserCount;
			params[1].Int = ringCatalyserLvl;
			PHRASE_UTILITIES::sendDynamicSystemMessage(c->getEntityRowId(), "XP_CATALYSER_CONSUME", params);
		}
		gainBySkill.clear();
	}

	// If no gain, indicate it
	set<SKILLS::ESkills>::iterator itNoGain;
	for( itNoGain = noGain.begin(); itNoGain != noGain.end(); ++itNoGain )
	{

		if ( (*itNoGain) != (uint32)(SKILLS::unknown) && !IsRingShard)
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::skill);
			params[0].Enum = (*itNoGain);

			PHRASE_UTILITIES::sendDynamicSystemMessage( c->getEntityRowId(), "PROGRESS_NO_GAIN", params);
		}
	}

	return gain;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------------
// storeAgresssor: store creature aggressor
//----------------------------------------------------------------------------
/*void CCreatureOpponent::storeAggressor( TDataSetRow Aggressor, uint32 dmg )
{
	const uint size = _Adversary.size();
	for( uint i = 0; i < size; ++i )
	{
		if( _Adversary[ i ].entityRowId() == Aggressor )
		{
			_Adversary[ i ].damageMade( _Adversary[ i ].damageMade() + dmg );
			return;
		}
	}

	CAdversary ca;
	ca.entityRowId( Aggressor );
	ca.damageMade( dmg );
	_Adversary.push_back( ca );
}


//----------------------------------------------------------------------------
// despawn: creature despawn
//----------------------------------------------------------------------------
void CCreatureOpponent::despawn()
{
	const uint size = _Adversary.size();
	for( uint i = 0; i < size; ++i )
	{
		CCharacterProgressionPVE::getInstance()->forgetXpGain( _TargetRowId, _Adversary[ i ].entityRowId() );
	}
}


//----------------------------------------------------------------------------
// death: creature is dead
//----------------------------------------------------------------------------
void CCreatureOpponent::death()
{
	CCharacterProgressionPVE::getInstance()->creatureDeath( _TargetRowId );
}


//----------------------------------------------------------------------------
// aggroLost: creature lost agro
//----------------------------------------------------------------------------
void CCreatureOpponent::aggroLost( const TDataSetRow& pcLostAgro )
{
	// only forget the Xp relative to this creature
	for( std::vector< CAdversary >::iterator it = _Adversary.begin(); it != _Adversary.end(); ++it )
	{
		if( (*it).entityRowId() == pcLostAgro )
		{
			_Adversary.erase( it );
			break;
		}
	}
}*/


//----------------------------------------------------------------------------
// CCreatureTakenDamage::attributeKillsForMission
//----------------------------------------------------------------------------
void CCreatureTakenDamage::attributeKillsForMission(TDataSetRow victimRowId)
{
	// compute the minimum damade to do to hava a kill attributed
	float minDamage = TotalCreatureInflictedDamage;
	const uint size = (uint)PlayerInflictedDamage.size();
	for ( uint i = 0; i < PlayerInflictedDamage.size(); i++ )
		minDamage += PlayerInflictedDamage[i].TotalDamage;
	minDamage = KillAttribMinFactor * minDamage;

	for (uint i = 0 ; i < size ; ++i)
	{
		// if total damage is sufficient attribute kill
		if ( PlayerInflictedDamage[i].TotalDamage >= minDamage )
		{
			if (PlayerInflictedDamage[i].TeamId != CTEAM::InvalidTeamId)
			{
				attributeKill(PlayerInflictedDamage[i].TeamId, victimRowId);
			}
			else
			{
				const TDataSetRow rowId = TheDataset.getDataSetRow(PlayerInflictedDamage[i].PlayerId);
				attributeKill(rowId, victimRowId);
			}
		}
	}
}

//----------------------------------------------------------------------------
// CCreatureTakenDamage::attributeKill
//----------------------------------------------------------------------------
void CCreatureTakenDamage::attributeKill( TDataSetRow killerRowId, TDataSetRow victimRowId)
{
	CCharacter *killer = PlayerManager.getChar(killerRowId);
	if (!killer)
	{
		return;
	}

	CCreature *killedCreature = CreatureManager.getCreature(victimRowId);
	if (!killedCreature)
	{
		nlwarning("Cannot find creature from row id %s", victimRowId.toString().c_str());
		return;
	}

	CNPCGroup * group = CreatureManager.getNPCGroup( killedCreature->getAIGroupAlias() );

	CMissionEventKill event(victimRowId,CMissionEvent::NoGroup);
	killer->processMissionEvent( event );

	if ( group )
	{
		if ( std::find( group->PlayerKillers.begin(),group->PlayerKillers.end(),killerRowId) == group->PlayerKillers.end() )
			group->PlayerKillers.push_back(killerRowId);
	}
}

//----------------------------------------------------------------------------
// CCreatureTakenDamage::transferDamageOnFictitiousCreature
//----------------------------------------------------------------------------
bool CCreatureTakenDamage::transferDamageOnFictitiousCreature(const CEntityId &playerId, uint16 teamId)
{
	bool retValue = false;
	bool found = false;

	const uint size = (uint)PlayerInflictedDamage.size();
	for ( uint i = 0 ; i < size ; ++i)
	{
		float damage;
		if ( PlayerInflictedDamage[i].PlayerId == playerId )
		{
			damage = PlayerInflictedDamage[i].TotalDamage;
			PlayerInflictedDamage[i] = PlayerInflictedDamage.back();
			PlayerInflictedDamage.pop_back();
			retValue = true;
			found = true;
		}
		else if ( teamId != CTEAM::InvalidTeamId && PlayerInflictedDamage[i].TeamId == teamId)
		{
			if (PlayerInflictedDamage[i].TeamMembers.empty())
				return false;

			found = true;

			damage = PlayerInflictedDamage[i].TotalDamage / PlayerInflictedDamage[i].TeamMembers.size();
			PlayerInflictedDamage[i].TotalDamage -= damage;

			PlayerInflictedDamage[i].removeMember(playerId);
			if (PlayerInflictedDamage[i].TeamMembers.empty())
			{
				PlayerInflictedDamage[i] = PlayerInflictedDamage.back();
				PlayerInflictedDamage.pop_back();
				retValue = true;
			}
		}

		if (found)
		{
			TotalCreatureInflictedDamage += damage;

			const uint nbC = (uint)CreatureInflictedDamage.size();
			for ( uint j = 0 ; j < nbC ; ++j)
			{
				if (CreatureInflictedDamage[j].CreatureId == CEntityId::Unknown)
				{
					CreatureInflictedDamage[j].TotalDamage += damage;
					return retValue;
				}
			}

			CCreatureInflictedDamage dam;
			dam.TotalDamage = damage;
			dam.CreatureId = CEntityId::Unknown;

			CreatureInflictedDamage.push_back(dam);

			return retValue;
		}
	}

	return false;
}


//----------------------------------------------------------------------------
// CCreatureTakenDamage::attributeKill
//----------------------------------------------------------------------------
void CCreatureTakenDamage::attributeKill( uint16 teamId, TDataSetRow victimRowId)
{
	CCreature *killedCreature = CreatureManager.getCreature(victimRowId);
	if (!killedCreature)
	{
		nlwarning("Cannot find creature from reow id %s", victimRowId.toString().c_str());
		return;
	}

	CTeam * team = TeamManager.getRealTeam( teamId);
	if ( !team )
	{
		nlwarning("Cannot find real team %u, error", teamId);
		return;
	}

	CNPCGroup * group = CreatureManager.getNPCGroup( killedCreature->getAIGroupAlias() );

	CCharacter * user = PlayerManager.getChar( team->getLeader() );

	if ( user && user->getEnterFlag())
	{
		CMissionEventKill event( victimRowId, CMissionEvent::NoSolo);
		// try to attribute the kill to the team only, so only test group / guild mission
		if ( user->processMissionEvent( event ) == false )
		{
			// no group mission processed the kill event. Send it to all team members until one of them has something to do with the event
			// the order the members are chosen must be random
			CMissionEventKill event2( victimRowId, CMissionEvent::NoGroup);
			vector<CEntityId> members;
			for ( list<CEntityId>::const_iterator it = team->getTeamMembers().begin(); it != team->getTeamMembers().end(); ++it )
				members.push_back(*it);

			while ( !members.empty() )
			{
				uint idx = RandomGenerator.rand( (uint16)members.size() - 1 );
				CCharacter * c = PlayerManager.getChar( members[idx] );
				if ( c  && c->getEnterFlag() )
				{
					CMissionEventKill event2( victimRowId, CMissionEvent::NoGroup);
					if ( c->processMissionEvent( event2 ) )
						break;
				}
				members[idx] = members.back();
				members.pop_back();
			}
		}
		if ( group )
		{
			if ( std::find( group->TeamKillers.begin(),group->TeamKillers.end(), teamId) == group->TeamKillers.end() )
				group->TeamKillers.push_back(teamId);
		}

	}
}


} // namespace PROGRESSIONPVE

