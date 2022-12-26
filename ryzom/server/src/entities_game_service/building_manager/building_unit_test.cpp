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

#if !FINAL_VERSION

// misc
#include "nel/misc/command.h"
// game_share
#include "game_share/timer.h"
// egs
#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "player_manager/player.h"
#include "building_manager/building_manager.h"
#include "destination.h"
#include "building_manager/building_physical.h"
#include "building_manager/room_instance.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

static const sint32 InitCellId = 666;
static const uint16 InitRoomIndex = 0xffff;

extern NLNET::TServiceId FeService;
extern CRandom RandomGenerator;

struct CBuildingTestCharacter
{
	CBuildingTestCharacter(CEntityId id = CEntityId::Unknown, sint32 cellId = InitCellId, bool inBuilding = false, const string & buildingName = "", uint16 roomIndex = InitRoomIndex)
		: EId(id), CellId(cellId), InBuilding(inBuilding), BuildingName(buildingName), RoomIndex(roomIndex)
	{
	}

	CEntityId	EId;
	sint32		CellId;
	bool		InBuilding;
	string		BuildingName;
	uint16		RoomIndex;
};

static vector<CEntityId> FakeCharacters;
static vector<CBuildingTestCharacter> TestCharacters;
static bool TestRunning = false;
static bool Verbose = true;

// forward
static CEntityId createFakeCharacter(uint32 playerId, const string & name, EGSPD::CPeople::TPeople people, GSGENDER::EGender gender);
//

/**
 * Class used to test building manager
 * \author Sebastien Guignot
 * \author Nevrax France
 * \date 2004
 */
class CBuildingTest: public CTimerEvent
{
public:
	CBuildingTest(NLMISC::TGameCycle delay, uint count, uint simultaneous)
		: _Delay(delay), _Count(count), _Simultaneous(simultaneous)
	{
	}

	void timerCallback(CTimer * timer);

	IDestination * getRandomDestination();
	CBuildingDestination * getRandomBuildingDestination();

	bool enterBuilding(uint charIndex);
	bool leaveBuilding(uint charIndex);
	bool changeRoom(uint charIndex);

	void moveCharacter(uint charIndex);

	static void checkCharIntegrity(uint charIndex);
	static void checkIntegrity();

	void setNextEvent(CTimer * timer)
	{
		if (_Count)
		{
			_Count--;
			timer->setRemaining( _Delay, this );
		}
		else
		{
			TestRunning = false;
		}
	}

private:
	const NLMISC::TGameCycle _Delay;
	uint _Count;
	uint _Simultaneous;
};

//----------------------------------------------------------------------------
void buildingCheckIntegrity(CCharacter * skipChar)
{
	if ( !TestRunning )
		return;

	if (skipChar)
	{
		CEntityId skipId = skipChar->getId();
		uint charIndex;
		for (charIndex = 0; charIndex < TestCharacters.size(); charIndex++)
		{
			if (TestCharacters[charIndex].EId == skipId)
				break;
		}

		if (charIndex < TestCharacters.size())
		{
			const bool inBuilding = TestCharacters[charIndex].InBuilding;
			TestCharacters[charIndex].InBuilding = false;
			CBuildingTest::checkIntegrity();
			TestCharacters[charIndex].InBuilding = inBuilding;
		}
	}
	else
	{
		CBuildingTest::checkIntegrity();
	}
}

//----------------------------------------------------------------------------
void CBuildingTest::timerCallback(CTimer * timer)
{
	H_AUTO(BuildingTestTimerEvent);

	if (Verbose)
	{
		nlinfo("*** BuildingUnitTest *** callback count %u", _Count);
	}

	const uint randomNumber = (uint) RandomGenerator.rand( (uint16)TestCharacters.size()-1 );
	for (uint i = 0; i < _Simultaneous; i++)
	{
		checkIntegrity();
		const uint charIndex = (randomNumber + i) % TestCharacters.size();
		moveCharacter( charIndex );
	}
	checkIntegrity();

	setNextEvent( timer );
}

