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

#ifndef NL_BACKGROUND_SOUND_MANAGER_H
#define NL_BACKGROUND_SOUND_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/sound/background_sound.h"
#include <vector>
#include <set>

#include "audio_mixer_user.h"

namespace NLLIGO
{
	class CPrimRegion;
	class CLigoConfig;
}

namespace NLSOUND {


class CListenerUser;
class IPlayable;
class IBoundingShape;
class CSourceCommon;

/// Number of background layer. Layer are identified in .prim by a letter starting from 'a' (for layer 0)
const uint32	BACKGROUND_LAYER = 5;	// 3 layer


/**
 * This manager handle the background sound :
 * - primitive positioned sound (point, path and patatoid supported)
 * - primitive positioned effect (patatoid only)
 * - primitive positioned sample bank (patatoid only)
 * - A set of 32 application definable flag that can be used in
 *	background sound to filter the sub sounds of a background sound.
 *  Each filter can be assigned at run time a fade in and fade out delay
 *	that is used when sound are muted/unmuted according to filter status.
 *
 *
 *
 * Sounds can be put in three separate layer (named a, b and c). In each
 * layer, sounds patatoid are concurent and the smaller in surface muffle the
 * other sounds.
 *
 * Effets are managed according to the EAX capacity of the driver.
 *
 * Sample bank patatoid are used to dynamicaly load/unload the sample banks will the
 * player walk the univers.
 *
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2002
 */
class CBackgroundSoundManager : CAudioMixerUser::IMixerUpdate
{
public:
	/** Load the background sounds from a CPrimRegion class.
	 *	deprecated
	 */
//	void		loadSoundsFromRegion(const NLLIGO::CPrimRegion &region);
	/** Load the effects from a CPrimRegion class.
	 *	deprecated
	 */
//	void		loadEffecsFromRegion(const NLLIGO::CPrimRegion &region);
	/** Load the samples banks from a CPrimRegion class.
	 *	deprecated
	 */
//	void		loadSamplesFromRegion(const NLLIGO::CPrimRegion &region);
	/** Load the sounds, effects and sample banks from a region class.
	*/
	void		loadAudioFromPrimitives(const NLLIGO::IPrimitive &audioRoot);

	/** Load background sound for a continent. It'll automatically unload the old continent before loading the new one.
	 * This method load the 'audio' regions (specifying the sounds), the 'effect' regions and the 'sample' regions.
	 * Continent is for example "matis" or "fyros". It'll add .prim to the continent name and lookup() to find zones.
	 * So, don't forget to add sound .prim in the CPath system for the lookup
	 * With the new primitive file, this method will try to load the .primitive file before
	 * attempting to load any .prim file. If the .primitive is found, then no .prim are loaded.
	 */
	void		load (const std::string &continent, NLLIGO::CLigoConfig &config);

	/** Remove all data about the current continents
	 */
	void		unload ();

	/// Start to play the background sound.
	void		play();
	/// Stop the background sound.
	void		stop();

	/// Call this method when the listener position change
	void		setListenerPosition (const NLMISC::CVector &listenerPosition);

	/// Call this method to update the background sound (sub method of setListenerPosition)
	void		updateBackgroundStatus();

	// Call this function evenly to update the stuffs
//	void		update ();

	/// Return a patatoid. If isPath is not null, set it.
	const std::vector<NLMISC::CVector> &getZone(uint32 zone, bool *isPath = 0);

	/// Return the position of the 3d source for a zone
	NLMISC::CVector getZoneSourcePos(uint32 zone);

	/// Get the background flags.
	const UAudioMixer::TBackgroundFlags &getBackgroundFlags();
	/// Set the background flags.
	void		setBackgroundFlags(const UAudioMixer::TBackgroundFlags &backgroundFlags);

	const UAudioMixer::TBackgroundFilterFades &getBackgroundFilterFades();
	void		setBackgroundFilterFades(const UAudioMixer::TBackgroundFilterFades &backgroundFilterFades);

	const float	*getFilterValues() { return _FilterFadeValues;}

private:

	// called by mixer when update registered.
	void onUpdate();

	// CAudioMixerUser will call private constructor and destructor, so, it is our friend ;)
	friend class CAudioMixerUser;

	/// Constructor
	CBackgroundSoundManager();
	/// Destructor
	virtual					~CBackgroundSoundManager();

	/** Load the sounds from primitive */
	void		loadSoundsFromPrimitives(const NLLIGO::IPrimitive &soundRoot);
	/** Load the sample banks from primitive */
	void		loadSamplesFromPrimitives(const NLLIGO::IPrimitive &sampleRoot);
	/** Load the sounds from primitive */
	void		loadEffectsFromPrimitives(const NLLIGO::IPrimitive &fxRoot);
	// add a sound in a layer
	void addSound(const std::string &soundName, uint layerId, const std::vector<NLLIGO::CPrimVector> &points, bool isPath);
	/// deprecated, Internal use only for loading.
	void addSound(const std::string &rawSoundName, const std::vector<NLLIGO::CPrimVector> &points, bool isPath);
	/// add a sample bank zone
	void addSampleBank(const std::vector<std::string> &bankNames, const std::vector<NLLIGO::CPrimVector> &points);
	/// add a fx zone
	void addFxZone(const std::string &fxName, const std::vector<NLLIGO::CPrimVector> &points);

