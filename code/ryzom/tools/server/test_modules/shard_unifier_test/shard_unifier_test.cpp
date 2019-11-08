
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/dynloadlib.h"
#include "nel/misc/path.h"
#include "nel/net/service.h"

#include "src/cpptest.h"

//#include "pd_lib/pd_string_manager.h"
#include "game_share/mysql_wrapper.h"

#include "entities_game_service/guild_manager/guild_manager_interface.h"
#include "entities_game_service/guild_manager/guild_interface.h"

#include "test_mapping.h"

#include <cstdlib>
#include <mysql.h>

using namespace std;
using namespace NLMISC;
using namespace NLNET;

class CGuildManagerPH : public IGuildManager
{
	virtual void	guildMemberChanged		(EGSPD::CGuildMemberPD *guildMemberPd)
	{
	nlstop;
	}
	/// the member list has been changed (ether add/replace or delete)
	virtual void	guildMemberListChanged	(EGSPD::CGuildPD *guildPd)
	{
	nlstop;
	}
	virtual void	updateGuildMembersStringIds()
	{
//	nlstop;
	}
	virtual void	createGuildStep2		(uint32 guildId, const ucstring &guildName, CHARSYNC::TCharacterNameResult result) 
	{
	nlstop;
	}

	virtual CGuild	*getGuildFromId			(uint32 guildId)
	{
	nlstop;
		return NULL;
	}
	virtual void	addGuildsAwaitingString	( const ucstring & guildStr, uint32 guildId )
	{
		nlstop;
	}

	/// get raw access to the guild list (not const)
	virtual const EGSPD::CGuildContainerPD *getGuildContainer() const
	{
		nlstop;
		return NULL;
	}

	/// A player entity have been removed from eid translator, check all guild member list
	virtual void	playerEntityRemoved(const NLMISC::CEntityId &eid)
	{
		nlstop;
	}
	/// check if guilds have been loaded
	virtual bool	guildLoaded()
	{
		return true;
	}

	/// fill a list with all guild descriptors
	virtual void	fillGuildInfos(std::vector<CHARSYNC::CGuildInfo> &guildInfos)
	{
//		nlstop;
	}

	/// Check all guild member lists against the entity translator
	virtual void	checkGuildMemberLists()
	{
	}

	// A character connect/disconnect on another shard, update the online tags
	virtual void	characterConnectionEvent(const NLMISC::CEntityId &eid, bool online)
	{
	}


};

CGuildManagerPH guildManager;

IGuildManager &IGuildManager::getInstance()
{
	return guildManager;
}


IGuild *IGuild::getGuildInterface(::CGuild *guild)
{
	nlstop;
	return NULL;
}

void	IGuild::setNameWrap(const ucstring &name)
{
	nlstop;
}
uint32	IGuild::getIdWrap()
{
	nlstop;
	return 0;
}

bool	IGuild::isProxyWrap()
{
	nlstop;
	return false;
}

void IGuild::updateMembersStringIds()
{
	nlstop;
}


class CGuildPH : public IGuild
{
};

//namespace RY_PDS 
//{
//	bool CPDStringManager::isWaitingIOSStoreStringResult()
//	{
//		return false;
//	}
//};


Test::Suite *createCNopeTS(const std::string &workingPath);
Test::Suite *createCRSMTS(const std::string &workingPath);
Test::Suite *createCCharacterSyncTS(const std::string &workingPath);


class CShardUnifierTS: public Test::Suite
{
	string			_WorkingPath;
//	string			_BaseDir;
//	string			_CurrentPath;
//	string			_CurrentPathToRestore;


	NLMISC::CApplicationContext	*_Context;

public:

	CShardUnifierTS(const std::string &workingPath)
	{
		_Context = new CApplicationContext();
		// init a new nel instance
		INelContext::getInstance();

		_WorkingPath = workingPath;

		NLMISC::createDebug();

//		add(auto_ptr<Test::Suite>(createCNopeTS(workingPath)));
		add(auto_ptr<Test::Suite>(createCRSMTS(workingPath)));
		add(auto_ptr<Test::Suite>(createCCharacterSyncTS(workingPath)));
		

	}

	~CShardUnifierTS()
	{
		CPath::setCurrentPath(_WorkingPath.c_str());

//		delete _Context;
	}

};

auto_ptr<Test::Suite> intRegisterTestSuite(const std::string &workingPath)
{
	return static_cast<Test::Suite*>(new CShardUnifierTS(workingPath));
}

NL_LIB_EXPORT_SYMBOL(registerTestSuite, void, intRegisterTestSuite);

