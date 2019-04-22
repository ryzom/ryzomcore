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

#include "stdopenal.h"
#include "listener_al.h"
#include "sound_driver_al.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

namespace NLSOUND {

/// Constructor
CListenerAL::CListenerAL() : IListener(), _Pos(0.0f, 0.0f, 0.0f)
{
	
}

/// Destructor
CListenerAL::~CListenerAL()
{

}

/*
 * Set the position vector (default: (0,0,0)) (3D mode only)
 */
void					CListenerAL::setPos( const NLMISC::CVector& pos )
{
	_Pos = pos;
	// Coordinate system: conversion from NeL to OpenAL/GL:
	alListener3f( AL_POSITION, pos.x, pos.z, -pos.y );
	alTestError();
}


/** Get the position vector.
 * See setPos() for details.
 */
const NLMISC::CVector &CListenerAL::getPos() const
{
	return _Pos;

// Currently, the OpenAL headers are different between Windows and Linux versions !
// alGetListener3f() is part of the spec, though.
/*
#ifdef NL_OS_WINDOWS
	// Coordsys conversion
	float glposz;
	alGetListener3f( AL_POSITION, &pos.x, &pos.z, &glposz );
	pos.y = -glposz;
#else
	float posarray [3];
	alGetListenerfv( AL_POSITION, posarray );
	// Coordsys conversion
	pos.set( posarray[0], -posarray[2], posarray[1] );
#endif
	alTestError();
*/
}


/*
 * Set the velocity vector (3D mode only)
 */
void					CListenerAL::setVelocity( const NLMISC::CVector& vel )
{
	// Coordsys conversion
	alListener3f( AL_VELOCITY, vel.x, vel.z, -vel.y );
	alTestError();
}


/*
 * Get the velocity vector
 */
void				 	CListenerAL::getVelocity( NLMISC::CVector& vel ) const
{
#ifdef NL_OS_WINDOWS
	// Coordsys conversion
	float glposz;
	alGetListener3f( AL_VELOCITY, &vel.x, &vel.z, &glposz );
	vel.y = - glposz;
#else
	float velarray [3];
	alGetListenerfv( AL_VELOCITY, velarray );
	// Coordsys conversion
	vel.set( velarray[0], -velarray[2], velarray[1] );
#endif
	alTestError();
}


/*
 * Set the orientation vectors (3D mode only)
 */
void					CListenerAL::setOrientation( const NLMISC::CVector& front, const NLMISC::CVector& up )
{
	// Forward then up
	ALfloat v[6];
	// Coordsys conversion
	v[0] = front.x;
	v[1] = front.z;
	v[2] = -front.y;
	v[3] = up.x;
	v[4] = up.z;
	v[5] = -up.y;
	alListenerfv( AL_ORIENTATION, v );
	alTestError();
}


/*
 * Get the orientation vectors
 */
void					CListenerAL::getOrientation( NLMISC::CVector& front, NLMISC::CVector& up ) const
{
	// Forward then up
	ALfloat v[6];
	alGetListenerfv( AL_ORIENTATION, v );
	alTestError();
	// Coordsys conversion
	front.set( v[0], -v[2], v[1] );
	up.set( v[3], -v[5], v[4] );
}


/* Set the gain (volume value inside [0 , 1]). (default: 1)
 * 0.0 -> silence
 * 0.5 -> -6dB
 * 1.0 -> no attenuation
 * values > 1 (amplification) not supported by most drivers
 */
void					CListenerAL::setGain( float gain )
{
	alListenerf( AL_GAIN, gain );
	alTestError();
}


/*
 * Get the gain
 */
float					CListenerAL::getGain() const
{
	ALfloat gain;
#ifdef NL_OS_WINDOWS
	alGetListenerf( AL_GAIN, &gain );
#else
	alGetListenerfv( AL_GAIN, &gain );
#endif
	alTestError();
	return gain;
}


/*
 * Set the doppler factor (default: 1) to exaggerate or not the doppler effect
 */
void					CListenerAL::setDopplerFactor( float f )
{
	alDopplerFactor( f );
	alTestError();
}


/*
 * Set the rolloff factor (default: 1) to scale the distance attenuation effect
 */
void					CListenerAL::setRolloffFactor( float f )
{
	nlassert(CSoundDriverAL::getInstance() != NULL);
	CSoundDriverAL::getInstance()->applyRolloffFactor(f);
}


/*
 * Set DSPROPERTY_EAXLISTENER_ENVIRONMENT and DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE if EAX available (see EAX listener properties)
 */
#if EAX_AVAILABLE == 1
void CListenerAL::setEnvironment(uint env, float size)
{
	if (AlExtEax)
	{
		eaxSet( &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENT, 0, &env, sizeof(unsigned long) );
		eaxSet( &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE, 0, &size, sizeof(float) );
	}
#else
void CListenerAL::setEnvironment(uint /* env */, float /* size */)
{
#endif
}


/*
 * Set any EAX listener property if EAX available
 */
#if EAX_AVAILABLE == 1
void CListenerAL::setEAXProperty(uint prop, void *value, uint valuesize)
{
	if (AlExtEax)
	{
		eaxSet( &DSPROPSETID_EAX_ListenerProperties, prop, 0, value, valuesize );
	}
#else
void CListenerAL::setEAXProperty(uint /* prop */, void * /* value */, uint /* valuesize */)
{
#endif
}


} // NLSOUND
