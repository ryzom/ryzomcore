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




#ifndef CL_GROUP_IN_SCENE_USER_INFO_HELP_H
#define CL_GROUP_IN_SCENE_USER_INFO_HELP_H

#include "nel/misc/types_nl.h"
#include "group_in_scene.h"

/**
 * Compas group
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2003
 */
class CGroupInSceneUserInfo : public CGroupInScene
{
public:
	DECLARE_UI_CLASS(CGroupInSceneUserInfo)
	// Constructor
	CGroupInSceneUserInfo(const TCtorParam &param);
	~CGroupInSceneUserInfo();

	// Build the bar
	// You must rebuild if guild or title change
	static CGroupInSceneUserInfo *build (class CEntityCL *entity);

	// release a group in scene. The group is then put in cache for futur reuse
	//static void release(CGroupInSceneUserInfo *group);

	// Show or hide the bar group
	void setLeftGroupActive( bool active );

	// Update dynamic data : "Selection", "Bars"
	// Call it at each frame for visible title
	void updateDynamicData ();

	// Tells if this group actually want guildName or guildSymbol => must rebuild interface if changes
	bool	needGuildNameId() const {return _NeedGuildNameId;}
	bool	needGuildSymbolId() const {return _NeedGuildSymbolId;}
	bool	isLeftGroupActive() const {return _IsLeftGroupActive;}

	virtual void serial(NLMISC::IStream &f);

protected:

	enum TBar
	{
		// Living entity	Forage source
		HP = 0,				Time = HP,		// If making changes to this enum, please update
		SAP,				Amount = SAP,	// CForageSourceCL::updateVisualPropertyBars() and
		STA,				Life = STA,		// updateVisiblePostPos() because the HP/SAP/STA
		Focus,				Danger = Focus, // properties are *not* in an array in CEntityCL.
		Action,				Spawn = Action,
		NumBars
	};

	/// Fill NumBars elements into bars and set dbEntry
	static void	getBarSettings( CInterfaceManager* pIM, bool isUser, bool isPlayer, bool isFriend, int &dbEntry, bool *bars );

	// The entity (character or forage source)
	CEntityCL	*_Entity;

	// View text
	class CViewText	*_Name;
	class CViewText	*_Title;
	class CViewText	*_GuildName;
	class CViewText	*_TribeName;
	class CViewText	*_EventFaction;
	// for Ring, symbol of permanent content
	class CViewBitmap *_PermanentContent;
	class CViewBitmap *_Bars[NumBars];
	class CViewBitmap *_Target;
	class CViewBitmap *_MissionTarget;
	static uint	_BatLength;
	static NLMISC::CRGBA BarColor[NumBars];
	static NLMISC::CRGBA BarColorHPNegative;

	// Node user leaf
	static NLMISC::CCDBNodeLeaf	*_Value;
	static NLMISC::CCDBNodeLeaf	*_ValueBegin;
	static NLMISC::CCDBNodeLeaf	*_ValueEnd;

	// Guild icon leafs
	static NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _GuildIconLeaf[256];

	// Special guild
	bool		_NeedGuildNameId;
	bool		_NeedGuildSymbolId;
	bool		_IsLeftGroupActive;


	static CGroupInSceneUserInfo *newGroupInScene(const std::string &templateName, const std::string &id);

};


#endif // CL_GROUP_IN_SCENE_USER_INFO_HELP_H
