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

#ifndef NLSOUND_EFFECT_XAUDIO2_H
#define NLSOUND_EFFECT_XAUDIO2_H

namespace NLSOUND {
	class CSoundDriverXAudio2;

/**
 * \brief CEffectXAudio2
 * \date 2008-09-25 07:46GMT
 * \author Jan Boon (Kaetemi)
 * CEffectXAudio2
 */
class CEffectXAudio2
{
protected:
	// outside pointers
	CSoundDriverXAudio2 *_SoundDriver;

	// pointers
	IXAudio2SubmixVoice *_DryVoice;
	IXAudio2SubmixVoice *_FilterVoice;
	IUnknown *_Effect; // set by subclass

public:
	CEffectXAudio2(CSoundDriverXAudio2 *soundDriver, uint channels);
	virtual ~CEffectXAudio2();
	virtual void release();

	inline IUnknown *getEffect() { return _Effect; }
	inline IXAudio2Voice *getDryVoice() { return _DryVoice; }
	inline IXAudio2Voice *getFilterVoice() { return _FilterVoice; }

}; /* class CEffectXAudio2 */

/**
 * \brief CReverbEffectXAudio2
 * \date 2008-09-17 17:27GMT
 * \author Jan Boon (Kaetemi)
 * CReverbEffectXAudio2
 */
class CReverbEffectXAudio2 : public IReverbEffect, public CEffectXAudio2
{
protected:
	// user data
	/// Parameters of the reverb (eax environment) effect.
	XAUDIO2FX_REVERB_PARAMETERS _ReverbParams;

public:
	CReverbEffectXAudio2(CSoundDriverXAudio2 *soundDriver);
	virtual ~CReverbEffectXAudio2();
	virtual void release();
	
	/// Set the environment (you have full control now, have fun)
	virtual void setEnvironment(const CEnvironment &environment = CEnvironment(), float roomSize = 100.0f);

}; /* class CReverbEffectXAudio2 */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_EFFECT_XAUDIO2_H */

/* end of file */
