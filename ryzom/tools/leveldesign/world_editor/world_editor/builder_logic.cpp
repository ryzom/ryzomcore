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

#include "world_editor_doc.h"
#include "display.h"

using namespace NLLIGO;
using namespace NLMISC;
using namespace NL3D;
using namespace std;

// ***************************************************************************
// SPrimBuild
// ***************************************************************************

// ---------------------------------------------------------------------------

// ***************************************************************************
// CBuilderLogic
// ***************************************************************************

// ---------------------------------------------------------------------------
CBuilderLogic::CBuilderLogic()
: _LogicCallback(0)
{
	_ItemSelected = NULL;
	_RegionSelected = -1;
	_ToolsLogic = NULL;
	_Display = NULL;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::setDisplay (CDisplay *pDisp)
{
	_Display = pDisp;
}


void CBuilderLogic::notify()
{
	if (_LogicCallback)
	{
		CWorldEditorDoc *doc = getDocument ();
		uint primCount = doc->getNumDatabaseElement ();
		for (uint i=0; i<primCount; i++)
		{
			_LogicCallback->onLogicChanged (i);
		}
	}
}

void CBuilderLogic::setLogicCallback(ILogicCallback *logicCallback)
{
	_LogicCallback = logicCallback;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::setToolsLogic (CToolsLogic *pTool)
{
	_ToolsLogic = pTool;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::uninit()
{
	_Display = NULL;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::updateToolsLogic ()
{
	notify();
	// Regenerate the toolsLogic with the data in the region
	if (_ToolsLogic == NULL)
		return;

	_ToolsLogic->reset();
	uint32 i, j;
	sint32 tempItem = -1;
	for (j = 0; j < getDocument ()->getNumDatabaseElement (); ++j)
	{
		const CPrimRegion &rRegion = getDocument ()->getPrimRegion (j);
		sint32 nReg = j;

		map<string,uint32> mapGroups;
		map<string,uint32>::iterator itGroups;

		// Create Regions
		nReg = _ToolsLogic->createNewRegion (rRegion.Name);

		// Create Groups
		for (i = 0; i < rRegion.VPoints.size(); ++i)
		{
			itGroups = mapGroups.find(rRegion.VPoints[i].LayerName);
			if (itGroups == mapGroups.end())
			{
				uint32 nGroup = _ToolsLogic->createNewGroup (j, rRegion.VPoints[i].LayerName);
				mapGroups.insert (make_pair(rRegion.VPoints[i].LayerName, nGroup));
			}
		}
		for (i = 0; i < rRegion.VPaths.size(); ++i)
		{
			itGroups = mapGroups.find(rRegion.VPaths[i].LayerName);
			if (itGroups == mapGroups.end())
			{
				uint32 nGroup = _ToolsLogic->createNewGroup (j, rRegion.VPaths[i].LayerName);
				mapGroups.insert (make_pair(rRegion.VPaths[i].LayerName, nGroup));
			}
		}
		for (i = 0; i < rRegion.VZones.size(); ++i)
		{
			itGroups = mapGroups.find(rRegion.VZones[i].LayerName);
			if (itGroups == mapGroups.end())
			{
				uint32 nGroup = _ToolsLogic->createNewGroup (j, rRegion.VZones[i].LayerName);
				mapGroups.insert (make_pair(rRegion.VZones[i].LayerName, nGroup));
			}
		}

		for (i = 0; i < rRegion.VPoints.size(); ++i)
		{
			HTREEITEM newItem = (HTREEITEM)tempItem;
			tempItem--;
			itGroups = mapGroups.find(rRegion.VPoints[i].LayerName);
			if (itGroups != mapGroups.end())
			if (rRegion.VPoints[i].Name != "-- Delete Me --")
			{
				newItem = _ToolsLogic->addPoint (nReg, itGroups->second, rRegion.VPoints[i].Name.c_str(), rRegion.VHidePoints[i]);
			}
		}

		for (i = 0; i < rRegion.VPaths.size(); ++i)
		{
			HTREEITEM newItem = (HTREEITEM)tempItem;
			tempItem--;
			itGroups = mapGroups.find(rRegion.VPaths[i].LayerName);
			if (itGroups != mapGroups.end())
			if (rRegion.VPaths[i].Name != "-- Delete Me --")
			{
				newItem = _ToolsLogic->addPath (nReg, itGroups->second, rRegion.VPaths[i].Name.c_str(), rRegion.VHidePaths[i]);
			}
		}

		for (i = 0; i < rRegion.VZones.size(); ++i)
		{
			HTREEITEM newItem = (HTREEITEM)tempItem;
			tempItem--;
			itGroups = mapGroups.find(rRegion.VZones[i].LayerName);
			if (itGroups != mapGroups.end())
			if (rRegion.VZones[i].Name != "-- Delete Me --")
			{
				newItem = _ToolsLogic->addZone (nReg, itGroups->second, rRegion.VZones[i].Name.c_str(), rRegion.VHideZones[i]);
			}
		}
	}


	_VerticesSelected.resize (0);
}

// ---------------------------------------------------------------------------
const string &CBuilderLogic::getZoneRegionName (uint32 nPos)
{
	return _PRegions[nPos]->Name;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::move (const string &regName, float x, float y)
{
	uint32 i, j, k;

	for (i = 0; i < _PRegions.size(); ++i)
	if (_PRegions[i]->Name == regName)
	{
		_MustAskSaves[i] = true;
		for (j = 0; j < _PRegions[i]->VPoints.size(); ++j)
		{
			_PRegions[i]->VPoints[j].Point.x += x;
			_PRegions[i]->VPoints[j].Point.y += y;
		}

		for (j = 0; j < _PRegions[i]->VPaths.size(); ++j)
		{
			for (k = 0; k < _PRegions[i]->VPaths[j].VPoints.size(); ++k)
			{
				_PRegions[i]->VPaths[j].VPoints[k].x += x;
				_PRegions[i]->VPaths[j].VPoints[k].y += y;
			}
		}

		for (j = 0; j < _PRegions[i]->VZones.size(); ++j)
		{
			for (k = 0; k < _PRegions[i]->VZones[j].VPoints.size(); ++k)
			{
				_PRegions[i]->VZones[j].VPoints[k].x += x;
				_PRegions[i]->VZones[j].VPoints[k].y += y;
			}
		}
		notify();
		return;
	}
}

// ---------------------------------------------------------------------------

void CBuilderLogic::insertPoint (uint32 pos, HTREEITEM item, const char *Name, const char *LayerName)
{

	CPrimPointEditor pp;
	pp.LayerName = LayerName;
	pp.Name = Name;
	pp.Point = CVector(0.0f, 0.0f, 0.0f);
	_PRegions[pos]->VPoints.push_back (pp);
	_PRegions[pos]->VHidePoints.push_back (false);
	SPrimBuild pB;
	pB.Type = 0; // Point
	pB.Pos = _PRegions[pos]->VPoints.size()-1;
	pB.PRegion = _PRegions[pos];
	_Primitives.insert (map<HTREEITEM, SPrimBuild>::value_type(item, pB));
	_MustAskSaves[pos] = true;

	notify();
}

// ---------------------------------------------------------------------------
void CBuilderLogic::insertPath (uint32 pos, HTREEITEM item, const char *Name, const char *LayerName)
{

	CPrimPathEditor pp;
	pp.LayerName = LayerName;
	pp.Name = Name;
	_PRegions[pos]->VPaths.push_back (pp);
	_PRegions[pos]->VHidePaths.push_back (false);
	SPrimBuild pB;
	pB.Type = 1; // Path
	pB.Pos = _PRegions[pos]->VPaths.size()-1;
	pB.PRegion = _PRegions[pos];
	_Primitives.insert (map<HTREEITEM, SPrimBuild>::value_type(item, pB));
	_MustAskSaves[pos] = true;

	notify();
}

// ---------------------------------------------------------------------------
void CBuilderLogic::insertZone (uint32 pos, HTREEITEM item, const char *Name, const char *LayerName)
{
	CPrimZoneEditor pz;
	pz.LayerName = LayerName;
	pz.Name = Name;
	_PRegions[pos]->VZones.push_back (pz);
	_PRegions[pos]->VHideZones.push_back (false);
	SPrimBuild pB;
	pB.Type = 2; // Zone
	pB.Pos = _PRegions[pos]->VZones.size()-1;
	pB.PRegion = _PRegions[pos];
	_Primitives.insert (map<HTREEITEM, SPrimBuild>::value_type(item, pB));
	_MustAskSaves[pos] = true;

	notify();
}

// ---------------------------------------------------------------------------
void CBuilderLogic::del (HTREEITEM item)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);

	if (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;
		CPrimRegion &PRegion = *rPB.PRegion;
		// Delete the entry in the document


		// Delete in the region
		switch (rPB.Type)
		{
			case 0:
			{
				PRegion.VPoints.erase (PRegion.VPoints.begin()+rPB.Pos);
				PRegion.VHidePoints.erase (PRegion.VHidePoints.begin()+rPB.Pos);
			}
			break;
			case 1:
			{
				PRegion.VPaths.erase (PRegion.VPaths.begin()+rPB.Pos);
				PRegion.VHidePaths.erase (PRegion.VHidePaths.begin()+rPB.Pos);
			}
			break;
			case 2:
				PRegion.VZones.erase (PRegion.VZones.begin()+rPB.Pos);
				PRegion.VHideZones.erase (PRegion.VHideZones.begin()+rPB.Pos);
			break;
		}
		// Update position
		map<HTREEITEM, SPrimBuild>::iterator it2 = _Primitives.begin ();
		while (it2 != _Primitives.end())
		{
			SPrimBuild &rPB2 = it2->second;

			if ((rPB2.PRegion == rPB.PRegion)&&(rPB2.Type == rPB.Type))
				if (rPB2.Pos > rPB.Pos)
					rPB2.Pos -= 1;
				
			++it2;
		}
		// Delete the entry in the map
		_Primitives.erase (it);

		for (uint32 z = 0; z < _PRegions.size(); ++z)
		if (_PRegions[z] == &PRegion)
		{
			_MustAskSaves[z] = true;
			break;
		}
	}
	notify();
}

// ---------------------------------------------------------------------------
void CBuilderLogic::primHide (HTREEITEM item)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;
		rPB.hidden = true;
		if (rPB.Type == 0) // Points
		{
			rPB.PRegion->VHidePoints[rPB.Pos] = true;
		}
		else if (rPB.Type == 1) // Paths
		{
			rPB.PRegion->VHidePaths[rPB.Pos] = true;
		}
		else if (rPB.Type == 2) /// Zones
		{
			rPB.PRegion->VHideZones[rPB.Pos] = true;
		}
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::primUnhide (HTREEITEM item)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;
		rPB.hidden = false;
		if (rPB.Type == 0) // Points
		{
			rPB.PRegion->VHidePoints[rPB.Pos] = false;
		}
		else if (rPB.Type == 1) // Paths
		{
			rPB.PRegion->VHidePaths[rPB.Pos] = false;
		}
		else if (rPB.Type == 2) /// Zones
		{
			rPB.PRegion->VHideZones[rPB.Pos] = false;
		}
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::hideAll (uint32 nPos, sint32 nID, bool bHide)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.begin ();
	while (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;

		if (rPB.PRegion == _PRegions[nPos])
		{
			if (rPB.Type == nID)
			{
				rPB.hidden = bHide;
				if (rPB.Type == 0) // Points
				{
					rPB.PRegion->VHidePoints[rPB.Pos] = bHide;
				}
				else if (rPB.Type == 1) // Paths
				{
					rPB.PRegion->VHidePaths[rPB.Pos] = bHide;
				}
				else if (rPB.Type == 2) /// Zones
				{
					rPB.PRegion->VHideZones[rPB.Pos] = bHide;
				}		
			}
		}
		
		++it;
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::regionSetName (uint32 nPos, HTREEITEM item, const string &sRegionName)
{
	CPrimRegion &rPR = *_PRegions[nPos];
	rPR.Name = sRegionName;
	notify();
}

// ---------------------------------------------------------------------------
void CBuilderLogic::regionCreateGroup (uint32 nPos, HTREEITEM newItem, const string &sGroupName)
{
	insertPoint (nPos, (HTREEITEM)-1, "-- Delete Me --", sGroupName.c_str());
}

// ---------------------------------------------------------------------------
void CBuilderLogic::regionDeleteGroup (uint32 nPos, const std::string &sGroupName)
{
	sint32 i, j;
	for (i = 0; i < (sint32)_PRegions[nPos]->VPoints.size(); ++i)
	{
		if (_PRegions[nPos]->VPoints[i].LayerName == sGroupName)
		{
			for (j = i+1; j < (sint32)_PRegions[nPos]->VPoints.size(); ++j)
				_PRegions[nPos]->VPoints[j-1] = _PRegions[nPos]->VPoints[j];
			_PRegions[nPos]->VPoints.resize (_PRegions[nPos]->VPoints.size()-1);
			i = -1;
		}
	}
	notify();
}

// ---------------------------------------------------------------------------
void CBuilderLogic::regionHideAll (uint32 nPos, bool bHide)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.begin ();
	while (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;

		if (rPB.PRegion == _PRegions[nPos])
			rPB.hidden = bHide;
		
		++it;
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::regionHideType (uint32 nPos, const string &Type, bool bHide)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.begin ();
	while (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;

		if (rPB.PRegion == _PRegions[nPos])
		{
			string Name, Tmp;
			switch (rPB.Type)
			{
				case 0: Name = _PRegions[nPos]->VPoints[rPB.Pos].Name; break;
				case 1: Name = _PRegions[nPos]->VPaths[rPB.Pos].Name; break;
				case 2: Name = _PRegions[nPos]->VZones[rPB.Pos].Name; break;
			}

			uint32 i = 0;
			while (Name[i] != '-') ++i;
			i++;
			while ((Name[i] != '-') && (i < Name.size()))
			{
				Tmp += Name[i];
				++i;
			}
			if (Tmp == Type)
				rPB.hidden = bHide;
		}
		
		++it;
	}
}

// ---------------------------------------------------------------------------
int CBuilderLogic::getMaxPostfix (const char *prefix)
{
	int nTmp;
	int nZeMax = -1;
	int nPFLen = strlen(prefix);

	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.begin ();
	while (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;
		nTmp = 0;
		switch (rPB.Type)
		{
			case 0: // Point
				if (strncmp(prefix, rPB.PRegion->VPoints[rPB.Pos].Name.c_str(), nPFLen) == 0)
				{
					CPrimPointEditor &rP = rPB.PRegion->VPoints[rPB.Pos];
					if ((rP.Name[nPFLen+0] >= '0') && (rP.Name[nPFLen+0] <= '9') &&
						(rP.Name[nPFLen+1] >= '0') && (rP.Name[nPFLen+1] <= '9') &&
						(rP.Name[nPFLen+2] >= '0') && (rP.Name[nPFLen+2] <= '9'))
					{
						nTmp  = (rP.Name[nPFLen+0] - '0')*100;
						nTmp += (rP.Name[nPFLen+1] - '0')*10;
						nTmp += (rP.Name[nPFLen+2] - '0')*1;
					}
				}
			break;
			case 1: // Path
				if (strncmp(prefix, rPB.PRegion->VPaths[rPB.Pos].Name.c_str(), nPFLen) == 0)
				{
					CPrimPathEditor &rP = rPB.PRegion->VPaths[rPB.Pos];
					if ((rP.Name[nPFLen+0] >= '0') && (rP.Name[nPFLen+0] <= '9') &&
						(rP.Name[nPFLen+1] >= '0') && (rP.Name[nPFLen+1] <= '9') &&
						(rP.Name[nPFLen+2] >= '0') && (rP.Name[nPFLen+2] <= '9'))
					{
						nTmp  = (rP.Name[nPFLen+0] - '0')*100;
						nTmp += (rP.Name[nPFLen+1] - '0')*10;
						nTmp += (rP.Name[nPFLen+2] - '0')*1;
					}
				}
			break;
			case 2: // Zone
				if (strncmp(prefix, rPB.PRegion->VZones[rPB.Pos].Name.c_str(), nPFLen) == 0)
				{
					CPrimZoneEditor &rP = rPB.PRegion->VZones[rPB.Pos];
					if ((rP.Name[nPFLen+0] >= '0') && (rP.Name[nPFLen+0] <= '9') &&
						(rP.Name[nPFLen+1] >= '0') && (rP.Name[nPFLen+1] <= '9') &&
						(rP.Name[nPFLen+2] >= '0') && (rP.Name[nPFLen+2] <= '9'))
					{
						nTmp  = (rP.Name[nPFLen+0] - '0')*100;
						nTmp += (rP.Name[nPFLen+1] - '0')*10;
						nTmp += (rP.Name[nPFLen+2] - '0')*1;
					}
				}
			break;
		}
		
		if (nZeMax < nTmp)
			nZeMax = nTmp;

		++it;
	}

	return nZeMax;
}

// ---------------------------------------------------------------------------
bool CBuilderLogic::isAlreadyExisting (const char *sPrimitiveName)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.begin ();
	while (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;

		switch (rPB.Type)
		{
			case 0: // Point
				if (strcmp(sPrimitiveName, rPB.PRegion->VPoints[rPB.Pos].Name.c_str()) == 0)
					return true;
			break;
			case 1: // Path
				if (strcmp(sPrimitiveName, rPB.PRegion->VPaths[rPB.Pos].Name.c_str()) == 0)
					return true;
			break;
			case 2: // Zone
				if (strcmp(sPrimitiveName, rPB.PRegion->VZones[rPB.Pos].Name.c_str()) == 0)
					return true;
			break;
			default:
			break;
		}
		++it;
	}
	return false;
}

// ---------------------------------------------------------------------------
const char* CBuilderLogic::getName (HTREEITEM item)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it == _Primitives.end())
		return NULL;
	else
	{
		SPrimBuild &rPB = it->second;
		CPrimRegion &PRegion = *rPB.PRegion;

		switch (rPB.Type)
		{
			case 0:
				return PRegion.VPoints[rPB.Pos].Name.c_str();
			break;
			case 1:
				return PRegion.VPaths[rPB.Pos].Name.c_str();
			break;
			case 2:
				return PRegion.VZones[rPB.Pos].Name.c_str();
			break;
		}
		return NULL;
	}
}

// ---------------------------------------------------------------------------
const char* CBuilderLogic::getLayerName (HTREEITEM item)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it == _Primitives.end())
		return NULL;
	else
	{
		SPrimBuild &rPB = it->second;
		CPrimRegion &PRegion = *rPB.PRegion;

		switch (rPB.Type)
		{
			case 0:
				return PRegion.VPoints[rPB.Pos].LayerName.c_str();
			break;
			case 1:
				return PRegion.VPaths[rPB.Pos].LayerName.c_str();
			break;
			case 2:
				return PRegion.VZones[rPB.Pos].LayerName.c_str();
			break;
		}
		return NULL;
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::getAllPrimZoneNames (vector<string> &allNames)
{
	allNames.clear ();
	for (uint32 i = 0; i < _PRegions.size(); ++i)
		for (uint32 j = 0; j < _PRegions[i]->VZones.size(); ++j)
			allNames.push_back (_PRegions[i]->VZones[j].Name);
}


// ---------------------------------------------------------------------------
bool CBuilderLogic::isHidden (HTREEITEM item)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it == _Primitives.end())
	{
		return true;
	}
	else
	{
		SPrimBuild &rPB = it->second;
		return rPB.hidden;
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::setName (HTREEITEM item, const char* pStr)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it == _Primitives.end())
		return;
	else
	{
		SPrimBuild &rPB = it->second;
		CPrimRegion &PRegion = *rPB.PRegion;

		switch (rPB.Type)
		{
			case 0:
				PRegion.VPoints[rPB.Pos].Name = pStr;
			break;
			case 1:
				PRegion.VPaths[rPB.Pos].Name = pStr;
			break;
			case 2:
				PRegion.VZones[rPB.Pos].Name = pStr;
			break;
		}

		for (uint32 z = 0; z < _PRegions.size(); ++z)
		if (_PRegions[z] == &PRegion)
		{
			_MustAskSaves[z] = true;
			break;
		}

	}
	notify();
}

