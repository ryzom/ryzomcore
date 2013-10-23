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

#include "DialogFlags.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/rgba.h"
#include "nel/misc/entity_id.h"
#include "../world_editor/display.h"
#include "../../../../common/src/game_share/mode_and_behaviour.h"


#define REGKEY_BASE_PATH "Software\\Nevrax\\nel\\world_editor_shard_monitor_plugin"

// 
struct CServerParams
{
	uint32 Version;
	bool   LoginRequired;
};


struct CConnectionMsg
{
	enum	TMsgType { NoMsg = 0, AuthentValid, AuthentInvalid, ServerParamsMsg };
	TMsgType		MsgType;
	CServerParams	ServerParams; // filled when msgtype is 'ServerParams'
};

class CPlugin : public IPluginCallback
{
public:	

	enum TCloseUpFlag
	{
		CloseUpEntityType = 0,
		CloseUpEntityMode,
		CloseUpEntityHP,
		CloseUpFlagCount
	};

	
	friend void serverSentAdd (NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);
	friend void serverSentPos (NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);
	friend void serverSentRemove (NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);
	friend void serverSentString (NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);
	friend void serverSentMiscProp (NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);
	
	/// Constructor
	CPlugin();
	/// Destructor
	~CPlugin();

	// Connect disconnect
	void connectDisconnect();

	virtual bool		isActive();

	virtual bool		activatePlugin();

	virtual bool		closePlugin();

	virtual std::string& getName();

	void setEntityDisplayMode(TEntityDisplayMode dm);

	// update colors for all entities
	void updateDisplay();

	// post render part
	virtual void		postRender(CDisplay &display);

	// display hits on entity 
	void				setDisplayHitsFlag(bool on) { _DisplayHits = on; }

	
	void				setCloseUpFlag(TCloseUpFlag flag, bool on);

	// Set distance at which close-up details are displayed
	void				setCloseUpDisplayDistance(float metersPerPixel) { _CloseUpDisplayDistance = metersPerPixel; updateDisplay(); }


	IPluginAccess		*getPluginAccess() const { return _PluginAccess; }


	void				serverParamsReceived(const CServerParams &sp);

	void				authentValid();
	void				authentInvalid();

	// set current connection related msg
	void				setConnectionMsg(const CConnectionMsg &cm) { _ConnectionMsg = cm; }

private:

	/// An entity entry
	class CEntityEntry
	{
	public:
		CEntityEntry ()
		{
			Primitive = NULL;			
			ColorDirty = true;
			HitPoints = 0;
			MaxHitPoints = 0;
			Hidden = false;
			Mode = MBEHAV::UNKNOWN_MODE;
		}		
		NLMISC::CEntityId		EntityId;
		NLLIGO::CPrimPoint		*Primitive;
		NLMISC::CRGBA			CurrentColor;
		bool                    ColorDirty;
		sint32                  HitPoints;
		sint32                  MaxHitPoints;		
		MBEHAV::EMode			Mode;
		bool                    Hidden; // reminder for hidden flag
		// cache for the type icon (if any)
		CEntityIcon				IconForEntityType;
		// cache for the mode icon (if any)
		CEntityIcon				IconForEntityMode;
	};

	// Current host
	std::string		_SHost;

	// Root primitive index
	NLLIGO::IPrimitive *_Root;

	// Error messages
	bool yesNoMessage (const char *format, ... );
	void errorMessage (const char *format, ... );
	void infoMessage (const char *format, ... );

	// Connect disconnect
	void updateConnectionState();
	
	// @{
	// \name Overload for IPluginCallback
	virtual void		init(IPluginAccess *pluginAccess);

	/// The current region has changed.
	virtual void		primitiveChanged(const NLLIGO::IPrimitive *root);

	/// The region has been modified.
	/// The listener has been moved on the map.
	virtual void		positionMoved(const NLMISC::CVector &position);
	/// The plug-in lost the control of the position
	virtual void		lostPositionControl();

	// update color & visibility of an entity
	void				updateEntityAspect(CEntityEntry &entity);	

	// @}

	// Remove an entity primitive
	void removeEntity (CEntityEntry &entity);

	// Set an entity primitive name
	void setPrimitiveName (NLLIGO::IPrimitive *primitive, uint32 stringId);

	// callback for mixer init progression
	void progress		(float progressValue);

	/// The plug-in dialog.
	CDialogFlags	*_DialogFlag;

	/// The network callback
	NLNET::CCallbackClient *_Client;

	CConnectionMsg			_ConnectionMsg;	

	/// the position of the listener.
	virtual void onIdle();

	bool	_PluginActive;

	bool	m_Initialized;

	bool    _DisplayHits;

	// The entity vector
	std::vector<CEntityEntry>	_Entites;

	// The plugin acces
	IPluginAccess	*_PluginAccess;

	// Primitive to string id map
	std::map<NLLIGO::IPrimitive*, uint32>	_PrimitiveToString;

	// String id to primitive waiting there string id multimap
	std::multimap<uint32, NLLIGO::IPrimitive*>	_StringToPrimitive;

	// String id to string map
	std::map<uint32, std::string>	_StringIdToString;

	// Current window coordinates
	NLMISC::CVector					_WMin, _WMax;

	// Request Download
	uint							_RequestedDownload;
	uint							_CtrlRequestedDownload;

	TEntityDisplayMode				_EntityDisplayMode;
	
	NLMISC::TTime					_CurrentTime;

	// A hit on an entity. All hits are displayed on the map	
	struct CHit
	{
		uint			EntityId;
		NLMISC::TTime	HitTime;
	};

	/// list of hits
	std::list<CHit>		_HitList;

	CPrimTexture		*_EntityIcons;

	bool				_CloseUpFlags[CloseUpFlagCount];

	float				_CloseUpDisplayDistance;

	// display hits on entities
	void displayHits(CDisplay &display);
	// display an icon & update position for next display
	void pushIcon(CDisplay &display, sint stepX, sint stepY, NLMISC::CVector &currPos, const CEntityIcon &icon, const CPrimTexture &pt);
	// display close-up icons for all entities
	void displayCloseUp(CDisplay &display);
	
	
	enum TConnectState
	{
		Disconnected = 0,
		WaitingServerParams,
		WaitingLoginConfirmation,
		Authentificated
	};

	TConnectState _ConnectState;	

};

extern "C"
{
	/// Export the C factory method for dynamic linking..
	__declspec( dllexport ) void *createPlugin();
}

// geta string from string table
CString getStringRsc(UINT strID);

