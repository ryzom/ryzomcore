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


#ifndef FAME_PD_H
#define FAME_PD_H

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

namespace EGSPD
{
	
//
// Forward declarations
//



//
// Typedefs & Enums
//

/** TFameTrend
 * defined at entities_game_service/pd_scripts/fame.pds:4
 */
class CFameTrend
{

public:

	/// \name Enum values
	// @{
		
	enum TFameTrend
	{
		FameUpward = 0,
		FameDownward = 1,
		FameSteady = 2,
		___TFameTrend_useSize = 3,
		Unknown = 3,
		EndFameTrend = 3,
	};
	
	// @}


public:

	/// \name Conversion methods
	// @{
		
	/**
	 * Use these methods to convert from enum value to string (and vice versa)
	 */
	
	static const std::string&		toString(TFameTrend v);
	static CFameTrend::TFameTrend	fromString(const std::string& v);
	
	// @}


private:

	/// \name Enum initialisation
	// @{
		
	static void						init();
	static bool						_Initialised;
	static std::string				_UnknownString;
	static std::vector<std::string>	_StrTable;
	static std::map<std::string, TFameTrend>	_ValueMap;
	
	// @}

};

/** CFameContainerEntryPD
 * defined at entities_game_service/pd_scripts/fame.pds:11
 */
class CFameContainerEntryPD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	NLMISC::CSheetId				getSheet() const;
	
	sint32							getFame() const;
	void							setFame(sint32 __v, bool forceWrite=false);
	
	sint32							getFameMemory() const;
	void							setFameMemory(sint32 __v, bool forceWrite=false);
	
	CFameTrend::TFameTrend			getLastFameChangeTrend() const;
	void							setLastFameChangeTrend(CFameTrend::TFameTrend __v, bool forceWrite=false);
	
	CFameContainerPD*				getParent();
	const CFameContainerPD*			getParent() const;
	
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
	 * Cast base object to CFameContainerEntryPD
	 */
	static CFameContainerEntryPD*	cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CFameContainerEntryPD
	 */
	static const CFameContainerEntryPD*	cast(const RY_PDS::IPDBaseData* obj);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CFameContainerEntryPD();
	~CFameContainerEntryPD();
	
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
	
	NLMISC::CSheetId				_Sheet;
	sint32							_Fame;
	sint32							_FameMemory;
	CFameTrend::TFameTrend			_LastFameChangeTrend;
	CFameContainerPD*				_Parent;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const NLMISC::CSheetId &Sheet);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__setParent(CFameContainerPD* __parent);
	void							pds__setParentUnnotified(CFameContainerPD* __parent);
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

	friend class CFameContainerPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CFameContainerPD
 * defined at entities_game_service/pd_scripts/fame.pds:20
 */
