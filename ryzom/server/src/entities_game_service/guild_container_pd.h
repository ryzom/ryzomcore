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


#ifndef GUILD_CONTAINER_PD_H
#define GUILD_CONTAINER_PD_H

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

/** CGuildContainerPD
 * defined at entities_game_service/pd_scripts/guild.pds:80
 */
class CGuildContainerPD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	TGuildId						getGMGuild() const;
	void							setGMGuild(TGuildId __v, bool forceWrite=false);
	
	CGuildPD*						getGuilds(const TGuildId& __k);
	const CGuildPD*					getGuilds(const TGuildId& __k) const;
	std::map<TGuildId, CGuildPD*>::iterator	getGuildsBegin();
	std::map<TGuildId, CGuildPD*>::iterator	getGuildsEnd();
	std::map<TGuildId, CGuildPD*>::const_iterator	getGuildsBegin() const;
	std::map<TGuildId, CGuildPD*>::const_iterator	getGuildsEnd() const;
	const std::map<TGuildId, CGuildPD*> &	getGuilds() const;
	void							setGuilds(CGuildPD* __v);
	void							deleteFromGuilds(const TGuildId &__k);
	
	uint8							getDummy() const;
	
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
	 * Cast base object to CGuildContainerPD
	 */
	static CGuildContainerPD*		cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CGuildContainerPD
	 */
	static const CGuildContainerPD*	cast(const RY_PDS::IPDBaseData* obj);
	
	/**
	 * Create an object of the CGuildContainerPD class, and declare it to the PDS.
	 */
	static CGuildContainerPD*		create(const uint8 &Dummy);
	
	/**
	 * Destroy an object from the PDS. Caution! Object will no longer exist in database.
	 * Also children (that is objects that belong to this object) are also destroyed.
	 */
	static void						remove(const uint8& Dummy);
	
	/**
	 * Retrieve an object from the database.
	 * Data are sent asynchronously, so the load callback is called when data are ready.
	 * Use get() to access to the loaded object.
	 */
	static void						load(const uint8& Dummy);
	
	/**
	 * Setup load callback so client is warned that load succeded or failed.
	 */
	static void						setLoadCallback(void (*callback)(const uint8& key, CGuildContainerPD* object));
	
	/**
	 * Unload an object from the client memory. Object still exists in database and can be retrieved again using load.
	 */
	static void						unload(const uint8 &Dummy);
	
	/**
	 * Get an object in client. Object must have been previously loaded from database with a load.
	 */
	static CGuildContainerPD*		get(const uint8 &Dummy);
	
	/**
	 * Return the begin iterator of the global map of CGuildContainerPD
	 */
	static std::map<uint8, CGuildContainerPD*>::iterator	begin();
	
	/**
	 * Return the end iterator of the global map of CGuildContainerPD
	 */
	static std::map<uint8, CGuildContainerPD*>::iterator	end();
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CGuildContainerPD();
	~CGuildContainerPD();
	
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
	
	TGuildId						_GMGuild;
	std::map<TGuildId, CGuildPD*>	_Guilds;
	uint8							_Dummy;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const uint8 &Dummy);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__notifyInit();
	void							pds__notifyRelease();
	void							pds__unlinkGuilds(TGuildId __k);
	static void						pds_static__init();
	
	// @}


protected:

	static std::map<uint8,CGuildContainerPD*>	_Map;

protected:

	/// \name Default Factory and Fetch methods
	// @{
		
	static void						pds_static__setFactory(RY_PDS::TPDFactory userFactory);
	static bool						_FactoryInitialised;
	static void						pds_static__notifyFailure(uint64 key);
	static void						(*__pds__LoadCallback)(const uint8& key, CGuildContainerPD* object);
	static RY_PDS::CIndexAllocator	_IndexAllocator;
	static RY_PDS::IPDBaseData*		pds_static__factory();
	static void						pds_static__fetch(RY_PDS::IPDBaseData *object, RY_PDS::CPData &data);
	
	// @}


protected:

	friend class CGuildPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

	
} // End of EGSPD

#endif
