// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2012-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_AUDIO_MIXER_USER_H
#define NL_AUDIO_MIXER_USER_H
#include <nel/misc/types_nl.h>

#include <vector>
#include <list>
#include <numeric>

#include <nel/misc/time_nl.h>
#include <nel/misc/stream.h>
#include <nel/misc/singleton.h>
#include <nel/sound/u_audio_mixer.h>
#include <nel/georges/u_form.h>

#include "driver/source.h"
#include "nel/sound/listener_user.h"
//#include "background_sound_manager.h"
#include "nel/sound/mixing_track.h"
#include "nel/sound/sound.h"
#include "nel/sound/music_channel_fader.h"
#include "nel/sound/group_controller_root.h"

// Current version is 2, Ryzom Live uses 1
// Provided to allow compatibility with old binary files
#define NLSOUND_SHEET_VERSION_BUILT 1

namespace NLLIGO {
	class CLigoConfig;
}

namespace NLSOUND {
	class CSimpleSource;
	class CEnvSoundUser;
	class CEnvEffect;
	class CSampleBankManager;
	class CSoundBank;
	class CSourceCommon;
	class CClusteredSound;
	class CBackgroundSoundManager;
	class CMusicSoundManager;
	class IReverbEffect;

/**
 * Implementation of UAudioMixer
 *
 * The logical sources (_Sources) are the sources representing all entities in the world, from
 * the client's point of view.
 * The tracks (_Tracks) are the physical sources played by the sound driver. Their number
 * is small.
 *
 * When there are more sources than tracks, the process of choosing which sources go into
 * the tracks is called "balancing". The source are auto-balanced according to the
 * argument passed to init(). The sources are also balanced when
 * - Adding a new source
 * - Removing a new source
 * - Entering/Exiting from an envsound area
 *
 * Important: The user is responsible for deleting the sources that have been allocated by
 * createSource(), before deleting the audio mixer object.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CAudioMixerUser : public UAudioMixer, public ISoundDriver::IStringMapperProvider, public NLMISC::CManualSingleton<CAudioMixerUser>
{
public:

	/// Constructor
	CAudioMixerUser();
	/// Return the audio mixer object
	static CAudioMixerUser *instance() { return getInstance(); }
	/// Destructor
	virtual ~CAudioMixerUser();

	//@{
	/// @name IStringMapperProvider implementation
	/// map a string
	NLMISC::TStringId map(const std::string &str)			{ return NLMISC::CStringMapper::map(str);}
	/// unmap a string
	const std::string &unmap(const NLMISC::TStringId &stringId)		{ return NLMISC::CStringMapper::unmap(stringId);}
	//@}


	/** Initialization
	 *
	 * In case of failure, can throw one of these ESoundDriver (Exception) objects:
	 * ESoundDriverNotFound, ESoundDriverCorrupted, ESoundDriverOldVersion, ESoundDriverUnknownVersion.
	 *
	 * The sources will be auto-balanced every "balance_period" calls to update()
	 * (set 0 for "never auto-balance")
	 * 
	 * Deprecated by initDriver/getDevices/initDevice.
	 */
	virtual void		init(uint maxTrack = 32, bool useEax = true, bool useADPCM = true, NLMISC::IProgressCallback *progressCallBack = NULL, bool autoLoadSample = false, TDriver driverType = DriverAuto, bool forceSoftware = false, bool manualRolloff = true);
	
	/// Initialize the NeL Sound Driver with given driverName.
	virtual void		initDriver(const std::string &driverName);
	/// Get the available devices on the loaded driver.
	virtual void		getDevices(std::vector<std::string> &devices);
	/// Initialize the selected device on the currently initialized driver. Leave deviceName empty to select the default device.
	virtual void		initDevice(const std::string &deviceName, const CInitInfo &initInfo, NLMISC::IProgressCallback *progressCallback = NULL);

	
	virtual void		initClusteredSound(NL3D::UScene *uscene, float minGain, float maxDistance, float portalInterpolate);
	virtual void		initClusteredSound(NL3D::CScene *scene, float minGain, float maxDistance, float portalInterpolate);

