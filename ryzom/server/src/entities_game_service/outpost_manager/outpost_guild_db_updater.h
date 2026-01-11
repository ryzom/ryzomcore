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

#ifndef RY_OUTPOST_GUILD_DB_UPDATER_H
#define RY_OUTPOST_GUILD_DB_UPDATER_H

#include "nel/misc/types_nl.h"
#include "game_share/outpost.h"
#include "database_guild.h"


class CCDBGroup;
class ICDBStructNode;
class COutpost;


/**
 * Base class to manipulate the outpost guild client database.
 *
 * \author Sebastien Guignot
 * \author Nevrax France
 * \date october 2005
 */
class COutpostGuildDBHandler : public NLMISC::CRefCount
{
protected:
	/// protected ctor
	/// \param guildDB : the guild database
	/// \param outpostIndex : index of the outpost in the branch GUILD:OUTPOST:O
	COutpostGuildDBHandler(CCDBGroup * guildDB, uint32 outpostIndex);

	/// protected dtor
	~COutpostGuildDBHandler();

	void setOWNED					(bool val);
	void setSHEET					(const NLMISC::CSheetId val);
	void setLEVEL					(uint8 val);
	void setGUILD__NAME				(const ucstring &val);
	void setGUILD__ICON				(uint64 val);
	void setGUILD__TRIBE			(bool val);
	void setGUILD__NAME_ATT			(const ucstring &val);
	void setSTATUS					(uint8 val);
	void setSTATE_END_DATE			(uint32 val);
	void setDISPLAY_CRASH			(bool val);
	void setWARCOST					(uint32 val);
	
	void setROUND_LVL_THRESHOLD		(uint8 val);
	void setROUND_LVL_MAX_ATT		(uint8 val);
	void setROUND_LVL_MAX_DEF		(uint8 val);
	void setROUND_LVL_CUR			(uint8 val);
	void setROUND_ID_CUR			(uint8 val);
	void setROUND_ID_MAX			(uint8 val);
	
	void setTIME_RANGE_DEF_WANTED	(uint8 val);
	void setTIME_RANGE_DEF			(uint32 val);
	void setTIME_RANGE_ATT			(uint32 val);
	void setTIME_RANGE_LENGTH		(uint32 val);

	void setSQUAD_CAPITAL			(uint32 val);

	void setSQUAD_SPAWN_ZONE__X		(uint32 index, uint32 val);
	void setSQUAD_SPAWN_ZONE__Y		(uint32 index, uint32 val);

	void setSQUAD_SHOP__SHEET		(uint32 index, const NLMISC::CSheetId &val);

	void setSQUAD_SPAWNED__SHEET	(uint32 index, const NLMISC::CSheetId &val);
	void setSQUAD_TRAINING__SHEET	(uint32 index, const NLMISC::CSheetId &val);
	void setSQUAD_TRAINING__SPAWN	(uint32 index, uint8 val);

	void setBUILDINGS__SHEET		(uint32 index, const NLMISC::CSheetId &val);

private:
	/// set an outpost property in the database
//	void setProp(const std::string & name, sint64 val);
	/// set an outpost property in the database
//	void setPropString(const std::string & name, const ucstring &val);

private:
	/// guild client database
//	CCDBSynchronised *					_GuildDB;
	CCDBGroup							*_GuildDB;

	/// outpost node in the guild client database
//	ICDBStructNode *					_GuildDBOutpostNode;
	CBankAccessor_GUILD::TGUILD::TOUTPOST::TO	_GuildDBOutpostNode;
};


/**
 * This class updates the outpost guild client database.
 *
 * \author Sebastien Guignot
 * \author Nevrax France
 * \date october 2005
 */
class COutpostGuildDBUpdater : public COutpostGuildDBHandler
{
public:
	/// outpost database properties set (from GUILD:OUTPOST:O:#x: in database.xml)
	enum TDBPropSet
	{
		OUTPOST_PROPERTIES		= 1 << 0,	///< all leaves and branches but the following
		STATE_END_DATE			= 1 << 1,	///< leaf	STATE_END_DATE
		SQUAD_SPAWN_ZONE		= 1 << 2,	///< branch	SQUAD_SPAWN_ZONE
		SQUAD_SHOP				= 1 << 3,	///< branch	SQUAD_SHOP
		SQUAD_SPAWNED			= 1 << 4,	///< branch	S:S
		SQUAD_TRAINING			= 1 << 5,	///< branch	S:T
		BUILDINGS				= 1 << 6,	///< branch BUILDINGS

		ALL = ~0
	};

public:
	/// ctor
	/// \param guildDB : the guild database
	/// \param outpostIndex : index of the outpost in the branch GUILD:OUTPOST:O
	/// \param outpost : the outpost used to update the database
	/// \param side : side of the guild for the outpost (owner or attacker)
	COutpostGuildDBUpdater(CCDBGroup * guildDB, uint32 outpostIndex,
		const COutpost * outpost, OUTPOSTENUMS::TPVPSide side);

	/// update the given props of the outpost guild database
	void updateOutpostGuildDB(TDBPropSet dbPropSet = ALL);

private:
	/// update a specific part of the outpost guild database
	void updateOutpostGuildDBOUTPOST_PROPERTIES();
	void updateOutpostGuildDBSTATE_END_DATE();
	void updateOutpostGuildDBSQUAD_SPAWN_ZONE();
	void updateOutpostGuildDBSQUAD_SHOP();
	void updateOutpostGuildDBSQUAD_SPAWNED();
	void updateOutpostGuildDBSQUAD_TRAINING();
	void updateOutpostGuildDBBUILDINGS();

private:
	/// outpost used to fill the database (read only)
	NLMISC::CSmartPtr<const COutpost>	_Outpost;

	/// side of the guild for the outpost (owner or attacker)
	const OUTPOSTENUMS::TPVPSide		_Side;
};
typedef NLMISC::CSmartPtr<COutpostGuildDBUpdater>	COutpostGuildDBUpdaterPtr;


/**
 * This class clears the outpost guild client database.
 *
 * \author Sebastien Guignot
 * \author Nevrax France
 * \date october 2005
 */
class COutpostGuildDBEraser : public COutpostGuildDBHandler
{
public:
	/// ctor
	/// \param guildDB : the guild database
	/// \param outpostIndex : index of the outpost in the branch GUILD:OUTPOST:O
	COutpostGuildDBEraser(CCDBGroup * guildDB, uint32 outpostIndex);

	/// clear the outpost guild database
	void clearOutpostGuildDB();
};
typedef NLMISC::CSmartPtr<COutpostGuildDBEraser>	COutpostGuildDBEraserPtr;


static inline COutpostGuildDBUpdater::TDBPropSet operator|(COutpostGuildDBUpdater::TDBPropSet a, COutpostGuildDBUpdater::TDBPropSet b)
{
	return COutpostGuildDBUpdater::TDBPropSet(uint(a)|uint(b));
}

static inline COutpostGuildDBUpdater::TDBPropSet & operator|=(COutpostGuildDBUpdater::TDBPropSet & a, COutpostGuildDBUpdater::TDBPropSet b)
{
	a = a|b;
	return a;
}


#endif // RY_OUTPOST_GUILD_DB_UPDATER_H

/* End of outpost_guild_db_updater.h */
