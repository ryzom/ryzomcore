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
	_CurrentSong = 0;

	// If pause, stop, else play will resume
	if (_State == Paused)
		_State = Stopped;

	play ();
}


// ***************************************************************************

void CMusicPlayer::play ()
{
	if(!SoundMngr)
		return;

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
			pVT->setText (_Songs[_CurrentSong].Title);
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

class CMusicPlayerPlaySongs: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		if(!SoundMngr)
			return;

		if (Params == "play_songs")
		{
#ifdef NL_OS_WINDOWS
			// Backup the current directory
			string currentPath = CPath::getCurrentPath ();

			// Hardware mouse
			bool wasHardware = IsMouseCursorHardware ();
			InitMouseWithCursor (true);
			Driver->showCursor (true);

			if (false) //supportUnicode())
			{
			}
			else
			{
				static char szFilter[] =
					"All Supported Files\0*.mp3;*.mp2;*.mp1;*.ogg;*.m3u\0"
					"MPEG Audio Files (*.mp3;*.mp2;*.mp1)\0*.mp3;*.mp2;*.mp1\0"
					"Vorbis Files (*.ogg)\0*.ogg\0"
					"Playlist Files (*.m3u)\0*.m3u\0"
					"All Files (*.*)\0*.*\0"
					"\0";

				// Filename buffer
				char buffer[65535];
				buffer[0]=0;

				OPENFILENAME ofn;
				memset (&ofn, 0, sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = Driver ? Driver->getDisplay():NULL;
				ofn.hInstance = HInstance;
				ofn.lpstrFilter = szFilter;
				ofn.nFilterIndex = 0;
				ofn.lpstrFile = buffer;
				ofn.nMaxFile = sizeof(buffer);
				ofn.lpstrTitle = "Play songs";
				ofn.Flags = OFN_OVERWRITEPROMPT|OFN_ALLOWMULTISELECT|OFN_ENABLESIZING|OFN_EXPLORER;

				if (Driver)
					Driver->beginDialogMode();

				if (GetOpenFileName (&ofn))
				{
					// Skip the directory name
					const char *bufferPtr = buffer;

					// Multi filename ?
					string path;
					if (ofn.nFileOffset>strlen(buffer))
					{
						// Backup the path and point to the next filename
						path = buffer;
						path += "\\";
						bufferPtr+=strlen(bufferPtr)+1;
					}

					// Get selected files and playlists
					std::vector<std::string> filenames;
					std::vector<std::string> playlists;
					while (*bufferPtr)
					{
						// Concat the directory name with the filename
						if (strlwr (CFile::getExtension(bufferPtr)) == "m3u")
							playlists.push_back (path+bufferPtr);
						else
							filenames.push_back (path+bufferPtr);
						bufferPtr+=strlen(bufferPtr)+1;
					}

					// Sort songs by filename
					sort (filenames.begin(), filenames.end());

					// Add playlist
					uint i;
					for (i=0; i<playlists.size(); i++)
					{
						// Get the path of the playlist
						string basePlaylist = CFile::getPath (playlists[i]);
						FILE *file = fopen (playlists[i].c_str(), "r");
						if (file)
						{
							char line[512];
							while (fgets (line, 512, file))
							{
								// Not a comment line
								string lineStr = trim (std::string(line));
								if (lineStr[0] != '#')
									filenames.push_back (basePlaylist+lineStr);
							}
							fclose (file);
						}
					}

					// Build the songs array
					std::vector<CMusicPlayer::CSongs> songs;
					for (i=0; i<filenames.size(); i++)
					{
						CMusicPlayer::CSongs song;
						song.Filename = filenames[i];
						SoundMngr->getMixer()->getSongTitle(filenames[i], song.Title);
						songs.push_back (song);
					}

					MusicPlayer.playSongs(songs);
				}

				if (Driver)
					Driver->endDialogMode();
			}

			// Restaure mouse
			InitMouseWithCursor (wasHardware);
			Driver->showCursor (wasHardware);

			// Restaure current path
			CPath::setCurrentPath (currentPath.c_str());
#endif // NL_OS_WINDOWS
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
		}
	}
};
REGISTER_ACTION_HANDLER( CMusicPlayerPlaySongs, "music_player");

