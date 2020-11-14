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

#ifndef NL_U_AUDIO_MIXER_H
#define NL_U_AUDIO_MIXER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/sheet_id.h"
#include "nel/sound/u_source.h"
#include "nel/sound/u_group_controller.h"
#include "nel/ligo/primitive.h"
#include <vector>

namespace NL3D
{
	class CCluster;
	class UScene;
	class CScene;
}

namespace NLMISC
{
	class IProgressCallback;
}

namespace NLLIGO
{
	class CLigoConfig;
}

namespace NLSOUND {


class UEnvSound;
class UListener;


//const uint32 AUTOBALANCE_DEFAULT_PERIOD = 20;


/**
 * Game interface for audio
 *
 * The logical sources represent all entities in the world, from the client's point of view.
 * Their number can be higher than the number of simultaneous playable sound on the soundcard.
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
class UAudioMixer
{
public:

	/// Driver Creation Choice
	enum	TDriver
	{
		DriverAuto = 0,
		DriverFMod,
		DriverOpenAl,
		DriverDSound,
		DriverXAudio2,
		NumDrivers
	};


	/** Structure that contain the background flags.*/
	struct TBackgroundFlags
	{
		// pseudo enum to build constant in class def :)
		enum
		{
			/// Number of filter flags in the background.
			NB_BACKGROUND_FLAGS = 32
		};

		bool	Flags[NB_BACKGROUND_FLAGS];

		void serial(NLMISC::IStream &s)
		{
			for (uint i = 0; i<NB_BACKGROUND_FLAGS; ++i)
				s.serial(Flags[i]);
		}
	};

	/** Structure that contain the background filter fadein and fade out delay
	 *  These are configuration data.
	 */
	struct TBackgroundFilterFades
	{
		NLMISC::TTime	FadeIns[TBackgroundFlags::NB_BACKGROUND_FLAGS];
		NLMISC::TTime	FadeOuts[TBackgroundFlags::NB_BACKGROUND_FLAGS];

		TBackgroundFilterFades()
		{
			for (uint i=0; i<TBackgroundFlags::NB_BACKGROUND_FLAGS; ++i)
			{
				FadeIns[i] = 0;
				FadeOuts[i] = 0;
			}
		}
	};
	
	/// Initialization information for the audio mixer. Use instead of the long parametered init function for readability. *** Subject to change. ***
	struct CInitInfo
	{
	public:
		CInitInfo() 
			: MaxTrack(32), 
			EnableReverb(false), 
			EnableOccludeObstruct(false), 
			UseADPCM(false), 
			ForceSoftware(false), 
			ManualRolloff(true)
		{ }

		/// The number of allocated physical playback tracks. Default: 32.
		uint MaxTrack;
		
		/// Enable environment reverberation effect. Default: false.
		bool EnableReverb;

		/// Enable occlusion and obstruction lowpass filters from walls and objects. Default: false.
		bool EnableOccludeObstruct;
		
		/// Use lower quality ADPCM encoded sources for lower memory usage. Default: false.
		bool UseADPCM;

		/// Force using a software sound device. Default: false. [KAETEMI TODO: Allow sound device selection.]
		bool ForceSoftware;
		
		/// Use NeL's distance rolloff math. Default: true.
		bool ManualRolloff;

		/// I forgot what this does, but it's fairly important.
		bool AutoLoadSample;
	};
	
	//@{
	//@name Init methods
	/// Create the audio mixer singleton and return a pointer to its instance
	static UAudioMixer	*createAudioMixer();
	/// Build a sample bank from a directory containing .wav files, and return the path to the written file.
	static std::string buildSampleBank(const std::string &wavDir, const std::string &bankDir, const std::string &bankName);
	/// Build a sample bank from a list of .wav files, and return the path to the written file.
	static std::string buildSampleBank(const std::vector<std::string> &sampleList, const std::string &bankDir, const std::string &bankName);
	/// Build the sound bank packed sheets file from georges sound sheet files with .sound extension in the search path, and return the path to the written file.
	static std::string buildSoundBank(const std::string &packedSheetDir);
	/** Set the global path to the sample banks
	 *	If you have specified some sample bank to load in the
	 *	mixer config file, you MUST set the sample path
	 *	BEFORE calling init.
	 */
	
