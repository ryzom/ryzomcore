// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2020  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2020-2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "../client_cfg.h"

#include "nel/misc/thread.h"
#include "nel/misc/mutex.h"

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

CMusicPlayer MusicPlayer;
static NLMISC::CUnfairMutex MusicPlayerMutex;

// ***************************************************************************
class CMusicPlayerWorker : public NLMISC::IRunnable
{
private:
	bool _Running;
	IThread *_Thread;

	std::vector<std::string> _Files;

public:
	CMusicPlayerWorker(): _Running(false), _Thread(NULL)
	{
	}

	~CMusicPlayerWorker()
	{
		_Running = false;
		if (_Thread)
		{
			_Thread->terminate();
			delete _Thread;
			_Thread = NULL;
		}
	}

	bool isRunning() const { return _Running; }

	void run()
	{
		_Running = true;

		uint i = 0;
		while(_Running && SoundMngr && i < _Files.size())
		{
			// get copy incase _Files changes
			std::string filename(_Files[i]);

			std::string title;
			float length;

			if (SoundMngr->getMixer()->getSongTitle(filename, title, length))
			{
				MusicPlayer.updateSong(filename, title, length);
			}

			++i;
		}

		_Running = false;
		_Files.clear();
	}

	// called from GUI
	void getSongsInfo(const std::vector<std::string> &filenames)
	{
		if (_Thread)
		{
			stopThread();
		}

		_Files = filenames;

		_Thread = IThread::create(this);
		nlassert(_Thread != NULL);
		_Thread->start();
	}

	void stopThread()
	{
		_Running = false;
		if (_Thread)
		{
			_Thread->wait();
			delete _Thread;
			_Thread = NULL;
		}
	}
};

static CMusicPlayerWorker MusicPlayerWorker;

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
void CMusicPlayer::playSongs (const std::vector<std::string> &filenames)
{
	_Songs.clear();
	for (uint i=0; i<filenames.size(); i++)
	{
		_Songs.push_back(CSongs(filenames[i], CFile::getFilename(filenames[i]), 0.f));
	}

	// reset song index if out of bounds
	if (_CurrentSongIndex > _Songs.size())
		_CurrentSongIndex = 0;

	if (isShuffleEnabled())
		shuffleAndRebuildPlaylist();
	else
		rebuildPlaylist();

	// If pause, stop, else play will resume
	if (_State == Paused || _Songs.empty())
		stop();

	// get song title/duration using worker thread
	MusicPlayerWorker.getSongsInfo(filenames);
}

// ***************************************************************************
void CMusicPlayer::updatePlaylist(uint index, bool state)
{
	if (index >= _Songs.size()) return;

	std::string rowId = toString("%s:s%d:bg", MP3_PLAYER_PLAYLIST_LIST, index);
	CInterfaceElement *pIE = dynamic_cast<CInterfaceElement*>(CWidgetManager::getInstance()->getElementFromId(rowId));
	if (pIE) pIE->setActive(state);
}

void CMusicPlayer::updatePlaylist(sint prevIndex)
{
	if (prevIndex >= 0 && prevIndex < _Songs.size())
	{
		updatePlaylist(prevIndex, false);
	}

	updatePlaylist(_CurrentSongIndex, true);
}

// ***************************************************************************
// called from worker thread
void CMusicPlayer::updateSong(const std::string filename, const std::string title, float length)
{
	CAutoMutex<CUnfairMutex> mutex(MusicPlayerMutex);

	_SongUpdateQueue.push_back(CSongs(filename, title, length));
}

// ***************************************************************************
// called from GUI
void CMusicPlayer::updateSongs()
{
	CAutoMutex<CUnfairMutex> mutex(MusicPlayerMutex);
	if (!_SongUpdateQueue.empty())
	{
		for(uint i = 0; i < _SongUpdateQueue.size(); ++i)
		{
			updateSong(_SongUpdateQueue[i]);
		}
		_SongUpdateQueue.clear();
	}
}

