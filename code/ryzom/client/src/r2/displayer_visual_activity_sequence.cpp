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
#include "displayer_visual_activity_sequence.h"
#include "displayer_visual_group.h"
#include "r2_config.h"
//
#include "nel/gui/ctrl_quad.h"
#include "../interface_v3/interface_manager.h"
#include "nel/gui/view_renderer.h"


using namespace NLMISC;

namespace R2
{

// *********************************************************************************************************
CDisplayerVisualActivitySequence::CDisplayerVisualActivitySequence()
{
	_Touched = true;
	_Active = false;
	_AddedToWorldMap = false;
	_DecalColor = CRGBA(0, 0, 0, 0);
}

// *********************************************************************************************************
CDisplayerVisualActivitySequence::~CDisplayerVisualActivitySequence()
{
	clear();
	_WPCache.clear();
}

// *********************************************************************************************************
void CDisplayerVisualActivitySequence::clear(bool wantRemoveFromWorldMap)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_clear)
	for(uint k = 0; k < _ObserverHandles.size(); ++k)
	{
		getEditor().removeInstanceObserver(_ObserverHandles[k]);
	}
	_ObserverHandles.clear();
	_Decals.clear();
	_FootSteps.clear();
	_WPCache.clear();
	if (wantRemoveFromWorldMap)
	{
		removeFromWorldMap();
	}
	_TraversedPrimInfos.clear();
}

// *********************************************************************************************************
void CDisplayerVisualActivitySequence::onAttrModified(const std::string &attrName,sint32 /* attrIndex */)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_onAttrModified)
	if (attrName == "Components")
	{
		touch();
	}
}

// *********************************************************************************************************
void CDisplayerVisualActivitySequence::touch()
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_touch)
	_Touched = true;
}

// *********************************************************************************************************
CDisplayerVisual *CDisplayerVisualActivitySequence::getParentDV() const
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_getParentDV)
	CInstance *currParent = getDisplayedInstance()->getParent();
	CDisplayerVisual *prevDV = NULL;
	while (currParent)
	{
		prevDV = currParent->getDisplayerVisual();
		if (prevDV) break;
		currParent = currParent->getParent();
	}
	return prevDV;
}

