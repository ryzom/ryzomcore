// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2020  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
		CSongs(std::string file = std::string(), std::string title = std::string(), float length = 0.f)
		: Filename(file), Title(title), Length(length)
		{ }

		std::string	Filename;
		std::string	Title;
		float       Length;
	};

	void playSongs (const std::vector<std::string> &filenames);
	void play (sint index = -1);						// Play the song at current position, if playing, restart. If paused, resume.
	void pause ();
	void stop ();
	void previous ();
	void next ();

	/* Update the playlist. Call it in the main loop */

	void update ();

	// update currently playing song info/duration on main gui
	void updatePlayingInfo(const std::string info);
	void clearPlayingInfo();

	bool isRepeatEnabled() const;
	bool isShuffleEnabled() const;

	// Build playlist UI from songs
	void rebuildPlaylist();
	// Randomize playlist and rebuild the ui
	void shuffleAndRebuildPlaylist();
	// Update playlist active row
	void updatePlaylist(sint prevIndex = -1);
	// set/remove playlist highlight
	void updatePlaylist(uint index, bool state);

	// Update single song title/duration on _Songs and on playlist
	void updateSong(const CSongs &song);

	// Update _Songs and playlist from _SongUpdateQueue
	void updateSongs();

	// update song from worker thread
	void updateSong(const std::string filename, const std::string title, float length);

	// scan music folder and rebuild playlist
	void createPlaylistFromMusic();

private:

	// The playlist
	CSongs								_CurrentSong;
	uint								_CurrentSongIndex;	// If (!_Songs.empty()) must always be <_Songs.size()
	std::vector<CSongs>					_Songs;
	// updated info from worker thread
	std::vector<CSongs>					_SongUpdateQueue;

	// State
	enum TState { Stopped, Playing, Paused }	_State;

	TTime _PlayStart;
	TTime _PauseTime;
};

extern CMusicPlayer MusicPlayer;

// ***************************************************************************

#endif // NL_MUSIC_PLAYER_H

/* End of music_player.h */
