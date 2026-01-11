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

#ifndef EGS_CHARACTER_ENCYCLOPEDIA_H
#define EGS_CHARACTER_ENCYCLOPEDIA_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// game share
#include "game_share/persistent_data.h"

//-----------------------------------------------------------------------------

class CCharacter;
class CEncyMsgThema;

/**
 * Dynamic part of the encyclopedia stored in a character
 * This structure is optimized for size because its stored directly in the player persistant data stuff
 * We use CEncyMsgXXX for sending info to the player
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2004
 */
class CCharacterEncyclopedia
{
	NL_INSTANCE_COUNTER_DECL(CCharacterEncyclopedia);
public:

	CCharacterEncyclopedia(CCharacter &c);

	// Construct the encyclopedia album structure from the static sheet that defines encyclopedia
	// This method ensure that we have at least the same number of album and the same number of thema by album 
	// as in the sheets defines the encyclopedia
	void init();

	// remove all
	void clear();

	DECLARE_PERSISTENCE_METHODS

	// update a task and send a message to the client
	void updateTask(uint32 nAlbum, uint32 nThema, uint32 nTask, uint8 nNewState, bool bSendMsg=true);

	// unlock a thema so all tasks that belongs to this thema are shown and send a message to the client
	void unlockThema(uint32 nAlbum, uint32 nThema, bool bSendMsg=true);

	// send the whole encyclopedia to the client (just the opened or completed parts)
	void sendEncycloToClient();
	
	// return true if all ritual tasks of a thema are done (we do not test the rite of course)
	bool isAllTaskDone(uint32 nAlbum, uint32 nThema);

	// return true if at least one ritual task is not done (we do not test the rite)
	bool isAtLeastOneTaskNotDone(uint32 nAlbum, uint32 nThema);

private:

	// helpers for sending messages to client
	bool checkIfThemaCompleted(uint32 nAlbum, uint32 nThemaInt, bool bSendMsg=true);
	bool checkIfAlbumCompleted(uint32 nAlbum);
	void sendAlbumMessage(uint32 nAlbum, uint32 nThema);
	void makeThemaMessage(CEncyMsgThema &outThema, uint32 nAlbum, uint32 nThema);
	
private:
	
	// The parent class
	CCharacter &_Char;

	// State of tasks and rite is
	// 0 - not displayed
	// 1 - displayed but not done yet
	// 2 - displayed and done
	struct CEncyCharThema
	{
		// Persistent data

		uint8	ThemaState;
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

			uint16 RiteTaskStatePacked;
		};
		
		// Non Persistent data

		uint32 ThemaNameCache;			// Caches are used because the sendStringToClient method generate
		uint32 RewardTextCache;			// a new id for each call, but the string we give it do not change.
		uint32 RiteTaskNameCache[8];
		
		// --------------------
		
		CEncyCharThema()
		{
			ThemaState = 0;
			RiteTaskStatePacked = 0;
			ThemaNameCache = 0;
			RewardTextCache = 0;
			for (uint32 i = 0; i < 8; ++i)
				RiteTaskNameCache[i] = 0;
		}

		DECLARE_PERSISTENCE_METHODS

		void setTaskState(uint8 nTaskNb, uint8 nNewState)
		{
			nlassert(nTaskNb < 8);
			nlassert(nNewState < 3);
			switch (nTaskNb)
			{
				case 0: Unpack.RiteState  = nNewState; break;
				case 1: Unpack.Task0State = nNewState; break;
				case 2: Unpack.Task1State = nNewState; break;
				case 3: Unpack.Task2State = nNewState; break;
				case 4: Unpack.Task3State = nNewState; break;
				case 5: Unpack.Task4State = nNewState; break;
				case 6: Unpack.Task5State = nNewState; break;
				case 7: Unpack.Task6State = nNewState; break;
			}			
		}

		uint8 getTaskState(uint8 nTaskNb)
		{
			nlassert(nTaskNb < 8);
			switch (nTaskNb)
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

	struct CEncyCharAlbum
	{
		// Persitent data

		uint8	AlbumState;
		std::vector<CEncyCharThema> Themas;

		// Non persistent data

		uint32	AlbumNameCache;

		// --------------------------------

		CEncyCharAlbum()
		{
			AlbumState = 0;
			AlbumNameCache = 0;
		}

		DECLARE_PERSISTENCE_METHODS
	};

	std::vector<CEncyCharAlbum> _EncyCharAlbums;
};

#endif // EGS_CHARACTER_ENCYCLOPEDIA_H
