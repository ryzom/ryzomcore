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

#ifndef R2_TOOL_H
#define R2_TOOL_H


#include "nel/misc/smart_ptr.h"
#include "nel/misc/array_2d.h"
//
#include "nel/gui/interface_element.h"
#include "game_share/scenario_entry_points.h"
//

class CInterfaceManager;

namespace NLGUI
{
	class CEventDescriptor;
	class CLuaObject;
	class CGroupContainer;
}

class CGroupMap;

namespace NLMISC
{
	class CVector;
}

namespace DMS
{
	class CDynamicMapClient;
}

namespace R2
{

extern const uint32 DEFAULT_ENTITY_MIN_OPACITY;


struct IDisplayerUIHandle;
class CEditor;
class CInstance;
class CDynamicMapClient;

/** Base class for manipulation tools found in the R2 editor
  * There's only one tool at a moment and mouse/keyboard events are routed to that tool
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 5/2005
  */
class CTool : public CReflectableRefPtrTarget, public NLMISC::IClassable
{
public:
	class CWorldViewRay
	{
	public:
		NLMISC::CVector Origin;
		NLMISC::CVector Dir;
		NLMISC::CVector Right;
		NLMISC::CVector Up;
		bool	OnMiniMap;
		bool    Valid;
	public:
		CWorldViewRay()
		{
			Origin = NLMISC::CVector::Null;
			Dir    = NLMISC::CVector::Null;
			Right  = NLMISC::CVector::Null;
			Up     = NLMISC::CVector::Null;
			OnMiniMap = false;
			Valid = false;
		}
	};

	//\TODO nico find a better place for this
	enum TRayIntersectionType { NoIntersection, ValidPacsPos, InvalidPacsPos };
	//
	typedef NLMISC::CSmartPtr<CTool> TSmartPtr;
	//
	CTool();
	virtual ~CTool() {}
	//
	// Init parameters from script
	virtual bool init(const CLuaObject &/* parameters */) { return true; }
	/** Get this tool name in the ui. This name is used to identify the tool in the ui (used by the r2.ToolUI:setActiveToolUIByName function defined in r2ed_ui.lua)
	  * May return "" if there's no ui associated with that tool
	  */
	virtual const char *getToolUIName() const = 0;
	virtual bool  isCreationTool() const = 0;
	virtual bool  isPickTool() const {return false; }

	//
	// This update is called at each frame for the current tool just before rendering
	virtual void updateBeforeRender() {}
	// This update is called at each frame for the current tool just after rendering
	virtual void updateAfterRender() = 0;

	//////////////////////////
	//   EVENTS HANDLING    //
	//////////////////////////

	/** Entry point for events handling.
	  * Methods where defined below for convenience for the most common events
	  * like 'onMouseLeftButtonDown' or 'onMouseRightButtonUp'.
	  * Default behaviour of this member function is to forward the event to these event handling methods.
	  *
	  * A deriver may handle events not listed below by redefining this method,
	  * possibly calling its parent version for events he is not interested in.
	  *
	  * \return true if the event has been handled by the tool
	  */
	virtual	bool handleEvent (const NLGUI::CEventDescriptor &eventDesc);
	//
	virtual void onFocusGained() {} // the app window gained the focus (there's no 'focus lost' event here because it reset current tool, so 'CTooll::cancel' will be called instead)
									// IMPORTANT :  Reacting to this should be unnecessary, as lost focus reset the current tool,
									// defaulting to the 'SelectMove' tool, that handle this event correctly.
	virtual bool onMouseLeftButtonDown() { return false; }
	virtual bool onMouseLeftButtonUp() { releaseMouse(); return false; }
	virtual bool onMouseRightButtonDown() { return false; }
	virtual bool onMouseRightButtonUp() { return false; }
	virtual bool onMouseMove() { return false; }

	// call when this tool is just being activated
	virtual void onActivate() {}
	// special messages for shortcut keys
	virtual bool onDeleteCmd() { return false; }
	/**
	  * Unlike the other onMousexxx method, these are actually called AFTER the camera event handling
	  * has been done. Doing the same in onMouseRightButtonUp or onMouseRightButtonDown
	  * instead would require that the user test if mouse button was not released after a camera
	  * move (in which case the click should not be handled of course)
	  *
	  * Usually the onMousexxxButtonUp are ususeful when working in pair with the associated 'mouse button down' event.
	  * Such events usually provoke a mouse capture (example : selectMoveTool)
	  */
	virtual bool onMouseLeftButtonClicked() { return false; }
	/** NB : if the onMouseRightButtonClicked isn't handled by the tool then
	  * editor will show the context menu
	  */
	virtual bool onMouseRightButtonClicked();
	// Called by editor just before a new tool is made current
	virtual void cancel() = 0;


	// when returning true -> ignore next click causing unselect
	virtual bool getPreviousToolClickEndFlag(bool /* clear */ = true) { return false; }


	// double click handling :
	// 'startDoubleClickCheck' should be called when the 'onMouseLeftButtonClicked' msg is handled
	// then checkDoubleClick'  should be call on any subsequent onMouseLeftButtonDown. If
	// the result is true then double click should be handled
	void startDoubleClickCheck();
	bool checkDoubleClick();

	// test if one of the 'shift' keys is down
	static bool isShiftDown();

