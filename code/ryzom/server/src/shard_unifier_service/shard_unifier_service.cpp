
#include "stdpch.h"
#include <string>
#include "shard_unifier_service.h"
#include "nel/misc/command.h"

#include "game_share/singleton_registry.h"
#include <time.h>
//#include <sys/utime.h>

using namespace std;
using namespace NLMISC;
using namespace NLNET;

// force admin module and command executor to link in
extern void admin_modules_forceLink();
extern void commandExecutor_forcelink();

void foo()
{
	admin_modules_forceLink();
	commandExecutor_forcelink();
}

// force module linking
extern void forceCharSyncLink();
extern void forceEntityLocatorLink();
extern void forceMailForumFwdLink();
extern void forceChatUnifierLink();



NLNET::TUnifiedCallbackItem cbArraySU[] = 
{ 
	{"", NULL}
};

// declare the serive
NLNET_SERVICE_MAIN(CShardUnifier, "SU", "shard_unifier_service", 50505, cbArraySU, "", "");



void CShardUnifier::init()
{
	CSingletonRegistry::getInstance()->init();
}

bool CShardUnifier::update()
{
	CSingletonRegistry::getInstance()->serviceUpdate();

	return true;
}

void CShardUnifier::release()
{
	CSingletonRegistry::getInstance()->release();
}


// force modules linking
void forceModuleLink()
{
	forceCharSyncLink();
	forceEntityLocatorLink();
	forceMailForumFwdLink();
	forceChatUnifierLink();
}

