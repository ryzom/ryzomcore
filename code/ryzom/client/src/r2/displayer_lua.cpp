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
#include "displayer_lua.h"
#include "nel/gui/lua_ihm.h"
#include "../interface_v3/lua_ihm_ryzom.h"
#include "editor.h"

namespace R2
{

// *********************************************************************************************************
CDisplayerLua::CDisplayerLua()
{
	_ToLua._Displayer = this;
}

// *********************************************************************************************************
bool CDisplayerLua::init(const CLuaObject &parameters)
{
	//H_AUTO(R2_CDisplayerLua_init)
	// parameters should be a function that create the lua displayer
	CLuaStackChecker lsc(parameters.getLuaState());
	_ToLua._LuaTable.release();
	if (parameters.isString())
	{
		getEditor().getEnv()[parameters.toString()].push(); // get method from the R2 env
	}
	else
	{
		parameters.push();
	}
	CLuaState &ls = *parameters.getLuaState();
	getEditor().getEnv().push(); // this is a method call
	if (CLuaIHMRyzom::executeFunctionOnStack(ls,  1,  1))
	{
		_ToLua._LuaTable.pop(ls);
	}
	else
	{
		nlwarning("<CDisplayerLua::init> Error while calling displayer ctor (parameter should be a r2 method, or the *name* of a r2 method) : param is : ");
		parameters.dump();
		return false;
	}
	return CDisplayerBase::init(parameters);
}

// *********************************************************************************************************
CDisplayerLua::CToLua::CToLua():_Displayer(NULL)
{
}

// *********************************************************************************************************
CDisplayerLua* CDisplayerLua::CToLua::getEnclosing()
{
	return _Displayer;
}

// *********************************************************************************************************
void CDisplayerLua::CToLua::executeHandler(const CLuaString &eventName, int numArgs)
{
	CLuaState &ls = *getLua();
	CLuaStackRestorer lsr(&ls, ls.getTop() - numArgs);
	//
	if (!_LuaTable.isValid()) return; // init failed
	if (_LuaTable[ eventName.getStr().c_str() ].isNil()) return; // event not handled
	static volatile bool dumpStackWanted = false;
	if (dumpStackWanted) ls.dumpStack();
	_LuaTable[ eventName.getStr().c_str() ].push();
	if (dumpStackWanted) ls.dumpStack();
	// put method before its args
	ls.insert(- numArgs - 1);
	if (dumpStackWanted) ls.dumpStack();
	// this is a method call
	_LuaTable.push();
	if (dumpStackWanted) ls.dumpStack();
	ls.insert(- numArgs - 1);
	if (dumpStackWanted) ls.dumpStack();
	// First arg always is the instance being displayed
	if (getEnclosing())
		getEnclosing()->getDisplayedInstance()->getLuaProjection().push();
	ls.insert(- numArgs - 1);
	if (dumpStackWanted) ls.dumpStack();
	CLuaIHMRyzom::executeFunctionOnStack(*_LuaTable.getLuaState(),  numArgs + 2,  0);
	if (dumpStackWanted) ls.dumpStack();
}


// *********************************************************************************************************
void CDisplayerLua::CToLua::pushLuaAccess(CLuaState &ls)
{
	if (_LuaTable.isValid())
	{
		nlassert(_LuaTable.getLuaState() == &ls);
		_LuaTable.push();
	}
	else
	{
		ls.pushNil();
	}
}


// *********************************************************************************************************
void CDisplayerLua::pushLuaAccess(CLuaState &ls)
{
	//H_AUTO(R2_CDisplayerLua_pushLuaAccess)
	_ToLua.pushLuaAccess(ls);
}

// *********************************************************************************************************
CLuaState *CDisplayerLua::CToLua::getLua()
{
	if (!_LuaTable.isValid())
	{
		nlwarning("Warning: try to access to a corrupted table");
		return 0;
	}
	return _LuaTable.getLuaState();
}

// *********************************************************************************************************
void CDisplayerLua::onActChanged()
{
	//H_AUTO(R2_CDisplayerLua_onActChanged)
	_ToLua.onActChanged();
}

// *********************************************************************************************************
void CDisplayerLua::onContinentChanged()
{
	//H_AUTO(R2_CDisplayerLua_onContinentChanged)
	_ToLua.onContinentChanged();
}

// *********************************************************************************************************
void CDisplayerLua::onPostCreate()
{
	//H_AUTO(R2_CDisplayerLua_onPostCreate)
	_ToLua.onPostCreate();
}

// *********************************************************************************************************
void CDisplayerLua::onCreate()
{
	//H_AUTO(R2_CDisplayerLua_onCreate)
	_ToLua.onCreate();
}

// *********************************************************************************************************
void CDisplayerLua::onErase()
{
	//H_AUTO(R2_CDisplayerLua_onErase)
	_ToLua.onErase();
}

// *********************************************************************************************************
void CDisplayerLua::onPreHrcMove()
{
	//H_AUTO(R2_CDisplayerLua_onPreHrcMove)
	_ToLua.onPreHrcMove();
}

// *********************************************************************************************************
void CDisplayerLua::onPostHrcMove()
{
	//H_AUTO(R2_CDisplayerLua_onPostHrcMove)
	_ToLua.onPostHrcMove();
}

// *********************************************************************************************************
void CDisplayerLua::onFocus(bool focused)
{
	//H_AUTO(R2_CDisplayerLua_onFocus)
	_ToLua.onFocus(focused);
}

// *********************************************************************************************************
void CDisplayerLua::onSelect(bool selected)
{
	//H_AUTO(R2_CDisplayerLua_onSelect)
	_ToLua.onSelect(selected);
}

// *********************************************************************************************************
void CDisplayerLua::onAttrModified(const std::string &attrName,sint32 index)
{
	//H_AUTO(R2_CDisplayerLua_onAttrModified)
	_ToLua.onAttrModified(attrName, index);
}

// *********************************************************************************************************
void CDisplayerLua::onTargetInstanceCreated(const std::string &refMakerAttr, sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CDisplayerLua_onTargetInstanceCreated)
	_ToLua.onTargetInstanceCreated(refMakerAttr, refMakerAttrIndex);
}

