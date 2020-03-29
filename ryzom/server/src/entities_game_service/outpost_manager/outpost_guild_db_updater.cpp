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

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include "stdpch.h"
#include "outpost_guild_db_updater.h"

#include "player_manager/cdb.h"
#include "outpost_manager/outpost.h"
#include "egs_sheets/egs_static_outpost.h"
#include "guild_manager/guild.h"
#include "guild_manager/guild_manager.h"
#include "outpost_manager/outpost_manager.h"


//----------------------------------------------------------------------------
// namespaces
//----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//----------------------------------------------------------------------------
// methods COutpostGuildDBHandler
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
COutpostGuildDBHandler::COutpostGuildDBHandler(CCDBGroup * guildDB, uint32 outpostIndex)
: _GuildDB(guildDB)
{
	nlassert(guildDB != NULL);
	nlassert(outpostIndex < OUTPOSTENUMS::MAX_OUTPOST);

//	string outpostDBPath = toString("GUILD:OUTPOST:O%u", outpostIndex);
//	_GuildDBOutpostNode = guildDB->getICDBStructNodeFromName(outpostDBPath);
	_GuildDBOutpostNode = CBankAccessor_GUILD::getGUILD().getOUTPOST().getO(outpostIndex);
//	nlassert(_GuildDBOutpostNode != NULL);
}

//----------------------------------------------------------------------------
COutpostGuildDBHandler::~COutpostGuildDBHandler()
{
}

