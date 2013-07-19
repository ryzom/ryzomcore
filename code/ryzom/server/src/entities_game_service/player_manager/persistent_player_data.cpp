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


/*

  TODO:
	add history of time on line per player

	housekeeping
	- move the persistent data methods out to the different classes' source files
	- create handy pdr commands and add to PDR stuff in game share
		- bin2xml
		- xml2bin
		- showNode <pdr file or wildcard> <node name or wildcard>
		- setFlag <pdr file or wildcard> <address>
		- setUInt32 <pdr file or wildcard> <address> <value>
		- setSInt32 <pdr file or wildcard> <address> <value>
		- setUInt64 <pdr file or wildcard> <address> <value>
		- setSInt64 <pdr file or wildcard> <address> <value>
		- setFloat <pdr file or wildcard> <address> <value>
		- setDouble <pdr file or wildcard> <address> <value>
		- setString <pdr file or wildcard> <address> <value>
		- removeNode <pdr file or wildcard> <address>
		- copyNode <pdr src file> <pdr dest file> <node address>
		address:= [<struct>':']*(<NodesId as wildcard>[@<map entry as wildcard>]
*/



//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"
#include "nel/misc/sstring.h"
#include "nel/misc/variable.h"
#include "player_manager/character.h"
#include "player_manager/gm_tp_pending_command.h"
#include "player_manager/character_respawn_points.h"
#include "building_manager/building_physical.h"
#include "building_manager/building_manager.h"
#include "player_manager/character_version_adapter.h"
#include "pvp_manager/pvp_challenge.h"
#include "pvp_manager/pvp.h"
#include "player_manager/character_encyclopedia.h"
#include "player_manager/character_game_event.h"
#include "player_manager/player_room.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "shop_type/items_for_sale.h"
#include "death_penalties.h"
#include "guild_manager/fame_manager.h"
#include "zone_manager.h"
#include "ring_reward_points.h"

#include "egs_sheets/egs_sheets.h"
#include "server_share/log_item_gen.h"

//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace EGSPD;

CVariable<uint32>			MaxNoRentDisconnectedTime("egs","MaxNoRentDisconnectedTime", "Number of minutes of log off time after no rent item are deleted", 8*60, 0, true );


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

// If the following macro is defined then the macros such as PERSISTENT_CLASS, PERSISTENT_DATA, PERSISTENT_PRE_STORE, etc
// are undefined after inclusion of perstent_data_template.h
#define PERSISTENT_MACROS_AUTO_UNDEF

//-----------------------------------------------------------------------------
// external
//-----------------------------------------------------------------------------
extern CPlayerManager PlayerManager;

//-----------------------------------------------------------------------------
// Persistent data for CFameContainerEntryPD
//-----------------------------------------------------------------------------

class CFameContainerEntryProxy
{
public:
	DECLARE_PERSISTENCE_METHODS_WITH_TARGET(CFameContainerEntryPD &target)
	void clear(CFameContainerEntryPD &target)
	{
		target.setFame(0);
		target.setFameMemory(0);
		target.setLastFameChangeTrend(CFameTrend::Unknown);
	}
};

#define PERSISTENT_CLASS CFameContainerEntryProxy

#define PERSISTENT_STORE_ARGS const CFameContainerEntryPD &target
#define PERSISTENT_APPLY_ARGS CFameContainerEntryPD &target

#define PERSISTENT_PRE_STORE\
	H_AUTO(CFameContainerEntryProxyStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CFameContainerEntryProxyApply);\

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear(target))\
	PROP2(Fame,sint32,target.getFame(),target.setFame(val))\
	PROP2(FameMemory,sint32,target.getFameMemory(),target.setFameMemory(val))\
	PROP2(LastFameChangeTrend,string,CFameTrend::toString(target.getLastFameChangeTrend()),target.setLastFameChangeTrend(CFameTrend::fromString(val)))\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CFameContainerPD
//-----------------------------------------------------------------------------

class CFameContainerProxy
{
public:
	DECLARE_PERSISTENCE_METHODS_WITH_TARGET(CFameContainerPD &target)
};

#define PERSISTENT_CLASS CFameContainerProxy

#define PERSISTENT_STORE_ARGS const CFameContainerPD &target
#define PERSISTENT_APPLY_ARGS CFameContainerPD &target


#define PERSISTENT_PRE_STORE\
	typedef std::map<NLMISC::CSheetId, CFameContainerEntryPD>::const_iterator	TIterator;\
	TIterator itBegin= target.getEntriesBegin();\
	TIterator itEnd= target.getEntriesEnd();\
	
#define PERSISTENT_DATA\
	FLAG0(CLEAR,while(target.getEntriesBegin()!=target.getEntriesEnd()) target.deleteFromEntries((*target.getEntriesBegin()).first))\
	LSTRUCT_MAP2(_Fame,NLMISC::CSheetId,\
		for (TIterator it=itBegin;it!=itEnd;++it) if ((*it).second.getFame()!=0x7fffffff),\
		(*it).first,\
		CFameContainerEntryProxy().store(pdr,(*it).second),\
		CFameContainerEntryProxy().apply(pdr,*target.addToEntries(key)))\
	
//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for inventories
//-----------------------------------------------------------------------------

class CInventoryProxy
{
public:
	DECLARE_PERSISTENCE_METHODS

	/// ctor
	CInventoryProxy(CCharacter * owner, const CInventoryPtr & inv) : _Owner(owner), _Inventory(inv) {}

	/// load item from pdr to inventory
	void loadItem(CPersistentDataRecord & pdr)
	{
		CGameItemPtr itm;
		CGameItem::CPersistentApplyArg applyArgs(_Owner);
		itm.newItem()->apply(pdr, applyArgs);

		BOMB_IF( itm->getStackSize() == 0, "CInventoryProxy::addItem load a empty stack from PDR", itm.deleteItem(); return );

		// ANTIBUG: remove items with unknown sheet
		const CStaticItem* form = CSheets::getForm(itm->getSheetId());
		if (itm->getSheetId() == CSheetId::Unknown || form == NULL )
		{
			nlwarning("Player %s : found unknown item in inventory '%s' at slot %u sheet %s",
				_Owner->getId().toString().c_str(),
				INVENTORIES::toString(_Inventory->getInventoryId()).c_str(),
				itm->getInventorySlot(),
				itm->getSheetId().toString().c_str()
				);
			TLogContext_Item_BadSheet contextBadSheet(_Owner->getId());
			itm.deleteItem();
			return;
		}

		// check if item is NO Rent and enough time have passed off line
		if(form->NoRent)
		{
			if( CTime::getSecondsSince1970() - _Owner->getLastDisconnectionDate() > MaxNoRentDisconnectedTime * 60)
			{
				nlinfo("Player %s : a no rent item in inventory '%s' at slot %u sheet %s are deleted",
					_Owner->getId().toString().c_str(),
					INVENTORIES::toString(_Inventory->getInventoryId()).c_str(),
					itm->getInventorySlot(),
					itm->getSheetId().toString().c_str()
					);
				TLogContext_Item_NoRent contextNoRent(_Owner->getId());
				itm.deleteItem();
				return;
			}
		}

		if (_Inventory->getFreeSlotCount() > 0)
		{
			_Inventory->forceLoadItem(itm, applyArgs.InventorySlot);
		}
		else
		{
			STOP(NLMISC::toString("Failed to fit item loaded from input into inventory! (sheet=%s)",itm->getSheetId().toString().c_str()));
			itm.deleteItem();
		}
	}

	/// clear inventory
	void clear()
	{
		_Inventory->clearInventory();
	}

private:
	CCharacter * const	_Owner;
	CInventoryPtr		_Inventory;
};

