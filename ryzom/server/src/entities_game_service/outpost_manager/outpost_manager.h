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

#ifndef RY_OUTPOST_MANAGER_H
#define RY_OUTPOST_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"
#include "outpost.h"

#include "cdb_group.h"
#include "outpost_manager/outpost_guild_db_updater.h"


extern NLMISC::CFileDisplayer OutpostDisplayer;
extern NLMISC::CLog OutpostDbgLog, OutpostInfLog, OutpostWrnLog, OutpostErrLog;

#define RY_OUTPOST_TEMP_LOG 1

#ifdef RY_OUTPOST_TEMP_LOG

#define OUTPOST_DBG nldebug
//#define OUTPOST_INF nlinfo
#define OUTPOST_INF nldebug
#define OUTPOST_WRN nlwarning
#define OUTPOST_ERR nlerror

#else

#define OUTPOST_DBG OutpostDbgLog.setPosition( __LINE__, __FILE__, __FUNCTION__ ), OutpostDbgLog.displayNL
#define OUTPOST_INF OutpostInfLog.setPosition( __LINE__, __FILE__, __FUNCTION__ ), OutpostInfLog.displayNL
#define OUTPOST_WRN OutpostWrnLog.setPosition( __LINE__, __FILE__, __FUNCTION__ ), OutpostWrnLog.displayNL
#define OUTPOST_ERR OutpostErrLog.setPosition( __LINE__, __FILE__, __FUNCTION__ ), OutpostErrLog.displayNL

#endif


class COutpost;
class CGuildMemberModule;
class CFileDescription;


/**
 * Singleton used to manage outpost
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class COutpostManager : public NLMISC::CSingleton<COutpostManager>
{
private:
	struct CSquadTemplate
	{
		std::string		SquadName;
		struct CSquadTemplateVariant
		{
			std::string			VariantName;
			bool				IsTribe;
			TAIAlias			Alias;
			NLMISC::CSheetId	SquadSheet;
		};
		std::vector<CSquadTemplateVariant>	Variants;
	};

public:

	COutpostManager();
	/// return the singleton instance
//	static COutpostManager & getInstance();

	/// load the outposts from primitives (static data)
	void loadOutpostPrimitives();

	/// load the outposts from save files (persistent dynamic data)
	/// WARNING: preconditions:
	/// - loadOutpostPrimitives() must have been called before
	///	- guilds must be loaded
	void loadOutpostSaveFiles();

	// callback for each file loaded from the backup service
	void outpostFileCallback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream);

	/// Check all outpost against the list of loaded guilds. If any guild is missing,
	/// update the outpost to a coherent state
	void validateOutpostOwners();

	/// update called each tick
	void tickUpdate();

	/// save all the outposts (called when EGS quits)
	void saveAll();

	/// update the outposts owned or attacked by the specified guild, because the guild is being deleted. guild must be non-null.
	void onRemoveGuild( CGuild *guild );

	/// called when a building has spawned (pBuilding must be != NULL)
	void onBuildingSpawned(CCreature *pBuilding);

	/// get an outpost from an alias
	NLMISC::CSmartPtr<COutpost> getOutpostFromAlias( TAIAlias alias );
	/// get an outpost from a sheet
	NLMISC::CSmartPtr<COutpost> getOutpostFromSheet( NLMISC::CSheetId sheet );
	/// get an outpost short id from its alias
	uint16 getOutpostShortId( TAIAlias alias );
	/// get an outpost alias from its short id
	TAIAlias getOutpostAliasFromShortId( uint16 shortId );

	/// dump the complete list of loaded outposts
	void dumpOutpostList(NLMISC::CLog & log) const;
	/// dump the given outpost
	void dumpOutpost(TAIAlias outpostAlias, NLMISC::CLog & log) const;

	/// Select the requested variant in templates, and fill the descriptor.
	/// only valid during a part of loadOutposts().
	/// Return true if a template was found.
	bool fillSquadDescriptor( const std::string& squadName, bool isTribe, const std::string& variantContext, COutpostSquadDescriptor& squadDesc );

	/// call this when an outpost needs an update in guild database
	void askOutpostGuildDBUpdate(TAIAlias outpostAlias, COutpostGuildDBUpdater::TDBPropSet dbPropSet);

	TAIAlias getOutpostFromUserPosition( CCharacter *user ) const;
	void enterOutpostZone(CCharacter* user);
	void leaveOutpostZone(CCharacter* user);

	/// called only by the command : outpostAccelerateConstruction
	void setConstructionTime(uint32 nNbSeconds);

protected:

	friend class CPlayerService;

	/// save the given outpost
	void saveOutpost(NLMISC::CSmartPtr<COutpost> outpost);

	/// report when an AIS (re)starts (to resent dynamic data for the corresponding outposts) or stops
	void onAIInstanceReadyOrDown( uint32 instanceNumber, bool startOrStop );

	/// record a squad template
	void addSquadTemplate( const CSquadTemplate& t )
	{
		_SquadTemplates.push_back( t );
	}

	/// do updates in outpost guild database asked using askOutpostGuildDBUpdate()
	void doOutpostGuildDBUpdates();

private:
	/// unique instance
//	static COutpostManager* _Instance;

	typedef std::vector<NLMISC::CSmartPtr<COutpost> > TOutposts;
	/// managed outposts vector
	TOutposts	 _Outposts;

	/// managed outposts, accessible by alias
	CHashMap<uint, NLMISC::CSmartPtr<COutpost> > _OutpostsByAlias;

	/// managed outposts, accessible by sheet
	std::map<NLMISC::CSheetId, NLMISC::CSmartPtr<COutpost> > _OutpostsBySheet;

	/// outposts that asked a database update
	std::vector<std::pair<TAIAlias,COutpostGuildDBUpdater::TDBPropSet> >	_OutpostsWaitingGuildDBUpdate;

	/// squad templates
	std::vector<CSquadTemplate>	_SquadTemplates;

	/// index in _Outposts of the next outpost to be saved
	uint32		_NextOutpostToSave;

	/// floating index in _Outposts vector of the next outpost to update
	uint32		_OutpostUpdateCursor;

	/// get a 16 bit id from outpost alias (used for VP sent to client)
	std::map<TAIAlias,uint16>	_OutpostAliasToShortId;
	/// get alias from outpost 16 bit id
	std::map<uint16,TAIAlias>	_OutpostShortIdToAlias;

	/// true if outposts have been loaded from primitives
	bool		_OutpostPrimitivesLoaded;
	/// true if outposts have been loaded from save files
	bool		_OutpostSaveFilesLoaded;

	/// AI instances that were ready before loading outposts is complete
	std::set<uint32>	_PendingReadyAIInstances;
};


#endif
