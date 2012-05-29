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


#ifndef MISSION_PD_H
#define MISSION_PD_H

#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <nel/misc/entity_id.h>
#include <nel/misc/sheet_id.h>
#include <vector>
#include <map>
#include <pd_lib/pd_lib.h>
#include <game_share/persistent_data.h>

// User #includes
#include "mission_manager/mission_base_behaviour.h"

namespace EGSPD
{
	
//
// Forward declarations
//



//
// Typedefs & Enums
//

/** CActiveStepStatePD
 * defined at entities_game_service/pd_scripts/mission.pds:6
 */
class CActiveStepStatePD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	uint32							getIndex() const;
	
	uint32							getState() const;
	void							setState(uint32 __v, bool forceWrite=false);
	
	CActiveStepPD*					getStep();
	const CActiveStepPD*			getStep() const;
	
	// @}


public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	void							clear();
	
	/**
	 * Cast base object to CActiveStepStatePD
	 */
	static CActiveStepStatePD*		cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CActiveStepStatePD
	 */
	static const CActiveStepStatePD*	cast(const RY_PDS::IPDBaseData* obj);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CActiveStepStatePD();
	~CActiveStepStatePD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	void							apply(CPersistentDataRecord &__pdr);
	void							store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name Attributes
	// @{
		
	/**
	 * Don't modify those value manually, use accessors and mutators above
	 */
	
	uint32							_Index;
	uint32							_State;
	CActiveStepPD*					_Step;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const uint32 &Index);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__setParent(CActiveStepPD* __parent);
	void							pds__setParentUnnotified(CActiveStepPD* __parent);
	void							pds__notifyInit();
	void							pds__notifyRelease();
	static void						pds_static__init();
	
	// @}


protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	
	// @}


protected:

	friend class CActiveStepPD;
	friend class CMissionContainerPD;
	friend class CMissionPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CActiveStepPD
 * defined at entities_game_service/pd_scripts/mission.pds:13
 */
class CActiveStepPD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	uint32							getIndexInTemplate() const;
	
	CActiveStepStatePD*				getStates(const uint32& __k);
	const CActiveStepStatePD*		getStates(const uint32& __k) const;
	std::map<uint32, CActiveStepStatePD>::iterator	getStatesBegin();
	std::map<uint32, CActiveStepStatePD>::iterator	getStatesEnd();
	std::map<uint32, CActiveStepStatePD>::const_iterator	getStatesBegin() const;
	std::map<uint32, CActiveStepStatePD>::const_iterator	getStatesEnd() const;
	const std::map<uint32, CActiveStepStatePD> &	getStates() const;
	CActiveStepStatePD*				addToStates(const uint32 &__k);
	void							deleteFromStates(const uint32 &__k);
	
	CMissionPD*						getMission();
	const CMissionPD*				getMission() const;
	
	// @}


public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	void							clear();
	
	/**
	 * Cast base object to CActiveStepPD
	 */
	static CActiveStepPD*			cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CActiveStepPD
	 */
	static const CActiveStepPD*		cast(const RY_PDS::IPDBaseData* obj);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CActiveStepPD();
	~CActiveStepPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	void							apply(CPersistentDataRecord &__pdr);
	void							store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name Attributes
	// @{
		
	/**
	 * Don't modify those value manually, use accessors and mutators above
	 */
	
	uint32							_IndexInTemplate;
	std::map<uint32, CActiveStepStatePD>	_States;
	CMissionPD*						_Mission;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const uint32 &IndexInTemplate);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__setParent(CMissionPD* __parent);
	void							pds__setParentUnnotified(CMissionPD* __parent);
	void							pds__notifyInit();
	void							pds__notifyRelease();
	static void						pds_static__init();
	
	// @}


protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	
	// @}


protected:

	friend class CMissionContainerPD;
	friend class CMissionPD;
	friend class CActiveStepStatePD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CDoneStepPD
 * defined at entities_game_service/pd_scripts/mission.pds:20
 */
class CDoneStepPD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	uint32							getIndex() const;
	
	CMissionPD*						getMission();
	const CMissionPD*				getMission() const;
	
	// @}


