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

#include "nel/misc/command.h"
#include "nel/net/unified_network.h"

#include "r2_give_item.h"
#include "r2_mission_item.h"

#include "egs_mirror.h"
#include "egs_sheets/egs_sheets.h"


#include "game_share/bot_chat_types.h"

#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "creature_manager/creature_manager.h"
#include "creature_manager/creature.h"
#include "world_instances.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

uint32	CR2GiveItem::_NextActionId = 0;

// CGiveItemRequestMsg Transport class callback implementation
void CGiveItemRequestMsgImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CR2GiveItem::getInstance().giveItemRequest( *this );
}

// CReceiveItemRequestMsg transport class callback implementation
void CReceiveItemRequestMsgImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CR2GiveItem::getInstance().receiveItemRequest( *this ); 
}

//----------------------------------------------------------------------------
void CR2GiveItem::giveItemRequest( const CGiveItemRequestMsg &msg )
{
	TItemRequest req;
	req = (const CItemRequestMsgItf&)msg;
	if( _ValidateGiveItemRequest( req ) )
	{
		TPendingRequest::iterator it = _PendingRequest.find( req.CreatureRowId );
		if( it == _PendingRequest.end() )
		{
			pair< TPendingRequest::iterator, bool > res = _PendingRequest.insert( make_pair(req.CreatureRowId, TCreatureItemRequest() ) );
			if( res.second == false )
			{
				_SendAckToAIS( false, req );
				return;
			}
			it = res.first;
		}

		TCreatureItemRequest &vec = (*it).second;

		uint32 i;
		for( i = 0; i < vec.size(); ++i )
			if( vec[i] == msg )
				break;				
		if( i == vec.size() )
			vec.push_back( TItemRequest() );
		vec[i] = msg;
		vec[i].ActionId = ++_NextActionId;
		vec[i].IsGiveItem = true;
		_SetClientDB(vec[i], i);
		_SendAckToAIS( true, req );
	}
	else
	{
		_SendAckToAIS( false, req );
	}
}

//----------------------------------------------------------------------------
void CR2GiveItem::receiveItemRequest( const CReceiveItemRequestMsg &msg )
{
	TItemRequest req;
	req = (const CItemRequestMsgItf&)msg;

	TPendingRequest::iterator it = _PendingRequest.find( req.CreatureRowId );
	if( it == _PendingRequest.end() )
	{
		pair< TPendingRequest::iterator, bool > res = _PendingRequest.insert( make_pair(req.CreatureRowId, TCreatureItemRequest() ) );
		if( res.second == false )
		{
			return;
		}
		it = res.first;
	}
	
	TCreatureItemRequest &vec = (*it).second;
	
	uint32 i;
	for( i = 0; i < vec.size(); ++i )
		if( vec[i] == msg )
			break;				
	if( i == vec.size() )
		vec.push_back( TItemRequest() );
	vec[i] = msg;
	vec[i].ActionId = ++_NextActionId;
	vec[i].IsGiveItem = false;
	_SetClientDB(vec[i], i);
}

//----------------------------------------------------------------------------
void CR2GiveItem::_SetClientDB( const TItemRequest & req, uint32 index)
{
	if( index < BOTCHATTYPE::MaxR2MissionEntryDatabase )
	{
		CCharacter *c = PlayerManager.getChar( req.CharacterRowId );
		if( c )
		{
			ucstring ucstr;
			ucstr.fromUtf8( req.MissionText );
//			c->_PropertyDatabase.setProp( NLMISC::toString("TARGET:CONTEXT_MENU:MISSION_RING:%d:TITLE", index), _regiserLiteralString( req.CharacterRowId, ucstr ) );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSION_RING().getArray(index).setTITLE(c->_PropertyDatabase, _regiserLiteralString( req.CharacterRowId, ucstr ) );
//			c->_PropertyDatabase.setProp( NLMISC::toString("TARGET:CONTEXT_MENU:MISSION_RING:%d:ID", index), req.ActionId );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSION_RING().getArray(index).setID(c->_PropertyDatabase, req.ActionId );
			c->updateTargetingChars();
		}
	}
}

