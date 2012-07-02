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

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"
#include "DialogFlags.h"


using namespace NLMISC;


const std::string		FAUNA_PLACE = "fauna_generic_place";
const float				ARROW_POS = 0.7f;
const float				ARROW_WIDTH = 8.f;
const float				ARROW_LENGTH = 20.f;
const float				MIN_ARROW_LENGTH_BEFORE_SCALE = 4 * ARROW_LENGTH;
const NLMISC::CRGBA     ARROW_COLOR(192, 168, 128, 255);	
const sint				FLAG_ICON_SIZE = 16;

//****************************************************************
CPlugin::CPlugin() : _PluginActive(true), _PluginAccess(NULL), _FaunaFlagIcons(NULL)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	_DialogFlags = new CDialogFlags(this);
}

// ***************************************************************************
CPlugin::~CPlugin()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (_DialogFlags)
	{
		_DialogFlags->DestroyWindow();
	}
	delete _DialogFlags;
	if (_PluginAccess && _FaunaFlagIcons)
	{
		_PluginAccess->deleteTexture(_FaunaFlagIcons);
	}
}

// ***************************************************************************
bool CPlugin::isActive()
{
	return _PluginActive;
}

// ***************************************************************************
std::string &CPlugin::getName()
{
	static std::string ret="Fauna graph";
	return ret;
}

// ***************************************************************************
bool CPlugin::activatePlugin()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if(!_PluginActive)
	{
		//_DialogFlag->ShowWindow(TRUE);	
		_PluginActive=true;
		_DialogFlags->ShowWindow(SW_SHOW);
		return true;
	}

	return false;
}

// ***************************************************************************
bool CPlugin::closePlugin()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if(_PluginActive)
	{
		//_DialogFlag->ShowWindow(FALSE);

		_PluginActive=false;
		_DialogFlags->ShowWindow(SW_HIDE);
		return true;
	}	
	return false;
}

//****************************************************************
void CPlugin::init(IPluginAccess *pluginAccess)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	std::vector<std::string> classNames;
	classNames.push_back(FAUNA_PLACE);
	_PluginAccess = pluginAccess;
	_PluginAccess->registerPrimitiveDisplayer(this, classNames);
	//
	_DialogFlags->Create(IDD_DIALOG_FLAGS, CWnd::FromHandle(_PluginAccess->getMainWindow()->m_hWnd));		
	_DialogFlags->ShowWindow(TRUE);
}

//****************************************************************
void CPlugin::drawPrimitive(const NLLIGO::IPrimitive *primitive, const TRenderContext &renderContext)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	nlassert(primitive);	
	//
	_FaunaGroups.insert(primitive->getParent()); 
	// default drawing
	// NB: default drawing isn't done by caller, because it is assumed
	// that we, as a IPrimitveDisplayer, should do it.
	renderContext.Display->primitiveRenderProxy(*primitive);	
}


class CVertex
{
public:
	NLMISC::CVector	  Pos;
	std::set<sint>	  Arcs;
	bool			  ReachNext;
public:	
	void swap(CVertex &other)
	{
		std::swap(other.Pos, Pos);
		std::swap(other.ReachNext, ReachNext);
		Arcs.swap(other.Arcs);
	}
};


//****************************************************************
void CPlugin::drawArrow(CDisplay &display, const NLMISC::CVector &start, const NLMISC::CVector &end)
{
	if (start == end) return;	
	NLMISC::CVector pixelArrow = ARROW_POS * end + (1.f - ARROW_POS) * start;
	NLMISC::CVector I = (end- start).normed();
	NLMISC::CVector J(I.y, - I.x, 0.f);
	display.pixelVectorToWorld(I);
	display.pixelVectorToWorld(J);
	display.lineRenderProxy(ARROW_COLOR, start, end, 0);
	NLMISC::CVector pixelStart = start;
	NLMISC::CVector pixelEnd = end;
	display.worldToFloatPixel(pixelStart);
	display.worldToFloatPixel(pixelEnd);
	float arrowSizeFactor = 1.f;
	float arrowLength = (pixelStart - pixelEnd).norm();
	if (arrowLength < MIN_ARROW_LENGTH_BEFORE_SCALE)
	{
		arrowSizeFactor = arrowLength / MIN_ARROW_LENGTH_BEFORE_SCALE;
	}
	display.triRenderProxy(ARROW_COLOR, pixelArrow + arrowSizeFactor * ARROW_WIDTH  * J, pixelArrow + arrowSizeFactor * ARROW_LENGTH  * I, pixelArrow - arrowSizeFactor * ARROW_WIDTH  * J, 0);
}


