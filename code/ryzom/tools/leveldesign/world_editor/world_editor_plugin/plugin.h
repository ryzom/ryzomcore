
#include "nel/sound/u_audio_mixer.h"
#include "nel/ligo/primitive.h"
#include "nel/misc/progress_callback.h"
#include "../world_editor/plugin_interface.h"
#include "DialogFlags.h"
#include "nel/sound/u_audio_mixer.h"
#include "LoadDialog.h"

class CPlugin : public IPluginCallback
{
public:
	/// Constructor
	CPlugin();
	/// Destructor
	~CPlugin();
private:
	// @{
	// \name Overload for IPluginCallback
	virtual void		init(IPluginAccess *pluginAccess);

	/// The current region has changed.
	virtual void		primitiveChanged(const NLLIGO::IPrimitive *root);

	/// The region has been modifed.
	/// The listener has been moved on the map.
	virtual void		positionMoved(const NLMISC::CVector &position);
	/// The plugin lost the control of the position
	virtual void		lostPositionControl();
	// @}

	// callback for mixer init progression
	void progress		(float progressValue);
	// Dialog for mixer init progression display
	//CLoadDialog			*LoadDlg;

	/// The world editor interface.
	IPluginAccess		*_PluginAccess;

	/// Sound plugin dialog.
	CDialogFlags	*_DialogFlag;

	//NLSOUND::UAudioMixer	*_Mixer;

	/// the position of the listener.
	//NLMISC::CVector			_ListenerPos;
	virtual void onIdle();

	bool	m_Initialized;
};

extern "C"
{
	/// Export the C factory method for dynamic linking..
	__declspec( dllexport ) void *createPlugin();
}
