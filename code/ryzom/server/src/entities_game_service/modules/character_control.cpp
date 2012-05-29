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

#include "nel/net/service.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

#include "game_share/r2_modules_itf.h"
#include "server_share/r2_variables.h"
#include "server_share/r2_vision.h"
#include "game_share/r2_types.h"

#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/character_respawn_points.h"
#include "character_control.h"
#include "modules/r2_mission_item.h"
#include "modules/easter_egg.h"
#include "creature_manager/creature_manager.h"
#include "modules/animation_session_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"
	
	
using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace R2;

extern CVariable<string>	AlwaysInvisiblePriv;
extern CVariable<string>	NeverAggroPriv;


class CCharacterControl : public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav <CModuleBase> > >,
	public ICharacterControl,
	public CCharacterControlItfSkel
{
public:
	typedef uint32 TCharId;
//	typedef uint32 TSessionId;

public:

	// Proxy to the module to reach in the DSS
	TModuleProxyPtr			_ServerAnimationProxy;

public:

	CCharacterControl()
	{
		CCharacterControlItfSkel::init(this);
	}

	void onModuleUp(IModuleProxy *module)
	{
		if (module->getModuleClassName() == "ServerAnimationModule")
		{
			WARN_IF(_ServerAnimationProxy != NULL, ("CCharacterControl::onModuleUp : receiving module for class CharacterSynchronisation as '%s', but already have one as '%s', replacing", module->getModuleName().c_str(), _ServerAnimationProxy->getModuleName().c_str()));
			_ServerAnimationProxy = module;
		}
	}

	void onModuleDown(IModuleProxy *module)
	{
		if (module == _ServerAnimationProxy)
		{
			_ServerAnimationProxy = NULL;
		}
	}

//	bool onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
//	{
//		if (CCharacterControlItfSkel::onDispatchMessage(sender, message))
//			return true;
//		
//		nlwarning("CCharacterControl : Unknown message '%s' received", message.getName().c_str());
//
//		return false;
//	}

	/*
	 * Sending messages to the server animation module
	 */

	virtual void requestStartParams( const NLMISC::CEntityId& entityId, TSessionId lastStoredSessionId )
	{
		DROP_IF( !_ServerAnimationProxy, "Server animation module not present", return );

		CServerAnimationItfProxy saip( _ServerAnimationProxy );
		saip.getStartParams( this, (uint32)entityId.getShortId(), lastStoredSessionId );
	}

	/*
	 * Receiving messages from the server animation module
	 */
	// This message is send by dss when player go from test mode to animation mode
	virtual void setUserCharCurrentSession(NLNET::IModuleProxy *sender, uint32 charId, TSessionId oldSessionId, const CFarPosition &respawnPoint, R2::TUserRole role )
	{
		ICharacter * cItf = ICharacter::getInterface( charId, false );
		DROP_IF(!cItf, NLMISC::toString("User ( userId:%u characterId=%u) no more connected", charId >> 4, charId & 0x0f), return);

		cItf->getRespawnPoints().setRingAdventureRespawnpoint( respawnPoint );
		cItf->setSessionUserRole( role );		
		// if the next line is commented it resolved the 14025 but there is another but that randomly appears.
		// (back in the good session) but at the previous session position		
		cItf->leavePreviousSession(oldSessionId);
		if ( role == TUserRole::ur_editor )
		{
			cItf->applyEditorPosition( respawnPoint );
		}
		else
		{
			cItf->applyAndPushNewPosition( respawnPoint );
		}
	}
	
	// The reply of CServerAnimationItf::getStartParams
	virtual void setUserCharStartParams(NLNET::IModuleProxy *sender, uint32 charId, const CFarPosition &farPosConst, bool reloadPos, uint8 scenarioSeason, R2::TUserRole role )
	{
		nlassert( IsRingShard );

		// Find character interface
		ICharacter * cItf = ICharacter::getInterface( charId, false ); // here we are *before* cItf's EnterFlag is true, because we're sending the initial position to the client
		DROP_IF(!cItf, NLMISC::toString("User ( userId:%u characterId=%u) no more connected", charId >> 4, charId & 0x0f), return);

		CFarPosition farPos(farPosConst);
		// Trick for not beeing in 0,0 (while loading screen) 
		// We take the previous Session position (so we just see bot to depop)
		if (farPos.PosState.X == 0 && farPos.PosState.Y == 0)
		{
			CCharacter *character = safe_cast<CCharacter*>(cItf);
			if (!character->PositionStack.empty())
			{
				farPos.PosState.X = character->PositionStack.top().PosState.X;
				farPos.PosState.Y = character->PositionStack.top().PosState.Y;
			}			
		}

		if ( reloadPos )
		{
			if ( role == TUserRole::ur_editor )
				nlwarning( "DSS asks to reload position in editor mode for char %u!", charId );

			// Continue with the stored normal position, but set the respawn point given by the DSS
			cItf->applyTopOfPositionStack();
			cItf->getRespawnPoints().setRingAdventureRespawnpoint( farPos );
		}
		else
		{
			// Set the new position and the respawn point from the start position given by the DSS
			if ( role == TUserRole::ur_editor )
			{
				cItf->applyEditorPosition( farPos );
			}
			else
			{
				cItf->applyAndPushNewPosition( farPos );
			}
			cItf->getRespawnPoints().setRingAdventureRespawnpoint( farPos );
		}

		// Store role
		cItf->setSessionUserRole( role );

		// Send USER_CHAR to client
		cItf->sendUserChar( charId >> 4, scenarioSeason, role );		
	}

	// A character enter an anim session as player
	virtual void charJoinAnimSession(NLNET::IModuleProxy *sender, uint32 charId, uint32 sessionId)
	{
		if (IAnimSessionMgr::isInitialized())
			IAnimSessionMgr::getInstance()->characterEnterAnimSession(sessionId, charId);
	}

	// A character leave an anim session as player
	virtual void charLeaveAnimSession(NLNET::IModuleProxy *sender, uint32 charId, uint32 sessionId)
	{
		if (IAnimSessionMgr::isInitialized())
			IAnimSessionMgr::getInstance()->characterLeaveAnimSession(sessionId, charId);
	}

	// the specified character an to report the RRP
	// earned during the session because the character
	// has leave an animation session.
	// The reply of CServerAnimationItf::startAct telling to teleport user
	virtual void setUserCharActPosition(NLNET::IModuleProxy *sender, uint32 charId, const CFarPosition &farPos, uint8 season)
	{
		nlassert( IsRingShard );

		// Find character
		ICharacter * cItf = ICharacter::getInterface( charId, true );
		DROP_IF(!cItf, NLMISC::toString("User ( userId:%u ) no more connected", charId), return);

		cItf->setCurrentSessionId(farPos.SessionId);

		// update re-spawn point
		if( cItf->getRespawnPoints().setRingAdventureRespawnpoint( farPos ) )
		{
			// teleport un-dead character to a new position or rez dead character
			if(! cItf->isDead())
			{
				CCharacter *character = safe_cast<CCharacter*>(cItf);

				if ( character->sessionUserRole() != TUserRole::ur_editor )
				{
					WARN_IF( farPos.SessionId.asInt() == 0, NLMISC::toString("Setting sessionId 0 in setUserCharActPosition() for %s", cItf->getId().toString().c_str()) );
					cItf->setSessionId( farPos.SessionId );
				}
				cItf->setCurrentSessionId(  farPos.SessionId );
				// Do not tp if X==0 && Y==0 (because special case loading screen.. whe will be tp short after)
				
				if (farPos.PosState.X != 0 && farPos.PosState.Y!=0)
				{
					// Do not tp at all (create double/triple tp) when manually tp or start scenario
					if (0)
					{
						
						character->forbidNearPetTp();
						cItf->teleportCharacter( farPos.PosState.X, farPos.PosState.Y);

						CCharacter *character = safe_cast<CCharacter*>(cItf);
						CEntityState& state = character->getState();
						// 
						if ( state.X() != farPos.PosState.X  
							|| state.Y() != farPos.PosState.Y )
						{						
							character->teleportCharacter( farPos.PosState.X, farPos.PosState.Y, 0, true, true, state.Heading(),  0xFF, 0, season);
						}
					}					
				}
				
			}
			else
			{
				cItf->respawn(0);
			}
			CR2EasterEgg::getInstance().easterEggTPActChange(cItf->getId(), farPos);
		}
	}

	// A DSS to EGS signal that an anim session is started
	virtual void animSessionStarted(NLNET::IModuleProxy *sender, TSessionId sessionId, const TRunningScenarioInfo &scenarioInfo)
	{
		if (IAnimSessionMgr::isInitialized())
			IAnimSessionMgr::getInstance()->animSessionStarted(sessionId, scenarioInfo);
	}

		// A DSS to EGS signal that an anim session is ended
	virtual void animSessionEnded(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 scenarioScore, NLMISC::TTime timeTaken)
	{
		if (IAnimSessionMgr::isInitialized())
			IAnimSessionMgr::getInstance()->animSessionEnded(sessionId, scenarioScore, timeTaken);
	}
	
	// After setUserCharActPosition call, we ask for position again for insure position are no been changed during teleportation time
	virtual void requestEntryPoint( const NLMISC::CEntityId& entityId )
	{
		DROP_IF( !_ServerAnimationProxy, "Server animation module not present", return );
		nlassert( IsRingShard );

		CServerAnimationItfProxy saip( _ServerAnimationProxy );
		saip.askSetUserCharActPosition( this, (uint32)entityId.getShortId() );
	}

	// Send item definition to EGS for a scenario
	virtual void sendItemDescription( TSessionId scenarioId, const std::vector<R2::TMissionItem> &missionItem )
	{
		CR2MissionItem::getInstance().itemsDescriptionsForScenario( scenarioId, missionItem );
	}

	// From dss
	virtual void sendItemDescription( NLNET::IModuleProxy *sender, TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItems)		
	{
		TSessionId scenarioId=getSessionId(sessionId);
		sendItemDescription(scenarioId, missionItems);
	}




	// Say to EGS a scenario are ended
	virtual void scenarioEnded( TSessionId sessionId )
	{
		TSessionId scenarioId=getSessionId(sessionId);
		CR2MissionItem::getInstance().endScenario( scenarioId );
		CR2EasterEgg::getInstance().endScenario( scenarioId );

		if( IsRingShard )
		{
			// resurrect dead players  //**** TODO **** : optimize this(need the list of players in scenario) !!!!!
			for (CPlayerManager::TMapPlayers::const_iterator it = PlayerManager.getPlayers().begin(); it != PlayerManager.getPlayers().end(); ++it)
			{
				if ((*it).second.Player != 0)
				{
					CCharacter *c = (*it).second.Player->getActiveCharacter();
					if (c != 0)
					{
						if( c->getInstanceNumber() == sessionId.asInt()  || c->getInstanceNumber() == scenarioId.asInt() )
						{
							if( c->isDead() )
							{
								c->revive();
							}
						}
					}
				}
			}
		}			
	}
	
	virtual void scenarioEnded( NLNET::IModuleProxy *sender,  TSessionId sessionId)
	{
		TSessionId scenarioId=getSessionId(sessionId);
		scenarioEnded(scenarioId);
	}

	virtual void stealMissionItem( const NLMISC::CEntityId &eid, const std::vector<R2::TItemAndQuantity> &items )
	{
		CR2MissionItem::getInstance().destroyMissionItem(eid, items);
	}
	
	virtual void getMissionItemOwnedByCharacter(const NLMISC::CEntityId & eid)
	{
		std::vector<R2::TItemAndQuantity> items;
		CR2MissionItem::getInstance().getMissionItemOwnedByCharacter(eid, items);

		// TODO: send result message to DSS
	}

	// activate a scenario generated easter egg
	virtual void activateEasterEgg(uint32 easterEggId, TSessionId scenarioId, uint32 aiInstanceId, const std::vector< R2::TItemAndQuantity > &items, const CFarPosition &pos, const std::string& name, const std::string&look)
	{
		CR2EasterEgg::getInstance().activateEasterEgg(easterEggId, scenarioId, aiInstanceId, items, pos, name, look);
	}
	
	// deactivate a scenario generated easter egg
	virtual void deactivateEasterEgg(uint32 easterEggId, TSessionId scenarioId)
	{
		CR2EasterEgg::getInstance().deactivateEasterEgg(easterEggId, scenarioId);
	}

	virtual void activateEasterEgg( NLNET::IModuleProxy *sender, uint32 easterEggId, TSessionId sessionId, uint32 aiInstanceId, const std::vector< R2::TItemAndQuantity > &items, const CFarPosition &pos, const std::string& name, const std::string &look)
	{
		TSessionId scenarioId=getSessionId(sessionId);
		activateEasterEgg(easterEggId, scenarioId, aiInstanceId, items, pos, name, look);
	}		
	
	virtual void deactivateEasterEgg( NLNET::IModuleProxy *sender, uint32 easterEggId, TSessionId sessionId)
	{	
		TSessionId scenarioId=getSessionId(sessionId);
		deactivateEasterEgg(easterEggId, scenarioId);
	}

	virtual void deactivateEasterEggs( NLNET::IModuleProxy *sender, const std::set<uint32> & easterEggIds, TSessionId sessionId)
	{	
		TSessionId scenarioId=getSessionId(sessionId);
		std::set<uint32>::const_iterator first(easterEggIds.begin()), last(easterEggIds.end());
		for ( ; first != last ; ++first)
		{
			deactivateEasterEgg(*first, scenarioId);
		}
		
	}


	virtual void getEasterEggDropped(TSessionId scenarioId, std::vector<R2::TEasterEggInfo> &easterEgg )
	{
		CR2EasterEgg::getInstance().getEasterEggForScenario( scenarioId, easterEgg );
	}

	virtual void lootEasterEggEvent( uint32 externalEasterEggId, TSessionId scenarioId )
	{
		DROP_IF( !_ServerAnimationProxy, "Server animation module not present", return );		
		CServerAnimationItfProxy saip( _ServerAnimationProxy );
		saip.onEasterEggLooted( this, externalEasterEggId, scenarioId );
	}
	
	virtual void giveRewardMessage(NLNET::IModuleProxy *sender, TDataSetRow characterRowId, TDataSetRow creatureRowId,
								const std::string& rewardText,
								const std::string& rareRewardText,
								const std::string& inventoryFullText,
								const std::string& notEnoughPointsText)
	{
		// retrieve the character that is rewarded
		CEntityId charEId = TheDataset.getEntityId(characterRowId);
		if (charEId == CEntityId::Unknown)
		{
			nlinfo("giveRewardMessage : receive a reward message for character row %s, but can't find the corresponding character", characterRowId.toString().c_str());
			return;
		}
		ICharacter *ic = ICharacter::getInterface(charEId, true);

		const R2::TRunningScenarioInfo *scenario = NULL;
		// get the scenario info
		if (IAnimSessionMgr::isInitialized())
		{
			const TAnimSessionInfo* animSessionInfo= IAnimSessionMgr::getInstance()->getAnimSessionForChar(uint32(charEId.getShortId()));
			DROP_IF(animSessionInfo == NULL, "giveRewardMessage : receive a reward message for character row "<<characterRowId.toString()<<", but can't find the SESSION info", return);
			scenario= &animSessionInfo->ScenarioInfo;
		}
		CRingRewardPoints &rrp = ic->getRingRewardPoints();

		SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
		CRingRewardPoints::TGenerateRewardResult result = rrp.generateReward(scenario->getSessionLevel());
		switch(result)
		{
		case CRingRewardPoints::grr_ok:
			params[0].Literal = rewardText;
			break;
		case CRingRewardPoints::grr_ok_rare:
			params[0].Literal = rareRewardText;
			break;
		case CRingRewardPoints::grr_no_place:
			params[0].Literal = inventoryFullText;
			break;
		case CRingRewardPoints::grr_no_points:
			params[0].Literal = notEnoughPointsText;
			break;
		case CRingRewardPoints::grr_invalid:
		default:
			params[0].Literal = "";
		}
		if (!params[0].Literal.empty())
			PHRASE_UTILITIES::sendDynamicSystemMessage( creatureRowId, "LITERAL", params );
	}

	/*
	Call by the dss when he needs to know information about the target of a player

	the dss will performs operation depending on the target of the player (indicates to the player if the element can be targeted,
	use dm command like kill)
	*/
	virtual void sendCharTargetToDss( NLNET::IModuleProxy *sender, const NLMISC::CEntityId & eid, const std::vector<std::string>& params)
	{		

		DROP_IF( !_ServerAnimationProxy, "Server animation module not present", return );
		CServerAnimationItfProxy saip( _ServerAnimationProxy );
		bool alive = false;
		CCharacter *c = PlayerManager.getChar(eid); 

		if (!c)
		{
			return;
		}
		
		TDataSetRow entityRowId = c->getTargetDataSetRow();		
		const NLMISC::CEntityId&  creatureId = TheDataset.getEntityId( entityRowId );
		TAIAlias alias = CAIAliasTranslator::getInstance()->getAIAlias(creatureId);
		uint32 nameId =  CEntityIdTranslator::getInstance()->getEntityNameStringId(creatureId);
		const ucstring & ucName =	CEntityIdTranslator::getInstance()->getByEntity(creatureId);
		CCreature* creature = CreatureManager.getCreature(entityRowId);
		alive = creature && creature->getMode() != MBEHAV::DEATH;

		if ( !alive ||
			creatureId == NLMISC::CEntityId::Unknown ||
			alias == CAIAliasTranslator::Invalid ||
			entityRowId == TDataSetRow()
			)
		{			
			saip.onCharTargetReceived( this, eid, NLMISC::CEntityId::Unknown, CAIAliasTranslator::Invalid,TDataSetRow(), ucstring(), 0, params, alive);;
			return;
		}

		
		saip.onCharTargetReceived( this, eid, creatureId, alias, entityRowId, ucName, nameId, params, alive);
	
	}


	virtual void onTpPositionAsked( NLNET::IModuleProxy *sender, const NLMISC::CEntityId & eid, float x, float y, float z, uint8 season, const R2::TR2TpInfos& tpInfos)
	{
		CCharacter *c = PlayerManager.getChar(eid); 
		if (!c) { return; }
		c->forbidNearPetTp();
		
	// In a ring shard we refuse to tp a player that is the 3 m aroud its previous position (with the same season)
		sint32 dx = sint32(x - c->getState().X());
		sint32 dy = sint32(y - c->getState().Y());
		if (dx < 0) { dx = -dx;}
		if (dy < 0 ) { dy = -dy;}
		if ( dx < 5*1000 && dy < 5*1000 && season == c->getRingSeason())
		{
			return;
		}

		c->teleportCharacter(sint32(1000.0f*x), sint32(1000.0f*y), sint32(1000.0f*z), true, false, 0.f, 0xFF, 0, season, tpInfos);
	}

	virtual void disconnectChar(NLNET::IModuleProxy *sender, uint32 charId)
	{
		uint32 playerId = charId >> 4;

		CPlayer* player = PlayerManager.getPlayer( playerId);
		if (player)
		{
			//PlayerManager.addPlayerMustBeDisconnected( playerId);
			uint32 userId = player->getUserId(); 
		
			// ask to front-end for disconnect client
			NLNET::CMessage msgOut("DISCONNECT_CLIENT");
		
			msgOut.serial( userId );
			NLNET::TServiceId playerFrontEndId = PlayerManager.getPlayerFrontEndId(userId);
			CUnifiedNetwork::getInstance()->send( playerFrontEndId, msgOut );

		}
		
	}
	
	virtual void returnToPreviousSession(NLNET::IModuleProxy *sender, uint32 charId)
	{
		ICharacter * cItf = ICharacter::getInterface( charId , true);
		DROP_IF(!cItf, NLMISC::toString("User ( userId:%u characterId=%u) no more connected", charId >> 4, charId & 0x0f), return);
		cItf->returnToPreviousSession();
	}
	

	virtual void  setPioneerRight(NLNET::IModuleProxy *sender, uint32 charId, bool isDM)
	{
		nlinfo("Setting pioneer rights for character %u %s %s",charId,CEntityId((uint8)RYZOMID::player,charId).toString().c_str(),isDM?"(DM)":"");

		ICharacter * cItf = ICharacter::getInterface( charId, true );
		DROP_IF(!cItf, NLMISC::toString("User ( userId:%u characterId=%u) no more connected", charId >> 4, charId & 0x0f), return);

		CCharacter *character = safe_cast<CCharacter*>(cItf);
		uint32 whoSeesMeVisionLevel= R2_VISION::extractVisionLevel(character->getWhoSeesMe())<<R2_VISION::NUM_WHOSEESME_BITS;
		uint64 NOT_TELEPORTING_FLAG= (((uint64)0x12345678)<<32)| (uint64)0x87654321;
		if (isDM)
		{
			character->setAggroableOverride(0);
			// are we teleporting ?
			if (character->whoSeesMeBeforeTP() == NOT_TELEPORTING_FLAG)
			{
				// we're NOT teleportting so change current who sees me value
				character->setWhoSeesMe( R2_VISION::buildWhoSeesMe(R2_VISION::TInvisibilityLevel(whoSeesMeVisionLevel|R2_VISION::INVISIBLE_DM), false) );
			}
			else
			{
				// we ARE teleportting so clear out set who sees me before TP and reinitialise to the correct value
				character->setWhoSeesMeBeforeTP(NOT_TELEPORTING_FLAG);
				character->setWhoSeesMeBeforeTP(R2_VISION::buildWhoSeesMe(R2_VISION::TInvisibilityLevel(whoSeesMeVisionLevel|R2_VISION::INVISIBLE_DM), false) );
			}
			
			character->setGodMode(1);
		}
		else
		{
			CPlayer *p = PlayerManager.getPlayer(uint32(character->getId().getShortId())>>4);
			if (p && p->havePriv(":SGM:GM:VG:SG:G:"))
			{
				// do not override state with those from the ring
			}
			else
			{
				character->setAggroableOverride(-1);
				
				if (character->whoSeesMeBeforeTP() == NOT_TELEPORTING_FLAG)
				{
					character->setWhoSeesMe( R2_VISION::buildWhoSeesMe(R2_VISION::TInvisibilityLevel(whoSeesMeVisionLevel|R2_VISION::VISIBLE), true) );
				}
				else
				{
					character->setWhoSeesMeBeforeTP(NOT_TELEPORTING_FLAG);
					character->setWhoSeesMeBeforeTP(R2_VISION::buildWhoSeesMe(R2_VISION::TInvisibilityLevel(whoSeesMeVisionLevel|R2_VISION::VISIBLE), true) );
				}
				
				character->setGodMode(0);
			}
		}
	}

	virtual void teleportOneCharacterToAnother(NLNET::IModuleProxy *sender, TCharId sourceCharId, TCharId destCharId, uint8 season )
	{
		ICharacter * cItf1 = ICharacter::getInterface( sourceCharId, true );
		DROP_IF(!cItf1, NLMISC::toString("User ( userId:%u characterId=%u) no more connected", sourceCharId >> 4, sourceCharId & 0x0f), return);
		
		ICharacter * cItf2 = ICharacter::getInterface( destCharId, true );
		DROP_IF(!cItf2, NLMISC::toString("User ( userId:%u characterId=%u) no more connected", destCharId >> 4, destCharId & 0x0f), return);
		CCharacter *character1 = safe_cast<CCharacter*>(cItf1);
		CCharacter *character2 = safe_cast<CCharacter*>(cItf2);

		CEntityState& state = character2->getState();

		character1->teleportCharacter( state.X(), state.Y(), state.Z(), true, true, state.Heading(),  0xFF, 0, season);

	}

	virtual void teleportFarCharacter(NLNET::IModuleProxy *sender, const NLMISC::CEntityId & playerId, uint32 sessionId)
	{
		TCharId sourceCharId = static_cast<TCharId>(playerId.getShortId());

		ICharacter * cItf1 = ICharacter::getInterface( sourceCharId, true );
		DROP_IF(!cItf1, NLMISC::toString("User ( userId:%u characterId=%u) no more connected", sourceCharId >> 4, sourceCharId & 0x0f), return);
		CCharacter *c = safe_cast<CCharacter*>(cItf1);
		c->PositionStack.topToModify().SessionId = (TSessionId)(sessionId);		
		// Lock the stack to save it in this state, and make the client Far TP 
		c->setSessionId( SessionLockPositionStack );
		c->requestFarTP( c->PositionStack.topToModify().SessionId, true );

	}
	

	virtual void teleportCharacterToNpc(NLNET::IModuleProxy *sender, TCharId sourceCharId, const NLMISC::CEntityId & destId, uint8 season )
	{
		ICharacter * cItf1 = ICharacter::getInterface( sourceCharId, true );
		DROP_IF(!cItf1, NLMISC::toString("User ( userId:%u characterId=%u) no more connected", sourceCharId >> 4, sourceCharId & 0x0f), return);		
		CCharacter *character1 = safe_cast<CCharacter*>(cItf1);

		if( !character1 || !TheDataset.isAccessible(character1->getEntityRowId()))
		{	
			return;
		}

		CCreature * creature = CreatureManager.getCreature( destId );
		if( !creature || !TheDataset.isAccessible(creature->getEntityRowId()))
		{			
			return;
		}
		
			
		CEntityState& state = creature->getState();

		character1->teleportCharacter( state.X(), state.Y(), state.Z(), true, true, state.Heading(),  0xFF, 0, season);

	}

	virtual void reportLinkedSession(NLNET::IModuleProxy *sender, TSessionId editSessionId, TSessionId animSessionId)
	{
		_LinkedSessions[animSessionId] = editSessionId;
	}

	virtual void reportUnlinkedSession(NLNET::IModuleProxy *sender, TSessionId editSessionId, TSessionId animSessionId)
	{
		std::map<TSessionId, TSessionId>::iterator toRemove = _LinkedSessions.find(animSessionId);
		if (toRemove != _LinkedSessions.end())
		{
			_LinkedSessions.erase(toRemove);
		}
	}

	TSessionId getSessionId(TSessionId sessionId) const
	{
		std::map<TSessionId, TSessionId>::const_iterator found = _LinkedSessions.find(sessionId);
		if (found == _LinkedSessions.end()) { return sessionId; }
		return found->second;
	}

	void characterReady(const CEntityId &entityId)
	{
		BOMB_IF(_ServerAnimationProxy == NULL, "CCharacterControl::characterReady : No server animation module available", return);

		CServerAnimationItfProxy sam(_ServerAnimationProxy);
		sam.characterReady(this, entityId);
	}
	
	void reportNpcControl(NLNET::IModuleProxy *sender, const NLMISC::CEntityId& playerEid, const NLMISC::CEntityId&  botEid)
	{
		
		ICharacter * cItf1 = ICharacter::getInterface( playerEid, true );
		DROP_IF(!cItf1, NLMISC::toString("User %s is not connected", playerEid.toString().c_str()), return);		
		CCharacter *character = safe_cast<CCharacter*>(cItf1);
		if (!character)
		{
			return;			
		}
		character->setNpcControl(botEid);
		
	}


	void reportStopNpcControl(NLNET::IModuleProxy *sender, const NLMISC::CEntityId& playerEid, const NLMISC::CEntityId&  botEid)
	{
		
		ICharacter * cItf1 = ICharacter::getInterface( playerEid, true );
		DROP_IF(!cItf1, NLMISC::toString("User %s is not connected", playerEid.toString().c_str()), return);		
		CCharacter *character = safe_cast<CCharacter*>(cItf1);
		if(  !character) 
		{ 	 					
			return;
		}
		character->setStopNpcControl();
	}
		
	// DSS ask to put a character in the ring universe channel
	// This is for editors and animator characters only
	virtual void subscribeCharacterInRingUniverse(NLNET::IModuleProxy *sender, uint32 charId)
	{
		CEntityId eid(RYZOMID::player, charId, 0, 0);
		TDataSetRow dsr = TheDataset.getDataSetRow(eid);
		ICharacter *character = ICharacter::getInterface(eid, true);
		if (character == NULL || !dsr.isValid())
			return;

		DynChatEGS.addSession(CEntityId(RYZOMID::dynChatGroup, RingDynChanOffset+character->getHomeMainlandSessionId()), dsr, true);
	}

	// DSS ask to remove a character from the ring universe channel
	// This is for editors and animator characters only
	virtual void unsubscribeCharacterInRingUniverse(NLNET::IModuleProxy *sender, uint32 charId)
	{
		CEntityId eid(RYZOMID::player, charId, 0, 0);
		TDataSetRow dsr = TheDataset.getDataSetRow(eid);
		ICharacter *character = ICharacter::getInterface(eid, true);
		if (character == NULL || !dsr.isValid())
			return;

		DynChatEGS.removeSession(CEntityId(RYZOMID::dynChatGroup, RingDynChanOffset+character->getHomeMainlandSessionId()), dsr);
	}


	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CCharacterControl, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CCharacterControl, disconnectChar, "disconnect a Char", "<charId>")	
		NLMISC_COMMAND_HANDLER_ADD(CCharacterControl, returnToPreviousSession, "return a player to Previous Session", "<charId>")	
		NLMISC_COMMAND_HANDLER_ADD(CCharacterControl, setPioneerRight, "setPioneerRight", "<charId> <isDm>")	
		
	NLMISC_COMMAND_HANDLER_TABLE_END

	// Debugs functions for displaying infos on sessions
	NLMISC_CLASS_COMMAND_DECL(disconnectChar);
	NLMISC_CLASS_COMMAND_DECL(returnToPreviousSession);
	NLMISC_CLASS_COMMAND_DECL(setPioneerRight);