// ---------------------------------------------------------------------------
void CBuilderLogic::setLayerName (HTREEITEM item, const char* pStr)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it == _Primitives.end())
		return;
	else
	{
		SPrimBuild &rPB = it->second;
		CPrimRegion &PRegion = *rPB.PRegion;

		switch (rPB.Type)
		{
			case 0:
				PRegion.VPoints[rPB.Pos].LayerName = pStr;
			break;
			case 1:
				PRegion.VPaths[rPB.Pos].LayerName = pStr;
			break;
			case 2:
				PRegion.VZones[rPB.Pos].LayerName = pStr;
			break;
		}

		for (uint32 z = 0; z < _PRegions.size(); ++z)
		if (_PRegions[z] == &PRegion)
		{
			_MustAskSaves[z] = true;
			break;
		}
	}
	notify();
}

// ---------------------------------------------------------------------------
HTREEITEM CBuilderLogic::getSelPB ()
{
	return _ItemSelected;
}

// ---------------------------------------------------------------------------
std::string CBuilderLogic::getSelPBName ()
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (_ItemSelected);
	if (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;
		CPrimRegion &PRegion = *rPB.PRegion;

		switch (rPB.Type)
		{
			case 0: return PRegion.VPoints[rPB.Pos].Name;
			case 1: return PRegion.VPaths[rPB.Pos].Name;
			case 2: return PRegion.VZones[rPB.Pos].Name;
		}
	}
	
	return "";
}