public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	void							clear();
	
	/**
	 * Cast base object to CDoneStepPD
	 */
	static CDoneStepPD*				cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CDoneStepPD
	 */
	static const CDoneStepPD*		cast(const RY_PDS::IPDBaseData* obj);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CDoneStepPD();
	~CDoneStepPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	void							apply(CPersistentDataRecord &__pdr);
	void							store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name Attributes
	// @{
		
	/**
	 * Don't modify those value manually, use accessors and mutators above
	 */
	
	uint32							_Index;
	CMissionPD*						_Mission;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const uint32 &Index);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__setParent(CMissionPD* __parent);
	void							pds__setParentUnnotified(CMissionPD* __parent);
	void							pds__notifyInit();
	void							pds__notifyRelease();
	static void						pds_static__init();
	
	// @}


protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	
	// @}


protected:

	friend class CMissionContainerPD;
	friend class CMissionPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CMissionCompassPD
 * defined at entities_game_service/pd_scripts/mission.pds:27
 */
class CMissionCompassPD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	uint32							getIndex() const;
	
	uint32							getPlace() const;
	void							setPlace(uint32 __v, bool forceWrite=false);
	
	uint32							getBotId() const;
	void							setBotId(uint32 __v, bool forceWrite=false);
	
	CMissionPD*						getMission();
	const CMissionPD*				getMission() const;
	
	// @}


public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	void							clear();
	
	/**
	 * Cast base object to CMissionCompassPD
	 */
	static CMissionCompassPD*		cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CMissionCompassPD
	 */
	static const CMissionCompassPD*	cast(const RY_PDS::IPDBaseData* obj);
	
	// @}


public:

	/// \name User defined attributes and methods
	// @{
		
	/**
	 * This code was verbatim copied from source file
	 */
	
	// User code appended in mission.pds
	uint32							NameStringId;
	std::string						NameString;
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CMissionCompassPD();
	~CMissionCompassPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	void							apply(CPersistentDataRecord &__pdr);
	void							store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name Attributes
	// @{
		
	/**
	 * Don't modify those value manually, use accessors and mutators above
	 */
	
	uint32							_Index;
	uint32							_Place;
	uint32							_BotId;
	CMissionPD*						_Mission;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const uint32 &Index);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__setParent(CMissionPD* __parent);
	void							pds__setParentUnnotified(CMissionPD* __parent);
	void							pds__notifyInit();
	void							pds__notifyRelease();
	static void						pds_static__init();
	
	// @}


protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	
	// @}


protected:

	friend class CMissionContainerPD;
	friend class CMissionPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CMissionTeleportPD
 * defined at entities_game_service/pd_scripts/mission.pds:49
 */
class CMissionTeleportPD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	uint32							getIndex() const;
	
	CMissionPD*						getMission();
	const CMissionPD*				getMission() const;
	
	// @}


public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	void							clear();
	
	/**
	 * Cast base object to CMissionTeleportPD
	 */
	static CMissionTeleportPD*		cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CMissionTeleportPD
	 */
	static const CMissionTeleportPD*	cast(const RY_PDS::IPDBaseData* obj);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CMissionTeleportPD();
	~CMissionTeleportPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	void							apply(CPersistentDataRecord &__pdr);
	void							store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name Attributes
	// @{
		
	/**
	 * Don't modify those value manually, use accessors and mutators above
	 */
	
	uint32							_Index;
	CMissionPD*						_Mission;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const uint32 &Index);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__setParent(CMissionPD* __parent);
	void							pds__setParentUnnotified(CMissionPD* __parent);
	void							pds__notifyInit();
	void							pds__notifyRelease();
	static void						pds_static__init();
	
	// @}


protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	
	// @}


protected:

	friend class CMissionContainerPD;
	friend class CMissionPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CMissionInsidePlacePD
 * defined at entities_game_service/pd_scripts/mission.pds:55
 */
