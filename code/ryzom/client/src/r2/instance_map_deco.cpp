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
#include "instance_map_deco.h"
#include "r2_config.h"
#include "tool.h"
//
#include "nel/gui/ctrl_quad.h"
#include "nel/gui/group_container.h"
//
#include "nel/misc/i18n.h"
//
#include "game_share/scenario_entry_points.h"

using namespace NLMISC;

namespace R2
{

// *********************************************************************************************************
void CInstanceMapDeco::CCtrlButtonEntity::getContextHelp(ucstring &help) const
{
	//H_AUTO(R2_CCtrlButtonEntity_getContextHelp)
	help = _Instance.getDisplayName();
	if (help == NLMISC::CI18N::get("uiR2EDNoName")) help.clear();
}

// *********************************************************************************************************
bool CInstanceMapDeco::CCtrlButtonEntity::handleEvent(const NLGUI::CEventDescriptor &/* event */)
{
	//H_AUTO(R2_CCtrlButtonEntity_handleEvent)
	return false; // just a display with tooltip capability
}

// *********************************************************************************************************
CInstanceMapDeco::CInstanceMapDeco()
{
	//H_AUTO(R2_CInstanceMapDeco_CInstanceMapDeco)
	_Main = NULL;
	_Over = NULL;
	_OverInvalid = NULL;
	_Orient = NULL;
	_GlowStar[0] = _GlowStar[1] = NULL;
	_GlowStarActive = false;
	_OrientBlendFactor = 0.f;
	_LastCloseView = false;
	_Instance = NULL;
	_AddedToMap = false;
	_Orientable = false;
	_Active = true;
	_InvalidPos = false;
}

// *********************************************************************************************************
void CInstanceMapDeco::setDisplayedInstance(CInstance *instance, bool orientable)
{
	//H_AUTO(R2_CInstanceMapDeco_setDisplayedInstance)
	nlassert(instance);
	nlassert(!_Instance); // should be called once only
	_Instance = instance;
	_Orientable = orientable;
}

// *********************************************************************************************************
CVector2f CInstanceMapDeco::getWorldPos() const
{
	//H_AUTO(R2_CInstanceMapDeco_getWorldPos)
	nlassert(_Instance);
	CDisplayerVisual *vd = _Instance->getDisplayerVisual();
	if (!vd) return CVector2f::Null;
	return vd->getWorldPos2f();
}

// *********************************************************************************************************
void CInstanceMapDeco::invalidateCoords()
{
	//H_AUTO(R2_CInstanceMapDeco_invalidateCoords)
	nlassert(_Instance);
	nlassert(_Main);
	nlassert(_Over);
	nlassert(_OverInvalid);
	_Main->invalidateCoords();
	_Over->invalidateCoords();
	_OverInvalid->invalidateCoords();
	if (_GlowStar[0])
	{
		_GlowStar[0]->invalidateCoords();
		_GlowStar[1]->invalidateCoords();
	}
	if (_Orient) _Orient->invalidateCoords();
}

// *********************************************************************************************************
CCtrlQuad *CInstanceMapDeco::newQuad(CGroupMap &owner)
{
	//H_AUTO(R2_CInstanceMapDeco_newQuad)
	nlassert(_Instance);
	CViewBase::TCtorParam param;
	CCtrlQuad *q = new CCtrlQuad( param );
	q->setActive(false);
	q->setModulateGlobalColor(false);
	owner.addCtrl(q);
	q->setParent(&owner);
	return q;
}


// *********************************************************************************************************
void CInstanceMapDeco::onAdd(CGroupMap &owner)
{
	//H_AUTO(R2_CInstanceMapDeco_onAdd)
	nlassert(_Instance);
	nlassert(!_Main);
	nlassert(!_Over);
	nlassert(!_OverInvalid);
	_Main = new CCtrlButtonEntity(*_Instance);
	_Main->setPosRef(Hotspot_MM);
	_Main->setParentPosRef(Hotspot_BL);
	_Main->setModulateGlobalColorAll(false);
	owner.addCtrl(_Main);
	_Main->setParent(&owner);
	_Main->setRenderLayer(2);
	_Main->setId(owner.getId() + ":" + _Instance->getId());
	_Main->setActive(_Active);
	//
	_Over   = newQuad(owner);
	_Over->setRenderLayer(3);
	_Over->setActive(_Active);
	//
	_OverInvalid = newQuad(owner);
	_OverInvalid->setRenderLayer(4);
	_OverInvalid->setActive(_Active && _InvalidPos);
	//
	if (_Orientable)
	{
		_Orient = newQuad(owner);
		_Orient->setTexture(CV_MapEntityOrientTexture.get());
		_Orient->setRenderLayer(3);
		_Orient->setActive(_Active);
	}
	//
	CInterfaceGroup *window = owner.getParentContainer();
	if (window)
	{
		CViewBase::TCtorParam param;

		for(uint k = 0; k < 2; ++k)
		{
			_GlowStar[k] = new CCtrlQuad( param );
			_GlowStar[k]->setActive(false);
			_GlowStar[k]->setModulateGlobalColor(false);
			window->addCtrl(_GlowStar[k]);
			_GlowStar[k]->setParent(window);
			_GlowStar[k]->setAdditif(true);
			_GlowStar[k]->setTexture(CV_MapGlowStarTexture.get());
		}
	}
	_AddedToMap = true;
}

// *********************************************************************************************************
void CInstanceMapDeco::onRemove(CGroupMap &owner)
{
	//H_AUTO(R2_CInstanceMapDeco_onRemove)
	nlassert(_Instance);
	nlassert(_Main);
	nlassert(_Over);
	nlassert(_OverInvalid);
	owner.delCtrl(_Main);
	_Main = NULL;
	owner.delCtrl(_Over);
	_Over = NULL;
	owner.delCtrl(_OverInvalid);
	_OverInvalid = NULL;
	if (_Orient)
	{
		owner.delCtrl(_Orient);
		_Orient = NULL;
	}
	if (_GlowStar[0])
	{
		_GlowStar[0]->getParent()->delCtrl(_GlowStar[0]);
		_GlowStar[0] = NULL;
		_GlowStar[1]->getParent()->delCtrl(_GlowStar[1]);
		_GlowStar[1] = NULL;
	}
	_AddedToMap = false;
	_Instance = NULL;
}


// *********************************************************************************************************
void CInstanceMapDeco::onPreRender(CGroupMap &groupMap)
{
	//H_AUTO(R2_CInstanceMapDeco_onPreRender)
	if (!_Active) return;
	nlassert(_Instance);
	if (_GlowStarActive)
	{
		if (_GlowStar[0])
		{
			// draw glowing stars on the edge to signal position well
			for(uint k = 0; k < 2; ++k)
			{
				_GlowStar[k]->setActive(true);
				_GlowStar[k]->setQuad(_GlowStarPos, CV_MapGlowStarSize.get(), (float) (CV_MapGlowStarSpeed[k].get() * 0.001 * (double) T1));
				_GlowStar[k]->updateCoords();
			}
		}
	}
	//
	if (_Orient)
	{
		bool closeView = groupMap.getMeterPerPixel() < CV_MapEntityCloseDist.get();
		CDisplayerVisual *vd = _Instance->getDisplayerVisual();
		if (vd)
		{
			if (_LastCloseView!= closeView)
			{
				_OrientBlendFactor = closeView ? 0.5f : 0.f;
			}
			if (vd->getRotateInProgress())
			{
				_OrientBlendFactor = 1.f;
			}
			else
			{
				// fade to default alpha
				NLMISC::incrementalBlend(_OrientBlendFactor, closeView ? 0.5f : 0.f, DT * 1000.f / favoid0(CV_MapEntityOrientBlendTimeInMs.get()));
			}
		}
		else
		{
			_OrientBlendFactor = 0.f;
		}


		if (_OrientBlendFactor == 0.f)
		{
			_Orient->setActive(false);
		}
		else
		{
			_Orient->setActive(true);
			_Orient->setColorRGBA(CRGBA(255, 255, 255, (uint8) (255 * _OrientBlendFactor)));
			CVector2f worldPos = getWorldPos();
			sint32 x;
			sint32 y;
			groupMap.worldToWindowSnapped(x, y, getWorldPos());
			_Orient->setQuad(CV_MapEntityOrientTexture.get(), CVector((float) x, (float) y, 0.f), vd->getAngle(), closeView ? CV_MapEntityOrientOriginDist.get() : CV_MapEntityOrientOriginDistSmall.get());
			_Orient->updateCoords();
		}
		_LastCloseView = closeView;
	}
	if (_OverInvalid->getActive())
	{
		_OverInvalid->setColorRGBA(CTool::getInvalidPosColor());
	}
}

// *********************************************************************************************************
void CInstanceMapDeco::onUpdate(CGroupMap &groupMap)
{
	//H_AUTO(R2_CInstanceMapDeco_onUpdate)
	if (!_Active) return;
	nlassert(_Instance);
	_GlowStarActive = false;
	if (!_Main || !_Over || !_OverInvalid) return;
	sint32 x;
	sint32 y;
	CVector2f worldPos = getWorldPos();
	// if not in current map then don't disply anything
	CIslandCollision &col = getEditor().getIslandCollision();
	R2::CScenarioEntryPoints::CCompleteIsland	*currIsland = col.getCurrIslandDesc();
	if (currIsland)
	{
		if (!currIsland->isIn(worldPos))
		{
			setActive(false);
			return;
		}
	}
	groupMap.worldToWindowSnapped(x, y, getWorldPos());
	_Main->setX(x);
	_Main->setY(y);
	CDisplayerVisual *vd = _Instance->getDisplayerVisual();
	if (!vd)
	{
		_Over->setActive(false);
		_OverInvalid->setActive(false);
		return;
	}
	//
	bool closeView = _CloseTexture.empty() ? false : groupMap.getMeterPerPixel() < CV_MapEntityCloseDist.get();
	//
	bool selected = vd->getDisplayFlag(CDisplayerVisual::FlagSelected);
	bool hasFocus = vd->getDisplayFlag(CDisplayerVisual::FlagHasFocus);
	//
	setTextureAndFit(closeView ? _CloseTexture : CV_MapEntitySmallTexture.get());
	_Main->setColor((selected && ! closeView) ? CV_MapEntitySelectColor.get() : vd->getDisplayModeColorInMap()); // if small icon, then change the icon color directly, because no over will be displayed
	//
	if (selected || hasFocus)
	{
		// if the selection is out of the window, then draw an arrow to locate it
		const CVector2f &wmin = groupMap.getVisibleWorldMin();
		const CVector2f &wmax = groupMap.getVisibleWorldMax();
		if (worldPos.x < wmin.x || worldPos.x > wmax.x ||
			worldPos.y < wmin.y || worldPos.y > wmax.y)
		{
			// OUT OF VISIBLE REGION CASE
			_Over->setActive(true);
			_Over->setColorRGBA(selected ? CV_MapEntitySelectColor.get() : CV_MapEntityHighlightColor.get());
			// out of the visible portion, so draw an arrow instead
			_Over->setTexture(CV_MapEntityFarTexture.get());
			// snap position to inner visible world rect
			CVector2f m = 0.5f * (wmin + wmax);
			CVector2f dir = worldPos - m;
			CVector2f inter;
			float d0;
			float d1;
			if (dir.x > 0.f)
			{
				d0 = (wmax.x - m.x) / dir.x;
				if (dir.y > 0.f)
				{
					d1 = (wmax.y - m.y) / dir.y;
					inter = m + std::min(d0, d1) * dir;
				}
				else if (dir.y < 0.f)
				{
					d1 = (wmin.y - m.y) / dir.y;
					inter = m + std::min(d0, d1) * dir;
				}
				else
				{
					inter.set(wmax.x, m.y);
				}
			}
			else if (dir.x < 0.f)
			{
				d0 = (wmin.x - m.x) / dir.x;
				if (dir.y > 0.f)
				{
					d1 = (wmax.y - m.y) / dir.y;
					inter = m + std::min(d0, d1) * dir;
				}
				else if (dir.y < 0.f)
				{
					d1 = (wmin.y - m.y) / dir.y;
					inter = m + std::min(d0, d1) * dir;
				}
				else
				{
					inter.set(wmin.x, m.y);
				}
			}
			else
			{
				if (dir.y > 0.f)
				{
					inter.set(m.x, wmax.y);
				}
				else if (dir.y < 0.f)
				{
					inter.set(m.x, wmin.y);
				}
				else
				{
					inter = m;
				}
			}
			float size = CV_MapEntityFarArrowSize.get();
			// TMP TMP
			size = size;
			float bias = 1.f;
			dir.normalize();
			CVector2f winInter;
			groupMap.worldToWindow(winInter, inter);

			_Over->setRenderLayer(3);
			_Over->setQuad(winInter - (size + bias) * dir, winInter - bias * dir, 0.5f * size);
			//
			if (_GlowStar[0])
			{
				sint32 screenInterX, screenInterY;
				groupMap.windowToScreen(screenInterX, screenInterY, (sint32) winInter.x, (sint32) winInter.y);
				sint32 refCornerX, refCornerY;
				_GlowStar[0]->getParent()->getCorner(refCornerX, refCornerY, Hotspot_BL);
				_GlowStarPos.set((float) (screenInterX - refCornerX), (float) (screenInterY - refCornerY), 0.f);
				_GlowStarActive = true;
			}
		}
		else
		{
			// VISIBLE CASE
			_GlowStar[0]->setActive(false);
			_GlowStar[1]->setActive(false);
			if (closeView || hasFocus)
			{
				_Over->setActive(true);
				if (!closeView)
				{
					_Over->setColorRGBA(CV_MapEntitySelectColor.get());
				}
				else
				{
					_Over->setColorRGBA(selected ? CV_MapEntitySelectColor.get() : CV_MapEntityHighlightColor.get());
				}
				const std::string &tex = closeView ? CV_MapEntitySelectTexture.get() : CV_MapEntitySmallHighlightTexture.get();
				_Over->setTexture(tex);
				_Over->setRenderLayer(2);
				_Over->setQuad(tex, CVector((float) x, (float) y, 0.f));
			}
			else
			{
				_Over->setActive(false);
			}
		}
	}
	else
	{
		// no focus
		_Over->setActive(false);
		_GlowStar[0]->setActive(false);
		_GlowStar[1]->setActive(false);
	}
	// update 'quad that signal invalid pos'
	if (_OverInvalid->getActive())
	{
		const std::string &tex = closeView ? CV_MapEntityInvalidTexture.get() : CV_MapEntityInvalidTextureSmall.get();
		_OverInvalid->setTexture(tex);
		_OverInvalid->setQuad(tex, CVector((float) x, (float) y, 0.f));
	}
}


// *********************************************************************************************************
void CInstanceMapDeco::setTextureAndFit(const std::string &bitmapName)
{
	//H_AUTO(R2_CInstanceMapDeco_setTextureAndFit)
	nlassert(_Instance);
	nlassert(_Main);
	_Main->setTexture(bitmapName);
	if (!_Main->isTextureValid())
	{
		_Main->setTexture(CV_MapEntityDefaultTexture.get());
		_Main->setX(14);
		_Main->setY(14);
	}
	else
	{
		_Main->fitTexture();
	}
}

// *********************************************************************************************************
void CInstanceMapDeco::setActive(bool active)
{
	//H_AUTO(R2_CInstanceMapDeco_setActive)
	if (active == _Active) return;
	if (_Main) _Main->setActive(active);
	if (_Over) _Over->setActive(active);
	if (_GlowStar[0]) _GlowStar[0]->setActive(active);
	if (_GlowStar[1]) _GlowStar[1]->setActive(active);
	if (_Orient) _Orient->setActive(active);
	_Active = active;
}

// *********************************************************************************************************
void CInstanceMapDeco::setInvalidPosFlag(bool invalid)
{
	//H_AUTO(R2_CInstanceMapDeco_setInvalidPosFlag)
	_InvalidPos = invalid;
	if (_OverInvalid)
	{
		_OverInvalid->setActive(_Active && _InvalidPos);
	}
}



} // R2
