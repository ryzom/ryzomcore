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

#ifndef R2_DISPLAYER_VISUAL_SHAPE_H
#define R2_DISPLAYER_VISUAL_SHAPE_H


#include "displayer_visual.h"
#include "../interface_v3/group_map.h"
#include "instance_map_deco.h"

class CEntityCL;

namespace NLGUI
{
	class CViewBitmap;
}

namespace NL3D
{
	class UVisualCollisionEntity;
}

namespace R2
{

class CObjectTable;

// Display a scenario object with a 'Shape'
class CDisplayerVisualShape : public CDisplayerVisual, public CGroupMap::IDeco
{
public:
	NLMISC_DECLARE_CLASS(R2::CDisplayerVisualShape);
	// ctor
	CDisplayerVisualShape(const std::string &shapeName = "", float scale = 1.f, bool worldMapDisplay = true);
	// dtor
	~CDisplayerVisualShape();
	// Init from a lua table. Parms should contain 'ShapeName' as a string
	virtual bool init(const CLuaObject &parameters);
	////////////
	// EVENTS //
	////////////
	virtual void onPreRender();
	virtual void onPostRender();
	virtual void onAttrModified(const std::string &name, sint32 index);
	virtual void onFocus(bool focused);
	virtual void onSelect(bool selected);
	//
	virtual void onParentDisplayModeChanged();


	// From ISelectableObject
	virtual bool			getLastClip() const;
	virtual NLMISC::CAABBox getSelectBox() const;
	virtual float			preciseIntersectionTest(const NLMISC::CVector &worldRayStart, const NLMISC::CVector &worldRayDir) const;
	const NLMISC::CMatrix	&getInvertedMatrix() const;
	virtual void			snapToGround();

	virtual NLMISC::CVector evalLinkPoint(bool leader = false);

	// from CDisplayerVisual
	void setDisplayMode(sint32 mode);

	// get current instance being displayed
	NL3D::UInstance &getMesh() { return _Instance; }


private:
	std::string					_ShapeName;
	bool						_Touched;
	mutable NLMISC::CMatrix		_InvertedMatrix;
	bool						_BadShapeName;
	bool						_Active;
	bool						_WorldMapDisplay;
	float						_Scale;
	NL3D::UInstance				_Instance;
	CInstanceMapDeco			_MapDeco;
	bool						_VisualSnapToGroundDone;
	float						_LastCamDist;
	NLMISC::CMatrix				_BBoxMatrix;
	NL3D::UVisualCollisionEntity   *_VisualCollisionEntity;
private:
	void					drawBBox(NLMISC::CRGBA color) const;
	void					deleteShape();
protected:
	// from CDisplayerVisual
	virtual void setActive(bool active);
	virtual bool getActive() const;
	virtual void updateWorldPos();
	// from CGroupMap::IDeco
	virtual void onAdd(CGroupMap &owner);
	virtual void onRemove(CGroupMap &owner);
	virtual void onPreRender(CGroupMap &owner);
	virtual void onUpdate(CGroupMap &owner);
private:
	void visualSnapToGround();
	void updateMapDeco();
	void deleteVisualCollisionEntity();
};


} // R2





#endif