//----------------------------------------------------------------------------
IDestination * CBuildingTest::getRandomDestination()
{
	CBuildingManager * bm = CBuildingManager::getInstance();
	nlassert( bm );

	IDestination * dest = NULL;
	do
	{
		sint32 randomNumber = RandomGenerator.rand( (uint16)bm->_Triggers.size()-1 );
		CHashMap<sint,CBuildingManager::CTrigger>::iterator itTrigger = bm->_Triggers.begin();
		for (sint32 i = 0; i < randomNumber; i++)
		{
			++itTrigger;
			if (itTrigger == bm->_Triggers.end())
				itTrigger = bm->_Triggers.begin();
		}

		const sint triggerId = (*itTrigger).first;
		CBuildingManager::CTrigger & trigger = (*itTrigger).second;
		if (trigger.Destinations.empty())
		{
			nlwarning("*** BuildingUnitTest *** trigger %u has no destination.", triggerId);
			continue;
		}

		randomNumber = RandomGenerator.rand( (uint16)trigger.Destinations.size()-1 );
		dest = trigger.Destinations[randomNumber];
	} while ( !dest );

	return dest;
}

//----------------------------------------------------------------------------
CBuildingDestination * CBuildingTest::getRandomBuildingDestination()
{
	CBuildingDestination * buildingDest;
	do
	{
		buildingDest = dynamic_cast<CBuildingDestination *>( getRandomDestination() );
	} while( !buildingDest );

	return buildingDest;
}

//----------------------------------------------------------------------------
bool CBuildingTest::enterBuilding(uint charIndex)
{
	nlassert( charIndex < TestCharacters.size() );
	nlassert( !TestCharacters[charIndex].InBuilding );

	CEntityId id = TestCharacters[charIndex].EId;

	CCharacter * c = PlayerManager.getChar( id );
	nlassert( c );

	CBuildingDestination * buildingDest = getRandomBuildingDestination();
	nlassert( buildingDest );

	IBuildingPhysical * buildingInst = buildingDest->_ArrivalBuilding;
	nlassert( buildingInst );
	const uint16 roomIndex = buildingDest->_ArrivalRoomIndex;

	uint16 ownerId = 0;
	CBuildingPhysicalPlayer * playerBuilding = dynamic_cast<CBuildingPhysicalPlayer *>(buildingDest->_ArrivalBuilding);
	if (playerBuilding)
	{
		playerBuilding->addPlayer( id );
		const vector<CEntityId> & players = playerBuilding->_Players;
		bool found = false;
		for (uint i = 0; i < players.size(); i++)
		{
			if (players[i] == id)
			{
				ownerId = (uint16)i;
				found = true;
				break;
			}
		}
		nlassert( found );
	}
	else
	{
		CBuildingPhysicalGuild * guildBuilding = dynamic_cast<CBuildingPhysicalGuild *>(buildingDest->_ArrivalBuilding);
		if (guildBuilding)
		{
			const EGSPD::TGuildId guildId = charIndex;

			const vector<EGSPD::TGuildId> & guilds = guildBuilding->_Guilds;
			bool found = false;
			for (uint i = 0; i < guilds.size(); i++)
			{
				if (guilds[i] == guildId)
				{
					ownerId = (uint16)i;
					found = true;
					break;
				}
			}

			if ( !found )
			{
				guildBuilding->addGuild( guildId );

				for (uint i = 0; i < guilds.size(); i++)
				{
					if (guilds[i] == guildId)
					{
						ownerId = (uint16)i;
						found = true;
						break;
					}
				}
				nlassert( found );
			}
		}
	}

	sint32 cellId = InitCellId;
	if ( !buildingDest->addUser(c, ownerId, cellId) )
	{
		nlwarning("*** BuildingUnitTest *** character %u cannot enter building %s in room %hu.",
			charIndex, buildingInst->getName().c_str(), buildingDest->_ArrivalRoomIndex
			);
		CBuildingManager::getInstance()->removePlayerFromRoom( c );
		return false;
	}

	if (Verbose)
	{
		nlinfo("*** BuildingUnitTest *** character %u enters building %s in room %hu (cell = %d).",
			charIndex, buildingInst->getName().c_str(), buildingDest->_ArrivalRoomIndex, cellId
			);
	}

	// integrity check
	const IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cellId );
	nlassert( room );
	if ( room->getBuilding() != buildingInst )
	{
		nlwarning("*** BuildingUnitTest *** INTEGRITY FAILURE for character %u: asked teleport to %s, but has been teleported to %s",
			charIndex, buildingInst->getName().c_str(), room->getBuilding()->getName().c_str()
			);
		DEBUG_STOP;
		return false;
	}

	// fake teleport
	CBuildingManager::getInstance()->removePlayerFromRoom( c );
	CMirrorPropValue<TYPE_CELL> mirrorCell( TheDataset, c->getEntityRowId(), DSPropertyCELL );
	mirrorCell = cellId;

	// update character data
	TestCharacters[charIndex].CellId		= cellId;
	TestCharacters[charIndex].InBuilding	= true;
	TestCharacters[charIndex].BuildingName	= buildingInst->getName();
	TestCharacters[charIndex].RoomIndex		= roomIndex;

	return true;
}

