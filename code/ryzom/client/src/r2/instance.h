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

#ifndef R2_INSTANCE_H
#define R2_INSTANCE_H

#include "nel/misc/smart_ptr.h"
//
#include "nel/gui/lua_object.h"
//
#include "game_share/object.h"
#include "displayer_base.h"
#include "nel/gui/interface_element.h"
#include "lua_event_forwarder.h"

class CEntityCL;
namespace NLGUI
{
class CLuaState;
}

namespace R2
{
class CDisplayerVisual;
class CInstance;

// unique identifier of an instance in the current module
typedef std::string TInstanceId;


// callback for instance tree traversal
struct IInstanceVisitor
{
	virtual void visit(CInstance &inst) = 0;
};

void setDisplayedInstance(CDisplayerBase  *displayer, CInstance *displayedInstance);


/** An instance in the editor (should be created from a CEditor object)
  * An instance allows to attach displayers to an objects in the scenario,
  *
  * \TODO : find a better name that 'CInstance', something like 'CClientObject', 'CClientView' or 'CEditorObject' ...
  */

class CInstance : public NLMISC::CRefCount, public CReflectable
{
public:
	typedef NLMISC::CSmartPtr<CInstance> TSmartPtr;
	typedef NLMISC::CRefPtr<CInstance>   TRefPtr;
	// shortcut to get the id of this instance
	TInstanceId		   getId() const;
	// shortcut to get the "Name" field in utf8 format (or returns an empty string if not present)
	std::string		   getName() const;
	// get the display name for an entity
	ucstring		   getDisplayName();
	// recursively visit the tree of instances rooted at this instance
	void			   visit(IInstanceVisitor &visitor);
	// get class name for that instance
	std::string        getClassName() const;
	// get the index of the class of this object
	sint			   getClassIndex()const { return _ClassIndex; }
	// get class definition in lua for that instance (nil if not found)
	CLuaObject         &getClass() const;
	// Get the lua projection for the CObjectInstance that goes with that object
	CLuaObject         &getLuaProjection();
	// Advanced : get the counterpart CObjectTable object	(which is wrapped by the object
	// returned by 'getLuaProjection').
	const CObjectTable *getObjectTable() const { return _ObjectTable; }
	// TMP TMP TMP for lag compensation (should never write in object properties directly...)
	CObjectTable *getObjectTable() { return const_cast<CObjectTable *>(_ObjectTable); }
	// get id in the palette
	std::string getPaletteId();
	// Get this instance parent
	CInstance		   *getParent() const;
	// Test if that instance is son of another one
	bool			   isSonOf(CInstance *other) const;
	// Get the parent act
	CInstance		   *getParentAct() const;
	/** See if this instance is of the wanted class (or of a derived one)
	  * \TODO nico : duplicated in lua code, see if worth merging
	  */
	bool			   isKindOf(const std::string &className) const;
	// if this object is inside a group, get the group leader, returns the object else, or NULL on error
	const CInstance	   *getParentGroupLeader() const;
	// get parent group if any, or return this object else
	CInstance		   *getParentGroup();
	// get possible parent group

	// if this object is a logic entity, return its selected sequence, elsedefault to 0
	sint				getSelectedSequence() const;
	// resturn selected activity sequence for this entity. If a group entity or a group, return the current sequence for the group
	CObject			   *getGroupSelectedSequence() const;

	// get the instance id for the position object into this one
	std::string getPosInstanceId() const;

	////////////////
	// DISPLAYERS //
	////////////////

	void setDisplayerVisual(CDisplayerVisual *displayer);
	CDisplayerVisual *getDisplayerVisual() const;
	void setDisplayerUI(CDisplayerBase *displayer);
	CDisplayerBase *getDisplayerUI() const { return _DisplayerUI; }
	void setDisplayerProperties(CDisplayerBase *displayer);
	CDisplayerBase *getDisplayerProperties() const { return _DisplayerProperties; }

	////////////
	// EVENTS //
	////////////
	void onPreActChanged();
	void onActChanged();
	void onContinentChanged();
	// onCreate is called at creation, and before sons are created
	void onCreate();
	// onPostCreate after creation of sons is done
	void onPostCreate();
	void onErase();
	void onPreHrcMove();	// instance is about to move in the hierarchy of object
	void onPostHrcMove();  // instance has moved in the hierarchy of objects
	void onFocus(bool focused);
	void onSelect(bool selected);
	/** An attribute inside this object has been modified
	  * \param attrName Name of the attribute inside this object, as given by its class definition. If the attribute
      *                 is an array, then an additionnal parameter gives the index of the element being modified in the array (or -1 if the whole array is set)
	  * \param attrIndex Index in the element in its array, or -1 if attrName doesn't refer to an array, or if the whole array is modified
	  */
	void onAttrModified(const std::string &attrName, sint32 attrIndex = -1);
	//virtual void onTableModified(const std::string &tableName, const std::string &keyInTable, sint32 indexInTable);
	//
	void onPreRender();
	void onPostRender();