// *********************************************************************************************************
void CDisplayerLua::onTargetInstanceErased(const std::string &refMakerAttr, sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CDisplayerLua_onTargetInstanceErased)
	_ToLua.onTargetInstanceErased(refMakerAttr, refMakerAttrIndex);
}

// *********************************************************************************************************
void CDisplayerLua::onTargetInstanceEraseRequested(const std::string &refMakerAttr, sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CDisplayerLua_onTargetInstanceEraseRequested)
	_ToLua.onTargetInstanceEraseRequested(refMakerAttr, refMakerAttrIndex);
}

// *********************************************************************************************************
void CDisplayerLua::onTargetInstanceAttrModified(const std::string &refMakerAttr,sint32 refMakerAttrIndex,const std::string &targetAttrName,sint32 targetAttrIndex)
{
	//H_AUTO(R2_CDisplayerLua_onTargetInstanceAttrModified)
	_ToLua.onTargetInstanceAttrModified(refMakerAttr, refMakerAttrIndex, targetAttrName, targetAttrIndex);
}

// *********************************************************************************************************
void CDisplayerLua::onTargetInstancePreHrcMove(const std::string &refMakerAttr,sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CDisplayerLua_onTargetInstancePreHrcMove)
	_ToLua.onTargetInstancePreHrcMove(refMakerAttr, refMakerAttrIndex);
}

// *********************************************************************************************************
void CDisplayerLua::onTargetInstancePostHrcMove(const std::string &refMakerAttr,sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CDisplayerLua_onTargetInstancePostHrcMove)
	_ToLua.onTargetInstancePostHrcMove(refMakerAttr, refMakerAttrIndex);
}




} // R2
