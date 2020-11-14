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



#include "stdpch.h"

#include "music_player.h"
#include "nel/gui/action_handler.h"
#include "../input.h"
#include "../sound_manager.h"
#include "interface_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

#ifdef NL_OS_WINDOWS
	extern HINSTANCE HInstance;
#endif

extern UDriver	*Driver;

// xml element ids
#define MP3_PLAYER_PLAYLIST_LIST "ui:interface:playlist:content:songs:list"
#define TEMPLATE_PLAYLIST_SONG "playlist_song"
#define TEMPLATE_PLAYLIST_SONG_TITLE "title"
#define TEMPLATE_PLAYLIST_SONG_DURATION "duration"
// ui state
#define MP3_SAVE_SHUFFLE "UI:SAVE:MP3_SHUFFLE"
#define MP3_SAVE_REPEAT "UI:SAVE:MP3_REPEAT"

static const std::string MediaPlayerDirectory("music/");

CMusicPlayer MusicPlayer;

// ***************************************************************************

CMusicPlayer::CMusicPlayer ()
{
	_CurrentSongIndex = 0;
	_State = Stopped;
	_PlayStart = 0;
	_PauseTime = 0;
}

bool CMusicPlayer::isRepeatEnabled() const
{
	return (NLGUI::CDBManager::getInstance()->getDbProp(MP3_SAVE_REPEAT)->getValue32() == 1);
}

bool CMusicPlayer::isShuffleEnabled() const
{
	return (NLGUI::CDBManager::getInstance()->getDbProp(MP3_SAVE_SHUFFLE)->getValue32() == 1);
}


// ***************************************************************************
void CMusicPlayer::playSongs (const std::vector<CSongs> &songs)
{
	_Songs = songs;

	// reset song index if out of bounds
	if (_CurrentSongIndex > _Songs.size())
		_CurrentSongIndex = 0;

	if (isShuffleEnabled())
		shuffleAndRebuildPlaylist();
	else
		rebuildPlaylist();

	// If pause, stop, else play will resume
	if (_State == Paused)
		_State = Stopped;
}

// ***************************************************************************
void CMusicPlayer::updatePlaylist(sint prevIndex)
{
	CInterfaceElement  *pIE;
	std::string rowId;

	if (prevIndex >= 0 && prevIndex < _Songs.size())
	{
		rowId = toString("%s:s%d:bg", MP3_PLAYER_PLAYLIST_LIST, prevIndex);
		pIE = dynamic_cast<CInterfaceElement*>(CWidgetManager::getInstance()->getElementFromId(rowId));
		if (pIE) pIE->setActive(false);
	}

	rowId = toString("%s:s%d:bg", MP3_PLAYER_PLAYLIST_LIST, _CurrentSongIndex);
	pIE = dynamic_cast<CInterfaceElement*>(CWidgetManager::getInstance()->getElementFromId(rowId));
	if (pIE) pIE->setActive(true);
}

// ***************************************************************************
void CMusicPlayer::shuffleAndRebuildPlaylist()
{
	std::random_shuffle(_Songs.begin(), _Songs.end());
	rebuildPlaylist();
}

// ***************************************************************************
void CMusicPlayer::rebuildPlaylist()
{
	CGroupList *pList = dynamic_cast<CGroupList *>(CWidgetManager::getInstance()->getElementFromId(MP3_PLAYER_PLAYLIST_LIST));
	if (pList)
	{
		pList->clearGroups();
		pList->setDynamicDisplaySize(true);
		bool found = _CurrentSong.Filename.empty();
		for (uint i=0; i < _Songs.size(); ++i)
		{
			if (!found && _CurrentSong.Filename == _Songs[i].Filename)
			{
				found = true;
				_CurrentSongIndex = i;
			}

			uint min = (sint32)(_Songs[i].Length / 60) % 60;
			uint sec = (sint32)(_Songs[i].Length) % 60;
			uint hour = _Songs[i].Length / 3600;
			std::string duration(toString("%02d:%02d", min, sec));
			if (hour > 0)
				duration = toString("%02d:", hour) + duration;

			vector< pair<string, string> > vParams;
			vParams.push_back(pair<string, string>("id", "s" + toString(i)));
			vParams.push_back(pair<string, string>("index", toString(i)));
			CInterfaceGroup *pNew = CWidgetManager::getInstance()->getParser()->createGroupInstance(TEMPLATE_PLAYLIST_SONG, pList->getId(), vParams);
			if (pNew)
			{
				CViewText *pVT = dynamic_cast<CViewText *>(pNew->getView(TEMPLATE_PLAYLIST_SONG_TITLE));
				if (pVT)
				{
					ucstring title;
					title.fromUtf8(_Songs[i].Title);
					pVT->setText(title);
				}

				pVT = dynamic_cast<CViewText *>(pNew->getView(TEMPLATE_PLAYLIST_SONG_DURATION));
				if (pVT)
				{
					pVT->setText(duration);
				}

				pNew->setParent(pList);
				pList->addChild(pNew);
			}
		}
		pList->invalidateCoords();
	}

	updatePlaylist();
}