// ---------------------------------------------------------------------------
void CBuilderLogic::setSelPB (HTREEITEM item)
{
	_VerticesSelected.clear ();
	_ItemSelected = NULL;
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it != _Primitives.end())
	{
		_ItemSelected = item;
		_ItemSelectedName = getName(_ItemSelected);
		_ToolsLogic->GetTreeCtrl()->Select(item, TVGN_CARET);
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::createVertexOnSelPB (CVector &v, sint32 atPos)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (_ItemSelected);
	if (it == _Primitives.end())
		return;
	SPrimBuild &rPB = it->second;
	CPrimRegion &PRegion = *rPB.PRegion;

	switch (rPB.Type)
	{
		case 0:
			if (!rPB.Created)
			{
				PRegion.VPoints[rPB.Pos].Point = v;
				rPB.Created = true;
			}
		break;
		case 1:
			if (!rPB.Created)
			{
				rPB.Created = true;
			}
			if (atPos != -1)
			{
				PRegion.VPaths[rPB.Pos].VPoints.insert (PRegion.VPaths[rPB.Pos].VPoints.begin()+atPos, v);
			}
			else
			{
				PRegion.VPaths[rPB.Pos].VPoints.push_back(v);
			}
		break;
		case 2:
			if (!rPB.Created)
			{
				rPB.Created = true;
			}
			if (atPos != -1)
			{
				PRegion.VZones[rPB.Pos].VPoints.insert (PRegion.VZones[rPB.Pos].VPoints.begin()+atPos, v);
			}
			else
			{
				PRegion.VZones[rPB.Pos].VPoints.push_back(v);
			}
		break;
	}
	for (uint32 z = 0; z < _PRegions.size(); ++z)
	if (_PRegions[z] == &PRegion)
	{
		_MustAskSaves[z] = true;
		break;
	}

	notify();
}

