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
#include "nel/gui/lua_manager.h"
#include "nel/gui/lua_helper.h"

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
		delete luaState;
		luaState = NULL;
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
			nlwarning( e.luaWhat().c_str() );
			return false;
		}

		return true;
	}

	void CLuaManager::ResetLuaState()
	{
		delete luaState;
		luaState = new CLuaState( debugLua );
	}

	void CLuaManager::forceGarbageCollect()
	{
		nlinfo("Collecting Garbaged LUA variables");
		luaState->setGCThreshold( 0 );
		nlinfo( NLMISC::toString( "Memory Used : %d Kb", luaState->getGCCount() ).c_str() );
	}
}

