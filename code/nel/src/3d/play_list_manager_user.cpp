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

#include "nel/3d/play_list_manager_user.h"
#include "nel/misc/hierarchical_timer.h"

using namespace NLMISC;

namespace NL3D
{


// ***************************************************************************
UPlayList	*CPlayListManagerUser::createPlayList(UAnimationSet	*animSet)
{
	if(!animSet)
		nlerror("createPlayList(): animSet==NULL");

	CPlayListUser	*pl= new CPlayListUser( safe_cast<CAnimationSetUser*>(animSet)->_AnimationSet );
	_PlayLists.insert(pl);

	_PlayListManager.addPlaylist(&pl->_PlayList, &pl->_ChannelMixer);

	return pl;
}


// ***************************************************************************
void		CPlayListManagerUser::deletePlayList(UPlayList *playList)
{
	CPlayListUser	*pl= safe_cast<CPlayListUser*>(playList);

	_PlayListManager.removePlaylist(&pl->_PlayList);
	_PlayLists.erase(pl, "deletePlayList(): bad playList");
}


// ***************************************************************************
void		CPlayListManagerUser::animate(TGlobalAnimationTime	time)
{
		H_AUTO( NL3D_Render_PlayListMgr_Animate );

	_PlayListManager.animate(time);
}


// ***************************************************************************
void		CPlayListManagerUser::setup(TGlobalAnimationTime	time)
{
	H_AUTO( NL3D_Render_PlayListMgr_Setup );

	_PlayListManager.setup(time);
}



} // NL3D
