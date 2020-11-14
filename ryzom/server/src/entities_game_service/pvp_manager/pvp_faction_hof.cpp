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

#include "pvp_manager/pvp_faction_hof.h"
#include "player_manager/character.h"
#include "stat_db.h"

#include <time.h>

using namespace std;
using namespace NLMISC;

CPVPFactionHOF * CPVPFactionHOF::_Instance = NULL;
sint32 CPVPFactionHOF::lastDayOfMonthHOFDone = 0;

const std::string CPVPFactionHOF::_dailyString("daily");
const std::string CPVPFactionHOF::_weeklyString("weekly");
const std::string CPVPFactionHOF::_monthlyString("monthly");
const std::string CPVPFactionHOF::_globalString("global");

const std::string CPVPFactionHOF::_FactionPointString("faction_point");
const std::string CPVPFactionHOF::_KillString("kill");
const std::string CPVPFactionHOF::_FinalBlowString("final_blow");
const std::string CPVPFactionHOF::_LostString("lost");
const std::string CPVPFactionHOF::_BuildedSpire("builded_spire");
const std::string CPVPFactionHOF::_DestroyedSpire("destroyed_spire");

NL_INSTANCE_COUNTER_IMPL(CPVPFactionHOF);

//----------------------------------------------------------------------------
void CPVPFactionHOF::init()
{
	BOMB_IF( _Instance != 0, "CPVPFactionHOF already allocated", return );
	_Instance = new CPVPFactionHOF();
	BOMB_IF( _Instance == 0, "Can't allocate CPVPFactionHOF singleton", nlstop );

	// create HOF database, if already exist this do nothing
	// add static path for hall of fame database (faction part of path must be faction in war dependent, TODO later)
	_Instance->createDailyHOFDatabase();
	_Instance->createWeeklyHOFDatabase();
	_Instance->createMonthlyHOFDatabase();
	_Instance->createGlobalHOFDatabase();
	CStatDB::getInstance()->createValue("pvp_faction.LastDayOfMonthHOFDone", 0);
	CStatDB::getInstance()->valueGet( "pvp_faction.LastDayOfMonthHOFDone", lastDayOfMonthHOFDone );
}

//----------------------------------------------------------------------------
void CPVPFactionHOF::release()
{
	if( _Instance != 0 )
	{
		delete _Instance;
		_Instance = 0;
	}
}

//----------------------------------------------------------------------------
CPVPFactionHOF * CPVPFactionHOF::getInstance()
{
	if(CPVPFactionHOF::_Instance == 0)
	{
		CPVPFactionHOF::init();
	}
	return CPVPFactionHOF::_Instance;
}

//----------------------------------------------------------------------------
void CPVPFactionHOF::tickUpdate()
{
	time_t currentTime = time( NULL );
	struct tm *localTime = localtime( &currentTime );

	if( localTime->tm_mday != lastDayOfMonthHOFDone )
	{
		lastDayOfMonthHOFDone = localTime->tm_mday;
		CStatDB::getInstance()->valueSet( "pvp_faction.LastDayOfMonthHOFDone", lastDayOfMonthHOFDone );

		clearDailyHOFDatabase();

		if( localTime->tm_wday == 0 )
			clearWeeklyHOFDatabase();

		if( localTime->tm_mday == 1)
			clearMonthlyHOFDatabase();
	}
}

//----------------------------------------------------------------------------
const std::string& CPVPFactionHOF::getPeriodStatString( TPeriodStat period ) const
{
	switch( period )
	{
	case daily:
		return _dailyString;
	case weekly:
		return _weeklyString;
	case monthly:
		return _monthlyString;
	case global:
		return _globalString;
	default:
		nlstop;
	}
	// only for avoid stupid compiler warning
	return _dailyString;
}

//----------------------------------------------------------------------------
const std::string& CPVPFactionHOF::getStatString( THOFStat stat ) const
{
	switch( stat )
	{
	case faction_point:
		return _FactionPointString;
	case kill:
		return _KillString;
	case final_blow:
		return _FinalBlowString;
	case lost:
		return _LostString;
	case builded_spire:
		return _BuildedSpire;
	case destroyed_spire:
		return _DestroyedSpire;
	default:
		nlstop;
	}
	// only for avoid stupid compiler warning
	return _FactionPointString;
}

