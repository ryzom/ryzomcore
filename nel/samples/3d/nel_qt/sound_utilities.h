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

#ifndef NLQT_SOUND_UTILITIES_H
#define NLQT_SOUND_UTILITIES_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes

namespace NLSOUND {
	class UAudioMixer;
	class CSoundAnimManager;
}

namespace NLQT {
	class CConfiguration;
	class CInternationalization;
	class CGraphicsViewport;
	//class CLandscapeUtilities;
	//class CPacsUtilities;

/**
 * CSoundUtilities
 * \brief CSoundUtilities
 * \date 2010-02-06 12:26GMT
 * \author Jan Boon (Kaetemi)
 */
class CSoundUtilities
{
public:
	CSoundUtilities();
	virtual ~CSoundUtilities();
	
	void init(CConfiguration *configuration, CInternationalization *internationalization);
	void release();
	
	void initGraphics(CGraphicsViewport *graphicsViewport);
	void releaseGraphics();
	
	//void initLandscape(CLandscapeUtilities *landscapeUtilities);
	//void releaseLandscape();
	
	//void initPacs(CPacsUtilities *pacsUtilities);
	//void releasePacs();

	void updateSound();

	inline NLSOUND::UAudioMixer *getAudioMixer() { return m_AudioMixer; }
	inline NLSOUND::CSoundAnimManager *getSoundAnimManager() { return m_SoundAnimManager; }
	
private:
	CConfiguration *m_Configuration;
	CInternationalization *m_Internationalization;
	CGraphicsViewport *m_GraphicsViewport;
	//CLandscapeUtilities *m_LandscapeUtilities;
	//CPacsUtilities *m_PacsUtilities;

	NLSOUND::UAudioMixer *m_AudioMixer;
	NLSOUND::CSoundAnimManager *m_SoundAnimManager;
	
private:
	CSoundUtilities(const CSoundUtilities &);
	CSoundUtilities &operator=(const CSoundUtilities &);
	
}; /* class CSoundUtilities */

} /* namespace NLQT */

#endif /* #ifndef NLQT_SOUND_UTILITIES_H */

/* end of file */
