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

#ifndef R2_MESSAGES_H
#define R2_MESSAGES_H

// stl
#include <string>

// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/entity_id.h"

// game share
#include "game_share/base_types.h"

#include "r2_types.h"
#include "object.h" //for CObjectSerializer
//#include "scenario.h"
#include "small_string_manager.h"



namespace R2
{

class CAnimationProp
{
public:
	enum TNpcAnimationProperty {

		Spawnable=0x00000001, //Object that are not in permanent content
		Alive=0x00000002, //Plant, Herbivore, Carnivore, Cuthroat
		Controlable=0x00000004, //As Living without plants
		Speaking=0x00000008,//Human
		SpeakedAs = (1 << 4), //
		Controled = (1 << 5),
		Grouped = (1 << 6)

	};

};

	// message sent by EGS to GPMS to set player flags
	class CMessageSetGPMSPlayerFlags
	{
	public:
		// ctor - with parameterised initialisation of all data fields
		CMessageSetGPMSPlayerFlags(	TDataSetRow playerIndex, bool limitSpeed ):
			PlayerIndex(playerIndex),
			LimitSpeed(limitSpeed)
		{
		}

		// ctor - default
		CMessageSetGPMSPlayerFlags()
		{
			LimitSpeed= false;
		}

		// serial
		void serial(NLMISC::IStream& stream)
		{
			stream.serial(PlayerIndex);
			stream.serial(LimitSpeed);
		}

		// public data
		TDataSetRow PlayerIndex;
		bool		LimitSpeed;
	};




class CClientMessageAdventureUserConnection
{
public:

	CClientMessageAdventureUserConnection()
	{
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(SessionId);
		stream.serial(AiInstance);
		stream.serial(EditSlotId);
		stream.serial(HighLevel);
		stream.serial(Mode);
		stream.serial(MustTp);
		stream.serial(InCache);
		stream.serial(VersionName);
		stream.serialEnum(SessionType);
		stream.serial(InitialActIndex);
		stream.serial(RingAccess);
		stream.serial(IsSessionOwner);
		stream.serial(EditSessionLink);


	}

public:
	uint32				AiInstance;
	TSessionId			SessionId;
	uint32				Mode; //1 edition, 2 Test, 3 Animation
	uint32				EditSlotId;
	CObjectSerializerClient	HighLevel;
	bool				MustTp;
	bool				InCache;
	std::string			VersionName;
	TScenarioSessionType SessionType;
	uint32				InitialActIndex;
	std::string			RingAccess;
	bool				IsSessionOwner;
	TSessionId			EditSessionLink;

};


class CAnimationMessageAnimationStart
{
public:
	enum TAnimationType	{Test, Play, Unkown}; //when eTest finish reconnect

public:
	CAnimationMessageAnimationStart(){ StartingAct = 1; }

	CAnimationMessageAnimationStart(const CAnimationMessageAnimationStart& other)
	{
		NLMISC::CMemStream mem;
		mem.serial( const_cast<CAnimationMessageAnimationStart&>(other));
		mem.invert();
		serial(mem);
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serialCont(AnimatorCharId);
		stream.serial(SessionId, RtData);
		stream.serial(StartingAct);
		stream.serial(AiInstance);
		stream.serial(ScenarioHeader);

	}

public:
	std::vector<uint32>			AnimatorCharId;
	TSessionId					SessionId;
	uint32						AiInstance;
	CObjectSerializerServer		RtData;
	TScenarioHeaderSerializer	ScenarioHeader;
	uint32						StartingAct;

};

} //namespace R2

#endif