class CMissionInsidePlacePD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	uint32							getAlias() const;
	
	uint32							getDelay() const;
	void							setDelay(uint32 __v, bool forceWrite=false);
	
	CMissionPD*						getMission();
	const CMissionPD*				getMission() const;
	
	// @}


public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	void							clear();
	
	/**
	 * Cast base object to CMissionInsidePlacePD
	 */
	static CMissionInsidePlacePD*	cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CMissionInsidePlacePD
	 */
	static const CMissionInsidePlacePD*	cast(const RY_PDS::IPDBaseData* obj);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CMissionInsidePlacePD();
	~CMissionInsidePlacePD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	void							apply(CPersistentDataRecord &__pdr);
	void							store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name Attributes
	// @{
		
	/**
	 * Don't modify those value manually, use accessors and mutators above
	 */
	
	uint32							_Alias;
	uint32							_Delay;
	CMissionPD*						_Mission;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const uint32 &Alias);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__setParent(CMissionPD* __parent);
	void							pds__setParentUnnotified(CMissionPD* __parent);
	void							pds__notifyInit();
	void							pds__notifyRelease();
	static void						pds_static__init();
	
	// @}


protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	
	// @}


protected:

	friend class CMissionContainerPD;
	friend class CMissionPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CMissionOutsidePlacePD
 * defined at entities_game_service/pd_scripts/mission.pds:61
 */
class CMissionOutsidePlacePD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	uint32							getAlias() const;
	
	uint32							getDelay() const;
	void							setDelay(uint32 __v, bool forceWrite=false);
	
	CMissionPD*						getMission();
	const CMissionPD*				getMission() const;
	
	// @}


public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	void							clear();
	
	/**
	 * Cast base object to CMissionOutsidePlacePD
	 */
	static CMissionOutsidePlacePD*	cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CMissionOutsidePlacePD
	 */
	static const CMissionOutsidePlacePD*	cast(const RY_PDS::IPDBaseData* obj);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CMissionOutsidePlacePD();
	~CMissionOutsidePlacePD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	void							apply(CPersistentDataRecord &__pdr);
	void							store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name Attributes
	// @{
		
	/**
	 * Don't modify those value manually, use accessors and mutators above
	 */
	
	uint32							_Alias;
	uint32							_Delay;
	CMissionPD*						_Mission;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const uint32 &Alias);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__setParent(CMissionPD* __parent);
	void							pds__setParentUnnotified(CMissionPD* __parent);
	void							pds__notifyInit();
	void							pds__notifyRelease();
	static void						pds_static__init();
	
	// @}


protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	
	// @}


protected:

	friend class CMissionContainerPD;
	friend class CMissionPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CHandledAIGroupPD
 * defined at entities_game_service/pd_scripts/mission.pds:68
 */
class CHandledAIGroupPD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	uint32							getGroupAlias() const;
	
	uint32							getDespawnTime() const;
	void							setDespawnTime(uint32 __v, bool forceWrite=false);
	
	CMissionPD*						getMission();
	const CMissionPD*				getMission() const;
	
	// @}


public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	void							clear();
	
	/**
	 * Cast base object to CHandledAIGroupPD
	 */
	static CHandledAIGroupPD*		cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CHandledAIGroupPD
	 */
	static const CHandledAIGroupPD*	cast(const RY_PDS::IPDBaseData* obj);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CHandledAIGroupPD();
	~CHandledAIGroupPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	void							apply(CPersistentDataRecord &__pdr);
	void							store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name Attributes
	// @{
		
	/**
	 * Don't modify those value manually, use accessors and mutators above
	 */
	
	uint32							_GroupAlias;
	uint32							_DespawnTime;
	CMissionPD*						_Mission;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const uint32 &GroupAlias);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__setParent(CMissionPD* __parent);
	void							pds__setParentUnnotified(CMissionPD* __parent);
	void							pds__notifyInit();
	void							pds__notifyRelease();
	static void						pds_static__init();
	
	// @}


protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	
	// @}


protected:

	friend class CMissionContainerPD;
	friend class CMissionPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CMissionPD
 * defined at entities_game_service/pd_scripts/mission.pds:75
 */