#define PERSISTENT_CLASS CInventoryProxy

#define PERSISTENT_PRE_STORE\
	H_AUTO(CInventoryProxyStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CInventoryProxyApply);\

#define PERSISTENT_DATA\
	FLAG0(CLEAR, clear())\
	LSTRUCT_VECT(_Item,\
	ARRAY_LOGIC(_Inventory->getSlotCount()) if (_Inventory->getItem(i)!=NULL && _Inventory->getItem(i)->getStaticForm()->Family!=ITEMFAMILY::SCROLL_R2),\
		_Inventory->getItem(i)->store(pdr),\
		loadItem(pdr))\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


// Position pre-processing before saving
static void prepareCharacterPositionForStore ( COfflineEntityState & state, const CCharacter & user )
{
	sint32 cell = 0;
	TDataSetRow dsr = user.getEntityRowId();
	if ( dsr.isNull() )
	{
		nlwarning( "ANTIBUG: Character %s EnterFlag but row not set", user.getId().toString().c_str() );
	}
	else
	{
		CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, dsr, DSPropertyCELL );
		cell = mirrorCell;			
		if ( CBuildingManager::getInstance()->isRoomCell( cell ) )
		{
			const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone( user.getBuildingExitZone() );
			if ( zone )
			{
				zone->getRandomPoint( state.X, state.Y,state.Z,state.Heading );
			}
			else
			{
				nlwarning("user %s is not found in a room but cell is %d)", user.getId().toString().c_str(), cell );
			}
		}
		else if ( cell <= -2 && ( cell & 0x00000001) != 0 && user.getPVPInterface().isValid() )
		{
			const CPVPChallenge * ch = dynamic_cast<const CPVPChallenge *>( user.getPVPInterface().getPVPSession() );
			if ( ch )
			{
				uint16 teamIdx,memberIdx;
				const CPVPChallenge::CMember* member = ch->getMember(user.getEntityRowId(),teamIdx, memberIdx );
				if ( member )
				{
					state = member->OldCoords;
				}
			}
		}
	}
	if ( state.X <= 0 || state.Y >= 0 )
	{
		/* saving a guy that is in a teleportation */
		state.X = user.getTpCoordinate().X;
		state.Y = user.getTpCoordinate().Y;
		state.Z = user.getTpCoordinate().Z;
		state.Heading = user.getTpCoordinate().Heading;
	}
}


//-----------------------------------------------------------------------------
// Persistent data for CCharacter
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CCharacter

#define PERSISTENT_PRE_STORE\
	H_AUTO(CCharacterStore);\
	CFameManager::getInstance().savePlayerFame(_Id, const_cast<EGSPD::CFameContainerPD &>(*_Fames));\
	/* Update the current playing session duration */ \
	if (!_LastLogStats.empty()) _LastLogStats.begin()->Duration = CTime::getSecondsSince1970() - _LastLogStats.begin()->LoginTime;\
	else nlwarning("Cannot update play session duration, _LastLogStats is empty, new character?");\
	\
	/* Unless the top of the position stack is locked, */ \
	/* update the stored position stack with the current position */ \
	if (_SessionId != SessionLockPositionStack) \
	{ \
		if ( getEnterFlag() )\
		{\
			CFarPosition farPosToSave;\
			farPosToSave.SessionId = _SessionId;\
			farPosToSave.PosState = _EntityState;\
			prepareCharacterPositionForStore( farPosToSave.PosState, *this ); \
			\
			if (farPosToSave.SessionId.asInt() == 0)\
				nlwarning("Can't save position with sessionId 0 for %s (%u)", getId().toString().c_str(), (uint)getEnterFlag());\
			else\
				PositionStack.topToModify() = farPosToSave;\
		}\
	} \
	sint32 hp = _PhysScores._PhysicalScores[SCORES::hit_points].Current;\
	if (IsRingShard) { const_cast<CCharacter*>(this)->_PhysScores._PhysicalScores[SCORES::hit_points].Current = std::max(hp,sint32(1)); }\
	if (!IsRingShard) { const_cast<CCharacter*>(this)->_HPB = _PhysScores._PhysicalScores[SCORES::hit_points].Current; }\

#define PERSISTENT_POST_STORE\
	if (IsRingShard) { const_cast<CCharacter*>(this)->_PhysScores._PhysicalScores[SCORES::hit_points].Current = hp; }\


// Sidenote:
// We could have used the PositionStack the following way:
// - Load PositionStack
// - When character connects, apply top position and pop
// - Evenly push the current position, save the stack and pop
// But then we would have encountered an issue with the "return to mainland" feature
// because we can't modify the current position with the new top of stack (after pop)
// when a character is 'EnterGame'.
// Thus the following scheme is used:
// - Load PositionStack
// - When character connects, apply top position 
// - Evenly overwrite the top position with the current position, and save the stack
// Hence the "return to mainland" feature does not change the current position but
// only pops and locks the stack (to prevent from overwriting it) so that the new top
// will be saved and loaded at the next logon.

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CCharacterApply);\
	uint32 version=~0u;\
	prepareToLoad();\
	_HPB= 0x12345678; \

#define PERSISTENT_POST_APPLY\
	if (version!=~0u)\
		CCharacterVersionAdapter::getInstance()->adaptCharacterFromVersion(*this,version);\
	\
	if (!IsRingShard) /*for now: done only on normal (non-ring) shard*/\
	{\
		/* process pending Tp command for this character if exist */\
		COfflineEntityState state;\
		if( CGmTpPendingCommand::getInstance()->getTpPendingforCharacter( _Name.toString(), state, *this ) )\
		{\
			if (PositionStack.empty())\
				_EntityState = state;\
			else  /*loading an old file with no normal positions*/ \
				PositionStack.topToModify().PosState = state;\
		}\
	}\
	\
	postLoadTreatment();\
	if(!IsRingShard && _HPB!=0x12345678) { _PhysScores._PhysicalScores[SCORES::hit_points].Current = _HPB; }\

#define PERSISTENT_DATA\
	FLAG(CLEAR,clear())\
	PROP2(VERSION,uint32,CCharacterVersionAdapter::getInstance()->currentVersionNumber(),version=val)\
	\
	PROP(uint8,	_HairType)\
	PROP(uint8,	_HairColor)\
	PROP(uint8,	_HatColor)\
	PROP(uint8,	_JacketColor)\
	PROP(uint8,	_ArmsColor)\
	PROP(uint8,	_TrousersColor)\
	PROP(uint8,	_FeetColor)\
	PROP(uint8,	_HandsColor)\
	PROP(uint64,_Money)\
\
	/* property for backup character HP independently of what happened in ring session*/\
	PROP(sint32,_HPB)\
\
	LPROP_MAP2(FactionPoints, string, uint32,\
	for(uint32 i = 0; i < (PVP_CLAN::EndClans-PVP_CLAN::BeginClans+1); ++i),\
	PVP_CLAN::toString((PVP_CLAN::TPVPClan)(i+PVP_CLAN::BeginClans)),\
	_FactionPoint[i],\
	PVP_CLAN::TPVPClan k=PVP_CLAN::fromString(key); if ((k>=PVP_CLAN::BeginClans) && (k<=PVP_CLAN::EndClans)) _FactionPoint[k-PVP_CLAN::BeginClans]=val)\
\
	PROP(uint32,_PvpPoint)\
	PROP2(_LangChannel,string,_LangChannel,_LangChannel=val)\
	PROP(uint32,_Organization)\
	PROP(uint32,_OrganizationStatus)\
	PROP(uint32,_OrganizationPoints)\
	PROP2(DeclaredCult,string,PVP_CLAN::toString(_DeclaredCult),_DeclaredCult=PVP_CLAN::fromString(val))\
	PROP2(DeclaredCiv,string,PVP_CLAN::toString(_DeclaredCiv),_DeclaredCiv=PVP_CLAN::fromString(val))\
