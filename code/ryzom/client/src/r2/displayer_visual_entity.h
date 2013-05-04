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

#ifndef R2_DISPLAYER_VISUAL_ENTITY_H
#define R2_DISPLAYER_VISUAL_ENTITY_H

#include "displayer_visual.h"
#include "instance.h"
#include "../decal.h"
#include "nel/gui/lua_object.h"
#include "instance_map_deco.h"

class CEntityCL;
class CGroupMap;

namespace R2
{

class CObjectTable;


class CMapDecoEntity; // display of entity on the map
class CDisplayerVisualShape;

/** Display of entities
  */
class CDisplayerVisualEntity : public CDisplayerVisual
{
public:
	typedef NLMISC::CRefPtr<CDisplayerVisualEntity> TRefPtr;

	enum TSelectionDisplayMode { AutoSelection = 0, BoxSelection = 1, CircleSelection = 2, SelectionDisplayModeCount };

	NLMISC_DECLARE_CLASS(R2::CDisplayerVisualEntity);
	// ctor
	CDisplayerVisualEntity();
	// dtor
	~CDisplayerVisualEntity();

	////////////
	// EVENTS //
	////////////
	virtual void onPreRender();
	virtual void onPostRender();
	virtual void onAttrModified(const std::string &name, sint32 index);
	virtual void onSelect(bool selected);
	virtual void onPostHrcMove();
	virtual void onFocus(bool focused);
	virtual void onCreate();
	virtual void onPostCreate();
	//
	virtual void onParentDisplayModeChanged();

	////////////////
	// PROPERTIES //
	////////////////
	std::string		  getSheet() const;
	float			  getAngle() const { return _Angle; }
	std::string		  getVisualProperties() const;
	void			  setVisualProperties(const std::string&) {}

	// from ISelectableObject
	const NLMISC::CMatrix	&getInvertedMatrix() const;
	virtual bool			getLastClip() const;
	virtual NLMISC::CAABBox getSelectBox() const;
	virtual float			preciseIntersectionTest(const NLMISC::CVector &worldRayStart, const NLMISC::CVector &worldRayDir) const;

	//////////
	// MISC //
	//////////
	float					getSelectionDecalRadius() const;
	//virtual NLMISC::CVector evalLinkPoint(bool leader) const;
	virtual void			snapToGround();
	// Get pointer to client entity that is used to display that object
	CEntityCL *getEntity() const { return _Entity; }


	// slot entity
	void		setSlotEntity(sint32 /* val */){}
	sint32		getSlotEntity() const;

	// value of type TSelectDisplayMode (uint32 type for script export)
	void	setSelectionDisplayMode(sint32 value) { _SelectionDisplayMode = (TSelectionDisplayMode) ((uint32) value % SelectionDisplayModeCount); }
	sint32	getSelectionDisplayMode() const { return (sint32) _SelectionDisplayMode; }

	/// Return the entity permanent content texture name.
	const std::string &getPermanentStatutIcon() const {return _PermanentStatutIcon;}
	/// Change the entity permanent content texture name.
	void setPermanentStatutIcon(const std::string &name) { _PermanentStatutIcon=name; }

	int luaUpdateName(CLuaState &ls);
	int luaUpdatePermanentStatutIcon(CLuaState &ls);
	REFLECT_EXPORT_START(R2::CDisplayerVisualEntity, R2::CDisplayerVisual)
			// lua
			REFLECT_SINT32("slotEntity", getSlotEntity, setSlotEntity);
			REFLECT_STRING("visualProperties", getVisualProperties, setVisualProperties);
			REFLECT_LUA_METHOD("updateName", luaUpdateName);
			REFLECT_LUA_METHOD("updatePermanentStatutIcon", luaUpdatePermanentStatutIcon);
			REFLECT_SINT32("SelectionDisplayMode", getSelectionDisplayMode, setSelectionDisplayMode);
	REFLECT_EXPORT_END

	// from ISelectableObject
	virtual	bool			isInProjection(const NLMISC::CVector2f &pos) const;
	// from ISelectableObject
	virtual TSelectionType getSelectionType() const;

	// return radius of the cylinder for the collision primitive, or -1 if a box is used
	float getCylinderRadius() const;

	// from CDisplayerVisual
	virtual void setDisplayMode(sint32 mode);

	// manage clipping of entity to display then earest entities (and thus bypass the 255 entities limit)
	void setClipBlend(float amount);


	// test if the entity was created by the user right now (not by another user or undo/reload/reload operation)
	bool isCreatedThisFrame() const;

	bool isUserCreated() const;

	// from CDisplayerBase
	virtual bool maxVisibleEntityExceeded() const;

	// from CDisplayerVisual
	virtual void setActive(bool active);
	virtual bool getActive() const;


private:
	uint64					_CreationTimeSwpBufCount;
	TSelectionDisplayMode	_SelectionDisplayMode;
	CEntityCL				*_Entity;
	mutable NLMISC::CMatrix _InvertedMatrix;
	mutable bool			_InvertedMatrixTouched;
	CInstanceMapDeco		_MapDeco;
	float					_Angle;
	bool					_AddedToMap;
	bool					_RegisteredInEditor; // entitiy displayer register themselves in editor (it sorts entities by distance
												 // and display the nearest one by updating their clipping by calling 'setClipAmount')
	float					_ClipBlend;

	// Current permanent content symbol for the entity
	std::string				_PermanentStatutIcon;
	//
	CDisplayerVisualShape   *_PlaceHolder; // place holder used when there are too many visible entities

private:
	void deletePlaceHolder();
	void eraseEntity();
	void updateEntity();
	void updateRaceAndSex();
	void updateEntityWorldPos();
	void updateEntityRot();
	void updateName();
	void updatePermanentStatutIcon(const std::string & textureName);
	void updateMapDeco();
	void updateWorldMapPresence();
	void retrieveAngle();
	// draw the bbox
	void drawBBox(NLMISC::CRGBA colOverZ, NLMISC::CRGBA colUnderZ);
	void drawBBox(const NLMISC::CMatrix &modelMatrix, const NLMISC::CAABBox &bbox, NLMISC::CRGBA colOverZ, NLMISC::CRGBA colUnderZ);
	void showSelected();
	void showHighlighted();
	void setVisualSelectionBlink(bool on, NLMISC::CRGBA color);
	TSelectionDisplayMode getActualSelectionDisplayMode() const;
protected:

	// update world position from position property
	virtual void updateWorldPos();
	//void updateValidPosFlag();
	void createEntity();
	void updateVisibility();
};


} // R2





#endif
