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




#ifndef CL_ENTITIES_H
#define CL_ENTITIES_H


/////////////
// INCLUDE //
/////////////

// Client
#include "ground_fx_manager.h"
#include "projectile_manager.h"
#include "user_entity.h"
// Some constants
#include "game_share/mission_desc.h"
#include "game_share/inventories.h"
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/cdb_leaf.h"
// 3D
#include "nel/3d/u_instance.h"
// Std.
#include <vector>


///////////
// CLASS //
///////////
class CEntityCL;
class CUserEntity;
struct TNewEntityInfo;

/*
 *	Enum for entity selection
 */
class CEntityFilterFlag
{
public:
	enum	TFlag
	{
		NotUser		= 1 << 0,
		Friend		= 1 << 1,
		Enemy		= 1 << 2,
		Alive		= 1 << 3,
		Dead		= 1 << 4,
		Player		= 1 << 5,	// ok if entity is a Player (not user)
		NonPlayer	= 1 << 6,	// ok if entity is a non-player (ie a bot, whaterver Npc fauna etc...)

		NoFilter	= 0			// no filter
	};
};

/*
 *	Class to make cache entities
 */
class CEntityReference
{
public:
	CEntityReference (uint slot, CEntityCL *entity)
	{
		Slot = slot;
		Entity = entity;
	}

	CEntityCL	*Entity;
	uint		Slot;
};

/*
 *	Class to make cache shape instances
 */
class CShapeInstanceReference
{
public:
	CShapeInstanceReference (NL3D::UInstance instance, const string &text, const string &url, bool bbox_active=true)
	{
		Instance = instance;
		ContextText = text;
		ContextURL = url;
		BboxActive = bbox_active;
	}

	NL3D::UInstance Instance;
	string ContextText;
	string ContextURL;
	bool BboxActive;
};

