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

#ifndef NL_U_SOURCE_H
#define NL_U_SOURCE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

namespace NLMISC {

class CVector;

}


namespace NLSOUND {


static const uint SoundContextNbArgs = 10;

struct CSoundContext
{
	// -1 for args is a special value means that unset. Don't use negative number

	CSoundContext()
	:	Position(NLMISC::CVector::Null),
		PreviousRandom(100),
		RelativeGain(1.0f)
	{
		for (uint i = 0; i < SoundContextNbArgs; i++)
			Args[i] = -1;
	}

	sint32			Args[SoundContextNbArgs];
	NLMISC::CVector Position;
	uint32			PreviousRandom;
	float			RelativeGain;
};


class CSound;
class USource;


/// Sound sample identifiers
typedef CSound* TSoundId;

/// Priority of the sources (p1<p2 means p1 has higher priority than p2)
enum TSoundPriority { HighestPri, HighPri, MidPri, LowPri, NbSoundPriorities};
//const uint NbSoundPriorities = 4;

/// Type of callbacks called before a spawned source is deleted
typedef void (*TSpawnEndCallback) (USource *, void *);


/**
 * Game interface for sound sources (stereo or 3D sound instances).
 *
 * The mode is 3D if the sound buffer (specified by its Sound Id) is mono,
 * otherwise it is stereo.
 *
 * For arguments as 3D vectors, use the NeL vector coordinate system:
 * \verbatim
 *     (top)
 *       z
 *       |  y (front)
 *       | /
 *       -----x (right)
 *
 * The default priority is MidPri.
 * Some properties are assigned at initialization's time, from the
 * specified sound id (use NeL Sources Sound Builder to set these
 * initial values).
 *
 * \endverbatim
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class USource
{
public:

	/// Return the sound binded to the source (or NULL if there is no sound)
	virtual TSoundId				getSound() = 0;
	/// Change the priority of the source
	virtual void					setPriority( TSoundPriority pr) = 0;

	/// \name Playback control
	//@{
	/// Set looping on/off for future playbacks (default: off)
	virtual void					setLooping( bool l ) = 0;
	/// Return the looping state
	virtual bool					getLooping() const = 0;
	/// Play
	virtual void					play() = 0;
	/// Stop playing
	virtual void					stop() = 0;
	/// Get playing state. Return false even if the source has stopped on its own.
	virtual bool					isPlaying() = 0;
	/// Tells this source not to call its callbacks when it ends. This is valid for spawned sources only.
	virtual	void					unregisterSpawnCallBack() = 0;
	/// Returns the number of milliseconds the source has been playing
	virtual uint32					getTime() =0;
	//@}


	/// \name Source properties
	//@{
	/** Set the position vector (default: (0,0,0)).
	 * 3D mode -> 3D position
	 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
	 */
	virtual void					setPos( const NLMISC::CVector& pos ) = 0;
	/// Get the position vector (3D mode only)
	virtual const NLMISC::CVector	&getPos() const = 0;
	/// Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
	virtual void					setVelocity( const NLMISC::CVector& vel ) = 0;
	/// Get the velocity vector
	virtual void					getVelocity( NLMISC::CVector& vel ) const = 0;
	/// Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
	virtual void					setDirection( const NLMISC::CVector& dir ) = 0;
	/// Get the direction vector
	virtual void					getDirection( NLMISC::CVector& dir ) const = 0;
	/** Set the gain (volume value inside [0 , 1]). (default: 1)
	 * 0.0 -> silence
	 * 0.5 -> -6dB
	 * 1.0 -> no attenuation
	 * values > 1 (amplification) not supported by most drivers
	 */
	virtual void					setGain( float gain ) = 0;
	/// Get the gain
	virtual float					getGain() const = 0;

	/** Set the gain amount (value inside [0, 1]) to map between 0 and the nominal gain
	 * (which is getSource()->getGain()). Does nothing if getSource() is null.
	 */
	virtual void					setRelativeGain( float gain ) = 0;
	/// Return the relative gain (see setRelativeGain()), or the absolute gain if getSource() is null.
	virtual float					getRelativeGain() const = 0;
	/** Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
	 * of one octave. 0 is not a legal value.
	 */
	virtual void					setPitch( float pitch ) = 0;
	/// Get the pitch
	virtual float					getPitch() const = 0;
	/// Set the source relative mode. If true, positions are interpreted relative to the listener position (default: false)
	virtual void					setSourceRelativeMode( bool mode ) = 0;
	/// Get the source relative mode
	virtual bool					getSourceRelativeMode() const = 0;
	//@}

	/// Destructor
	virtual							~USource() {}

protected:

	/// Constructor
	USource() {}

};


} // NLSOUND


#endif // NL_U_SOURCE_H

/* End of u_source.h */
