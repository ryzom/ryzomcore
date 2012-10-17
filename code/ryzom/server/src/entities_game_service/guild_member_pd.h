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


#ifndef GUILD_MEMBER_PD_H
#define GUILD_MEMBER_PD_H

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
#include "guild_manager/guild_manager_interface.h"

namespace EGSPD
{
	
//
// Forward declarations
//



//
// Typedefs & Enums
//

/** CGuildMemberPD
 * defined at entities_game_service/pd_scripts/guild.pds:6
 */
class CGuildMemberPD : public RY_PDS::IPDBaseData
{

public:

	/// \name Accessors and Mutators methods
	// @{
		
	/**
	 * Use these methods to change a value, add or delete elements.
	 */
	
	TCharacterId					getId() const;
	
	CGuildGrade::TGuildGrade		getGrade() const;
	void							setGrade(CGuildGrade::TGuildGrade __v, bool forceWrite=false);
	
	uint32							getEnterTime() const;
	void							setEnterTime(uint32 __v, bool forceWrite=false);
	
	CGuildPD*						getGuild();
	const CGuildPD*					getGuild() const;
	
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
	 * Cast base object to CGuildMemberPD
	 */
	static CGuildMemberPD*			cast(RY_PDS::IPDBaseData* obj);
	
	/**
	 * Cast base object to const CGuildMemberPD
	 */
	static const CGuildMemberPD*	cast(const RY_PDS::IPDBaseData* obj);
	
	/**
	 * Set user factory for this class (as class is indicated as derived, a home made constructor must be provided)
	 */
	static void						setFactory(RY_PDS::TPDFactory userFactory);
	
	/**
	 * Create an object of the CGuildMemberPD class, and declare it to the PDS.
	 */
	static CGuildMemberPD*			create(const TCharacterId &Id);
	
	// @}


public:

	/// \name Public constructor
	// @{
		
	/**
	 * This constructor is public to allow direct instanciation of the class
	 */
	
	CGuildMemberPD();
	virtual ~CGuildMemberPD();
	
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
	
	TCharacterId					_Id;
	CGuildGrade::TGuildGrade		_Grade;
	uint32							_EnterTime;
	CGuildPD*						_Guild;
	
	// @}


protected:

	/// \name Internal Management methods
	// @{
		
	void							pds__init(const TCharacterId &Id);
	void							pds__destroy();
	void							pds__fetch(RY_PDS::CPData &data);
	void							pds__register();
	void							pds__registerAttributes();
	void							pds__unregister();
	void							pds__unregisterAttributes();
	void							pds__setParent(CGuildPD* __parent);
	void							pds__setParentUnnotified(CGuildPD* __parent);
	void							pds__notifyInit();
	void							pds__notifyRelease();
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
	friend class CGuildPD;
	friend class RY_PDS::CPDSLib;
	friend void EGSPD::init(uint32);
};

	
} // End of EGSPD

#endif
