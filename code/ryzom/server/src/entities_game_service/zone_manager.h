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



#ifndef RY_ZONE_MANAGER_H
#define RY_ZONE_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"
#include "nel/ligo/primitive.h"

#include "game_share/base_types.h"
#include "game_share/continent.h"
#include "server_share/respawn_point_type.h"
#include "server_share/place_type.h"
#include "entity_manager/entity_base.h"
#include "game_share/string_manager_sender.h"
#include "mission_manager/ai_alias_translator.h"
#include "deposit.h"

class CCharacter;
extern NLMISC::CRandom RandomGenerator;

static const uint16 InvalidSpawnZoneId = 0xFFFF;
static const uint16 InvalidPlaceId = 0xFFFF;


/**
 * A teleport destination zone
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CTpSpawnZone : public NLLIGO::CPrimPoint
{
public:
	/**
	 * build the zone from a primitive file
	 * \return true on success
	 */
	bool build(const NLLIGO::CPrimPoint * point);
	/// get a random destination point
	void getRandomPoint( sint32 & x, sint32 & y, sint32 & z, float & heading )const
	{
		static const float angleMax = (float)(2 * NLMISC::Pi);
		z = (sint32)Point.z;
		float angle = RandomGenerator.frand( angleMax );
		sint32 radius = (sint32)RandomGenerator.rand( (uint16) _Radius );
		x = sint32( radius * cos ( angle ) + Point.x );
		y = sint32( radius * sin ( angle ) + Point.y );
		heading = Angle;
	}
	void getCenter( sint32 & x, sint32 & y )const
	{
		x = sint32(Point.x);
		y = sint32(Point.y);
	}
	RESPAWN_POINT::TRespawnPointType getType() const { return _Type; }

	void setContinent( CONTINENT::TContinent contId )
	{
		_Continent = contId;
	}
	CONTINENT::TContinent getContinent() const
	{
		return _Continent;
	}

	void setRegion( uint16 regionId )
	{
		_Region = regionId;
	}
	uint16 getRegion() const
	{
		return _Region;
	}

	const std::string &getName() const
	{
		return _Name;
	}

	PLACE_TYPE::TPlaceType getPlaceType() const
	{
		return _PlaceType;
	}

	void setPlaceType(PLACE_TYPE::TPlaceType placeType)
	{
		_PlaceType = placeType;
	}

private:
	/// Name of the zone
	std::string							_Name;
	/// radius of the zone
	uint16								_Radius;
	/// type of the zone ( useful for respawn points )
	RESPAWN_POINT::TRespawnPointType	_Type;
	/// parent continent id
	CONTINENT::TContinent				_Continent;
	/// parent region id
	uint16								_Region;
	// place type: capital, village etc
	PLACE_TYPE::TPlaceType				_PlaceType;
};


/**
 * A place in Ryzom is a patatoid that delimits a named place (village, special zones,...)
 * Regions and Continents inherits from theis class.
 * Places are in region, which are in continents
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CPlace : public NLLIGO::CPrimZone
{
public:
	/**
	 * build the place from a primitive file
	 * \param prim: the source primitive
	 * \param id: id of the place
	 * \return true on success
	 */
	bool build(const NLLIGO::CPrimZone * zone,uint16 id, bool reportAutorised = true);
	bool build(const NLLIGO::CPrimPath * zone,uint16 id);

	///\return the name of the place
	inline const std::string &  getName() const{ return _Name; }
	///\return the id of the place
	inline uint16 getId() const{ return _Id; }

	///\return center coords
	inline sint32 getCenterX(){ return _CenterX;}
	inline sint32 getCenterY(){ return _CenterY;}
	inline void setGooActive(bool state) { _GooActive = state; }

	bool getReported() const	{ return _Reported; }
	bool isGooPath() const { return _GooPath; }
	bool isGooActive() const { return _GooActive; }
	bool isMainPlace() const { return _MainPlace; }
	TAIAlias getAlias()const{ return _Alias; }

	const std::vector<uint16> & getRespawnPoints() const { return _RespawnPoints; }

protected:
	///\id of the place
	uint16		_Id;