// ---------------------------------------------------------------------------
bool CBuilderLogic::selectVerticesOnSelPB (CVector &selMin, CVector &selMax)
{
	_VerticesSelected.clear ();
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (_ItemSelected);
	if (it == _Primitives.end())
		return false;

	SPrimBuild &rPB = it->second;
	CPrimRegion &PRegion = *rPB.PRegion;

	if (!rPB.Created)
		return false;

	CPrimVector *pV = NULL;
	uint32 nNbV = 0;
	switch (rPB.Type)
	{
		case 0:
			nNbV = 1;
			pV = &(PRegion.VPoints[rPB.Pos].Point);
		break;
		case 1:
			nNbV = PRegion.VPaths[rPB.Pos].VPoints.size();
			pV = &(PRegion.VPaths[rPB.Pos].VPoints[0]);
		break;
		case 2:
			nNbV = PRegion.VZones[rPB.Pos].VPoints.size();
			pV = &(PRegion.VZones[rPB.Pos].VPoints[0]);
		break;
	}
	float selMinX = min(selMin.x, selMax.x);
	float selMinY = min(selMin.y, selMax.y);
	float selMaxX = max(selMin.x, selMax.x);
	float selMaxY = max(selMin.y, selMax.y);

	for (uint32 i = 0; i < nNbV; ++i)
	{
		if ((pV[i].x > selMinX) && (pV[i].x < selMaxX) &&
			(pV[i].y > selMinY) && (pV[i].y < selMaxY))
		{
			_VerticesSelected.push_back (i);
		}
	}
	if (_VerticesSelected.size() == 0)
		return false;
	return true;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::createIsoVertexOnSelVertexOnSelPB ()
{
	if ((_VerticesSelected.size() <= 1) || (_ItemSelected == NULL))
		return;
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (_ItemSelected);
	if (it == _Primitives.end())
		return;
	SPrimBuild &rPB = it->second;
	CPrimRegion &PRegion = *rPB.PRegion;
	CVector v(0.0f, 0.0f, 0.0f);
	
	sint minvs = 30000;
	for (uint32 i = 0; i < _VerticesSelected.size(); ++i)
	{
		switch (rPB.Type)
		{
			case 0:
				v += PRegion.VPoints[rPB.Pos].Point;
			break;
			case 1:
				v += PRegion.VPaths[rPB.Pos].VPoints[_VerticesSelected[i]];
			break;
			case 2:
				v += PRegion.VZones[rPB.Pos].VPoints[_VerticesSelected[i]];
			break;
		}
		if (_VerticesSelected[i] < minvs)
			minvs = _VerticesSelected[i];
	}
	v /= (float)_VerticesSelected.size();

	if (rPB.Type == 2)
	{
		CPrimZoneEditor &pz = PRegion.VZones[rPB.Pos];
		if (((_VerticesSelected[0] == 0) && (_VerticesSelected[1] == ((sint32)pz.VPoints.size()-1))) ||
			((_VerticesSelected[0] == ((sint32)pz.VPoints.size()-1)) && (_VerticesSelected[1] == 0)))
		{
			createVertexOnSelPB (v, 0);
		}
		else
		{
			createVertexOnSelPB (v, minvs+1);
		}
	}
	else
	{
		createVertexOnSelPB (v, minvs+1);
	}
}

// ---------------------------------------------------------------------------
bool CBuilderLogic::isSelection()
{
	return (_VerticesSelected.size() > 0);
}

// ---------------------------------------------------------------------------
void CBuilderLogic::setSelVerticesOnSelPB (NLMISC::CVector &v)
{
	if ((_VerticesSelected.size() == 0) || (_ItemSelected == NULL))
		return;
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (_ItemSelected);
	if (it == _Primitives.end())
		return;
	SPrimBuild &rPB = it->second;
	CPrimRegion &PRegion = *rPB.PRegion;

	for (uint32 i = 0; i < _VerticesSelected.size(); ++i)
	{
		switch (rPB.Type)
		{
			case 0:
				PRegion.VPoints[rPB.Pos].Point += v;
			break;
			case 1:
				PRegion.VPaths[rPB.Pos].VPoints[_VerticesSelected[i]] += v;
			break;
			case 2:
				PRegion.VZones[rPB.Pos].VPoints[_VerticesSelected[i]] += v;
			break;
		}
	}

	for (uint32 z = 0; z < _PRegions.size(); ++z)
	if (_PRegions[z] == &PRegion)
	{
		_MustAskSaves[z] = true;
		break;
	}
	notify();
}

// ---------------------------------------------------------------------------
void CBuilderLogic::delSelVerticesOnSelPB ()
{
	if ((_VerticesSelected.size() == 0) || (_ItemSelected == NULL))
		return;
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (_ItemSelected);
	if (it == _Primitives.end())
		return;
	SPrimBuild &rPB = it->second;
	CPrimRegion &PRegion = *rPB.PRegion;

	uint32 i;
	for (uint32 k = 0; k < _VerticesSelected.size(); ++k)
	{
		int VertexSelected = _VerticesSelected[k];
		switch (rPB.Type)
		{
			case 0:
				rPB.Created = false;
			break;
			case 1:
				if (PRegion.VPaths[rPB.Pos].VPoints.size() == 1)
					rPB.Created = false;

				for (i = VertexSelected+1; i < PRegion.VPaths[rPB.Pos].VPoints.size(); ++i)
					PRegion.VPaths[rPB.Pos].VPoints[i-1] = PRegion.VPaths[rPB.Pos].VPoints[i];
				PRegion.VPaths[rPB.Pos].VPoints.resize (PRegion.VPaths[rPB.Pos].VPoints.size()-1);
			break;
			case 2:
				if (PRegion.VZones[rPB.Pos].VPoints.size() == 1)
					rPB.Created = false;

				for (i = VertexSelected+1; i < PRegion.VZones[rPB.Pos].VPoints.size(); ++i)
					PRegion.VZones[rPB.Pos].VPoints[i-1] = PRegion.VZones[rPB.Pos].VPoints[i];
				PRegion.VZones[rPB.Pos].VPoints.resize (PRegion.VZones[rPB.Pos].VPoints.size()-1);
			break;
		}
	}

	for (uint32 z = 0; z < _PRegions.size(); ++z)
	if (_PRegions[z] == &PRegion)
	{
		_MustAskSaves[z] = true;
		break;
	}

	notify();
}

// ---------------------------------------------------------------------------
void CBuilderLogic::stackSelPB ()
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (_ItemSelected);
	if (it == _Primitives.end())
		return;
	SPrimBuild &rPB = it->second;
	CPrimRegion &PRegion = *rPB.PRegion;
}