// ***************************************************************************

void CMusicPlayer::play (sint index)
{
	if(!SoundMngr)
		return;


	sint prevSongIndex = _CurrentSongIndex;

	if (index >= 0 && index < (sint)_Songs.size())
	{
		if (_State == Paused)
		{
			stop();
		}

		_CurrentSongIndex = index;
		_PauseTime = 0;
	}

	if (!_Songs.empty())
	{
		nlassert (_CurrentSongIndex<_Songs.size());

		/* If the player is paused, resume, else, play the current song */
		if (_State == Paused)
		{
			SoundMngr->resumeMusic();
		}
		else
		{
			SoundMngr->playMusic(_Songs[_CurrentSongIndex].Filename, 0, true, false, false);
			_PauseTime = 0;
		}

		_State = Playing;
		_PlayStart = CTime::getLocalTime() - _PauseTime;

		_CurrentSong = _Songs[_CurrentSongIndex];

		updatePlaylist(prevSongIndex);

		NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:MP3_PLAYING")->setValueBool(true);
	}
}

// ***************************************************************************

void CMusicPlayer::pause ()
{
	if(!SoundMngr)
		return;

	// pause the music only if we are really playing (else risk to pause a background music!)
	if(_State==Playing)
	{
		SoundMngr->pauseMusic();
		_State = Paused;

		if (_PlayStart > 0)
			_PauseTime = CTime::getLocalTime() - _PlayStart;

		NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:MP3_PLAYING")->setValueBool(false);
	}
}

// ***************************************************************************

void CMusicPlayer::stop ()
{
	if(!SoundMngr)
		return;

	// stop the music only if we are really playing (else risk to stop a background music!)
	SoundMngr->stopMusic(0);
	_State = Stopped;
	_PlayStart = 0;
	_PauseTime = 0;

	NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:MP3_PLAYING")->setValueBool(false);
}

// ***************************************************************************

void CMusicPlayer::previous ()
{
	if (!_Songs.empty())
	{
		// Point the previous song
		sint index;
		if (_CurrentSongIndex == 0)
			index = (uint)_Songs.size()-1;
		else
			index = _CurrentSongIndex-1;

		play(index);
	}
}

// ***************************************************************************

void CMusicPlayer::next ()
{
	if (!_Songs.empty())
	{
		sint index = _CurrentSongIndex+1;
		if (index == _Songs.size())
			index = 0;

		play(index);
	}
}

// ***************************************************************************

void CMusicPlayer::update ()
{
	if(!SoundMngr)
		return;
	if (_State == Playing)
	{
		CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:mp3_player:screen:text"));
		if (pVT)
		{
			TTime dur = (CTime::getLocalTime() - _PlayStart) / 1000;
			uint min = (dur / 60) % 60;
			uint sec = dur % 60;
			uint hour = dur / 3600;

			std::string title(toString("%02d:%02d", min, sec));
			if (hour > 0) title = toString("%02d:", hour) + title;
			title += " " + _CurrentSong.Title;
			pVT->setText(ucstring::makeFromUtf8(title));
		}

		if (SoundMngr->isMusicEnded ())
		{
			// select next song from playlist
			sint index = _CurrentSongIndex + 1;
			if (isRepeatEnabled() || index < _Songs.size())
			{
				if (index == _Songs.size())
				{
					index = 0;

					if (isShuffleEnabled())
						shuffleAndRebuildPlaylist();
				}

				play(index);
			}
			else
			{
				SoundMngr->stopMusic(0);
				_State = Stopped;
			}
		}
	}
}