\
	PROP(bool,_PVPFlag)\
	PROP_GAME_CYCLE_COMP(_PVPFlagLastTimeChange)\
	PROP_GAME_CYCLE_COMP(_PVPRecentActionTime)\
	PROP_GAME_CYCLE_COMP(_PVPFlagTimeSettedOn)\
	PROP(uint16,_RegionKilledInPvp)\
\
	/* we don't want save pvp outpost status */\
	/*PROP2(OutpostAlias, uint32, getOutpostAlias(), _OutpostAlias.tempStore(val))*/\
	/*PROP2(OutpostSide, uint8, getOutpostSide(), _OutpostSide.tempStore(val))*/\
	/*PROP_GAME_CYCLE_COMP(_OutpostLeavingTime)*/\
\
	LPROP(string, _SDBPvPPath, if (!_SDBPvPPath.empty()))\
	PROP(uint32,_GuildId)\
	PROP(uint8, _CreationPointsRepartition)\
	PROP_GAME_CYCLE_COMP(_ForbidAuraUseStartDate)\
	PROP_GAME_CYCLE_COMP(_ForbidAuraUseEndDate)\
	PROP2(_Title, string, CHARACTER_TITLE::toString(getTitle()), setTitle(CHARACTER_TITLE::toCharacterTitle(val)))\
	PROP2(_NewTitle, string, _NewTitle, _NewTitle=val)\
	PROP2(_TagPvPA, string, _TagPvPA, _TagPvPA=val)\
	PROP2(_TagPvPB, string, _TagPvPB, _TagPvPB=val)\
	PROP2(_TagA, string, _TagA, _TagA=val)\
	PROP2(_TagB, string, _TagB, _TagB=val)\
\
	/* Visual Properties */\
	PROP2(HairType,				uint8, _VisualPropertyA().PropertySubData.HatModel,			SET_STRUCT_MEMBER(_VisualPropertyA,PropertySubData.HatModel,val))\
	PROP2(HairColor,			uint8, _VisualPropertyA().PropertySubData.HatColor,			SET_STRUCT_MEMBER(_VisualPropertyA,PropertySubData.HatColor,val))\
	PROP2(GabaritHeight,		uint8, _VisualPropertyC().PropertySubData.CharacterHeight,	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.CharacterHeight,val))\
	PROP2(GabaritTorsoWidth,	uint8, _VisualPropertyC().PropertySubData.TorsoWidth,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.TorsoWidth,val))\
	PROP2(GabaritArmsWidth,		uint8, _VisualPropertyC().PropertySubData.ArmsWidth,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.ArmsWidth,val))\
	PROP2(GabaritLegsWidth,		uint8, _VisualPropertyC().PropertySubData.LegsWidth,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.LegsWidth,val))\
	PROP2(GabaritBreastSize,	uint8, _VisualPropertyC().PropertySubData.BreastSize,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.BreastSize,val))\
	PROP2(MorphTarget1,			uint8, _VisualPropertyC().PropertySubData.MorphTarget1,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget1,val))\
	PROP2(MorphTarget2,			uint8, _VisualPropertyC().PropertySubData.MorphTarget2,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget2,val))\
	PROP2(MorphTarget3,			uint8, _VisualPropertyC().PropertySubData.MorphTarget3,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget3,val))\
	PROP2(MorphTarget4,			uint8, _VisualPropertyC().PropertySubData.MorphTarget4,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget4,val))\
	PROP2(MorphTarget5,			uint8, _VisualPropertyC().PropertySubData.MorphTarget5,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget5,val))\
	PROP2(MorphTarget6,			uint8, _VisualPropertyC().PropertySubData.MorphTarget6,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget6,val))\
	PROP2(MorphTarget7,			uint8, _VisualPropertyC().PropertySubData.MorphTarget7,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget7,val))\
	PROP2(MorphTarget8,			uint8, _VisualPropertyC().PropertySubData.MorphTarget8,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget8,val))\
	PROP2(EyesColor,			uint8, _VisualPropertyC().PropertySubData.EyesColor,		SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.EyesColor,val))\
	PROP2(Tattoo,				uint8, _VisualPropertyC().PropertySubData.Tattoo,			SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.Tattoo,val))\
	PROP2(NameStringId,			uint8, _VisualPropertyB().PropertySubData.Name,				SET_STRUCT_MEMBER(_VisualPropertyB,PropertySubData.Name,val))\
\
	PROP(bool, _HairCuteDiscount )\
\
	PROP_SET(CSheetId,_BoughtPhrases)\
	PROP_SET(CSheetId,_KnownBricks)\
	LPROP_VECT2(_FriendsList, CEntityId, VECT_LOGIC(_FriendsList), _FriendsList[i].EntityId,\
		{\
		CContactId	contactId;	\
		contactId.EntityId= val;	\
		contactId.ContactId= ~0u; /* Default Invalid Id. Init at sendContactListInit() time */	\
		_FriendsList.push_back(contactId);	\
		}\
	)\
	LPROP_VECT2(_IgnoreList, CEntityId, VECT_LOGIC(_IgnoreList), _IgnoreList[i].EntityId,\
		{\
		CContactId	contactId;	\
		contactId.EntityId= val;	\
		contactId.ContactId= ~0u; /* Default Invalid Id. Init at sendContactListInit() time */	\
		_IgnoreList.push_back(contactId);	\
		}\
	)\
	PROP_VECT(CEntityId,_IsFriendOf)\
	PROP_VECT(CEntityId,_IsIgnoredBy)\
\
	STRUCT(_MemorizedPhrases)\
	STRUCT2(_ForbidPowerDates, _ForbidPowerDates.store(pdr), _ForbidPowerDates.apply(pdr))\
	STRUCT2(_IneffectiveAuras, _IneffectiveAuras.store(pdr), _IneffectiveAuras.apply(pdr))\
	STRUCT2(_ConsumableOverdoseEndDates, _ConsumableOverdoseEndDates.store(pdr), _ConsumableOverdoseEndDates.apply(pdr))\
	STRUCT(_ModifiersInDB)\
	STRUCT2(_DeathPenalties, _DeathPenalties->store(pdr), _DeathPenalties->apply(pdr))\
	STRUCT2(_Missions, _Missions->store(pdr), _Missions->apply(pdr))\
	STRUCT2(_ItemsInShopStore, _ItemsInShopStore->store(pdr), _ItemsInShopStore->apply(pdr))\
	STRUCT2(_PlayerRoom, _PlayerRoom->store(pdr), _PlayerRoom->apply(pdr,this))\
	STRUCT2(EntityBase,	CEntityBase::store(pdr), CEntityBase::apply(pdr))\
	STRUCT2(RespawnPoints,	_RespawnPoints->store(pdr), _RespawnPoints->apply(pdr))\
	LPROP(float,_NextDeathPenaltyFactor,if(_NextDeathPenaltyFactor!=1.0f))\
	STRUCT2(_Fames,CFameContainerProxy().store(pdr,*_Fames),CFameContainerProxy().apply(pdr,*_Fames))\
	STRUCT(RingRewardPoints)\
	STRUCT(_PersistentEffects)\
\
	STRUCT_VECT(_Pact)\
	STRUCT_VECT(_KnownPhrases)\
	STRUCT_MAP(TAIAlias,	TMissionHistory,	_MissionHistories)\
	LSTRUCT(_WelcomeMissionDesc, if (_WelcomeMissionDesc.isValid()))\
	STRUCT_ARRAY(_PlayerPets,_PlayerPets.size())\
