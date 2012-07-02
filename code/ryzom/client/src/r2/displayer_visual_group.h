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

#ifndef R2_VISUAL_DISPLAYER_GROUP_H
#define R2_VISUAL_DISPLAYER_GROUP_H

#include "displayer_visual.h"
#include "prim_render.h"
#include "island_collision.h"
//
#include "nel/3d/u_instance.h"
//
#include "nel/misc/polygon.h"




class CEntityCL;


namespace R2
{

/**
  * Display an instance in a R2 scenario as a primitive. (through the CPrimRender class)
  * Display is made in both the 3D scene and the world map.
  *
  * This displayer is intended to be used for compound objects display
  * The displayer will look into a sub-instance array in the displayed instance to find the vertices of the primitive
  * to display.
  *
  * The displayer is initialized from a lua table, (instance displayers are defined in their lua class definition, see base_class.lua for details)
  *
  *
  * \TODO: rename this into CDisplayerVisualPrimitive (would be more informative)
  */
class CDisplayerVisualGroup : public CDisplayerVisual
{
public:
	typedef NLMISC::CRefPtr<CDisplayerVisualGroup> TRefPtr;
	enum TShape { Star, PolyLine, ClosedPolyLine, ShapeCount };
	NLMISC_DECLARE_CLASS(R2::CDisplayerVisualGroup);
	// ctor
	CDisplayerVisualGroup ();
	/** Init from a lua table : T
	  * The 'Shape' field is a string that give the shape (same values than the TShape enum).
	  * The 'Array' field is a string that give the name of the sub-instance array containing the primitive vertices
	  */
	virtual bool init(const CLuaObject &parameters);
	// dtor
	~CDisplayerVisualGroup();
	// events
	virtual void onPreRender();
	virtual void onPostRender();
	virtual void onFocus(bool focused);
	virtual void onSelect(bool selected);
	virtual void onAttrModified(const std::string &attrName, sint32 attrIndex);
	//
	virtual void onParentDisplayModeChanged();
	//virtual void onTableModified(const std::string &tableName, const std::string &keyInTable, sint32 indexInTable);
	// eval of link point returns first vetrex
	virtual NLMISC::CVector evalLinkPoint(bool leader);
	virtual bool evalEnterPoint(const NLMISC::CVector &startPoint,  NLMISC::CVector &result);
	virtual NLMISC::CVector evalExitPoint();
	// from ISelectableObject
	virtual TSelectionType	getSelectionType() const { return GroundProjected; }
	virtual	bool			isInProjection(const NLMISC::CVector2f &pos) const;
	virtual	bool			isInProjectionBorder(const NLMISC::CVector2f &pos) const;
	// see if a position if over an edge, return the edge index in this case, or -1 if not found
	virtual	sint			isOnEdge(const NLMISC::CVector2f &pos) const;
	const NLMISC::CMatrix	&getInvertedMatrix() const;
	uint					getFadeTimeInMS() const;
	virtual void					getSons(std::vector<CDisplayerVisual *> &sons) const;
	// from CDisplayerVisual
	virtual bool isInvalidPacsPosAcceptable() const { return false; }
	virtual void getSonsWorldPos2f(std::vector<NLMISC::CVector2f> &result);
	virtual uint	  getNumSons() const;
	virtual CDisplayerVisual *getSon(uint index) const;
	virtual bool isAccessible();
	virtual	bool isGroup() const { return true; }
	virtual bool isValidShape() const { return _CurrPrimValid; }
	virtual bool isCompound() const { return true; }
	virtual void setDisplayMode(sint32 mode);

	/** Active contextual visibility. This is road / region specific
	  * In this mode, the road or the region is only displayed if some entity is attached to it
	  * (no-op for groups)
	  */
	void setContextualVisibilityActive(bool active);
	bool getContextualVisibilityActive() const { return _ContextualVisibilityActive; }
	/** If contextual is enabled, tells whether this object is visible with current context
      * Actually when one entity uses that road / region
	  */
	bool isContextuallyVisible();
	//

	// from CVisualDisplayer
	virtual TDisplayMode getActualDisplayMode() const;

	REFLECT_EXPORT_START(R2::CDisplayerVisualGroup, R2::CDisplayerBase)
			REFLECT_BOOL("ContextualVisibilityActive", getContextualVisibilityActive, setContextualVisibilityActive);
	REFLECT_EXPORT_END

private:
	mutable std::vector<CDisplayerVisual *> _Instances;
	/** Special CRenderPrim : when 'in world map' intersection test is done, the CCtrlPolygon will
	  * allow to know which CInstance is actually displayed by this polygon.
	  * This is because the CPrimRender class does not now anything about R2::CInstance or anything related to the scenario...
	  */
	class CSelectablePrimRender : public CPrimRender
	{
	public:
		CSelectablePrimRender() : DisplayedInstance(NULL) {}
		// from CPrimRender
		virtual CCtrlPolygon *newCtrlPolygon() const;
		// from CPrimRender
		virtual CCtrlQuad *newCtrlQuad(uint edgeIndex) const;
		CInstance *DisplayedInstance;
	};
	CSelectablePrimRender			_Prim;				// rendering of edges in the worldmap
	CPrimRender						_InaccessiblePrim;  // inaccessible parts of primitive
	CPrimLook						_PrimLook;
	CPrimLook						_PrimLookInvalid;
	bool							_CurrPrimValid; // is the current polygon valid (e.g can be split into convex polys ?)
	bool							_CurrPrimInaccessible;
	mutable bool					_PrimTouched;
	mutable bool					_InstanceListTouched;
	mutable	bool					_AccessibilityTouched;
	bool							_DrawnThisFrame;
	bool							_Active;
	bool							_ContextualVisibilityActive;	// when true, visibility depends on current selection : group must be part of selectedentity sequence
	//
	sint64							_ContextualVisibilityDate;
	bool							_VisibleLastFrame;
	mutable NLMISC::CPolygon2D		_Poly2D;
	mutable NLMISC::CAABBox			_BBox;
	std::string						_ArrayName;	 // name of the array containing the sons
	sint64							_TimeOver;
	CRefCountedPackedWorld::TSmartPtr _LastIsland;
private:
	NLMISC::CVector evalLinkPoint(CEntityCL &entity);
	//
	void updateInstanceList() const;
	void updatePrimVertices();
	void updateAccessibility();
	void updateBoundingBox();
	void updatePrimLook();
	void touch();
protected:
	// from CDisplayerVisual
	virtual void setActive(bool active);
	virtual bool getActive() const;
	virtual void updateWorldPos();
	// from CDisplayerBase
	virtual void   setDisplayedInstance(CInstance *instance);
public:
	virtual void setActiveRecurse(bool active);
	void setContextualVisibilityDate(sint64 date) { _ContextualVisibilityDate = date; }
};


} // R2

#endif
