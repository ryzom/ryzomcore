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

#include "stdfmod.h"
#include "listener_fmod.h"
#include "sound_driver_fmod.h"


using namespace NLMISC;


namespace NLSOUND
{


// ***************************************************************************
CListenerFMod::CListenerFMod() //: IListener()
:	_Pos(CVector::Null), _Vel(CVector::Null), _Front(CVector::J), _Up(CVector::K)
{
	_RolloffFactor= 1.f;
	_Pos= CVector::Null;
	_Vel= CVector::Null;
	_Front= CVector::J;
	_Up= CVector::K;
	if (CSoundDriverFMod::getInstance()->getOption(ISoundDriver::OptionManualRolloff))
	{
		// Manual RollOff => disable API rollOff
		if( CSoundDriverFMod::getInstance()->fmodOk() )
		{
			FSOUND_3D_SetRolloffFactor(0);
		}
	}
}


// ***************************************************************************
CListenerFMod::~CListenerFMod()
{
	//nldebug("Destroying FMod listener");

    release();
}


// ***************************************************************************
void CListenerFMod::release()
{
}


// ***************************************************************************
void CListenerFMod::setPos( const NLMISC::CVector& pos )
{
	_Pos = pos;
	updateFModPos();
}


// ***************************************************************************
const NLMISC::CVector &CListenerFMod::getPos() const
{
	return _Pos;
}


// ***************************************************************************
void CListenerFMod::setVelocity( const NLMISC::CVector& vel )
{
	_Vel= vel;
	updateFModPos();
}


// ***************************************************************************
void CListenerFMod::getVelocity( NLMISC::CVector& vel ) const
{
	vel= _Vel;
}


// ***************************************************************************
void CListenerFMod::setOrientation( const NLMISC::CVector& front, const NLMISC::CVector& up )
{
	_Front= front;
	_Up= up;
	updateFModPos();
}


// ***************************************************************************
void CListenerFMod::getOrientation( NLMISC::CVector& front, NLMISC::CVector& up ) const
{
	front= _Front;
	up= _Up;
}


// ***************************************************************************
void CListenerFMod::setGain( float gain )
{
	CSoundDriverFMod::getInstance()->setGain(gain);
}


// ***************************************************************************
float CListenerFMod::getGain() const
{
    return CSoundDriverFMod::getInstance()->getGain();
}


// ***************************************************************************
void CListenerFMod::setDopplerFactor( float f )
{
	if( !CSoundDriverFMod::getInstance()->fmodOk() )
		return;

	// clamp as in DSound.
	clamp(f, 0.f, 10.f);

	// set
	FSOUND_3D_SetDopplerFactor(f);
}


// ***************************************************************************
void CListenerFMod::setRolloffFactor( float f )
{
	// Works only in API rolloff mode
	if (CSoundDriverFMod::getInstance()->getOption(ISoundDriver::OptionManualRolloff))
		nlerror("OptionManualRolloff");
	else
	{
		if(!CSoundDriverFMod::getInstance()->fmodOk())
			return;

		// clamp as in DSound (FMod requirement)
		clamp(f, 0.f, 10.f);

		_RolloffFactor = f;

		// set
		FSOUND_3D_SetRolloffFactor(f);
	}
}


// ***************************************************************************
float CListenerFMod::getRolloffFactor()
{
	return _RolloffFactor;
}


// ***************************************************************************
void CListenerFMod::setEnvironment( uint /* env */, float /* size */ )
{
	// TODO_EAX
}


// ***************************************************************************
void CListenerFMod::setEAXProperty( uint /* prop */, void * /* value */, uint /* valuesize */ )
{
	// TODO_EAX
}


// ***************************************************************************
void CListenerFMod::updateFModPos()
{
	// recompute matrix (for Source relative mode)
	// same orientation
	_PosMatrix.identity();
	_PosMatrix.setRot(CVector::I, _Front, _Up);
	_PosMatrix.normalize(CMatrix::YZX);
	_VelMatrix= _PosMatrix;
	// special position
	_PosMatrix.setPos(_Pos);
	_VelMatrix.setPos(_Vel);

	// set up FMod attributes
	float		pos[3];
	float		vel[3];
	float		front[3];
	float		up[3];
	CVector		ful= _Front.normed(), tul= _Up.normed();
	CSoundDriverFMod::toFModCoord(_Pos, pos);
	CSoundDriverFMod::toFModCoord(_Vel, vel);
	CSoundDriverFMod::toFModCoord(ful, front);
	CSoundDriverFMod::toFModCoord(tul, up);
	FSOUND_3D_Listener_SetAttributes(pos, vel, front[0], front[1], front[2], up[0], up[1], up[2]);
}


} // NLSOUND