//----------------------------------------------------------------------------
bool CBuildingTest::leaveBuilding(uint charIndex)
{
	nlassert( charIndex < TestCharacters.size() );
	nlassert( TestCharacters[charIndex].InBuilding );

	CEntityId id = TestCharacters[charIndex].EId;

	CCharacter * c = PlayerManager.getChar( id );
	nlassert( c );

	sint32 cellId = TestCharacters[charIndex].CellId;
	const IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cellId );
	if ( !room )
	{
		nlwarning("*** BuildingUnitTest *** cannot get room instance for character %u at cell %d.", charIndex, cellId);
		return false;
	}

	IBuildingPhysical * buildingInst = room->getBuilding();
	nlassert( buildingInst );

	CTPDestination * exitDest = const_cast<CTPDestination *>( buildingInst->getExit( 0 ) );
	nlassert( exitDest );

	cellId = InitCellId;
	if ( !exitDest->addUser(c, 0, cellId) )
	{
		nlwarning("*** BuildingUnitTest *** character %u cannot leave building %s by exit %s.",
			charIndex, TestCharacters[charIndex].BuildingName.c_str(), exitDest->getName().c_str()
			);
		CBuildingManager::getInstance()->removePlayerFromRoom( c );
		return false;
	}

	nlassert( cellId != InitCellId );
	if (Verbose)
	{
		nlinfo("*** BuildingUnitTest *** character %u leaves building %s by exit %s (old cell = %d).",
			charIndex, TestCharacters[charIndex].BuildingName.c_str(), exitDest->getName().c_str(), TestCharacters[charIndex].CellId
			);
	}

	// fake teleport
	CBuildingManager::getInstance()->removePlayerFromRoom( c );
	CMirrorPropValue<TYPE_CELL> mirrorCell( TheDataset, c->getEntityRowId(), DSPropertyCELL );
	mirrorCell = cellId;

	// update character data
	TestCharacters[charIndex].CellId		= cellId;
	TestCharacters[charIndex].InBuilding	= false;
	TestCharacters[charIndex].BuildingName	= "";
	TestCharacters[charIndex].RoomIndex		= InitRoomIndex;

	return true;
}

