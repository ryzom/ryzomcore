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




#ifndef CL_GROUP_IN_SCENE_HELP_H
#define CL_GROUP_IN_SCENE_HELP_H

#include "nel/misc/types_nl.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_menu.h"


/**
 * Group Displayed "InScene", with a 3d Position
 *	NB: it does require that you change the Position each frame, but you don't have to call
 *	invalidateCoords() each frame
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2003
 */
class CGroupInScene : public CInterfaceGroup
{
public:
	DECLARE_UI_CLASS(CGroupInScene)
	// Constructor
	CGroupInScene(const TCtorParam &param);
	~CGroupInScene();

	// From CInterfaceElement
	virtual void updateCoords();
	virtual void draw();
	virtual bool parse (xmlNodePtr cur,  CInterfaceGroup *parent);
	// Called each frame to just move X/Y positions.
	virtual void onFrameUpdateWindowPos (sint dx, sint dy);

	// Position of the group in world space
	NLMISC::CVector	Position;
	// useful only if getUserScale()==true
	float			Scale;

	void		setUserScale(bool us);
	bool		getUserScale() const {return _UserScale;}

	REFLECT_EXPORT_START(CGroupInScene, CInterfaceGroup)
		REFLECT_SINT32("offset_x", getOffsetX, setOffsetX);
		REFLECT_SINT32("offset_y", getOffsetY, setOffsetY);
	REFLECT_EXPORT_END

	sint32 getOffsetX() const { return _OffsetX; }
	void setOffsetX(sint32 dmh) { _OffsetX = dmh; }
	sint32 getOffsetY() const { return _OffsetY; }
	void setOffsetY(sint32 dmh) { _OffsetY = dmh; }

	// set/return the ZBias for this group in scene (default 0)
	void	setZBias(float zbias) {_ZBias= zbias;}
	float	getZBias() const {return _ZBias;}

	virtual void serial(NLMISC::IStream &f);

protected:

	// Projected Position memorized. x/y is in window coordinate, while z in is world/camera coordinate
	NLMISC::CVector		_ProjCenter;

	// Offset
	sint32				_OffsetX, _OffsetY;

	// scale the group according to user setup
	bool				_UserScale;

	// Zbias
	float				_ZBias;

	void		computeWindowPos(sint32 &newX, sint32 &newY, NLMISC::CVector &newProjCenter);

	static const float NearDrawClip;
};


#endif // CL_GROUP_IN_SCENE_HELP_H
