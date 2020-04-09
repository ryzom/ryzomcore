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

#ifndef R2_DISPLAYER_VISUAL_H
#define R2_DISPLAYER_VISUAL_H


#include "displayer_base.h"
#include "instance.h"
//
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/rgba.h"
//
#include "../decal.h"

class CGroupInScene;

namespace R2
{

// ***********************************************************************************************************************
// Interface for object selection in the scene
struct ISelectableObject
{
	enum TSelectionType { LocalSelectBox = 0, WorldSelectBox, GroundProjected };
	// Intersection & selection testing
	virtual bool			isSelectable() const = 0;
	virtual bool			getLastClip() const { return false; }
	// the object is projected on the scene, and so intersection test
	// ask if current intersection with mouse ray in in the projection of the object (must call isInProjection)
	virtual const NLMISC::CMatrix &getInvertedMatrix() const { return NLMISC::CMatrix::Identity; }
	virtual TSelectionType getSelectionType() const { return LocalSelectBox; }
	virtual	bool			isInProjection(const NLMISC::CVector2f &/* pos */) const { return false; }
	virtual	bool			isInProjectionBorder(const NLMISC::CVector2f &/* pos */) const { return false; }
	virtual NLMISC::CAABBox getSelectBox() const { return NLMISC::CAABBox();  }
	virtual float			preciseIntersectionTest(const NLMISC::CVector &/* worldRayStart */, const NLMISC::CVector &/* worldRayDir */) const { return FLT_MAX; }
	virtual CInstance		*getInstanceInEditor() const = 0;
};

// ***********************************************************************************************************************
// Decoration in a CGroupMap that expose this interface (through dynamic_cast) are representation of CInstance in the island map
struct IDisplayerUIHandle
{
	virtual CInstance &getDisplayedInstance() = 0;
	// test that the given position intesersect the displayer (default is true, because this is call
	// only if bounding rect test succeeded)
	virtual bool contains(sint32 /* mouseXInWindow */, sint32 /* mouseYInWindow */) const { return true; }
	// special test : is it an edge of a zone ?
	virtual bool isEdge() const { return false; }
	// for edge, give the edge index in its parent zone
	virtual uint getEdgeIndex() const { nlassert(0); return 0; }
};


// ***********************************************************************************************************************
// Displays an object of the editor in the 3D scene / in the worldmap
class CDisplayerVisual : public CDisplayerBase, public ISelectableObject
{
public:
	typedef NLMISC::CSmartPtr<CDisplayerVisual> TSmartPtr;
	typedef NLMISC::CRefPtr<CDisplayerVisual> TRefPtr;
	enum TDisplayFlags
	{
		FlagNone        = 0,
		FlagSelected,
		FlagHasFocus,
		FlagHighlighted,
		FlagBadPos, // a 'stop' icon is displayed on entity when it is on a bad landscape position
				    // may happen whn a group is moved as a whole
		FlagHideActivities,
		FlagCount
	};


	// Current display mode. Display mode is retrieved from the lua property 'DisplayMode', and is thus
	// saved with the object
	enum TDisplayMode
	{
		DisplayModeVisible = 0, // the object is visible
		DisplayModeHidden = 1,  // the object is not visible (not in scene / not in the minimap)
		DisplayModeFrozen = 2,  // the object can't be selected, but is visible
		DisplayModeLocked = 3,  // the object can be selected, but not moved / rotated
		DisplayModeArray = 4,  // The instance is in an array being created by the 'array' tool
		DisplayModeCount
	};

	CDisplayerVisual();
	virtual ~CDisplayerVisual();

	// Init parameters from script
	virtual bool init(const CLuaObject &parameters);


	////////////
	// EVENTS //
	////////////

	/** Default behaviour when act is changed is to check whether
	  * the object is visible in current act by calling "isVisibleInCurrentAct"
	  * If not visible then "setActive(false)" will be called to remove the displayed object from the view.
	  * Modification messages (if any ...) should then be ignored by derivers until onCreate is called again
	  */
	virtual void onPreActChanged();
	virtual void onActChanged();
	virtual void onContinentChanged();
	virtual void onPreRender() {}
	virtual void onPostRender();
	/** NB : derivers should not usually react to the 'onPostCreate' or 'onErase' methods
	  * because default behavior of these is to call 'setActive' as necessary when the user select
	  * an act in which the displayed instance is visible in the scenario.
	  */
	virtual void onPostCreate();
	// for derivers : see 'onPostCreate' remarks
	virtual void onErase();
	virtual void onFocus(bool focused);
	virtual void onSelect(bool selected);
	/** Derivers note : 'onAttrModifier' takes care of updating position of the displayer,
      * hence derivers should call their parent version before updating real position in the 3D scene
	  */
	virtual void onAttrModified(const std::string &attrName, sint32 attrIndex);
	/** Default behavior of 'onPostHrcMove' is to force to recompute the world pos
	  * Because object may have been made son of an new object with another world pos
      */
	virtual void onPostHrcMove();

	//////////
	// MISC //
	//////////

	// get display mode, taking possible inheritance in account
	virtual TDisplayMode getActualDisplayMode() const;
	// get display mode, (not taking possible inheritance in account), sint32 for lua export
	sint32 getDisplayMode() const { return _DisplayMode; }
	// set display moed (sint32 for lua export)
	virtual void setDisplayMode(sint32 mode);

	// return true if the object is currently visible *in current act* (mean it is not and'ed with the getActive() flag)
	bool		getActualVisibility() const { return getActualDisplayMode() != DisplayModeHidden; }

