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



#ifndef NL_MUSIC_PLAYER_H
#define NL_MUSIC_PLAYER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "dbctrl_sheet.h"
#include "nel/gui/group_container.h"

// ***************************************************************************
/*
Music player manager.
This class handle the music player playlist actions.
*/
class CMusicPlayer
{
public:

	CMusicPlayer ();

	/* Play list commands */
	class CSongs
	{
	public:
		std::string	Filename;
		std::string	Title;
	};

	void playSongs (const std::vector<CSongs> &songs);
	void play ();											// Play the song at current position, if playing, restart. If paused, resume.
	void pause ();
	void stop ();
	void previous ();
	void next ();

	/* Update the playlist. Call it in the main loop */

	void update ();

private:

	// The playlist
	uint								_CurrentSong;	// If (!_Songs.empty()) must always be <_Songs.size()
	std::vector<CSongs>					_Songs;

	// State
	enum TState { Stopped, Playing, Paused }	_State;
};

extern CMusicPlayer MusicPlayer;

// ***************************************************************************

#endif // NL_MUSIC_PLAYER_H

/* End of music_player.h */