// ***************************************************************************
static void addFromPlaylist(const std::string &playlist, std::vector<std::string> &filenames)
{
	static uint8 utf8Header[] = { 0xefu, 0xbbu, 0xbfu };

	// Add playlist
	uint i;
	// Get the path of the playlist
	string basePlaylist = CFile::getPath (playlist);
	FILE *file = nlfopen (playlist, "r");

	bool useUtf8 = CFile::getExtension(playlist) == "m3u8";
	if (file)
	{
		char line[512];
		while (fgets (line, 512, file))
		{
			string lineStr = trim(std::string(line));

			// id a UTF-8 BOM header is present, parse as UTF-8
			if (!useUtf8 && lineStr.length() >= 3 && memcmp(line, utf8Header, 3) == 0)
				useUtf8 = true;

			if (!useUtf8)
			{
				lineStr = ucstring(line).toUtf8();
				lineStr = trim(lineStr);
			}

			// Not a comment line
			if (lineStr[0] != '#')
			{
				std::string filepath = CFile::getPath(lineStr);
				std::string filename = CFile::getFilename(lineStr);
				filenames.push_back (CPath::makePathAbsolute(filepath, basePlaylist)+filename);
			}
		}
		fclose (file);
	}
}

// ***************************************************************************
class CMusicPlayerPlaySongs: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		if(!SoundMngr)
			return;

		if (Params == "play_songs")
		{
			std::vector<std::string> extensions;
			SoundMngr->getMixer()->getMusicExtensions(extensions);

			// no format supported
			if (extensions.empty()) return;

			std::string message;
			for(uint i = 0; i < extensions.size(); ++i)
			{
				message += " " + extensions[i];
			}
			message += " m3u m3u8";
			nlinfo("Media player supports: '%s'", message.substr(1).c_str());

			// Recursive scan for files from media directory
			vector<string> filesToProcess;
			string newPath = CPath::standardizePath(MediaPlayerDirectory);
			CPath::getPathContent (newPath, true, false, true, filesToProcess);

			uint i;
			std::vector<std::string> filenames;
			std::vector<std::string> playlists;

			for (i = 0; i < filesToProcess.size(); ++i)
			{
				std::string ext = toLower(CFile::getExtension(filesToProcess[i]));
				if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end())
				{
					filenames.push_back(filesToProcess[i]);
				}
				else if (ext == "m3u" || ext == "m3u8")
				{
					playlists.push_back(filesToProcess[i]);
				}
			}

			// Sort songs by filename
			sort (filenames.begin(), filenames.end());

			// Add songs from playlists
			for (i = 0; i < playlists.size(); ++i)
			{
				addFromPlaylist(playlists[i], filenames);
			}

			// Build the songs array
			std::vector<CMusicPlayer::CSongs> songs;
			for (i=0; i<filenames.size(); i++)
			{
				if (!CFile::fileExists(filenames[i])) {
					nlwarning("Ignore non-existing file '%s'", filenames[i].c_str());
					continue;
				}

				CMusicPlayer::CSongs song;
				song.Filename = filenames[i];
				// TODO: cache the result for next refresh
				SoundMngr->getMixer()->getSongTitle(filenames[i], song.Title, song.Length);
				if (song.Length > 0)
					songs.push_back (song);
			}

			MusicPlayer.playSongs(songs);
		}
		else if (Params == "update_playlist")
		{
			if (MusicPlayer.isShuffleEnabled())
				MusicPlayer.shuffleAndRebuildPlaylist();

			MusicPlayer.rebuildPlaylist();
		}
		else if (Params == "previous")
			MusicPlayer.previous();
		else if (Params == "play")
			MusicPlayer.play();
		else if (Params == "stop")
			MusicPlayer.stop();
		else if (Params == "pause")
			MusicPlayer.pause();
		else if (Params == "next")
			MusicPlayer.next();
		else
		{
			string volume = getParam(Params, "volume");
			if (!volume.empty())
			{
				CInterfaceExprValue result;
				if (CInterfaceExpr::eval (volume, result))
				{
					if (result.toDouble ())
					{
						float value = (float)result.getDouble() / 255.f;
						clamp (value, 0, 1);
						SoundMngr->setUserMusicVolume (value);
					}
				}
			}

			string song = getParam(Params, "song");
			if (!song.empty())
			{
				sint index=0;
				fromString(song, index);
				MusicPlayer.play(index);
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CMusicPlayerPlaySongs, "music_player");