	/** Set the priority channel reserve.
	 *	Each priority channel can be assign a restrictive reserve value.
	 *	This value is used when the number free track available for playing drop
	 *	under the low water mark value (see setLowWaterMark).
	 *	The mixer count the number of playing source in each priority channel.
	 *	A priority channel can orverflow it's reserve value only if the low water
	 *	mark is not reach.
	 *	In other word, when the number of played source increase, you can control
	 *	a 'smooth' cut in priority layer. The idea is to try to keep some free track
	 *	for the HighestPri source.
	 *	By default, reserve are set for each channel to the number of available tracks.
	 */
	virtual void		setPriorityReserve(TSoundPriority priorityChannel, uint reserve);
	/** Set the Low water mark.
	 *	This value is use to mute sound source that try to play when there priority
	 *	channel is full (see setPriorityReserve).
	 *	Set a value 1 to 4 to keep some extra track available when a
	 *	HighestPri source need to play.
	 *	By default, the value is set to 0, witch mean no special treatment is done
	 *	and the mixer will mute sound with no user control at all.
	 *	Note also that the availability of a track is not guarantie if the sum of
	 *	the priority reserve (see setPriorityReserve) is grater than the number od
	 *	available tracks (witch is almos alwais the case). But this value will help
	 *	the mixer make it's best.
	 */
	virtual void		setLowWaterMark(uint value);

	virtual	void		changeMaxTrack(uint maxTrack);

	/// Resets the audio system (deletes all the sources, include envsounds)
	virtual void				reset();
	/// Disables or reenables the sound
	virtual void				enable( bool b );
	/// Load environmental effects
//	virtual void				loadEnvEffects( const char *filename );

	void						buildSampleBankList();
	bool						useAPDCM()	{	return _UseADPCM;};
	/** Load buffers. Returns the number of buffers successfully loaded.
	 * If you specify a non null notfoundfiles vector, it is filled with the names of missing files if any.
	 * You can call this method several times, to load several sound banks.
	 */
	virtual uint32				loadSampleBank(bool async, const std::string &name, std::vector<std::string> *notfoundfiles=NULL );
	/** Unload buffers.
	*/
	virtual bool				unloadSampleBank( const std::string &name);
	virtual void				reloadSampleBanks(bool async);
	virtual uint32				getLoadedSampleSize();
	virtual void				getLoadedSampleBankInfo(std::vector<std::pair<std::string, uint> > &result);



	/// Load sounds. Returns the number of sounds successfully loaded.
//	virtual void				loadSoundBank( const std::string &path );


	// Load environment sounds ; treeRoot can be null if you don't want an access to the envsounds
//	virtual	void				loadEnvSounds( const char *filename, UEnvSound **treeRoot=NULL );
	/// Get a TSoundId from a name (returns NULL if not found)
	virtual TSoundId			getSoundId( const NLMISC::TStringId &name );

	/// Gets the group controller for the given group tree path with separator '/', if it doesn't exist yet it will be created.
	/// Examples: "music", "effects", "dialog", "music/background", "music/loading", "music/player", etcetera
	virtual UGroupController *getGroupController(const std::string &path);

	/** Add a logical sound source (returns NULL if name not found).
	 * If spawn is true, the source will auto-delete after playing. If so, the return USource* pointer
	 * is valid only before the time when calling play() plus the duration of the sound. You can
	 * pass a callback function that will be called (if not NULL) just before deleting the spawned
	 * source.
	 */
	virtual USource				*createSource( const NLMISC::TStringId &name, bool spawn=false, TSpawnEndCallback cb=NULL, void *cbUserParam = NULL, NL3D::CCluster *cluster = 0, CSoundContext *context = 0, UGroupController *groupController = NULL);
	/// Add a logical sound source (by sound id). To remove a source, just delete it. See createSource(const char*)
	virtual USource				*createSource( TSoundId id, bool spawn=false, TSpawnEndCallback cb=NULL, void *cbUserParam = NULL, NL3D::CCluster *cluster = 0, CSoundContext *context = 0, UGroupController *groupController = NULL);
	/// Add a source which was created by an EnvSound
	void						addSource( CSourceCommon *source );
	/** Delete a logical sound source. If you don't call it, the source will be auto-deleted
	 * when deleting the audio mixer object
	 */
	virtual void				removeSource( CSourceCommon *source );

	/// Put source into a track
//	void						giveTrack( CSimpleSource *source );
	/// Release track
//	void						releaseTrack( CSimpleSource *source );

	/** Use this method to set the listener position instead of using getListener->setPos();
	 * It's because we have to update the background sounds in this case.
	 */
	virtual void				setListenerPos (const NLMISC::CVector &pos);

	/// Return the listener interface
	virtual UListener			*getListener()	{ return &_Listener; }


	/// Choose the environmental effect(s) corresponding to tag
	virtual void				selectEnvEffects( const std::string &tag );
	/// Update audio mixer (call evenly)
	virtual void				update();


