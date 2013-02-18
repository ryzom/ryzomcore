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

#ifndef R2_TOOL_CHOOSE_POS_LUA_H
#define R2_TOOL_CHOOSE_POS_LUA_H

#include "tool_choose_pos.h"
#include "nel/misc/vector.h"
#include "nel/gui/lua_object.h"

class CEntity;

namespace NLGUI
{
	class CLuaObject;
}

namespace R2
{

/**
  * Choose a position and forward the result to lua
  */
class CToolChoosePosLua : public CToolChoosePos
{
public:
	NLMISC_DECLARE_CLASS(R2::CToolChoosePosLua);
	CToolChoosePosLua() { nlassert(0); }
	CToolChoosePosLua(uint ghostSlot,
					  const CLuaObject &validFunc,
					  const CLuaObject &cancelFunc,
					  const std::string &toolName,
					  const std::string &cursValid = "curs_create.tga",
					  const std::string &cursInvalid = "curs_stop.tga",
					  const std::vector<NLMISC::CPolygon2D> &polyList = std::vector<NLMISC::CPolygon2D>(),
					  const CPrimLook &polyValidLook = CPrimLook(),
					  const CPrimLook &polyInvalidLook = CPrimLook()
					 );
	virtual const char *getToolUIName() const { return _ToolName.c_str(); }
protected:
	// from CToolChoosePos
	virtual void commit(const NLMISC::CVector &createPosition, float createAngle);
	virtual void cancel();
private:
	CLuaObject			_ValidFunc;
	CLuaObject			_CancelFunc;
	bool				_Commited;
	std::string			_ToolName;
};




} // R2

#endif