\
	LPROP_MAP2(SkillPoints, string, double,\
		for(uint32 i=0;i<CSPType::EndSPType;++i),\
		CSPType::toString((CSPType::TSPType)i),\
		_SpType[i],\
		CSPType::TSPType k=CSPType::fromString(key); if (k!=CSPType::Unknown) _SpType[k]=val)\
\
	LPROP_MAP2(SpentSkillPoints, string, uint32,\
	for(uint32 i=0;i<CSPType::EndSPType;++i),\
	CSPType::toString((CSPType::TSPType)i),\
	_SpentSpType[i],\
	CSPType::TSPType k=CSPType::fromString(key); if (k!=CSPType::Unknown) _SpentSpType[k]=val)\
\
	LPROP_MAP2(ScorePermanentModifiers,string,sint32,\
		for(uint32 i=0;i<SCORES::NUM_SCORES;++i) if(_ScorePermanentModifiers[i] != 0),\
		SCORES::toString((SCORES::TScores)i),\
		_ScorePermanentModifiers[i],\
		SCORES::TScores k=SCORES::toScore(key); if (k!=SCORES::unknown) _ScorePermanentModifiers[k]=val)\
\
	LPROP_MAP2(StartingCharacteristicValues,string,uint8,\
		for(uint32 i=0;i<CHARACTERISTICS::NUM_CHARACTERISTICS;++i),\
		CHARACTERISTICS::toString((CHARACTERISTICS::TCharacteristics)i),\
		_StartingCharacteristicValues[i],\
		CHARACTERISTICS::TCharacteristics k=CHARACTERISTICS::toCharacteristic(key); if (k!=CHARACTERISTICS::Unknown) _StartingCharacteristicValues[k]=val)\
\
	PROP(uint32, _FirstConnectedTime)\
	PROP(uint32, _LastConnectedTime)\
	PROP(uint32, _PlayedTime)\
	STRUCT_LIST(TCharacterLogTime, _LastLogStats)\
\
	LSTRUCT_MAP2(Inventory,string,\
		for(uint32 i=0; i<INVENTORIES::NUM_INVENTORY; ++i) \
			if (i!=INVENTORIES::handling && i!=INVENTORIES::equipment) \
				if (_Inventory[i]!=NULL && _Inventory[i]->getSlotCount() != 0),\
		INVENTORIES::toString((INVENTORIES::TInventory)i),\
		CInventoryProxy(const_cast<CCharacter *>(this), _Inventory[i]).store(pdr),\
		{\
			INVENTORIES::TInventory k = INVENTORIES::toInventory(key);\
			if (k != INVENTORIES::UNDEFINED)\
			{\
				if (_Inventory[k] == NULL)\
				{\
					DEBUG_STOP;\
					continue;\
				}\
				CInventoryProxy(this, _Inventory[k]).apply(pdr);\
			}\
			else  \
			{ \
				NLMISC::WarningLog->displayNL("CCharacter pdr, can't load inventory named '%s' (unknown)", key.c_str()); \
				while (!pdr.isEndOfStruct()) \
					pdr.skipData(); \
			} \
		}\
	)\
\
	STRUCT2(_EncycloChar, _EncycloChar->store(pdr), _EncycloChar->apply(pdr))\
	STRUCT2(_GameEvent, _GameEvent->store(pdr), _GameEvent->apply(pdr))\
	PROP_VECT(CSheetId,_PersistentItemServices)\
	PROP_VECT(uint32,_MissionsQueues)\
\
	STRUCT2(_EntityPosition,DEFAULT_LOGIC,_EntityState.apply(pdr)) /* Conversion from old version */\
\
	STRUCT2(NormalPositions, PositionStack.store(pdr), PositionStack.apply(pdr)) /* Name changed but keeping the same name in the file */\
	PROP2(Invisible, bool, getInvisibility(), setInvisibility(val)) \
	PROP2(Aggroable, sint8, getAggroableSave(), setAggroableSave(val)) \
	PROP2(GodMode, bool, getGodModeSave(), setGodModeSave(val)) \
	PROP2(FriendVisibility, uint8, getFriendVisibilitySave(), setFriendVisibilitySave(val)) \


//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CEntityBase
//
// Note: _EntityPosition has been moved to CCharacter.
// Thus saving another class based on CEntityBase is not supported.
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CEntityBase

#define PERSISTENT_PRE_APPLY\

#define PERSISTENT_PRE_STORE\

#define PERSISTENT_POST_APPLY\

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	STRUCT2(_EntityPosition,DEFAULT_LOGIC,_EntityState.apply(pdr)) /* has been moved to CCharacter, only load for previous version */\
	PROP2(_SheetId,uint32,_SheetId(),_SheetId=val)\
	PROP2(_Name,string,_Name.toUtf8(),_Name.fromUtf8(val))\
	PROP2(_Race,string,CPeople::toString(_Race),_Race=CPeople::fromString(val))\
	PROP(uint8,_Gender)\
	PROP(uint8,_Size)\
	PROP(bool,_DodgeAsDefense)\
	STRUCT(_PhysCharacs)\
	STRUCT(_PhysScores)\
	STRUCT(_Skills)\
	\
	NPROP(MeleeAttackModifierOnEnemy,sint32,_SpecialModifiers.MeleeAttackModifierOnEnemy)\
	NPROP(MeleeAttackModifierOnSelf,sint32,_SpecialModifiers.MeleeAttackModifierOnSelf)\
	NPROP(MagicCastingModifierOnSelf,sint32,_SpecialModifiers.MagicCastingModifierOnSelf)\
	NPROP(MagicCastingModifierOnEnemy,sint32,_SpecialModifiers.MagicCastingModifierOnEnemy)\
	NPROP(RangeAttackModifierOnEnemy,sint32,_SpecialModifiers.RangeAttackModifierOnEnemy)\
	NPROP(RangeAttackModifierOnSelf,sint32,_SpecialModifiers.RangeAttackModifierOnSelf)\
	NPROP(AttackModifierOnSelf,sint32,_SpecialModifiers.AttackModifierOnSelf)\
	NPROP(ChanceToFailStrategy,sint32,_SpecialModifiers.ChanceToFailStrategy)\
	NPROP(ChanceToFailSpell,sint32,_SpecialModifiers.ChanceToFailSpell)\
	NPROP(ChanceToFailFaber,sint32,_SpecialModifiers.ChanceToFailFaber)\
	NPROP(ChanceToFailHarvest,sint32,_SpecialModifiers.ChanceToFailHarvest)\
	NPROP(ChanceToFailTracking,sint32,_SpecialModifiers.ChanceToFailTracking)\
	NPROP(MeleeAttackSlow,sint32,_SpecialModifiers.MeleeAttackSlow)\
	NPROP(MeleeSlashingDamageArmor,sint32,_SpecialModifiers.MeleeSlashingDamageArmor)\
	NPROP(MeleeBluntDamageArmor,sint32,_SpecialModifiers.MeleeBluntDamageArmor)\
	NPROP(MeleePiercingDamageArmor,sint32,_SpecialModifiers.MeleePiercingDamageArmor)\
	NPROP(MeleeDamageModifierFactor,sint32,_SpecialModifiers.MeleeDamageModifierFactor)\
	NPROP(RangeDamageModifierFactor,sint32,_SpecialModifiers.RangeDamageModifierFactor)\
	NPROP(CreatureMeleeTakenDamageFactor,sint32,_SpecialModifiers.CreatureMeleeTakenDamageFactor)\
	NPROP(CreatureRangeTakenDamageFactor,sint32,_SpecialModifiers.CreatureRangeTakenDamageFactor)\
	NPROP(CombatBrickLatencyMultiplier,sint32,_SpecialModifiers.CombatBrickLatencyMultiplier)\
	NPROP(MagicBrickLatencyMultiplier,sint32,_SpecialModifiers.MagicBrickLatencyMultiplier)\
	NPROP(ArmorQualityModifier,sint32,_SpecialModifiers.ArmorQualityModifier)\
	NPROP(WeaponQualityModifier,sint32,_SpecialModifiers.WeaponQualityModifier)\
	NPROP(ArmorAbsorbtionMultiplier,sint32,_SpecialModifiers.ArmorAbsorbtionMultiplier)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CPetAnimal
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CPetAnimal

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CPetAnimalApply);\
	IsTpAllowed= true;\

