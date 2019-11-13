// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "std3d.h"

#include "nel/3d/play_list_manager.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
void	CPlayListManager::addPlaylist(CAnimationPlaylist* plist, CChannelMixer *chanMixer)
{
	nlassert(plist);
	if(!chanMixer)
		return;
	// this do all the good things.
	_List[plist]= CNode(plist, chanMixer);
}


// ***************************************************************************
void	CPlayListManager::removePlaylist(CAnimationPlaylist* plist)
{
	nlassert(plist);
	// this do all the good things.
	_List.erase(plist);
}


// ***************************************************************************
void	CPlayListManager::animate(TGlobalAnimationTime time)
{
	TPlayListList::iterator		it;

	for(it=_List.begin();it!=_List.end();)
	{
		CNode	*node= &(it->second);
		// Test refPtrs.
		if( node->PlayList==NULL || node->ChannelMixer==NULL)
		{
			// erase it from map.
			TPlayListList::iterator		itDel= it++;
			_List.erase(itDel);
		}
		else
		{
			// animate!!
			node->PlayList->setupMixer(*node->ChannelMixer, time);
			node->ChannelMixer->eval(false);
			it++;
		}
	}
}


// ***************************************************************************
void	CPlayListManager::setup(TGlobalAnimationTime time)
{
	TPlayListList::iterator		it;

	for(it=_List.begin();it!=_List.end();)
	{
		CNode	*node= &(it->second);
		// Test refPtrs.
		if( node->PlayList==NULL || node->ChannelMixer==NULL)
		{
			// erase it from map.
			TPlayListList::iterator		itDel= it++;
			_List.erase(itDel);
		}
		else
		{
			// animate!!
			node->PlayList->setupMixer(*node->ChannelMixer, time);
			it++;
		}
	}
}

// ***************************************************************************

void CPlayListManager::deleteAll()
{
	TPlayListList::iterator	it = _List.begin();
	while (it != _List.end())
	{
		TPlayListList::iterator itDel = it;
		++it;
		CNode *node= &(itDel->second);
		delete node->ChannelMixer;
		delete node->PlayList;
		_List.erase(itDel);
	}
	_List.clear();
}


} // NL3D
