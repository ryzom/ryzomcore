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

#ifndef NL_LISTENER_H
#define NL_LISTENER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

namespace NLSOUND
{

/// Default environmental effect
#define ENVFX_DEFAULT_NUM 2

/// Default environmental effect size
#define ENVFX_DEFAULT_SIZE 7.5f

/**
 * Sound listener interface (implemented in sound driver dynamic library)
 *
 * For arguments as 3D vectors, use the NeL vector coordinate system:
 * \verbatim
 *     (top)
 *       z
 *       |  y (front)
 *       | /
 *       -----x (right)
 * \endverbatim
 *
 * The listener is a singleton.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class IListener
{
public:
	/// Constructor
	IListener() { }
	/// Destructor
	virtual ~IListener() { }

	/// \name Listener properties
	//@{
	/// Set the position vector (default: (0,0,0)) (3D mode only)
	virtual void			setPos( const NLMISC::CVector& pos ) = 0;
	/** Get the position vector.
	 * See setPos() for details.
	 */
	virtual const NLMISC::CVector &getPos() const = 0;
	/// Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
	virtual void			setVelocity( const NLMISC::CVector& vel ) = 0;
	/// Get the velocity vector
	virtual void			getVelocity( NLMISC::CVector& vel ) const = 0;
	/// Set the orientation vectors (3D mode only, ignored in stereo mode) (default: (0,1,0), (0,0,-1))
	virtual void			setOrientation( const NLMISC::CVector& front, const NLMISC::CVector& up ) = 0;
	/// Get the orientation vectors
	virtual void			getOrientation( NLMISC::CVector& front, NLMISC::CVector& up ) const = 0;
	/** Set the gain (volume value inside [0 , 1]). (default: 1)
	 * 0.0 -> silence
	 * 0.5 -> -6dB
	 * 1.0 -> no attenuation
	 * values > 1 (amplification) not supported by most drivers
	 */
	virtual void			setGain( float gain ) = 0;
	/// Get the gain
	virtual float			getGain() const = 0;
	//@}

	/// \name Global properties
	//@{
	/// Set the doppler factor (default: 1) to exaggerate or not the doppler effect
	virtual void			setDopplerFactor( float f ) = 0;
	/// Set the rolloff factor (default: 1) to scale the distance attenuation effect
	virtual void			setRolloffFactor( float f ) = 0;
	//@}
};


} // NLSOUND


#endif // NL_LISTENER_H

/* End of listener.h */