/**
 * Class to manage entities and shapes instances.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CEntityManager : public NLMISC::IStreamable
{
private:
	typedef std::vector<CEntityCL *> TEntities;
	// Contain all entities.
	TEntities _Entities;
	// maximum number of entities.
	uint32 _NbMaxEntity;
	/// Nb Entities really allocated.
	uint32 _EntitiesAllocated;

	/// Entity caches
	std::vector<CEntityReference>	_ActiveEntities;
	std::vector<CEntityReference>	_VisibleEntities;

	/// Shapes Instances caches
	std::vector<CShapeInstanceReference>	_ShapeInstances;
	bool									_InstancesRemoved;

	typedef struct
	{
		NLMISC::TGameCycle	GC;
		sint64				Value;
	}										TProperty;
	typedef std::map<uint32, TProperty>		TProperties;
	typedef std::map<uint32, TProperties>	TBackupedChanges;
	TBackupedChanges						_BackupedChanges;

	// ground FXs
	CGroundFXManager	_GroundFXManager;
	// Handle of each entity in the ground fx manager
	std::vector<CGroundFXManager::TEntityHandle> _EntityGroundFXHandle;

	// For selection. NB: the pointer is just a cache. Must not be accessed
	CEntityCL				*_LastEntityUnderPos;

	NL3D::UInstance _LastInstanceUnderPos;

	// DB node pointers used to update some entity flags
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _MissionTargetTitleDB[MAX_NUM_MISSIONS][MAX_NUM_MISSION_TARGETS];
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _GroupMemberUidDB[8]; // MaxNumPeopleInTeam in people_interaction.h
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _GroupMemberNameDB[8]; // MaxNumPeopleInTeam in people_interaction.h
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _BeastUidDB[MAX_INVENTORY_ANIMAL];
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _BeastStatusDB[MAX_INVENTORY_ANIMAL];
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _BeastTypeDB[MAX_INVENTORY_ANIMAL];

	//////////////
	//// DEBUG ///
	uint	_NbUser;
	uint	_NbPlayer;
	uint	_NbChar;
	// For loging the property stage changes of the watched entity
	struct CStageSetLog
	{
		bool					Enabled;
		CLFECOMMON::TCLEntityId	LastEntityLoged;
		// start time of recording for the last entity
		sint32					StartGameCycle;
		sint64					StartLocalTime;
		CStageSet::TStageSet	StageSet;

		CStageSetLog()
		{
			Enabled= false;
			LastEntityLoged= CLFECOMMON::INVALID_SLOT;
		}
	};
	CStageSetLog			_LogStageChange;

private:
	/// Reset Counters
	void resetCounters() {_EntitiesAllocated = 0; _NbUser = 0; _NbPlayer = 0; _NbChar = 0;}


public:
	NLMISC_DECLARE_CLASS(CEntityManager);

	/// Constructor.
	CEntityManager();
	/// Destructor.
	~CEntityManager();

	/**
	 * Initialize some dynamic parameters.
	 * \param uint nbMaxEntity : maximum number of entities allocated.
	 */
	void initialize(uint nbMaxEntity);

	// get ground fx manager
	CGroundFXManager &getGroundFXManager() { return _GroundFXManager; }

	/// Free the class and all the components.
	void release();

	/// Release + initialize
	void reinit();


	CShapeInstanceReference createInstance(const string& shape, const CVector &pos, const string &text, const string &url, bool active=true);
	bool removeInstances();
	bool instancesRemoved();
	CShapeInstanceReference getShapeInstanceUnderPos(float x, float y);

	/**
	 * Create an entity according to the slot and the form.
	 * \param uint slot : slot for the entity.
	 * \param uint32 form : form to create the entity.
	 * \param TClientDataSetIndex : persitent id while the entity is connected.
	 * \return CEntityCL * : pointer on the new entity.
	 */
	CEntityCL *create(uint slot, uint32 form, const TNewEntityInfo& newEntityInfo);

	/**
	 * Delete an entity.
	 * \return bool : 'true' if the entity has been correctly removed.
	 */
	bool remove(uint slot, bool warning);
	/// Remove the collision for all entities.
	void removeCollision();
	/// Re-load animations (remove and load).
	void reloadAnims();

	/**
	 * Get a pointer on an entity according to the asked slot.
	 * \param uint slot : the asked slot.
	 */
	CEntityCL *entity(uint slot);

	/**
	 * Return if there is an entity near a door.
	 * \param float openingDist : near is when you are under the 'openingDist'.
	 * \param const CVector& posDoor1 : first door position.
	 * \param const CVector& posDoor2 : second door position.
	 * \return bool ; 'true' if any entity is near one of the door.
	 */
	bool entitiesNearDoors(float openingDist, const NLMISC::CVector& posDoor1, const NLMISC::CVector& posDoor2);

	/**
	 * Get the entity under the (2d) position. Return NULL if no entity under this position.
	 *	NB: the UserEntity can NOT be returned. But if the code has find it, isPlayerUnderCursor is set to true
	 *	NB: somewhat slow. should be called ONLY ONCE per frame.
	 *	NB: unselectable entities are not returned
	 */
	CEntityCL *getEntityUnderPos(float x, float y, float distSelection, bool &isPlayerUnderCursor);

	/**
	 * Get the entity (not user) in the camera. Return NULL if not entity under this position.
	 *	\param flags a ORed of CEntityFilterFlag::TFlag
	 *	\param distSelection don't go beyond
	 *	\param precEntity is used for Cyclic management. If found in the select list, get the next one, else get the first.
	 */
	CEntityCL *getEntityInCamera(uint flags, float distSelection, CLFECOMMON::TCLEntityId	precEntity);

	/// Get an entity by name. Returns NULL if the entity is not found.
	CEntityCL *getEntityByName (uint32 stringId) const;

	/** Get an entity by name. Returns NULL if the entity is not found.
      * \param name of the entity to find
	  * \param caseSensitive type of test to perform
	  * \param complete : if true, the name must match the full name of the entity.
	  */
	CEntityCL *getEntityByName (const ucstring &name, bool caseSensitive, bool complete) const;

	/// Get an entity by dataset index. Returns NULL if the entity is not found.
	CEntityCL *getEntityByCompressedIndex(TDataSetIndex compressedIndex) const;

	/// Return number of entities allocated.
	uint nbEntitiesAllocated() const {return _EntitiesAllocated;}
	/// Return the number of user allocated.
	uint nbUser() const {return _NbUser;}
	/// Return the number of player allocated.
	uint nbPlayer() const {return _NbPlayer;}
	/// Return the number of character allocated.
	uint nbChar() const {return _NbChar;}



	/// Continent has changed.
	void changeContinent();

	/**
	 * Update the entity (position\animation).
	 */
	void updatePreCamera();
	void updatePostCamera(uint clippedUpdateTime, const std::vector<NLMISC::CPlane> &clippingPlanes, const NLMISC::CVector &camPos);
	void updatePostRender();

	/**
	 * Method to update the visual property 'prop' for the entity in 'slot'.
	 * \param uint slot : slot of the entity to update.
	 * \param uint prop : the property to udapte.
	 */
	void updateVisualProperty(const NLMISC::TGameCycle &gameCycle, const uint &slot, const uint &prop, const NLMISC::TGameCycle &predictedInterval = 0);

	/**
	 *
	 */
	void applyBackupedProperties(uint slot);

	/// Return the reference for all entities in the vision. An entity object, for a slot, is not allocated if not visible.
	TEntities &entities() {return _Entities;}

	/// Manage PACS Triggers.
	void managePACSTriggers();
	///
	void removeColUserOther();
	void restoreColUserOther();

	// For Sound Reset/Reload
	void	resetAllSoundAnimId();

	///////////
	// DEBUG //
	///////////
	/// Write a file with the position of all entities.
	void writeEntities();
	/// Dump entities state.
	void dump(class NLMISC::IStream &f);
	/// Dump entities state (XML Format).
	void dumpXML(class NLMISC::IStream &f);
	/// Log Watched Entity Stages Change
	void startLogStageChange(sint32 currentGameCycle, sint64 currentLocalTime);
	void logStageChange(sint64 currentLocalTime);
	void stopLogStageChange();
	bool isLogingStageChange() const;
	sint32 getLogStageChangeStartCycle() const;
	sint64 getLogStageChangeStartLocalTime() const;

	/// Serialize entities.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	// remove all attached fx of all entities (so that they can be reloaded)
	virtual void removeAllAttachedFX();

	/**
	 * If the slot leads to a CCharacterCL compatible with mission icon, and
	 * who isFriend(), perform releaseInSceneInterfaces() and buildInSceneInterface().
	 * Otherwise, do nothing.
	 */
	void refreshInsceneInterfaceOfFriendNPC(uint slot);

	inline NLMISC::CCDBNodeLeaf *getMissionTargetTitleDB(int mission, int target) { return _MissionTargetTitleDB[mission][target]; }
	inline NLMISC::CCDBNodeLeaf *getGroupMemberUidDB(int member) { return _GroupMemberUidDB[member]; }
	inline NLMISC::CCDBNodeLeaf *getGroupMemberNameDB(int member) { return _GroupMemberNameDB[member]; }
	inline NLMISC::CCDBNodeLeaf *getBeastUidDB(int beast) { return _BeastUidDB[beast]; }
	inline NLMISC::CCDBNodeLeaf *getBeastStatusDB(int beast) { return _BeastStatusDB[beast]; }
	inline NLMISC::CCDBNodeLeaf *getBeastTypeDB(int beast) { return _BeastTypeDB[beast]; }

private:

	// NB: don't return unselectable entities
	void	getEntityListForSelection(std::vector<CEntityCL*> &entities, uint flags);

	void	updateEntitiesIsInTeam();

	/// Log entities Property Stages Change
	void	logPropertyChange(CLFECOMMON::TCLEntityId who, const CStage &oldStage, const CStage &newStage,
		const NLMISC::CVectorD &precOldPos, const NLMISC::CVectorD &precNewPos, sint32 relGameCycle, sint64 relLocalTime);
};

extern CEntityManager	EntitiesMngr;



#endif // CL_ENTITIES_H

/* End of entities.h */
