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


#ifndef RY_MSG_ENCYCLOPEDIA_H
#define RY_MSG_ENCYCLOPEDIA_H

#include "nel/misc/types_nl.h"
#include "nel/misc/bit_mem_stream.h"
#include <vector>

/**
 * CEncyMsgThema
 *
 * Maximum is 47 bytes per thema. There are 350 themas, so a potential total of 16450 bytes. Its a theorical maximum
 * that is never reached because few thema has 8 tasks. Some has 8, some has 1.
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date October 2004
 */
class CEncyMsgThema
{
public:
	uint32	Name;				// Server string
	uint32	RewardText;			// Server string
	uint32	RewardSheet;		// Sheet id (brick or phrase or item)
	uint8	State : 2;			// 0 Inactive 1 Running 2 Finished
	uint8	NbTask : 4;			// Max 8 tasks Min 1 task (the rite=task[0])
	uint32	TaskName[8];		// Task 0 is the rite
	uint32	TaskNPCName[8];		// Task 0 is the rite

	union
	{
		struct
		{
			uint16	RiteState  : 2;
			uint16	Task0State : 2;
			uint16	Task1State : 2;
			uint16	Task2State : 2;
			uint16	Task3State : 2;
			uint16	Task4State : 2;
			uint16	Task5State : 2;
			uint16	Task6State : 2;
		} Unpack;

		uint16 RiteTaskStatePacked; // All states of the 8 tasks
	};

	CEncyMsgThema()
	{
		Name = 0;
		RewardText = 0;
		RewardSheet = 0;
		State = 0;
		NbTask = 0;
		TaskName[0] = TaskName[1] = TaskName[2] = TaskName[3] = 0;
		TaskName[4] = TaskName[5] = TaskName[6] = TaskName[7] = 0;
		TaskNPCName[0] = TaskNPCName[1] = TaskNPCName[2] = TaskNPCName[3] = 0;
		TaskNPCName[4] = TaskNPCName[5] = TaskNPCName[6] = TaskNPCName[7] = 0;
		RiteTaskStatePacked = 0;
	}

	void serial(NLMISC::IStream &f)
	{
		f.serial(Name);

		uint8 StateNbTask = State + (NbTask << 2);
		f.serial(StateNbTask);
		State = StateNbTask & 0x3;
		NbTask = StateNbTask >> 2;

		if (State == 2)
		{
			f.serial(RewardText);
			f.serial(RewardSheet);
		}

		for (uint32 i = 0; i < NbTask; ++i)
		{
			f.serial(TaskName[i]);
			f.serial(TaskNPCName[i]);
		}

		f.serial(RiteTaskStatePacked);
	}

	uint8 getTaskState(uint8 nTask)
	{
		nlassert(nTask < NbTask);
		switch(nTask)
		{
			case 0: return Unpack.RiteState;
			case 1: return Unpack.Task0State;
			case 2: return Unpack.Task1State;
			case 3: return Unpack.Task2State;
			case 4: return Unpack.Task3State;
			case 5: return Unpack.Task4State;
			case 6: return Unpack.Task5State;
			case 7: return Unpack.Task6State;
		}
		return 0;
	}
};

/**
 * CEncyMsgAlbum
 *
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date October 2004
 */
class CEncyMsgAlbum
{
public:
	uint32						Name;			// Server string
	uint32						RewardBrick;	// Sheet id (brick that can unblock a title)
	uint8						State;			// 0 Inactive 1 Running 2 Finished
	std::vector<CEncyMsgThema>	Themas;			// All the themas

	CEncyMsgAlbum()
	{
		Name = RewardBrick = 0;
	}

	void serial(NLMISC::IStream &f)
	{
		f.serial(Name);
		f.serial(RewardBrick);
		f.serial(State);
		f.serialCont(Themas);
	}

	CEncyMsgThema *getThema(uint32 nThemaName)
	{
		for (uint32 i = 0; i < Themas.size(); ++i)
			if (Themas[i].Name == nThemaName)
				return &Themas[i];
		return NULL;
	}

};

/**
 * CEncyclopediaUpdateMsg
 *
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date October 2004
 */
class CEncyclopediaUpdateMsg
{
public:
	enum TEncyUpdateType
	{
		UpdateInit,
		UpdateAlbum,
		UpdateThema,
	};

	TEncyUpdateType Type;

	std::vector<CEncyMsgAlbum>	AllAlbums;

	CEncyMsgAlbum	Album;		// used for UpdateAlbum

	uint32			AlbumName;	// used for UpdateThema
	CEncyMsgThema	Thema;		// used for UpdateThema


	CEncyclopediaUpdateMsg()
	{
		Type = UpdateThema;
		AlbumName = 0;
	}

	void serial(NLMISC::CBitMemStream &f)
	{
		f.serialEnum(Type);
		switch(Type)
		{
			case UpdateInit:
				f.serialCont(AllAlbums);
			break;
			case UpdateAlbum:
				f.serial(Album);
			break;
			case UpdateThema:
				f.serial(AlbumName);
				f.serial(Thema);
			break;
		}
	}

};

#endif // RY_MSG_ENCYCLOPEDIA_H

/* End of msg_encyclopedia.h */