// *********************************************************************************************************
void CDisplayerVisualActivitySequence::update()
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_update)
	if (!_Active) return;
	if (!_Touched) return;
	_Touched = false;
	CObjectTable *activities = getProps().toTable("Components");
	if (!activities)
	{
		clear();
		return;
	}
	// get first world object parent to get start position
	nlassert(getDisplayedInstance());
	CInstance *prevZone = NULL;
	CDisplayerVisual *prevDV = getParentDV();
	if (!prevDV)
	{
		clear();
		return;
	}
	//
	clear(false);
	//
	for(uint k = 0; k < activities->getSize(); ++k)
	{
		// search next zone of activity
		CObjectTable *activity = activities->getValue(k)->toTable();
		if (!activity) continue;
		std::string activityStr = getString(activity, "Activity");
		if (activityStr == "Stand Still" || activityStr == "Inactive") continue;
		//
		CInstance *nextZone = NULL;
		std::string zoneId = getString(activity, "ActivityZoneId");
		//
		if (!zoneId.empty())
		{
			_ObserverHandles.push_back(getEditor().addInstanceObserver(zoneId, this));
			nextZone = getEditor().getInstanceFromId(zoneId);
		}

		if (!nextZone) break;
		CDisplayerVisual *nextDV = nextZone->getDisplayerVisual();
		if (!nextDV) break;

		_TraversedPrimInfos.push_back(CTraversedPrimInfo());
		_TraversedPrimInfos.back().PrimDisplay = nextDV;
		_TraversedPrimInfos.back().Visible = nextDV->getActualDisplayMode() != DisplayModeHidden;


		CWorldPosCache wpc;
		wpc.DV = nextDV;
		wpc.WorldPos2f = nextDV->getWorldPos2f();
		_WPCache.push_back(wpc);

		if (nextDV->getActualDisplayMode() != DisplayModeHidden
			&& prevDV->getActualDisplayMode() != DisplayModeHidden)
		{
			// special case for regions
			if (nextZone->isKindOf("Region"))
			{
				// first case : previous zone is not a region
				if (!prevZone || !prevZone->isKindOf("Region"))
				{
					// search shortest distance bewteen last pos and the region
					CVector entryPos;
					if (nextDV->evalEnterPoint(prevDV->evalExitPoint(), entryPos))
					{
						addFootSteps(CLine(prevDV->evalExitPoint(), entryPos));
					}
					else
					{
						addWanderSteps(prevDV->evalExitPoint());
					}
				}
				else
				{
					// region-region footsteps
					// just use the couple of vertices for which the distance is the smallest
					static std::vector<CVector2f> r0;
					static std::vector<CVector2f> r1;
					prevDV->getSonsWorldPos2f(r0);
					nextDV->getSonsWorldPos2f(r1);
					if (!r0.empty() && !r1.empty())
					{
						CVector2f p0(0.f, 0.f), p1(0.f, 0.f);
						float bestDist = FLT_MAX;
						for(uint k = 0; k < r0.size(); ++k)
						{
							for(uint l = 0; l < r0.size(); ++l)
							{
								float dist = (r0[k] - r1[l]).norm();
								if (dist <bestDist)
								{
									bestDist = dist;
									p0 = r0[k];
									p1 = r1[l];
								}
							}
						}
						nlassert(bestDist != FLT_MAX);
						addFootSteps(CLine(p0.asVector(), p1.asVector()));
					}
				}
			}
			else
			{
				// special case if prev zone is a region
				if (prevZone && prevZone->isKindOf("Region"))
				{
					// search shortest distance bewteen last pos and the region
					CVector entryPos;
					if (prevDV->evalEnterPoint(nextDV->evalLinkPoint(), entryPos))
					{
						addFootSteps(CLine(entryPos, nextDV->evalLinkPoint()));
					}
					else
					{
						addWanderSteps(nextDV->evalLinkPoint());
					}
				}
				else
				{
					// simple footsteps between last & new pos
					addFootSteps(CLine(prevDV->evalExitPoint(), nextDV->evalLinkPoint()));
				}
			}
		}
		prevDV   = nextDV;
		prevZone = nextZone;
	}
	//
	CGroupMap *gm = CTool::getWorldMap();
	if (!_AddedToWorldMap && gm)
	{
		gm->addDeco(this);
	}
	if (_AddedToWorldMap)
	{
		setWorldMapNumEdges((uint)_FootSteps.size());
		nlassert(gm);
		onUpdate(*gm);
	}
}

// *********************************************************************************************************
void CDisplayerVisualActivitySequence::addFootSteps(const NLMISC::CLine &line)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_addFootSteps)
	CDecal *decal = new CDecal;
	decal->setTexture(CV_FootStepDecalTexture.get(),  false,  true);
	CVector2f start2f(line.V0.x,  line.V0.y);
	CVector2f end2f(line.V1.x,  line.V1.y);
	float dist = (end2f - start2f).norm();
	if (dist != 0.f)
	{
		decal->setWorldMatrixForArrow(start2f,  end2f,  CV_FootStepDecalWidth.get());
		CMatrix uvMatrix;
		uvMatrix.setScale(CVector(CV_FootStepDecalUScale.get() * dist,  1.f,  1.f));
		decal->setTextureMatrix(uvMatrix);
	}
	_Decals.push_back(decal);
	_FootSteps.push_back(line);
}

// *********************************************************************************************************
void CDisplayerVisualActivitySequence::addWanderSteps(const CVector &pos)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_addWanderSteps)
	CDecal *decal = new CDecal;
	decal->setTexture(CV_WanderDecalTexture.get(),  true,  true);
	decal->setWorldMatrixForSpot(CVector2f(pos.x, pos.y), CV_WanderDecalSize.get());
	_Decals.push_back(decal);
}

