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



#ifndef NL_SOUND_MANAGER_H
#define NL_SOUND_MANAGER_H

//////////////
// INCLUDES //
//////////////
// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/config_file.h"
#include "nel/misc/fast_id_map.h"
// game_share
#include "nel/misc/entity_id.h"
// sound
#include "nel/sound/u_audio_mixer.h"
#include "nel/sound/u_listener.h"
#include "nel/misc/sheet_id.h"

extern class CSoundManager *SoundMngr;

///////////
// USING //
///////////
using std::string;
using NLSOUND::USource;

namespace NLMISC
{
	class IProgressCallback;
}

/**
 * class managing all the sounds for the client
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CSoundManager
{
public:
	typedef uint32 TSourceId;

private:
	typedef CHashMultiMap<NLMISC::CEntityId, TSourceId, NLMISC::CEntityIdHashMapTraits> TMultiMapEntityToSource;
	typedef NLMISC::CFastIdMap<TSourceId, NLSOUND::USource *> TMapIdToSource;

	/// Load the properties for this sound and aplly them.
	void loadProperties(const string &soundName, USource *source);

	/// Load the positioned sounds
	//void loadPositionedSounds();

public:

	// Loading music XFade
	enum	{LoadingMusicXFade= 1000};

public:

	/**
	 * constructor
	 * \param string& samplePath The path to directory containing the .wav sample files
	 * \param vector& sampleBanks The list of sample banks to load
	 */
	CSoundManager(NLMISC::IProgressCallback *progressCallBack = NULL);


	/// destructor
	~CSoundManager();

	/// Return the audio mixer instance pointer.
	NLSOUND::UAudioMixer *getMixer();

	TSourceId	addSource( const NLMISC::CSheetId &soundName, const NLMISC::CVector &position, bool play = true , bool loop = false,  const NLMISC::CEntityId &id = NLMISC::CEntityId::Unknown );

	/// spawn a new source to the world but sound manager don't keep any link and the sound will be automatically deleted when finnished
	bool	spawnSource (const NLMISC::CSheetId &soundName, NLSOUND::CSoundContext &context);

	/// spawn a new source to the world but sound manager don't keep any link and the sound will be automatically deleted when finnished
	bool	spawnSource( const NLMISC::CSheetId &soundName, const NLMISC::CVector &position );

	/**
	 * remove a source
	 * \param uint32 source id
	 */
	void removeSource( TSourceId sourceId );


	/**
	 * update the pos of all the sounds attached to that entity
	 * \param NLMISC::CEntityId& id of the entity
	 * \param CVector& new position
	 */
	void updateEntityPos( const NLMISC::CEntityId &id, const NLMISC::CVector &pos);

	/**
	 * update the velocity of all the sounds attached to that entity
	 * \param NLMISC::CEntityId& id of the entity
	 * \param CVector& new velocity
	 */
	void updateEntityVelocity( const NLMISC::CEntityId &id, const NLMISC::CVector &velocity);

	/**
	 * update the direction of all the sounds attached to that entity
	 * \param NLMISC::CEntityId& id of the entity
	 * \param CVector& new direction
	 */
	void updateEntityDirection( const NLMISC::CEntityId &id, const NLMISC::CVector &dir );

	/**
	 * remove an entity from the view : delete all the sounds attached to that entity
	 * \param NLMISC::CEntityId& id of the entity to remove
	 */
	void removeEntity( const NLMISC::CEntityId &id);

	/**
	 * set the listener position
	 * \param CVector & new position
	 */
	inline void setListenerPos( const NLMISC::CVector &pos)
	{
		static NLMISC::CVector oldPos(0.0f, 0.0f, 0.0f);
		if(oldPos != pos)
		{
			_AudioMixer->setListenerPos( pos );
			oldPos = pos;
		}
	}

	/**
	 * set the listener velocity
	 * \param CVector& new velocity
	 */
	inline void setListenerVelocity( const NLMISC::CVector &velocity)
	{
		static NLMISC::CVector oldVelocity(1564152.0f,1561.0f,846.0f);
		if(oldVelocity != velocity)
		{
			_AudioMixer->getListener()->setVelocity( velocity );
			oldVelocity = velocity;
		}
	}

	/**
	 * set the listener orientation
	 * \param CVector& new orientation 'front'
	 * \param CVector& new orientation 'up'
	 */
	inline void setListenerOrientation( const NLMISC::CVector &front, const NLMISC::CVector &up = NLMISC::CVector(0.0f,0.0f,1.0f) )
	{
		static NLMISC::CVector oldFront(1564152.0f,1561.0f,846.0f), oldUp(1564152.0f,1561.0f,846.0f);
		if(oldFront != front || oldUp != up)
		{
			_AudioMixer->getListener()->setOrientation( front, up );
			oldFront = front;
			oldUp = up;
		}
	}


	/** Get the gain value for the sound emited by the user entity
	 *	This value come from the UserEntitySoundLevel var in config file.
	 */
	float getUserEntitySoundLevel() {return _UserEntitySoundLevel; }

	void reset ();

	void setFilterState(uint filter, bool state);

	void playBackgroundSound ();
	void stopBackgroundSound ();

	/**
	 * set sound position (sound must exist)
	 * \param uint32 source id
	 * \param CVector& new position
	 */
	void setSoundPosition( TSourceId sourceId, const NLMISC::CVector &position);

	/**
	 * loop a sound (or stop looping)
	 * \param uint32 source id
	 * \param bool loop (true = loop)
	 */
	void loopSound( TSourceId sourceId, bool loop);

	/**
	 * play or stop a sound
	 * \param uint32 source id
	 * \param bool play (true = play, false = stop)
	 */
	void playSound( TSourceId sourceId, bool play);

	/**
	 * test whether the sepcified source is playing or not
	 * \param uint32 source id
	 * \return bool true if the source is playing
	 */
	bool isPlaying( TSourceId sourceId );

	/**
	 * select the env effect corresponding to tag
	 * \param string& tag
	 */
	inline void selectEnvEffect( const std::string &tag)
	{
		nlassert( _AudioMixer );
		_AudioMixer->selectEnvEffects( tag);
	}

	/**
	 * select the env corresponding to tag
	 * \param string& tag
	 */
	void selectEnv( const std::string &tag);

	/**
	 * set source Gain
	 * (Set the gain amount (value inside [0, 1]) to map between 0 and the nominal gain
	 * (which is getSource()->getGain()). Does nothing if getSource() is null )
	 * \param uint32 sourceId
	 * \param float new gain (0-1)
	 */
	void setSourceGain(  TSourceId sourceId, float gain);

	/**
	 * get source Gain
	 * \param uint32 sourceId
	 * \return float new gain (0-1) (-1 if source not found)
	 */
	float getSourceGain( TSourceId sourceId );

	/**
	 * set source Pitch
	 * (Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift of one octave. 0 is not a legal value.)
	 * \param uint32 sourceId
	 * \param float new Pitch (0-1)
	 */
	void setSourcePitch(  TSourceId sourceId, float gain);

	/**
	 * get source Pitch
	 * \param uint32 sourceId
	 * \return float new Pitch (0-1) (>0) (-1 if source not found)
	 */
	float getSourcePitch( TSourceId sourceId );

	/**
	 * Play all the positioned sounds which are near the given position
	 *
	 * \param pos is the position of the user
	 */
	void playPositionedSounds( const NLMISC::CVector& pos );

	void update ();

	// called at outgame time
	void updateAudioMixerOnly();

	/// Return the number of sources
	uint		getSourcesInstanceCount() const		{ if (_AudioMixer) return _AudioMixer->getSourcesInstanceCount(); else return 0; };
	/// Return the number of playing sources
	uint		getPlayingSourcesCount() const { if (_AudioMixer) return _AudioMixer->getPlayingSourcesCount(); else return 0; };
	/// Return the memory size for samples.
	uint		getLoadingSamplesSize() const	{ if (_AudioMixer) return _AudioMixer->getLoadedSampleSize(); else return 0; };

	void		switchSoundState ();

	/// Load all stuffs for a continent (must be "lesfalaises", "tryker" ... look ryzom.world for good name)
	void		loadContinent (const std::string &name, const NLMISC::CVector& pos);

	/// Draw the sounds/cluster/audio path for debugging purpose
	void		drawSounds(float camHeight);

	/// Play Music (see UAudioMixer for detail). NB: the background music system is disabled until the music is stopped
	void		playMusic(const string &fileName, uint xFadeTime= 2000, bool async= true, bool loop=true, bool forceGameMusicVolume= false);
	/// Stop Music. NB: the background music system is then reenabled
	void		stopMusic(uint xFadeTime= 2000);
	/// Pause Music
	void		pauseMusic();
	/// Resume Music
	void		resumeMusic();
	/// Is Music ended ?
	bool		isMusicEnded();
	/// set game music volume (0-1) (outgame and ingame music)
	void		setGameMusicVolume(float val);
	/// set user music volume (0-1) (mp3 player)
	void		setUserMusicVolume(float val);

	/// Set the SFX global volume (don't impact on music volume)
	void		setSFXVolume(float val);

	/// Fade in/out game volume (game music and sfx volume)
	void		fadeInGameSound(sint32 timeFadeMs);
	void		fadeOutGameSound(sint32 timeFadeMs);
	void		setupFadeSound(float sfxFade, float musicFade);

	/// Start an Event music (always async). Don't restart if the same music is currently playing
	void		playEventMusic(const string &fileName, uint xFadeTime= 2000, bool loop=true);
	/// Stop the event music played. Stop the music only if it is the same music. Set empty filename to stop any music
	void		stopEventMusic(const string &fileName, uint xFadeTime= 2000);
	/// get the eventmusic played (empty if none or ended)
	const std::string	&getEventMusicPlayed() const {return _EventMusicPlayed;}

	/**
	 * Initialize the audio mixer, load the sound banks, called by the constructors
	 * \param string& sound buffer file (.nss)
	 */
	void init(NLMISC::IProgressCallback *progressCallBack = NULL);