	/// Return the names of the sounds (call this method after loadSounds())
	virtual void				getSoundNames( std::vector<NLMISC::TStringId> &names ) const;
	/// Return the number of mixing tracks (voices)
	virtual uint				getPolyphony() const { return (uint)_Tracks.size(); }
	/// Return the number of sources instance.
	virtual uint				getSourcesInstanceCount() const { return (uint)_Sources.size(); }
	/// Return the number of playing sources (slow)
	virtual uint				getPlayingSourcesCount() const;
	uint 						countPlayingSimpleSources() const; // debug
	uint						countSimpleSources() const; // debug
	/// Return the number of available tracks
	virtual uint				getAvailableTracksCount() const;
	/// Return the number of used tracks
	virtual uint				getUsedTracksCount() const;
	/// Return the number muted playing source
	virtual uint				getMutedPlayingSourcesCount() const		{ return _PlayingSourcesMuted; }

	/// Return a string showing the playing sources (slow)
	virtual std::string			getSourcesStats() const;


	/// Take a listener's move into account
	void						applyListenerMove( const NLMISC::CVector& listenerpos );
	/// Return the root of the envsounds tree
//	CEnvSoundUser				*getEnvSounds()							{ return _EnvSounds; }
	/// Return the listen pos vector
	const NLMISC::CVector&		getListenPosVector() const				{ return _ListenPosition; }
	/** Same as removeSource() but does not delete the object (e.g. when not allocated by new,
	 * as the CAmbiantSource channels)
	 */
//	void						removeMySource( USource *source );
	/// Add ambiant sound pointer for later deletion
//	void						addAmbiantSound( CSound *sound )		{ _AmbSounds.insert( sound ); }
	// Allow to load sound files (nss) when the corresponding wave file is missing (see CSound)
	//static void					allowMissingWave( bool b )				{ CSound::allowMissingWave( b ); }

	/// Set the global path to the sample banks
	virtual void				setSamplePath(const std::string& path);
	virtual void				setSamplePaths(const std::string &wavAssetPath, const std::string &bankBuildPath);
	virtual void				setPackedSheetOption(const std::string &path, bool update);
	std::string					&getPackedSheetPath()						{return _PackedSheetPath; }
	bool						getPackedSheetUpdate()						{return _UpdatePackedSheet; }


	CBackgroundSoundManager		*getBackgroundSoundManager()				{ return _BackgroundSoundManager; }
	/// Write profiling information about the mixer to the output stream.
	virtual void				writeProfile(std::string& out);

	virtual void				setBackgroundFlagName(uint flagIndex, const std::string &flagName);
	virtual void				setBackgroundFlagShortName(uint flagIndex, const std::string &flagShortName);
	virtual const std::string	&getBackgroundFlagName(uint flagIndex);
	virtual const std::string	&getBackgroundFlagShortName(uint flagIndex);
//	virtual void				loadBackgroundSoundFromRegion (const NLLIGO::CPrimRegion &region);
//	virtual void				loadBackgroundEffectsFromRegion (const NLLIGO::CPrimRegion &region);
//	virtual void				loadBackgroundSamplesFromRegion (const NLLIGO::CPrimRegion &region);
	virtual void				loadBackgroundAudioFromPrimitives(const NLLIGO::IPrimitive &audioRoot);
	virtual void				loadBackgroundSound (const std::string &continent, NLLIGO::CLigoConfig &config);
	virtual void				playBackgroundSound ();
	virtual void				stopBackgroundSound ();

	CClusteredSound				*getClusteredSound()	{ return _ClusteredSound; }

//	virtual void				setBackgroundSoundDayNightRatio (float ratio) { CBackgroundSoundManager::setDayNightRatio(ratio); }

	/// Return the sound driver.
	ISoundDriver				*getSoundDriver();
	inline CSoundBank			*getSoundBank() { return _SoundBank; }
	inline CSampleBankManager	*getSampleBankManager() { return _SampleBankManager; }

	void						registerBufferAssoc(CSound *sound, IBuffer *buffer);
	void						unregisterBufferAssoc(CSound *sound, IBuffer *buffer);

	void						bufferUnloaded(IBuffer *buffer);

	void						setBackgroundFlags(const TBackgroundFlags &backgroundFlags);
	void						setBackgroundFilterFades(const TBackgroundFilterFades &backgroundFilterFades);
	const TBackgroundFlags		&getBackgroundFlags();
	const TBackgroundFilterFades &getBackgroundFilterFades();


//	bool						setPlaying(CSimpleSource *source);
//	void						unsetPlaying(CSimpleSource *source);

