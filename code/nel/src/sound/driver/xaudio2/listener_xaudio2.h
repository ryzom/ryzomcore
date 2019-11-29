// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2008  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NLSOUND_LISTENER_XAUDIO2_H
#define NLSOUND_LISTENER_XAUDIO2_H

#include "nel/sound/driver/listener.h"
#include "nel/sound/driver/sound_driver.h"

namespace NLSOUND {
	class CSoundDriverXAudio2;

/**
 * \brief CListenerXAudio2
 * \date 2008-08-20 12:32GMT
 * \author Jan Boon (Kaetemi)
 * CListenerXAudio2 is an implementation of the IListener interface to run on XAudio2.
 * TODO: For occlusion reverb output gain must be controllable per voice (source send with lower gain to reverb submix).
 */
class CListenerXAudio2 : public IListener, public NLMISC::CManualSingleton<CListenerXAudio2>
{
protected:
	// outside pointers
	CSoundDriverXAudio2 *_SoundDriver;

	// pointers
	/// Submix voice for volume change, also direct sample input.
	IXAudio2SubmixVoice *_DryVoice;
	/// Dummy passtrough submix voice for the setting up the filtered send
	IXAudio2SubmixVoice *_FilterVoice;
	
	// instances
	/// X3DAudio data for listener position in space.
	X3DAUDIO_LISTENER _Listener;
	/// NeL Position used for manual rolloff calculation.
	NLMISC::CVector _Pos;
	/// If the listener initialized correctly.
	bool _ListenerOk;

	// user vars
	/// Doppler scaler, set by user
	float _DopplerScaler;
	/// Distance/Roloff scaler
	float _RolloffScaler;
public:
	CListenerXAudio2(CSoundDriverXAudio2 *soundDriver);
	virtual ~CListenerXAudio2();
	void release();

	inline CSoundDriverXAudio2 *getSoundDriver() { return _SoundDriver; }
	inline X3DAUDIO_LISTENER *getListener() { return &_Listener; }
	inline IXAudio2SubmixVoice *getDryVoice() { return _DryVoice; }
	inline IXAudio2SubmixVoice *getFilterVoice() { return _FilterVoice; }
	inline float getDopplerScaler() { return _DopplerScaler; }
	inline float getRolloffScaler() { return _RolloffScaler; }

	/// \name Listener properties
	//@{
	/// Set the position vector (default: (0,0,0)) (3D mode only)
	virtual void setPos(const NLMISC::CVector& pos);
	/** Get the position vector.
	 * See setPos() for details.
	 */
	virtual const NLMISC::CVector &getPos() const;
	/// Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
	virtual void setVelocity(const NLMISC::CVector& vel);
	/// Get the velocity vector
	virtual void getVelocity(NLMISC::CVector& vel) const;
	/// Set the orientation vectors (3D mode only, ignored in stereo mode) (default: (0,1,0), (0,0,-1))
	virtual void setOrientation(const NLMISC::CVector& front, const NLMISC::CVector& up);
	/// Get the orientation vectors
	virtual void getOrientation(NLMISC::CVector& front, NLMISC::CVector& up) const;
	/** Set the gain (volume value inside [0 , 1]). (default: 1)
	 * 0.0 -> silence
	 * 0.5 -> -6dB
	 * 1.0 -> no attenuation
	 * values > 1 (amplification) not supported by most drivers
	 */
	virtual void setGain(float gain);
	/// Get the gain
	virtual float getGain() const;
	//@}

	/// \name Global properties
	//@{
	/// Set the doppler factor (default: 1) to exaggerate or not the doppler effect
	virtual void setDopplerFactor(float f);
	/// Set the rolloff factor (default: 1) to scale the distance attenuation effect
	virtual void setRolloffFactor(float f);
	//@}
}; /* class CListenerXAudio2 */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_LISTENER_XAUDIO2_H */

/* end of file */