//----------------------------------------------------------------------------
//void COutpostGuildDBHandler::setProp(const std::string & name, sint64 val)
//{
//	ICDBStructNode::CTextId txtId(name);
//	ICDBStructNode * node = _GuildDBOutpostNode->getNode(txtId, false);
//	nlassert(node != NULL);
//	nlverify( _GuildDB->setProp(node, val) );
//}
//
////----------------------------------------------------------------------------
//void COutpostGuildDBHandler::setPropString(const std::string & name, const ucstring &val)
//{
//	ICDBStructNode::CTextId txtId(name);
//	ICDBStructNode * node = _GuildDBOutpostNode->getNode(txtId, false);
//	nlassert(node != NULL);
//	nlverify( _GuildDB->setPropString(node, val) );
//}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setOWNED(bool val)
{
//	setProp("OWNED", val);
	_GuildDBOutpostNode.setOWNED(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setSHEET(const NLMISC::CSheetId val)
{
//	setProp("SHEET", val);
	_GuildDBOutpostNode.setSHEET(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setLEVEL(uint8 val)
{
//	setProp("LEVEL", val);
	_GuildDBOutpostNode.setLEVEL(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setGUILD__NAME(const ucstring &val)
{
//	setPropString("GUILD:NAME", val);
	_GuildDBOutpostNode.getGUILD().setNAME(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setGUILD__ICON(uint64 val)
{
//	setProp("GUILD:ICON", val);
	_GuildDBOutpostNode.getGUILD().setICON(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setGUILD__TRIBE(bool val)
{
//	setProp("GUILD:TRIBE", val);
	_GuildDBOutpostNode.getGUILD().setTRIBE(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setGUILD__NAME_ATT(const ucstring &val)
{
//	setPropString("GUILD:NAME_ATT", val);
	_GuildDBOutpostNode.getGUILD().setNAME_ATT(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setSTATUS(uint8 val)
{
//	setProp("STATUS", val);
	_GuildDBOutpostNode.setSTATUS(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setSTATE_END_DATE(uint32 val)
{
//	setProp("STATE_END_DATE", val);
	_GuildDBOutpostNode.setSTATE_END_DATE(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setDISPLAY_CRASH(bool val)
{
//	setProp("DISPLAY_CRASH", val);
	_GuildDBOutpostNode.setDISPLAY_CRASH(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setWARCOST(uint32 val)
{
//	setProp("WARCOST", val);
	_GuildDBOutpostNode.setWARCOST(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setROUND_LVL_THRESHOLD(uint8 val)
{
//	setProp("ROUND_LVL_THRESHOLD", val);
	_GuildDBOutpostNode.setROUND_LVL_THRESHOLD(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setROUND_LVL_MAX_ATT(uint8 val)
{
//	setProp("ROUND_LVL_MAX_ATT", val);
	_GuildDBOutpostNode.setROUND_LVL_MAX_ATT(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setROUND_LVL_MAX_DEF(uint8 val)
{
//	setProp("ROUND_LVL_MAX_DEF", val);
	_GuildDBOutpostNode.setROUND_LVL_MAX_DEF(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setROUND_LVL_CUR(uint8 val)
{
//	setProp("ROUND_LVL_CUR", val);
	_GuildDBOutpostNode.setROUND_LVL_CUR(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setROUND_ID_CUR(uint8 val)
{
//	setProp("ROUND_ID_CUR", val);
	_GuildDBOutpostNode.setROUND_ID_CUR(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setROUND_ID_MAX(uint8 val)
{
//	setProp("ROUND_ID_MAX", val);
	_GuildDBOutpostNode.setROUND_ID_MAX(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setTIME_RANGE_DEF_WANTED(uint8 val)
{
//	setProp("TIME_RANGE_DEF_WANTED", val);
	_GuildDBOutpostNode.setTIME_RANGE_DEF_WANTED(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setTIME_RANGE_DEF(uint32 val)
{
//	setProp("TIME_RANGE_DEF", val);
	_GuildDBOutpostNode.setTIME_RANGE_DEF(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setTIME_RANGE_ATT(uint32 val)
{
//	setProp("TIME_RANGE_ATT", val);
	_GuildDBOutpostNode.setTIME_RANGE_ATT(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setTIME_RANGE_LENGTH(uint32 val)
{
//	setProp("TIME_RANGE_LENGTH", val);
	_GuildDBOutpostNode.setTIME_RANGE_LENGTH(*_GuildDB, uint16(val));
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setSQUAD_CAPITAL(uint32 val)
{
//	setProp("SQUAD_CAPITAL", val);
	_GuildDBOutpostNode.setSQUAD_CAPITAL(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setSQUAD_SPAWN_ZONE__X(uint32 index, uint32 val)
{
//	std::string path = NLMISC::toString("SQUAD_SPAWN_ZONE:%d:X", index);
//	setProp(path, val);
	_GuildDBOutpostNode.getSQUAD_SPAWN_ZONE().getArray(index).setX(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setSQUAD_SPAWN_ZONE__Y(uint32 index, uint32 val)
{
//	std::string path = NLMISC::toString("SQUAD_SPAWN_ZONE:%d:Y", index);
//	setProp(path, val);
	_GuildDBOutpostNode.getSQUAD_SPAWN_ZONE().getArray(index).setY(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setSQUAD_SHOP__SHEET(uint32 index, const CSheetId &val)
{
//	std::string path = NLMISC::toString("SQUAD_SHOP:%d:SHEET", index);
//	setProp(path, val);
	_GuildDBOutpostNode.getSQUAD_SHOP().getArray(index).setSHEET(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setSQUAD_SPAWNED__SHEET(uint32 index, const CSheetId &val)
{
//	std::string path = NLMISC::toString("S:S%d:SHEET", index);
//	setProp(path, val);
	_GuildDBOutpostNode.getSQUADS().getSP(index).setSHEET(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setSQUAD_TRAINING__SHEET(uint32 index, const CSheetId &val)
{
//	std::string path = NLMISC::toString("S:T%d:SHEET", index);
//	setProp(path, val);
	_GuildDBOutpostNode.getSQUADS().getT(index).setSHEET(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setSQUAD_TRAINING__SPAWN(uint32 index, uint8 val)
{
//	std::string path = NLMISC::toString("S:T%d:SPAWN", index);
//	setProp(path, val);
	_GuildDBOutpostNode.getSQUADS().getT(index).setSPAWN(*_GuildDB, val);
}

//----------------------------------------------------------------------------
void COutpostGuildDBHandler::setBUILDINGS__SHEET(uint32 index, const CSheetId &val)
{
//	std::string path = NLMISC::toString("BUILDINGS:%d:SHEET", index);
//	setProp(path, val);
	_GuildDBOutpostNode.getBUILDINGS().getArray(index).setSHEET(*_GuildDB, val);
}


//----------------------------------------------------------------------------
// methods COutpostGuildDBUpdater
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
COutpostGuildDBUpdater::COutpostGuildDBUpdater(CCDBGroup * guildDB, uint32 outpostIndex, const COutpost * outpost, OUTPOSTENUMS::TPVPSide side)
: COutpostGuildDBHandler(guildDB, outpostIndex), _Outpost(outpost), _Side(side)
{
	nlassert(outpost != NULL);
	nlassert(side == OUTPOSTENUMS::OutpostOwner || side == OUTPOSTENUMS::OutpostAttacker);
}

//----------------------------------------------------------------------------
void COutpostGuildDBUpdater::updateOutpostGuildDB(TDBPropSet dbPropSet)
{
	H_AUTO(COutpostGuildDBUpdater_updateOutpostGuildDB);

	if (dbPropSet & OUTPOST_PROPERTIES)
		updateOutpostGuildDBOUTPOST_PROPERTIES();
	if (dbPropSet & STATE_END_DATE)
		updateOutpostGuildDBSTATE_END_DATE();
	if (dbPropSet & SQUAD_SPAWN_ZONE)
		updateOutpostGuildDBSQUAD_SPAWN_ZONE();
	if (dbPropSet & SQUAD_SHOP)
		updateOutpostGuildDBSQUAD_SHOP();
	if (dbPropSet & SQUAD_SPAWNED)
		updateOutpostGuildDBSQUAD_SPAWNED();
	if (dbPropSet & SQUAD_TRAINING)
		updateOutpostGuildDBSQUAD_TRAINING();
	if (dbPropSet & BUILDINGS)
		updateOutpostGuildDBBUILDINGS();
}

//----------------------------------------------------------------------------
void COutpostGuildDBUpdater::updateOutpostGuildDBOUTPOST_PROPERTIES()
{
	CGuild* owner = CGuildManager::getInstance()->getGuildFromId(_Outpost->_OwnerGuildId);
	CGuild* attacker = CGuildManager::getInstance()->getGuildFromId(_Outpost->_AttackerGuildId);
//	uint32 const OWNED					= (_Side == OUTPOSTENUMS::OutpostOwner)?1:0;
//	uint32 const SHEET					= _Outpost->_Sheet.asInt();
//	uint32 const LEVEL					= _Outpost->getStaticForm()?_Outpost->getStaticForm()->Level:0;
	ucstring const GUILD_NAME				= owner?owner->getName():ucstring();
	uint64 const GUILD_ICON				= owner?owner->getIcon():0;
//	uint32 const TRIBE					= _Outpost->isBelongingToAGuild()?0:1;
	ucstring const GUILD_NAME_ATT			= attacker?attacker->getName():ucstring();
//	uint32 const STATUS					= _Outpost->computeStatusForClient();
//	uint32 const DISPLAY_CRASH			= _Outpost->_CrashHappened?1:0;
	uint32 const WARCOST				= _Outpost->_Form->ChallengeCost;
	uint32 const ROUND_LVL_THRESHOLD	= _Outpost->_CurrentOutpostLevel;
	uint32 const ROUND_LVL_MAX_ATT		= _Outpost->_FightData._MaxAttackLevel;
	uint32 const ROUND_LVL_MAX_DEF		= _Outpost->_FightData._MaxDefenseLevel;
	uint32 const ROUND_LVL_CUR			= _Outpost->_FightData._CurrentCombatLevel;
	uint32 const ROUND_ID_CUR			= _Outpost->_FightData._CurrentCombatRound;
	uint32 const ROUND_ID_MAX			= _Outpost->computeRoundCount();
	uint32 const TIME_RANGE_DEF_WANTED	= _Outpost->_DefenseHour;
	uint32 const TIME_RANGE_DEF			= _Outpost->computeTimeRangeDefForClient();
	uint32 const TIME_RANGE_ATT			= _Outpost->computeTimeRangeAttForClient();
	uint32 const TIME_RANGE_LENGTH		= _Outpost->computeTimeRangeLengthForClient();
	uint32 const SQUAD_CAPITAL			= (_Side == OUTPOSTENUMS::OutpostOwner) ? _Outpost->_OwnerExpenseLimit : _Outpost->_AttackerExpenseLimit;
	
	setOWNED(_Side == OUTPOSTENUMS::OutpostOwner);
	
	setSHEET(_Outpost->_Sheet);
	setLEVEL(_Outpost->getStaticForm()?_Outpost->getStaticForm()->Level:0);
	setGUILD__NAME(GUILD_NAME);
	setGUILD__ICON(GUILD_ICON);
	setGUILD__TRIBE(!_Outpost->isBelongingToAGuild());
	setGUILD__NAME_ATT(GUILD_NAME_ATT);
	setSTATUS(_Outpost->computeStatusForClient());
	setDISPLAY_CRASH(_Outpost->_CrashHappened);
	setWARCOST(WARCOST);
	
	setROUND_LVL_THRESHOLD(uint8(ROUND_LVL_THRESHOLD));
	setROUND_LVL_MAX_ATT(uint8(ROUND_LVL_MAX_ATT));
	setROUND_LVL_MAX_DEF(uint8(ROUND_LVL_MAX_DEF));
	setROUND_LVL_CUR(uint8(ROUND_LVL_CUR));
	setROUND_ID_CUR(uint8(ROUND_ID_CUR));
	setROUND_ID_MAX(uint8(ROUND_ID_MAX));
	
	setTIME_RANGE_DEF_WANTED(uint8(TIME_RANGE_DEF_WANTED));
	setTIME_RANGE_DEF(TIME_RANGE_DEF);
	setTIME_RANGE_ATT(TIME_RANGE_ATT);
	setTIME_RANGE_LENGTH(TIME_RANGE_LENGTH);

	setSQUAD_CAPITAL(SQUAD_CAPITAL);
}

//----------------------------------------------------------------------------
void COutpostGuildDBUpdater::updateOutpostGuildDBSTATE_END_DATE()
{
	uint32 const STATE_END_DATE = _Outpost->computeStateEndDateTickForClient();

	setSTATE_END_DATE(STATE_END_DATE);
}

//----------------------------------------------------------------------------
void COutpostGuildDBUpdater::updateOutpostGuildDBSQUAD_SPAWN_ZONE()
{
	if (_Outpost->_SpawnZones.size() > OUTPOSTENUMS::OUTPOST_MAX_SPAWN_ZONE)
	{
		OUTPOST_WRN("too many spawn zones for client database : %u > %u", _Outpost->_SpawnZones.size(), OUTPOSTENUMS::OUTPOST_MAX_SPAWN_ZONE);
		DEBUG_STOP;
		return;
	}
	for (uint i = 0; i < OUTPOSTENUMS::OUTPOST_MAX_SPAWN_ZONE; ++i)
	{
		if (i < _Outpost->_SpawnZones.size())
		{
			setSQUAD_SPAWN_ZONE__X(i, sint32(_Outpost->_SpawnZones[i].getCenter().x));
			setSQUAD_SPAWN_ZONE__Y(i, sint32(_Outpost->_SpawnZones[i].getCenter().y));
		}
		else
		{
			setSQUAD_SPAWN_ZONE__X(i, 0);
			setSQUAD_SPAWN_ZONE__Y(i, 0);
		}
	}
}

//----------------------------------------------------------------------------
void COutpostGuildDBUpdater::updateOutpostGuildDBSQUAD_SHOP()
{
	if (_Outpost->_DefaultSquads.size() + _Outpost->_BuyableSquads.size() > OUTPOSTENUMS::OUTPOST_MAX_SQUAD_SHOP)
	{
		OUTPOST_WRN("too many squads in shop for client database : (%u default squads + %u buyable squads) > %u",
			_Outpost->_DefaultSquads.size(),
			_Outpost->_BuyableSquads.size(),
			OUTPOSTENUMS::OUTPOST_MAX_SQUAD_SHOP
			);
		DEBUG_STOP;
		return;
	}

	uint32 squadIndex = 0;
	for (uint i = 0; i < _Outpost->_DefaultSquads.size(); ++i, ++squadIndex)
	{
		setSQUAD_SHOP__SHEET(squadIndex, _Outpost->_DefaultSquads[i].sheet());
	}
	for (uint i = 0; i < _Outpost->_BuyableSquads.size(); ++i, ++squadIndex)
	{
		setSQUAD_SHOP__SHEET(squadIndex, _Outpost->_BuyableSquads[i].sheet());
	}
	while (squadIndex < OUTPOSTENUMS::OUTPOST_MAX_SQUAD_SHOP)
	{
		setSQUAD_SHOP__SHEET(squadIndex, CSheetId::Unknown);
		++squadIndex;
	}
}

//----------------------------------------------------------------------------
void COutpostGuildDBUpdater::updateOutpostGuildDBSQUAD_SPAWNED()
{
	nlassert(_Outpost->_CurrentSquadsA.size() <= OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	nlassert(_Outpost->_CurrentSquadsB.size() <= OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);

	if (	(_Outpost->getState() == OUTPOSTENUMS::AttackRound && _Side == OUTPOSTENUMS::OutpostOwner)
		||	(_Outpost->getState() == OUTPOSTENUMS::DefenseRound && _Side == OUTPOSTENUMS::OutpostAttacker))
	{
		// the guild can see its own spawned squads
		uint32 squadIndex = 0;
		for (uint i = 0; i < OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS; ++i, ++squadIndex)
		{
			if (i < _Outpost->_CurrentSquadsA.size())
			{
				const COutpostSquadPtr & squad = _Outpost->_CurrentSquadsA[i];
				if (squad != NULL && squad->isSpawned())
				{
					setSQUAD_SPAWNED__SHEET(squadIndex, squad->getSheet());
					continue;
				}
			}
			setSQUAD_SPAWNED__SHEET(squadIndex, CSheetId::Unknown);
		}
		for (uint i = 0; i < OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS; ++i, ++squadIndex)
		{
			if (i < _Outpost->_CurrentSquadsB.size())
			{
				const COutpostSquadPtr & squad = _Outpost->_CurrentSquadsB[i];
				if (squad != NULL && squad->isSpawned())
				{
					setSQUAD_SPAWNED__SHEET(squadIndex, squad->getSheet());
					continue;
				}
			}
			setSQUAD_SPAWNED__SHEET(squadIndex, CSheetId::Unknown);
		}
	}
	else
	{
		// the guild cannot see the opponent spawned squads
		for (uint squadIndex = 0; squadIndex < OUTPOSTENUMS::OUTPOST_MAX_SQUAD_SPAWNED; ++squadIndex)
		{
			setSQUAD_SPAWNED__SHEET(squadIndex, CSheetId::Unknown);
		}
	}
}

//----------------------------------------------------------------------------
void COutpostGuildDBUpdater::updateOutpostGuildDBSQUAD_TRAINING()
{
	const vector<COutpostSquadData> * squadsA;
	const vector<COutpostSquadData> * squadsB;
	if (_Side == OUTPOSTENUMS::OutpostOwner)
	{
		squadsA = &_Outpost->_NextAttackSquadsA;
		squadsB = &_Outpost->_NextAttackSquadsB;
	}
	else
	{
		squadsA = &_Outpost->_NextDefenseSquadsA;
		squadsB = &_Outpost->_NextDefenseSquadsB;
	}

	nlassert(squadsA->size() + squadsB->size() == OUTPOSTENUMS::OUTPOST_MAX_SQUAD_TRAINING);
	uint32 squadIndex = 0;
	for (uint i = 0; i < squadsA->size(); ++i, ++squadIndex)
	{
		const COutpostSquadData & squadData = (*squadsA)[i];
		uint32 spawnZoneIndex;
		if (!_Outpost->getSpawnZoneIndex(squadData.getSpawnZone(), spawnZoneIndex))
		{
			DEBUG_STOP;
			setSQUAD_TRAINING__SHEET(squadIndex, CSheetId::Unknown);
			setSQUAD_TRAINING__SPAWN(squadIndex, 0);
			continue;
		}
		setSQUAD_TRAINING__SHEET(squadIndex, squadData.getSquadDescriptor().sheet());
		setSQUAD_TRAINING__SPAWN(squadIndex, uint8(spawnZoneIndex));
	}
	for (uint i = 0; i < squadsB->size(); ++i, ++squadIndex)
	{
		const COutpostSquadData & squadData = (*squadsB)[i];
		uint32 spawnZoneIndex;
		if (!_Outpost->getSpawnZoneIndex(squadData.getSpawnZone(), spawnZoneIndex))
		{
			DEBUG_STOP;
			setSQUAD_TRAINING__SHEET(squadIndex, CSheetId::Unknown);
			setSQUAD_TRAINING__SPAWN(squadIndex, 0);
			continue;
		}
		setSQUAD_TRAINING__SHEET(squadIndex, squadData.getSquadDescriptor().sheet());
		setSQUAD_TRAINING__SPAWN(squadIndex, uint8(spawnZoneIndex));
	}
}

//----------------------------------------------------------------------------
void COutpostGuildDBUpdater::updateOutpostGuildDBBUILDINGS()
{
	if (_Outpost->_Buildings.size() > OUTPOSTENUMS::OUTPOST_MAX_BUILDINGS)
	{
		OUTPOST_WRN("too many buildings for client database : %u > %u", _Outpost->_Buildings.size(), OUTPOSTENUMS::OUTPOST_MAX_BUILDINGS);
		DEBUG_STOP;
		return;
	}

	for (uint i = 0; i < OUTPOSTENUMS::OUTPOST_MAX_BUILDINGS; ++i)
	{
		if (i < _Outpost->_Buildings.size())
			setBUILDINGS__SHEET(i, _Outpost->_Buildings[i].getCurrentSheet());
		else
			setBUILDINGS__SHEET(i, CSheetId::Unknown);
	}
}


//----------------------------------------------------------------------------
// methods COutpostGuildDBEraser
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
COutpostGuildDBEraser::COutpostGuildDBEraser(CCDBGroup * guildDB, uint32 outpostIndex)
: COutpostGuildDBHandler(guildDB, outpostIndex)
{
}

//----------------------------------------------------------------------------
void COutpostGuildDBEraser::clearOutpostGuildDB()
{
	H_AUTO(COutpostGuildDBEraser_clearOutpostGuildDB);

	uint32 const OWNED					= 0;
	uint32 const SHEET					= 0;
	uint32 const LEVEL					= 0;
	uint32 const GUILD_NAME				= 0;
	uint64 const GUILD_ICON				= 0;
	uint32 const TRIBE					= 0;
	uint32 const STATUS					= 0;
	uint32 const STATE_END_DATE			= 0;
	uint32 const DISPLAY_CRASH			= 0;
	uint32 const WARCOST				= 0;
	uint32 const ROUND_LVL_THRESHOLD	= 0;
	uint32 const ROUND_LVL_MAX_ATT		= 0;
	uint32 const ROUND_LVL_MAX_DEF		= 0;
	uint32 const ROUND_LVL_CUR			= 0;
	uint32 const ROUND_ID_CUR			= 0;
	uint32 const ROUND_ID_MAX			= 0;
	uint32 const TIME_RANGE_DEF_WANTED	= 0;
	uint32 const TIME_RANGE_DEF			= 0;
	uint32 const TIME_RANGE_ATT			= 0;
	uint32 const TIME_RANGE_LENGTH		= 0;
	uint32 const SQUAD_CAPITAL			= 0;
	
	setOWNED(OWNED);
	
	setSHEET(CSheetId::Unknown);
	
	setLEVEL(LEVEL);
	setSTATUS(STATUS);
	setSTATE_END_DATE(STATE_END_DATE);
	setDISPLAY_CRASH(DISPLAY_CRASH);
	setWARCOST(WARCOST);
	
	setROUND_LVL_THRESHOLD(ROUND_LVL_THRESHOLD);
	setROUND_LVL_MAX_ATT(ROUND_LVL_MAX_ATT);
	setROUND_LVL_MAX_DEF(ROUND_LVL_MAX_DEF);
	setROUND_LVL_CUR(ROUND_LVL_CUR);
	setROUND_ID_CUR(ROUND_ID_CUR);
	setROUND_ID_MAX(ROUND_ID_MAX);
	
	setTIME_RANGE_DEF_WANTED(TIME_RANGE_DEF_WANTED);
	setTIME_RANGE_DEF(TIME_RANGE_DEF);
	setTIME_RANGE_ATT(TIME_RANGE_ATT);
	setTIME_RANGE_LENGTH(TIME_RANGE_LENGTH);

	setSQUAD_CAPITAL(SQUAD_CAPITAL);
	
	for (uint i=0; i<OUTPOSTENUMS::OUTPOST_MAX_SPAWN_ZONE; ++i)
	{
		setSQUAD_SPAWN_ZONE__X(i, 0);
		setSQUAD_SPAWN_ZONE__Y(i, 0);
	}

	for (uint i=0; i<OUTPOSTENUMS::OUTPOST_MAX_SQUAD_SHOP; ++i)
	{
		setSQUAD_SHOP__SHEET(i, CSheetId::Unknown);
	}

	for (uint i=0; i<OUTPOSTENUMS::OUTPOST_MAX_SQUAD_SPAWNED; ++i)
	{
		setSQUAD_SPAWNED__SHEET(i, CSheetId::Unknown);
	}

	for (uint i=0; i<OUTPOSTENUMS::OUTPOST_MAX_SQUAD_TRAINING; ++i)
	{
		setSQUAD_TRAINING__SHEET(i, CSheetId::Unknown);
		setSQUAD_TRAINING__SPAWN(i, 0);
	}

	for (uint i=0; i<OUTPOSTENUMS::OUTPOST_MAX_BUILDINGS; ++i)
	{
		setBUILDINGS__SHEET(i, CSheetId::Unknown);
	}
}

