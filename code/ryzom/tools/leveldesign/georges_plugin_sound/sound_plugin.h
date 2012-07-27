// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef _NLGEORGES_SOUND_PLUGIN_H
#define _NLGEORGES_SOUND_PLUGIN_H

#include "std_sound_plugin.h"

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "sound_dialog.h"

#include "../georges_dll/plugin_interface.h"

#include "nel/misc/vector.h"

#include "nel/sound/driver/sound_driver.h"
#include "nel/sound/driver/source.h"
#include "nel/sound/driver/listener.h"
// #include "sound/driver/dsound/source_dsound.h"

#include "nel/sound/u_audio_mixer.h"
#include "nel/sound/u_listener.h"


class NLSOUND::IBuffer;
class NLSOUND::IListener;
class NLSOUND::USource;
// class NLSOUND::CSourceDSound;
class NLSOUND::CSound;



namespace NLGEORGES
{


class CSoundPlugin : public IEditPlugin
{
public:
	// From IEditPlugin
	CSoundPlugin(NLGEORGES::IEdit	*globalInterface); 
	virtual ~CSoundPlugin();

	// Overriden methods
	virtual void			dialogInit(HWND mainFrm);
	virtual bool			pretranslateMessage(MSG *pMsg);
	virtual void			onCreateDocument(IEditDocument *document);
	virtual void			activate(bool activate);
	virtual void			getPluginName(std::string &name); 

	// Source control
	virtual void			play(std::string &filename); 
	virtual void			play()  { play(_Filename); }; 
	virtual void			stop(); 
	virtual uint32			getTime();

	void					update();
	bool					isPlaying();

	// Source parameters
	virtual void			setListenerPos(const NLMISC::CVector& pos)						{ _Listener->setPos(pos); /*commit();*/ }
	virtual void			setListenerOrientation(const NLMISC::CVector& front, const NLMISC::CVector& up)						{ _Listener->setOrientation(front, up); /*commit(); */}
	virtual void			setDirection(const NLMISC::CVector& dir)				{ /*_Source->setDirection(dir);*/ commit(); }
	virtual void			setGain(float gain)										{ /*_Source->setGain(gain);*/ commit(); }
	virtual void			setPitch(float pitch)									{ /*_Source->setPitch(pitch);*/ commit(); }
	virtual void			setMinMaxDistances(float mindist, float maxdist)		{ /*_Source->setMinMaxDistances(mindist, maxdist);*/ commit(); _Dialog.setMinMaxDistances(mindist, maxdist); }
	virtual void			setCone(float inner, float outer, float outerGain)		{ /*_Source->setCone(inner, outer, outerGain);*/ commit(); }
	virtual void			setLoop(bool v)											{ /*_Source->setLooping(v);*/ }
	virtual void			commit()												{ /*_SoundDriver->commit3DChanges(); */}
	virtual void			setAlpha(double alpha)									{ /*_Source->setAlpha(alpha);*/ _Dialog.setAlpha(alpha); }

	// Dialog display
	virtual void			setName(std::string& name)								{ _Dialog.setName(name); }
	virtual void			setFilename(std::string& filename)						{ _Filename = filename; _Dialog.setFilename(filename); }
	virtual void			setAngles(uint32 inner, uint32 outer)					{ _Dialog.setAngles(inner, outer); }

	// Form creation
	virtual void			createNew();

	// Boris : hum, simpler ?
	void					setActiveDocument(IEditDocument *pdoc);
	IEditDocument			*getActiveDocument()									{ return _ActiveDoc; }

	/// Ask if the sound has an alpha info (ie simple sound).
	bool					hasAlpha();

	bool					isSoundValid()											{ return !_InvalidSound;}

	/// The dialog update the environnement flags.
	void					updateEnvFlags(const NLSOUND::UAudioMixer::TBackgroundFlags &backgroundFlags);

	// Return the global interface (for dialog)
	IEdit					*getGlobalInterface()									{ return _GlobalInterface; }

	void					reloadSamples();
	void					reloadSounds();

	void					updateDisplay();

	NLSOUND::UAudioMixer	*getMixer()			{ return _Mixer;}

	NLSOUND::USource		*getSource()		{ return _Source; }
	NLSOUND::CSound			*getSound()			{ return _Sound; }

private:

	bool					checkSound(NLSOUND::CSound *sound, const std::vector<std::pair<std::string, NLSOUND::CSound*> > &subsounds, std::vector<std::string> &missingFiles);

	IEdit					*_GlobalInterface;
	CSoundDialog			_Dialog;
//	NLSOUND::ISoundDriver	*_SoundDriver;
//	NLSOUND::IBuffer		*_Buffer;
	NLSOUND::UListener		*_Listener;
	NLSOUND::USource		*_Source;
	NLSOUND::CSound			*_Sound;
	bool					_FreeSound;
	bool					_PlayBackground;
	std::string				_Filename;

	NLSOUND::CSoundContext	_Context;

	/// Flag for sound with infinite recursion !
	bool					_InvalidSound;

	IEditDocument			*_ActiveDoc;
	NLSOUND::UAudioMixer	*_Mixer;

};


} // namespace NLGEORGES

#endif // _NLGEORGES_SOUND_PLUGIN_H