// ---------------------------------------------------------------------------
string CBuilderLogic::getZonesNameAt (CVector &v)
{
	string ret;
	for (uint32 i = 0; i < _PRegions.size(); ++i)
	{
		for (uint32 j = 0; j < _PRegions[i]->VZones.size(); ++j)
		{
			CPrimZoneEditor *pz = &_PRegions[i]->VZones[j];

			// Find if the patat is visible
			bool bHidden = true;
			map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.begin ();
			while (it != _Primitives.end())
			{
				if ((it->second.Type == 2) && ((sint32)j == it->second.Pos))
				{
					if (it->second.hidden)
						bHidden = true;
					else
						bHidden = false;
					break;
				}
				++it;
			}

			if ((!bHidden) && (pz->contains(v)))
			{
				if (ret.size() == 0)
				{
					ret += pz->Name;
				}
				else
				{
					ret += ", " + pz->Name;
				}
			}
		}
	}
	return ret;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::selectZoneAt (NLMISC::CVector &v)
{
	vector<string> zonelist;
	uint32 i, j;

	for (i = 0; i < _PRegions.size(); ++i)
	{
		for (j = 0; j < _PRegions[i]->VZones.size(); ++j)
		{
			CPrimZoneEditor *pz = &_PRegions[i]->VZones[j];
			// Find if the patat is visible
			bool bHidden = true;
			map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.begin ();
			while (it != _Primitives.end())
			{
				if ((it->second.Type == 2) && ((sint32)j == it->second.Pos))
				{
					if (it->second.hidden)
						bHidden = true;
					else
						bHidden = false;
					break;
				}
				++it;
			}

			if ((!bHidden) && (pz->contains(v)))
			{
				zonelist.push_back (pz->Name);
			}
		}
	}

	if (zonelist.size() == 0)
		return;

	string sSelPB = getSelPBName();
	for (i = 0; i < zonelist.size(); ++i)
	{
		if (zonelist[i] == sSelPB)
			break;
	}

	if (i == zonelist.size())
		i = 0;
	else
		i = (i+1)%zonelist.size();
	sSelPB = zonelist[i];

	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.begin ();
	while (it != _Primitives.end())
	{
		if (sSelPB == getName(it->first))
		{
			setSelPB (it->first);
			break;
		}
		++it;
	}
}

// ---------------------------------------------------------------------------
CRGBA CBuilderLogic::findColor(const string &Name) // region-type-001
{
	// Get the type from the name
	string sType;
	uint32 i = 0;
	while (i < Name.size())
	{
		if (Name[i]=='-') break;
		++i;
	}
	if (i == Name.size())
		return CRGBA (255, 255, 255, 255);
	++i;
	while (i < Name.size())
	{
		if (Name[i]=='-') break;
		sType += Name[i];
		++i;
	}
	if (i == Name.size())
		return CRGBA (255, 255, 255, 255);
	
	vector<SType> &rTypes = _Display->_MainFrame->_Environnement.Types;
	for (i = 0; i < rTypes.size(); ++i)
		if (sType == rTypes[i].Name)
			return rTypes[i].Color;

	return CRGBA (255, 255, 255, 255);
}

// ---------------------------------------------------------------------------
void CBuilderLogic::render (CVector &viewMin, CVector &viewMax)
{
	if (DontUse3D)
		return;
	// Accelerate rendering with vertex buffer
	CVertexBuffer VB;
	CPrimitiveBlock PB;
	CVertexBuffer VBL;
	CPrimitiveBlock PBL;
	CMaterial Mat;

	CRGBA colSel = CRGBA(255, 0, 0, 192);

	Mat.initUnlit ();
	Mat.setSrcBlend(CMaterial::srcalpha);
	Mat.setDstBlend(CMaterial::invsrcalpha);
	Mat.setBlend (true);
	Mat.setColor (CRGBA(255, 255, 255, 192));
	VB.setVertexFormat (CVertexBuffer::PositionFlag|CVertexBuffer::PrimaryColorFlag);
	VBL.setVertexFormat (CVertexBuffer::PositionFlag|CVertexBuffer::PrimaryColorFlag);
	
	// Parse the map
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.begin();
	while (it != _Primitives.end())
	{
		HTREEITEM curItem = it->first;
		SPrimBuild &curPB = it->second;
		CPrimRegion &PRegion = *curPB.PRegion;

		CRGBA col;
		uint32 i;
		
		// If not created do not display or hidden
		if ((!curPB.Created)||(curPB.hidden))
		{
			++it;
			continue;
		}

		CPrimVector *pVec = NULL;
		uint32 nNbVec = 0;

		// Clip

		if (curPB.Type == 0) // Point
		{
			col = findColor (PRegion.VPoints[curPB.Pos].Name);
			pVec = &PRegion.VPoints[curPB.Pos].Point;
			nNbVec = 1;
		}

		if (curPB.Type == 1) // Path
		{
			col = findColor (PRegion.VPaths[curPB.Pos].Name);
			nNbVec = PRegion.VPaths[curPB.Pos].VPoints.size();
			if (nNbVec > 0)
				pVec = &PRegion.VPaths[curPB.Pos].VPoints[0];
		}

		if (curPB.Type == 2) // Zone
		{
			col = findColor (PRegion.VZones[curPB.Pos].Name);
			nNbVec = PRegion.VZones[curPB.Pos].VPoints.size();
			if (nNbVec > 0)
				pVec = &PRegion.VZones[curPB.Pos].VPoints[0];
		}

		col.A = 192;

		if (nNbVec == 0)
		{
			++it;
			continue;
		}

		if (clip(pVec, nNbVec, viewMin, viewMax))
		{
			++it;
			continue;
		}	
			

		// Draw all interiors
		
		if (curPB.Type == 2) // For Zones
		{
			vector<sint32> vRef;
			uint32 nStart, VBStart = VB.getNumVertices(), PBStart = PB.getNumTri();
			vRef.resize(nNbVec);
			for(i = 0; i < vRef.size(); ++i)
				vRef[i] = i;

			nStart = 0;
			while (vRef.size() > 2)
			{
				// Is triangle (nStart, nStart+1, nStart+2) back face ?
				sint32 nP1 = vRef[nStart];
				sint32 nP2 = vRef[(nStart+1)%vRef.size()];
				sint32 nP3 = vRef[(nStart+2)%vRef.size()];
				CVector pos1 = pVec[nP1];
				CVector pos2 = pVec[nP2];
				CVector pos3 = pVec[nP3];
				if (((pos2.x-pos1.x) * (pos3.y-pos1.y) - (pos2.y-pos1.y) * (pos3.x-pos1.x)) < 0.0f)
				{
					// Yes -> next triangle
					nStart++;
					//nlassert(nStart != vRef.size());
					if (nStart == vRef.size())
					{
						VB.setNumVertices (VBStart);
						PB.setNumTri(PBStart);
						break;
					}
					continue;
				}
				// Is triangle (nStart, nStart+1, nStart+2) contains the other point ?
				bool bInside = false;
				for (i = 0; i < vRef.size(); ++i)
				{
					if ((vRef[i] != nP1) && (vRef[i] != nP2) && (vRef[i] != nP3))
					{
						if (isInTriangleOrEdge(	pVec[vRef[i]].x, pVec[vRef[i]].y, 
												pos1.x, pos1.y,
												pos2.x, pos2.y,
												pos3.x, pos3.y ))
						{
							bInside = true;
							break;
						}
					}
				}
				if (bInside)
				{
					// Yes -> next triangle
					nStart++;
					//nlassert(nStart != vRef.size());
					if (nStart == vRef.size())
					{
						VB.setNumVertices (VBStart);
						PB.setNumTri(PBStart);
						break;
					}
					continue;
				}

				// Draw the triangle
				convertToScreen (&pos1, 1, viewMin, viewMax);
				convertToScreen (&pos2, 1, viewMin, viewMax);
				convertToScreen (&pos3, 1, viewMin, viewMax);
				if (curItem == _ItemSelected)
					renderDrawTriangle(pos1, pos2, pos3, colSel, &VB, &PB);
				else
					renderDrawTriangle(pos1, pos2, pos3, col, &VB, &PB);
				
				// Erase the point in the middle
				for (i = 1+((nStart+1)%vRef.size()); i < vRef.size(); ++i)
					vRef[i-1] = vRef[i];
				vRef.resize (vRef.size()-1);
				nStart = 0;
			}
		}

		// Draw all lines
		
		if ((curPB.Type == 1) || (curPB.Type == 2)) // For Pathes and Zones
		{
			uint32 nNbLineToDraw = (curPB.Type == 2)?(nNbVec):(nNbVec-1);
			if ((nNbLineToDraw == 1)&&(curPB.Type == 2))
				nNbLineToDraw = 0;
			for (i = 0; i < nNbLineToDraw; ++i)
			{
				CVector pos = pVec[i];
				CVector pos2 = pVec[(i+1)%(nNbVec)];
	
				convertToScreen (&pos, 1, viewMin, viewMax);
				convertToScreen (&pos2, 1, viewMin, viewMax);
				if (curItem == _ItemSelected)
					renderDrawLine (pos, pos2, colSel, &VBL, &PBL);
				else
					renderDrawLine (pos, pos2, col, &VBL, &PBL);
			}
		}

		// Draw all points
		bool bDrawPoints = true;

		if (curPB.Type == 2) // Zone
		if (curItem != _ItemSelected)
			bDrawPoints = false; // Do not draw the points on zone other than selected one
		
		if (bDrawPoints)
		for (i = 0; i < nNbVec; ++i)		// For Points, Pathes and Zones
		{
			CVector pos = pVec[i];
			convertToScreen (&pos, 1, viewMin, viewMax);
			if (curItem == _ItemSelected)
			{
				bool bFound = false;
				for (uint32 k = 0; k < _VerticesSelected.size(); ++k)
				if (_VerticesSelected[k] == (sint32)i)
				{
					bFound = true;
					break;
				}
				if (bFound)
					renderDrawPoint (pos, colSel, &VBL, &PBL);
				else
					renderDrawPoint (pos, col, &VBL, &PBL);
			}
			else
			{
				renderDrawPoint (pos, col, &VBL, &PBL);
			}
		}

		++it;
	}

	// Flush the Vertex Buffer
	
	CMatrix mtx;
	mtx.identity();
	CNELU::Driver->setupViewport (CViewport());
	CNELU::Driver->setupViewMatrix (mtx);
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);
	CNELU::Driver->activeVertexBuffer(VB);
	CNELU::Driver->render(PB, Mat);
	CNELU::Driver->activeVertexBuffer(VBL);
	CNELU::Driver->render(PBL, Mat);
}

