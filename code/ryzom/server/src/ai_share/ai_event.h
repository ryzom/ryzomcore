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




#ifndef RY_AI_EVENT_H
#define RY_AI_EVENT_H

#include <string>

#include "nel/misc/stream.h"
#include "nel/misc/entity_id.h"

//------------------------------MESSAGES DESCRIPTION---------------------------------
// STUN
//		CEntityId CreatureId
// STUN_END
//		CEntityId CreatureId
// AGGRO
//		CEntityId CreatureId
//		CEntityId EntityId
//		sint32	  Modifier
// SURVIE
//		CEntityId CreatureId
//		CEntityId EntityId
//		sint32	  Modifier
// FEAR
//		CEntityId CreatureId
//		CEntityId EntityId
// FEAR_END
//		CEntityId CreatureId
//		CEntityId EntityId
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// class CAIEventType
//-----------------------------------------------------------------------------------
// this class encapsulates the class id for classes derived from IAIEvent
// for now class names are limitted to 8 characters - if need be this can be
// extended at a later date but will require services to synchronise event id
// tables at startup

class CAIEventType
{
public:
	CAIEventType()
	{ _val = (uint64)0;}

	CAIEventType(const CAIEventType &other)
	{
		*this=other;
	}
	
	CAIEventType(const char *typeName)
	{
		// copy text from input string to _val variable
		uint i;
		for (i=0;i<8 && typeName[i];++i)
			((char *)&_val)[i]=typeName[i];

		// if type name is longer than 8 characters it won't fit in an int64!
		nlassert(typeName[i]==0);
		
		// pad out _val variable with 0s
		while(i<8)
			((char *)&_val)[i++]=0;
	}
	CAIEventType(const std::string &typeName)
	{
		*this=CAIEventType(typeName.c_str());
	}

	const CAIEventType &operator=(const CAIEventType &other)
	{
		_val=other._val;
	} 
	bool operator==(const CAIEventType &other)	const
	{
		return _val==other._val;
	}
	bool operator!=(const CAIEventType &other)	const
	{
		return _val!=other._val;
	}
	bool operator<=(const CAIEventType &other)	const
	{
		return _val<=other._val;
	}
	bool operator>=(const CAIEventType &other)	const
	{
		return _val>=other._val;
	}
	bool operator<(const CAIEventType &other)	const
	{
		return _val<other._val;
	}
	bool operator>(const CAIEventType &other)	const
	{
		return _val>other._val;
	}

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(_val);
	}

	std::string toString() const
	{
		return NLMISC::toString("%8.8s",&_val);
	}

private:
	uint64 _val;
};


//-----------------------------------------------------------------------------------
// base class IAIEvent
//-----------------------------------------------------------------------------------
// This is the base class for classes of event sent from the game dev services to
// the AI. Note that the serial has a special syntax to allow for skipping of 
// unrecognised events.

class IAIEvent
{
	NL_INSTANCE_COUNTER_DECL(IAIEvent);
public:

	virtual ~IAIEvent() {}

	// this is the name of the class
	// for now it is limited to 8 letters - wil be extended at a later date if need be
	virtual const CAIEventType &type() const = 0;

	// serial()
	// note serial should serialise: <Type> <uint16 sizeof(EventClass)> <event_parameters>
	// the 'read' version of the serial should test the <sizeof> to ensure version robustness
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) = 0;
};


/**
 * CAIStunEvent : event STUN
 * \author Fleury David
 * \author Nevrax France
 * \date 2003
 */
class CAIStunEvent: public IAIEvent
{
	NL_INSTANCE_COUNTER_DECL(CAIStunEvent);
public:

	// this is the name of the class
	// for now it is limited to 8 letters - will be extended at a later date if need be
	virtual const CAIEventType &type() const { static CAIEventType type("STUN"); return type; }

	// serial()
	// note serial should serialise: <Type> <uint16 sizeof(EventClass)> <event_parameters>
	// the 'read' version of the serial should test the <sizeof> to ensure version robustness
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
public:
	/// the stunned creature id
	NLMISC::CEntityId	CreatureId;
};


/**
 * CAIStunEndEvent : event STUN_END
 * \author Fleury David
 * \author Nevrax France
 * \date 2003
 */
class CAIStunEndEvent: public IAIEvent
{
	NL_INSTANCE_COUNTER_DECL(CAIStunEndEvent);
public:

	// this is the name of the class
	// for now it is limited to 8 letters - will be extended at a later date if need be
	virtual const CAIEventType &type() const { static CAIEventType type("STUN_END"); return type; }

	// serial()
	// note serial should serialise: <Type> <uint16 sizeof(EventClass)> <event_parameters>
	// the 'read' version of the serial should test the <sizeof> to ensure version robustness
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
public:
	/// the waked creature id
	NLMISC::CEntityId	CreatureId;
};



/**
 * CAIAggroEvent : event AGGRO
 * \author Fleury David
 * \author Nevrax France
 * \date 2003
 */
class CAIAggroEvent: public IAIEvent
{
public:
	CAIAggroEvent() : AggroModifier(0)
	{}

	// this is the name of the class
	// for now it is limited to 8 letters - will be extended at a later date if need be
	virtual const CAIEventType &type() const { static CAIEventType type("AGGRO"); return type; }

	// serial()
	// note serial should serialise: <Type> <uint16 sizeof(EventClass)> <event_parameters>
	// the 'read' version of the serial should test the <sizeof> to ensure version robustness
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
public:
	/// the creature Id
	NLMISC::CEntityId	CreatureId;

	/// the entity Id affected by the aggro
	NLMISC::CEntityId	EntityId;

	/// aggro modifier
	sint32				AggroModifier;
};




/**
 * CAISurvivalInstinctEvent : event SURVIE
 * \author Fleury David
 * \author Nevrax France
 * \date 2003
 */
class CAISurvivalInstinctEvent: public IAIEvent
{
public:
	// this is the name of the class
	// for now it is limited to 8 letters - will be extended at a later date if need be
	virtual const CAIEventType &type() const { static CAIEventType type("SURVIE"); return type; }

	// serial()
	// note serial should serialise: <Type> <uint16 sizeof(EventClass)> <event_parameters>
	// the 'read' version of the serial should test the <sizeof> to ensure version robustness
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
public:
	/// the affected creature id
	NLMISC::CEntityId	CreatureId;

	/// the entity for which the creature survival instinct is modified
	NLMISC::CEntityId	EntityId;

	/// the modifier
	sint32				Modifier;
};




/**
 * CAIFearEvent : event FEAR
 * \author Fleury David
 * \author Nevrax France
 * \date 2003
 */
class CAIFearEvent: public IAIEvent
{
public:
	// this is the name of the class
	// for now it is limited to 8 letters - will be extended at a later date if need be
	virtual const CAIEventType &type() const { static CAIEventType type("FEAR"); return type; }

	// serial()
	// note serial should serialise: <Type> <uint16 sizeof(EventClass)> <event_parameters>
	// the 'read' version of the serial should test the <sizeof> to ensure version robustness
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
public:
	/// the creature id
	NLMISC::CEntityId	CreatureId;
	/// the entity feared by the creature
	NLMISC::CEntityId	EntityId;
};


/**
 * CAIFearEndEvent : event FEAR_END
 * \author Fleury David
 * \author Nevrax France
 * \date 2003
 */
class CAIFearEndEvent: public IAIEvent
{
public:
	// this is the name of the class
	// for now it is limited to 8 letters - will be extended at a later date if need be
	virtual const CAIEventType &type() const { static CAIEventType type("FEAR_END"); return type; }

	// serial()
	// note serial should serialise: <Type> <uint16 sizeof(EventClass)> <event_parameters>
	// the 'read' version of the serial should test the <sizeof> to ensure version robustness
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
public:
	/// the creature id
	NLMISC::CEntityId	CreatureId;
	/// the entity no longer feared by the creature
	NLMISC::CEntityId	EntityId;
};

/**
 * CAIHungerEvent : event HUNGER
 * \author Fleury David
 * \author Nevrax France
 * \date 2003
 */
class CAIHungerEvent: public IAIEvent
{
public:
	// this is the name of the class
	// for now it is limited to 8 letters - will be extended at a later date if need be
	virtual const CAIEventType &type() const { static CAIEventType type("HUNGER"); return type; }

	// serial()
	// note serial should serialise: <Type> <uint16 sizeof(EventClass)> <event_parameters>
	// the 'read' version of the serial should test the <sizeof> to ensure version robustness
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
public:
	/// the affected creature id
	NLMISC::CEntityId	CreatureId;

	/// the modifier
	sint32				Modifier;
};



#endif

