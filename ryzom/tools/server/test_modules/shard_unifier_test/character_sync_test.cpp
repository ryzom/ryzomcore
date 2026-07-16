/** Test file for the ring sesison manager module */

#include "nel/misc/types_nl.h"
#include "nel/misc/command.h"

#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"
#include "nel/net/unified_network.h"
#include "nel/net/service.h"

#include "src/cpptest.h"

#include "game_share/mysql_wrapper.h"
#include "entities_game_service/player_manager/player_manager_interface.h"
#include "entities_game_service/player_manager/character_interface.h"
#include "entities_game_service/modules/shard_unifier_client.h"
#include "entities_game_service/modules/char_name_mapper_client.h"
#include "shard_unifier_service/database_mapping.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace RSMGR;

// some globals for the test
static TSessionId sessionId(0);

uint32	userId1 = 1;
static uint32	charId = (userId1*16)+0;

uint32	userId2 = 2;
static uint32	invitedCharId = (userId2*16)+0;

class CPlayer;

void sendCharactersSummary( CPlayer *player, bool AllAutorized)
{
}

/** Interface to the char name mapper client singleton */
class CCharNameMapperClient : public ICharNameMapperClient
{
public:

	void mapCharacterName(const NLMISC::CEntityId &charEid, const ucstring &charName)
	{

	}
};

CCharNameMapperClient	charNameMapper;


class CCharacter;

class CPHCharacter : public ICharacter
{
	virtual const NLMISC::CEntityId& getCharId() const
	{
		static NLMISC::CEntityId eid;
		return eid;
	}

	virtual void setName(const ucstring &name)
	{

	}

	virtual bool getEnterFlag() const
	{
		return true;
	}

	virtual uint32 getLastDisconnectionDate()
	{
		return 0;
	}
};

ICharacter *ICharacter::getInterface(::CCharacter *character, bool onlyOnline)
{
	return NULL;
}

ICharacter *ICharacter::getInterface(uint32 charId, bool onlyOnline)
{
	return NULL;
}

void cbCreateChar_part2(uint32 userId, const CCreateCharMsg &createCharMsg, bool ok)
{
}

void sendIfNameIsValide( uint32 userId, bool nameValide )
{
}

class CPlayerManagerPH : public IPlayerManager
{
	TMapPlayers	_Players;

	virtual const TMapPlayers& getPlayers()
	{
		return _Players;
	}

	/** Get the active character for the given player id */
	virtual ::CCharacter * getActiveChar( uint32 userId )
	{
		return NULL;
	}

	/** Get the specified character of a player */
	virtual ::CCharacter * getChar( uint32 userId, uint32 index )
	{
		return NULL;
	}

	/** A character has been renamed by name unifier */
	virtual void characterRenamed(uint32 charId, const std::string &newName)
	{

	}

	virtual void sendImpulseToClient(const NLMISC::CEntityId & id, const std::string & msgName )
	{

	}

	virtual void addAllCharForStringIdRequest()
	{
	}

	virtual void addEntityForStringIdRequest(const NLMISC::CEntityId &eid)
	{
	}

	virtual void checkContactLists()
	{
	}

	virtual void playerEntityRemoved(const NLMISC::CEntityId &eid)
	{
	}



};

CPlayerManagerPH	playerManager;

IPlayerManager	*IPlayerManager::_Instance = NULL;

class CTestService : public NLNET::IService
{

};

class CCharacterSyncTS: public Test::Suite
{
	MSW::CConnection _Conn;

