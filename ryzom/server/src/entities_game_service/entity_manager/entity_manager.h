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



#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

//game share
#include "game_share/ryzom_entity_id.h"
#include "game_share/sentence_appraisal.h"
#include "server_share/msg_ai_service.h"
//
#include "egs_mirror.h"

class CEntityBase;


/**
 * Implementation of the entity info transport class
 */
class CAIInfosOnEntityMsgImp : public CAIInfosOnEntityMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

/**
 * Implementation of transport class
 */
class CQueryEgsImp : public CQueryEgs
{
	void callback (const std::string &name, NLNET::TServiceId id);
};

/**
 * CEntityBaseManager
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CEntityBaseManager
{
public:
	/// exception thrown when entity is unknown
	struct EEntity : public NLMISC::Exception
	{
		EEntity( const NLMISC::CEntityId& id ) : Exception ("The entity "+id.toString()+" doesn't exist") {}
	};

/*	// Success table for calculation of success probability and associate xp-gains
	struct SSuccessXpLine
	{
		sint16	RelativeLevel;
		uint16	SuccessProbability;
		float	XpGain;
		SENTENCE_APPRAISAL::ESentenceAppraisal DifficultyAppreciation;
	};

	struct TSuccessTable
	{
		float	MaxSuccess;
		float	FadeSuccess;
		uint8	FadeRoll;

		uint8	CraftFullSuccessRole;
		float	CraftMinSuccess;
		uint8	CraftMinSuccessRole;
	};

*/	/**
	 * Constructor
	 */
	CEntityBaseManager();

	/**
	 * Add callback for entity management
	 */
	void addEntityCallback();

	// getEntityPtr : return CEntityBase * ptr on Id corresponding entity
	/*A*/static CEntityBase			*getEntityBasePtr	( const NLMISC::CEntityId& id );
	/*A*/static CEntityBase			*getEntityBasePtr	( const	TDataSetRow	&entityRowId )
	{
		if ( TheDataset.isAccessible( entityRowId ) )
		{
			return getEntityBasePtr( TheDataset.getEntityId( entityRowId ) );
		}
		return 0;
	}

	/*A*/static NLMISC::CEntityId	getEntityId			( const	TDataSetRow	&entityRowId )
	{
		if ( TheDataset.isAccessible( entityRowId ) ) //( entityRowId.isValid() && TheDataset.isDataSetRowStillValid(entityRowId) )
			return TheDataset.getEntityId( entityRowId );
		else
			return NLMISC::CEntityId::Unknown;
	}

	/**
	 * GPMS connexion
	 */
	void gpmsConnexion();

	/**
	 * load table of success probability and xp gains
	 * \param tableName is name of table contained success change and xp gain
	 */
//	void loadSuccessXpTable( const std::string& tableName );

//	static std::vector< SSuccessXpLine >	_SuccessXpTable;
//	static TSuccessTable					_SuccessTable;
};


#endif //CREATURE_MANAGER