private:
	std::map<TSessionId, TSessionId> _LinkedSessions; //map AnimationSesison -> EditionSession

};


NLMISC_CLASS_COMMAND_IMPL(CCharacterControl, disconnectChar)
{
	if (args.size() != 1)
	{
		log.displayNL("Wrong usage : CharacterControl.disconnectChar <charId>");
		return false;
	}

	uint32 charId;
	NLMISC::fromString(args[0], charId);
	log.displayNL("Try to disconnect char %u", charId);
	this->disconnectChar(0, charId);
			

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CCharacterControl, returnToPreviousSession)
{
	if (args.size() != 1)
	{
		log.displayNL("Wrong usage : CharacterControl.returnToMainLand <charId>");
		return false;
	}

	uint32 charId;
	NLMISC::fromString(args[0], charId);
	log.displayNL("Try to returnToMainLand char %u", charId);
	this->returnToPreviousSession(0, charId);
			

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CCharacterControl, setPioneerRight)
{
	if (args.size() != 2)
	{
		log.displayNL("Wrong usage : CharacterControl.setPioneerRight <charId> <isDM>");
		return false;
	}

	uint32 charId;
	NLMISC::fromString(args[0], charId);
	bool isDm;
	NLMISC::fromString(args[1], isDm);
	log.displayNL("Try to setPioneerRight char %u", charId);
	this->setPioneerRight(0, charId, isDm);
			

	return true;
}

NLNET_REGISTER_MODULE_FACTORY(CCharacterControl, "CharacterControl");