private:
	/// Name of the place
	std::string _Name;
	/// center coords
	sint32		_CenterX;
	sint32		_CenterY;
	/// Flag for sending enter message to client
	bool		_Reported;
	/// Flag indicate this place is a goo path
	bool		_GooPath;
	bool		_GooActive;
	/// true if the place is the main place where a user can be
	bool		_MainPlace;
	/// respawn points validated when a user enters the place
	std::vector<uint16> _RespawnPoints;
	/// persistant alias
	TAIAlias	_Alias;
};

/**
 * Region class
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CRegion : public CPlace
{
public:
	/**
	 * build the region from a primitive file
	 * \param prim: the source primitive
	 * \param id: id of the region
	 * \return true on success
	 */
	bool build(const NLLIGO::CPrimZone * zone,uint16 id);

	/// ctor
	CRegion() : _Continent(CONTINENT::UNKNOWN) { }
	/// dtor
	~CRegion();

	/**
	 * add a place in that region
	 * \param place: the place to add
	 */
	inline void addPlace( CPlace* place ) { _Places.push_back( place ); }

	/**
	 * add a deposit in that region
	 * \param deposit: the deposit to add
	 */
	inline void addDeposit( CDeposit* deposit ) { _Deposits.push_back( deposit ); }

	///\return the places contained in that region
	inline const std::vector< CPlace* > & getPlaces() const {return _Places;}

	///\return the deposits contained in that region
	inline std::vector< CDeposit* > & getDeposits(){return _Deposits;}

	///\return true if the region is a newbie region
	inline bool isNewbieRegion()const { return _NewbieRegion; }

	/// add a user to the region
	void addPlayer( const NLMISC::CEntityId & id );

	/// remove a user from the region
	void removePlayer( const NLMISC::CEntityId & id );

	/// register the region chat group
	void registerChatGroup();

	/// players in region
	const std::set< NLMISC::CEntityId > & getPlayersInside() { return _Players; }

	/// get the continent of the region
	CONTINENT::TContinent getContinent() const { return _Continent; }

	/// set Continent of region
	void setContinent( CONTINENT::TContinent continent ) { _Continent = continent; }

private:
	/// places contained in this region
	std::vector< CPlace* >			_Places;

	/// deposits contained in this region
	std::vector< CDeposit* >		_Deposits;

	/// true if the region is a newbie region
	bool							_NewbieRegion;

	/// players in the region
	std::set< NLMISC::CEntityId >	_Players;

	/// continent where the region are
	CONTINENT::TContinent			_Continent;
};


/**
 * Continent class
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CContinent : public CPlace
{
public:
	/// continent names
//	static const std::string ContNames [];

	/**
	 * build the continent from a primitive file
	 * \param prim: the source primitive
	 * \return true on success
	 */
	bool build(const NLLIGO::CPrimZone * zone);

	/// dtor
	~CContinent();

	/**
	 * add a region in that continent
	 * \param region: the region to add
	 */
	inline void addRegion( CRegion * region ){ _Regions.push_back(region); }

	///\return the regions contained in that continent
	inline const std::vector< CRegion* > & getRegions(){return _Regions;}

private:
	///\regions contained in this continent
	std::vector< CRegion* > _Regions;

};


