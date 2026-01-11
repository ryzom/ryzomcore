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

#ifndef R2_TOOL_PICK
#define R2_TOOL_PICK

#include "tool.h"
#include "editor.h"
//
#include "nel/misc/smart_ptr.h"

namespace R2
{

/** Tool to select an instance in scene / in the component feature list / in the world map
  * The tool can possibly select a position in scene if there's not entity
  * under the mouse.
  */
class CToolPick : public CTool
{
public:
	// build a new pick tool with the given CPicker interface (should not be NULL or assert)
	CToolPick(
				const std::string &cursCanPickInstance = "r2ed_tool_can_pick.tga",
				const std::string &cursCannotPickInstance = "curs_stop.tga",
				const std::string &cursCanPickPos = "r2ed_tool_pick.tga",
				const std::string &cursCannotPickPos = "r2ed_tool_pick.tga",
				bool wantMouseUp = false
			);

	// from CTool
	virtual const char *getToolUIName() const { return ""; } // by default, no associated icon in the ui
	virtual bool  isCreationTool() const { return false; }
	virtual bool  isPickTool() const { return true; }
	virtual void updateAfterRender();
	virtual bool onMouseRightButtonClicked();
	virtual bool onMouseLeftButtonDown();
	virtual bool onMouseLeftButtonClicked();
	virtual void cancel() {}

	void setIgnoreInstances(const std::string & ignoreInstances);

	//////////////////
	// FOR DERIVERS //
	//////////////////

	// Called by the picker tool when an instance has been selected
	virtual void pick(CInstance &instance) = 0;
	// Called by the picker tool when a position has been selecte
	virtual void pick(const NLMISC::CVector &pos) = 0;
	/** Called when the picking action has been canceled.
	  * Default behaviour is to restore the default tool
	  */
	virtual void cancelPick() { getEditor().setCurrentTool(NULL); }
	// Test to see if an instance is 'pickable' (default is yes)
	virtual bool canPick(const CInstance &/* instance */) const { return true; }

	// lua exports
	/** Export the pick method to lua.
	  * We need to expose this to lua, because there's no way
	  * to route user events of the ui to the current editor tool.
	  * So to maintain UI abstraction we use 2 steps :
	  * CTool::checkInstanceUnderMouse() is called -> if no collision is found in scene or in world map, a call
	  * is made into lua to retrieve ui instance over which the mouse is. Then the mouse cursor is updated ifthat instance is 'pickable'
	  * On a mouse button click, lua check that the current tool is a picking tool. If so, then
	  * lua code call the 'pick' method exported below to do the selection.
	  */
	int luaPick(CLuaState &ls);
	int luaCanPick(CLuaState &ls);
	//
	REFLECT_EXPORT_START(R2::CToolPick, R2::CTool)
		REFLECT_LUA_METHOD("pick", luaPick);
		REFLECT_LUA_METHOD("canPick", luaCanPick); // return true if the curent instance under the mouse can be selected
	REFLECT_EXPORT_END

private:
	std::string        _CursCanPickPos;
	std::string        _CursCannotPickPos;
	std::string        _CursCanPickInstance;
	std::string        _CursCannotPickInstance;
	CInstance::TRefPtr _CandidateInstance;
	NLMISC::CVector	   _Intersection;
	bool			   _ValidPos;
	bool			   _WantMouseUp;
	std::vector<std::string> _IgnoreInstances;
private:
	bool	validate();
};




} // R2



#endif