	/// Get a free track for a CSimpleSource, or steal one if needed.
	CTrack *getFreeTrack(CSourceCommon *source);
	/// Free a track.
	void freeTrack(CTrack *track);

	///// Get a free track without a source! Steal one if if you want when no tracks are available! Used by music channel, etc.
	//CTrack *getFreeTrackWithoutSource(bool steal);
	///// Free a track that has no source. Used by music channel, etc.
	//void freeTrackWithoutSource(CTrack *track);

	void						incPlayingSource()	{ ++_PlayingSources; };
	void						decPlayingSource()	{ --_PlayingSources; };
	void						incPlayingSourceMuted()	{ ++_PlayingSourcesMuted; };
	void						decPlayingSourceMuted()	{ --_PlayingSourcesMuted; };

	void		setUserVar(NLMISC::TStringId varName, float value);
	float		getUserVar(NLMISC::TStringId varName);

	// music
	virtual bool	playMusic(const std::string &fileName, uint xFadeTime= 0, bool async= true, bool loop=true);
	virtual void	stopMusic(uint xFadeTime= 0);
	virtual void	pauseMusic();
	virtual void	resumeMusic();
	virtual bool	isMusicEnded();
	virtual void	setMusicVolume(float gain);
	virtual float	getMusicLength();
	virtual bool	getSongTitle(const std::string &filename, std::string &result, float &length);
	virtual void	enableBackgroundMusic(bool enable);
	virtual void	enableBackgroundMusicTimeConstraint(bool enable);
	CMusicSoundManager *getBackgroundMusicManager() const {return _BackgroundMusicManager;}
	// Event music
	virtual bool	playEventMusic(const std::string &fileName, uint xFadeTime= 0, bool async= true, bool loop=true);
	virtual void	stopEventMusic(uint xFadeTime= 0);
	virtual void	setEventMusicVolume(float gain);
	virtual bool	isEventMusicEnded();
	/// Get audio/container extensions that are currently supported by nel or the used driver implementation.
	virtual void getMusicExtensions(std::vector<std::string> &extensions);

	inline IReverbEffect *getReverbEffect() { return _ReverbEffect; }
	inline bool useEnvironmentEffects() const { return _UseEax; }

	//@{
	//\name Reverb environment functions
	/// Add a reverb environment.
	void addEnvironment(const std::string &name, const IReverbEffect::CEnvironment &environment);
	/// Set the current reverb environment.
	void setEnvironment(NLMISC::TStringId environmentName, float roomSize);
	/// Set the current reverb environment.
	inline void setEnvironment(const std::string &environmentName, float roomSize) { setEnvironment(NLMISC::CStringMapper::map(environmentName), roomSize); }
	/// Get a reverb environment
	const IReverbEffect::CEnvironment & getEnvironment(NLMISC::TStringId environmentName);
	/// Get a reverb environment
	inline const IReverbEffect::CEnvironment & getEnvironment(const std::string &environmentName) { return getEnvironment(NLMISC::CStringMapper::map(environmentName)); }
	//@}

private:
	enum	TMusicChannel
	{
		GeneralMusicChannel= 0,
		EventMusicChannel= 1
	};
	bool	playMusicChannel(TMusicChannel chan, const std::string &fileName, uint xFadeTime, bool async, bool loop);


public:
	/// Interface for registering object in the mixer update.
	class IMixerUpdate : public NLMISC::CDbgRefCount<IMixerUpdate>
	{
	public:
		virtual void onUpdate() =0;
		virtual ~IMixerUpdate()
		{
			//nldebug("Destroying IMixerUpdate %p", this);
		}
	};

	/// Register an object in the update list.
	void						registerUpdate(IMixerUpdate *pmixerUpdate);
	/// Unregister an object from the update list.
	void						unregisterUpdate(IMixerUpdate *pmixerUpdate);

	/// Intergace for registering object in the mixer eventlist.
	class IMixerEvent : public NLMISC::CDbgRefCount<IMixerEvent>
	{
	public:
		virtual void onEvent() =0;
		virtual ~IMixerEvent()
		{
//			nldebug("Destroying IMixerEvent %p", this);
		}

	};

	/// Add an event in the future.
	void						addEvent(IMixerEvent *pmixerEvent, const NLMISC::TTime &date);
	/// Remove any event programmed for this object.
	void						removeEvents(IMixerEvent *pmixerEvent);

