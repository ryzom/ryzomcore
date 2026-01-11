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

#include "nel/sound/u_audio_mixer.h"
#include "nel/ligo/primitive.h"
#include "nel/misc/progress_callback.h"
#include "../world_editor/plugin_interface.h"
#include "DialogFlags.h"
#include "nel/sound/u_audio_mixer.h"
#include "LoadDialog.h"


class CSoundPlugin : public IPluginCallback, public NLMISC::IProgressCallback
{
public:
	/// Constructor
	CSoundPlugin();
	/// Destructor
	~CSoundPlugin();

	void		play();
	void		stop();
	void		startMoveEar();
	void		stopMoveEar();

	void		update();
//	uint		getLoadedSampleSize();
	
	std::string getStatusString();

	void getLoadedSampleBankInfo(std::vector<std::pair<std::string, uint> > &result);
	void ReInit();

	NLSOUND::UAudioMixer	*getMixer()				{ return _Mixer; };
	IPluginAccess			*getPlugingAccess()		{ return _PluginAccess; };
	void		onIdle();

	virtual bool isActive();

	virtual bool activatePlugin();

	virtual bool closePlugin();

	std::string& getName();
	
private:
	// @{
	// \name Overload for IPluginCallback
	virtual void		init(IPluginAccess *pluginAccess);

	/// The current region has changed.
//	virtual void		primRegionChanged(const std::vector<NLLIGO::CPrimRegion*> &regions);
	virtual void		primitiveChanged(const NLLIGO::IPrimitive *root);

	/// The region has been modifed.
//	virtual void		primRegionModifed();
	/// The listener has been moved on the map.
	virtual void		positionMoved(const NLMISC::CVector &position);
	/// The plugin lost the control of the position
	virtual void		lostPositionControl();
	// @}



	std::string _PluginName;

	bool _PluginActive;

	// callback for mixer init progression
	void progress		(float progressValue);
	// Dialog for mixer init progression display
	CLoadDialog			*LoadDlg;

	/// The world editor interface.
	IPluginAccess		*_PluginAccess;


	/// Sound plugin tool bat.
//	CToolBar		_Toolbar;

	/// Sound plugin dialog.
	CDialogFlags	*_DialogFlag;

	NLSOUND::UAudioMixer	*_Mixer;

	/// the position of the listener.
	NLMISC::CVector			_ListenerPos;

	/// If play is enabled or not
	bool					_BackgroundSoundPlayed;
};

extern "C"
{
	/// Export the C factory method for dynamic linking..
	//__declspec( dllexport ) void *createSoundPlugin();
	__declspec( dllexport ) void *createPlugin();

}