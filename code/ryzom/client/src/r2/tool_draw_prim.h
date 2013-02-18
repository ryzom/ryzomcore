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

#ifndef R2_TOOL_DRAW_PRIM_H
#define R2_TOOL_DRAW_PRIM_H

#include "tool.h"
#include "prim_render.h"
#include "../decal.h"
//
#include "nel/misc/vector.h"


class CEntity;
namespace NLGUI
{
	class CLuaObject;
}

namespace R2
{

/**
  * Tool to draw  / extend a PRIMITIVE (road or zone)
  */
class CToolDrawPrim : public CTool
{
public:
	enum { PrimMaxNumPoints = 16 };
	enum TPrimType { Road = 0, Region, PrimTypeCount };
	NLMISC_DECLARE_CLASS(R2::CToolDrawPrim);
	// ctor
	CToolDrawPrim(TPrimType primType = Road, CInstance *extending = NULL);
	virtual bool init(const CLuaObject &parameters);
	virtual const char *getToolUIName() const;
	virtual bool  isCreationTool() const { return true; }
	virtual void cancel();
	virtual void updateAfterRender();
	virtual void updateBeforeRender();
	virtual bool onMouseLeftButtonClicked();
	virtual bool onMouseRightButtonClicked();
	virtual bool onMouseLeftButtonDown();
	virtual bool onDeleteCmd();

	// update the look of an inaccessible primitive
	static void updateInaccessiblePrimRenderLook(CPrimRender &dest);

protected:
	virtual void onActivate();
private:
	CPrimRender			 _Prim;					// the primitive being drawn
	CPrimRender			 _InaccessiblePrim;		// inaccessible parts for the primitive being drawn (on an invalid pos on heightmap)
	CPrimLook			 _PrimLook;
	CPrimLook			 _PrimLookInvalid;
	CPrimLook			 _PrimLookCanClose;
	CPrimLook			 _PrimLookInaccessible;
	uint				 _NumPoints;
	bool				 _ValidPos;
	bool				 _MustClose;
	bool			     _PrimInitialized;
	bool				 _ValidPrim;
	bool				 _InaccessibleParts;
	bool				 _DistinctLastPoint;
	TPrimType			 _PrimType;
	std::vector<NLMISC::CVector> _Points;
	std::vector<NLMISC::CVector> _InitialPoints; // for extending
	std::vector<NLMISC::CVector> _LastPoints;	// last points for validity test
	std::string			 _CookieKey;			// cookie to be attached if creation succeed (empty string if no cookie)
	CLuaObject			 _CookieValue;
	CLuaObject			 _CancelFunc;
	bool				 _SelectInstance;
	CInstance::TRefPtr	 _ExtendedPrimitive;	// road being extended (NULL is this tool is used to create a new primitive)
	uint				 _StartNumPoints;		// number of points before extend
	bool				 _Extending;
	bool				 _Commited;
	bool				 _ForceShowPrims;
	CDecal				 _TestDecal;
private:
	void commit();
	void setPrimLook(bool closed, bool lastSegmentValid, bool valid);
	void removeFromWorldMap();
	void updateValidityFlag(bool ignoreLast);
	bool canTerminate() const;
	bool checkRoomLeft();
	void displayNoMoreRoomLeftMsg();
	// for closed polygon only : test if the current shape is valid (e.g not intersecting, with at least 3 points)
	bool isValidPolyShape(bool ignoreLast, std::list<NLMISC::CPolygon> &splitPoly) const;
	bool testAccessibleEdges(bool ignoreLast);
	void doUpdateBeforeRender();
};




} // R2

#endif