class CMissionPD : public RY_PDS::IPDBaseData, public CMissionBaseBehaviour
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	uint32							getTemplateId() const;
	
	uint32							getMainMissionTemplateId() const;
	void							setMainMissionTemplateId(uint32 __v, bool forceWrite=false);
	
	uint32							getGiver() const;
	void							setGiver(uint32 __v, bool forceWrite=false);
	
	float							getHourLowerBound() const;
	void							setHourLowerBound(float __v, bool forceWrite=false);
	
	float							getHourUpperBound() const;
	void							setHourUpperBound(float __v, bool forceWrite=false);
	
	CSeason::TSeason				getSeason() const;
	void							setSeason(CSeason::TSeason __v, bool forceWrite=false);
	
	uint32							getMonoEndDate() const;
	void							setMonoEndDate(uint32 __v, bool forceWrite=false);
	
	uint32							getEndDate() const;
	void							setEndDate(uint32 __v, bool forceWrite=false);
	
	uint32							getCriticalPartEndDate() const;
	void							setCriticalPartEndDate(uint32 __v, bool forceWrite=false);
	
	uint32							getBeginDate() const;
	void							setBeginDate(uint32 __v, bool forceWrite=false);
	
	uint32							getFailureIndex() const;
	void							setFailureIndex(uint32 __v, bool forceWrite=false);
	
	uint32							getCrashHandlerIndex() const;
	void							setCrashHandlerIndex(uint32 __v, bool forceWrite=false);
	
	uint32							getPlayerReconnectHandlerIndex() const;
	void							setPlayerReconnectHandlerIndex(uint32 __v, bool forceWrite=false);
	
	bool							getFinished() const;
	void							setFinished(bool __v, bool forceWrite=false);
	
	bool							getMissionSuccess() const;
	void							setMissionSuccess(bool __v, bool forceWrite=false);
	
	uint32							getDescIndex() const;
	void							setDescIndex(uint32 __v, bool forceWrite=false);
	
	uint32							getWaitingQueueId() const;
	void							setWaitingQueueId(uint32 __v, bool forceWrite=false);
	
	CActiveStepPD*					getSteps(const uint32& __k);
	const CActiveStepPD*			getSteps(const uint32& __k) const;
	std::map<uint32, CActiveStepPD>::iterator	getStepsBegin();
	std::map<uint32, CActiveStepPD>::iterator	getStepsEnd();
	std::map<uint32, CActiveStepPD>::const_iterator	getStepsBegin() const;
	std::map<uint32, CActiveStepPD>::const_iterator	getStepsEnd() const;
	const std::map<uint32, CActiveStepPD> &	getSteps() const;
	CActiveStepPD*					addToSteps(const uint32 &__k);
	void							deleteFromSteps(const uint32 &__k);
	
	CMissionCompassPD*				getCompass(const uint32& __k);
	const CMissionCompassPD*		getCompass(const uint32& __k) const;
	std::map<uint32, CMissionCompassPD>::iterator	getCompassBegin();
	std::map<uint32, CMissionCompassPD>::iterator	getCompassEnd();
	std::map<uint32, CMissionCompassPD>::const_iterator	getCompassBegin() const;
	std::map<uint32, CMissionCompassPD>::const_iterator	getCompassEnd() const;
	const std::map<uint32, CMissionCompassPD> &	getCompass() const;
	CMissionCompassPD*				addToCompass(const uint32 &__k);
	void							deleteFromCompass(const uint32 &__k);
	
	CDoneStepPD*					getStepsDone(const uint32& __k);
	const CDoneStepPD*				getStepsDone(const uint32& __k) const;
	std::map<uint32, CDoneStepPD>::iterator	getStepsDoneBegin();
	std::map<uint32, CDoneStepPD>::iterator	getStepsDoneEnd();
	std::map<uint32, CDoneStepPD>::const_iterator	getStepsDoneBegin() const;
	std::map<uint32, CDoneStepPD>::const_iterator	getStepsDoneEnd() const;
	const std::map<uint32, CDoneStepPD> &	getStepsDone() const;
	CDoneStepPD*					addToStepsDone(const uint32 &__k);
	void							deleteFromStepsDone(const uint32 &__k);
	
	CMissionTeleportPD*				getTeleports(const uint32& __k);
	const CMissionTeleportPD*		getTeleports(const uint32& __k) const;
	std::map<uint32, CMissionTeleportPD>::iterator	getTeleportsBegin();
	std::map<uint32, CMissionTeleportPD>::iterator	getTeleportsEnd();
	std::map<uint32, CMissionTeleportPD>::const_iterator	getTeleportsBegin() const;
	std::map<uint32, CMissionTeleportPD>::const_iterator	getTeleportsEnd() const;
	const std::map<uint32, CMissionTeleportPD> &	getTeleports() const;
	CMissionTeleportPD*				addToTeleports(const uint32 &__k);
	void							deleteFromTeleports(const uint32 &__k);
	
	CMissionInsidePlacePD*			getInsidePlaces(const uint32& __k);
	const CMissionInsidePlacePD*	getInsidePlaces(const uint32& __k) const;
	std::map<uint32, CMissionInsidePlacePD>::iterator	getInsidePlacesBegin();
	std::map<uint32, CMissionInsidePlacePD>::iterator	getInsidePlacesEnd();
	std::map<uint32, CMissionInsidePlacePD>::const_iterator	getInsidePlacesBegin() const;
	std::map<uint32, CMissionInsidePlacePD>::const_iterator	getInsidePlacesEnd() const;
	const std::map<uint32, CMissionInsidePlacePD> &	getInsidePlaces() const;
	CMissionInsidePlacePD*			addToInsidePlaces(const uint32 &__k);
	void							deleteFromInsidePlaces(const uint32 &__k);
	
	CMissionOutsidePlacePD*			getOutsidePlaces(const uint32& __k);
	const CMissionOutsidePlacePD*	getOutsidePlaces(const uint32& __k) const;
	std::map<uint32, CMissionOutsidePlacePD>::iterator	getOutsidePlacesBegin();
	std::map<uint32, CMissionOutsidePlacePD>::iterator	getOutsidePlacesEnd();
	std::map<uint32, CMissionOutsidePlacePD>::const_iterator	getOutsidePlacesBegin() const;
	std::map<uint32, CMissionOutsidePlacePD>::const_iterator	getOutsidePlacesEnd() const;
	const std::map<uint32, CMissionOutsidePlacePD> &	getOutsidePlaces() const;
	CMissionOutsidePlacePD*			addToOutsidePlaces(const uint32 &__k);
	void							deleteFromOutsidePlaces(const uint32 &__k);
	
	CHandledAIGroupPD*				getHandledAIGroups(const uint32& __k);
	const CHandledAIGroupPD*		getHandledAIGroups(const uint32& __k) const;
	std::map<uint32, CHandledAIGroupPD>::iterator	getHandledAIGroupsBegin();
	std::map<uint32, CHandledAIGroupPD>::iterator	getHandledAIGroupsEnd();
	std::map<uint32, CHandledAIGroupPD>::const_iterator	getHandledAIGroupsBegin() const;
	std::map<uint32, CHandledAIGroupPD>::const_iterator	getHandledAIGroupsEnd() const;
	const std::map<uint32, CHandledAIGroupPD> &	getHandledAIGroups() const;
	CHandledAIGroupPD*				addToHandledAIGroups(const uint32 &__k);
	void							deleteFromHandledAIGroups(const uint32 &__k);
	
	CMissionContainerPD*			getContainer();
	const CMissionContainerPD*		getContainer() const;
	
	// @}