//----------------------------------------------------------------------------
void CR2GiveItem::_SetClientDBAll( CCharacter *c, const TCreatureItemRequest & req )
{
	nlassert(c);

	for( uint32 i = 0; i < BOTCHATTYPE::MaxR2MissionEntryDatabase; ++i )
	{
		if( i < req.size() )
		{
			_SetClientDB( req[i], i );
		}
		else
		{
//			c->_PropertyDatabase.setProp( NLMISC::toString("TARGET:CONTEXT_MENU:MISSION_RING:%d:TITLE", i), 0 );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSION_RING().getArray(i).setTITLE(c->_PropertyDatabase, 0 );
//			c->_PropertyDatabase.setProp( NLMISC::toString("TARGET:CONTEXT_MENU:MISSION_RING:%d:ID", i), 0 );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSION_RING().getArray(i).setID(c->_PropertyDatabase, 0 );
		}
	}
	c->updateTargetingChars();
}


//----------------------------------------------------------------------------
void CR2GiveItem::onUntarget( CCharacter *c, TDataSetRow oldTarget )
{
	for( uint32 i = 0; i < BOTCHATTYPE::MaxR2MissionEntryDatabase; ++i )
	{
//		c->_PropertyDatabase.setProp( NLMISC::toString("TARGET:CONTEXT_MENU:MISSION_RING:%d:TITLE", i), 0 );
		CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSION_RING().getArray(i).setTITLE(c->_PropertyDatabase, 0 );
//		c->_PropertyDatabase.setProp( NLMISC::toString("TARGET:CONTEXT_MENU:MISSION_RING:%d:ID", i), 0 );
		CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSION_RING().getArray(i).setID(c->_PropertyDatabase, 0 );
	}

	TPendingRequest::iterator it = _PendingRequest.find( oldTarget );
	if( it != _PendingRequest.end() )
	{
		(*it).second.clear();
		_PendingRequest.erase( it );
	}
}

//----------------------------------------------------------------------------
bool CR2GiveItem::_ValidateGiveItemRequest( const TItemRequest &req )
{
	CCharacter *c = PlayerManager.getChar( req.CharacterRowId );
	if( c == 0 )
		return false;

	CCreature *bot = CreatureManager.getCreature( req.CreatureRowId );
	if( bot == 0)
		return false;

	if( bot->isDead() || c->isDead() )
		return false;

	for( uint i = 0; i < req.ItemsRequest.size(); ++i )
	{
		if( req.ItemsRequest[i].Quantity > CR2MissionItem::getInstance().getNumberMissionItem(c->getId(), req.ItemsRequest[i].SheetId ) )
			return false;
	}
	return true;
}

//----------------------------------------------------------------------------
void CR2GiveItem::_SendAckToAIS( bool ok, const TItemRequest &req )
{
	CUserEventMsg eventMsg;
	eventMsg.InstanceNumber = req.InstanceId;
	eventMsg.GrpAlias = req.GroupAlias;
	eventMsg.EventId = (ok ? 1 : 2);
	CWorldInstances::instance().msgToAIInstance(eventMsg.InstanceNumber, eventMsg);
}

//----------------------------------------------------------------------------
uint32 CR2GiveItem::_regiserLiteralString( TDataSetRow userRowId, const ucstring &litStr )
{
	SM_STATIC_PARAMS_1(params,STRING_MANAGER::literal);
	params[0].Literal = litStr;
	return STRING_MANAGER::sendStringToClient( userRowId,"LITERAL", params );
}

