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
#include "auto_group.h"
#include "editor.h"
#include "r2_config.h"
//
#include "nel/misc/i18n.h"

using namespace NLMISC;


namespace R2
{


// ***************************************************************
CAutoGroup::CAutoGroup()
{
	CPrimLook primGroup;
	primGroup.init(getEditor().getEnv()["PrimRender"]["AutoGroupLook"]);
	_AutoGroup.setLook(primGroup);
	_AutoGroupEnabled = false;
}

// ***************************************************************
CAutoGroup::~CAutoGroup()
{
	clear();
}

// ***************************************************************
void CAutoGroup::clear()
{
	//H_AUTO(R2_CAutoGroup__clear)
	if (_AutoGroup.isAddedToWorldMap())
	{
		CGroupMap *gm = CTool::getWorldMap();
		if (gm)
		{
			gm->removeDeco(&_AutoGroup);
		}
	}
	_AutoGroup.clear();
}

// ***************************************************************
void CAutoGroup::update(const CVector &testPos, const std::string &paletteEntry, bool valid)
{
	//H_AUTO(R2_CAutoGroup_update)
	_TestPos = testPos;
	_PaletteEntry = paletteEntry;
	_AutoGroupEnabled = valid;
	CInstance *candidate = getGroupingCandidate();
	if (!candidate && _AutoGroup.isAddedToWorldMap())
	{
		clear();
	}
	else if (candidate)
	{
		// update the display
		if (!_AutoGroup.isAddedToWorldMap())
		{
			CGroupMap *gm = CTool::getWorldMap();
			if (gm)
			{
				gm->addDeco(&_AutoGroup);
			}
		}
		CDisplayerVisual *dv = candidate->getDisplayerVisual();
		nlassert(dv); // should not be null because getGrou^pingCandidate succeeded
		_PrimRenderVertices.resize(2);
		CVector pos = dv->isCompound() ? dv->getSon(0)->getWorldPos() : dv->getWorldPos();
		_PrimRenderVertices[0] = pos;
		_PrimRenderVertices[1] = _TestPos;
		_AutoGroup.setVertices(_PrimRenderVertices);
		_AutoGroup.addDecalsToRenderList();
	}
}

// ***************************************************************
CInstance *CAutoGroup::getGroupingCandidate()
{
	//H_AUTO(R2_CAutoGroup_getGroupingCandidate)
	if (!_AutoGroupEnabled) return NULL;
	// if I'm a bot object, don't auto-group
	CObject *palEntry = getEditor().getDMC().getPaletteElement(_PaletteEntry);
	if (!palEntry || !palEntry->isTable()) return NULL;
	if (getNumber(palEntry, "IsBotObject") == 1) return NULL;
	// auto-group feature
	// look in default feature and sort objects by distance
	CInstance *defaultFeatInst = getEditor().getDefaultFeature(getEditor().getCurrentAct());
	CInstance *baseDefaultFeatInst = getEditor().getDefaultFeature(getEditor().getBaseAct());
	if (!defaultFeatInst || !baseDefaultFeatInst)
	{
		nlwarning("Can't access to Default Features"); // syntax error in lua was making the client crash
		return NULL; //In this case there is no default features
	}
	CObjectTable *defaultFeat = defaultFeatInst->getObjectTable();
	CObjectTable *baseDefaultFeat = baseDefaultFeatInst->getObjectTable();
	CObject *components = defaultFeat->getAttr("Components");
	CObject *baseComponents = baseDefaultFeat->getAttr("Components");
	if (!components || !baseComponents || !palEntry->isTable()) return NULL;
	_SortedComponents.clear();
	for (uint k = 0; k < (components->getSize()+baseComponents->getSize()); ++k)
	{
		CObject *obj = NULL;
		if(k<components->getSize())
			obj = components->getValue(k);
		else
			obj = baseComponents->getValue(k - components->getSize());
		CInstance *inst = getEditor().getInstanceFromObject(obj);
		if (!inst)
		{
			nlwarning("Error: can not find create Instance of an object.");
			continue;
		}
		CDisplayerVisual *dv = inst->getDisplayerVisual();
		if (!dv) continue;
		CComponentSort cs;
		cs.Dist = (_TestPos - dv->getWorldPos()).norm();
		if (cs.Dist > CV_AutoGroupMaxDist.get()) continue;
		cs.Instance = inst;
		_SortedComponents.push_back(cs);
	}
	// iterate through other features
	CObjectTable *act = getEditor().getCurrentAct()->getObjectTable();
	CObjectTable *baseAct = getEditor().getBaseAct()->getObjectTable();
	if (!act || !baseAct) return NULL;
	CObject *features = act->getAttr("Features");
	CObject *baseFeatures = baseAct->getAttr("Features");
	if (!features || !baseFeatures) return NULL;
	for (uint k = 0; k < (features->getSize()+baseFeatures->getSize()); ++k)
	{
		CObject *obj = NULL;
		if(k<features->getSize())
			obj = features->getValue(k);
		else
			obj = baseFeatures->getValue(k - features->getSize());
		CInstance *inst = getEditor().getInstanceFromObject(obj);
		CDisplayerVisual *dv = inst->getDisplayerVisual();
		if (!dv) continue;
		if (inst->isKindOf("NpcGrpFeature"))
		{
			if (dv->getNumSons() == 0) continue;
			CComponentSort cs;
			cs.Dist = (_TestPos - dv->getSon(0)->getWorldPos()).norm();
			if (cs.Dist > CV_AutoGroupMaxDist.get()) continue;
			cs.Instance = inst;
			_SortedComponents.push_back(cs);
		}
	}


	std::sort(_SortedComponents.begin(), _SortedComponents.end());
	CLuaState &ls = getEditor().getLua();
	const CObject *categoryObj = getObject(palEntry, "Category");
	if (!categoryObj)
	{
		nlwarning("No 'Category' field in palEntry '%s'", _PaletteEntry.c_str());
		return NULL;
	}
	if (!categoryObj->isString()) return NULL;
	std::string category = categoryObj->toString();
	//
	const CObject *subCategoryObj = getObject(palEntry, "SubCategory");
	std::string subCategory;
	if (subCategoryObj && subCategoryObj->isString())
	{
		subCategory = subCategoryObj->toString();
	}
	else
	{
		//nlwarning("No 'SubCategory' field in palEntry '%s'", paletteEntry.c_str());
	}

	//
	if (category.empty()) return NULL;
	for(uint k = 0; k < _SortedComponents.size(); ++k)
	{
		CLuaStackRestorer lsr(&ls, 0);
		if (_SortedComponents[k].Instance->isKindOf("Npc"))
		{
			_SortedComponents[k].Instance->getLuaProjection().callMethodByNameNoThrow("isPlant", 0, 1);
			if (ls.toBoolean(-1) == true) continue;
			_SortedComponents[k].Instance->getLuaProjection().callMethodByNameNoThrow("isBotObject", 0, 1);
			if (ls.toBoolean(-1) == true) continue;
		}
		else if (!_SortedComponents[k].Instance->isKindOf("NpcGrpFeature"))
		{
			continue;
		}
		std::string destCategory;
		if (_SortedComponents[k].Instance->getLuaProjection().callMethodByNameNoThrow("getCategory", 0, 1))
		{
			destCategory = ls.toString(-1);
			ls.pop();
		}
		if (destCategory != category) continue;
		//
		std::string destSubCategory;
		if (_SortedComponents[k].Instance->getLuaProjection().callMethodByNameNoThrow("getSubCategory", 0, 1))
		{
			if (ls.isString(-1))
			{
				destSubCategory = ls.toString(-1);
			}
			ls.pop();
		}
		if (destSubCategory != subCategory) continue;
		// good candidate
		return _SortedComponents[k].Instance;
	}
	return NULL;
}


// ***************************************************************
void CAutoGroup::group(CObject *newEntityDesc, const NLMISC::CVectorD &createPosition)
{
	//H_AUTO(R2_CAutoGroup_group)
	CInstance *destGroup = getGroupingCandidate();
	if (!destGroup || !_AutoGroupEnabled) return;
	_AutoGroupEnabled = false; // force user to call 'update' again
	clear();
	// remove any activity, dialog, or event in the copy
	CObject *behav = newEntityDesc->findAttr("Behavior");
	if (behav)
	{
		behav->setObject("Actions", new CObjectTable());
		behav->setObject("Activities", new CObjectTable());
		behav->setObject("ChatSequences", new CObjectTable());
		newEntityDesc->setObject("ActivitiesId", new CObjectTable());
	}
	nlassert(newEntityDesc);
	nlassert(destGroup);
	std::string targetGroupId;
	if (destGroup->isKindOf("NpcGrpFeature"))
	{
		// make relative to newgroup and insert
		CVectorD relPos = createPosition;
		CDisplayerVisual *vd = destGroup->getDisplayerVisual();
		if (vd)
		{
			relPos = relPos - vd->getWorldPos();
		}
		newEntityDesc->setObject("Position", buildVector(relPos));
		targetGroupId = destGroup->getId();
	}
	else
	{
		// other is a standalone entity -> create a new group
		std::auto_ptr<CObject> newGroup(getEditor().getDMC().newComponent("NpcGrpFeature"));
		if (!newGroup.get())
		{
			nlwarning("Syntax error in r2_features_npc_group.lua.");
			getEditor().getDMC().getActionHistoric().endAction();
			getEditor().getDMC().flushActions();
			return;
		}
		ucstring readableName;
		CLuaState &ls = getEditor().getLua();
		R2::getEditor().getEnv()["PaletteIdToGroupTranslation"][newEntityDesc->getAttr("Base")->toString()].push();
		if (ls.isString(-1))
			readableName.fromUtf8(ls.toString(-1));
		ucstring ucGroupName = ucstring(readableName + " " + CI18N::get("uiR2EDNameGroup").toUtf8());

		newGroup->set("Name", getEditor().genInstanceName(ucGroupName).toUtf8());
		getEditor().getDMC().requestInsertNode(destGroup->getParentAct()->getId(),
							   "Features",
							   -1,
							   "",
							   newGroup.get());
		targetGroupId = getString(newGroup.get(), "InstanceId");
		// move target instance in that group (becomes the leader)
		getEditor().getDMC().requestMoveNode(destGroup->getId(), "", -1, targetGroupId, "Components", -1);
	}
	// move newly created entity into target group
	getEditor().getDMC().requestInsertNode(targetGroupId,
							   "Components",
							   -1,
							   "",
							   newEntityDesc);
	getEditor().getDMC().getActionHistoric().endAction();
	getEditor().getDMC().flushActions();
}


} // R2

