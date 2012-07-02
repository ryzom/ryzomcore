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



//#include "ai_entity_id.h" // this must be BEFORE the RYAI_AI_H test

#ifndef RYAI_AI_INSTANCE_H
#define RYAI_AI_INSTANCE_H

#include "child_container.h"
#include "server_share/msg_ai_service.h"
#include "ai_entity_matrix.h"
#include "ai_entity.h"
#include "continent.h"
#include "ai_grp.h"
#include "service_dependencies.h"

#include "sheets.h"

class CBot;
class CGroup;
class CManager;

template <class T>
class CAIEntityMatrix;
class CAIEntityMatrixIteratorTblRandom;
class CAIEntityMatrixIteratorTblLinear;

class CAIEntityPhysical;
class CAIEntity;
class CMgrPet;
class CManagerPlayer;
class CGroupNpc;
class CPersistentOfPhysical;
class CManagerPlayer;
class CAIS;
class CContinent;
class CCell;
class COutpostSquadFamily;
class CBotPlayer;
class CBotEasterEgg;

class CAIInstance
: public NLMISC::CDbgRefCount<CAIInstance>
, public NLMISC::CRefCount
, public CChild<CAIS>
, public CAIEntity
, public IManagerParent
, public CServiceEvent::CHandler
{
public:
	CAIInstance(CAIS* owner);
	virtual ~CAIInstance();
	
	typedef	CHashMap<NLMISC::TStringId, NLMISC::CDbgPtr<CNpcZone>, NLMISC::CStringIdHashMapTraits> TZoneList;
	TZoneList zoneList;
	void addZone(std::string const& zoneName, CNpcZone* zone);
	void removeZone(std::string const& zoneName, CNpcZone* zone);
	CNpcZone* getZone(NLMISC::TStringId zoneName);
	// Trig Event if player in zone
	void updateZoneTrigger(CBotPlayer* player);
	
 	// overloads for IManagerParent virtuals
	CAIInstance* getAIInstance() const { return const_cast<CAIInstance*>(this); }
	CCellZone* getCellZone() { return NULL; }
	virtual std::string getIndexString() const;
	virtual std::string getOneLineInfoString() const;
	virtual std::vector<std::string> getMultiLineInfoString() const;
	
	std::string getManagerIndexString(CManager const* manager) const;
	
	void groupDead(CGroup* grp) { }
	
	void serviceEvent(CServiceEvent const& info);
	
	//-------------------------------------------------------------------
	// classic init(), update() and release()
	
	// the update routine called once per tick
	// this is the routine that calls the managers' updates
	void update();
	
	//-------------------------------------------------------------------
	// managing the set of managers

	// factory for creating new managers and for adding them to the _managers map
	CManager* newMgr(AITYPES::TMgrType type, uint32 alias, std::string const& name, std::string const& mapName, std::string const& filename);
	
	//	a method that parse a supposed know type of manager:group:bot hierarchy and return the element as CAIEntity.
	CManager* tryToGetManager(char const* str);
	CGroup* tryToGetGroup(char const* str);
	
	CAIEntity* tryToGetEntity(char const* str, CAIS::TSearchType searchType);
	
	// erase a manager (free resources, free id, etc, etc
	// asserts if the id is invalid (<0 or >1023)
	// displays a warning and returns cleanly if the id is unused
	void deleteMgr(sint mgrId);
	
	//-------------------------------------------------------------------
	//	the previous interfaces for searching the data structures for named objects are transfered in CAIEntityId 
	//	as its one of their object behavior. a solution to build id directly was added.
	CMgrPet* getPetMgr() { return _PetManager; }
	CManagerPlayer* getPlayerMgr() { return _PlayerManager; }
	
	//-------------------------------------------------------------------
	// Interface to kami management
	void registerKamiDeposit(uint32 alias, CGroupNpc* grp);
	void unregisterKamiDeposit(uint32 alias);

	//-------------------------------------------------------------------
	// Interface to the vision management matrices
	
	// read accessors for getting hold of the vision matrices and their associated iterator tables
	CAIEntityMatrix<CPersistentOfPhysical>& playerMatrix() { return _PlayerMatrix; }
	CAIEntityMatrix<CPersistentOfPhysical>& botMatrix() { return _BotMatrix; }
	
	CAliasCont<CManager>& managers() { return _Managers; }
	
	CCont<CContinent>& continents() { return _Continents; }
	CCont<CContinent> const& continents() const { return _Continents; }
	
	// Methods to retreive location in the dynamic system.
	CContinent* locateContinentForPos(CAIVector const& pos);
	CRegion*    locateRegionForPos(CAIVector const& pos);
	CCellZone*  locateCellZoneForPos(CAIVector const& pos);
	CCell*      locateCellForPos(CAIVector const& pos);
	
	
	//-------------------------------------------------------------------
	// Mission name/alias retreiver
	/** Add a mission name and alias info. If alias is already mapped to a mission name, replace the mapping.
	 *	Many alias can be mapped to the same name, but a name can only belong to one alias.
	 */
	void addMissionInfo(std::string const& missionName, uint32 alias);
	/** Return all mission alias that have a given mission name.
	 *	n log n search
	 */
	void findMissionAlias(std::vector<uint32>& result, std::string const& missionName)
	{
		std::map<std::string, std::vector<uint32> >::iterator it(_MissionToAlias.find(missionName));
		if (it != _MissionToAlias.end())
			result.insert(result.end(), it->second.begin(), it->second.end());
	}
	/**	Return the name of the mission for a given alias.
	 *	Return an empty string if no mission have this alias.
	 *	This search is not optimized, linar time search !
	 */
	std::string const& findMissionName(uint32 alias);
	
	//-------------------------------------------------------------------
	// group name/alias retreiver
	void addGroupInfo(CGroup* grp);
	void removeGroupInfo(CGroup* grp, CAliasTreeOwner* grpAliasTreeOwner);
	CGroup* findGroup(uint32 alias);
	void findGroup(std::vector<CGroup*>& result, std::string const& name);
	
	/// Time warp management. This method is called when time as warped more than 600ms
	bool advanceUserTimer(uint32 nbTicks);
	
	bool spawn();
	bool despawn();
	
	uint32 getInstanceNumber() const { return _InstanceNumber; }
	std::string const& getContinentName() const { return _ContinentName; }
	std::string& getContinentName() { return _ContinentName; }
	
	void initInstance(std::string const& continentName, uint32 instanceNumber);
	
	/// Main squad family accessor
	COutpostSquadFamily *getSquadFamily() { return _SquadFamily; }

	/// Store mapping 'name:variant' -> squad to be used later by getSquadByVariantName()
	void registerSquadVariant(const std::string& nameAndVariant, CGroupDesc<COutpostSquadFamily> *squad )
	{
		if ( ! _SquadVariantNameToGroupDesc.insert( std::make_pair( NLMISC::toLower( nameAndVariant ), squad ) ).second )
			nlwarning( "Duplicate squad template / squad variant '%s'", nameAndVariant.c_str() );
	}

	/// Clear mapping 'name:variant' -> squad when it's not useful anymore
	void clearSquadVariantNames()
	{
		_SquadVariantNameToGroupDesc.clear();
	}

	/// Get a squad by name:variant (works only during primitive parsing), or NULL if not found. Not case-sensitive.
	CGroupDesc<COutpostSquadFamily> *getSquadByVariantName(const std::string& nameAndVariant)
	{
		std::map<std::string, NLMISC::CSmartPtr< CGroupDesc<COutpostSquadFamily> > >::iterator it = _SquadVariantNameToGroupDesc.find( NLMISC::toLower( nameAndVariant ) );
		if ( it != _SquadVariantNameToGroupDesc.end() )
			return (*it).second;
		else
			return NULL;
	}

	CGroupNpc* eventCreateNpcGroup(uint nbBots, NLMISC::CSheetId const& sheetId, CAIVector const& pos, double dispersionRadius, bool spawnBots, double orientation, const std::string &botsName, const std::string &look);

	/// create a new easter egg
	CBotEasterEgg* createEasterEgg(uint32 easterEggId, NLMISC::CSheetId const& sheetId, std::string const& botName, double x, double y, double z, double heading, const std::string& look);
	/// destroy an easter egg
	void destroyEasterEgg(uint32 easterEggId);
	/// get an easter egg by ID
	CBotEasterEgg* getEasterEgg(uint32 easterEggId);

private:
	void sendInstanceInfoToEGS();
	
private:
	/// @name AI service hierarchy
	//@{
	/// The set of continents
	CCont<CContinent> _Continents;
	/// the set of managers and the service's root alias description tree node
	CAliasCont<CManager> _Managers;
	//@}
	
	/// The ai instance continent name (multi ai system)
	std::string _ContinentName;
	/// The ai instance number (multi ai system)
	uint32 _InstanceNumber;
	
	CAIEntity* tryToGetEntity(char const* str);
	
	//	we must share pets and players .. later :)
	///	pet manager
	CMgrPet* _PetManager;
	///	player Manager.
	CManagerPlayer* _PlayerManager;
	///	event npc Manager.
	CMgrNpc* _EventNpcManager;
	///	easter egg manager
	NLMISC::CRefPtr<CMgrNpc> _EasterEggManager;

	/// easter egg group
	NLMISC::CRefPtr<CGroupNpc> _EasterEggGroup;

	/// map of kami groups by alias (for kami groups associated with deposits)
	std::map<uint, CGroupNpc*> _KamiDeposits;
	
	/// matrices used for vision generation and their associated iterator tables
	CAIEntityMatrix<CPersistentOfPhysical> _PlayerMatrix;
	CAIEntityMatrix<CPersistentOfPhysical> _BotMatrix;
	
	// Mission name to alias container.
	std::map<std::string, std::vector<uint32> > _MissionToAlias;
	
	/// Group name and alias container.
	std::map<std::string, std::vector<NLMISC::CDbgPtr<CGroup> > > _GroupFromNames;
	std::map<uint32, NLMISC::CDbgPtr<CGroup> > _GroupFromAlias;

	// Container for all the squad templates
	NLMISC::CSmartPtr<COutpostSquadFamily>	_SquadFamily;

	// Squad look-up (only valid during primitive parsing)
	std::map< std::string, NLMISC::CSmartPtr< CGroupDesc<COutpostSquadFamily> > >	_SquadVariantNameToGroupDesc;
};

#endif
