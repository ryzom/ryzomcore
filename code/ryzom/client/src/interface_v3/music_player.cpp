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

static const std::string MediaPlayerDirectory("music/");

CMusicPlayer MusicPlayer;

// ***************************************************************************

CMusicPlayer::CMusicPlayer ()
{
	_CurrentSong = 0;
	_State = Stopped;
}


// ***************************************************************************
void CMusicPlayer::playSongs (const std::vector<CSongs> &songs)
{
	_Songs = songs;

	// reset song index if out of bounds
	if (_CurrentSong > _Songs.size())
		_CurrentSong = 0;

	CGroupList *pList = dynamic_cast<CGroupList *>(CWidgetManager::getInstance()->getElementFromId(MP3_PLAYER_PLAYLIST_LIST));
	if (pList)
	{
		pList->clearGroups();
		pList->setDynamicDisplaySize(true);
		for (uint i=0; i < _Songs.size(); ++i)
		{
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

	// If pause, stop, else play will resume
	if (_State == Paused)
		_State = Stopped;
}


// ***************************************************************************

void CMusicPlayer::play (sint index)
{
	if(!SoundMngr)
		return;

	if (index >= 0 && index < (sint)_Songs.size())
	{
		if (_State == Paused)
			stop();

		_CurrentSong = index;
	}

	if (!_Songs.empty())
	{
		nlassert (_CurrentSong<_Songs.size());

		/* If the player is paused, resume, else, play the current song */
		if (_State == Paused)
			SoundMngr->resumeMusic();
		else
			SoundMngr->playMusic(_Songs[_CurrentSong].Filename, 0, true, false, false);

		_State = Playing;

		/* Show the song title */
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:mp3_player:screen:text"));
		if (pVT)
			pVT->setText (ucstring::makeFromUtf8(_Songs[_CurrentSong].Title));
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
}

// ***************************************************************************

void CMusicPlayer::previous ()
{
	if (!_Songs.empty())
	{
		// Point the previous song
		if (_CurrentSong == 0)
			_CurrentSong = (uint)_Songs.size()-1;
		else
			_CurrentSong--;

		play ();
	}
}

// ***************************************************************************

void CMusicPlayer::next ()
{
	if (!_Songs.empty())
	{
		_CurrentSong++;
		_CurrentSong%=_Songs.size();
		play ();
	}
}

// ***************************************************************************

void CMusicPlayer::update ()
{
	if(!SoundMngr)
		return;
	if (_State == Playing)
	{
		if (SoundMngr->isMusicEnded ())
		{
			// Point the next song
			_CurrentSong++;
			_CurrentSong%=_Songs.size();

			// End of the playlist ?
			if (_CurrentSong != 0)
			{
				// No, play the next song
				play ();
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

			bool oggSupported = false;
			bool mp3Supported = false;

			std::string message;
			for(uint i = 0; i < extensions.size(); ++i)
			{
				if (extensions[i] == "ogg")
				{
					oggSupported = true;
					message += " ogg";
				}
				else if (extensions[i] == "mp3")
				{
					mp3Supported = true;
					message += " mp3";
				}
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
				if (ext == "ogg")
				{
					if (oggSupported)
						filenames.push_back(filesToProcess[i]);
				}
				else if (ext == "mp3" || ext == "mp2" || ext == "mp1")
				{
					if (mp3Supported)
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
				// '@' in filenames are reserved for .bnp files
				// and sound system fails to open such file
				if (filenames[i].find("@") != string::npos)
				{
					nlwarning("Ignore media file containing '@' in name: '%s'", filenames[i].c_str());
					continue;
				}

				CMusicPlayer::CSongs song;
				song.Filename = filenames[i];
				SoundMngr->getMixer()->getSongTitle(filenames[i], song.Title, song.Length);
				songs.push_back (song);
			}

			MusicPlayer.playSongs(songs);
		}
		else if (Params == "previous")
			MusicPlayer.previous();
		else if (Params == "play")
			MusicPlayer.play();
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

