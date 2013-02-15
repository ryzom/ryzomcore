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

#ifndef NL_U_LISTENER_H
#define NL_U_LISTENER_H

#include "nel/misc/types_nl.h"

namespace NLMISC {

class CVector;

}

namespace NLSOUND {


/**
 * Game interface for listener control.
 *
 * For arguments as 3D vectors, use the NeL vector coordinate system:
 * \verbatim
 *     (top)
 *       z
 *       |  y (front)
 *       | /
 *       -----x (right)
 * \endverbatim
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class UListener
{
public:

	/// \name Listener properties
	//@{
	/// Set the position vector (default: (0,0,0)) (3D mode only)
	virtual void			setPos( const NLMISC::CVector& pos ) = 0;
	/** Get the position vector.
	 * See setPos() for details.
	 */
	virtual const NLMISC::CVector &	getPos() const = 0;
	/// Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
	virtual void			setVelocity( const NLMISC::CVector& vel ) = 0;
	/// Get the velocity vector
	virtual void			getVelocity( NLMISC::CVector& vel ) const = 0;
	/// Set the orientation vectors (3D mode only, ignored in stereo mode) (default: (0,1,0), (0,0,1))
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

	/// Destructor
	virtual					~UListener() {}

protected:

	/// Constructor
	UListener() {}

};


} // NLSOUND


#endif // NL_U_LISTENER_H

/* End of u_listener.h */
