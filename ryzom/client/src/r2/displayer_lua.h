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
	virtual bool init(const CLuaObject &parameters) NL_OVERRIDE;
	virtual void pushLuaAccess(CLuaState &ls) NL_OVERRIDE;
	// from CDisplayerBase
	virtual void onActChanged() NL_OVERRIDE;
	virtual void onContinentChanged() NL_OVERRIDE;
	virtual void onPostCreate() NL_OVERRIDE;
	virtual void onCreate() NL_OVERRIDE;
	virtual void onErase() NL_OVERRIDE;
	virtual void onPreHrcMove() NL_OVERRIDE;	// instance is about to move in the hierarchy of object
	virtual void onPostHrcMove() NL_OVERRIDE;  // instance has moved in the hierarchy of objects
	virtual void onFocus(bool focused) NL_OVERRIDE;
	virtual void onSelect(bool selected) NL_OVERRIDE;
	virtual void onAttrModified(const std::string &attrName, sint32 index) NL_OVERRIDE;
	//virtual void onTableModified(const std::string &tableName, const std::string &keyInTable, sint32 indexInTable);
	// from CDisplayerBase : event from targeted instances
	virtual void onTargetInstancePreHrcMove(const std::string &refMakerAttr, sint32 refMakerAttrIndex) NL_OVERRIDE;
	virtual void onTargetInstancePostHrcMove(const std::string &refMakerAttr, sint32 refMakerAttrIndex) NL_OVERRIDE;
	virtual void onTargetInstanceCreated(const std::string &refMakerAttr, sint32 refMakerAttrIndex) NL_OVERRIDE;
	virtual void onTargetInstanceErased(const std::string &refMakerAttr, sint32 refMakerAttrIndex) NL_OVERRIDE;
	virtual void onTargetInstanceEraseRequested(const std::string &refMakerAttr, sint32 refMakerAttrIndex) NL_OVERRIDE;
	virtual void onTargetInstanceAttrModified(	const std::string &refMakerAttr, sint32 refMakerAttrIndex,
													const std::string &targetAttrName, sint32 targetAttrIndex) NL_OVERRIDE;
private:
	class CToLua : public CLuaEventForwarder
	{
	public:
		CToLua();
		CLuaObject _LuaTable; // reference to lua version of the displayer
		CDisplayerLua *_Displayer;
		virtual CLuaState *getLua() NL_OVERRIDE;
		virtual void executeHandler(const CLuaString &eventName, int numArgs) NL_OVERRIDE;
		void pushLuaAccess(CLuaState &ls);
		CDisplayerLua* getEnclosing();
	};
	friend class CToLua;
	CToLua _ToLua;
};


} // R2

#endif
