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



#include "stdpch.h"
#include "ai_event.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(IAIEvent);
NL_INSTANCE_COUNTER_IMPL(CAIStunEvent);
NL_INSTANCE_COUNTER_IMPL(CAIStunEndEvent);

//------------------------------IMPORTANT----------------------------------
// The EventType isn't read in the serial as it is used in the read process
// to get the right event type (in the AI service)
//-------------------------------------------------------------------------



//--------------------------------------------------------------
//						CAIStunEvent::serial()  
//--------------------------------------------------------------
void CAIStunEvent::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading() )
	{
//		CAIEventType type;
		uint16 size;
//		f.serial(type);
		f.serial(size);

		if (/*type != CAIEventType("STUN") || */size != sizeof(CAIStunEvent))
		{
			CreatureId = NLMISC::CEntityId();
			try
			{
				// seek takes a param in bytes (8 bits)
				f.seek( size, NLMISC::IStream::current);
			}
			catch(const ESeekNotSupported &)
			{
				uint8 tmp;
				for (uint i = 0 ; i < size ; ++i)
					f.serial(tmp);
			}
		}
		else
		{
			f.serial(CreatureId);
		}
	}
	else
	{
		CAIEventType type("STUN");
		uint16 size = sizeof(CAIStunEvent);

		f.serial(type);
		f.serial(size);
		f.serial(CreatureId);
	}
} // CAIStunEvent::serial //



//--------------------------------------------------------------
//						CAIAggroEvent::serial()  
//--------------------------------------------------------------
void CAIAggroEvent::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading() )
	{
//		CAIEventType type;
		uint16 size;
//		f.serial(type);
		f.serial(size);

		if (/*type != CAIEventType("AGGRO") || */size != sizeof(CAIAggroEvent))
		{
			CreatureId = NLMISC::CEntityId();
			EntityId = NLMISC::CEntityId();
			AggroModifier = 0;
			try
			{
				// seek takes a param in bytes (8 bits)
				f.seek( size, NLMISC::IStream::current);
			}
			catch(const ESeekNotSupported &)
			{
				uint8 tmp;
				for (uint i = 0 ; i < size ; ++i)
					f.serial(tmp);
			}
		}
		else
		{
			f.serial(CreatureId);
			f.serial(EntityId);
			f.serial(AggroModifier);
		}
	}
	else
	{
		CAIEventType type("AGGRO");
		uint16 size = sizeof(CAIAggroEvent);

		f.serial(type);
		f.serial(size);
		f.serial(CreatureId);
		f.serial(EntityId);
		f.serial(AggroModifier);
	}
} // CAIAggroEvent::serial //




//--------------------------------------------------------------
//						CAIStunEndEvent::serial()  
//--------------------------------------------------------------
void CAIStunEndEvent::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading() )
	{
	//	CAIEventType type;
		uint16 size;
	//	f.serial(type);
		f.serial(size);

		if (/*type != CAIEventType("STUN_END") ||*/ size != sizeof(CAIStunEndEvent))
		{
			CreatureId = NLMISC::CEntityId();
			try
			{
				// seek takes a param in bytes (8 bits)
				f.seek( size, NLMISC::IStream::current);
			}
			catch(const ESeekNotSupported &)
			{
				uint8 tmp;
				for (uint i = 0 ; i < size ; ++i)
					f.serial(tmp);
			}
		}
		else
		{
			f.serial(CreatureId);
		}
	}
	else
	{
		CAIEventType type("STUN_END");
		uint16 size = sizeof(CAIStunEndEvent);

		f.serial(type);
		f.serial(size);
		f.serial(CreatureId);
	}
} // CAIStunEndEvent::serial //




