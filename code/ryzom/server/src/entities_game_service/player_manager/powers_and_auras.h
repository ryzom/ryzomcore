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



#ifndef RY_POWERS_AND_AURAS_H
#define RY_POWERS_AND_AURAS_H

/**
 * struct used to keep powers activation date
 */
struct CPowerActivationDate
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	POWERS::TPowerType	PowerType;
	uint16				ConsumableFamilyId; // ~0 mean is not a consumable who create this power
	NLMISC::TGameCycle	DeactivationDate;
	NLMISC::TGameCycle	ActivationDate;

	CPowerActivationDate() : PowerType(POWERS::UnknownType), ConsumableFamilyId((uint16)~0), DeactivationDate(0), ActivationDate(0)
	{}

	CPowerActivationDate(POWERS::TPowerType type, uint16 consumableFamilyId, NLMISC::TGameCycle dateOff, NLMISC::TGameCycle dateOn) : PowerType(type), ConsumableFamilyId(consumableFamilyId), DeactivationDate(dateOff), ActivationDate(dateOn)
	{}

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
};

/**
 * struct used to keep powers activation date
 */
struct CPowerActivationDateVector
{

	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	std::vector <CPowerActivationDate>	PowerActivationDates;
	bool doNotClear;

	CPowerActivationDateVector() { doNotClear = true; }

	void clear();

	/// remove only consumable entries
	void clearConsumable();

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/// remove entries for which date has been reached
	void cleanVector();

	/// is power allowed ? also clean the vector
	bool isPowerAllowed(POWERS::TPowerType type, uint16 consumableFamilyId, NLMISC::TGameCycle &endDate);

	void writeUsablePowerFlags( uint32 &usablePowerFlags);

	void activate();
};

/************************************************************************/
/*                                                                      */
/************************************************************************/

/**
 * struct used to keep aura activation date
 */
struct CAuraActivationDateVector
{
private:
	std::vector <CPowerActivationDate>	_AuraActivationDates;
	std::vector <NLMISC::CEntityId>		_AuraUsers;

public:

	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	void clear();

	void disableAura(POWERS::TPowerType type, NLMISC::TGameCycle startDate, NLMISC::TGameCycle endDate, const NLMISC::CEntityId &userId);

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/// remove entries for which date has been reached
	void cleanVector();

	bool isAuraEffective(POWERS::TPowerType type, const NLMISC::CEntityId &user, NLMISC::TGameCycle &endDate);

	void activate();
};

/************************************************************************/
/*                                                                      */
/************************************************************************/

/**
 * struct used to keep consumable families overdose timer end date
 */
struct CConsumableOverdoseTimer
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	uint16	Family;
	NLMISC::TGameCycle	ActivationDate;

	CConsumableOverdoseTimer() : Family(0), ActivationDate(0)
	{}

	CConsumableOverdoseTimer(uint16 type, NLMISC::TGameCycle date) : Family(type), ActivationDate(date)
	{}
};

/**
 * struct used to keep consumable families overdose timer end date
 */
struct CConsumableOverdoseTimerVector
{

	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	std::vector <CConsumableOverdoseTimer>	Dates;

	inline void clear() { Dates.clear(); }

	/// remove entries for which date has been reached, return true if entry are removed
	bool cleanVector();

	/// can consume this family ? also clean the vector
	bool canConsume(uint16 family, NLMISC::TGameCycle &endDate);

	void activate();
};



#endif //RY_POWERS_AND_AURAS_H


/* End of powers_and_auras.h */