//****************************************************************
void CPlugin::pushIcon(CDisplay &display, sint stepX, sint stepY, NLMISC::CVector &currPos, uint srcX, uint srcY, const CPrimTexture &pt)
{	
	CVector center = currPos;
	display.worldToPixel(center);	
	CVector tl(center.x - FLAG_ICON_SIZE / 2, center.y + FLAG_ICON_SIZE / 2, 0.f);
	CVector br(center.x + FLAG_ICON_SIZE / 2, center.y - FLAG_ICON_SIZE / 2, 0.f);
	display.pixelToWorld(tl);
	display.pixelToWorld(br);
	NLMISC::CQuadColorUV quvc;
	quvc.V0.set(tl.x, tl.y, 0.f);
	quvc.V1.set(br.x, tl.y, 0.f);
	quvc.V2.set(br.x, br.y, 0.f);
	quvc.V3.set(tl.x, br.y, 0.f);
	quvc.Color0 = quvc.Color1 = quvc.Color2 = quvc.Color3 = CRGBA::White;
	if (pt.getWidth() == 0	|| pt.getHeight() == 0)
	{
		// if texture width is 0, then texture hasn't been found, so display the whole 'not found' texture
		quvc.Uv0.set(0.f, 0.f);
		quvc.Uv1.set(1.f, 0.f);
		quvc.Uv2.set(1.f, 1.f);
		quvc.Uv3.set(0.f, 1.f);
	}
	else
	{
		float invWidth  = 1.f / pt.getWidth();
		float invHeight = 1.f / pt.getHeight();
		srcX *= FLAG_ICON_SIZE;
		srcY *= FLAG_ICON_SIZE;
		quvc.Uv0.set(srcX * invWidth, srcY * invHeight);
		quvc.Uv1.set((srcX + FLAG_ICON_SIZE) * invWidth, srcY * invHeight);
		quvc.Uv2.set((srcX + FLAG_ICON_SIZE) * invWidth, (srcY  + FLAG_ICON_SIZE) * invHeight);
		quvc.Uv3.set(srcX * invWidth, (srcY  + FLAG_ICON_SIZE) * invHeight);
	}
	display.texQuadRenderProxy(quvc, 0);
	center.x += (float) stepX;
	center.y += (float) stepY;
	currPos = center;
	display.pixelToWorld(currPos);
}

typedef std::multimap<uint, CVertex> TVertMap;


