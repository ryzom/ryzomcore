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

#ifndef R2_TOOL_CHOOSE_POS_H
#define R2_TOOL_CHOOSE_POS_H

#include "tool.h"
#include "../decal.h"
#include "prim_render.h"
#include "nel/misc/vector.h"
#include "nel/misc/polygon.h"

class CEntity;
class CEntityCL;
namespace NLGUI
{
	class CLuaObject;
}

namespace R2
{

/**
  * Tool to choose a position on landscape, with transparent entity drawn under the mouse
  * A transparent version of the entity to be created is drawn under the mouse
  * at the chosen position.
  * Additionnally, a set of zone can be created to choose the pos. These zone must be at a valid
  * position on the map to allow validation
  *
  */
class CToolChoosePos : public CTool
{
public:

	CToolChoosePos() { nlassert(0); }

	enum TAction { NoAction, SelectPos, Cancel };
	//
	CToolChoosePos(sint ghostSlot,
				   const std::string &cursValid = "curs_create.tga",
				   const std::string &cursInvalid = "curs_stop.tga",
				   const std::vector<NLMISC::CPolygon2D> &polyList = std::vector<NLMISC::CPolygon2D>(),
				   const CPrimLook &polyValidLook = CPrimLook(),
				   const CPrimLook &polyInvalidLook = CPrimLook()
				  );

	/** Init for multiple pos selection (for the creation tool)
	  * Multiple position can be chosen by holding 'shift' down
	  */
	void enableMultiPos(const std::string &cursValidMulti = "curs_create_multi.tga");
	bool isMultiPos() const { return _MultiPos; }

	// force multi-creation to be used by default (no need for the shift key down)
	void lockMultiPos() { _MultiPosLocked = true; }


	// {}
	~CToolChoosePos();
	// from CTool
	virtual const char *getToolUIName() const { return ""; }
	virtual bool  isCreationTool() const { return true; }
	virtual void updateAfterRender();
	virtual void updateBeforeRender();
	virtual bool onMouseLeftButtonClicked();
	virtual bool onMouseRightButtonClicked();
	virtual void cancel();
	/** Update the cursor on the ui. Depending on the context the cursor
      * may take different shapes (for example, it wouldn't show a 'stop' cursor when
	  * the palette is under and one has just selected an object to drop)
	  */
	virtual void updateInvalidCursorOnUI();
protected:
	std::string				 _CursValid;
	std::string				 _CursValidMulti;
	std::string				 _CursInvalid;
	bool					 _Valid;
	NLMISC::CVector			 _CreatePosition;
private:
	CDecal					 _BadPlaceDecal;
	CDecal					 _TestDecal;
	bool					 _MultiPos;
	bool					 _MultiPosLocked;
	float					 _CreateAngle;
	TAction					 _WantedAction;
	bool					 _WarningNoMeshOrSkeletonShown;
	NLMISC::CVector			 _DefaultScale;
	float					 _LocalSelectBoxSize;
	std::vector<NLMISC::CPolygon2D>	 _PolyList;
	std::vector<CPrimRender> _PolyRender;
	CPrimLook				 _PolyValidLook;
	CPrimLook				 _PolyInvalidLook;
protected:
	sint					 _GhostSlot;
protected:
	CEntityCL	*getGhost();
	// for derivers
	virtual void commit(const NLMISC::CVector &createPosition, float createAngle) = 0;
	virtual bool stopAfterCommit() const { return true; }
	/** For derivers : additionnal checking can be done on the pos to choose
	  * Pos must at least be a valid pacs pos
	  */
	virtual bool isValidChoosePos(const NLMISC::CVector2f &pos) const { return isValid2DPos(pos); }
	void removeGhostSlot();
private:
	void hideMapSelectionAxis();
	void showPolys(bool visible);
};




} // R2

#endif
