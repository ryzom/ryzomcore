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

#ifndef R2_DISPLAYER_BASE_H
#define R2_DISPLAYER_BASE_H

#include "nel/misc/smart_ptr.h"
#include "nel/misc/class_registry.h"
#include "nel/gui/interface_element.h"

namespace NLGUI
{
	class CLuaObject;
}

namespace R2
{

class CInstance;
class CObjectTable;

/** Base class to display instances / editor objects
  * Each instance may have several displayers attached to it :
  * - a displayer for the 3D scene (derived from CDisplayerVisual...)
  * - one for the ui (displaying instance in the object tree for example)
  * - one for the property sheet of the instance
  */
class CDisplayerBase : public NLMISC::IClassable, public NLGUI::CReflectableRefPtrTarget
{
public:
	typedef NLMISC::CSmartPtr<CDisplayerBase> TSmartPtr;
	CDisplayerBase();
	virtual ~CDisplayerBase();
	// Init parameters from script
	virtual bool init(const CLuaObject &/* parameters */) { return true; }
	// Get the instance being displayed
	CInstance   *getDisplayedInstance() const { return _DisplayedInstance; }
	// Push lua access to the displayer
	virtual void pushLuaAccess(CLuaState &ls);
	/////////////////////////////////////
	// EVENTS (same than in CInstance) //
	/////////////////////////////////////
	virtual void onPreActChanged() {}
	virtual void onActChanged() {}
	virtual void onContinentChanged() {}
	virtual void onCreate() {}
	virtual void onPostCreate() {}
	virtual void onErase() {}
	virtual void onPreHrcMove() {}	// instance is about to move in the hierarchy of objects
	virtual void onPostHrcMove() {}  // instance has moved in the hierarchy of objects
	virtual void onFocus(bool /* focused */) {}
	virtual void onSelect(bool /* selected */) {}
	/** An single attribute / a table has been modified in the object
	  * This msg propagate to parent until the top of the hierarchy is reached.
	  */
	virtual void onAttrModified(const std::string &/* attrName */, sint32 /* attrIndex */) {}
	//virtual void onTableModified(const std::string &tableName, const std::string &keyInTable, sint32 indexInTable) {}

	////////////////////////////////////////////////////////
	// TARGETED INSTANCES EVENTS (same than in CInstance) //
	////////////////////////////////////////////////////////
	virtual void onTargetInstancePreHrcMove(const std::string &/* refMakerAttr */, sint32 /* refMakerAttrIndex */) {}
	virtual void onTargetInstancePostHrcMove(const std::string &/* refMakerAttr */, sint32 /* refMakerAttrIndex */) {}
	virtual void onTargetInstanceCreated(const std::string &/* refMakerAttr */, sint32 /* refMakerAttrIndex */) {}
	virtual void onTargetInstanceErased(const std::string &/* refMakerAttr */, sint32 /* refMakerAttrIndex */) {}
	virtual void onTargetInstanceEraseRequested(const std::string &/* refMakerAttr */, sint32 /* refMakerAttrIndex */) {}
	virtual void onTargetInstanceAttrModified(const std::string &/* refMakerAttr */, sint32 /* refMakerAttrIndex */,
										   const std::string &/* targetAttrName */, sint32 /* targetAttrIndex */) {}


	// shortcut to lua projection of displayed instance
	CLuaObject &getLuaProjection();
	const CObjectTable &getProps() const;

	REFLECT_EXPORT_START(R2::CDisplayerBase, CReflectable)
			// lua
	REFLECT_EXPORT_END


	virtual bool maxVisibleEntityExceeded() const { return false; }


///////////////////////////////////////////////////////////////////////////////////////////////
private:
	CInstance *_DisplayedInstance;
private:
	// NB : with use a tierce function there because making CInstance::setVisualDisplayer a friend create a header cyclic dependency between CBaseDisplayer
	// and CInstance
	friend void setDisplayedInstance(CDisplayerBase  *displayer, CInstance *displayedInstance);
protected:
	virtual void   setDisplayedInstance(CInstance *instance) { _DisplayedInstance = instance; }
public:
	static uint ObjCount;
};

} // R2

#endif
