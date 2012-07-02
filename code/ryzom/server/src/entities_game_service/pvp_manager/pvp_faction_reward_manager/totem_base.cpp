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
#include "totem_base.h"
#include "totem_effect.h"
#include "player_manager/character.h"
#include "phrase_manager/s_effect.h"
#include "game_share/pvp_clan.h"
#include "game_share/backup_service_interface.h"
#include "pvp_faction_reward_manager.h"
#include "nel/net/message.h"
#include "world_instances.h"
#include "creature_manager/creature_manager.h"
#include "pvp_manager/pvp_manager_2.h"
#include "pvp_manager/pvp_faction_hof.h"
#include "phrase_manager/phrase_utilities_functions.h"

using namespace std;
using namespace NLMISC;
using namespace EFFECT_FAMILIES;
using namespace NLNET;

CVariable<uint32> TotemBuildTime( "egs", "TotemBuildTime", "Time needed to build a spire (in ticks)", 6000, 0, true  );
CVariable<uint32> TotemRebuildWait( "egs", "TotemRebuildWait", "Time to wait to rebuild a spire (in ticks)", 72000, 0, true ); 

#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily

//----------------------------------------------------------------------------

CTotemBase::CTotemBase( std::string const& name )
: _Name(name)
{
	_OwnerFaction = PVP_CLAN::Neutral;
	_TotemEffect = EFFECT_FAMILIES::Unknown;
	_BotObject = NULL;
	_IsBuildingFinished = true;
	_TotemCurrentHP = 1;
	_TotemMaxHP = 1;
	_BuildHpGain = 0;
	_LastTickUpdate = 0;
	_BuildingStartTime = 0;
	_LastTickAttackMessageSended = 0;

	for ( int i=0; i<PVP_CLAN::NbClans; i++)
	{
		_LastTimeOwned[i] = 0;
	}
}


//----------------------------------------------------------------------------

// TODO : this method must be updated with the new fame system
// Currently, it only uses personal faction points to determine
// the category of reward, and we missing NEUTRAl_GUILD category
CTotemBase::TRewardCategory CTotemBase::_GetRewardCatergory( CCharacter* user ) const
{
	pair< PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan > allegiance = user->getAllegiance();
	
	if( allegiance.first == _OwnerFaction || allegiance.second == _OwnerFaction )
		return FACTION_MEMBER;

	if( CPVPManager2::getInstance()->factionWarOccurs( allegiance.first, _OwnerFaction ) || 
		CPVPManager2::getInstance()->factionWarOccurs( allegiance.second, _OwnerFaction ) )
		return FACTION_ENEMY;

	return NEUTRAL_STRICT;
}


//----------------------------------------------------------------------------

void CTotemBase::getTotemEffect( CCharacter* user, vector<CSEffect*>& outEffects ) const
{
	// no one owns this totem (or the totem building isn't over yet)
	if ( ( _OwnerFaction == PVP_CLAN::Neutral ) || ( !_IsBuildingFinished ) )
		return;

	CSEffect* pEffect = NULL;
	bool ok = false;
	bool statEffect = true;
	sint32 effectValue = CPVPFactionRewardManager::EffectValues[ _TotemEffect ];

	switch ( _GetRewardCatergory( user ) )
	{
	case FACTION_ENEMY :
		effectValue *= -1;
		break;

	case NEUTRAl_GUILD :
		effectValue /= 2;
		break;
		
	case NEUTRAL_STRICT :
		effectValue /= 5;
		break;
	}

	// is this effect already added ?
	uint i = 0;
	while ( !ok && ( i < outEffects.size() ) )
	{
		pEffect = outEffects[i];
		ok = ( pEffect->getFamily() == _TotemEffect );
		i++;
	}

	if ( !ok )
	{
		// no, add it

		switch ( _TotemEffect )
		{
		// characteristics modifiers
		case TotemStatsHP :
		case TotemStatsSap :
		case TotemStatsSta :
		case TotemStatsFoc :
		case TotemRegenHP :
		case TotemRegenSap :
		case TotemRegenSta :
		case TotemRegenFoc :
		case TotemMiscMov :
			pEffect = new CTotemCharacEffect( user->getEntityRowId(), user->getEntityRowId(), 
											 _TotemEffect, effectValue );
			break;

			// harvest
		case TotemHarvestAgg :
		case TotemHarvestQty :
		case TotemHarvestZRs :
		
		// craft
		case TotemCraftSuc :

		// combat
		case TotemCombatRes :
		case TotemCombatPar :
		case TotemCombatCri :
		case TotemCombatMagOff :
		case TotemCombatMagDef :
		case TotemCombatMROff :
		case TotemCombatMRSpd :
		case TotemCombatDS :
		case TotemCombatArm :
		case TotemCombatPoZ :
			pEffect = new CTotemEffect( user->getEntityRowId(), user->getEntityRowId(), 
				                        _TotemEffect, effectValue );
			break;

		default :
			pEffect = NULL;
		}
		
		if ( pEffect != NULL )
			outEffects.push_back( pEffect );
	}
	else
		// if already added, just update the value
		pEffect->setParamValue( pEffect->getParamValue() + effectValue );
}

