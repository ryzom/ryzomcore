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

#ifndef NL_LISTENER_FMOD_H
#define NL_LISTENER_FMOD_H

#include "nel/sound/driver/listener.h"
#include "nel/misc/singleton.h"

namespace NLSOUND
{


/**
 * FMOD listener.
 *
 * For arguments as 3D vectors, use the NeL vector coordinate system
 * (not FMOD's one).
 *
 * \author Peter Hanappe, Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CListenerFMod : public IListener, public NLMISC::CManualSingleton<CListenerFMod>
{
friend class CSoundDriverFMod;

public:

	/// Constructor
	CListenerFMod();

	/// Deconstructor
	virtual ~CListenerFMod();

	/// \name Listener properties
	//@{

    /// Set the position vector (default: (0,0,0)) (3D mode only)
	virtual void			setPos( const NLMISC::CVector& pos );

    /// Get the position vector. See setPos() for details.
	virtual const NLMISC::CVector	&getPos() const;

    /// Set the velocity vector (3D mode only) (default: (0,0,0))
	virtual void			setVelocity( const NLMISC::CVector& vel );

    /// Get the velocity vector
	virtual void			getVelocity( NLMISC::CVector& vel ) const;

    /// Set the orientation vectors (3D mode only) (default: (0,1,0), (0,0,1))
	virtual void			setOrientation( const NLMISC::CVector& front, const NLMISC::CVector& up );

    /// Get the orientation vectors
	virtual void			getOrientation( NLMISC::CVector& front, NLMISC::CVector& up ) const;

    /** Set the gain (volume value inside [0 , 1]). (default: 1)
	 * 0.0 -> silence
	 * 0.5 -> -6dB
	 * 1.0 -> no attenuation
	 * values > 1 (amplification) not supported by most drivers
	 */
    virtual void			setGain( float gain );

    /// Get the gain
	virtual float			getGain() const;
	//@}

	/// \name Global properties
	//@{

    /// Set the doppler factor (default: 1) to exaggerate or not the doppler effect
	virtual void			setDopplerFactor( float f );

    /// Set the rolloff factor (default: 1) to scale the distance attenuation effect
	virtual void			setRolloffFactor( float f );

    /// Return the rolloff factor
	virtual float			getRolloffFactor();

    /// Set DSPROPERTY_EAXLISTENER_ENVIRONMENT and DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE if EAX available (see EAX listener properties)
	virtual void			setEnvironment( uint env, float size=7.5f );

    /// Set any EAX listener property if EAX available
	virtual void			setEAXProperty( uint prop, void *value, uint valuesize );
	//@}


	/// Get Matrix for RelativeToAbsolute compute in Sources
	const NLMISC::CMatrix	&getPosMatrix() const {return _PosMatrix;}
	const NLMISC::CMatrix	&getVelMatrix() const {return _VelMatrix;}

private:

	/// Release all DirectSound resources
	void					release();

	// Nel Basis
	NLMISC::CVector			_Pos;
	NLMISC::CVector			_Vel;
	NLMISC::CVector			_Front;
	NLMISC::CVector			_Up;
	NLMISC::CMatrix			_PosMatrix;
	NLMISC::CMatrix			_VelMatrix;

	// Some props
	float					_RolloffFactor;

	void	updateFModPos();

};


} // NLSOUND


#endif // NL_LISTENER_FMOD_H

