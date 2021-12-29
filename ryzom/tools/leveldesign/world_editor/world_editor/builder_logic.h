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

// This class is interface between all that is displayed in
// display/tools view and the game core

#ifndef BUILDERLOGIC_H
#define BUILDERLOGIC_H

// ***************************************************************************

#include <map>

// ***************************************************************************

class CDisplay;
class CToolsLogic;

namespace NL3D
{
	class CVertexBuffer;
	class CPrimitiveBlock;
}

// ***************************************************************************

class CBuilderLogic
{
public:

	/// Callback class for logic dependent object.
	class ILogicCallback
	{
	public:
		virtual void onLogicChanged(uint prim) =0;
	};
	
private:

	std::vector<bool>					_MustAskSaves;
	sint32								_RegionSelected;

	// Selection
	HTREEITEM							_ItemSelected;
	std::string							_ItemSelectedName;
	std::vector<sint32>					_VerticesSelected;
	NLMISC::CRGBA						_SelectionCol;

	CDisplay							*_Display;
	CToolsLogic							*_ToolsLogic;

	ILogicCallback						*_LogicCallback;

public:

	CBuilderLogic();
	void setLogicCallback(ILogicCallback *logicCallback);

	void setDisplay (CDisplay *pDisp);
	void setToolsLogic	(CToolsLogic *pTool);
	void uninit ();
	void updateToolsLogic ();
	void newZone ();
	void move (const std::string &name, float x, float y);
	
	void insertPoint (uint32 nPos, HTREEITEM item, const char *Name, const char *LayerName);
	void insertPath (uint32 nPos, HTREEITEM item, const char *Name, const char *LayerName);
	void insertZone (uint32 nPos, HTREEITEM item, const char *Name, const char *LayerName);
	void del (HTREEITEM item);
	void primHide (HTREEITEM item);
	void primUnhide (HTREEITEM item);
	void hideAll (uint32 nPos, sint32 nID, bool bHide);
	void regionSetName (uint32 nPos, HTREEITEM item, const std::string &sRegionName);
	void regionCreateGroup (uint32 nPos, HTREEITEM item, const std::string &sGroupName);
	void regionDeleteGroup (uint32 nPos, const std::string &sGroupName);
	void regionHideAll (uint32 nPos, bool bHide); // (bHide == false) -> unhide
	void regionHideType (uint32 nPos, const std::string &Type, bool bHide); // (bHide == false) -> unhide
	int  getMaxPostfix (const char *prefix);
	bool isAlreadyExisting (const char *sPrimitiveName);

	const char* getName (HTREEITEM item);
	const char* getLayerName (HTREEITEM item);
	void getAllPrimZoneNames (std::vector<std::string> &allNames);
	bool isHidden (HTREEITEM item);
	void setName (HTREEITEM item, const char *pStr);
	void setLayerName (HTREEITEM item, const char *pStr);

	// Operation on Selected PrimBuild
	HTREEITEM getSelPB ();
	std::string getSelPBName ();
	void setSelPB (HTREEITEM item);
	void createVertexOnSelPB (NLMISC::CVector &v, sint32 atPos = -1);
	bool selectVerticesOnSelPB (NLMISC::CVector &selMin, NLMISC::CVector &selMax);
	void createIsoVertexOnSelVertexOnSelPB ();
	bool isSelection();
	void setSelVerticesOnSelPB (NLMISC::CVector &v);
	void delSelVerticesOnSelPB ();
	void stackSelPB ();

	std::string getZonesNameAt (NLMISC::CVector &v);
	void selectZoneAt (NLMISC::CVector &v);

	void render (NLMISC::CVector &viewMin, NLMISC::CVector &viewMax);

private:

	void notify();

	// Help for rendering
	bool isInTriangleOrEdge(	double x, double y, 
								double xt1, double yt1, 
								double xt2, double yt2, 
								double xt3, double yt3 );
	void convertToScreen (NLMISC::CVector* pVec, sint nNbVec, NLMISC::CVector &viewMin, NLMISC::CVector &viewMax);
	bool clip (NLMISC::CPrimVector *pVec, uint32 nNbVec, NLMISC::CVector &viewMin, NLMISC::CVector &viewMax);
	void renderDrawPoint (NLMISC::CVector &pos, NLMISC::CRGBA &col, NL3D::CVertexBuffer *pVB, NL3D::CPrimitiveBlock *pPB);
	void renderDrawLine (NLMISC::CVector &pos, NLMISC::CVector &pos2, NLMISC::CRGBA &col, NL3D::CVertexBuffer *pVB, NL3D::CPrimitiveBlock *pPB);
	void renderDrawTriangle (NLMISC::CVector &pos, NLMISC::CVector &pos2, NLMISC::CVector &pos3, NLMISC::CRGBA &col, NL3D::CVertexBuffer *pVB, NL3D::CPrimitiveBlock *pPB);
	NLMISC::CRGBA findColor(const std::string &LayerName);

};

// ***************************************************************************

#endif // BUILDERLOGIC_H