//----------------------------------------------------------------------------
 void CR2GiveItem::giveItemGranted( TDataSetRow creatureRowId, uint32 actionId )
{
	TPendingRequest::iterator it = _PendingRequest.find( creatureRowId );
	if( it != _PendingRequest.end() )
	{
		TCreatureItemRequest &vec = (*it).second;
		for( uint32 i = 0; i < vec.size(); ++ i)
		{
			if( vec[i].ActionId == actionId )
			{
				CCharacter *c = PlayerManager.getChar( vec[i].CharacterRowId );
				nlassert(c);
				CCreature *e = CreatureManager.getCreature(creatureRowId);
				CMirrorPropValueRO<TYPE_SHEET> sheetInMirror( TheDataset, creatureRowId, DSPropertySHEET );
				NLMISC::CSheetId sheetId(sheetInMirror());
				const CStaticCreatures *clientCreatureForm = CSheets::getCreaturesForm( sheetId );
				if(e == NULL) return; 
				sint32 dx = c->getX() - e->getX();
				sint32 dy = c->getY() - e->getY();
				sint32 sqrDist = dx*dx + dy*dy;
				sint32 squareCreatureColRadius = 0;
				if(clientCreatureForm != 0)
				{
					squareCreatureColRadius = (sint32)(1000.0f * max(clientCreatureForm->getColRadius(), max(clientCreatureForm->getColWidth(),clientCreatureForm->getColLength())));
					squareCreatureColRadius= squareCreatureColRadius * squareCreatureColRadius;
				}

				if (sqrDist > (MaxTalkingDistSquare * 1000 * 1000 + squareCreatureColRadius))
				{
					CCharacter::sendDynamicSystemMessage( vec[i].CharacterRowId, "BS_TARGET_TOO_FAR");
					return;
				}

				CUserEventMsg eventMsg;
				eventMsg.InstanceNumber = vec[i].InstanceId;
				eventMsg.GrpAlias = vec[i].GroupAlias;
				// add the character and npc as parameter of the event
				eventMsg.Params.push_back(c->getId().toString());
				eventMsg.Params.push_back(e->getId().toString());

				if( vec[i].IsGiveItem )
				{
					if( _ValidateGiveItemRequest( vec[i] ) )
					{
						CR2MissionItem::getInstance().destroyMissionItem( c->getId(), vec[i].ItemsRequest );

						eventMsg.EventId = 3;
						CWorldInstances::instance().msgToAIInstance(eventMsg.InstanceNumber, eventMsg);
					}
				}
				else
				{
					CR2MissionItem::getInstance().giveMissionItem( c->getId(), c->currentSessionId(), vec[i].ItemsRequest );
					
					eventMsg.EventId = 1;
					CWorldInstances::instance().msgToAIInstance(eventMsg.InstanceNumber, eventMsg);
				}

				vec[i] = vec.back();
				vec.pop_back();
				
				_SetClientDBAll( c, vec );
				
				if( vec.size() == 0 )
					_PendingRequest.erase( it );
				return;
			}
		}
	}
}

//----------------------------------------------------------------------------
void CR2GiveItem::onUnspawn( TDataSetRow creatureRowId )
{
	TPendingRequest::iterator it = _PendingRequest.find( creatureRowId );
	if( it != _PendingRequest.end() )
	{
		_PendingRequest.erase( it );
	}
}

//============================================================================
NLMISC_COMMAND(giveItemRequest, "test command for give item R2 feature", "<Character eid><Creature eid><GroupAlias><InstanceId><Item SheetId><Item quantity><Mission text>")
{
	if( args.size() != 7 )
		return false;

	CGiveItemRequestMsg msg;
	CEntityId eid;

	eid.fromString(args[0].c_str());
	CCharacter *ch = PlayerManager.getChar(eid);
	if(ch) msg.CharacterRowId = ch->getEntityRowId();
	else return false;

	eid.fromString(args[1].c_str());
	CCreature *c = CreatureManager.getCreature(eid);
	if(c) msg.CreatureRowId = c->getEntityRowId();
	else return false;

	NLMISC::fromString(args[2], msg.GroupAlias);
	NLMISC::fromString(args[3], msg.InstanceId);
	msg.Items.push_back( CSheetId(args[4]));
	uint32 quantity;
	NLMISC::fromString(args[5], quantity);
	msg.Quantities.push_back(quantity);
	msg.MissionText = args[6];
	CR2GiveItem::getInstance().giveItemRequest( msg );
	return true;
}

NLMISC_COMMAND(giveItemGranted, "test command for give item R2 features", "<CreatureId><ActionId>")
{
	if( args.size() != 2 )
		return false;

	CEntityId eid;
	TDataSetRow creatureRowId;

	eid.fromString(args[0].c_str());
	CCreature *c = CreatureManager.getCreature(eid);
	if(c) creatureRowId = c->getEntityRowId();
	else return false;
	uint32 actionId;
	NLMISC::fromString(args[1], actionId);
	CR2GiveItem::getInstance().giveItemGranted( creatureRowId, actionId );
	return true;
}

NLMISC_COMMAND(unTarget, "test command for give item R2 features", "<CreatureId>")
{
	if( args.size() != 2 )
		return false;
	
	CEntityId eid;

	eid.fromString(args[0].c_str());
	CCharacter *ch = PlayerManager.getChar(eid);
	if( ch == 0 ) return false;

	eid.fromString(args[1].c_str());
	CCreature *c = CreatureManager.getCreature(eid);
	if(c) CR2GiveItem::getInstance().onUntarget(ch, c->getEntityRowId());
	else return false;
	return true;
}

NLMISC_COMMAND(unSpawn, "test command for give item R2 features", "<CharacterId><CreatureId>")
{
	if( args.size() != 1 )
		return false;
	
	CEntityId eid;
	eid.fromString(args[0].c_str());
	CCreature *c = CreatureManager.getCreature(eid);
	if(c) CR2GiveItem::getInstance().onUnspawn(c->getEntityRowId());
	else return false;
	return true;
}