private:
	// attributes------------------------------
	bool	_PlaySound;

	/// Pointer on the audio mixer object
	NLSOUND::UAudioMixer		*_AudioMixer;

	/// The root effects group controller for effects volume settings by the user
	NLSOUND::UGroupController	*_GroupControllerEffects;

	/// The root effects group controller for effects fading by the game
	NLSOUND::UGroupController	*_GroupControllerEffectsGame;

	/// Pointer on the root of the environmental sounds tree (if any)
	NLSOUND::UEnvSound			*_EnvSoundRoot;

	/// The current filter state.
	NLSOUND::UAudioMixer::TBackgroundFlags	_BackgroundFlags;

	/// map attached sounds to the parent entity
	TMultiMapEntityToSource		_AttachedSources;

	/// map sound Id to sound object
	TMapIdToSource				_Sources;

	/// all the step sounds
	//CStepSounds					_StepSounds;

	/// list of positioned sounds
	std::list<TSourceId>		_PositionedSounds;

	/// Gain value for user entity sound.
	float						_UserEntitySoundLevel;

	/// Music Player and outgame music: re-enable the background music after a while
	NLMISC::TTime				_EnableBackgroundMusicAtTime;

	/// release SoundAnim system
	void		releaseSoundAnim();

	// volume
	float						_SFXVolume;
	float						_GameMusicVolume;
	float						_UserMusicVolume;
	bool						_UseGameMusicVolume;
	// if event music is possible
	bool						_EventMusicEnabled;
	// if event music is currently  playing, the name of it
	std::string					_EventMusicPlayed;
	// if the event music currently playing is looping
	bool						_EventMusicLoop;
	// the fading music volume during TP
	float						_FadeGameMusicVolume;
	// the fading game music volume that raise when an event music is playing (replacing the background music)
	float						_FadeGameMusicVolumeDueToEvent;
	float						_FadeSFXVolume;
	void						updateVolume();
	void						setGameMusicMode(bool enableBackground, bool useGameMusicVolume);
	void						fadeGameSound(bool fadeIn,sint32 timeFadeMs);
	void						updateEventAndGameMusicVolume();
};






#endif // NL_SOUND_MANAGER_H

/* End of sound_manager.h */