//----------------------------------------------------------------------------
bool CBuildingTest::changeRoom(uint charIndex)
{
	nlassert( charIndex < TestCharacters.size() );
	nlassert( TestCharacters[charIndex].InBuilding );

	CEntityId id = TestCharacters[charIndex].EId;

	CCharacter * c = PlayerManager.getChar( id );
	nlassert( c );

	sint32 cellId = TestCharacters[charIndex].CellId;
	const IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cellId );
	if ( !room )
	{
		nlwarning("*** BuildingUnitTest *** cannot get room instance for character %u at cell %d.", charIndex, cellId);
		return false;
	}

	IBuildingPhysical * buildingInst = room->getBuilding();
	nlassert( buildingInst );

	const uint16 roomIndex = room->getRoomIndex();

	vector<IBuildingPhysical::CRoomPhysical> & rooms = buildingInst->_Rooms;
	nlassert( !rooms.empty() );

	if ( rooms.size() == 1 )
		return true;

	// find another room
	uint16 newRoomIndex;
	const uint maxLoops = 1000;
	uint nbLoops = 0;
	while (1)
	{
		nlassert(++nbLoops <= maxLoops);

		newRoomIndex = (uint16) RandomGenerator.rand( (uint16)rooms.size()-1 );
		if (newRoomIndex >= rooms.size())
			newRoomIndex = 0;

		if (newRoomIndex != roomIndex)
			break;
	}

	nlassert( !rooms[newRoomIndex].Cells.empty() );
	const sint32 newCellId = rooms[newRoomIndex].Cells[0];
	const IRoomInstance * newRoom = CBuildingManager::getInstance()->getRoomInstanceFromCell( newCellId );
	if ( !newRoom )
	{
		nlwarning("*** BuildingUnitTest *** cannot get room instance for character %u at cell %d.", charIndex, newCellId);
		return false;
	}

	IBuildingPhysical * newBuildingInst = newRoom->getBuilding();
	nlassert( newBuildingInst );
	nlassert( newBuildingInst == buildingInst );

	// cannot change room if we are in a guild/player building
	if ( !dynamic_cast<const CRoomInstanceCommon *>( newRoom ) )
		return true;

	if ( !newBuildingInst->addUser( c, newRoomIndex, 0, cellId ) )
	{
		nlwarning("*** BuildingUnitTest *** character %u cannot go from room %hu to room %hu in building %s.",
			charIndex, roomIndex, newRoomIndex, TestCharacters[charIndex].BuildingName.c_str()
			);
		CBuildingManager::getInstance()->removePlayerFromRoom( c );
		return false;
	}

	if (Verbose)
	{
		nlinfo("*** BuildingUnitTest *** character %u goes from room %hu (cell = %d) to room %hu (cell = %d) in building %s.",
			charIndex, roomIndex, TestCharacters[charIndex].CellId, newRoomIndex, cellId, TestCharacters[charIndex].BuildingName.c_str()
			);
	}

	// fake teleport
	CBuildingManager::getInstance()->removePlayerFromRoom( c );
	CMirrorPropValue<TYPE_CELL> mirrorCell( TheDataset, c->getEntityRowId(), DSPropertyCELL );
	mirrorCell = cellId;

	// update character data
	TestCharacters[charIndex].CellId		= cellId;
	TestCharacters[charIndex].RoomIndex		= newRoomIndex;

	return true;
}

//----------------------------------------------------------------------------
void CBuildingTest::moveCharacter(uint charIndex)
{
	nlassert( charIndex < TestCharacters.size() );

	if ( !TestCharacters[charIndex].InBuilding )
	{
		enterBuilding( charIndex );
	}
	else
	{
		const bool leave = (RandomGenerator.rand(99) & 1);
		if (leave)
			leaveBuilding( charIndex );
		else
			changeRoom( charIndex );
	}
}