/**
 * Singleton used to manage zones in the EGS.
 * Its purpose is to store the data concerning zones forbe used for gameplay rules linked to geographic position.
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CZoneManager : public NLMISC::CSingleton<CZoneManager>
{
public:
	const uint DepositSearchTime;

	/// start point structure
	struct CStartPoint
	{
		uint16		SpawnZoneId;
		TAIAlias	Mission;
		TAIAlias	Welcomer;
	};

	// default constructor
	CZoneManager() 	: DepositSearchTime(50) {}

	~CZoneManager();

	/// init the manager
	void init();

	/// release the manager
	void release ();

	/// callback called when IOS connects
	void iosConnection();

	/// get a teleport zone
	inline const CTpSpawnZone * getTpSpawnZone( uint16 idx ) const;

	/// get a teleport zone id from its name. return InvalidSpawnZoneId if not found.
	uint16 getTpSpawnZoneIdByName( const std::string & name);

	/**
	 * get the continent where an entity is
	 * \param entity: the entity to check
	 * \return a pointer on the continent
	 */
	inline const CContinent * getContinent( CEntityBase * entity );

	/**
	 * get the region where an entity is
	 * \param entity: the entity to check
	 * \param region: pointer to be filled with the appropriate value ( must be allocated )
	 * \param continent: pointer to be filled with the appropriate value ( must be allocated if not NULL )
	 * \return a pointer on the continent
	 */
	inline bool getRegion( CEntityBase * entity, const CRegion ** region, const CContinent ** continent = NULL);

	/**
	 * get the place where an entity is
	 * \param entity: the entity to check
	 * \param stable: pointer to be filled with the appropriate value ( must be allocated )
	 * \param place: pointer to be filled with the appropriate value ( must be allocated )
	 * \param region: pointer to be filled with the appropriate value ( must be allocated if not NULL)
	 * \param continent: pointer to be filled with the appropriate value ( must be allocated if not NULL )
	 * \return a pointer on the continent
	 */
	inline bool getPlace( CEntityBase * entity, float& gooDistance, const CPlace ** stable, std::vector<const CPlace *>& places, const CRegion ** region = NULL, const CContinent ** continent = NULL );

	/**
	 * get the continent containing the given position
	 * \param x: X coord of the position
	 * \param y: Y coord of the position
	 * \return a pointer on the continent
	 */
	CContinent * getContinent( sint32 x, sint32 y );

	/// Same with vector
	CContinent * getContinent( const NLMISC::CVector& pos );

	/**
	 * get the region containing the given position
	 * \param x: X coord of the position
	 * \param y: Y coord of the position
	 * \param region: pointer to be filled with the appropriate value ( must be allocated )
	 * \param continent: pointer to be filled with the appropriate value ( must be allocated if not NULL )
	 * \return a pointer on the continent
	 */
	bool getRegion( sint32 x, sint32 y, const CRegion ** region, const CContinent ** continent = NULL);

	/// Same with vector
	CRegion * getRegion( const NLMISC::CVector& pos );

	/**
	 * get the place containing the given position
	 * \param x: X coord of the position
	 * \param y: Y coord of the position
	 * \param stable: pointer to be filled with the appropriate value ( must be allocated )
	 * \param place: pointer to be filled with the appropriate value ( must be allocated )
	 * \param region: pointer to be filled with the appropriate value ( must be allocated if not NULL)
	 * \param continent: pointer to be filled with the appropriate value ( must be allocated if not NULL )
	 * \return a pointer on the continent
	 */
	bool getPlace( sint32 x, sint32 y, float& gooDistance, const CPlace ** stable, std::vector<const CPlace *>& places, const CRegion ** region = NULL, const CContinent ** continent = NULL );

	/**
	 * get the deposits under the position
	 * \param user: user which position is to be checked
	 * \param deposits: vector of deposits to be filled
	 */
	void getDepositsUnderPos( const NLMISC::CVector& pos, std::vector<CDeposit*>& deposits, bool warnIfOutsideOfRegion=true );

	/**
	 * get the first deposit found under the position (faster than getDepositsUnderPos()), or NULL if not found
	 */
	CDeposit* getFirstFoundDepositUnderPos( const NLMISC::CVector& pos );

	/**
	 * Get a place from id, alias or name.
	 * A Place is a region, a zone, a stable or a goo border
	 */
	//@{
		/// get a place from its id (no search - direct access)
		inline CPlace* getPlaceFromId( uint16 id );

		/// get a place from its alias (search with a map.find)
		CPlace* getPlaceFromAlias( TAIAlias alias );

		/// get a place from its name (linear search in a vector)
		CPlace* getPlaceFromName( const std::string & name );
	//@}

	/**
	 * get a continent from its id
	 * \param id: id of the continent
	 * \return a pointer on the continent
	 */
	CContinent* getContinentFromId( CONTINENT::TContinent id );

	/**
	 * update the position of a character
	 * \param entityRow: row id of the chracter to update
	 */
	void updateCharacterPosition( CCharacter *  user );

	///update called at each tick
	void tickUpdate();

	/*
	 * dump the world organisation
	 * \param log: log in which we want to dump the result.
	 */
	void dumpWorld(NLMISC::CLog & log);

	/*
	 * dump the tp spawn zones
	 * \param log: log in which we want to dump the result.
	 */
	void dumpTpSpawnZones(NLMISC::CLog & log);

	/**
	 * a player used the /where command
	 * \param eId : entity leaving the zone
	 */
	void answerWhere(const NLMISC::CEntityId & eId);

	/**
	 *  a player harvest a deposit
	 * \param character the harvesting player
	 */
	//void harvestDeposit(CCharacter * user);

	/**
	 * remove a raw material from a deposit
	 * \param user: harvesting player
	 * \param depositindex: index of the deposit
	 * \param depositIndexContent : index of the harvested content in the deposit
	 */
	//void removeRmFromDeposit( CCharacter * user, uint32 depositIndex, uint32 depositIndexContent, uint16 quantity );

	/*
	 * dump one or all the deposits
	 * \param log: log in which we want to dump the result.
	 * \param depName: name of the deposit to dump, or "ALL" for all deposits
	 * \param extendedInfo: true to get more info on the raw materials of the deposit
	 */
	void dumpDeposits( NLMISC::CLog & log, const std::string& depName, bool extendedInfo=false );

	/// Accessor for deposits
	const std::vector< CDeposit* >& getDeposits() const { return _Deposits; }

	/// Used by CDeposit only.
	void	registerDepositToAutoSpawnUpdate(CDeposit *);
	void	unregisterDepositToAutoSpawnUpdate(CDeposit *);

	/// get a starting point for new character. NULL if invalid
	const CTpSpawnZone * getStartPoint( uint16 startPointIdx, TAIAlias & bot, TAIAlias & mission )
	{
		if ( startPointIdx >= _StartPoints.size() )
		{
			nlwarning("bad start point index %u ( count %u )",startPointIdx , _StartPoints.size() );
			return NULL;
		}
		// select a spawn zone randomly among the village spawn zones
		if ( _StartPoints[startPointIdx].empty() )
		{
			nlwarning("bad start point index %u: no spawn point in vector",startPointIdx );
			return NULL;
		}
		uint16 idx = (uint16)RandomGenerator.rand( (uint16)_StartPoints[startPointIdx].size() - 1 );
		mission = _StartPoints[startPointIdx][idx].Mission;
		bot = _StartPoints[startPointIdx][idx].Welcomer;
		return getTpSpawnZone( _StartPoints[startPointIdx][idx].SpawnZoneId );
	}

	/// get start point vector, slow because it makes a copy
	/// warning: this should only be used by CCharacterVersionAdapter::adaptToVersion3()
	std::vector<CStartPoint> getStartPointVector( uint16 startPointIdx ) const;

	/// send place name to a user
	uint32 sendPlaceName( const TDataSetRow & userRow, const std::string & placeName )
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::place);
		params[0].Identifier = placeName;
		return STRING_MANAGER::sendStringToClient( userRow,"SOLE_PLACE",params );
	}

	/// Add an ecotype information
	static void		addEcotype( CEcotypeZone *ecotypeZone ) { _EcotypeZones.push_back( ecotypeZone ); }

	/*
	 * Get the ecosystem under the position.
	 * If not found, a NULL pointer is returned.
	 */
	ECOSYSTEM::EECosystem getEcotype( const NLMISC::CVector& pos );

	/*
	 * Clear ecotype information
	 */
	void clearEcotypes();