public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	virtual void					clear();
	
	/**
	 * Cast base object to CMissionPD
	 */
	static CMissionPD*				cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CMissionPD
	 */
	static const CMissionPD*		cast(const RY_PDS::IPDBaseData* obj);
	
	/**
	 * Set user factory for this class (as class is indicated as derived, a home made constructor must be provided)
	 */
	static void						setFactory(RY_PDS::TPDFactory userFactory);
	
	/**
	 * Create an object of the CMissionPD class, and declare it to the PDS.
	 */
	static CMissionPD*				create(const uint32 &TemplateId);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CMissionPD();
	virtual ~CMissionPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	virtual void					apply(CPersistentDataRecord &__pdr);
	virtual void					store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name User defined init and release methods
	// @{
		
	/**
	 * Overload those methods to implement init and release behaviours
	 */
	
	virtual void					init();
	virtual void					release();
	
	// @}


protected:

	/// \name Attributes
	// @{
		
	/**
	 * Don't modify those value manually, use accessors and mutators above
	 */
	
	uint32							_TemplateId;
	uint32							_MainMissionTemplateId;
	uint32							_Giver;
	float							_HourLowerBound;
	float							_HourUpperBound;
	CSeason::TSeason				_Season;
	uint32							_MonoEndDate;
	uint32							_EndDate;
	uint32							_CriticalPartEndDate;
	uint32							_BeginDate;
	uint32							_FailureIndex;
	uint32							_CrashHandlerIndex;
	uint32							_PlayerReconnectHandlerIndex;
	bool							_Finished;
	bool							_MissionSuccess;
	uint32							_DescIndex;
	uint32							_WaitingQueueId;
	std::map<uint32, CActiveStepPD>	_Steps;
	std::map<uint32, CMissionCompassPD>	_Compass;
	std::map<uint32, CDoneStepPD>	_StepsDone;
	std::map<uint32, CMissionTeleportPD>	_Teleports;
	std::map<uint32, CMissionInsidePlacePD>	_InsidePlaces;
	std::map<uint32, CMissionOutsidePlacePD>	_OutsidePlaces;
	std::map<uint32, CHandledAIGroupPD>	_HandledAIGroups;
	CMissionContainerPD*			_Container;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	virtual void					pds__init(const uint32 &TemplateId);
	virtual void					pds__destroy();
	virtual void					pds__fetch(RY_PDS::CPData &data);
	virtual void					pds__register();
	virtual void					pds__registerAttributes();
	virtual void					pds__unregister();
	virtual void					pds__unregisterAttributes();
	void							pds__setParent(CMissionContainerPD* __parent);
	void							pds__setParentUnnotified(CMissionContainerPD* __parent);
	virtual void					pds__notifyInit();
	virtual void					pds__notifyRelease();
	static void						pds_static__init();
	
	// @}


protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static void						pds_static__setFactory(RY_PDS::TPDFactory userFactory);
	static bool						_FactoryInitialised;
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	static void						pds_static__fetch(RY_PDS::IPDBaseData *object, RY_PDS::CPData &data);
	
	// @}


protected:

	friend class CMissionContainerPD;
	friend class CActiveStepPD;
	friend class CDoneStepPD;
	friend class CHandledAIGroupPD;
	friend class CMissionCompassPD;
	friend class CMissionInsidePlacePD;
	friend class CMissionOutsidePlacePD;
	friend class CMissionTeleportPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CMissionGuildPD
 * defined at entities_game_service/pd_scripts/mission.pds:135
 */
class CMissionGuildPD : public CMissionPD
{

public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	virtual void					clear();
	
	/**
	 * Cast base object to CMissionGuildPD
	 */
	static CMissionGuildPD*			cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CMissionGuildPD
	 */
	static const CMissionGuildPD*	cast(const RY_PDS::IPDBaseData* obj);
	
	/**
	 * Set user factory for this class (as class is indicated as derived, a home made constructor must be provided)
	 */
	static void						setFactory(RY_PDS::TPDFactory userFactory);
	
	/**
	 * Create an object of the CMissionGuildPD class, and declare it to the PDS.
	 */
	static CMissionGuildPD*			create(const uint32 &TemplateId);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CMissionGuildPD();
	virtual ~CMissionGuildPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	virtual void					apply(CPersistentDataRecord &__pdr);
	virtual void					store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name User defined init and release methods
	// @{
		
	/**
	 * Overload those methods to implement init and release behaviours
	 */
	
