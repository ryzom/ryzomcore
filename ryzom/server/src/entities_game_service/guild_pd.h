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


#ifndef GUILD_PD_H
#define GUILD_PD_H

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
#include "mission_manager/ai_alias_translator.h"

namespace EGSPD
{
	
//
// Forward declarations
//



//
// Typedefs & Enums
//

/** CGuildPD
 * defined at entities_game_service/pd_scripts/guild.pds:36
 */
class CGuildPD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	TGuildId						getId() const;
	
	uint64							getMoney() const;
	void							setMoney(uint64 __v, bool forceWrite=false);
	
	uint32							getCreationDate() const;
	void							setCreationDate(uint32 __v, bool forceWrite=false);
	
	CPeople::TPeople				getRace() const;
	void							setRace(CPeople::TPeople __v, bool forceWrite=false);
	
	uint64							getIcon() const;
	void							setIcon(uint64 __v, bool forceWrite=false);
	
	uint32							getBuilding() const;
	void							setBuilding(uint32 __v, bool forceWrite=false);
	
	uint32							getVersion() const;
	void							setVersion(uint32 __v, bool forceWrite=false);
	
	CGuildMemberPD*					getMembers(const TCharacterId& __k);
	const CGuildMemberPD*			getMembers(const TCharacterId& __k) const;
	std::map<TCharacterId, CGuildMemberPD*>::iterator	getMembersBegin();
	std::map<TCharacterId, CGuildMemberPD*>::iterator	getMembersEnd();
	std::map<TCharacterId, CGuildMemberPD*>::const_iterator	getMembersBegin() const;
	std::map<TCharacterId, CGuildMemberPD*>::const_iterator	getMembersEnd() const;
	const std::map<TCharacterId, CGuildMemberPD*> &	getMembers() const;
	void							setMembers(CGuildMemberPD* __v);
	void							deleteFromMembers(const TCharacterId &__k);
	
	CGuildFameContainerPD*			getFameContainer();
	const CGuildFameContainerPD*	getFameContainer() const;
	void							setFameContainer(CGuildFameContainerPD* __v);
	
	CGuildContainerPD*				getParent();
	const CGuildContainerPD*		getParent() const;
	
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
	 * Cast base object to CGuildPD
	 */
	static CGuildPD*				cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CGuildPD
	 */
	static const CGuildPD*			cast(const RY_PDS::IPDBaseData* obj);
	
	/**
	 * Set user factory for this class (as class is indicated as derived, a home made constructor must be provided)
	 */
	static void						setFactory(RY_PDS::TPDFactory userFactory);
	
	/**
	 * Create an object of the CGuildPD class, and declare it to the PDS.
	 */
	static CGuildPD*				create(const TGuildId &Id);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CGuildPD();
	virtual ~CGuildPD();
	
	// @}


public:

	/// \name Persistent methods declaration
	// @{
		
	void							apply(CPersistentDataRecord &__pdr);
	void							store(CPersistentDataRecord &__pdr) const;
	
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
	
	TGuildId						_Id;
	uint64							_Money;
	uint32							_CreationDate;
	CPeople::TPeople				_Race;
	uint64							_Icon;
	uint32							_Building;
	uint32							_Version;
	std::map<TCharacterId, CGuildMemberPD*>	_Members;
	CGuildFameContainerPD*			_FameContainer;
	CGuildContainerPD*				_Parent;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const TGuildId &Id);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__setParent(CGuildContainerPD* __parent);
	void							pds__setParentUnnotified(CGuildContainerPD* __parent);
	void							pds__notifyInit();
	void							pds__notifyRelease();
	void							pds__unlinkMembers(TCharacterId __k);
	void							pds__unlinkFameContainer(NLMISC::CEntityId dummy);
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

	friend class CGuildContainerPD;
	friend class CGuildFameContainerPD;
	friend class CGuildMemberPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

	
} // End of EGSPD

#endif
