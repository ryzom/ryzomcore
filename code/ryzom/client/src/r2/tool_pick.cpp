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
#include "tool_pick.h"
#include "instance.h"
//
#include "nel/gui/lua_ihm.h"


using namespace NLMISC;

namespace R2
{

// **********************************************
CToolPick::CToolPick(const std::string &cursCanPickInstance,
					 const std::string &cursCannotPickInstance,
					 const std::string &cursCanPickPos,
					 const std::string &cursCannotPickPos,
					 bool wantMouseUp
				    )
{
	_CursCanPickInstance = cursCanPickInstance;
	_CursCannotPickInstance = cursCannotPickInstance;
	_CursCanPickPos = cursCanPickPos;
	_CursCannotPickPos = cursCannotPickPos;
	_ValidPos = false;
	_WantMouseUp = wantMouseUp;
}

// **********************************************
void CToolPick::setIgnoreInstances(const std::string & ignoreInstances)
{
	//H_AUTO(R2_CToolPick_setIgnoreInstances)
	string allKind = ignoreInstances;
	while (allKind.size() > 0)
	{
		std::string::size_type e = allKind.find(',');
		string tmp;
		if (e == std::string::npos || e == 0)
		{
			tmp = allKind;
			allKind="";
		}
		else
		{
			tmp = allKind.substr(0,e);
			allKind = allKind.substr(e+1,allKind.size());
		}

		while(tmp.size()>0 && tmp[0]==' ')
		{
			if(tmp.size()==1)
				tmp="";
			else
				tmp = tmp.substr(1,tmp.size());
		}
		while(tmp.size()>0 && tmp[tmp.size()]==' ')
		{
			if(tmp.size()==1)
				tmp="";
			else
				tmp = tmp.substr(0,tmp.size()-1);
		}
		_IgnoreInstances.push_back(tmp);
	}
}

// **********************************************
void CToolPick::updateAfterRender()
{
	//H_AUTO(R2_CToolPick_updateAfterRender)
	// See if the mouse is over a valid position
	_ValidPos = false;
	sint32 mouseX,  mouseY;
	getMousePos(mouseX,  mouseY);
	if (!isInScreen(mouseX,  mouseY))
	{
		getEditor().setFocusedInstance(NULL);
		setMouseCursor(_CursCannotPickPos);
		return;
	}
	_CandidateInstance = NULL;
	CInstance *instanceUnder = checkInstanceUnderMouse();
	bool ignoreInstanceUnder = false;
	if(instanceUnder && _IgnoreInstances.size()>0)
	{
		for(uint i=0; i<_IgnoreInstances.size(); i++)
		{
			if(instanceUnder->isKindOf(_IgnoreInstances[i]))
			{
				ignoreInstanceUnder = true;
				break;
			}
		}
	}
	if (!instanceUnder || ignoreInstanceUnder)
	{
		if (isMouseOnUI() && !isMouseOnContainer())
		{
			setMouseCursor(DEFAULT_CURSOR);
		}
		else
		{
			CTool::CWorldViewRay worldViewRay;
			computeWorldViewRay(mouseX,  mouseY,  worldViewRay);
			CVector inter;
			_ValidPos = (ValidPacsPos == computeLandscapeRayIntersection(worldViewRay,  _Intersection));
			setMouseCursor(_ValidPos ? _CursCanPickPos : _CursCannotPickPos);
			getEditor().setFocusedInstance(NULL);
		}
		return;
	}
	getEditor().setFocusedInstance(instanceUnder);
	if (canPick(*instanceUnder))
	{
		_CandidateInstance = instanceUnder;
		setMouseCursor(_CursCanPickInstance);
	}
	else
	{
		setMouseCursor(_CursCannotPickInstance);
	}
}

// **********************************************
bool CToolPick::onMouseRightButtonClicked()
{
	//H_AUTO(R2_CToolPick_onMouseRightButtonClicked)
	TSmartPtr holdThis = this; // for safety, prevent real deletion of this before this method is exited
	cancelPick();
	return true;
}

// **********************************************
bool CToolPick::validate()
{
	//H_AUTO(R2_CToolPick_validate)
	if (!_CandidateInstance)
	{
		if (_ValidPos)
		{
			captureMouse();
			pick(_Intersection);
			return true;
		}
		return false; // ... else, do not handle, this allow to control the camera
	}
	if (!_WantMouseUp) captureMouse();
	pick(*_CandidateInstance);
	return true;
}

// **********************************************
bool CToolPick::onMouseLeftButtonDown()
{
	//H_AUTO(R2_CToolPick_onMouseLeftButtonDown)
	if (_WantMouseUp) return false;
	return validate();
}

// **********************************************
bool CToolPick::onMouseLeftButtonClicked()
{
	//H_AUTO(R2_CToolPick_onMouseLeftButtonClicked)
	if (!_WantMouseUp) return false;
	return validate();
}

// **********************************************
int CToolPick::luaPick(CLuaState &ls)
{
	//H_AUTO(R2_CToolPick_luaPick)
	CLuaIHM::checkArgCount(ls, "pick", 0);
	validate();
	return 0;
}

// **********************************************
int CToolPick::luaCanPick(CLuaState &ls)
{
	//H_AUTO(R2_CToolPick_luaCanPick)
	CLuaIHM::checkArgCount(ls, "canPick", 0);
	if (_CandidateInstance) ls.push(canPick(*_CandidateInstance));
	else ls.push(false);
	return 1;
}


} // R2