	virtual void					init();
	virtual void					release();
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	virtual void					pds__init(const uint32 &TemplateId);
	virtual void					pds__destroy();
	virtual void					pds__fetch(RY_PDS::CPData &data);
	virtual void					pds__register();
	virtual void					pds__registerAttributes();
	virtual void					pds__unregister();
	virtual void					pds__unregisterAttributes();
	virtual void					pds__notifyInit();
	virtual void					pds__notifyRelease();
	static void						pds_static__init();
	
	// @}


protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static void						pds_static__setFactory(RY_PDS::TPDFactory userFactory);
	static bool						_FactoryInitialised;
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	
	// @}


protected:

	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CMissionTeamPD
 * defined at entities_game_service/pd_scripts/mission.pds:139
 */
class CMissionTeamPD : public CMissionPD
{

public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	virtual void					clear();
	
	/**
	 * Cast base object to CMissionTeamPD
	 */
	static CMissionTeamPD*			cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CMissionTeamPD
	 */
	static const CMissionTeamPD*	cast(const RY_PDS::IPDBaseData* obj);
	
	/**
	 * Set user factory for this class (as class is indicated as derived, a home made constructor must be provided)
	 */
	static void						setFactory(RY_PDS::TPDFactory userFactory);
	
	/**
	 * Create an object of the CMissionTeamPD class, and declare it to the PDS.
	 */
	static CMissionTeamPD*			create(const uint32 &TemplateId);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CMissionTeamPD();
	virtual ~CMissionTeamPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	virtual void					apply(CPersistentDataRecord &__pdr);
	virtual void					store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name User defined init and release methods
	// @{
		
	/**
	 * Overload those methods to implement init and release behaviours
	 */
	
	virtual void					init();
	virtual void					release();
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	virtual void					pds__init(const uint32 &TemplateId);
	virtual void					pds__destroy();
	virtual void					pds__fetch(RY_PDS::CPData &data);
	virtual void					pds__register();
	virtual void					pds__registerAttributes();
	virtual void					pds__unregister();
	virtual void					pds__unregisterAttributes();
	virtual void					pds__notifyInit();
	virtual void					pds__notifyRelease();
	static void						pds_static__init();
	
	// @}


protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static void						pds_static__setFactory(RY_PDS::TPDFactory userFactory);
	static bool						_FactoryInitialised;
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	
	// @}


protected:

	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CMissionSoloPD
 * defined at entities_game_service/pd_scripts/mission.pds:144
 */
class CMissionSoloPD : public CMissionPD
{

public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	virtual void					clear();
	
	/**
	 * Cast base object to CMissionSoloPD
	 */
	static CMissionSoloPD*			cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CMissionSoloPD
	 */
	static const CMissionSoloPD*	cast(const RY_PDS::IPDBaseData* obj);
	
	/**
	 * Set user factory for this class (as class is indicated as derived, a home made constructor must be provided)
	 */
	static void						setFactory(RY_PDS::TPDFactory userFactory);
	
	/**
	 * Create an object of the CMissionSoloPD class, and declare it to the PDS.
	 */
	static CMissionSoloPD*			create(const uint32 &TemplateId);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CMissionSoloPD();
	virtual ~CMissionSoloPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	virtual void					apply(CPersistentDataRecord &__pdr);
	virtual void					store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name User defined init and release methods
	// @{
		
	/**
	 * Overload those methods to implement init and release behaviours
	 */
	
	virtual void					init();
	virtual void					release();
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	virtual void					pds__init(const uint32 &TemplateId);
	virtual void					pds__destroy();
	virtual void					pds__fetch(RY_PDS::CPData &data);
	virtual void					pds__register();
	virtual void					pds__registerAttributes();
	virtual void					pds__unregister();
	virtual void					pds__unregisterAttributes();
	virtual void					pds__notifyInit();
	virtual void					pds__notifyRelease();
	static void						pds_static__init();
	
	// @}


protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static void						pds_static__setFactory(RY_PDS::TPDFactory userFactory);
	static bool						_FactoryInitialised;
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	
	// @}


protected:

	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CMissionContainerPD
 * defined at entities_game_service/pd_scripts/mission.pds:148
 */
class CMissionContainerPD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	NLMISC::CEntityId				getCharId() const;
	