//----------------------------------------------------------------------------
void CBuildingTest::checkCharIntegrity(uint charIndex)
{
	nlassert( charIndex < TestCharacters.size() );
	
	if ( !TestCharacters[charIndex].InBuilding )
		return;

	sint32 cellId = TestCharacters[charIndex].CellId;
	if ( cellId > -2 || (cellId & 1) )
	{
		nlwarning("*** BuildingUnitTest *** INTEGRITY FAILURE for character %u: invalid cell id (%d)", charIndex, cellId);
		DEBUG_STOP;
		return;
	}

	CCharacter * c = PlayerManager.getChar( TestCharacters[charIndex].EId );
	nlassert( c );
	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, c->getEntityRowId(), DSPropertyCELL );
	if (mirrorCell != cellId)
	{
		nlwarning("*** BuildingUnitTest *** INTEGRITY FAILURE for character %u: cell does not match with mirror: %d != %d",
			charIndex, cellId, (sint32)mirrorCell
			);
		DEBUG_STOP;
		return;
	}

	const IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cellId );
	if ( !room )
	{
		nlwarning("*** BuildingUnitTest *** INTEGRITY FAILURE for character %u: cannot find room (null)", charIndex);
		DEBUG_STOP;
		return;
	}

	if ( !room->getBuilding() )
	{
		nlwarning("*** BuildingUnitTest *** INTEGRITY FAILURE for character %u: cannot find building (null)", charIndex);
		DEBUG_STOP;
		return;
	}

	if ( TestCharacters[charIndex].BuildingName != room->getBuilding()->getName() )
	{
		nlwarning("*** BuildingUnitTest *** INTEGRITY FAILURE for character %u: building does not match: %s != %s",
			charIndex, TestCharacters[charIndex].BuildingName.c_str(), room->getBuilding()->getName().c_str()
			);
		DEBUG_STOP;
		return;
	}

	if ( TestCharacters[charIndex].RoomIndex != room->getRoomIndex() )
	{
		CBuildingManager * bm = CBuildingManager::getInstance();
		nlassert( bm );

		nlwarning("*** BuildingUnitTest *** INTEGRITY FAILURE for character %u: room index does not match: %hu != %hu",
			charIndex, TestCharacters[charIndex].RoomIndex, room->getRoomIndex()
			);
		DEBUG_STOP;
		return;
	}
}

//----------------------------------------------------------------------------
void CBuildingTest::checkIntegrity()
{
	if (Verbose)
	{
		nlinfo("*** BuildingUnitTest *** checking integrity...");
	}

	if (TestCharacters.empty())
		return;

	CBuildingManager * bm = CBuildingManager::getInstance();
	nlassert( bm );

	// check free cells
	std::set<sint32> freeCells;
	uint32 freeRoomIdx = bm->_FirstFreeRoomId;
	while ( freeRoomIdx < bm->_RoomInstances.size() )
	{
		const sint32 freeCellId = bm->getRoomCellFromIdx( freeRoomIdx );
		if ( freeCellId > -2 || (freeCellId & 1) )
		{
			nlwarning("*** BuildingUnitTest *** INTEGRITY FAILURE: invalid cell id (%d)", freeCellId);
			DEBUG_STOP;
			return;
		}
		if ( freeCells.find( freeCellId ) != freeCells.end() )
		{
			nlwarning("*** BuildingUnitTest *** INTEGRITY FAILURE: cell id (%d) is twice in the free cells list", freeCellId);
			DEBUG_STOP;
			return;
		}
		freeCells.insert( freeCellId );
		freeRoomIdx = bm->_RoomInstances[freeRoomIdx].NextFreeId;
	}

	uint nbCharsInBuilding = 0;
	sint32 minCellId = 0;
	std::set<sint32> cells;

	for (uint charIndex = 0; charIndex < TestCharacters.size(); charIndex++)
	{
		if ( !TestCharacters[charIndex].InBuilding )
			continue;

		const sint32 cellId = TestCharacters[charIndex].CellId;
		if ( cellId < minCellId )
			minCellId = cellId;

		if ( freeCells.find( cellId ) != freeCells.end() )
		{
			nlwarning("*** BuildingUnitTest *** INTEGRITY FAILURE for character %u: cell id %d is considered free by building manager",
				charIndex, cellId
				);
			DEBUG_STOP;
			return;
		}
		cells.insert( cellId );

		checkCharIntegrity( charIndex );
		nbCharsInBuilding++;
	}

	if (Verbose)
	{
		nlinfo("*** BuildingUnitTest *** checked %u cells for %u characters, minimum cell id is %d. Total number of room instances = %u",
			cells.size(), nbCharsInBuilding, minCellId, bm->_RoomInstances.size()
			);
	}
}