//----------------------------------------------------------------------------

void CTotemBase::startBuilding( CCharacter * builder )
{
	_BuildingStartTime = CTickEventHandler::getGameCycle();
	_LastTickUpdate = _BuildingStartTime;
	_IsBuildingFinished = false;
	if( builder != 0 )
		_Builder = builder->getEntityRowId();
	else
		_Builder = TDataSetRow();

	if ( _BotObject )
	{
		_BotObject->getScores()._PhysicalScores[SCORES::hit_points].Current = 1;
		_TotemCurrentHP = 1;
		
		_BotObject->getPhysCharacs()._PhysicalCharacteristics[SCharacteristicsAndScores::current_regenerate].Base = 0;

		_TotemMaxHP = (float)_BotObject->getScores()._PhysicalScores[SCORES::hit_points].Max;
		_BuildHpGain = _TotemMaxHP / (float)TotemBuildTime;
	}
}

//----------------------------------------------------------------------------

void CTotemBase::setBotObject( CCreature* botObject )
{
	if ( !botObject )
		return;
	
	_BotObject = botObject;

	// if the totem is totem is currently building, it means
	// that this method was called because of the sheetId change
	// we must then restore some information
	if ( _IsBuildingFinished == false )
	{
		_BotObject->getScores()._PhysicalScores[SCORES::hit_points].Max = (sint32)_TotemMaxHP;
		_BotObject->getScores()._PhysicalScores[SCORES::hit_points].Current = (sint32)_TotemCurrentHP;
	}
}

//----------------------------------------------------------------------------

void CTotemBase::destroyTotem()
{	
	CPVPFactionHOF::getInstance()->writeStatInHOFDatabase( 0, _OwnerFaction, CPVPFactionHOF::destroyed_spire, 1);

	_IsBuildingFinished = true;
	_LastTimeOwned[ _OwnerFaction ] = CTickEventHandler::getGameCycle();
	_OwnerFaction = PVP_CLAN::Neutral;

	if ( _BotObject )
	{
		_BotObject->getPhysCharacs()._PhysicalCharacteristics[SCORES::hit_points].CurrentRegenerate = 0;
		_BotObject->getScores()._PhysicalScores[SCORES::hit_points].Current = 
			_BotObject->getScores()._PhysicalScores[SCORES::hit_points].Max;
		
		// change the used sheet
	
		uint32 messageVersion = 3;
		bool bAutoSpawnDespawn = false;
		string botName = toString( _BotObject->getId() );
		string sCustomName;
		CSheetId sheetId( "spire_neutral.creature" );

		if (sheetId==CSheetId::Unknown)
		{
			nlwarning("Unknown sheet id '%s'", "spire_neutral.creature" );
		}
		
		CMessage msgout("EVENT_BOT_SHEET");
		msgout.serial(messageVersion);
		msgout.serial(botName);
		msgout.serial(sheetId);
		msgout.serial(bAutoSpawnDespawn);
		msgout.serial(sCustomName);

		CWorldInstances::instance().msgToAIInstance2( _BotObject->getInstanceNumber(), msgout);
	}
}

//----------------------------------------------------------------------------