	CMissionPD*						getMissions(const uint32& __k);
	const CMissionPD*				getMissions(const uint32& __k) const;
	std::map<uint32, CMissionPD*>::iterator	getMissionsBegin();
	std::map<uint32, CMissionPD*>::iterator	getMissionsEnd();
	std::map<uint32, CMissionPD*>::const_iterator	getMissionsBegin() const;
	std::map<uint32, CMissionPD*>::const_iterator	getMissionsEnd() const;
	const std::map<uint32, CMissionPD*> &	getMissions() const;
	void							setMissions(CMissionPD* __v);
	void							deleteFromMissions(const uint32 &__k);
	
	// @}


public:

	/// \name Public Management methods
	// @{
		
	/**
	 * Use these methods to create, load, unload and get
	 * an object from database.
	 */
	
	
	/**
	 * Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.
	 */
	void							clear();
	
	/**
	 * Cast base object to CMissionContainerPD
	 */
	static CMissionContainerPD*		cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CMissionContainerPD
	 */
	static const CMissionContainerPD*	cast(const RY_PDS::IPDBaseData* obj);
	
	/**
	 * Create an object of the CMissionContainerPD class, and declare it to the PDS.
	 */
	static CMissionContainerPD*		create(const NLMISC::CEntityId &CharId);
	
	/**
	 * Destroy an object from the PDS. Caution! Object will no longer exist in database.
	 * Also children (that is objects that belong to this object) are also destroyed.
	 */
	static void						remove(const NLMISC::CEntityId& CharId);
	
	/**
	 * Retrieve an object from the database.
	 * Data are sent asynchronously, so the load callback is called when data are ready.
	 * Use get() to access to the loaded object.
	 */
	static void						load(const NLMISC::CEntityId& CharId);
	
	/**
	 * Setup load callback so client is warned that load succeded or failed.
	 */
	static void						setLoadCallback(void (*callback)(const NLMISC::CEntityId& key, CMissionContainerPD* object));
	
	/**
	 * Unload an object from the client memory. Object still exists in database and can be retrieved again using load.
	 */
	static void						unload(const NLMISC::CEntityId &CharId);
	
	/**
	 * Get an object in client. Object must have been previously loaded from database with a load.
	 */
	static CMissionContainerPD*		get(const NLMISC::CEntityId &CharId);
	
	/**
	 * Return the begin iterator of the global map of CMissionContainerPD
	 */
	static std::map<NLMISC::CEntityId, CMissionContainerPD*>::iterator	begin();
	
	/**
	 * Return the end iterator of the global map of CMissionContainerPD
	 */
	static std::map<NLMISC::CEntityId, CMissionContainerPD*>::iterator	end();
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CMissionContainerPD();
	~CMissionContainerPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	void							apply(CPersistentDataRecord &__pdr);
	void							store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name Attributes
	// @{
		
	/**
	 * Don't modify those value manually, use accessors and mutators above
	 */
	
	NLMISC::CEntityId				_CharId;
	std::map<uint32, CMissionPD*>	_Missions;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const NLMISC::CEntityId &CharId);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__notifyInit();
	void							pds__notifyRelease();
	void							pds__unlinkMissions(uint32 __k);
	static void						pds_static__init();
	
	// @}


protected:

	static std::map<NLMISC::CEntityId,CMissionContainerPD*>	_Map;

protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static void						pds_static__setFactory(RY_PDS::TPDFactory userFactory);
	static bool						_FactoryInitialised;
	static void						pds_static__notifyFailure(uint64 key);
	static void						(*__pds__LoadCallback)(const NLMISC::CEntityId& key, CMissionContainerPD* object);
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	static RY_PDS::IPDBaseData*		pds_static__factory();
	static void						pds_static__fetch(RY_PDS::IPDBaseData *object, RY_PDS::CPData &data);
	
	// @}


protected:

	friend class CMissionPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

	
} // End of EGSPD

#endif