	// test if one of the 'ctrl' keys is down
	static bool isCtrlDown();

	//////////////////////
	// HELPER FUNCTIONS //
	//////////////////////

	/** check which instance is under the mouse, possibly fading player in / out
	  * \param miniMapHandle if not NULL, pointer will be filled with the minimap ui element that is under the mouse
      * \return NULL if there's no instance under the mouse
	  */
	static CInstance *checkInstanceUnderMouse(IDisplayerUIHandle **miniMapHandle = NULL);

	// helper : handle mouse over instance: highlight them when mouse is over & change mouse cursor accordingly
	static void handleMouseOverInstance(const char *cursorDefault,
										const char *cursorOverUnselectedInstance,
										const char *cursorOverSelectedInstance);

	// handle player under cursor (fade in / fade out)
	static void handleMouseOverPlayer(bool over);

	/** Default right button down handling :
	  * If an entity is highlighted,
	  * then select it and pop its menu
	  * \return true if the event was handled
	  */
	bool defaultRightButtonDownHandling();

	/** Capture the mouse
	  * - The ui won't receive events from the mouse
	  * - Maintaining the left button down doesn't trigger camera rotation any more.
	  * Useful for tools such as 'select', 'move' ...
	  */
	static void captureMouse();
	static void releaseMouse();
	static bool isMouseCaptured();

	// shortcut to get the ui
	static CInterfaceManager &getUI();
	// Get mouse  position
	static void getMousePos(sint32 &x, sint32 &y) ;
	// Get mouse x position
	static sint32 getMouseX();
	// Get mouse y position
	static sint32 getMouseY();
	// Set the current mouse cursor
	static void  setMouseCursor(const char *cursorTexture);
	static void  setMouseCursor(const std::string &cursorTexture) { setMouseCursor(cursorTexture.c_str()); }
	/** Compute a view vector (with its direction z set to 1) from coordinate of the mouse on screen
	  * If the mouse is on the island map, then a vector looking down from heights will be returned
	  */
	static void computeWorldViewRay(sint32 posX, sint32 posY, CWorldViewRay &dest);
	// specific test for the world map
	static TRayIntersectionType computeWorldMapIntersection(float x, float y, NLMISC::CVector &inter);

	// get current screen size
	static void getScreenSize(uint32 &scrW, uint32 &scrH);
	// get current screen width
	static uint32 getScreenWidth();
	// get current screen height
	static uint32 getScreenHeight();
	// see if a point is in screen
	static bool isInScreen(sint32 x, sint32 y);
	// test whether the mouse is over the user interface
	static bool isMouseOnUI();
	// retriever ptr on world map in the ui
	static CGroupMap *getWorldMap();
	// test whether the mouse is over the map
	static CGroupMap *isMouseOnWorldMap();
	// test whether the mouse is over a container
	static CGroupContainer *isMouseOnContainer();
	/** Compute collision of a segment with the landscape
	  * \param inter If return type is different from 'NoIntersection', then 'inter' is filled with the collision position
	  */
	static TRayIntersectionType computeLandscapeRayIntersection(const CWorldViewRay &worldViewRay, NLMISC::CVector &inter);
	// Get pacs type at the given position, with the given threshold.
	static TRayIntersectionType getPacsType(const NLMISC::CVector &pos, float threshold, NLPACS::UGlobalPosition &destPos);

	// set context help for the current tool
	static void setContextHelp(const ucstring &contextHelp);

	// shortcut to get the interface to the server
	CDynamicMapClient &getDMC();

	/** handle world map auto-panning feature, should be called whenever auto-pan should be done
	  * dx & dy are filled with delta of the map for this frame
	  */
	void handleWorldMapAutoPan(sint32 &dx, sint32 &dy);

	// lua exports
	int luaIsPickTool(CLuaState &ls);
	//
	REFLECT_EXPORT_START(R2::CTool, CReflectable)
		REFLECT_LUA_METHOD("isPickTool", luaIsPickTool);
	REFLECT_EXPORT_END

	static NLMISC::CRGBA getInvalidPosColor();

	/** For derivers : additionnal checking can be done on the pos to choose
	  * Pos must at least be a valid pacs pos
	  * Default will check with a radius of 0.5 meter
	  */
	static bool isValid2DPos(const NLMISC::CVector2f &pos);

private:
	sint64	_DoubleClickStartTime;
	sint32	_DoubleClickX;
	sint32	_DoubleClickY;
	uint64  _AutoPanLastHandlingFrame;
	sint64  _AutoPanDelay;
	sint64  _NumPans;
	static bool _MouseCaptured;
	static NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _UserCharFade;
private:
	/** compute the nearest valid surface at a given position from the island heightmap
	  * (heightmap must not be empty or an assertion is raised)
	  * \return true if a valid surface was found
	  */
	static bool computeNearestValidSurfaceFromHeightMap(float x, float y, NLMISC::CVector &inter);
	// trace a ray though the scene, using precise camera collision first, island packed collisions then.
	static bool raytrace(const NLMISC::CVector &segmentStart, const NLMISC::CVector &dir, NLMISC::CVector &inter);
	static bool isIslandValidPos(const NLMISC::CArray2D<sint16> &heightMap, const CScenarioEntryPoints::CCompleteIsland &islandDesc, float x, float y);
};

} // R2

#endif