bool CTotemBase::tickUpdate()
{
	if ( _IsBuildingFinished == true )
		return false;

	NLMISC::TGameCycle currentCycle = CTickEventHandler::getGameCycle();
	
	_TotemCurrentHP += ( currentCycle - _LastTickUpdate ) * _BuildHpGain;

	if ( _TotemCurrentHP > _TotemMaxHP )
		_TotemCurrentHP = _TotemMaxHP;

	if ( _BotObject )
	{
		_BotObject->getScores()._PhysicalScores[SCORES::hit_points].Current = (sint32)_TotemCurrentHP;
		nlinfo( "Totem %d HP : %d", _RegionAlias, _BotObject->currentHp() );
	}
	
	if ( currentCycle - _BuildingStartTime >= TotemBuildTime )
	{
		// the totem has finished building
		_IsBuildingFinished = true;
		CPVPFactionHOF::getInstance()->writeStatInHOFDatabase( 0, _OwnerFaction, CPVPFactionHOF::builded_spire, 1 );
	}

	// save updates every 10 seconds
	if ( ( currentCycle % 100 ) == 0 )
	{
		string sFilePath = toString( "totems/totems_%d.bin", _RegionAlias );
				
		static CPersistentDataRecordRyzomStore pdr;
		pdr.clear();

		store( pdr );

		CBackupMsgSaveFile msg( sFilePath, CBackupMsgSaveFile::SaveFile, Bsi );
		
		uint32 bufSize= pdr.totalDataSize();
		vector<char> buffer;
		buffer.resize( bufSize );
		pdr.toBuffer( &buffer[0], bufSize );
		msg.DataMsg.serialBuffer( (uint8*)&buffer[0], bufSize );

		Bsi.sendFile( msg );
	}

	_LastTickUpdate = currentCycle;

	return true;
}

//----------------------------------------------------------------------------

struct TTotemFileCallback : public IBackupFileReceiveCallback
{
	CTotemBase *TotemBase;
	TTotemFileCallback(CTotemBase *totemBase)
		: TotemBase(totemBase)
	{
	}

	virtual void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		TotemBase->totemFileCallback(fileDescription, dataStream);
	}

};

void CTotemBase::totemFileCallback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
{
	static CPersistentDataRecordRyzomStore pdr;
	pdr.clear();
	pdr.fromBuffer(dataStream);
//	pdr.readFromFile( sFilePath.c_str() );

	apply( pdr );
}


void CTotemBase::loadFromPDR()
{
	TTotemFileCallback *cb = new TTotemFileCallback(this);
	Bsi.syncLoadFile(toString( "totems/totems_%d.bin", _RegionAlias ), cb);

//	string sFilePath = Bsi.getLocalPath();
//	sFilePath += toString( "totems/totems_%d.bin", _RegionAlias );
//	
//	if ( !CFile::fileExists( sFilePath ) )
//		// file does not exist, there is nothing to load
//		return;
	
//	static CPersistentDataRecordRyzomStore pdr;
//	pdr.clear();
//	pdr.readFromFile( sFilePath.c_str() );
//
//	apply( pdr );
}

//----------------------------------------------------------------------------

bool CTotemBase::canStartBuilding( CCharacter* actor )
{
	if ( _OwnerFaction != PVP_CLAN::Neutral )
	{
		nlwarning("Can't build totem: totem not neutral");
		return false;
	}
	
	pair< PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan > allegiance = actor->getAllegiance();
	NLMISC::TGameCycle currentCycle = CTickEventHandler::getGameCycle();

	// a faction must wait some time before being able to rebuild a totem
	bool returnValue = currentCycle - _LastTimeOwned[allegiance.first] > TotemRebuildWait;
	if ( !returnValue  )
	{
		nlwarning("Can't build totem: rebuild delay not finished");
		return false;
	}

	// send message to clients
	if( returnValue == false && actor != 0 )
	{
		uint32 minutes, seconds;
		minutes = (TotemRebuildWait - (currentCycle - _LastTimeOwned[allegiance.first])) / 600;
		seconds = (TotemRebuildWait - (currentCycle - _LastTimeOwned[allegiance.first]) - minutes * 600) / 10;
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::integer, STRING_MANAGER::integer);
			params[0].Int = minutes;
			params[1].Int = seconds;
		PHRASE_UTILITIES::sendDynamicSystemMessage(actor->getEntityRowId(),"PVP_SPIRE_REBUILD_DELAY",params);
	}
	return returnValue;
}

// ----------------------------------------------------------------------------
// PERSISTENCE METHODS
// ----------------------------------------------------------------------------

#define PERSISTENT_MACROS_AUTO_UNDEF

#define PERSISTENT_CLASS CTotemBase

#define PERSISTENT_PRE_STORE\
	H_AUTO(CTotemBaseStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CTotemBaseApply);\

#define PERSISTENT_DATA\
	PROP2(_OwnerFaction, string, PVP_CLAN::toString( _OwnerFaction ), _OwnerFaction = PVP_CLAN::fromString( val ); )\
	PROP(bool,			_IsBuildingFinished)\
	PROP_GAME_CYCLE_COMP(_BuildingStartTime)\
	PROP_GAME_CYCLE_COMP(_LastTickUpdate)\
	PROP(float,			_BuildHpGain)\
	PROP(float,			_TotemMaxHP)\
	PROP(float,			_TotemCurrentHP)\
	
//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"
