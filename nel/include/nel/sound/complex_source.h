// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_COMPLEX_SOURCE_H
#define NL_COMPLEX_SOURCE_H

#include "nel/misc/types_nl.h"
//#include "nel/sound/u_source.h"
#include "nel/sound/source_common.h"
#include "nel/sound/audio_mixer_user.h"


namespace NLSOUND {

class CComplexSound;

/** Implemetation class for Complex source.
 *	Complex source are source that use a CPatternSound object.
 */
class CComplexSource : public CSourceCommon, public CAudioMixerUser::IMixerEvent, public CAudioMixerUser::IMixerUpdate
{
public:
	/// Constructor
	CComplexSource	(CComplexSound *soundPattern=NULL, bool spawn=false, TSpawnEndCallback cb=0, void *cbUserParam = 0, NL3D::CCluster *cluster = 0, CGroupController *groupController = NULL);
	/// Destructor
	~CComplexSource	();

	/// Return the sound binded to the source (or NULL if there is no sound)
	virtual TSoundId				getSound();
	/// Change the priority of the source
//	virtual void					setPriority( TSoundPriority pr, bool redispatch=true );

	/// \name Playback control
	//@{
	/// Set looping on/off for future playbacks (default: off)
//	virtual void					setLooping( bool l );
	/// Return the looping state
//	virtual bool					getLooping() const;
	/// Play
	virtual void					play();
	/// Stop playing
	virtual void					stop();
	/// Get playing state. Return false even if the source has stopped on its own.
	virtual bool					isPlaying();
	/// Tells this source not to call its callbacks when it ends. This is valid for spawned sources only.
//	virtual	void					unregisterSpawnCallBack();
	/// Returns the number of milliseconds the source has been playing
	virtual uint32					getTime();
	//@}


	/// \name Source properties
	//@{
	/** Set the position vector (default: (0,0,0)).
	 * 3D mode -> 3D position
	 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
	 */
	virtual void					setPos( const NLMISC::CVector& pos );
	/// Get the position vector (3D mode only)
//	virtual void					getPos( NLMISC::CVector& pos ) const;
	/// Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
	virtual void					setVelocity( const NLMISC::CVector& vel );
	/// Get the velocity vector
//	virtual void					getVelocity( NLMISC::CVector& vel ) const;
	/// Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
	virtual void					setDirection( const NLMISC::CVector& dir );
	/// Get the direction vector
//	virtual void					getDirection( NLMISC::CVector& dir ) const;
	/** Set the gain (volume value inside [0 , 1]). (default: 1)
	 * 0.0 -> silence
	 * 0.5 -> -6dB
	 * 1.0 -> no attenuation
	 * values > 1 (amplification) not supported by most drivers
	 */
	virtual void					setGain( float gain );
	/// Get the gain
//	virtual float					getGain() const;
	/** Set the gain amount (value inside [0, 1]) to map between 0 and the nominal gain
	 * (which is getSource()->getGain()). Does nothing if getSource() is null.
	 */
	virtual void					setRelativeGain( float gain );
	/// Return the relative gain (see setRelativeGain()), or the absolute gain if getSource() is null.
//	virtual float					getRelativeGain() const;
	/** Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
	 * of one octave. 0 is not a legal value.
	 */
//	virtual void					setPitch( float pitch );
	/// Get the pitch
//	virtual float					getPitch() const;
	/// Set the source relative mode. If true, positions are interpreted relative to the listener position (default: false)
//	virtual void					setSourceRelativeMode( bool mode );
	/// Get the source relative mode
//	virtual bool					getSourceRelativeMode() const;

	void							checkup();

private:

	TSOURCE_TYPE getType() const {return SOURCE_COMPLEX;}

	/// Mixer update implementation.
	void onUpdate();
	/// Mixer event implementation.
	void onEvent();
	/// Do the dirty work of starting to play. Called by play and by setGain or setRelativeGain when the source is muted.
	void playStuf();

	USource			*_Source1;
	USource			*_Source2;

	std::vector<USource	*>	_AllSources;

	/** When play is called but the gain is 0, then the sound is muted.
	 *	This flag keek track of this to restart the source when the
	 *	gain is updated.
	 */
	bool				_Muted;

//	NLMISC::TTime	_Length1;
//	NLMISC::TTime	_Length2;

	NLMISC::TTime	_StartTime1;
	NLMISC::TTime	_StartTime2;

	float			_TickPerSecond;
	uint32			_FadeLength;

	float			_FadeFactor;

	uint			_SoundSeqIndex;
	uint			_DelaySeqIndex;

	bool			_LastSparseEvent;

	/// The reference Sound.
	CComplexSound	*_PatternSound;
};


} // NLSOUND

#endif // NL_COMPLEX_SOURCE_H