	// Set one display flag for the displayer (selected, highlighted ...)
	void			 setDisplayFlag(TDisplayFlags flag, bool on);
	bool			 getDisplayFlag(TDisplayFlags flag) const;
	// Get parent visual displayer if one exists
	CDisplayerVisual *getParent();
	const CDisplayerVisual *getParent() const;
	/** Eval a point at which the displayed object may be linked to when displaying groups
      * Must be expressed in world coordinates
	  */
	virtual NLMISC::CVector evalLinkPoint(bool /* leader */ = false) { return getWorldPos().asVector(); }
	// Eval enter point (useful for zones)
	virtual bool evalEnterPoint(const NLMISC::CVector &startPoint, NLMISC::CVector &result);
	// Eval exit point (for objects such as roads), default resume to evalEnterPoint
	virtual NLMISC::CVector evalExitPoint();
	// From ISelectableObject
	virtual CInstance		*getInstanceInEditor() const { return getDisplayedInstance(); }
	// Snap the displayed object to the ground (if supported)
	virtual void snapToGround() {}
	// Make this displayer blink
	void blink();
	int luaBlink(CLuaState &ls);
	/** See if instance may be dropped on an invalid pacs pos after a move
      * May have sens for entity such as region which are just projected over the scene
	  * and don't depend on PACS for their display
	  */
	virtual bool isInvalidPacsPosAcceptable() const { return false; }

	/** For the move tool : test if all parts of the object are accessible (e.g not on an unreachable part of the map)
	  * NB : this is a display, flag, not updated if the current displayer is not active !!
	  */
	virtual bool isAccessible() { return true; }

	// test if the current shape for this displayer is valid (may be false for self-intersecting polys)
	virtual bool isValidShape() const { return true; }

	// test if this displayer is a group displayer
	virtual	bool isGroup() const { return false; }

	// from ISelectableObject
	virtual bool			isSelectable() const;

	NLMISC::CRGBA getDisplayModeColorInScene() const;
	NLMISC::CRGBA getDisplayModeColorInMap() const;

	// called by parent when the display mode has changed, so sons may want to update their display if their inherit it
	virtual void onParentDisplayModeChanged() {}

	/////////////////
	// POSITION(S) //
	/////////////////
	// TODO nico : should really move the position into "CInstance", or better, find a way to derive CWorldObject
	// in C++ rather than in lua. Position is a property of the object, not a display property!! Should not be the responsability
	// of the displayer to maintain this ...


	// Get 3D position relative to parent.
	NLMISC::CVectorD  getPos() const   { return _Pos; }
	// Get world 2D position
	NLMISC::CVector2f getWorldPos2f() const { return NLMISC::CVector2f((float) _WorldPos.x, (float) _WorldPos.y); }
	// Get world 3D position
	NLMISC::CVectorD  getWorldPos() const { return _WorldPos; }
	// Test if this object should inherit its parent pos
	bool			  inheritPos() const;
	// Eval all sub-positions in world as CVector2f (default is to clear the vector)
	virtual void	  getSonsWorldPos2f(std::vector<NLMISC::CVector2f> &result);
	virtual void	  getSons(std::vector<CDisplayerVisual *> &sons) const;
	virtual uint	  getNumSons() const { return 0; }
	virtual CDisplayerVisual *getSon(uint /* index */) const { nlassert(0); return NULL; }
	virtual bool	  isCompound() const { return false; }
	//
	virtual float	  getAngle() const { nlassert(0); return 0.f; }

	/////////////////
	// LUA EXPORTS //
	/////////////////

	REFLECT_EXPORT_START(R2::CDisplayerVisual, R2::CDisplayerBase)
			REFLECT_LUA_METHOD("blink", luaBlink);
			REFLECT_SINT32("DisplayMode", getDisplayMode, setDisplayMode);
	REFLECT_EXPORT_END

	// signal that this entity start / stops to be rotated
	virtual void setRotateInProgress(bool rotateInProgress);
	bool getRotateInProgress() const { return _RotateInProgress; }
	virtual void setMoveInProgress(bool moveInProgress);
	bool getMoveInProgress() const { return _MoveInProgress; }

private:
	uint32				_DisplayFlags; // a combination of the TDisplayFlags flags
	sint64				_BlinkStartDate;
	//
	NLMISC::CVectorD	_Pos;
	CGroupInScene		*_IconInScene; // a "stop" icon showing that the object has an invalid position
	bool				 _IconInSceneCreationFailed;
	bool				 _RotateInProgress;
	bool				 _MoveInProgress;
	bool				 _InheritDisplayMode;
	float				 _LastCamDist;
	TDisplayMode		 _DisplayMode;
	// Caching of current parent pointer
	bool				 _LastParentOk;
	CDisplayerVisual	 *_LastParent;
protected:
	NLMISC::CVectorD	_WorldPos;
private:
	// update world position for the objet tree rooted at this object
	void updateWorldPosRecurse();
	void updatePos();
	void updateLocalPos();
	//void updateDisplayMode();
protected:
	NLMISC::CRGBA getBlinkColor(NLMISC::CRGBA defaultColor, NLMISC::CRGBA blinkColor = NLMISC::CRGBA(192, 192, 192)) const;

	// See if the z need to be reevaluated. May happen after a tp or when the camera moves (used by shapes)
	// Calling this will update the invalidity flag
	bool testNeedZEval();
	void updateValidPosFlag();

	//////////////////
	// FOR DERIVERS //
	//////////////////
public:
	virtual void setActive(bool active) = 0;
	virtual bool getActive() const = 0;
	virtual bool isActiveInCurrentAct() const;
protected:
	virtual void evalIconInScenePos(NLMISC::CVector &dest) const;
public:
	/** Protected : Called by parent when its world pos has changed.
	  * Default behaviour is to add instance relative pos to parent world pos
	  */
	virtual void updateWorldPos();
};

} // R2

#endif