#define PERSISTENT_PRE_STORE\
	H_AUTO(CPetAnimalStore);\
	if (PetStatus==not_present)\
		return;\
	/* 16-bit ticketSlot is only used here for backward compatibility */\
	/* !!! DO NOT BREAK THIS !!! */\
	sint16 ticketSlot = -1;\
	if (ItemPtr != NULL && ItemPtr->getInventorySlot() != INVENTORIES::INVALID_INVENTORY_SLOT)\
		ticketSlot = sint16(ItemPtr->getInventorySlot());\
	TAIAlias stableAlias = CAIAliasTranslator::Invalid;\
	if (PetStatus == stable)\
	{\
		CPlace * stablePlace = NULL;\
		if (StableId != 0xffff)\
			stablePlace = CZoneManager::getInstance().getPlaceFromId( uint16(StableId) );\
		if (stablePlace)\
			stableAlias = stablePlace->getAlias();\
	}\

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP2(TicketPetSheetId,CSheetId,TicketPetSheetId,\
		TicketPetSheetId = val;\
		const CStaticItem* ticketPetForm = CSheets::getForm( TicketPetSheetId );\
		MaxSatiety = ticketPetForm ? ticketPetForm->PetHungerCount : 0;\
	)\
	PROP(CSheetId,PetSheetId)\
	LPROP(uint32,Price,if(Price!=0))\
	PROP(CEntityId,OwnerId)\
	PROP2(StableAlias,TAIAlias,stableAlias,\
		if (val != CAIAliasTranslator::Invalid)\
		{\
			CPlace * place = CZoneManager::getInstance().getPlaceFromAlias( val );\
			if (place)\
				StableId = place->getId();\
		}\
	)\
	PROP(sint32,Landscape_X)\
	PROP(sint32,Landscape_Y)\
	PROP(sint32,Landscape_Z)\
	PROP_GAME_CYCLE_COMP(DeathTick)\
	PROP2(PetStatus,uint16,PetStatus,PetStatus=(CPetAnimal::TStatus)val)\
	PROP2(Slot,sint16,ticketSlot,if (val != -1) Slot = uint32(val))\
	LPROP(bool,IsFollowing,if(IsFollowing))\
	LPROP(bool,IsMounted,if(IsMounted))\
	PROP(bool,IsTpAllowed)\
	PROP(TSatiety,Satiety)\
	PROP2(CustomName, ucstring, CustomName, CustomName = val)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CPact
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CPact

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP(uint8,PactNature)\
	PROP(uint8,PactType)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CAuraActivationDateVector
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CAuraActivationDateVector

#define PERSISTENT_POST_APPLY\
	_AuraUsers.resize(_AuraActivationDates.size(), CEntityId::Unknown);\
//	cleanVector();

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	STRUCT_VECT(_AuraActivationDates)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CPowerActivationDate
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CPowerActivationDate

#define PERSISTENT_DATA\
	PROP2(DeactivationDate, NLMISC::TGameCycle, CTickEventHandler::getGameCycle() - DeactivationDate, DeactivationDate = val)\
	PROP2(ActivationDate, NLMISC::TGameCycle, ActivationDate - CTickEventHandler::getGameCycle(), ActivationDate = val)\
	PROP(uint16, ConsumableFamilyId)\
	PROP2(PowerType,string,POWERS::toString(PowerType),PowerType=POWERS::toPowerType(val))\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

//-----------------------------------------------------------------------------
// Persistent data for CPowerActivationDateVector
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CPowerActivationDateVector

#define PERSISTENT_POST_APPLY\
//	cleanVector();

#define PERSISTENT_PRE_STORE\

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	STRUCT_VECT(PowerActivationDates)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

//-----------------------------------------------------------------------------
// Persistent data for CConsumableOverdoseTimer
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CConsumableOverdoseTimer

#define PERSISTENT_DATA\
	LPROP2(ActivationDate, NLMISC::TGameCycle, if(ActivationDate >= CTickEventHandler::getGameCycle()), ActivationDate - CTickEventHandler::getGameCycle(), ActivationDate = val)\
	PROP2(Family,string,  CConsumable::getFamilyName(Family), Family=CConsumable::getFamilyIndex(val))\
	
//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

//-----------------------------------------------------------------------------
// Persistent data for CConsumableOverdoseTimerVector
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CConsumableOverdoseTimerVector

#define PERSISTENT_POST_APPLY\
//	cleanVector();

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	STRUCT_VECT(Dates)

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CPlayerPhraseMemory
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CPlayerPhraseMemory

#define PERSISTENT_PRE_STORE\
	H_AUTO(CPlayerPhraseMemoryStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CPlayerPhraseMemoryApply);\

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	LSTRUCT_MAP2(_MemSets,uint32,\
		 VECT_LOGIC(_MemSets) if (_MemSets[i]!=NULL),\
		 i,\
		 _MemSets[i]->store(pdr),\
		 if (key<_MemSets.size()) getMemSet(key)->apply(pdr))\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CMemorizationSet
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CMemorizationSet

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	LSTRUCT_MAP2(Phrases,uint32,\
		 VECT_LOGIC(Phrases) if (Phrases[i]!=NULL),\
		 i,\
		 Phrases[i]->store(pdr),\
		if (key<Phrases.size()) \
		{ \
			if (Phrases[key]==NULL) \
				Phrases[key]=new CMemorizedPhrase; \
			Phrases[key]->apply(pdr);\
		})\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CMemorizedPhrase
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CMemorizedPhrase

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP_VECT(CSheetId,Bricks)\
	PROP(uint16,PhraseId)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CKnownPhrase
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CKnownPhrase

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	STRUCT(PhraseDesc)\
	PROP(CSheetId,PhraseSheetId)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for TMissionHistory
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS TMissionHistory

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP(bool, Successfull)\
	PROP_GAME_CYCLE_COMP(LastSuccessDate)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CWelcomeMissionDesc
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CWelcomeMissionDesc

#define PERSISTENT_DATA\
	PROP(TAIAlias, MissionAlias)\
	PROP(TAIAlias, BotAlias)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CDeathPenalties
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CDeathPenalties

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP(uint8,_NbDeath)\
	PROP(double,_CurrentDeathXP)\
	PROP(double,_DeathXPToGain)\
	PROP(uint32,_BonusUpdateTime)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CEntityState
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CEntityState

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP(sint32,X)\
	PROP(sint32,Y)\
	PROP(sint32,Z)\
	PROP(float,Heading)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for COfflineEntityState
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS COfflineEntityState

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP(sint32,X)\
	PROP(sint32,Y)\
	PROP(sint32,Z)\
	PROP(float,Heading)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CFarPosition
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CFarPosition

#define PERSISTENT_DATA\
	PROP2(SessionId,uint32,SessionId.asInt(),SessionId=TSessionId(val))\
	STRUCT(PosState)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CFarPositionStack
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CFarPositionStack

#define PERSISTENT_DATA\
	STRUCT_VECT(_Vec)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CPhysicalCharacteristics
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CPhysicalCharacteristics