enum TSequenceState { Hidden = 0, HasFocus, Selected };
static void updateState(TSequenceState &state, CDisplayerVisual *dv)
{
	if (dv->getDisplayFlag(CDisplayerVisual::FlagHasFocus)) state = (TSequenceState) std::max((sint) state, (sint) HasFocus);
	if (dv->getDisplayFlag(CDisplayerVisual::FlagSelected)) state = (TSequenceState) std::max((sint) state, (sint) Selected);
}


// *********************************************************************************************************
CDisplayerVisual *CDisplayerVisualActivitySequence::getPossibleGroupDV(CDisplayerVisual *entityDV)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_getPossibleGroupDV)
	if (!entityDV) return NULL;
	CInstance *parentGroup = entityDV->getDisplayedInstance()->getParent();
	while (parentGroup)
	{
		if (dynamic_cast<CDisplayerVisualGroup *>(parentGroup->getDisplayerVisual()))
		{
			return parentGroup->getDisplayerVisual();
			break;
		}
		parentGroup = parentGroup->getParent();
	}
	return entityDV;
}

// *********************************************************************************************************
bool CDisplayerVisualActivitySequence::isVisible(CDisplayerVisual *groupDV, CDisplayerVisual *entityDV)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_isVisible)
	if (!entityDV || !groupDV || !_Active) return false;
	if (entityDV->getActualDisplayMode() == DisplayModeHidden)
	{
			return false;
	}
	// see if render is on for primitives (don't use a config var because may be dynamically changed)
	if (entityDV->getDisplayFlag(CDisplayerVisual::FlagHideActivities))
	{
		return false; // when selected for auto group, the activity won't be duplicated
	}
	// If in a group and not the leader of this group, or not selected, don't display
	if(entityDV != groupDV)
	{
		if (groupDV->getSon(0) != entityDV) return false;
	}
	//if (!getEditor().getConfig()["PrimDisplayEnabled"].toBoolean()) return false;
	return true;
}

// *********************************************************************************************************
void CDisplayerVisualActivitySequence::removeFromWorldMap()
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_removeFromWorldMap)
	if (_AddedToWorldMap)
	{
		CGroupMap *gm = CTool::getWorldMap();
		nlassert(gm);
		gm->removeDeco(this);
		_AddedToWorldMap = false;
	}
}


