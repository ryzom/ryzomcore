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
#include "lua_event_forwarder.h"
#include "nel/gui/lua_object.h"


namespace R2
{

// events names

static CLuaString LuaStr_onActChanged("onActChanged");
static CLuaString LuaStr_onContinentChanged("onContinentChanged");
static CLuaString LuaStr_onCreate("onCreate");
static CLuaString LuaStr_onPostCreate("onPostCreate");
static CLuaString LuaStr_onErase("onErase");
static CLuaString LuaStr_onPreHrcMove("onPreHrcMove");
static CLuaString LuaStr_onPostHrcMove("onPostHrcMove");
static CLuaString LuaStr_onFocus("onFocus");
static CLuaString LuaStr_onSelect("onSelect");
static CLuaString LuaStr_onAttrModified("onAttrModified");
static CLuaString LuaStr_onTargetInstanceCreated("onTargetInstanceCreated");
static CLuaString LuaStr_onTargetInstanceErased("onTargetInstanceErased");
static CLuaString LuaStr_onTargetInstanceAttrModified("onTargetInstanceAttrModified");
static CLuaString LuaStr_onTargetInstanceEraseRequested("onTargetInstanceEraseRequested");
static CLuaString LuaStr_onTargetInstancePostHrcMove("onTargetInstancePostHrcMove");
static CLuaString LuaStr_onTargetInstancePreHrcMove("onTargetInstancePreHrcMove");



// *********************************************************************************************************
void CLuaEventForwarder::onActChanged()
{
	//H_AUTO(R2_CLuaEventForwarder_onActChanged)
	if (!getLua()) return;
	executeHandler(LuaStr_onActChanged, 0);
}

// *********************************************************************************************************
void CLuaEventForwarder::onContinentChanged()
{
	//H_AUTO(R2_CLuaEventForwarder_onContinentChanged)
	if (!getLua()) return;
	executeHandler(LuaStr_onContinentChanged, 0);
}

// *********************************************************************************************************
void CLuaEventForwarder::onCreate()
{
	//H_AUTO(R2_CLuaEventForwarder_onCreate)
	if (!getLua()) return;
	executeHandler(LuaStr_onCreate, 0);
}

// *********************************************************************************************************
void CLuaEventForwarder::onPostCreate()
{
	//H_AUTO(R2_CLuaEventForwarder_onPostCreate)
	if (!getLua()) return;
	executeHandler(LuaStr_onPostCreate, 0);
}

// *********************************************************************************************************
void CLuaEventForwarder::onErase()
{
	//H_AUTO(R2_CLuaEventForwarder_onErase)
	if (!getLua()) return;
	executeHandler(LuaStr_onErase, 0);
}

// *********************************************************************************************************
void CLuaEventForwarder::onPreHrcMove()
{
	//H_AUTO(R2_CLuaEventForwarder_onPreHrcMove)
	if (!getLua()) return;
	executeHandler(LuaStr_onPreHrcMove, 0);
}

// *********************************************************************************************************
void CLuaEventForwarder::onPostHrcMove()
{
	//H_AUTO(R2_CLuaEventForwarder_onPostHrcMove)
	if (!getLua()) return;
	executeHandler(LuaStr_onPostHrcMove, 0);
}

// *********************************************************************************************************
void CLuaEventForwarder::onFocus(bool focused)
{
	//H_AUTO(R2_CLuaEventForwarder_onFocus)
	CLuaState *ls = getLua();
	if (!ls) return;
	ls->push(focused);
	executeHandler(LuaStr_onFocus, 1);
}

// *********************************************************************************************************
void CLuaEventForwarder::onSelect(bool selected)
{
	//H_AUTO(R2_CLuaEventForwarder_onSelect)
	CLuaState *ls = getLua();
	if (!ls) return;
	ls->push(selected);
	executeHandler(LuaStr_onSelect, 1);
}

// *********************************************************************************************************
void CLuaEventForwarder::onAttrModified(const std::string &attrName, sint32 index)
{
	//H_AUTO(R2_CLuaEventForwarder_onAttrModified)
	CLuaState *ls = getLua();
	if (!ls) return;
	ls->push(attrName);
	ls->push((double) index);
	executeHandler(LuaStr_onAttrModified, 2);
}

// *********************************************************************************************************
/*void CDisplayerLua::onTableModified(const std::string &tableName, const std::string &keyInTable, sint32 indexInTable)
{
	//H_AUTO(R2_CDisplayerLua_onTableModified)
  CLuaState *ls = getLua();
	if (!ls) return;
	ls->push(tableName);
	ls->push(keyInTable);
	ls->push((double) indexInTable);
	executeHandler("onTableModified", 3);
}*/

// *********************************************************************************************************
void CLuaEventForwarder::onTargetInstanceCreated(const std::string &refMakerAttr, sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CLuaEventForwarder_onTargetInstanceCreated)
	CLuaState *ls = getLua();
	if (!ls) return;
	ls->push(refMakerAttr);
	ls->push((double) refMakerAttrIndex);
	executeHandler(LuaStr_onTargetInstanceCreated, 2);
}

// *********************************************************************************************************
void CLuaEventForwarder::onTargetInstanceErased(const std::string &refMakerAttr, sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CLuaEventForwarder_onTargetInstanceErased)
	CLuaState *ls = getLua();
	if (!ls) return;
	ls->push(refMakerAttr);
	ls->push((double) refMakerAttrIndex);
	executeHandler(LuaStr_onTargetInstanceErased, 2);
}

// *********************************************************************************************************
void CLuaEventForwarder::onTargetInstanceEraseRequested(const std::string &refMakerAttr, sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CLuaEventForwarder_onTargetInstanceEraseRequested)
	CLuaState *ls = getLua();
	if (!ls) return;
	ls->push(refMakerAttr);
	ls->push((double) refMakerAttrIndex);
	executeHandler(LuaStr_onTargetInstanceEraseRequested, 2);
}

// *********************************************************************************************************
void CLuaEventForwarder::onTargetInstanceAttrModified(const std::string &refMakerAttr, sint32 refMakerAttrIndex, const std::string &targetAttrName, sint32 targetAttrIndex)
{
	//H_AUTO(R2_CLuaEventForwarder_onTargetInstanceAttrModified)
	CLuaState *ls = getLua();
	if (!ls) return;
	ls->push(refMakerAttr);
	ls->push((double) refMakerAttrIndex);
	ls->push(targetAttrName);
	ls->push((double) targetAttrIndex);
	executeHandler(LuaStr_onTargetInstanceAttrModified, 4);
}

// *********************************************************************************************************
void CLuaEventForwarder::onTargetInstancePreHrcMove(const std::string &refMakerAttr, sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CLuaEventForwarder_onTargetInstancePreHrcMove)
	CLuaState *ls = getLua();
	if (!ls) return;
	ls->push(refMakerAttr);
	ls->push((double) refMakerAttrIndex);
	executeHandler(LuaStr_onTargetInstancePreHrcMove, 2);
}

// *********************************************************************************************************
void CLuaEventForwarder::onTargetInstancePostHrcMove(const std::string &refMakerAttr, sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CLuaEventForwarder_onTargetInstancePostHrcMove)
	CLuaState *ls = getLua();
	if (!ls) return;
	ls->push(refMakerAttr);
	ls->push((double) refMakerAttrIndex);
	executeHandler(LuaStr_onTargetInstancePostHrcMove, 2);
}

}
