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

#ifndef NLSOUND_EFFECT_AL_H
#define NLSOUND_EFFECT_AL_H

namespace NLSOUND {
	class CSoundDriverAL;

/**
 * \brief CEffectAL
 * \date 2008-09-25 08:21GMT
 * \author Jan Boon (Kaetemi)
 * CEffectAL
 */
class CEffectAL
{
protected:
	// outside pointers
	CSoundDriverAL *_SoundDriver;
	
	// instances
	ALuint _AlEffect;
	ALuint _AlAuxEffectSlot;
	
public:
	CEffectAL(CSoundDriverAL *soundDriver, ALuint alEffect, ALuint alAuxEffectSlot);
	virtual ~CEffectAL();
	virtual void release();
	
	inline ALuint getAlEffect() { return _AlEffect; }
	inline ALuint getAuxEffectSlot() { return _AlAuxEffectSlot; }
	
}; /* class CEffectAL */

/**
 * \brief CStandardReverbEffectAL
 * \date 2008-09-15 23:09GMT
 * \author Jan Boon (Kaetemi)
 * CStandardReverbEffectAL
 */
class CStandardReverbEffectAL : public IReverbEffect, public CEffectAL
{
public:
	CStandardReverbEffectAL(CSoundDriverAL *soundDriver, ALuint alEffect, ALuint alAuxEffectSlot);
	virtual ~CStandardReverbEffectAL();
	
	virtual void setEnvironment(const CEnvironment &environment = CEnvironment(), float roomSize = 100.0f);
	
}; /* class CReverbEffectAL */

#if EFX_CREATIVE_AVAILABLE

/**
 * \brief CCreativeReverbEffectAL
 * \date 2008-09-15 23:09GMT
 * \author Jan Boon (Kaetemi)
 * CCreativeReverbEffectAL
 */
class CCreativeReverbEffectAL : public IReverbEffect, public CEffectAL
{
public:
	CCreativeReverbEffectAL(CSoundDriverAL *soundDriver, ALuint alEffect, ALuint alAuxEffectSlot);
	virtual ~CCreativeReverbEffectAL();
	
	virtual void setEnvironment(const CEnvironment &environment = CEnvironment(), float roomSize = 100.0f);
	
}; /* class CReverbEffectAL */

#endif /* #if EFX_CREATIVE_AVAILABLE */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_EFFECT_AL_H */

/* end of file */