	string	OldPath;

public:
	CCharacterSyncTS(const std::string &workingPath)
	{
		OldPath = CPath::getCurrentPath();

		CPath::setCurrentPath(workingPath.c_str());

		// force instanciation of the module manager
		IModuleManager::getInstance();
		// create the session manager module
		CCommandRegistry &cr = CCommandRegistry::getInstance();

		string DBHost = "borisb2";

		// create a pseudo service
		CTestService	service;
		service.setArgs("-Zu");
		service.main("TS", "test_service", 0, ".", ".", "");
		service.anticipateShardId(100);
	
		cr.execute("addNegativeFilterInfo EIT", InfoLog());

		cr.execute("moduleManager.createModule StandardGateway gw", InfoLog());
		cr.execute("moduleManager.createModule ShardUnifierClient suc", InfoLog());
		cr.execute("suc.plug gw", InfoLog());
		cr.execute("moduleManager.createModule CharacterSynchronisation cs ring_db(host="+DBHost+" user=root password= base=ring) nel_db(host="+DBHost+" user=root password= base=nel)", InfoLog());
		cr.execute("cs.plug gw", InfoLog());

		// check module creation
		nlassert(IModuleManager::getInstance().getLocalModule("suc") != NULL);
		nlassert(IModuleManager::getInstance().getLocalModule("cs") != NULL);
		nlassert(IModuleManager::getInstance().getLocalModule("gw") != NULL);

		// check database population
		nlverify(_Conn.connect(DBHost, "root", "", "ring"));

		// check the first user id
		CRingUserPtr ru = CRingUser::load(_Conn, userId1, __FILE__, __LINE__);
		if (ru == NULL)
		{
			// create a new record
			ru = CRingUser::createTransient(__FILE__, __LINE__);

			ru->setUserName("toto");
//			ru->setCurrentSession(0);

			nlverify(ru->create(_Conn));

			userId1 = ru->getObjectId();
		}

		charId = (userId1 * 16)+0;

		// check the character
		CCharacterPtr character = RSMGR::CCharacter::load(_Conn, charId, __FILE__, __LINE__);
		if (character == NULL)
		{
			// create a character
			character = RSMGR::CCharacter::createTransient(__FILE__, __LINE__);

			character->setObjectId(charId);
			character->setCharName("toto_char");
			character->setUserId(userId1);
			character->setGuildId(0);
			character->setBestCombatLevel(100);

			nlverify(character->create(_Conn));

		}

		// check the second user id
		ru = CRingUser::load(_Conn, userId2, __FILE__, __LINE__);
		if (ru == NULL)
		{
			// create a new record
			ru = CRingUser::createTransient(__FILE__, __LINE__);

			ru->setUserName("invited");
//			ru->setCurrentSession(0);

			nlverify(ru->create(_Conn));

			userId2 = ru->getObjectId();
		}

		invitedCharId = (userId2 * 16)+0;

		// check the character
		character = RSMGR::CCharacter::load(_Conn, invitedCharId, __FILE__, __LINE__);
		if (character == NULL)
		{
			// create a character
			character = RSMGR::CCharacter::createTransient(__FILE__, __LINE__);

			character->setObjectId(invitedCharId);
			character->setCharName("invited_char");
			character->setUserId(userId2);
			character->setGuildId(0);
			character->setBestCombatLevel(100);

			nlverify(character->create(_Conn));

		}

		// add the tests
		TEST_ADD(CCharacterSyncTS::addCharacter);
	}

	~CCharacterSyncTS()
	{
		// delete the session manager module
		CCommandRegistry &cr = CCommandRegistry::getInstance();

		cr.execute("moduleManager.deleteModule suc", InfoLog());
		cr.execute("moduleManager.deleteModule cs", InfoLog());
		cr.execute("moduleManager.deleteModule gw", InfoLog());

		// remove the object created for the test
		if (userId1 != 1)
		{
			CRingUserPtr ru = CRingUser::load(_Conn, userId1, __FILE__, __LINE__);
			if (ru != NULL)
				ru->remove(_Conn);
			ru = CRingUser::load(_Conn, userId2, __FILE__, __LINE__);
			if (ru != NULL)
				ru->remove(_Conn);

		}
		_Conn.closeConn();

		CPath::setCurrentPath(OldPath.c_str());
	}


	void addCharacter()
	{
		// egs create a new character in slot 0

		uint32 newCharId = (userId1<<4)+0;

		TEST_ASSERT(IShardUnifierEvent::getInstance() != NULL);

		CCharacterPtr character = RSMGR::CCharacter::load(_Conn, newCharId, __FILE__, __LINE__);
		if (character != NULL)
		{
			character->remove(_Conn);
			character = CCharacterPtr();
		}

		CHARSYNC::TCharInfo charInfo;
		charInfo.setCharEId(CEntityId(0, newCharId));
		charInfo.setBestCombatLevel(10);
		charInfo.setCharName("0th character");
		IShardUnifierEvent::getInstance()->onNewChar(charInfo);

		// message dispatch will be done immediately, so the result should be cool right now

		character = RSMGR::CCharacter::load(_Conn, newCharId, __FILE__, __LINE__);
		TEST_ASSERT(character != NULL);

		TEST_ASSERT(character->getBestCombatLevel() == 10);
		TEST_ASSERT(character->getCharName() == "0th character");
	}

};


Test::Suite *createCCharacterSyncTS(const std::string &workingPath)
{
	return new CCharacterSyncTS(workingPath);
}