	virtual void		setSamplePath(const std::string& path) = 0;
	/** Set the global path to the sample banks
	 *	If you have specified some sample bank to load in the
	 *	mixer config file, you MUST set the sample path
	 *	BEFORE calling init.
	 */
	virtual void		setSamplePaths(const std::string &wavAssetPath, const std::string &bankBuildPath) = 0;
	/** Set the global path to the packed sheet files.
	 *	This must be set BEFORE calling init.
	 *	Default is to store packed sheet in the current directory.
	 */
	virtual void		setPackedSheetOption(const std::string &path, bool update) = 0;
	/** Initialization
	 *
	 * In case of failure, can throw one of these ESoundDriver (Exception) objects:
	 * ESoundDriverNotFound, ESoundDriverCorrupted, ESoundDriverOldVersion, ESoundDriverUnknownVersion.
	 *
	 * You can ask for EAX support. If EAX support is requested, then the mixer will try to allocate
	 * hardware accelerated audio tracks.
	 * If the total of available hardware track is less than 10, then EAX is automatically
	 * deactivated. (lies!)
	 * autoLoadSample is used for tools like georges or object viewer where you don't bother to
	 * specify each sample bank to load, you just want to ear the sound.
	 * 
	 * Deprecated by initDriver/getDevices/initDevice.
	 * 
	 *	\param forceSoftware: to force the driver to load in software buffer, not hardware
	 */
	virtual void		init(uint maxTrack = 32, bool useEax = true, bool useADPCM = true, NLMISC::IProgressCallback *progressCallBack = NULL, bool autoLoadSample = false, TDriver driverType = DriverAuto, bool forceSoftware = false, bool manualRolloff = true) = 0;
	
	/// Initialize the NeL Sound Driver with given driverName.
	virtual void		initDriver(const std::string &driverName) = 0;
	/// Get the available devices on the loaded driver.
	virtual void		getDevices(std::vector<std::string> &devices) = 0;	
	/// Initialize the selected device on the currently initialized driver. Leave deviceName empty to select the default device.
	virtual void		initDevice(const std::string &deviceName, const CInitInfo &initInfo, NLMISC::IProgressCallback *progressCallback = NULL) = 0;

	/** Initialisation of the clustered sound system.
	  */
	virtual void		initClusteredSound(NL3D::UScene *uscene, float minGain, float maxDistance, float portalInterpolate = 20.0f) = 0;
	/** Initialisation of the clustered sound system. CNELU version
	 */
	virtual void		initClusteredSound(NL3D::CScene *scene, float minGain, float maxDistance, float portalInterpolate = 20.0f) = 0;
	/** Set the priority channel reserve.
	 *	Each priority channel can be assign a restrictive reserve value.
	 *	This value is used when the number of free tracks available for playing drop
	 *	under the low water mark value (see setLowWaterMark).
	 *	The mixer counts the number of playing sources in each priority channel.
	 *	A priority channel can overflow its reserve value only if the low water
	 *	mark is not reached.
	 *	In other word, when the number of played sources increase, you can control
	 *	a 'smooth' cut in priority layer. The idea is to try to keep some free tracks
	 *	for the HighestPri source.
	 *	By default, reserve are set for each channel to the number of available tracks.
	 */
	virtual void		setPriorityReserve(TSoundPriority priorityChannel, uint reserve) = 0;
	/** Set the Low water mark.
	 *	This value is use to mute sound source that try to play when their priority
	 *	channel is full (see setPriorityReserve).
	 *	Set a value 1 to 4 to keep some extra track available when a
	 *	HighestPri source need to play.
	 *	By default, the value is set to 0, witch means no special treatment is done
	 *	and the mixer will mute sound with no user control at all.
	 *	Note also that the availability of a track is not guaranteed if the sum of
	 *	the priority reserve (see setPriorityReserve) is greater than the number of
	 *	available tracks (which is almost always the case). But this value will help
	 *	the mixer make its best.
	 */
	virtual void		setLowWaterMark(uint value) = 0;
	/** Change the number of tracks in RealTime. If the number is lowered, random tracks are deleted.
	 *	Any playing sources of such deleted track will be stopped
	 */
	virtual	void		changeMaxTrack(uint maxTrack) = 0;

	//@}


	 /// Resets the audio system (deletes all the sources, include envsounds)
	virtual void		reset() = 0;
	/// Disables or reenables the sound
	virtual void		enable( bool b ) = 0;

	//@{
	//@name Sample banks related methods
	/** Load buffers. Returns the number of buffers successfully loaded.
	 *  If you specify a non null notfoundfiles vector, it is filled with the names of missing files if any.
	 *	\param async If true, the sample are loaded in a background thread.
	 *	\param filename Name of the directory that contains the samples to load.
	 *	\param notfoundfiles An optional pointer to a vector that will be filled with the list of not found files.
	 */
	virtual uint32		loadSampleBank(bool async, const std::string &filename, std::vector<std::string> *notfoundfiles=NULL ) = 0;
	/** Unload buffers. Return false if the bank can't be unloaded because an async loading is running.
	*/
	virtual bool		unloadSampleBank( const std::string &filename) = 0;
	/** Reload all the sample bank.
	 *	This method use provided for use in a sound editor or sound tool to update the list of available samples.
	 *	\param async If true, the samples are loaded in a background thread.
	 */
	virtual void		reloadSampleBanks(bool async) =0;
	/** Return the total size in byte of loaded samples.
	 */
	virtual uint32		getLoadedSampleSize() =0;

