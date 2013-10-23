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



#ifndef ENTITY_CALLBACKS_H
#define ENTITY_CALLBACKS_H

// net
#include "nel/net/message.h"

class CPlayer;


// Utility functions for player manager (OfflineCommands)
bool			getIdFromOfflineCommandsFile(const std::string& filename, uint32& userId, uint32& charId);
struct			CBackupMsgAppendCallback;
void			cbReceivedOfflineCmds(CBackupMsgAppendCallback& append);

///////////////////////////////////////////////////////////////////////////////
/////////////////////// Callback for characters messages //////////////////////
///////////////////////////////////////////////////////////////////////////////
// One client is connected
void cbClientConnection( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
// One is disconnected
void cbClientDisconnection( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
void disconnectUser(uint32 userId);
// One client is enter in game
//void cbClientEnter(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
// One Client is ready (Inits finish)
void cbClientReady(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
// finalizeClientReady
void finalizeClientReady( uint32 userId, uint32 index );
// cbSelectChar
void cbSelectChar( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
// cbCheckName
void cbCheckName( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
// cbCreateChar
void cbCreateChar( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
// cbDeleteChar
void cbDeleteChar( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
// Send player summary to client :
void sendPlayerSummary( uint32 userId, NLNET::TServiceId frontEndId );
// Received a position of entity
void cbEntityPos( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

/// callback for UNFREEZE_SENTENCE msg 
//void cbUnfreezeSentence( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId);


/// callback for drop an item
void cbItemDrop( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// callback for pickup an item
void cbItemPickup( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// callback for close pickup inventory
void cbItemClosePickup( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// callback for swap an item
void cbItemSwap( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);


/// callback for LEAVE_TEAM message
void cbLeaveTeam( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// callback for JOIN_TEAM message
void cbJoinTeam( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// callback for JOIN_TEAM_DECLINE message
void cbJoinTeamDecline( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// callback for JOIN_TEAM_PROPOSAL message
void cbJoinTeamProposal( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
// kick your target from your team
void cbKickTeammate( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);


/// callback for JOIN_LEAGUE message
void cbJoinLeague( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// callback for JOIN_LEAGUE_DECLINE message
void cbJoinLeagueDecline( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// callback for JOIN_LEAGUE_PROPOSAL message
void cbJoinLeagueProposal( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);


// toogle god mode on / off for testing
//void cbGodMode( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

/// callback called when receiving harvest results from the BS
void cbHarvestResult( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// callback called when mp were destroyed during the harvest action
void cbHarvestMPDestroyed( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// callback called when harvest has been canceled (ore not mp harvested or destroyed)
void cbHarvestInterrupted( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// the player is harvesting an mp
void cbHarvest( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// the player is harvesting a deposit
void cbHarvestDeposit( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// the player close the harvest interface and thus stop harvesting
void cbHarvestClose( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// receive infos to update the database for the harvest interface
void cbHarvestDB( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// receive infos to update an item quantity in player database (for the harvest interface)
void cbHarvestDBUpdateQty( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// receive clear command for harvest DB
void cbClearHarvestDB( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

/// create item(s) in player bag (harvest result for example)
//void cbCreateItemInBag( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

/// callback for changing fighting target
void cbFightingTarget( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// callback for send command to pack animal
void cbAnimalCommand( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

/// callback for send list traded by shopkeeper
void cbTradeListReceived( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// callback player bye an item in current trade page
void cbTradeBuySomething( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
// callback add seed to player
//void cbGiveSeed( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);


/// invite an character to exchange
void cbExchangeProposal( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// accept exchange invitation
void cbAcceptExchangeInvitation( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// decline exchange invitation
void cbDeclineExchangeInvitation( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// accept exchange
void cbAcceptExchange( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// end exchange
void cbEndExchange( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// seed exchange
void cbExchangeSeeds( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

// Add an survive pact
void cbAddSurvivePact( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

// Client finished loading during a TP
void cbClientTpAck( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

// Change one fame of character
//void cbFameChange( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);


////////////////////////////////////////////////////////////////////
//////////// Callback for both clients and creatures ///////////////
////////////////////////////////////////////////////////////////////
// cbSetValue
void cbSetValue( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
// cbModifyValue
void cbModifyValue( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
// cbChangeMode
void cbChangeMode( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
// cbChangeBehaviour
void cbChangeBehaviour( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
// cbTarget -> entity changed his target
void cbTarget( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

/// engage combat with current target
void cbEngage( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// default attack on current target
void cbDefaultAttack( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// Stun entity
void cbStun( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// wake up entity (after a stun...)
void cbWake( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

// Entity want mounting
void cbAnimalMount( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
// Entity unseat
void cbAnimalUnseat( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

// change weather for a given player (from DSS)
void cbSetPlayerWeather(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

// change season for a given player (from DSS)
void cbSetPlayerSeason(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

////////////////////////////////////////////////////////////////////
////////////////////// Callback for creatures //////////////////////
////////////////////////////////////////////////////////////////////
// Update creature mission list
void cbUpdateMissionList( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);


////////////////////////////////////////////////////////////////////
///////////////// Testing tool callback
////////////////////////////////////////////////////////////////////
/// Testing tool Actor Spawn
void cbTestToolSpawnActor( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// Testing tool engage combat
void cbTestToolEngageCombat( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
/// Testing tool unspawn actor
void cbTestToolUnspawnActor( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);


/// Forage source position validation
void cbRcvValidateSourceSpawnReply( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

/// A creature can't reach a player
void cbPlayerUnreachable( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

// AIS ask to teleport player via script
void cbTeleportPlayer(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

#endif //ENTITY_CALLBACKS_H