// *********************************************************************************************************
void CDisplayerVisualActivitySequence::onPostRender()
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_onPostRender)
	CDisplayerVisual *entityDV = getParentDV();
	CDisplayerVisual *groupDV = getPossibleGroupDV(entityDV);
	if (!isVisible(groupDV, entityDV))
	{
		removeFromWorldMap();
		return;
	}
	//
	TSequenceState state = Hidden;
	// if this activity sequence is not the selected one in its parent then it is hidden
	updateState(state, groupDV);
	// if current selection is a son of the group...
	static std::vector<CDisplayerVisual *> groupSons;
	groupDV->getSons(groupSons);
	for(uint k = 0; k < groupSons.size(); ++k)
	{
		nlassert(groupSons[k]);
		updateState(state, groupSons[k]);
	}
	// if one of the route is selected or highlighted
	CObjectTable *activities = getProps().toTable("Components");
	if (!activities)
	{
		removeFromWorldMap();
		return;
	}
	// get first world object parent to get start position
	for(uint k = 0; k < activities->getSize(); ++k)
	{
		// search next zone of activity
		CObjectTable *activity = activities->getValue(k)->toTable();
		if (!activity) continue;
		std::string zoneId = getString(activity, "ActivityZoneId");
		if (zoneId.empty()) continue;
		std::string activityStr = getString(activity, "Activity");
		if (activityStr == "Stand Still" || activityStr == "Inactive") continue;
		CInstance *zone = getEditor().getInstanceFromId(zoneId);
		if (!zone) continue;
		CDisplayerVisual *dv = zone->getDisplayerVisual();
		if (!dv) continue;
		updateState(state, dv);
		static std::vector<CDisplayerVisual *> vertices;
		dv->getSons(vertices);
		for (uint l = 0; l < vertices.size(); ++l)
		{
			nlassert(vertices[l]);
			updateState(state, vertices[l]);
		}
	}
	//
	if (state != Hidden && entityDV)
	{
		// see if this sequence is the selected one
		sint index = entityDV->getDisplayedInstance()->getSelectedSequence();
		CObject *sequences = getProps().getParent(); // see what is my index in the sequences list of my parent
		if (sequences)
		{
			clamp(index, (sint) 0, (sint) sequences->getSize() - 1);
			if (index != sequences->findIndex(&getProps()))
			{
				state = Hidden; // not the current selected sequence
			}
		}
	}
	//
	_DecalColor = CV_FootStepDecalSelectedColor.get();
	CRGBA mapColor = CV_FootStepMapSelectedColor.get();
	switch(state)
	{
		case Hidden:
			_DecalColor = CV_FootStepDecalHiddenColor.get();
			mapColor = CV_FootStepMapHiddenColor.get();
		break;
		case HasFocus:
			_DecalColor = CV_FootStepDecalFocusedColor.get();
			mapColor = CV_FootStepMapFocusedColor.get();
		break;
		default:
		break;
	}
	for(uint k = 0; k < _WPCache.size(); ++k)
	{
		if (_WPCache[k].DV)
		{
			if (_WPCache[k].DV->getWorldPos2f() != _WPCache[k].WorldPos2f)
			{
				touch();
				break;
			}
		}
	}
	update();
	if (_AddedToWorldMap)
	{
		setWorldMapColor(mapColor);
	}
}

// *********************************************************************************************************
void CDisplayerVisualActivitySequence::onPreRender()
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_onPreRender)
	CDisplayerVisual *entityDV = getParentDV();
	CDisplayerVisual *groupDV = getPossibleGroupDV(entityDV);
	if (!isVisible(groupDV, entityDV))
	{
		removeFromWorldMap();
		return;
	}
	// if visibility of one of the group has been modified then update our display
	bool recompute = false;
	for(uint k = 0; k < _TraversedPrimInfos.size(); ++k)
	{
		if (!_TraversedPrimInfos[k].PrimDisplay)
		{
			recompute = true;
			break;
		}
		bool visible = (_TraversedPrimInfos[k].PrimDisplay->getActualDisplayMode() != DisplayModeHidden);
		if (visible != _TraversedPrimInfos[k].Visible)
		{
			recompute = true;
			break;
		}
	}
	if (recompute)
	{
		touch();
		update();
	}
	//
	for(uint k = 0; k < _Decals.size(); ++k)
	{
		_Decals[k]->setDiffuse(_DecalColor);
		_Decals[k]->addToRenderList();
	}
}

// *********************************************************************************************************
void CDisplayerVisualActivitySequence::updateWorldPos()
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_updateWorldPos)
	touch(); // world pos of containing parent is modified
}


// *********************************************************************************************************
void CDisplayerVisualActivitySequence::onInstanceCreated(CInstance &/* instance */)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_onInstanceCreated)
	touch();
}

// *********************************************************************************************************
void CDisplayerVisualActivitySequence::onInstanceErased(CInstance &/* instance */)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_onInstanceErased)
	touch();
}

// *********************************************************************************************************
void CDisplayerVisualActivitySequence::onAttrModified(CInstance &/* instance */,const std::string &/* attrName */,sint32 /* attrIndex */)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_onAttrModified)
	touch();
}

