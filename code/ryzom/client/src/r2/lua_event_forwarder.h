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

#ifndef R2_LUA_EVENT_FORWARDER_H
#define R2_LUA_EVENT_FORWARDER_H

namespace NLGUI
{
	class CLuaState;
	class CLuaString;
}

using namespace NLGUI;

namespace R2
{

/** helper class to forward events to a lua table
  * Derivers should tell how to handle an event with a given name and number of parameters, and how to access lua
	*/

class CLuaEventForwarder
{
public:
	virtual ~CLuaEventForwarder() {}
	// events
	void onActChanged();
	void onContinentChanged();
	void onPostCreate();
	void onCreate();
	void onErase();
	void onPreHrcMove();	// instance is about to move in the hierarchy of object
	void onPostHrcMove();  // instance has moved in the hierarchy of objects
	void onFocus(bool focused);
	void onSelect(bool selected);
	void onAttrModified(const std::string &attrName, sint32 index);
	//virtual void onTableModified(const std::string &tableName, const std::string &keyInTable, sint32 indexInTable);
	// from CDisplayerBase : event from targeted instances
	void onTargetInstancePreHrcMove(const std::string &refMakerAttr, sint32 refMakerAttrIndex);	// instance is about to move in the hierarchy of object
	void onTargetInstancePostHrcMove(const std::string &refMakerAttr, sint32 refMakerAttrIndex);  // instance has moved in the hierarchy of objects
	void onTargetInstanceCreated(const std::string &refMakerAttr, sint32 refMakerAttrIndex);
	void onTargetInstanceErased(const std::string &refMakerAttr, sint32 refMakerAttrIndex);
	void onTargetInstanceEraseRequested(const std::string &refMakerAttr, sint32 refMakerAttrIndex);
	void onTargetInstanceAttrModified(	const std::string &refMakerAttr, sint32 refMakerAttrIndex,
													const std::string &targetAttrName, sint32 targetAttrIndex);
protected:
	// for derivers
	virtual NLGUI::CLuaState *getLua() = 0;
	virtual void executeHandler(const CLuaString &eventName, int numArgs) = 0;
};

} // R2

#endif