	/// TODO : Utility... should be in NLMISC ?
	/*template <class CharType>
		std::vector<std::basic_string<CharType> >	split(const std::basic_string<CharType> &str, CharType splitTag)
	{
		std::vector<std::basic_string<CharType> >	splitted;
		std::basic_string<CharType>::size_type pos = 0, nextPos = 0, size = 0;

		while ((nextPos = str.find(splitTag, nextPos)) != std::basic_string<CharType>::npos)
		{
			size = nextPos - pos;
			if (size > 0)
				splitted.push_back(std::basic_string<CharType>(str, pos, nextPos - pos));
			// skip the tag
			nextPos += 1;
			pos = nextPos;
		}
		// is there a last part ?
		size = nextPos - pos;
		if (pos != str.size())
			splitted.push_back(std::basic_string<CharType>(str, pos, str.size() - pos));

		return splitted;
	}*/


	/// Flag for playing background sounds.
	bool					_Playing;

	/// Background flags.
	UAudioMixer::TBackgroundFlags			_BackgroundFlags;
	/// Background filters fades
	UAudioMixer::TBackgroundFilterFades		_BackgroundFilterFades;
	/// The date of last fade in or out started for each filter
	NLMISC::TTime							_FilterFadesStart[UAudioMixer::TBackgroundFlags::NB_BACKGROUND_FLAGS];
	/// The filter fade values.
	float									_FilterFadeValues[UAudioMixer::TBackgroundFlags::NB_BACKGROUND_FLAGS];
	/// Is some fade in/out running?
	bool									_DoFade;

	/// The last position of the listener.
	NLMISC::CVector			_LastPosition;

	//@{
	//\name Samples banks related thinks

	/// Storage for a samples banks zone
	struct TBanksData
	{
		/// The list of samples banks that are active on this patatoid
		std::vector<std::string>		Banks;
		/// The min vector of the bounding box
		NLMISC::CVector		MinBox;
		/// The max vector of the bounding box
		NLMISC::CVector		MaxBox;
		/// The vector of points composing the patatoid.
		std::vector<NLMISC::CVector>	Points;

		void serial(NLMISC::IStream &s);
	};
	/// Container for the banks primitive.
	std::vector<TBanksData>		_Banks;
	/// Container for the list of currently loaded banks.
	std::set<std::string>		_LoadedBanks;
	//@}


	//@{
	//\name Sounds related thinks
	/// Storage for all a sound in a layer.
	struct TSoundData
	{
		/// The name of the sound.
		NLMISC::CSheetId	SoundName;
		/// The reference to the sound.
		CSound				*Sound;
		/// A source instance of the sound (may be NULL).
		CSourceCommon		*Source;

		/// The min vector of the bounding box
		NLMISC::CVector		MinBox;
		/// The max vector of the bounding box
		NLMISC::CVector		MaxBox;
		/// The surface of the bounding box (used for patatoid competition)
		float				Surface;
		/// The max earing distance of the sound.
		float				MaxDist;
		/// Flag for path/patatoid sound.
		bool				IsPath;
		/// The vector of points compositing the primitive
		std::vector<NLMISC::CVector>	Points;
		/// Flag telling if this sound is currently selected for play by bounding box.
		bool				Selected;

		void serial(NLMISC::IStream &s);
	};
	/// Array of vector of sound data.
	std::vector<TSoundData>		_Layers[BACKGROUND_LAYER];
	// utility structure for audio mixing computation (see cpp)
	struct TSoundStatus
	{
		/// The data of the sound.
		TSoundData			&SoundData;
		/// The position of the source.
		NLMISC::CVector		Position;
		/** The relative gain of the source. This is used for patatoid competition.when
		  * a smaller patatoid mute bigger one.
		  */
		float				Gain;
		/// The distance beween listener and source.
		float				Distance;
		/// flag if inside a sound zone
		bool				Inside;
		/// Constructor.
		TSoundStatus(TSoundData &sd, NLMISC::CVector position, float gain, float distance, bool inside)
			: SoundData(sd), Position(position), Gain(gain), Distance(distance), Inside(inside)
		{}
	};
	//@}

	//\name Env fx related thinks
	//@{
	/// Storage for a fx zone
	struct TFxZone
	{
		/// Name of the env fx
		NLMISC::TStringId				FxName;
		/// The vector of points compositing the primitive
		std::vector<NLMISC::CVector>	Points;
		/// The min vector of the bounding box
		NLMISC::CVector					MinBox;
		/// The max vector of the bounding box
		NLMISC::CVector					MaxBox;

		void serial(NLMISC::IStream &s);
	};
	/// Container for the fx primitive.
	std::vector<TFxZone>		_FxZones;
	/// Last setted env fx. Used when clustered sound is not active
	NLMISC::TStringId			_LastEnv;

	//@}
};


} // NLSOUND


#endif // NL_BACKGROUND_SOUND_MANAGER_H

/* End of background_sound_manager.h */
