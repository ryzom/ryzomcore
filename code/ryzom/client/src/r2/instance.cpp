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
//
#include "instance.h"
//
#include "editor.h"
#include "tool_pick.h"
#include "tool_select_rotate.h"
#include "tool_select_move.h"
//
#include "../entities.h"
#include "../interface_v3/interface_manager.h"
#include "nel/gui/lua_ihm.h"
#include "../interface_v3/lua_ihm_ryzom.h"
//
#include "displayer_visual_entity.h"


using namespace NLMISC;

namespace R2
{


// TMP TMP
#define TEST_LUA_PROJ
//#define TEST_LUA_PROJ nlassert(_ToLua._LuaProjection.isUserData());

// *********************************************************************************************************
/*static*/ void setDisplayedInstance(CDisplayerBase  *displayer, CInstance *displayedInstance)
{
	displayer->setDisplayedInstance(displayedInstance);
}

// *********************************************************************************************************
CInstance::CInstance(const CObjectTable *objectTable, CLuaState &ls)
{
	CLuaStackChecker lsc(&ls);
	nlassert(objectTable);
	_ObjectTable = objectTable;
	// create projection in lua
	getEditor().projectInLua(objectTable);
	_ToLua._LuaProjection.pop(getEditor().getLua());
	TEST_LUA_PROJ;
	static volatile bool debugMetatable = false;
	if (debugMetatable)
	{
		_ToLua._LuaProjection.push();
		if (!ls.getMetaTable())
		{
			ls.pop();
			nlwarning("No metatable found");
		}
		else
		{
			CLuaObject mt(ls);
			mt.dump();
		}
		ls.pop();
	}
	_Selectable = true;
	_LastParentOk = false;
	_LastParent = NULL;
	_RegisteredInDispNameList = false;
	_ClassIndex = getEditor().classToIndex(getClassName());
	if (_ClassIndex < 0)
	{
		nlwarning("Class index not found for '%s'", getClassName().c_str());
	}
}

// *********************************************************************************************************
std::string CInstance::getName() const
{
	//H_AUTO(R2_CInstance_getName)
	TEST_LUA_PROJ;
	return getString(_ObjectTable, "Name");
}

// *********************************************************************************************************
std::string CInstance::getPaletteId()
{
	//H_AUTO(R2_CInstance_getPaletteId)
	TEST_LUA_PROJ;
	return getString(_ObjectTable, "Base");
}

// *********************************************************************************************************
ucstring CInstance::getDisplayName()
{
	//H_AUTO(R2_CInstance_getDisplayName)
	TEST_LUA_PROJ;
	CLuaState &ls = getEditor().getLua();
	CLuaStackRestorer lsr(&ls, ls.getTop());
	if (getLuaProjection()["getDisplayName"].isNil())
	{
		// object is not displayed -> ignore name
		return ucstring("");
	}
	if (getLuaProjection().callMethodByNameNoThrow("getDisplayName", 0, 1))
	{
		ucstring result;
		if (CLuaIHM::pop(ls, result)) return result;
	}
	TEST_LUA_PROJ;
	return ucstring("Can't find display name");
}

// *********************************************************************************************************
void CInstance::visit(IInstanceVisitor &visitor)
{
	//H_AUTO(R2_CInstance_visit)
	TEST_LUA_PROJ;
	nlassert(_ObjectTable);
	// forward object vistor call to 'instance' visitor call
	struct CInstanceVisitor : public IObjectVisitor
	{
		IInstanceVisitor *Visitor;
		virtual void visit(CObjectTable &obj)
		{
			CInstance *inst = getEditor().getInstanceFromObject(&obj);
			if (inst)
			{
				Visitor->visit(*inst);
			}
		}
	};
	CInstanceVisitor instanceVisitor;
	instanceVisitor.Visitor = &visitor;
	const_cast<CObjectTable *>(_ObjectTable)->visit(instanceVisitor); // 'const' is respected here, because CInstanceVisitor gives a reference to CInstance
	                                                                  // which in turn only allow a const reference on CObjectTable
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::setDisplayerVisual(CDisplayerVisual *displayer)
{
	//H_AUTO(R2_CInstance_setDisplayerVisual)
	TEST_LUA_PROJ;
	if (_DisplayerVisual)
	{
		setDisplayedInstance(_DisplayerVisual, NULL);
	}
	if (displayer)
	{
		setDisplayedInstance(displayer, this);
	}
	_DisplayerVisual = displayer;
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
CDisplayerVisual *CInstance::getDisplayerVisual() const
{
	//H_AUTO(R2_CInstance_getDisplayerVisual)
	TEST_LUA_PROJ;
	if (!_DisplayerVisual) return NULL;
	return NLMISC::safe_cast<CDisplayerVisual *>((CDisplayerBase *)_DisplayerVisual);
}


// *********************************************************************************************************
void CInstance::setDisplayerUI(CDisplayerBase *displayer)
{
	//H_AUTO(R2_CInstance_setDisplayerUI)
	TEST_LUA_PROJ;
	if (_DisplayerUI)
	{
		setDisplayedInstance(_DisplayerUI, NULL);
	}
	if (displayer)
	{
		setDisplayedInstance(displayer, this);
	}
	_DisplayerUI = displayer;
}

// *********************************************************************************************************
void CInstance::setDisplayerProperties(CDisplayerBase *displayer)
{
	//H_AUTO(R2_CInstance_setDisplayerProperties)
	TEST_LUA_PROJ;
	if (_DisplayerProperties)
	{
		setDisplayedInstance(_DisplayerProperties, NULL);
	}
	if (displayer)
	{
		setDisplayedInstance(displayer, this);
	}
	_DisplayerProperties = displayer;
}

// *********************************************************************************************************
CInstance::~CInstance()
{
	TEST_LUA_PROJ;
	setDisplayerVisual(NULL);
	setDisplayerUI(NULL);
	setDisplayerProperties(NULL);
	TEST_LUA_PROJ;
}


// *********************************************************************************************************
void CInstance::onPreActChanged()
{
	//H_AUTO(R2_CInstance_onPreActChanged)
	TEST_LUA_PROJ;
	if (_DisplayerVisual)		getDisplayerVisual()->onPreActChanged();
	if (_DisplayerUI)			_DisplayerUI->onPreActChanged();
	if (_DisplayerProperties)	_DisplayerProperties->onPreActChanged();
	_ToLua.onActChanged();
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::onActChanged()
{
	//H_AUTO(R2_CInstance_onActChanged)
	TEST_LUA_PROJ;
	if (_DisplayerVisual)		getDisplayerVisual()->onActChanged();
	if (_DisplayerUI)			_DisplayerUI->onActChanged();
	if (_DisplayerProperties)	_DisplayerProperties->onActChanged();
	_ToLua.onActChanged();
	TEST_LUA_PROJ;
	refreshDisplayNameHandle();
}

// *********************************************************************************************************
void CInstance::onContinentChanged()
{
	//H_AUTO(R2_CInstance_onContinentChanged)
	TEST_LUA_PROJ;
	if (_DisplayerVisual)		getDisplayerVisual()->onContinentChanged();
	if (_DisplayerUI)			_DisplayerUI->onContinentChanged();
	if (_DisplayerProperties)	_DisplayerProperties->onContinentChanged();
	_ToLua.onContinentChanged();
	TEST_LUA_PROJ;
}


// *********************************************************************************************************
void CInstance::onCreate()
{
	//H_AUTO(R2_CInstance_onCreate)
	TEST_LUA_PROJ;
	if (_DisplayerVisual) getDisplayerVisual()->onCreate();
	if (_DisplayerUI) _DisplayerUI->onCreate();
	if (_DisplayerProperties) _DisplayerProperties->onCreate();
	_ToLua.onCreate();
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::onPostCreate()
{
	//H_AUTO(R2_CInstance_onPostCreate)
	TEST_LUA_PROJ;
	_LastParentOk = false;
	if (_DisplayerVisual) getDisplayerVisual()->onPostCreate();
	if (_DisplayerUI) _DisplayerUI->onPostCreate();
	if (_DisplayerProperties) _DisplayerProperties->onPostCreate();
	_ToLua.onPostCreate();
	TEST_LUA_PROJ;
	nlassert(!_RegisteredInDispNameList);
	getEditor().registerInstanceDispName(getDisplayName(), this);
	_RegisteredInDispNameList = true;
}

// *********************************************************************************************************
void CInstance::onErase()
{
	//H_AUTO(R2_CInstance_onErase)
	TEST_LUA_PROJ;
	if (_DisplayerVisual) getDisplayerVisual()->onErase();
	if (_DisplayerUI) _DisplayerUI->onErase();
	if (_DisplayerProperties) _DisplayerProperties->onErase();
	_ToLua.onErase();
	TEST_LUA_PROJ;
	if (_RegisteredInDispNameList)
	{
		getEditor().unregisterInstanceDispName(this);
		_RegisteredInDispNameList = false;
	}
}

// *********************************************************************************************************
void CInstance::refreshDisplayNameHandle()
{
	if (_RegisteredInDispNameList)
	{
		getEditor().unregisterInstanceDispName(this);
		getEditor().registerInstanceDispName(getDisplayName(), this);
	}
}

// *********************************************************************************************************
void CInstance::onPreHrcMove()
{
	//H_AUTO(R2_CInstance_onPreHrcMove)
	TEST_LUA_PROJ;
	if (_DisplayerVisual) getDisplayerVisual()->onPreHrcMove();
	if (_DisplayerUI) _DisplayerUI->onPreHrcMove();
	if (_DisplayerProperties) _DisplayerProperties->onPreHrcMove();
	_ToLua.onPreHrcMove();
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::onPostHrcMove()
{
	//H_AUTO(R2_CInstance_onPostHrcMove)
	TEST_LUA_PROJ;
	_LastParentOk = false;
	if (_DisplayerVisual) getDisplayerVisual()->onPostHrcMove();
	if (_DisplayerUI) _DisplayerUI->onPostHrcMove();
	if (_DisplayerProperties) _DisplayerProperties->onPostHrcMove();
	_ToLua.onPostHrcMove();
	TEST_LUA_PROJ;
	refreshDisplayNameHandle();
}


// *********************************************************************************************************
void CInstance::onFocus(bool highlighted)
{
	//H_AUTO(R2_CInstance_onFocus)
	TEST_LUA_PROJ;
	if (_DisplayerVisual) getDisplayerVisual()->onFocus(highlighted);
	if (_DisplayerUI) _DisplayerUI->onFocus(highlighted);
	if (_DisplayerProperties) _DisplayerProperties->onFocus(highlighted);
	_ToLua.onFocus(highlighted);
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::onSelect(bool selected)
{
	//H_AUTO(R2_CInstance_onSelect)
	TEST_LUA_PROJ;
	if (_DisplayerVisual) getDisplayerVisual()->onSelect(selected);
	if (_DisplayerUI) _DisplayerUI->onSelect(selected);
	if (_DisplayerProperties) _DisplayerProperties->onSelect(selected);
	_ToLua.onSelect(selected);
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::onAttrModified(const std::string &attrName, sint32 attrIndex)
{
	//H_AUTO(R2_CInstance_onAttrModified)
	TEST_LUA_PROJ;
	if (attrName == "InstanceId")
	{
		nlwarning("InstanceId modification not allowed !! Please create a new object with a new id");
		_Id.clear(); // nevertheless invalidate the cache
	}
	if (_DisplayerVisual) getDisplayerVisual()->onAttrModified(attrName, attrIndex);
	if (_DisplayerUI) _DisplayerUI->onAttrModified(attrName, attrIndex);
	if (_DisplayerProperties) _DisplayerProperties->onAttrModified(attrName, attrIndex);
	_ToLua.onAttrModified(attrName, attrIndex);
	TEST_LUA_PROJ;
	if (attrName == "Name" || attrName == "Title")
	{
		refreshDisplayNameHandle();
	}
}

// *********************************************************************************************************
/*void CInstance::onTableModified(const std::string &tableName, const std::string &keyInTable, sint32 indexInTable)
{
	if (_DisplayerVisual) getDisplayerVisual()->onTableModified(tableName, keyInTable, indexInTable);
	if (_DisplayerUI) _DisplayerUI->onTableModified(tableName, keyInTable, indexInTable);
	if (_DisplayerProperties) _DisplayerProperties->onTableModified(tableName, keyInTable, indexInTable);
}*/

// *********************************************************************************************************
void CInstance::onTargetInstanceCreated(const std::string &refMakerAttr, sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CInstance_onTargetInstanceCreated)
	TEST_LUA_PROJ;
	if (_DisplayerVisual) getDisplayerVisual()->onTargetInstanceCreated(refMakerAttr, refMakerAttrIndex);
	if (_DisplayerUI) _DisplayerUI->onTargetInstanceCreated(refMakerAttr, refMakerAttrIndex);
	if (_DisplayerProperties) _DisplayerProperties->onTargetInstanceCreated(refMakerAttr, refMakerAttrIndex);
	_ToLua.onTargetInstanceCreated(refMakerAttr, refMakerAttrIndex);
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::onTargetInstanceErased(const std::string &refMakerAttr, sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CInstance_onTargetInstanceErased)
	TEST_LUA_PROJ;
	if (_DisplayerVisual) getDisplayerVisual()->onTargetInstanceErased(refMakerAttr, refMakerAttrIndex);
	if (_DisplayerUI) _DisplayerUI->onTargetInstanceErased(refMakerAttr, refMakerAttrIndex);
	if (_DisplayerProperties) _DisplayerProperties->onTargetInstanceErased(refMakerAttr, refMakerAttrIndex);
	_ToLua.onTargetInstanceErased(refMakerAttr, refMakerAttrIndex);
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::onTargetInstancePreHrcMove(const std::string &refMakerAttr,sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CInstance_onTargetInstancePreHrcMove)
	TEST_LUA_PROJ;
	if (_DisplayerVisual) getDisplayerVisual()->onTargetInstancePreHrcMove(refMakerAttr, refMakerAttrIndex);
	if (_DisplayerUI) _DisplayerUI->onTargetInstancePreHrcMove(refMakerAttr, refMakerAttrIndex);
	if (_DisplayerProperties) _DisplayerProperties->onTargetInstancePreHrcMove(refMakerAttr, refMakerAttrIndex);
	_ToLua.onTargetInstancePreHrcMove(refMakerAttr, refMakerAttrIndex);
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::onTargetInstancePostHrcMove(const std::string &refMakerAttr,sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CInstance_onTargetInstancePostHrcMove)
	TEST_LUA_PROJ;
	if (_DisplayerVisual) getDisplayerVisual()->onTargetInstancePostHrcMove(refMakerAttr, refMakerAttrIndex);
	if (_DisplayerUI) _DisplayerUI->onTargetInstancePostHrcMove(refMakerAttr, refMakerAttrIndex);
	if (_DisplayerProperties) _DisplayerProperties->onTargetInstancePostHrcMove(refMakerAttr, refMakerAttrIndex);
	_ToLua.onTargetInstancePostHrcMove(refMakerAttr, refMakerAttrIndex);
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::onTargetInstanceEraseRequested(const std::string &refMakerAttr,sint32 refMakerAttrIndex)
{
	//H_AUTO(R2_CInstance_onTargetInstanceEraseRequested)
	TEST_LUA_PROJ;
	// forward to displayer for convenience & legacy code, but should not be handled by displayers ...
	if (_DisplayerVisual) getDisplayerVisual()->onTargetInstanceEraseRequested(refMakerAttr, refMakerAttrIndex);
	if (_DisplayerUI) _DisplayerUI->onTargetInstanceEraseRequested(refMakerAttr, refMakerAttrIndex);
	if (_DisplayerProperties) _DisplayerProperties->onTargetInstanceEraseRequested(refMakerAttr, refMakerAttrIndex);
	//
	_ToLua.onTargetInstanceEraseRequested(refMakerAttr, refMakerAttrIndex);
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::onTargetInstanceAttrModified(const std::string &refMakerAttr, sint32 refMakerAttrIndex, const std::string &targetAttrName, sint32 targetAttrIndex)
{
	//H_AUTO(R2_CInstance_onTargetInstanceAttrModified)
	TEST_LUA_PROJ;
	if (_DisplayerVisual) getDisplayerVisual()->onTargetInstanceAttrModified(refMakerAttr, refMakerAttrIndex, targetAttrName, targetAttrIndex);
	if (_DisplayerUI) _DisplayerUI->onTargetInstanceAttrModified(refMakerAttr, refMakerAttrIndex, targetAttrName, targetAttrIndex);
	if (_DisplayerProperties) _DisplayerProperties->onTargetInstanceAttrModified(refMakerAttr, refMakerAttrIndex, targetAttrName, targetAttrIndex);
	_ToLua.onTargetInstanceAttrModified(refMakerAttr, refMakerAttrIndex, targetAttrName, targetAttrIndex);
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::onPreRender()
{
	//H_AUTO(R2_CInstance_onPreRender)
	TEST_LUA_PROJ;
	if (_DisplayerVisual) getDisplayerVisual()->onPreRender();
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
void CInstance::onPostRender()
{
	//H_AUTO(R2_CInstance_onPostRender)
	TEST_LUA_PROJ;
	if (_DisplayerVisual) getDisplayerVisual()->onPostRender();
	TEST_LUA_PROJ;
}

// *********************************************************************************************************
CLuaObject &CInstance::getLuaProjection()
{
	//H_AUTO(R2_CInstance_getLuaProjection)
	TEST_LUA_PROJ;
	nlassert(_ToLua._LuaProjection.isValid());
	TEST_LUA_PROJ;
	return _ToLua._LuaProjection;
}

// *********************************************************************************************************
std::string CInstance::getClassName() const
{
	//H_AUTO(R2_CInstance_getClassName)
	TEST_LUA_PROJ;
	return getString(_ObjectTable, "Class");
}

// *********************************************************************************************************
CLuaObject &CInstance::getClass() const
{
	//H_AUTO(R2_CInstance_getClass)
	TEST_LUA_PROJ;
	if (_ToLua._Class.isValid()) return _ToLua._Class;
	_ToLua._Class = getEditor().getClasses()[getClassName()];
	return _ToLua._Class;
}

// *********************************************************************************************************
TInstanceId CInstance::getId() const
{
	//H_AUTO(R2_CInstance_getId)
	TEST_LUA_PROJ;
	if (!_Id.empty()) return _Id;
	nlassert(_ObjectTable);
	_Id = getString(_ObjectTable, "InstanceId");
	return _Id;
}


// *********************************************************************************************************
CEntityCL *CInstance::getEntity() const
{
	//H_AUTO(R2_CInstance_getEntity)
	TEST_LUA_PROJ;
	CDisplayerVisualEntity *dve = dynamic_cast<CDisplayerVisualEntity *>(getDisplayerVisual());
	if (dve)
	{
		return dve->getEntity();
	}
	return NULL;
}

// *********************************************************************************************************
std::string CInstance::getSheet() const
{
	CDisplayerVisualEntity *dve = dynamic_cast<CDisplayerVisualEntity *>(getDisplayerVisual());
	if (dve)
	{
		return dve->getSheet();
	}
	return "";
}

// *********************************************************************************************************
CInstance *CInstance::getParent() const
{
	//H_AUTO(R2_CInstance_getParent)
	TEST_LUA_PROJ;
	static volatile bool cacheTest = true;
	if (cacheTest)
	{
		if (_LastParentOk)
		{
			return _LastParent;
		}
	}
	CInstance *result = NULL;
	CObject *currParent = _ObjectTable->getParent();
	while (currParent)
	{
		CInstance *inst = getEditor().getInstanceFromObject(currParent);
		if (inst)
		{
			result = inst;
			break;
		}
		currParent = currParent->getParent();
	}
	_LastParentOk = true;
	_LastParent = result;
	return result;
}

// *********************************************************************************************************
bool CInstance::isSonOf(CInstance *other) const
{
	//H_AUTO(R2_CInstance_isSonOf)
	TEST_LUA_PROJ;
	CInstance *curr =this->getParent();
	while (curr)
	{
		if (curr == other) return true;
		curr = curr->getParent();
	}
	return false;
}

// *********************************************************************************************************
bool CInstance::isKindOf(const std::string &className) const
{
	//H_AUTO(R2_CInstance_isKindOf)
	CEditor &ed = getEditor();
	return ed.isKindOf(_ClassIndex, ed.classToIndex(className));
}

// *********************************************************************************************************
CInstance *CInstance::getParentAct() const
{
	//H_AUTO(R2_CInstance_getParentAct)
	TEST_LUA_PROJ;
	CInstance *curr = getParent();
	while (curr)
	{
		if (curr->isKindOf("Act")) return curr;
		curr = curr->getParent();
	}
	return NULL;
}

// *********************************************************************************************************
void CInstance::CToLua::executeHandler(const CLuaString &name, int numArgs)
{
	//H_AUTO(R2_CToLua_executeHandler)
	CLuaState &ls = getEditor().getLua();
	CLuaStackRestorer lsr(&ls, ls.getTop() - numArgs);
	//
	static volatile bool dumpStackWanted = false;
	if (dumpStackWanted) ls.dumpStack();
	_Class[ name.getStr().c_str() ].push();
	if (ls.isNil(-1)) return; // not handled
	if (dumpStackWanted) ls.dumpStack();
	// put method before its args
	ls.insert(- numArgs - 1);
	if (dumpStackWanted) ls.dumpStack();
	// Insert the 'this' as first parameter
	_LuaProjection.push();
	ls.insert(- numArgs - 1);
	//
	if (dumpStackWanted) ls.dumpStack();
	CLuaIHMRyzom::executeFunctionOnStack(ls,  numArgs + 1,  0);
	if (dumpStackWanted) ls.dumpStack();
}

// *********************************************************************************************************
CLuaState *CInstance::CToLua::getLua()
{
	//H_AUTO(R2_CToLua_getLua)
	return _LuaProjection.getLuaState();
}


// *********************************************************************************************************
sint CInstance::getSelectedSequence() const
{
	//H_AUTO(R2_CInstance_getSelectedSequence)
	CLuaObject selected = const_cast<CInstance *>(this)->getLuaProjection()["User"]["SelectedSequence"];
	sint index = 0;
	if (selected.isNumber())
	{
		index = (sint) selected.toNumber();
	}
	return index;
}


////////////////
// PROPERTIES //
////////////////

// *********************************************************************************************************
void CInstance::setSelectable(bool selectable)
{
	//H_AUTO(R2_CInstance_setSelectable)
	TEST_LUA_PROJ;
	if (selectable == _Selectable) return;
	_Selectable = selectable;
	CInstance *selInstance = getEditor().getSelectedInstance();
	if (selInstance)
	{
		if (selInstance && (selInstance == this || selInstance->isSonOf(this)))
		{
			getEditor().setSelectedInstance(NULL);
		}
	}
	onAttrModified("Selectable");
}

// *********************************************************************************************************
bool CInstance::getSelectableFromRoot() const
{
	//H_AUTO(R2_CInstance_getSelectableFromRoot)
	TEST_LUA_PROJ;
	const CInstance *curr = this;
	do
	{
		if (!curr->getSelectable()) return false;
		curr = curr->getParent();
	}
	while (curr);
	return true;
}

// *********************************************************************************************************
void CInstance::dummySetSelectableFromRoot(bool /* selectable */)
{
	//H_AUTO(R2_CInstance_dummySetSelectableFromRoot)
	nlwarning("SelectableFromRoot is a R/O property");
}

// *********************************************************************************************************
bool CInstance::getGhost() const
{
	//H_AUTO(R2_CInstance_getGhost)
	TEST_LUA_PROJ;
	if (_ObjectTable) return getObjectTable()->getGhost();
	return false;
}
// *********************************************************************************************************
void CInstance::setGhost(bool ghost)
{
	//H_AUTO(R2_CInstance_setGhost)
	TEST_LUA_PROJ;
	if (_ObjectTable) getObjectTable()->setGhost(ghost);
}

// *********************************************************************************************************
CInstance *CInstance::getParentGroup()
{
	//H_AUTO(R2_CInstance_getParentGroup)
	if (isKindOf("NpcGrpFeature")) return this;
	if (getParent() && getParent()->isKindOf("NpcGrpFeature"))
	{
		return getParent();
	}
	return this;
}

// *********************************************************************************************************
const CInstance *CInstance::getParentGroupLeader() const
{
	//H_AUTO(R2_CInstance_getParentGroupLeader)
	const CObjectTable *props = NULL;
	if (isKindOf("NpcGrpFeature"))
	{
		props = getObjectTable();
	}
	else if (getParent() && getParent()->isKindOf("NpcGrpFeature"))
	{
		props = getParent()->getObjectTable();
	}
	else
	{
		return this;
	}
	if (!props) return NULL;
	// this is a group
	const CObject *components = props->findAttr("Components");
	if (!components || components->getSize() == 0)
	{
		return NULL;
	}
	return getEditor().getInstanceFromObject(components->getValue(0));
}

// *********************************************************************************************************
CObject *CInstance::getGroupSelectedSequence() const
{
	//H_AUTO(R2_CInstance_getGroupSelectedSequence)
	const CInstance *leader = getParentGroupLeader();
	if (leader)
	{
		sint selectedSequence = leader->getSelectedSequence();
		const CObject *behav = leader->getObjectTable();
		if (behav)
		{
			behav = behav->findAttr("Behavior");
			if (behav)
			{
				const CObject *activities = behav->findAttr("Activities");
				if (activities)
				{
					if (selectedSequence >= 0 && selectedSequence < (sint) activities->getSize())
					{
						return activities->getValue((sint32) selectedSequence);
					}
				}
			}
		}
	}
	return NULL;
}


// *********************************************************************************************************
bool CInstance::maxVisibleEntityExceeded() const
{
	if (!_DisplayerVisual) return false;
	return _DisplayerVisual->maxVisibleEntityExceeded();
}

// *********************************************************************************************************
std::string CInstance::getPosInstanceId() const
{
	//H_AUTO(R2_CInstance_getPosInstanceId)
	CObject *posObj = getObjectTable()->getAttr("Position");
	if (!posObj)
	{
		nlwarning("<CInstance::getPosInstanceId> can't retrieve position from object");
		return "";
	}
	return posObj->toString("InstanceId");
}


/////////////////////
// ACTION HANDLERS //
/////////////////////

// *********************************************************************************************************
// Select an instance from its id
class CAHSelectInstance : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// retrieve instance from its Id
		CInstance *instance = getEditor().getInstanceFromId(sParams);
		if(instance != NULL)
		{
			getEditor().setSelectedInstance(instance);
		}
	}
};
REGISTER_ACTION_HANDLER(CAHSelectInstance, "r2ed_select_instance");


// *********************************************************************************************************
// Delete selected instance
class CAHDeleteSelectedInstance : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInstance *selectedInstance = getEditor().getSelectedInstance();
		if (selectedInstance)
		{
			getEditor().getDMC().requestEraseNode(selectedInstance->getId(), "", -1);
		}
	}
};
REGISTER_ACTION_HANDLER(CAHDeleteSelectedInstance, "r2ed_delete_selected_instance");

// *********************************************************************************************************
// handler to pick an instance
class CAHPickerLua : public IActionHandler
{
	//	 TODO nico : replace this action handler by a CTool (in the same way that CToolChoosePosLua derives from CToolChoosePos)
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// TODO : put this class in a separate file
		class CToolPickLua : public CToolPick
		{
		public:
			CToolPickLua() {}
			CToolPickLua(const std::string &cursCanPickInstance,
						const std::string &cursCannotPickInstance,
						const std::string &cursCanPickPos,
						const std::string &cursCannotPickPos,
						bool wantMouseUp
					   ) : CToolPick(cursCanPickInstance,
									 cursCannotPickInstance,
									 cursCanPickPos,
									 cursCannotPickPos,
									 wantMouseUp
									)
			{}

			NLMISC_DECLARE_CLASS(CToolPickLua);
			void pick(CInstance &instance)
			{
				CTool::TSmartPtr holdThis(this); // prevent 'setCurrentTool' from deleting 'this'
				getEditor().setCurrentTool(NULL);
				if (!LuaPickFunc.empty())
				{
					getEditor().getLua().executeScriptNoThrow(LuaPickFunc + "('" + instance.getId() + "')");
				}
			}
			void pick(const CVector &pos)
			{
				CTool::TSmartPtr holdThis(this); // prevent 'setCurrentTool' from deleting 'this'
				getEditor().setCurrentTool(NULL);
				if (!LuaPickPosFunc.empty())
				{
					getEditor().getLua().executeScriptNoThrow(NLMISC::toString("%s(%f, %f, %f)", LuaPickPosFunc.c_str(), pos.x, pos.y, pos.z));
				}
			}
			bool canPick(const CInstance &instance) const
			{
				if (LuaPickFunc.empty()) return true;
				CLuaState &lua = getEditor().getLua();
				int initialSize = lua.getTop();
				std::string script = "return " + LuaTestFunc + "('" + instance.getId() + "')";
				if (lua.executeScriptNoThrow(script, 1))
				{
					bool result = lua.toBoolean(initialSize + 1);
					lua.setTop(initialSize);
					return result;
				}
				lua.setTop(initialSize);
				return false;
			}

			std::string LuaTestFunc;
			std::string LuaPickFunc;
			std::string LuaPickPosFunc;
		};
		std::string cursCanPickInstance = getParam(sParams, "CursCanPickInstance");
		std::string cursCannotPickInstance = getParam(sParams, "CursCannotPickInstance");
		std::string cursCanPickPos = getParam(sParams, "CursCanPickPos");
		std::string cursCannotPickPos = getParam(sParams, "CursCannotPickPos");
		std::string wantMouseUp = getParam(sParams, "WantMouseUp");
		std::string ignoreInstances = getParam(sParams, "IgnoreInstances");
		//
		if (cursCanPickInstance.empty())  cursCanPickInstance = "r2ed_tool_can_pick.tga";
		if (cursCannotPickInstance.empty())  cursCannotPickInstance = "curs_stop.tga";
		if (cursCanPickPos.empty())  cursCanPickPos = "r2ed_tool_pick.tga";
		if (cursCannotPickPos.empty())  cursCannotPickPos = "r2ed_tool_pick.tga";
		//
		CToolPickLua *picker = new CToolPickLua(cursCanPickInstance, cursCannotPickInstance, cursCanPickPos, cursCannotPickPos, wantMouseUp == "true");
		picker->LuaTestFunc = getParam(sParams, "TestFunc");
		picker->LuaPickFunc = getParam(sParams, "PickFunc");
		picker->LuaPickPosFunc = getParam(sParams, "PickPosFunc");
		picker->setIgnoreInstances(ignoreInstances);
		getEditor().setCurrentTool(picker);
	}
};
REGISTER_ACTION_HANDLER(CAHPickerLua, "r2ed_picker_lua");

// *********************************************************************************************************
// rotate
class CAHRotateInstance : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		getEditor().setCurrentTool(new CToolSelectRotate);
	}
};
REGISTER_ACTION_HANDLER(CAHRotateInstance, "r2ed_rotate");

// *********************************************************************************************************
// move
class CAHMoveInstance : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		getEditor().setCurrentTool(new CToolSelectMove);
	}
};
REGISTER_ACTION_HANDLER(CAHMoveInstance, "r2ed_move");



// *********************************************************************************************************
// Debug : dump the lua table for current instance
class CAHDumpLuaTable : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		if (!getEditor().getSelectedInstance()) return;
		std::string maxDepthStr = getParam(sParams, "depth");
		uint maxDepth;
		if (maxDepthStr.empty())
			maxDepth = 20;
		else
			fromString(maxDepthStr, maxDepth);
		// don't want to display the parent
		std::set<const void *> negativeFilter;
		if (getEditor().getSelectedInstance()->getParent())
		{
			negativeFilter.insert(getEditor().getSelectedInstance()->getParent()->getLuaProjection().toPointer());
		}
		getEditor().getSelectedInstance()->getLuaProjection().dump(30, &negativeFilter);
	}
};
REGISTER_ACTION_HANDLER(CAHDumpLuaTable, "r2ed_dump_lua_table");




} // R2