	/// Add a source for play as possible (for non discadable sound)
	void						addSourceWaitingForPlay(CSourceCommon *source);
	void						removeSourceWaitingForPlay(CSourceCommon *source);

	/// Read all user controled var sheets
	void						initUserVar();
	void						addUserControledSource(CSourceCommon *source, NLMISC::TStringId varName);
	void						removeUserControledSource(CSourceCommon *source, NLMISC::TStringId varName);


	virtual void startDriverBench();
	virtual void endDriverBench();
	virtual void displayDriverBench(NLMISC::CLog *log);

private:

	// utility function for automatic sample bank loading.
	bool tryToLoadSampleBank(const std::string &sampleName);

	typedef CHashSet<IMixerUpdate*, THashPtr<IMixerUpdate*> >						TMixerUpdateContainer;
	typedef CHashMap<IBuffer*, std::vector<class CSound*>, THashPtr<IBuffer*> >	TBufferToSourceContainer;
//	typedef std::multimap<NLMISC::TTime, IMixerEvent*>									TTimedEventContainer;
	typedef std::multimap<NLMISC::TTime, NLMISC::CDbgPtr<IMixerEvent> >					TTimedEventContainer;
	typedef std::multimap<IMixerEvent*, TTimedEventContainer::iterator>					TEventContainer;

	/// Identify the parameter controled by user var.
	enum TControledParamId
	{
		gain_control,
		pitch_control,
		nb_control,
		bad_control
	};

	struct CControledSources
	{
		/// The user var name
		NLMISC::TStringId				Name;
		/// Witch parameter to control
		TControledParamId				ParamId;
		/// The controled sounds names.
		std::vector<NLMISC::TStringId>	SoundNames;
		/// Current parameter value
		float							Value;
		/// All the sources controled by this variable
		std::set<CSourceCommon*>		Sources;

		void serial (NLMISC::IStream &s);
	};

	friend struct CControledSources;
	friend class CUserVarSerializer;

protected:
	/// List of object to update.
	TMixerUpdateContainer							_UpdateList;
	/// List of update to add or remove (bool param of the pair).
	std::vector<std::pair<IMixerUpdate*, bool> >	_UpdateEventList;
	/// List of event ordered by time.
	TTimedEventContainer							_EventList;
	/// List of event ordered by event ptr with there respective multimap iterator.
	TEventContainer									_Events;
	/// List of update for the event list.
	std::list<std::pair<NLMISC::TTime, IMixerEvent*> >	_EventListUpdate;
	/// Returns nb available tracks (or NULL)
	void						getFreeTracks( uint nb, CTrack **tracks );
	/// Fill a vector of position and mute flag for all playing sound source.
	virtual void				getPlayingSoundsPos(bool virtualPos, std::vector<std::pair<bool, NLMISC::CVector> > &pos);

	typedef CHashMap<NLMISC::TStringId, CControledSources, NLMISC::CStringIdHashMapTraits>	TUserVarControlsContainer;
	/// Container for all user controler and currently controled playing source
	TUserVarControlsContainer	_UserVarControls;

private:
	/// flag for automatic sample bank loading.
	bool						_AutoLoadSample;
	/// flag for usage of ADPCM mixing
	bool						_UseADPCM;
	/// flag for usage of eax
	bool						_UseEax;

	/// The vector of curently free tracks.
	std::vector<CTrack *>		_FreeTracks;

	/// The list of non discardable sound to play as soon as possible in order of arrival.
	std::list<CSourceCommon *>	_SourceWaitingForPlay;

	/// Table of track reserve for each priority
	uint32						_PriorityReserve[NbSoundPriorities];
	/// Table of current playing source for each priority
	uint32						_ReserveUsage[NbSoundPriorities];
	/// Low water mark. After this number of free voice is reach, reserve can't be overloaded.
	uint32						_LowWaterMark;

	/// The sound driver instance
	ISoundDriver				*_SoundDriver;

	/// The sound bank used, contains georges sound sheets
	CSoundBank					*_SoundBank;
	/// Sample bank manager, contains loaded sample banks
	CSampleBankManager			*_SampleBankManager;

	/// The number of music fading channels
	static const uint			_NbMusicChannelFaders = 2;

	/// Music channel faders
	CMusicChannelFader			_MusicChannelFaders[_NbMusicChannelFaders];