// ***************************************************************************
void CMusicPlayer::updateSong(const CSongs &song)
{
	uint index = 0;
	while(index < _Songs.size())
	{
		if (_Songs[index].Filename == song.Filename)
		{
			_Songs[index].Title = song.Title;
			_Songs[index].Length = song.Length;
			break;
		}

		++index;
	}
	if (index == _Songs.size())
	{
		nlwarning("Unknown song file '%s'", song.Filename.c_str());
		return;
	}

	std::string rowId(toString("%s:s%d", MP3_PLAYER_PLAYLIST_LIST, index));
	CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(rowId));
	if (!pIG)
	{
		nlwarning("Playlist row '%s' not found", rowId.c_str());
		return;
	}

	CViewText *pVT;
	pVT = dynamic_cast<CViewText *>(pIG->getView(TEMPLATE_PLAYLIST_SONG_TITLE));
	if (pVT)
	{
		pVT->setHardText(song.Title);
	}
	else
	{
		nlwarning("title element '%s' not found", TEMPLATE_PLAYLIST_SONG_TITLE);
	}

	pVT = dynamic_cast<CViewText *>(pIG->getView(TEMPLATE_PLAYLIST_SONG_DURATION));
	if (pVT)
	{
		uint min = (sint32)(song.Length / 60) % 60;
		uint sec = (sint32)(song.Length) % 60;
		uint hour = song.Length / 3600;
		std::string duration(toString("%02d:%02d", min, sec));
		if (hour > 0)
			duration = toString("%02d:", hour) + duration;

		pVT->setHardText(duration);
	}
	else
	{
		nlwarning("duration element '%s' not found", TEMPLATE_PLAYLIST_SONG_DURATION);
	}
}

// ***************************************************************************
void CMusicPlayer::shuffleAndRebuildPlaylist()
{
#ifndef NL_CPP17
	std::random_shuffle(_Songs.begin(), _Songs.end());
#else
	std::shuffle(_Songs.begin(), _Songs.end(), std::default_random_engine());
#endif
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

			std::string duration("--:--");
			if (_Songs[i].Length > 0)
			{
				uint min = (sint32)(_Songs[i].Length / 60) % 60;
				uint sec = (sint32)(_Songs[i].Length) % 60;
				uint hour = _Songs[i].Length / 3600;
				duration = toString("%02d:%02d", min, sec);
				if (hour > 0)
					duration = toString("%02d:", hour) + duration;
			}

			vector< pair<string, string> > vParams;
			vParams.push_back(pair<string, string>("id", "s" + toString(i)));
			vParams.push_back(pair<string, string>("index", toString(i)));
			CInterfaceGroup *pNew = CWidgetManager::getInstance()->getParser()->createGroupInstance(TEMPLATE_PLAYLIST_SONG, pList->getId(), vParams);
			if (pNew)
			{
				CViewText *pVT = dynamic_cast<CViewText *>(pNew->getView(TEMPLATE_PLAYLIST_SONG_TITLE));
				if (pVT)
				{
					pVT->setText(_Songs[i].Title);
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

	if (_Songs.empty())
	{
		index = 0;
		createPlaylistFromMusic();
	}

	if (_Songs.empty())
	{
		stop();
		return;
	}

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
	if (_State != Stopped)
		SoundMngr->stopMusic(0);

	_State = Stopped;
	_PlayStart = 0;
	_PauseTime = 0;

	clearPlayingInfo();

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
void CMusicPlayer::updatePlayingInfo(const std::string info)
{
	CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:mp3_player:screen:text"));
	if (pVT)
	{
		pVT->setText(info);
	}
}

// ***************************************************************************
void CMusicPlayer::clearPlayingInfo()
{
	if (_Songs.empty())
	{
		updatePlayingInfo(CI18N::get("uiNoFiles"));
	}
	else
	{
		updatePlayingInfo("");
	}
}

// ***************************************************************************
void CMusicPlayer::update ()
{
	if(!SoundMngr)
	{
		if (_State != Stopped)
		{
			_State = Stopped;
			clearPlayingInfo();
		}
		return;
	}

	if (MusicPlayerWorker.isRunning() || !_SongUpdateQueue.empty())
	{
		updateSongs();
	}

	if (_State == Playing)
	{
		{
			TTime dur = (CTime::getLocalTime() - _PlayStart) / 1000;
			uint min = (dur / 60) % 60;
			uint sec = dur % 60;
			uint hour = dur / 3600;

			std::string title(toString("%02d:%02d", min, sec));
			if (hour > 0) title = toString("%02d:", hour) + title;
			title += " " + _CurrentSong.Title;
			updatePlayingInfo(title);
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
				// remove active highlight from playlist
				updatePlaylist(_CurrentSongIndex, false);

				stop();

				// restart from top on next 'play'
				_CurrentSongIndex = 0;
			}
		}
	}
}

// ***************************************************************************
static void addFromPlaylist(const std::string &playlist, const std::vector<std::string> &extensions, std::vector<std::string> &filenames)
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
			{
				useUtf8 = true;
				lineStr = trim(std::string(line + 3));
			}

			if (!useUtf8)
			{
				lineStr = NLMISC::mbcsToUtf8(line); // Attempt local codepage first
				if (lineStr.empty())
					lineStr = CUtfStringView::fromAscii(std::string(line)); // Fallback
				lineStr = trim(lineStr);
			}

			lineStr = CUtfStringView(lineStr).toUtf8(true); // Re-encode external string

			// Not a comment line
			if (lineStr[0] != '#')
			{
				std::string filename = CPath::makePathAbsolute(CFile::getPath(lineStr), basePlaylist) + CFile::getFilename(lineStr);
				std::string ext = toLowerAscii(CFile::getExtension(filename));
				if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end())
				{
					if (CFile::fileExists(filename))
						filenames.push_back(filename);
					else
						nlwarning("Ignore non-existing file '%s'", filename.c_str());
				}
				else
				{
					nlwarning("Ingnore invalid extension '%s'", filename.c_str());
				}
			}
		}
		fclose (file);
	}
}

