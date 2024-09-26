// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
#include "nel/gui/lua_manager.h"
#include "nel/gui/lua_helper.h"


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	bool CLuaManager::debugLua = false;
	bool CLuaManager::editorMode = false;
	CLuaManager* CLuaManager::instance = NULL;

	CLuaManager::CLuaManager()
	{
		luaState = new NLGUI::CLuaState( debugLua );
	}

	CLuaManager::~CLuaManager()
	{
		if (luaState)
		{
			delete luaState;
			luaState = NULL;
		}
	}

	void CLuaManager::releaseInstance()
	{
		if (instance)
		{
			delete instance;
			instance = NULL;
		}
	}

	bool CLuaManager::executeLuaScript( const std::string &luaScript, bool smallScript )
	{
		if( editorMode )
			return true;

		try
		{
			if( smallScript )
				luaState->executeSmallScript( luaScript );
			else
				luaState->executeScript( luaScript );
		}
		catch( const ELuaError &e )
		{
			nlwarning("--- LUA ERROR ---");
			nlwarning(e.luaWhat().c_str());
			std::vector<std::string> res;
			NLMISC::explode(luaScript, std::string("\n"), res);
			for(uint k = 0; k < res.size(); ++k)
			{
				nlwarning("%.05u %s", k, res[k].c_str());
			}
			nlwarning("--- ********* ---");
			return false;
		}

		return true;
	}

	void CLuaManager::ResetLuaState()
	{
		if (luaState) delete luaState;

		luaState = new CLuaState( debugLua );
	}

	void CLuaManager::forceGarbageCollect()
	{
		nlinfo("Collecting Garbaged LUA variables");
		luaState->setGCThreshold( 0 );
		nlinfo( NLMISC::toString( "Memory Used : %d Kb", luaState->getGCCount() ).c_str() );
	}
}

