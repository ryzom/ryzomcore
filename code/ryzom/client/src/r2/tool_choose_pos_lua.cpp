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
#include "editor.h"
#include "tool_choose_pos_lua.h"
//
#include "nel/misc/vectord.h"

using namespace NLPACS;
using namespace NLMISC;

namespace R2
{

// ***************************************************************
CToolChoosePosLua::CToolChoosePosLua(uint ghostSlot,
									 const CLuaObject &validFunc,
									 const CLuaObject &cancelFunc,
									 const std::string &toolName,
									 const std::string &cursValid,
									 const std::string &cursInvalid,
									 const std::vector<CPolygon2D> &polyList,
									 const CPrimLook &polyValidLook,
									 const CPrimLook &polyInvalidLook
									) :	CToolChoosePos(ghostSlot,
													   cursValid,
													   cursInvalid,
													   polyList,
													   polyValidLook,
													   polyInvalidLook
													  ),
										_ToolName(toolName)
{
	_ValidFunc = validFunc;
	_CancelFunc = cancelFunc;
	_Commited = false;
}

// ***************************************************************
void CToolChoosePosLua::commit(const NLMISC::CVector &createPosition, float /* createAngle */)
{
	//H_AUTO(R2_CToolChoosePosLua_commit)
	nlassert(!_Commited);
	if (_ValidFunc.isFunction())
	{
		CLuaState &lua = *_ValidFunc.getLuaState();
		lua.push(createPosition.x);
		lua.push(createPosition.y);
		lua.push(createPosition.z);
		_ValidFunc.callNoThrow(3, 0);
	}
	_Commited = true;
}

// *********************************************************************************************************
void CToolChoosePosLua::cancel()
{
	//H_AUTO(R2_CToolChoosePosLua_cancel)
	if (_Commited) return;
	if (_CancelFunc.isFunction())
	{
		_CancelFunc.callNoThrow(0, 0);
	}
}



} // R2