void CMusicPlayer::createPlaylistFromMusic()
{
	std::vector<std::string> extensions;
	SoundMngr->getMixer()->getMusicExtensions(extensions);

	// no format supported
	if (extensions.empty())
	{
		// in the very unlikely scenario
		static const string message("Sound driver has no support for music.");
		CInterfaceManager::getInstance()->displaySystemInfo(message, "SYS");
		nlinfo("%s", message.c_str());
		return;
	}
	std::string newPath = CPath::makePathAbsolute(CPath::standardizePath(ClientCfg.MediaPlayerDirectory), CPath::getCurrentPath(), true);
	std::string extlist;
	join(extensions, ", ", extlist);
	extlist += ", m3u, m3u8";

	std::string msg(CI18N::get("uiMk_system6"));
	msg += ": " + newPath + " (" + extlist + ")";
	CInterfaceManager::getInstance()->displaySystemInfo(msg, "SYS");
	nlinfo("%s", msg.c_str());

	// Recursive scan for files from media directory
	vector<string> filesToProcess;
	CPath::getPathContent (newPath, true, false, true, filesToProcess);

	uint i;
	std::vector<std::string> filenames;
	std::vector<std::string> playlists;

	for (i = 0; i < filesToProcess.size(); ++i)
	{
		std::string ext = toLowerAscii(CFile::getExtension(filesToProcess[i]));
		if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end())
		{
			filenames.push_back(filesToProcess[i]);
		}
		else if (ext == "m3u" || ext == "m3u8")
		{
			playlists.push_back(filesToProcess[i]);
		}
	}

	// Add songs from playlists
	for (i = 0; i < playlists.size(); ++i)
	{
		addFromPlaylist(playlists[i], extensions, filenames);
	}

	// Sort songs by filename
	sort(filenames.begin(), filenames.end());

	playSongs(filenames);
}

// ***************************************************************************
class CMusicPlayerPlaySongs: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		if(!SoundMngr)
		{
			// Do not show warning on volume change as its restored at startup
			if (Params.find("volume") == std::string::npos)
				CInterfaceManager::getInstance()->messageBox (CI18N::get ("uiMP3SoundDisabled"));

			return;
		}

		if (Params == "play_songs")
		{
			MusicPlayer.createPlaylistFromMusic();
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