private:

	/**
	 * init the instance. We had to do a separate method because there where methods that where called in the ctor that accessed to the singleton instance, but the intance is valid just AFTER the call to the ctor...
	 */
	void initInstance();

	/**
	 * parse the continents in a primitive
	 * \param prim : the root node of the primitive
	 */
	bool parseContinents(const NLLIGO::IPrimitive* prim);

	/**
	 * parse the regions in a primitive
	 * \param prim : the root node of the primitive
	 */
	bool parseRegions(const NLLIGO::IPrimitive* prim);

	/**
	 * parse the zones in a primitive
	 * \param prim : the root node of the primitive
	 */
	bool parseZones(const NLLIGO::IPrimitive* prim);

	/**
	 * parse the ecotypes in a primitive
	 * \param prim : the root node of the primitive
	 */
	bool parseEcotypes( const NLLIGO::IPrimitive* prim );

	/**
	 * parse the deposits in a primitive
	 * \param prim : the root node of the primitive
	 */
	bool parseDeposits( const NLLIGO::IPrimitive* prim );

	/**
	 * CZoneManager parseStables
	 * \param prim : the root node of the primitive
	 */
	bool parseStables( const NLLIGO::IPrimitive* prim );

	/**
	 * CZoneManager parseTpSpawnZones
	 * \param prim : the root node of the primitive
	 */
	bool parseTpSpawnZones( const NLLIGO::IPrimitive* prim );

	/**
	 * CZoneManager parseGooBorder
	 * \param prim : the root node of the primitive
	 */
	bool parseGooBorder( const NLLIGO::IPrimitive* prim );

	/**
	 * CZoneManager parseStartPoints
	 * \param prim : the root node of the primitive
	 */
	bool parseStartPoints( const NLLIGO::IPrimitive* prim );

	/**
	 * parse the PVP zones in a primitive
	 * \param prim : the root node of the primitive
	 */
	bool parsePVPZones( const NLLIGO::IPrimitive* prim );

	/**
	 * parse the PVP safe zones in a primitive
	 * \param prim : the root node of the primitive
	 */
	bool parsePVPSafeZones( const NLLIGO::IPrimitive* prim );

	/// the continents
	std::vector< CContinent >		_Continents;

	/// the places, regions and continent
	std::vector< CPlace* >			_Places;

	/// the deposits
	std::vector< CDeposit* >		_Deposits;

	/// time related to deposits update
	uint32							_NextDepositIndexUpdated;
	NLMISC::TGameCycle				_SpreadUpdateLoopBeginTick;

	/// The deposits that need updates for auto spawn
	std::set< CDeposit* >			_DepositNeedingAutoSpawnUpdate;

	/// teleport spawn zones
	std::vector< CTpSpawnZone >		_TpSpawnZones;

	std::map<std::string,uint16>	_TpSpawnZoneIdByName;

	/// initial starting points for new character. It is a vector of vector : each entry of the enclosing vector is a village. A village is a vector of spawn zone ids
	std::vector< std::vector<CStartPoint> >			_StartPoints;

	std::map<TAIAlias,CPlace*>		_PlacesByAlias;

	/// The ecotype zones
	static CEcotypeZones			_EcotypeZones;
};


