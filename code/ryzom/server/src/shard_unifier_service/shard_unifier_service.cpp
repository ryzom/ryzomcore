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
#include <string>
#include "shard_unifier_service.h"
#include "nel/misc/command.h"

#include "game_share/singleton_registry.h"
#include <time.h>
//#include <sys/utime.h>

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

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

