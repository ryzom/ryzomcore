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

#ifndef R2_INSTANCE_MAP_DECO_H
#define R2_INSTANCE_MAP_DECO_H


#include "../interface_v3/group_map.h"
#include "displayer_visual.h"

namespace NLGUI
{
	class CCtrlQuad;
	class CViewBitmap;
}

class CGroupMap;

namespace R2
{

class CInstance;

/** Tool to display of an instance in current scenario in the world map (a world map DECO
  * This class derives from CGroupMap::IDeco, meaning it must be added / removed to the world map
  * by calling CGroupMap::addDeco / CGroupMap::removeDeco
  */

class CInstanceMapDeco : public CGroupMap::IDeco
{
public:
	CInstanceMapDeco();

	// Set the instance displayed by this map decoration
	void setDisplayedInstance(CInstance *instance, bool orientable);
	bool isAddedToMap() const { return _AddedToMap; }

	NLMISC::CVector2f getWorldPos() const;

	/** Set the texture to use when the view in the world map is zoomed-in enough
	  * When empty, no close view display is done
	  */
	void setCloseTexture(const std::string &texture) { _CloseTexture = texture; }
	// from IDeco
	virtual void onAdd(CGroupMap &owner);
	// from IDeco
	virtual void onRemove(CGroupMap &owner);
	// from IDeco
	virtual void onUpdate(CGroupMap &owner);
	// from IDeco
	virtual void onPreRender(CGroupMap &owner);
	//
	void setTextureAndFit(const std::string &bitmapName);
	//
	void invalidateCoords();

	// set visibility
	void setActive(bool active);
	bool getActive() { return _Active; }

	// Set invalid pos flag
	void setInvalidPosFlag(bool invalid);

private:
	/** special derived class from CBitmap that allows to know what is the current instance when
      * the mouse position is tested against the map (see R2::CTool::checkInstanceUnderMouse for details)
	  */
	class CCtrlButtonEntity : public CCtrlButton, public IDisplayerUIHandle
	{
	public:
		CCtrlButtonEntity(CInstance &instance) : CCtrlButton(TCtorParam()), _Instance(instance) {}
		// from IDisplayerUIHandle
		virtual CInstance &getDisplayedInstance() { return _Instance; }
		virtual bool		handleEvent (const NLGUI::CEventDescriptor &event);
	private:
		CInstance			&_Instance;
	protected:
		virtual void		getContextHelp(ucstring &help) const;
		bool				emptyContextHelp() const { return true; }
		bool				wantInstantContextHelp() const { return true; }
		virtual	bool		isCapturable() const { return false; }
	};
	//
	CInstance			*_Instance;
	CCtrlButtonEntity	*_Main;
	CCtrlQuad			*_Over;
	CCtrlQuad			*_OverInvalid;
	CCtrlQuad			*_GlowStar[2];
	CCtrlQuad			*_Orient;
	CVector				_GlowStarPos;
	std::string			_CloseTexture;
	float				_OrientBlendFactor;
	bool				_GlowStarActive : 1;
	bool				_LastCloseView : 1;
	bool				_AddedToMap : 1;
	bool				_Active : 1;
	bool				_Orientable : 1;
	bool				_InvalidPos : 1;

private:
	CCtrlQuad *newQuad(CGroupMap &owner);
};


} // R2



#endif