//****************************************************************
void CPlugin::drawFaunaGraph(CDisplay &display, const NLLIGO::IPrimitive &grp)
{
	if (_DialogFlags->DisplayCondition == CDialogFlags::DisplayWhenSelected)
	{
		if (!_PluginAccess->isSelected(grp))
		{
			// if group is not selected, then see if at least one child is selected
			bool found = false;
			for(uint k = 0; k < grp.getNumChildren(); ++k)
			{
				const NLLIGO::IPrimitive *child;
				if (!grp.getChild(child, k)) continue;
				std::string className;
				if (!child->getPropertyByName("class", className)) continue;
				if (NLMISC::toLower(className) != FAUNA_PLACE) continue;
				if (_PluginAccess->isSelected(*child))
				{
					found = true;
					break;
				}
			}
			if (!found) return;
		}
	}
	TVertMap vertices;
	for(uint k = 0; k < grp.getNumChildren(); ++k)
	{
		const NLLIGO::IPrimitive *child;
		if (!grp.getChild(child, k)) continue;
		std::string className;
		if (!child->getPropertyByName("class", className)) continue;
		if (NLMISC::toLower(className) != FAUNA_PLACE) continue;
		std::string indexStr;
		int index;
		if (!child->getPropertyByName("index", indexStr)) continue;
		if (sscanf(indexStr.c_str(), "%d", &index) != 1) continue;
		CVector pos = child->getPrimVector()[0];
		CVertex vertex;
		vertex.Pos = pos;
		vertex.ReachNext = false;
		std::string reachableIndicesUniqueStr;
		if (!child->getPropertyByName("index_next", reachableIndicesUniqueStr)) continue;		
		std::vector<std::string> reachableIndicesStr;
		NLMISC::explode(reachableIndicesUniqueStr, std::string(","), reachableIndicesStr);
		for(uint k = 0; k < reachableIndicesStr.size(); ++k)
		{			
			if (NLMISC::nlstricmp(reachableIndicesStr[k], "next") == 0)
			{
				vertex.ReachNext = true;				
			}
			int currentIndex;
			if (sscanf(reachableIndicesStr[k].c_str(), "%d", &currentIndex) != 1) continue;
			vertex.Arcs.insert((uint) currentIndex);
		}
		TVertMap::iterator it = vertices.insert(TVertMap::value_type(index, CVertex()));
		it->second.swap(vertex);
		// write index for current vertex		
		CVector textPos = pos;
		display.worldToPixel(textPos);
		if (_DialogFlags->m_DisplayIndices)
		{			
			display.print(toString(index), textPos.x + 4, textPos.y, 12, CRGBA::White, CDisplay::MiddleLeft);
		}
		if (_DialogFlags->m_DisplayTargetIndices)
		{			
			std::string nextIndices;			
			display.print('(' + reachableIndicesUniqueStr + ')', textPos.x, textPos.y - 8, 12, CRGBA::White, CDisplay::MiddleTop);
		}
		// display icons depending on flags
		if (_FaunaFlagIcons && _DialogFlags->m_DisplayFlags)
		{
			CVector delta(- FLAG_ICON_SIZE / 2.f - 2, 0.f, 0.f);
			display.pixelVectorToWorld(delta);
			pos += delta;
			std::string flag;
			child->getPropertyByName("flag_spawn", flag);
			if (flag == "true") pushIcon(display, - FLAG_ICON_SIZE, 0, pos, 0, 0, *_FaunaFlagIcons);
			child->getPropertyByName("flag_food", flag);
			if (flag == "true") pushIcon(display, - FLAG_ICON_SIZE, 0, pos, 1, 0, *_FaunaFlagIcons);
			child->getPropertyByName("flag_rest", flag);
			if (flag == "true") pushIcon(display, - FLAG_ICON_SIZE, 0, pos, 2, 0, *_FaunaFlagIcons);
		}
	}
	for(TVertMap::const_iterator it = vertices.begin(); it != vertices.end(); ++it)
	{
		const CVertex &vertex = it->second;
		if (vertex.ReachNext)
		{
			// find next vertex with highest index, else loop to first index
			TVertMap::const_iterator nextIt = it;
			do
			{
				++ nextIt;
				if (nextIt == vertices.end())
				{
					nextIt = vertices.begin();
					if (nextIt != it && nextIt->first != it->first)
					{
						uint firstIndex = nextIt->first;
						do
						{
							drawArrow(display, vertex.Pos, nextIt->second.Pos);
							++nextIt ;
						}
						while (nextIt->first == firstIndex);
						break;
					}
				}
				if (nextIt->first > it->first)
				{
					uint wantedIndex = nextIt->first;
					do
					{
						drawArrow(display, vertex.Pos, nextIt->second.Pos);
						++ nextIt;
					}
					while (nextIt != vertices.end() && nextIt->first == wantedIndex);
					break;
				}
			}
			while (nextIt != it);			
		}		
		for(std::set<sint>::const_iterator it = vertex.Arcs.begin(); it != vertex.Arcs.end(); ++it)
		{		
			TVertMap::const_iterator first = vertices.lower_bound(*it);
			TVertMap::const_iterator last = vertices.upper_bound(*it);
			for (TVertMap::const_iterator targetIt = first; targetIt != last; ++targetIt)			
			{
				drawArrow(display, vertex.Pos, targetIt->second.Pos);
			}
		}		
	}
}

//****************************************************************
void CPlugin::postRender(CDisplay &display)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (_DialogFlags->DisplayCondition == CDialogFlags::DisplayOff) return;
	// Init icon texture if not already done
	if (!_FaunaFlagIcons)
	{	
		// TODO nico This code is duplicated with world_editor_shard_monitor
		static bool createFailed = false;
		if (createFailed) return;
		HRSRC rsc = FindResource(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_FLAG_ICONS_TGA), "TGA");
		if (rsc == NULL)
		{
			DWORD lastError = GetLastError();
			createFailed = true;
			return;
		}
		NLMISC::CBitmap bm;
		if (!_PluginAccess->buildNLBitmapFromTGARsc(rsc, AfxGetInstanceHandle(), bm))
		{
			createFailed = true;
			return;
		}
		_FaunaFlagIcons = _PluginAccess->createTexture();
		nlassert(_FaunaFlagIcons);
		_FaunaFlagIcons->buildFromNLBitmap(bm);
	}
	display.flush(); // must flush before we assign a next texture
	display.setLayerTexture(0, _FaunaFlagIcons);
	// for each fauna group parent, draw graph of its sons
	for(std::set<const NLLIGO::IPrimitive *>::iterator it = _FaunaGroups.begin(); it != _FaunaGroups.end(); ++it)
	{
		nlassert(*it);
		drawFaunaGraph(display, **it);
	}	
	_FaunaGroups.clear();	
}



extern "C"
{
	void *createPlugin()
	{
		return new CPlugin();
	}
}