// ---------------------------------------------------------------------------
void CBuilderLogic::renderDrawPoint (CVector &pos, CRGBA &col, CVertexBuffer *pVB, CPrimitiveBlock *pPB)
{
	if (DontUse3D)
		return;

	if (pVB == NULL)
	{
		CDRU::drawLine (pos.x-0.01f, pos.y, pos.x+0.01f, pos.y, *CNELU::Driver, col);
		CDRU::drawLine (pos.x, pos.y-0.01f, pos.x, pos.y+0.01f, *CNELU::Driver, col);
	}
	else
	{
		sint32 nVBPos = pVB->getNumVertices();
		pVB->setNumVertices (nVBPos+4);
		pVB->setVertexCoord (nVBPos+0, pos.x-0.01f, pos.z, pos.y);
		pVB->setVertexCoord (nVBPos+1, pos.x+0.01f, pos.z, pos.y);
		pVB->setVertexCoord (nVBPos+2, pos.x, pos.z, pos.y-0.01f);
		pVB->setVertexCoord (nVBPos+3, pos.x, pos.z, pos.y+0.01f);
		pVB->setColor (nVBPos+0, col);
		pVB->setColor (nVBPos+1, col);
		pVB->setColor (nVBPos+2, col);
		pVB->setColor (nVBPos+3, col);
		sint32 nPBPos = pPB->getNumLine();
		pPB->setNumLine (nPBPos+2);
		pPB->setLine (nPBPos+0, nVBPos+0, nVBPos+1);
		pPB->setLine (nPBPos+1, nVBPos+2, nVBPos+3);
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::renderDrawLine (CVector &pos, CVector &pos2, CRGBA &col, CVertexBuffer *pVB, CPrimitiveBlock *pPB)
{
	sint32 nVBPos = pVB->getNumVertices();
	pVB->setNumVertices (nVBPos+2);
	pVB->setVertexCoord (nVBPos+0, pos.x, pos.z, pos.y);
	pVB->setVertexCoord (nVBPos+1, pos2.x, pos2.z, pos2.y);
	pVB->setColor (nVBPos+0, col);
	pVB->setColor (nVBPos+1, col);
	sint32 nPBPos = pPB->getNumLine();
	pPB->setNumLine (nPBPos+1);
	pPB->setLine (nPBPos+0, nVBPos+0, nVBPos+1);
}

// ---------------------------------------------------------------------------
void CBuilderLogic::renderDrawTriangle (CVector &pos, CVector &pos2, CVector &pos3, CRGBA &col, CVertexBuffer *pVB, CPrimitiveBlock *pPB)
{
	sint32 nVBPos = pVB->getNumVertices();
	pVB->setNumVertices (nVBPos+3);
	pVB->setVertexCoord (nVBPos+0, pos.x, pos.z, pos.y);
	pVB->setVertexCoord (nVBPos+1, pos2.x, pos2.z, pos2.y);
	pVB->setVertexCoord (nVBPos+2, pos3.x, pos3.z, pos3.y);
	pVB->setColor (nVBPos+0, col);
	pVB->setColor (nVBPos+1, col);
	pVB->setColor (nVBPos+2, col);
	sint32 nPBPos = pPB->getNumTri();
	pPB->setNumTri (nPBPos+1);
	pPB->setTri (nPBPos+0, nVBPos+0, nVBPos+1, nVBPos+2);
}

// ---------------------------------------------------------------------------
bool CBuilderLogic::clip (CPrimVector *pVec, uint32 nNbVec, CVector &viewMin, CVector &viewMax)
{
	uint32 i;
	for (i = 0; i < nNbVec; ++i)
		if (pVec[i].x > viewMin.x)
			break;
	if (i == nNbVec)
		return true; // Entirely clipped

	for (i = 0; i < nNbVec; ++i)
		if (pVec[i].x < viewMax.x)
			break;
	if (i == nNbVec)
		return true;

	for (i = 0; i < nNbVec; ++i)
		if (pVec[i].y > viewMin.y)
			break;
	if (i == nNbVec)
		return true;

	for (i = 0; i < nNbVec; ++i)
		if (pVec[i].y < viewMax.y)
			break;
	if (i == nNbVec)
		return true;
	return false; // Not entirely clipped
}

// ---------------------------------------------------------------------------
void CBuilderLogic::convertToScreen (CVector* pVec, sint nNbVec, CVector &viewMin, CVector &viewMax)
{
	for (sint i = 0; i < nNbVec; ++i)
	{
		pVec[i].x = (pVec[i].x-viewMin.x)/(viewMax.x-viewMin.x);
		pVec[i].y = (pVec[i].y-viewMin.y)/(viewMax.y-viewMin.y);
		pVec[i].z = 0.0f;
	}
}

// ---------------------------------------------------------------------------
bool CBuilderLogic::isInTriangleOrEdge(	double x, double y, 
												double xt1, double yt1, 
												double xt2, double yt2, 
												double xt3, double yt3 )
{
	// Test vector T1X and T1T2
	double sign1 = ((xt2-xt1)*(y-yt1) - (yt2-yt1)*(x-xt1));
	// Test vector T2X and T2T3
	double sign2 = ((xt3-xt2)*(y-yt2) - (yt3-yt2)*(x-xt2));
	// Test vector T3X and T3T1
	double sign3 = ((xt1-xt3)*(y-yt3) - (yt1-yt3)*(x-xt3));
	if( (sign1 <= 0.0)&&(sign2 <= 0.0)&&(sign3 <= 0.0) )
		return true;
	if( (sign1 >= 0.0)&&(sign2 >= 0.0)&&(sign3 >= 0.0) )
		return true;
	return false;
}