//----------------------------------------------------------------------------
void CPVPFactionHOF::writeStatInHOFDatabase( CCharacter * pc, PVP_CLAN::TPVPClan clan, CPVPFactionHOF::THOFStat stat, sint32 value ) const
{
	string sdbPvPPath = PVP_CLAN::toLowerString(clan) + "." + getStatString(stat);
	string sdbPvPPathHeader = "pvp_faction.daily.";
	setHallOfFame(sdbPvPPathHeader+sdbPvPPath, pc, value, stat);
	if( stat == CPVPFactionHOF::kill || stat == CPVPFactionHOF::lost )
	{
		setHallOfFameKillDeathRatio( sdbPvPPathHeader + PVP_CLAN::toLowerString(clan), pc );
	}

	sdbPvPPathHeader = "pvp_faction.weekly.";
	setHallOfFame(sdbPvPPathHeader+sdbPvPPath, pc, value, stat);
	if( stat == CPVPFactionHOF::kill || stat == CPVPFactionHOF::lost )
	{
		setHallOfFameKillDeathRatio( sdbPvPPathHeader + PVP_CLAN::toLowerString(clan), pc );
	}

	sdbPvPPathHeader = "pvp_faction.monthly.";
	setHallOfFame(sdbPvPPathHeader+sdbPvPPath, pc, value, stat);
	if( stat == CPVPFactionHOF::kill || stat == CPVPFactionHOF::lost )
	{
		setHallOfFameKillDeathRatio( sdbPvPPathHeader + PVP_CLAN::toLowerString(clan), pc );
	}

	sdbPvPPathHeader = "pvp_faction.global.";
	setHallOfFame(sdbPvPPathHeader+sdbPvPPath, pc, value, stat);
	if( stat == CPVPFactionHOF::kill || stat == CPVPFactionHOF::lost )
	{
		setHallOfFameKillDeathRatio( sdbPvPPathHeader + PVP_CLAN::toLowerString(clan), pc );
	}
}

//----------------------------------------------------------------------------
void CPVPFactionHOF::setHallOfFame( string sdbPvPPath, CCharacter * pc, sint32 value, CPVPFactionHOF::THOFStat stat ) const
{
	switch( stat )
	{
	case CPVPFactionHOF::builded_spire:
	case CPVPFactionHOF::destroyed_spire:
		CStatDB::getInstance()->valueAdd( sdbPvPPath, value );
		break;
	default:
		CStatDB::getInstance()->tablePlayerAdd( sdbPvPPath, pc->getId(), value );
		if (pc->getGuildId() != 0)
		{
			CStatDB::getInstance()->tableGuildAdd(sdbPvPPath, pc->getGuildId(), value);
		}
		break;
	}
}

//----------------------------------------------------------------------------
void CPVPFactionHOF::setHallOfFameKillDeathRatio( string sdbPvPPath, CCharacter * pc ) const
{
	sint32 killValue;
	sint32 lostValue;
	if (!CStatDB::getInstance()->tablePlayerGet( sdbPvPPath + ".kill", pc->getId(), killValue ))
		killValue = 0;
	if (!CStatDB::getInstance()->tablePlayerGet( sdbPvPPath + ".lost", pc->getId(), lostValue ))
		lostValue = 0;

	CStatDB::getInstance()->tablePlayerSet( sdbPvPPath + ".kill_lost", pc->getId(), killValue * 100 / max(lostValue,sint32(1)) );
	if (pc->getGuildId() != 0)
	{
		if (!CStatDB::getInstance()->tableGuildGet( sdbPvPPath + ".kill", pc->getGuildId(), killValue ))
			killValue = 0;
		if (!CStatDB::getInstance()->tableGuildGet( sdbPvPPath + ".lost", pc->getGuildId(), lostValue ))
			lostValue = 0;

		CStatDB::getInstance()->tableGuildSet( sdbPvPPath + ".kill_lost", pc->getGuildId(), killValue * 100 / max(lostValue,sint32(1)) );
	}
}

//----------------------------------------------------------------------------
void CPVPFactionHOF::clearDailyHOFDatabase()
{
	CStatDB::getInstance()->removeNode("pvp_faction.daily", false);
	createDailyHOFDatabase();
}

//----------------------------------------------------------------------------
void CPVPFactionHOF::clearWeeklyHOFDatabase()
{
	CStatDB::getInstance()->removeNode("pvp_faction.weekly", false);
	createWeeklyHOFDatabase();
}

//----------------------------------------------------------------------------
void CPVPFactionHOF::clearMonthlyHOFDatabase()
{
	CStatDB::getInstance()->removeNode("pvp_faction.monthly", false);
	createMonthlyHOFDatabase();
}

//----------------------------------------------------------------------------
void CPVPFactionHOF::createDailyHOFDatabase()
{
	CStatDB::getInstance()->createTable("pvp_faction.daily.kami.faction_point");
	CStatDB::getInstance()->createTable("pvp_faction.daily.kami.kill");
	CStatDB::getInstance()->createTable("pvp_faction.daily.kami.final_blow");
	CStatDB::getInstance()->createTable("pvp_faction.daily.kami.lost");
	CStatDB::getInstance()->createValue("pvp_faction.daily.kami.builded_spire", 0);
	CStatDB::getInstance()->createValue("pvp_faction.daily.kami.destroyed_spire", 0);
	CStatDB::getInstance()->createTable("pvp_faction.daily.kami.kill_lost");

	CStatDB::getInstance()->createTable("pvp_faction.daily.karavan.faction_point");
	CStatDB::getInstance()->createTable("pvp_faction.daily.karavan.kill");
	CStatDB::getInstance()->createTable("pvp_faction.daily.karavan.final_blow");
	CStatDB::getInstance()->createTable("pvp_faction.daily.karavan.lost");
	CStatDB::getInstance()->createValue("pvp_faction.daily.karavan.builded_spire", 0);
	CStatDB::getInstance()->createValue("pvp_faction.daily.karavan.destroyed_spire", 0);
	CStatDB::getInstance()->createTable("pvp_faction.daily.karavan.kill_loast");
}

