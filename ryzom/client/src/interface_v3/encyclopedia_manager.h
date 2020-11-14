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

#ifndef RY_ENCYCLOPEDIA_MANAGER_H
#define RY_ENCYCLOPEDIA_MANAGER_H

#include "game_share/msg_encyclopedia.h"

// ***************************************************************************
/**
 * Encyclopedia Management (mostly interface)
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date October 2004
 */
class CEncyclopediaManager
{

public:

	static CEncyclopediaManager* getInstance()
	{
		if (!_Instance)
			_Instance = new CEncyclopediaManager;
		return _Instance;
	}

	// release memory
	static void releaseInstance();

	// to check if incoming text is arrived (for tree that have no viewtextID)
	void updateAllFrame();

	// update from network
	void update(const CEncyclopediaUpdateMsg &msg);

	void clickOnAlbum(uint32 albumName);
	void clickOnThema(uint32 themaName);

private:

	CEncyclopediaManager();
	void updateAlbum(const CEncyMsgAlbum &a);
	void updateThema(uint32 nAlbumName, const CEncyMsgThema &t);

	CEncyMsgAlbum *getAlbum(uint32 nName);

	void rebuildAlbumList();
	void rebuildAlbumPage(uint32 albumName);
	void rebuildThemaPage(uint32 themaName);

	bool isStringWaiting();

private:

	static CEncyclopediaManager *_Instance;

	std::vector<CEncyMsgAlbum>	_Albums;
	bool _CheckAllFrame;

	uint32 _AlbumNameSelected;
	uint32 _ThemaNameSelected;

	bool _Initializing;
};

#define CONT_ENCY		"ui:interface:encyclopedia"
#define LIST_ENCY_ALBUM "ui:interface:encyclopedia:content:sbtree:tree_list"
#define PAGE_ENCY_ALBUM "ui:interface:encyclopedia:content:album"
#define PAGE_ENCY_THEMA "ui:interface:encyclopedia:content:theme"
#define PAGE_ENCY_HELP	"ui:interface:encyclopedia:content:help"

#endif // RY_ENCYCLOPEDIA_MANAGER_H

/* End of encyclopedia_manager.h */