//--------------------------------------------------------------
//						CAISurvivalInstinctEvent::serial()  
//--------------------------------------------------------------
void CAISurvivalInstinctEvent::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading() )
	{
	//	CAIEventType type;
		uint16 size;
	//	f.serial(type);
		f.serial(size);

		if (/*type != CAIEventType("SURVIE") ||*/ size != sizeof(CAISurvivalInstinctEvent))
		{
			CreatureId = NLMISC::CEntityId();
			EntityId = NLMISC::CEntityId();
			Modifier = 0;
			try
			{
				// seek takes a param in bytes (8 bits)
				f.seek( size, NLMISC::IStream::current);
			}
			catch(const ESeekNotSupported &)
			{
				uint8 tmp;
				for (uint i = 0 ; i < size ; ++i)
					f.serial(tmp);
			}
		}
		else
		{
			f.serial(CreatureId);
			f.serial(EntityId);
			f.serial(Modifier);
		}
	}
	else
	{
		CAIEventType type("SURVIE");
		uint16 size = sizeof(CAISurvivalInstinctEvent);

		f.serial(type);
		f.serial(size);
		f.serial(CreatureId);
		f.serial(EntityId);
		f.serial(Modifier);
		
	}
} // CAISurvivalInstinctEvent::serial //



//--------------------------------------------------------------
//						CAIFearEvent::serial()  
//--------------------------------------------------------------
void CAIFearEvent::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading() )
	{
	//	CAIEventType type;
		uint16 size;
	//	f.serial(type);
		f.serial(size);

		if (/*type != CAIEventType("FEAR") ||*/ size != sizeof(CAIFearEvent))
		{
			CreatureId = NLMISC::CEntityId();
			EntityId = NLMISC::CEntityId();
			try
			{
				// seek takes a param in bytes (8 bits)
				f.seek( size, NLMISC::IStream::current);
			}
			catch(const ESeekNotSupported &)
			{
				uint8 tmp;
				for (uint i = 0 ; i < size ; ++i)
					f.serial(tmp);
			}
		}
		else
		{
			f.serial(CreatureId);
			f.serial(EntityId);
		}
	}
	else
	{
		CAIEventType type("FEAR");
		uint16 size = sizeof(CAIFearEvent);

		f.serial(type);
		f.serial(size);
		f.serial(CreatureId);
		f.serial(EntityId);
	}
} // CAIFearEvent::serial //


//--------------------------------------------------------------
//						CAIFearEndEvent::serial()  
//--------------------------------------------------------------
void CAIFearEndEvent::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading() )
	{
		//CAIEventType type;
		uint16 size;
		//f.serial(type);
		f.serial(size);

		if (/*type != CAIEventType("FEAR_END") ||*/ size != sizeof(CAIFearEndEvent))
		{
			CreatureId = NLMISC::CEntityId();
			EntityId = NLMISC::CEntityId();
			try
			{
				// seek takes a param in bytes (8 bits)
				f.seek( size, NLMISC::IStream::current);
			}
			catch(const ESeekNotSupported &)
			{
				uint8 tmp;
				for (uint i = 0 ; i < size ; ++i)
					f.serial(tmp);
			}
		}
		else
		{
			f.serial(CreatureId);
			f.serial(EntityId);
		}
	}
	else
	{
		CAIEventType type("FEAR_END");
		uint16 size = sizeof(CAIFearEndEvent);

		f.serial(type);
		f.serial(size);
		f.serial(CreatureId);
		f.serial(EntityId);
	}
} // CAIFearEndEvent::serial //



//--------------------------------------------------------------
//						CAIHungerEvent::serial()  
//--------------------------------------------------------------
void CAIHungerEvent::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading() )
	{
		//CAIEventType type;
		uint16 size;
		//f.serial(type);
		f.serial(size);

		if (/*type != CAIEventType("HUNGER") ||*/ size != sizeof(CAIHungerEvent))
		{
			CreatureId = NLMISC::CEntityId();
			Modifier = 0;
			try
			{
				// seek takes a param in bytes (8 bits)
				f.seek( size, NLMISC::IStream::current);
			}
			catch(const ESeekNotSupported &)
			{
				uint8 tmp;
				for (uint i = 0 ; i < size ; ++i)
					f.serial(tmp);
			}
		}
		else
		{
			f.serial(CreatureId);
			f.serial(Modifier);
		}
	}
	else
	{
		CAIEventType type("HUNGER");
		uint16 size = sizeof(CAIHungerEvent);

		f.serial(type);
		f.serial(size);
		f.serial(CreatureId);
		f.serial(Modifier);		
	}
} // CAIHungerEvent::serial //