	/// Intance of the background sound manager.
	CBackgroundSoundManager		*_BackgroundSoundManager;
	/// Array of filter name
	std::string					_BackgroundFilterNames[TBackgroundFlags::NB_BACKGROUND_FLAGS];
	/// Array of filter short names
	std::string					_BackgroundFilterShortNames[TBackgroundFlags::NB_BACKGROUND_FLAGS];

	/// Instance of the clustered sound system
	CClusteredSound				*_ClusteredSound;

	/// The listener instance
	CListenerUser				_Listener;
	
	/// The reverb effect
	IReverbEffect				*_ReverbEffect;
	/// The default reverb environment
	IReverbEffect::CEnvironment	_DefaultEnvironment;
	/// The default reverb room size
	float						_DefaultRoomSize;
	/// Available reverb environments
	typedef std::map<NLMISC::TStringId, IReverbEffect::CEnvironment> TEnvironments;
	TEnvironments				_Environments;
	
	/// Listener position vector
	NLMISC::CVector				_ListenPosition;

	/// The path to the sample banks. This should be specified in the config file.
	std::string					_SampleBankPath;
	std::string					_SampleWavPath;
	/// The path to the packed sheet files
	std::string					_PackedSheetPath;
	/// A flag to update or not the packed sheet
	bool						_UpdatePackedSheet;

	/// Assoc between buffer and source. Used when buffers are unloaded.
	TBufferToSourceContainer	_BufferToSources;

	// For debug purpose only (not called)
	void		debugLogEvent(const char *reason);

	// Instance of the background music manager
	CMusicSoundManager			*_BackgroundMusicManager;

	/// Group controller
	CGroupControllerRoot		_GroupController;

public:
	struct TSampleBankHeader
	{
		enum
		{
			// Mind to increment the version number each time the format change
			sample_bank_header_version = 7,
		};
		uint32						Version;
		std::vector<std::string>	Name;
		std::vector<uint32>			NbSample;
		std::vector<uint32>			Freq;
		std::vector<uint32>			OffsetMono16;
		std::vector<uint32>			OffsetAdpcm;
		std::vector<uint32>			SizeMono16;
		std::vector<uint32>			SizeAdpcm;

		TSampleBankHeader()
		{
			Version = sample_bank_header_version;
		}

		void addSample(const std::string &name, uint32 frequency, uint32 nbSample, uint32 sizeMono16, uint32 sizeAdpcm)
		{
			Name.push_back(name);
			Freq.push_back(frequency);
			NbSample.push_back(nbSample);
			uint32 off16;
			uint32 offAdpcm;
			off16 = std::accumulate(SizeMono16.begin(), SizeMono16.end(), 0);
			off16 = std::accumulate(SizeAdpcm.begin(), SizeAdpcm.end(), off16);
			OffsetMono16.push_back(off16);
			SizeMono16.push_back(sizeMono16);
			offAdpcm = std::accumulate(SizeMono16.begin(), SizeMono16.end(), 0);
			offAdpcm = std::accumulate(SizeAdpcm.begin(), SizeAdpcm.end(), offAdpcm);
			OffsetAdpcm.push_back(offAdpcm);
			SizeAdpcm.push_back(sizeAdpcm);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialCheck(Version);
			s.serialCont(Name);
			s.serialCont(Freq);
			s.serialCont(NbSample);
			s.serialCont(OffsetMono16);
			s.serialCont(OffsetAdpcm);
			s.serialCont(SizeMono16);
			s.serialCont(SizeAdpcm);
		}
	};

	/// Extension for sample bank list file
//	static const std::string	SampleBankListExt;

	/// All Logical sources
	TSourceContainer		_Sources;

	/// Number of source currently playing
	uint32					_PlayingSources;
	/// Number of source doing muted play
	uint32					_PlayingSourcesMuted;

public: // Temp (EDIT)
	/// Physical sources array
	std::vector<CTrack *>		_Tracks;
	/// Flag set in destructor
	bool						_Leaving;

	NLMISC::TTicks				_StartTime;

	uint32						curTime() { return (uint32) (NLMISC::CTime::getLocalTime() - _StartTime); }


#define NL_PROFILE_MIXER 1
#if NL_PROFILE_MIXER
public:

    double _UpdateTime;
    double _CreateTime;
	uint32 _UpdateCount;
	uint32 _CreateCount;
#endif

	friend struct displaySoundInfoClass;

};


/// Return the priority cstring (debug info)
const char *getPriorityStr( TSoundPriority p );


} // NLSOUND


#endif // NL_AUDIO_MIXER_USER_H

/* End of audio_mixer_user.h */
