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

#ifndef R2_TOOL_SELECT_MOVE_H
#define R2_TOOL_SELECT_MOVE_H

#include "tool_maintained_action.h"
#include "auto_group.h"
#include "nel/misc/vectord.h"

class CEntity;

namespace R2
{

/**
  * Tool to select an element in the scene
  * If the left button is maintained down, then the selected element is moved
  */
class CToolSelectMove : public CToolMaintainedAction
{
public:
	NLMISC_DECLARE_CLASS(R2::CToolSelectMove);

	CToolSelectMove();
	virtual ~CToolSelectMove() {}
	// fram CTool
	virtual void updateBeforeRender();
protected:
	// from CTool
	virtual void onActivate();
	virtual const char *getToolUIName() const { return "selectMove"; }
	virtual bool  isCreationTool() const { return false; }
	virtual bool isActionPossibleOn(const CInstance &instance) const;
	// from CToolMaintainedAction
	virtual void beginAction(CInstance &instance);
	virtual void cancelAction(CInstance &instance);
	virtual void commitAction(CInstance &instance);
	virtual void updateAction(CInstance &instance);
	virtual const char *getCursorForPossibleAction() const;
	virtual const char *getDefaultCursor() const;
	virtual const char *getPickCursor() const;
private:
	//
	sint32 _MouseX;
	sint32 _MouseY;
	NLMISC::CVectorD _StartPos;
	NLMISC::CVectorD _LastValidPos;
	NLMISC::CVectorD _FinalPos;
	NLMISC::CVectorD _DeltaAnchor; // delta pos in world between the point being move and the pos being updated
	bool    _ValidPos;
	bool    _Moved;
	bool	_Duplicating;
	bool	_InitiallyAccessible;
	bool	_InitiallyValidShape;
	sint32 _DeltaX;
	sint32 _DeltaY;
	// tmp tmp
	sint32 _RefX;
	sint32 _RefY;
	sint32 _PosRefX;
	sint32 _PosRefY;
	sint32 _MouseRefX;
	sint32 _MouseRefY;
	sint64 _StartTime;
	CInstance::TRefPtr _GhostInstance;
	CAutoGroup	_AutoGroup;
private:
	bool checkAdditionnalRoomLeftFor(CInstance &instance);
	void setInstancePos(const NLMISC::CVectorD &pos, CInstance &instance);
	CInstance *createGhost(CInstance &instance);
};




} // R2

#endif