//-----------------------------------------------
// CZoneManager getTpSpawnZone
//-----------------------------------------------
inline const CTpSpawnZone * CZoneManager::getTpSpawnZone( uint16 idx )const
{
	if ( idx >= _TpSpawnZones.size() )
	{
		nlwarning("CZoneManager::getTpSpawnZone -> invalid zone %u (count = %u )", idx, _TpSpawnZones.size() );
		return NULL;
	}
	return &_TpSpawnZones[idx];
}// CZoneManager getTpSpawnZone

//-----------------------------------------------
// CZoneManager getContinent
//-----------------------------------------------
inline const CContinent * CZoneManager::getContinent( CEntityBase * entity )
{
	return getContinent(entity->getState().X, entity->getState().Y);
}// CZoneManager getContinent

//-----------------------------------------------
// CZoneManager getRegion
//-----------------------------------------------
inline bool CZoneManager::getRegion( CEntityBase * entity, const CRegion ** region, const CContinent ** continent)
{
	return getRegion(entity->getState().X, entity->getState().Y,region,continent);
}// CZoneManager getRegion

//-----------------------------------------------
// CZoneManager getPlace
//-----------------------------------------------
inline bool CZoneManager::getPlace( CEntityBase * entity, float& gooDistance, const CPlace ** stable, std::vector<const CPlace *>& places, const CRegion ** region, const CContinent ** continent )
{
	return getPlace(entity->getState().X, entity->getState().Y,gooDistance, stable,places,region,continent);
}// CZoneManager getPlace

//-----------------------------------------------
// CZoneManager getPlaceFromId
//-----------------------------------------------
inline CPlace* CZoneManager::getPlaceFromId( uint16 id )
{
	if ( id < _Places.size() )
		return _Places[id];
	return NULL;
}// CZoneManager getPlaceFromId

#endif // RY_ZONE_MANAGER_H

/* End of zone_manager.h */