//----------------------------------------------------------------------------
void CPVPFactionHOF::createWeeklyHOFDatabase()
{
	CStatDB::getInstance()->createTable("pvp_faction.weekly.kami.faction_point");
	CStatDB::getInstance()->createTable("pvp_faction.weekly.kami.kill");
	CStatDB::getInstance()->createTable("pvp_faction.weekly.kami.final_blow");
	CStatDB::getInstance()->createTable("pvp_faction.weekly.kami.lost");
	CStatDB::getInstance()->createValue("pvp_faction.weekly.kami.builded_spire", 0);
	CStatDB::getInstance()->createValue("pvp_faction.weekly.kami.destroyed_spire", 0);
	CStatDB::getInstance()->createTable("pvp_faction.weekly.kami.kill_lost");

	CStatDB::getInstance()->createTable("pvp_faction.weekly.karavan.faction_point");
	CStatDB::getInstance()->createTable("pvp_faction.weekly.karavan.kill");
	CStatDB::getInstance()->createTable("pvp_faction.weekly.karavan.final_blow");
	CStatDB::getInstance()->createTable("pvp_faction.weekly.karavan.lost");
	CStatDB::getInstance()->createValue("pvp_faction.weekly.karavan.builded_spire", 0);
	CStatDB::getInstance()->createValue("pvp_faction.weekly.karavan.destroyed_spire", 0);
	CStatDB::getInstance()->createTable("pvp_faction.weekly.karavan.kill_lost");
}

//----------------------------------------------------------------------------
void CPVPFactionHOF::createMonthlyHOFDatabase()
{
	CStatDB::getInstance()->createTable("pvp_faction.monthly.kami.faction_point");
	CStatDB::getInstance()->createTable("pvp_faction.monthly.kami.kill");
	CStatDB::getInstance()->createTable("pvp_faction.monthly.kami.final_blow");
	CStatDB::getInstance()->createTable("pvp_faction.monthly.kami.lost");
	CStatDB::getInstance()->createValue("pvp_faction.monthly.kami.builded_spire", 0);
	CStatDB::getInstance()->createValue("pvp_faction.monthly.kami.destroyed_spire", 0);
	CStatDB::getInstance()->createTable("pvp_faction.monthly.kami.kill_lost");

	CStatDB::getInstance()->createTable("pvp_faction.monthly.karavan.faction_point");
	CStatDB::getInstance()->createTable("pvp_faction.monthly.karavan.kill");
	CStatDB::getInstance()->createTable("pvp_faction.monthly.karavan.final_blow");
	CStatDB::getInstance()->createTable("pvp_faction.monthly.karavan.lost");
	CStatDB::getInstance()->createValue("pvp_faction.monthly.karavan.builded_spire", 0);
	CStatDB::getInstance()->createValue("pvp_faction.monthly.karavan.destroyed_spire", 0);
	CStatDB::getInstance()->createTable("pvp_faction.monthly.karavan.kill_lost");
}

//----------------------------------------------------------------------------
void CPVPFactionHOF::createGlobalHOFDatabase()
{
	CStatDB::getInstance()->createTable("pvp_faction.global.kami.faction_point");
	CStatDB::getInstance()->createTable("pvp_faction.global.kami.kill");
	CStatDB::getInstance()->createTable("pvp_faction.global.kami.final_blow");
	CStatDB::getInstance()->createTable("pvp_faction.global.kami.lost");
	CStatDB::getInstance()->createValue("pvp_faction.global.kami.builded_spire", 0);
	CStatDB::getInstance()->createValue("pvp_faction.global.kami.destroyed_spire", 0);
	CStatDB::getInstance()->createTable("pvp_faction.global.kami.kill_lost");

	CStatDB::getInstance()->createTable("pvp_faction.global.karavan.faction_point");
	CStatDB::getInstance()->createTable("pvp_faction.global.karavan.kill");
	CStatDB::getInstance()->createTable("pvp_faction.global.karavan.final_blow");
	CStatDB::getInstance()->createTable("pvp_faction.global.karavan.lost");
	CStatDB::getInstance()->createValue("pvp_faction.global.karavan.builded_spire", 0);
	CStatDB::getInstance()->createValue("pvp_faction.global.karavan.destroyed_spire", 0);
	CStatDB::getInstance()->createTable("pvp_faction.global.karavan.kill_lost");
}