#define PERSISTENT_PRE_STORE\
	H_AUTO(CPhysicalCharacteristicsStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CPhysicalCharacteristicsApply);\

static void displayInfo(const std::string& s)
{
	
	egs_ppdinfo("%s",s.c_str());
}

static void displayWarning(const std::string& s)
{
	nlwarning("%s",s.c_str());
}

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	LPROP_MAP2(_PhysicalCharacteristics,string,sint32,\
		for(uint32 i=0;i<CHARACTERISTICS::NUM_CHARACTERISTICS;++i),\
		CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics)i ),\
		_PhysicalCharacteristics[i].Base,\
		CHARACTERISTICS::TCharacteristics k=CHARACTERISTICS::toCharacteristic(key);\
		if (k!=CHARACTERISTICS::Unknown) { _PhysicalCharacteristics[k].Base=val; _PhysicalCharacteristics[k].Current=val; _PhysicalCharacteristics[k].Max=val; displayInfo(string()+"Setting "+key+": "+toString(val)); }\
		else displayWarning(string()+"Failed to set "+key+": "+toString(val)) )

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CPhysicalScores
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CPhysicalScores

#define PERSISTENT_PRE_STORE\
	H_AUTO(CPhysicalScoresStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CPhysicalScoresApply);\

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP(float,BaseWalkSpeed)\
	PROP(float,BaseRunSpeed)\
	PROP2(CurrentWalkSpeed,float,CurrentWalkSpeed(),CurrentWalkSpeed=val)\
	PROP2(CurrentRunSpeed,float,CurrentRunSpeed(),CurrentRunSpeed=val)\
	LSTRUCT_MAP2(PhysicalScores,string,\
		 for(uint32 i=0;i<SCORES::NUM_SCORES;++i),\
		 SCORES::toString((uint16)i),\
		 _PhysicalScores[i].store(pdr),\
		 SCORES::TScores k=SCORES::toScore(key); if (k!=SCORES::unknown) _PhysicalScores[k].apply(pdr))\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for SCharacteristicsAndScores
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS SCharacteristicsAndScores

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP(sint32,Current)\
	PROP(sint32,Base)\
	PROP(sint32,Max)\
	LPROP(float,BaseRegenerateRepos,if (BaseRegenerateRepos!=0.0f))\
	LPROP(float,BaseRegenerateAction,if (BaseRegenerateRepos!=0.0f))\
	LPROP(float,CurrentRegenerate,if (BaseRegenerateRepos!=0.0f))\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"




//-----------------------------------------------------------------------------
// Persistent data for CSkills
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CSkills

#define PERSISTENT_PRE_STORE\
	H_AUTO(CSkillsStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CSkillsApply);\

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	LSTRUCT_MAP2(Skills,string,\
		for(uint32 i=0;i<SKILLS::NUM_SKILLS;++i) if (_Skills[i].Base!=0),\
		SKILLS::toString((uint16)i),\
		_Skills[i].store(pdr),\
		SKILLS::ESkills k=SKILLS::toSkill(key); if (k!=SKILLS::unknown) _Skills[k].apply(pdr))\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"




//-----------------------------------------------------------------------------
// Persistent data for SSkill
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS SSkill

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP(sint32,Base)\
	PROP(sint32,Current)\
	PROP(sint32,MaxLvlReached)\
	PROP(double,Xp)\
	PROP(double,XpNextLvl)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CSPhraseCom
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CSPhraseCom

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP2(Name,string,Name.toUtf8(),Name.fromUtf8(val))\
	PROP_VECT(CSheetId,Bricks)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CModifiersInDB
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CModifiersInDB

//#define PERSISTENT_PRE_STORE\
//	const TGameCycle time = CTickEventHandler::getGameCycle();

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	LSTRUCT_MAP2(Bonus,uint32,\
		VECT_LOGIC(Bonus) if (Bonus[i].SheetId!=CSheetId::Unknown),\
		i,\
		Bonus[i].store(pdr),\
		if (key<Bonus.size()) Bonus[key].apply(pdr))\
	LSTRUCT_MAP2(Malus,uint32,\
		VECT_LOGIC(Malus) if (Malus[i].SheetId!=CSheetId::Unknown),\
		i,\
		Malus[i].store(pdr),\
		if (key<Malus.size()) Malus[key].apply(pdr))\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CModifierInDB
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CModifierInDB

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP(CSheetId,SheetId)\
	PROP_GAME_CYCLE_COMP(ActivationDate)\
	PROP(bool,Disabled)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"




//-----------------------------------------------------------------------------
// Persistent data for CPlayerRoomInterface::CPlayerRoomData
//-----------------------------------------------------------------------------
/**
 * This class is used to load old player room inventory, DO NOT BREAK IT!
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005
 */
class COldPlayerRoomInventoryLoader
{
public:
	DECLARE_PERSISTENCE_METHODS

	/// ctor
	COldPlayerRoomInventoryLoader(const CInventoryPtr & inv) : _Inventory(inv) {}

	/// load an item in the inventory from pdr
	void loadItem(CPersistentDataRecord & pdr)
	{
		CGameItemPtr itm;
		CGameItem::CPersistentApplyArg applyArgs;
		itm.newItem()->apply(pdr, applyArgs);
		BOMB_IF( itm->getStackSize() == 0, "COldPlayerRoomInventoryLoader::addItem load a empty stack from PDR", itm.deleteItem(); return );

		// ANTIBUG: remove items with unknown sheet
		if (itm->getSheetId() == CSheetId::Unknown)
		{
			nlwarning("found unknown item in inventory '%s' at slot %u",
				INVENTORIES::toString(_Inventory->getInventoryId()).c_str(),
				itm->getInventorySlot()
				);
			itm.deleteItem();
			return;
		}

		if (_Inventory->getFreeSlotCount() > 0)
		{
			_Inventory->forceLoadItem(itm, applyArgs.InventorySlot);
		}
		else
		{
			STOP(NLMISC::toString("Failed to fit item loaded from input into inventory! (sheet=%s)", itm->getSheetId().toString().c_str()));
			itm.deleteItem();
		}
	}

	/// clear inventory
	void clear()
	{
		_Inventory->clearInventory();
	}

private:
	CInventoryPtr _Inventory;
};

//-----------------------------------------------------------------------------
#define PERSISTENT_CLASS COldPlayerRoomInventoryLoader

#define PERSISTENT_PRE_APPLY\
	H_AUTO(COldPlayerRoomInventoryLoaderApply);\

#define PERSISTENT_DATA\
	FLAG0(CLEAR, clear())\
	LSTRUCT_VECT(Child, if (0), ;/* do not store in old format anymore */, loadItem(pdr))

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

//-----------------------------------------------------------------------------
#define PERSISTENT_CLASS CPlayerRoomInterface::CPlayerRoomData
#define PERSISTENT_TOKEN_CLASS CPlayerRoomData

#define PERSISTENT_PRE_STORE\
	H_AUTO(CPlayerRoomDataStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CPlayerRoomDataApply);\

/*
	Token "Inventory" was used to save old player room inventory, we still use it to load old saves (DO NOT suppress it).
	New token "RoomInventory" is now used for new inventory format.
*/
#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP2(Building,TAIAlias,\
		(Building?Building->getAlias():CAIAliasTranslator::Invalid),\
		Building = dynamic_cast<CBuildingPhysicalPlayer*> ( CBuildingManager::getInstance()->getBuildingPhysicalsByAlias(val) ))\
	STRUCT2(Inventory, ;/* do not store in old format anymore */, COldPlayerRoomInventoryLoader(Inventory).apply(pdr))\
	STRUCT2(RoomInventory, Inventory->store(pdr), Inventory->apply(pdr, NULL))\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CGameItem
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CGameItem