	/////////////////////////////////
	// REFERENCED INSTANCES EVENTS //
	/////////////////////////////////

	/** A targeted instance has been created
	  * nico : I expect this event to be the result of an undo (will be implemented later, server side)
	  * may also happen if net msg telling that an instance is created arrives after its targeter is created
	  *
	  * \param refMaker Name of the attribute that reference the instance (not the instance id)
	  * \param refMakerAttrIndex If the reference maker is in an array, then refMakerAttr gives the name of the array, and refMakerAttrIndex
	  *                          give its indexin the array
	  */
	void onTargetInstanceCreated(const std::string &refMakerAttr, sint32 refMakerAttrIndex);
	// a targeted instance has been removed
	void onTargetInstanceErased(const std::string &refMakerAttr, sint32 refMakerAttrIndex);
	/** A targeted instance deletion request is about to be sent.
	  * This allow reference makers to delete themselves
	  * Unlike other notifications, this is not forwarded to the displayers
	  * This is because the client that delete a reference target is responsible to
	  * remove the reference target too (by reacting to that message in lua code)
	  */
	void onTargetInstanceEraseRequested(const std::string &refMakerAttr, sint32 refMakerAttrIndex);
	// an attribute of a targeted instance has been modified
	void onTargetInstanceAttrModified(const std::string &refMakerAttr, sint32 refMakerAttrIndex,
									  const std::string &targetAttrName, sint32 targetAttrIndex);
	// Target instance is about to move in the hierarchy of object
	void onTargetInstancePreHrcMove(const std::string &refMakerAttr, sint32 refMakerAttrIndex);
	// Target instance has moved in the hierarchy of objects
	void onTargetInstancePostHrcMove(const std::string &refMakerAttr, sint32 refMakerAttrIndex);


	/////////////
	// HELPERS //
	/////////////

	// If this entity is displayed using a CEntityCL instance, get it
	CEntityCL *getEntity() const;
	// If this entity is displayed using a CEntityCL instance, return its sheet (Entity may be NULL if more then 254 entities are displayed)
	std::string getSheet() const;

	////////////////
	// PROPERTIES //
	////////////////
	// nb : usually, when they are modified, displayers should be notified of it

	// Is the object selectable ?
	bool getSelectable() const { return _Selectable; }
	void setSelectable(bool selectable);
	// test select flag, including ancestors
	bool getSelectableFromRoot() const;
	void dummySetSelectableFromRoot(bool selectable ); // for export (this prop is read only)

	//Is object a ghost object ?
	bool getGhost() const;
	void setGhost(bool ghost);

	//////////////////////////////////////////////////////////////////
	// EXPORTED PROPERTIES (a.k.a native properties in the lua doc) //
	//////////////////////////////////////////////////////////////////

	REFLECT_EXPORT_START(R2::CInstance, CReflectable)
		REFLECT_BOOL("Selectable", getSelectable, setSelectable);
		REFLECT_BOOL("SelectableFromRoot", getSelectableFromRoot, dummySetSelectableFromRoot);
		REFLECT_BOOL("Ghost", getGhost, setGhost);
	REFLECT_EXPORT_END


	// is this object outnumbering the max number of visible entities (displayed in scene as an exclamation mark)
	bool maxVisibleEntityExceeded() const;

private:
	CDisplayerBase::TSmartPtr				   _DisplayerVisual;
	CDisplayerBase::TSmartPtr				   _DisplayerUI;
	CDisplayerBase::TSmartPtr				   _DisplayerProperties;
	class CToLua : public CLuaEventForwarder
	{
	public:
		CLuaObject							   _LuaProjection; // projection in lua of this instance properties & methods
		mutable CLuaObject					   _Class; // shortcut to class definition of this instance
		virtual CLuaState *getLua();
		virtual void executeHandler(const CLuaString &eventName, int numArgs);
	};
	CToLua									   _ToLua;
	const CObjectTable						   *_ObjectTable;	// the real datas for that object
	mutable TInstanceId						   _Id;				// cache for this object instance id
	bool									   _Selectable;
	sint									   _ClassIndex;
	//
	// Caching of current parent pointer
	mutable bool				 _LastParentOk;
	mutable CInstance			 *_LastParent;
	//
	bool						  _RegisteredInDispNameList;

private:
	friend class CEditor;
	// For editor : Create this object from the CObjectTable it materialize in the editor
	CInstance(const CObjectTable *objectTable, CLuaState &ls);
	// copy not supported
	CInstance(const CInstance &/* other */):NLMISC::CRefCount() { nlassert(0); }
	CInstance &operator = (const CInstance &/* other */) { nlassert(0); return *this; }
	//
	void executeHandler(const CLuaString &name, int numArgs);
	//
	void refreshDisplayNameHandle();
public:
	~CInstance();
};

} // R2

#endif
