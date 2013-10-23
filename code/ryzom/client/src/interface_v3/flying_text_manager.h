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


#ifndef NL_FLYING_TEXT_MANAGER_H
#define NL_FLYING_TEXT_MANAGER_H

#include "nel/misc/types_nl.h"
#include "interface_pointer.h"
#include "nel/misc/rgba.h"
#include "nel/misc/vector.h"
#include <vector>
#include <map>


// ***************************************************************************
/**
 * A Manager that manage allocation and display of flying text
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2004
 */
class CFlyingTextManager
{
public:

	/// Constructor
	CFlyingTextManager();
	~CFlyingTextManager();

	/// Called by CInterfaceManager::initInGame()
	void	initInGame();
	/// Called by CInterfaceManager::releaseInGame()
	void	releaseInGame();

	/** add a flying text at a position (called during entity display). NB: may fail if no more free groups
	 *	\param offsetx: screen offsetx of the group in scene
	 */
	void	addFlyingText(void *key, const ucstring &text, const NLMISC::CVector &pos, NLMISC::CRGBA color, float scale, sint offsetX=0);
	/// release no more used flying text (called by CEntityManager at each draw)
	void	releaseNotUsedFlyingText();

	/// get the special offsetX for character (from define)
	sint	getOffsetXForCharacter() const {return _CharacterWindowOffsetX;}

private:
	struct CGroupInfo
	{
		class CGroupInScene		*GroupInScene;
		class CViewText			*ViewText;
		bool					UsedThisFrame;
		CGroupInfo()
		{
			GroupInScene= NULL;
			ViewText= NULL;
			UsedThisFrame= false;
		}
	};

	// pool of interface
	std::vector<CGroupInfo>			_InScenePool;
	// interface currently displayed
	typedef	std::map<void *, CGroupInfo>	TInSceneCurrentMap;
	TInSceneCurrentMap				_InSceneCurrent;
	// Root Interface ptr
	CInterfaceGroupPtr				_Root;
	// special decal for character
	sint							_CharacterWindowOffsetX;

	// release properly
	void	linkToInterface(CGroupInfo &gi);
	void	unlinkToInterface(CGroupInfo &gi);
};


#endif // NL_FLYING_TEXT_MANAGER_H

/* End of flying_text_manager.h */