#define PERSISTENT_APPLY_ARGS	CPersistentApplyArg & applyArgs

#define PERSISTENT_PRE_STORE\
	H_AUTO(CGameItemStore);\
	BOMB_IF(this == NULL,"Attempt to save an NULL item!", return);\
	uint32 RefInventoryId = (uint32) INVENTORIES::NUM_INVENTORY;\
	if (_RefInventory != NULL)\
		RefInventoryId = (uint32)_RefInventory->getInventoryId();\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CGameItemApply);\
	_RefInventorySlot = INVENTORIES::INVALID_INVENTORY_SLOT;\
	uint32 RefInventoryId = INVENTORIES::NUM_INVENTORY;\
	uint16 slotImage = 0xffff;\


#define PERSISTENT_POST_APPLY\
	if ( slotImage != 0xFFFF )\
	{\
		nlassert(RefInventoryId == (uint32) INVENTORIES::NUM_INVENTORY);\
		if ( slotImage < 2 )\
		{\
			RefInventoryId = INVENTORIES::handling;\
			_RefInventorySlot = slotImage;\
			nlinfo("Convert item %s, had slot image %u, is now in HAND, slot %u", _SheetId.toString().c_str(),slotImage,_RefInventorySlot);\
		}\
		else\
		{\
			RefInventoryId = INVENTORIES::equipment;\
			_RefInventorySlot = slotImage - 2;\
			nlinfo("Convert item %s, had slot image %u, is now in EQUIPMENT, slot %u", _SheetId.toString().c_str(),slotImage,_RefInventorySlot);\
		}\
	}\
	postApply((INVENTORIES::TInventory) RefInventoryId, applyArgs.Owner);\

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP2(_ItemId,						uint64,		_ItemId.getRawId(),			_ItemId = INVENTORIES::TItemId(val))\
	PROP2(_SheetId,						CSheetId,	_SheetId,					_SheetId=val)\
/*	PROP2(_LocSlot,						uint32,		_InventorySlot,				_InventorySlot=val)*/\
	PROP2(_LocSlot,						uint32,		_InventorySlot,				applyArgs.InventorySlot=val)\
	/*PROP2(_ClientInventoryPosition,	sint16,		_ClientInventoryPosition,	_ClientInventoryPosition=val)*/\
	PROP2(_HP,							uint32,		_HP,						_HP=val)\
	PROP2(_Recommended,					uint32,		_Recommended,				_Recommended=val)\
	PROP2(_CreatorId,					CEntityId,	_CreatorId,					_CreatorId=val)\
	PROP2(_PhraseId,					string,		_PhraseId,					_PhraseId=val)\
	LSTRUCT2(_CraftParameters,						if (_CraftParameters != NULL),	_CraftParameters->store(pdr),	_CraftParameters = new CItemCraftParameters; _CraftParameters->apply(pdr))\
	LPROP2(_SlotImage,					uint16,		if (0),		0xffff,				slotImage=val)\
	LPROP2(_SapLoad,					uint32,		if (_SapLoad!=0),			_SapLoad,							_SapLoad=val)\
	LPROP2(_Dropable,					bool,		if (!_Dropable),			_Dropable,							_Dropable=val)\
	LPROP2(_Destroyable,				bool,		if (!_Destroyable),			_Destroyable,						_Destroyable=val)\
	LPROP2(_RefInventorySlot,			uint32,		if (_RefInventory != NULL),	_RefInventorySlot,					_RefInventorySlot=val)\
	LPROP2(RefInventoryId,				uint32,		if (_RefInventory != NULL),	_RefInventory->getInventoryId(),	RefInventoryId=val)\
	LPROP2(StackSize,					uint32,		if (_StackSize!=1),			_StackSize,							_StackSize=val)\
	PROP(bool, _UseNewSystemRequirement)\
	LPROP2(_RequiredSkill,				string,		if (_RequiredSkill!=SKILLS::unknown),	SKILLS::toString(_RequiredSkill),	_RequiredSkill=SKILLS::toSkill(val))\
	LPROP2(_RequiredSkillLevel,			uint16,		if (_RequiredSkillLevel!=0), _RequiredSkillLevel,				_RequiredSkillLevel=val )\
	LPROP2(_RequiredSkill2,				string,		if (_RequiredSkill2!=SKILLS::unknown),	SKILLS::toString(_RequiredSkill2),	_RequiredSkill2=SKILLS::toSkill(val))\
	LPROP2(_RequiredSkillLevel2,		uint16,		if (_RequiredSkillLevel2!=0),_RequiredSkillLevel2,				_RequiredSkillLevel2=val )\
	LPROP2(_RequiredCharac,				string,		if (_RequiredCharac!=CHARACTERISTICS::Unknown),	CHARACTERISTICS::toString(_RequiredCharac),		_RequiredCharac=CHARACTERISTICS::toCharacteristic(val))\
	LPROP2(_RequiredCharacLevel,		uint16,		if (_RequiredCharacLevel!=0),_RequiredCharacLevel,				_RequiredCharacLevel=val)\
	STRUCT_VECT(_TypeSkillMods)\
	LPROP_VECT(CSheetId, _Enchantment, VECT_LOGIC(_Enchantment) if (_Enchantment[i]!=CSheetId::Unknown))\
	PROP2(_CustomText,					ucstring,	_CustomText,				_CustomText=val)\
	PROP(bool, _LockedByOwner)\
	
//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
// Persistent data for CItemCraftParameters
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CItemCraftParameters

#define PERSISTENT_PRE_STORE\
	H_AUTO(CItemCraftParametersStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CItemCraftParametersApply);\

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	LPROP(float,Durability,if (Durability!=0.0f))\
	LPROP(float,Weight,if (Weight!=0.0f))\
	LPROP(float,SapLoad,if (SapLoad!=0.0f))\
	LPROP(float,StatEnergy,if (StatEnergy!=0.0f))\
	LPROP(float,Dmg,if (Dmg!=0.0f))\
	LPROP(float,Speed,if (Speed!=0.0f))\
	LPROP(float,Range,if (Range!=0.0f))\
	LPROP(float,DodgeModifier,if (DodgeModifier!=0.0f))\
	LPROP(float,ParryModifier,if (ParryModifier!=0.0f))\
	LPROP(float,AdversaryDodgeModifier,if (AdversaryDodgeModifier!=0.0f))\
	LPROP(float,AdversaryParryModifier,if (AdversaryParryModifier!=0.0f))\
	LPROP(float,ProtectionFactor,if (ProtectionFactor!=0.0f))\
	LPROP(float,MaxSlashingProtection,if (MaxSlashingProtection!=0.0f))\
	LPROP(float,MaxBluntProtection,if (MaxBluntProtection!=0.0f))\
	LPROP(float,MaxPiercingProtection,if (MaxPiercingProtection!=0.0f))\
	LPROP(uint8,Color,if (Color!=1))\
	LPROP(sint32,HpBuff,if (HpBuff!=0))\
	LPROP(sint32,SapBuff,if (SapBuff!=0))\
	LPROP(sint32,StaBuff,if (StaBuff!=0))\
	LPROP(sint32,FocusBuff,if (FocusBuff!=0))\
	LPROP(float,ElementalCastingTimeFactor,if (ElementalCastingTimeFactor!=0.0f))\
	LPROP(float,ElementalPowerFactor,if (ElementalPowerFactor!=0.0f))\
	LPROP(float,OffensiveAfflictionCastingTimeFactor,if (OffensiveAfflictionCastingTimeFactor!=0.0f))\
	LPROP(float,OffensiveAfflictionPowerFactor,if (OffensiveAfflictionPowerFactor!=0.0f))\
	LPROP(float,HealCastingTimeFactor,if (HealCastingTimeFactor!=0.0f))\
	LPROP(float,HealPowerFactor,if (HealPowerFactor!=0.0f))\
	LPROP(float,DefensiveAfflictionCastingTimeFactor,if (DefensiveAfflictionCastingTimeFactor!=0.0f))\
	LPROP(float,DefensiveAfflictionPowerFactor,if (DefensiveAfflictionPowerFactor!=0.0f))\
	PROP2(Protection, string, BACK_COMPAT::OLD_PROTECTION_TYPE::toString(Protection), Protection = BACK_COMPAT::OLD_PROTECTION_TYPE::fromString(val))\
	PROP2(Protection1, string, PROTECTION_TYPE::toString(Protection1), Protection1 = PROTECTION_TYPE::fromString(val))\
	LPROP(float,Protection1Factor,if (Protection1Factor!=0.0f))\
	PROP2(Protection2, string, PROTECTION_TYPE::toString(Protection2), Protection2 = PROTECTION_TYPE::fromString(val))\
	LPROP(float,Protection2Factor,if (Protection2Factor!=0.0f))\
	PROP2(Protection3, string, PROTECTION_TYPE::toString(Protection3), Protection3 = PROTECTION_TYPE::fromString(val))\
	LPROP(float,Protection3Factor,if (Protection3Factor!=0.0f))\
	LPROP(float,DesertResistanceFactor,if (Protection3Factor!=0.0f))\
	LPROP(float,ForestResistanceFactor,if (Protection3Factor!=0.0f))\
	LPROP(float,LacustreResistanceFactor,if (Protection3Factor!=0.0f))\
	LPROP(float,JungleResistanceFactor,if (Protection3Factor!=0.0f))\
	LPROP(float,PrimaryRootResistanceFactor,if (Protection3Factor!=0.0f))\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CCharacterRespawnPoints
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CCharacterRespawnPoints

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CCharacterRespawnPointsApply);\