// *********************************************************************************************************
void CDisplayerVisualActivitySequence::setActive(bool active)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_setActive)
	if (active == _Active) return;
	if (!active)
	{
		clear();
	}
	else
	{
		touch();
	}
	_Active = active;
}

// *********************************************************************************************************
bool CDisplayerVisualActivitySequence::getActive() const
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_getActive)
	return _Active;
}


// *********************************************************
void CDisplayerVisualActivitySequence::setWorldMapNumEdges(uint count)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_setWorldMapNumEdges)
	// TODO nico : try to factor with CPrimRender::setWorldMapNumEdges
	nlassert(_AddedToWorldMap);
	CGroupMap *gm = CTool::getWorldMap();
	nlassert(gm);
	if (count < _WorldMapEdges.size())
	{
		for(uint k = count; k < _WorldMapEdges.size(); ++k)
		{
			gm->delCtrl(_WorldMapEdges[k]);
		}
	}
	else
	{
		uint left = count - (uint)_WorldMapEdges.size();
		CViewBase::TCtorParam param;
		while (left --)
		{
			CCtrlQuad *cq = new CCtrlQuad( param );
			cq->setModulateGlobalColor(false);
			cq->setActive(true);
			gm->addCtrl(cq);
			cq->setParent(gm);
			cq->setRenderLayer(1);
			_WorldMapEdges.push_back(cq);
		}
	}
	_WorldMapEdges.resize(count);
	for(uint k = 0; k < count; ++k)
	{
		_WorldMapEdges[k]->setTexture(CV_FootStepMapTexture.get());
	}
}

// *********************************************************
void CDisplayerVisualActivitySequence::onAdd(CGroupMap &/* owner */)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_onAdd)
	nlassert(!_AddedToWorldMap);
	_AddedToWorldMap = true;
	_Touched = true;
	setWorldMapNumEdges((uint)_FootSteps.size());
}

// *********************************************************
void CDisplayerVisualActivitySequence::onRemove(CGroupMap &/* owner */)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_onRemove)
	nlassert(_AddedToWorldMap);
	setWorldMapNumEdges(0);
	_AddedToWorldMap = false;
}

// *********************************************************
void CDisplayerVisualActivitySequence::onPreRender(CGroupMap &/* owner */)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_onPreRender)

}

// *********************************************************
void CDisplayerVisualActivitySequence::onUpdate(CGroupMap &owner)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_onUpdate)
	nlassert(_AddedToWorldMap);
	nlassert(_FootSteps.size() == _WorldMapEdges.size());
	CViewRenderer &vr = *CViewRenderer::getInstance();
	sint32 id = vr.getTextureIdFromName(CV_FootStepMapTexture.get());
	sint32 width, height;
	vr.getTextureSizeFromId(id, width, height);
	float invWorldTextureWidth = width > 0 ? 1.f / width : 0.f;
	for(uint k = 0; k < _WorldMapEdges.size(); ++k)
	{
		sint32 px0, py0;
		sint32 px1, py1;
		owner.worldToWindowSnapped(px0, py0, _FootSteps[k].V0);
		owner.worldToWindowSnapped(px1, py1, _FootSteps[k].V1);
		CVector start((float) px0, (float) py0, 0.f);
		CVector end((float) px1, (float) py1, 0.f);
		_WorldMapEdges[k]->setQuad(start, end, CV_FootStepMapWidth.get());
		_WorldMapEdges[k]->setPattern(0.f, (start - end).norm() * invWorldTextureWidth, CCtrlQuad::Repeat);
		_WorldMapEdges[k]->updateCoords();
	}
}

// *********************************************************
void CDisplayerVisualActivitySequence::setWorldMapColor(CRGBA color)
{
	//H_AUTO(R2_CDisplayerVisualActivitySequence_setWorldMapColor)
	nlassert(_AddedToWorldMap);
	for(uint k = 0; k < _WorldMapEdges.size(); ++k)
	{
		_WorldMapEdges[k]->setColorRGBA(color);
	}
}

} // R2
