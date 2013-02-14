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

#ifndef R2_DISPLAYER_LUA_H
#define R2_DISPLAYER_LUA_H

#include "displayer_base.h"
#include "nel/gui/lua_object.h"
#include "lua_event_forwarder.h"

namespace R2
{

class CDisplayerLua : public CDisplayerBase
{
public:
	NLMISC_DECLARE_CLASS(R2::CDisplayerLua);
	CDisplayerLua();
	// expected parameter is a ctor function
	virtual bool init(const CLuaObject &parameters);
	virtual void pushLuaAccess(CLuaState &ls);
	// from CDisplayerBase
	virtual void onActChanged();
	virtual void onContinentChanged();
	virtual void onPostCreate();
	virtual void onCreate();
	virtual void onErase();
	virtual void onPreHrcMove();	// instance is about to move in the hierarchy of object
	virtual void onPostHrcMove();  // instance has moved in the hierarchy of objects
	virtual void onFocus(bool focused);
	virtual void onSelect(bool selected);
	virtual void onAttrModified(const std::string &attrName, sint32 index);
	//virtual void onTableModified(const std::string &tableName, const std::string &keyInTable, sint32 indexInTable);
	// from CDisplayerBase : event from targeted instances
	virtual void onTargetInstancePreHrcMove(const std::string &refMakerAttr, sint32 refMakerAttrIndex);
	virtual void onTargetInstancePostHrcMove(const std::string &refMakerAttr, sint32 refMakerAttrIndex);
	virtual void onTargetInstanceCreated(const std::string &refMakerAttr, sint32 refMakerAttrIndex);
	virtual void onTargetInstanceErased(const std::string &refMakerAttr, sint32 refMakerAttrIndex);
	virtual void onTargetInstanceEraseRequested(const std::string &refMakerAttr, sint32 refMakerAttrIndex);
	virtual void onTargetInstanceAttrModified(	const std::string &refMakerAttr, sint32 refMakerAttrIndex,
													const std::string &targetAttrName, sint32 targetAttrIndex);
private:
	class CToLua : public CLuaEventForwarder
	{
	public:
		CToLua();
		CLuaObject _LuaTable; // reference to lua version of the displayer
		CDisplayerLua *_Displayer;
		virtual CLuaState *getLua();
		virtual void executeHandler(const CLuaString &eventName, int numArgs);
		void pushLuaAccess(CLuaState &ls);
		CDisplayerLua* getEnclosing();
	};
	friend class CToLua;
	CToLua _ToLua;
};


} // R2

#endif