#define PERSISTENT_PRE_STORE\
	H_AUTO(CCharacterRespawnPointsStore);\
	vector<string> respawnPointNames;\
	for (uint i = 0; i < _RegularRespawnPoints.size(); i++)\
	{\
		const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone( _RegularRespawnPoints[i] );\
		if (zone)\
			respawnPointNames.push_back( zone->getName() );\
		else\
			STOP( NLMISC::toString("<SPAWN_ZONE> %s lost the respawn point with index %hu", _Char.getId().toString().c_str(), _RegularRespawnPoints[i]) );\
	}\

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clearRespawnPoints())\
	LPROP_VECT2(RespawnPoints, string, VECT_LOGIC(respawnPointNames), respawnPointNames[i],\
{\
	TRespawnPoint respawnPoint = CZoneManager::getInstance().getTpSpawnZoneIdByName(val);\
	if (respawnPoint != InvalidSpawnZoneId)\
		_RegularRespawnPoints.push_back(respawnPoint);\
	else\
		( nlwarning("<SPAWN_ZONE> %s lost the respawn point '%s'", _Char.getId().toString().c_str(), val.c_str()) );\
}\
		)\
	LSTRUCT_MAP2(MissionRespawnPoints, string,\
		MAP_LOGIC(sint32, CMissionRespawnPoints, _MissionRespawnPointsByContinent),\
		CONTINENT::toString((CONTINENT::TContinent)(*it).first),\
		(*it).second.store(pdr, _Char),\
	{\
		CONTINENT::TContinent continent = CONTINENT::toContinent(key);\
		if (continent != CONTINENT::UNKNOWN)\
		{\
			CMissionRespawnPoints& missionRespawnPoints = _MissionRespawnPointsByContinent[sint32(continent)];\
			missionRespawnPoints.apply(pdr, _Char);\
			if (missionRespawnPoints.empty())\
			{\
				_MissionRespawnPointsByContinent.erase(sint32(continent));\
				( nlwarning("<SPAWN_ZONE> %s has lost all mission respawn points of continent '%s'", _Char.getId().toString().c_str(), key.c_str()) );\
			}\
		}\
	}\
		)\


//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

//-----------------------------------------------------------------------------
// Persistent data for CCharacterRespawnPoints::CMissionRespawnPoints
//-----------------------------------------------------------------------------
#define PERSISTENT_CLASS CCharacterRespawnPoints::CMissionRespawnPoints
#define PERSISTENT_TOKEN_CLASS CMissionRespawnPoints

#define PERSISTENT_APPLY_ARGS const CCharacter& c
#define PERSISTENT_STORE_ARGS const CCharacter& c

#define PERSISTENT_PRE_STORE\
	vector<string> respawnPointNames;\
	for (uint i = 0; i < size(); i++)\
	{\
		const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone( (*this)[i] );\
		if (zone)\
			respawnPointNames.push_back( zone->getName() );\
		else\
			STOP( NLMISC::toString("<SPAWN_ZONE> %s lost the mission respawn point with index %hu", c.getId().toString().c_str(), (*this)[i]) );\
	}\

#define PERSISTENT_DATA\
	LPROP_VECT2(MissionRespawnPoints, string, VECT_LOGIC(respawnPointNames), respawnPointNames[i],\
{\
	TRespawnPoint respawnPoint = CZoneManager::getInstance().getTpSpawnZoneIdByName(val);\
	if (respawnPoint != InvalidSpawnZoneId)\
		push_back(respawnPoint);\
	else\
		( nlwarning("<SPAWN_ZONE> %s lost the mission respawn point '%s'", c.getId().toString().c_str(), val.c_str()) );\
}\
		)\
	PROP(bool, _HideOthers)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


// ****************************************************************************
// ENCYCLOPEDIA MANAGEMENT
// ****************************************************************************


//-----------------------------------------------------------------------------
// Persistent data for CCharaterEncyclopedia
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CCharacterEncyclopedia

#define PERSISTENT_PRE_STORE\
	H_AUTO(CCharacterEncyclopediaStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CCharacterEncyclopediaApply);\
	clear();\

#define PERSISTENT_POST_APPLY\
	init();\

#define PERSISTENT_DATA\
	STRUCT_VECT(_EncyCharAlbums)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

//-----------------------------------------------------------------------------
// Persistent data for CEncyCharAlbum
//-----------------------------------------------------------------------------
#define PERSISTENT_CLASS CCharacterEncyclopedia::CEncyCharAlbum
#define PERSISTENT_TOKEN_CLASS CEncyCharAlbum

#define PERSISTENT_DATA\
	PROP(uint8,AlbumState)\
	STRUCT_VECT(Themas)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

//-----------------------------------------------------------------------------
// Persistent data for CEncyCharThema
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CCharacterEncyclopedia::CEncyCharThema
#define PERSISTENT_TOKEN_CLASS CEncyCharThema

#define PERSISTENT_DATA\
	PROP(uint8,ThemaState)\
	PROP(uint16,RiteTaskStatePacked)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

//-----------------------------------------------------------------------------
// Persistent data for TCharacterLogTime
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS TCharacterLogTime

#define PERSISTENT_DATA\
	PROP(uint32,LoginTime)\
	PROP(uint32,Duration)\
	PROP(uint32,LogoffTime)\
	
//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


// ****************************************************************************
// GAME EVENT MANAGEMENT
// ****************************************************************************

//-----------------------------------------------------------------------------
// Persistent data for CCharaterGameEvent
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CCharacterGameEvent

#define PERSISTENT_PRE_STORE\
	H_AUTO(CCharacterGameEventStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CCharacterGameEventApply);\

#define PERSISTENT_DATA\
	PROP_GAME_CYCLE_COMP(_Date)\
	LPROP2(_EventFaction, string, if (!_EventFaction.empty()), _EventFaction, setEventFaction(val))\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