//----------------------------------------------------------------------------
NLMISC_COMMAND (testBuildingManager, "(debug) Unit test for building manager",
				"[<nb_test_characters> <nb_ops> <nb_simultaneous> <delay_in_ticks> <verbose=0/1>]")
{
	uint nbChars, nbOps, nbSimultaneous, opDelay;
	if (args.empty())
	{
		// default values
		nbChars			= 100;
		nbOps			= 200;
		nbSimultaneous	= 2;
		opDelay			= 5;
		Verbose			= true;
	}
	else if (args.size() == 5)
	{
		NLMISC::fromString(args[0], nbChars);
		NLMISC::fromString(args[1], nbOps);
		NLMISC::fromString(args[2], nbSimultaneous);
		NLMISC::fromString(args[3], opDelay);
		NLMISC::fromString(args[4], Verbose);
	}
	else
		return false;

	const uint32 firstPlayerId = 0x00666000;

	if (FakeCharacters.size() < nbChars)
	{
		for (uint i = (uint)FakeCharacters.size(); i < nbChars; i++)
		{
			CEntityId id = createFakeCharacter( firstPlayerId+i, toString("fake%u", i), EGSPD::CPeople::Fyros, GSGENDER::female );
			if (id == CEntityId::Unknown)
				continue;

			CCharacter * c = PlayerManager.getChar( id );
			BOMB_IF( (c == NULL), "character not found!", return false );
			c->tpWanted( 21980, -26162, 0 ); // kaemon (fyros newbie land)

			FakeCharacters.push_back( id );
		}
	}

	TestCharacters.resize( min(nbChars, (uint)FakeCharacters.size()) );
	for (uint i = 0; i < TestCharacters.size(); i++)
	{
		CCharacter * c = PlayerManager.getChar( FakeCharacters[i] );
		nlassert( c );

		// go out of building
		CBuildingManager::getInstance()->removePlayerFromRoom( c );
		CMirrorPropValue<TYPE_CELL> mirrorCell( TheDataset, c->getEntityRowId(), DSPropertyCELL );
		mirrorCell = 0;

		TestCharacters[i] = CBuildingTestCharacter( FakeCharacters[i] );
	}

	if (Verbose)
	{
		nlinfo("*** BuildingUnitTest *** nb test characters: %u", TestCharacters.size());
	}

	static CTimer timer;
	timer.reset();
	timer.setRemaining( 2, new CBuildingTest(opDelay, nbOps, nbSimultaneous) );

	TestRunning = true;

	return true;
}

//----------------------------------------------------------------------------
static CEntityId createFakeCharacter(uint32 playerId, const string & name, EGSPD::CPeople::TPeople people, GSGENDER::EGender gender)
{
	if (people >= EGSPD::CPeople::EndPlayable)
		return CEntityId::Unknown;

	if (gender != GSGENDER::male && gender != GSGENDER::female)
		return CEntityId::Unknown;

	CPlayer * player = PlayerManager.getPlayer( playerId );
	if (player == NULL)
	{
		player = new CPlayer;
		player->setId( playerId );
		PlayerManager.setPlayerFrontEndId( playerId, FeService );
		PlayerManager.addPlayer( playerId, player );
	}

	CEntityId id = player->createCharacter( name, people, gender );
	if ( !Mirror.createEntity( id ) )
	{
		PlayerManager.disconnectPlayer( playerId );
		return CEntityId::Unknown;
	}

	PlayerManager.setActiveCharForPlayer( playerId, (uint32) (id.getShortId() & 0xf), id );
	CCharacter * c = player->getActiveCharacter();
	TDataSetRow rowId = TheDataset.getDataSetRow( id );

	c->addPropertiesToMirror( rowId );
	c->mirrorizeEntityState(); // write the initial position into the mirror
	c->setEnterFlag( true );

	TheDataset.declareEntity( rowId );

	c->setDatabase();
	CBuildingManager::getInstance()->registerPlayer( c );

	CMirrorPropValue<TYPE_CELL> mirrorCell( TheDataset, c->getEntityRowId(), DSPropertyCELL );
	mirrorCell = 0;

	return id;
}

#endif // !FINAL_VERSION