	/** Return a list of loaded sample bank with their size.
	*/
	virtual void		getLoadedSampleBankInfo(std::vector<std::pair<std::string, uint> > &result) =0;
	//@}

	/// Get a TSoundId from a name (returns NULL if not found)
	virtual TSoundId	getSoundId( const NLMISC::CSheetId &name ) = 0;

	/// Gets the group controller for the given group tree path with separator '/', if it doesn't exist yet it will be created.
	/// Examples: "music", "effects", "dialog", "music/background", "music/loading", "music/player", etcetera
	virtual UGroupController *getGroupController(const std::string &path) = 0;

	/** Add a logical sound source (returns NULL if name not found).
	 * If spawn is true, the source will auto-delete after playing. If so, the return USource* pointer
	 * is valid only before the time when calling play() plus the duration of the sound. You can
	 * pass a callback function that will be called (if not NULL) just before deleting the spawned
	 * source.
	 */
	virtual USource		*createSource(const NLMISC::CSheetId &name, bool spawn=false, TSpawnEndCallback cb=NULL, void *callbackUserParam = NULL, NL3D::CCluster *cluster = 0, CSoundContext *context  = 0, UGroupController *groupController = NULL) = 0;
	/// Add a logical sound source (by sound id). To remove a source, just delete it. See createSource(const char*)
	virtual USource		*createSource(TSoundId id, bool spawn=false, TSpawnEndCallback cb=NULL, void *callbackUserParam  = NULL, NL3D::CCluster *cluster = 0, CSoundContext *context = 0, UGroupController *groupController = NULL) = 0;

	/** Use this method to set the listener position instead of using getListener->setPos();
	 * It's because we have to update the background sounds in this case.
	 */
	virtual void		setListenerPos (const NLMISC::CVector &pos) = 0;

	/// Return the listener interface
	virtual UListener	*getListener() = 0;


	/** Choose the environmental effect(s) corresponding to tag
	 *	\deprecated
	 */
	virtual void		selectEnvEffects( const std::string &tag ) = 0;
	/// Update audio mixer (call evenly)
	virtual void		update() = 0;


	//@{
	//@name Statistic and utility methods
	/// Fill a vector with the names of all loaded sounds.
	virtual void		getSoundNames( std::vector<NLMISC::CSheetId> &names ) const = 0;
	/// Return the number of mixing tracks (voices)
	virtual uint		getPolyphony() const = 0;
	/// Return the number of sources
	virtual uint		getSourcesInstanceCount() const = 0;
	/// Return the number of playing sources
	virtual uint		getPlayingSourcesCount() const = 0;
	/// Return the number of available tracks
	virtual uint		getAvailableTracksCount() const = 0;
	/// Return the number of used tracks
	virtual uint		getUsedTracksCount() const = 0;
	/// Return the number muted playing source
	virtual uint		getMutedPlayingSourcesCount() const = 0;
	/// Return a string showing the playing sources
	virtual std::string	getSourcesStats() const = 0;
	virtual void		getPlayingSoundsPos(bool virtualPos, std::vector<std::pair<bool, NLMISC::CVector> > &pos) =0;
	/** Write profiling information about the mixer to the output stream.
	 *  \param out The output stream to which to write the information
	 */
	virtual void		writeProfile(std::string& out) = 0;
	//@}




	//@{
	//@name Background sounds
	virtual void		setBackgroundFlagName(uint flagIndex, const std::string &flagName) = 0;
	virtual void		setBackgroundFlagShortName(uint flagIndex, const std::string &flagShortName) = 0;
	virtual const std::string &getBackgroundFlagName(uint flagIndex) =0;
	virtual const std::string &getBackgroundFlagShortName(uint flagIndex) =0;
	virtual void		setBackgroundFlags(const TBackgroundFlags &backgroundFlags) = 0;
	virtual const TBackgroundFlags &getBackgroundFlags() =0;
	virtual void		setBackgroundFilterFades(const TBackgroundFilterFades &backgroundFilterFades) = 0;
	virtual const TBackgroundFilterFades &getBackgroundFilterFades() = 0;

	virtual void		loadBackgroundAudioFromPrimitives(const NLLIGO::IPrimitive &audioRoot) =0;
	virtual void		loadBackgroundSound (const std::string &continent, NLLIGO::CLigoConfig &config) = 0;
	virtual void		playBackgroundSound () = 0;
	virtual void		stopBackgroundSound () = 0;