class CFameContainerPD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	NLMISC::CEntityId				getContId() const;
	
	CFameContainerEntryPD*			getEntries(const NLMISC::CSheetId& __k);
	const CFameContainerEntryPD*	getEntries(const NLMISC::CSheetId& __k) const;
	std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator	getEntriesBegin();
	std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator	getEntriesEnd();
	std::map<NLMISC::CSheetId, CFameContainerEntryPD>::const_iterator	getEntriesBegin() const;
	std::map<NLMISC::CSheetId, CFameContainerEntryPD>::const_iterator	getEntriesEnd() const;
	const std::map<NLMISC::CSheetId, CFameContainerEntryPD> &	getEntries() const;
	CFameContainerEntryPD*			addToEntries(const NLMISC::CSheetId &__k);
	void							deleteFromEntries(const NLMISC::CSheetId &__k);
	
	uint32							getLastGuildStatusChange() const;
	void							setLastGuildStatusChange(uint32 __v, bool forceWrite=false);
	
	uint32							getLastFameChangeDate() const;
	void							setLastFameChangeDate(uint32 __v, bool forceWrite=false);
	
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
	 * Cast base object to CFameContainerPD
	 */
	static CFameContainerPD*		cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CFameContainerPD
	 */
	static const CFameContainerPD*	cast(const RY_PDS::IPDBaseData* obj);
	
	/**
	 * Create an object of the CFameContainerPD class, and declare it to the PDS.
	 */
	static CFameContainerPD*		create(const NLMISC::CEntityId &ContId);
	
	/**
	 * Destroy an object from the PDS. Caution! Object will no longer exist in database.
	 * Also children (that is objects that belong to this object) are also destroyed.
	 */
	static void						remove(const NLMISC::CEntityId& ContId);
	
	/**
	 * Retrieve an object from the database.
	 * Data are sent asynchronously, so the load callback is called when data are ready.
	 * Use get() to access to the loaded object.
	 */
	static void						load(const NLMISC::CEntityId& ContId);
	
	/**
	 * Setup load callback so client is warned that load succeded or failed.
	 */
	static void						setLoadCallback(void (*callback)(const NLMISC::CEntityId& key, CFameContainerPD* object));
	
	/**
	 * Unload an object from the client memory. Object still exists in database and can be retrieved again using load.
	 */
	static void						unload(const NLMISC::CEntityId &ContId);
	
	/**
	 * Get an object in client. Object must have been previously loaded from database with a load.
	 */
	static CFameContainerPD*		get(const NLMISC::CEntityId &ContId);
	
	/**
	 * Return the begin iterator of the global map of CFameContainerPD
	 */
	static std::map<NLMISC::CEntityId, CFameContainerPD*>::iterator	begin();
	
	/**
	 * Return the end iterator of the global map of CFameContainerPD
	 */
	static std::map<NLMISC::CEntityId, CFameContainerPD*>::iterator	end();
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CFameContainerPD();
	virtual ~CFameContainerPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	virtual void					apply(CPersistentDataRecord &__pdr);
	virtual void					store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name Attributes
	// @{
		
	/**
	 * Don't modify those value manually, use accessors and mutators above
	 */
	
	NLMISC::CEntityId				_ContId;
	std::map<NLMISC::CSheetId, CFameContainerEntryPD>	_Entries;
	uint32							_LastGuildStatusChange;
	uint32							_LastFameChangeDate;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	virtual void					pds__init(const NLMISC::CEntityId &ContId);
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

	static std::map<NLMISC::CEntityId,CFameContainerPD*>	_Map;

protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static void						pds_static__setFactory(RY_PDS::TPDFactory userFactory);
	static bool						_FactoryInitialised;
	static void						pds_static__notifyFailure(uint64 key);
	static void						(*__pds__LoadCallback)(const NLMISC::CEntityId& key, CFameContainerPD* object);
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	static RY_PDS::IPDBaseData*		pds_static__factory();
	static void						pds_static__fetch(RY_PDS::IPDBaseData *object, RY_PDS::CPData &data);
	
	// @}


protected:

	friend class CFameContainerEntryPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

/** CGuildFameContainerPD
 * defined at entities_game_service/pd_scripts/fame.pds:30
 */
class CGuildFameContainerPD : public CFameContainerPD
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	CGuildPD*						getParent();
	const CGuildPD*					getParent() const;
	
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
	 * Cast base object to CGuildFameContainerPD
	 */
	static CGuildFameContainerPD*	cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CGuildFameContainerPD
	 */
	static const CGuildFameContainerPD*	cast(const RY_PDS::IPDBaseData* obj);
	
	/**
	 * Create an object of the CGuildFameContainerPD class, and declare it to the PDS.
	 */
	static CGuildFameContainerPD*	create(const NLMISC::CEntityId &ContId);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CGuildFameContainerPD();
	virtual ~CGuildFameContainerPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	virtual void					apply(CPersistentDataRecord &__pdr);
	virtual void					store(CPersistentDataRecord &__pdr) const;
	
	// @}


protected:

	/// \name Attributes
	// @{
		
	/**
	 * Don't modify those value manually, use accessors and mutators above
	 */
	
	CGuildPD*						_Parent;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	virtual void					pds__init(const NLMISC::CEntityId &ContId);
	virtual void					pds__destroy();
	virtual void					pds__fetch(RY_PDS::CPData &data);
	virtual void					pds__register();
	virtual void					pds__registerAttributes();
	virtual void					pds__unregister();
	virtual void					pds__unregisterAttributes();
	void							pds__setParent(CGuildPD* __parent);
	void							pds__setParentUnnotified(CGuildPD* __parent);
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
	static RY_PDS::IPDBaseData*		pds_static__factory();
	
	// @}


protected:

	friend class CGuildContainerPD;
	friend class CGuildPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

	
} // End of EGSPD

#endif