	/// Deprecated
//	virtual void		loadBackgroundSoundFromRegion (const NLLIGO::CPrimRegion &region) = 0;
	/// Deprecated
//	virtual void		loadBackgroundEffectsFromRegion (const NLLIGO::CPrimRegion &region) = 0;
	/// Deprecated
//	virtual void		loadBackgroundSamplesFromRegion (const NLLIGO::CPrimRegion &region) = 0;
	//@}

	//@{
	//@name User controlled variable
	/** Set the value of a user variable.
	 *	User variable are variables that can be used to control
	 *	the gain or transpose all the instances (source) of a
	 *	given sound.
	 *	This has been initially designed to control the gain of any
	 *	source playing some atmospheric sound (like rain) according
	 *	to the intensity of the effect (ie small rain or big rain).
	 *	Binding from user var to sound parameter is done in
	 *	one or more georges sheet .user_var_binding.
	 */
	virtual void		setUserVar(NLMISC::TStringId varName, float value) =0;
	/// Return the current value of a user var.
	virtual float		getUserVar(NLMISC::TStringId varName) =0;
	//@}

	//@{
	//@name Sound driver bench
	virtual void startDriverBench() =0;
	virtual void endDriverBench() =0;
	virtual void displayDriverBench(NLMISC::CLog *log) =0;

	//@}


	//@{
	//@name Sound Music
	/** Play some music (.mp3 etc...) (implemented in fmod only)
	 *	NB: if an old music was played, it is first stop with stopMusic()
	 *
	 *	\param fileName a CPath::lookup is done (the file can be in a BNP)
	 *	\param xFadeTime if not 0 the old music played is not stopped immediately but a cross-fade of xFadeTime (in ms) is made between the 2.
	 *	\param async if false, the music is entirely loaded in memory. Interesting for instance for music played
	 *		during loading (to not overload HardDrive). NB: The file is loaded into memory, but decompressed by FMod in a thread
	 *		Hence if the mp3 fileSize is 5 Mb, it will take only 5 Mb in memory (not the decompressed 40 Mb size)
	 *	\param loop must be true to play the music in loop.
	 */
	virtual bool	playMusic(const std::string &fileName, uint xFadeTime= 0, bool async= true, bool loop=true) =0;
	/** Stop the music previously loaded and played (the memory is also freed)
	 *	\param xFadeTime if not 0 the old music played is not stopped immediately but a fade out of xFadeTime (in ms) is made
	 */
	virtual void	stopMusic(uint xFadeTime= 0) =0;
	/** Pause the music previously loaded and played (the memory is not freed)
	 */
	virtual void	pauseMusic() =0;
	/** Resume the music previously paused
	 */
	virtual void	resumeMusic() =0;
	/** Return true if a song is finished.
	 */
	virtual bool	isMusicEnded() =0;
	/** Return the length of the music currently played (0 if none)
	 */
	virtual float	getMusicLength() =0;
	/** Set the music volume (if any music played). (volume value inside [0 , 1]) (default: 1)
	 *	NB: the volume of music is NOT affected by IListener::setGain()
	 */
	virtual void	setMusicVolume(float gain) =0;
	/** Get the song title. Returns false if the song is not found or the function is not implemented.
	 * If the song as no name, result is filled with the filename.
	 */
	virtual bool	getSongTitle(const std::string &filename, std::string &result, float &length) =0;
	/** enable or disable the background music system. disable it when you want to play your own mp3 for instance
	 */
	virtual void	enableBackgroundMusic(bool enable) =0;
	/** set to false to avoid TimeBeforeCanReplay and MinimumPlaytime behavior
	 */
	virtual void	enableBackgroundMusicTimeConstraint(bool enable) =0;
	/** Play some music in the event channel (at same time than general channel)
	 *	NB: it is user responsibility to lower/pause the generic music channel if he doesn't want 2 musics at same time
	 */
	virtual bool	playEventMusic(const std::string &fileName, uint xFadeTime= 0, bool async= true, bool loop=true) =0;
	/** Stop any event music
	 */
	virtual void	stopEventMusic(uint xFadeTime= 0) =0;
	/** Change the volume of event music channel
	 */
	virtual void	setEventMusicVolume(float gain) =0;
	/** true if the event music is ended
	 */
	virtual bool	isEventMusicEnded() =0;	
	/// Get audio/container extensions that are currently supported by nel or the used driver implementation.
	virtual void getMusicExtensions(std::vector<std::string> &extensions) = 0;
	//@}


	/// Destructor
	virtual				~UAudioMixer() {}
};


} // NLSOUND


#endif // NL_U_AUDIO_MIXER_H

/* End of u_audio_mixer.h */
