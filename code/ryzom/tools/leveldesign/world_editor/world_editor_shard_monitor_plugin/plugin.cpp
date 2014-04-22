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

#include "stdafx.h"
#include "nel/misc/time_nl.h"
#include "plugin.h"
#include "resource.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/time_nl.h"
#include "DialogLogin.h"
#include <deque>

using namespace NLMISC;
using namespace NLLIGO;
using namespace NLSOUND;
using namespace NLNET;
using namespace std;

CFileDisplayer		*ShardMonitorPluginLogDisplayer= NULL;

// vl: important to add the next line or AfxGetApp() will returns 0 after AFX_MANAGE_STATE(AfxGetStaticModuleState());
CWinApp theApp;


// ***************************************************************************
extern "C"
{
	void *createPlugin()
	{
		return new CPlugin();
	}
}

// ***************************************************************************

CPlugin::CPlugin()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	NLMISC::createDebug();
	ShardMonitorPluginLogDisplayer= new CFileDisplayer("world_editor_shard_monitor_plugin.log", true, "WORLD_EDITOR_SHARD_MONITOR_PLUGIN.LOG");
	DebugLog->addDisplayer (ShardMonitorPluginLogDisplayer);
	InfoLog->addDisplayer (ShardMonitorPluginLogDisplayer);
	WarningLog->addDisplayer (ShardMonitorPluginLogDisplayer);
	ErrorLog->addDisplayer (ShardMonitorPluginLogDisplayer);
	AssertLog->addDisplayer (ShardMonitorPluginLogDisplayer);

	nlinfo("Starting shard monitor plugin...");
	
	_DialogFlag = new CDialogFlags(NULL);
	m_Initialized = false;
	_Client = NULL;
	_Root = NULL;
	_RequestedDownload = 0;
	_EntityDisplayMode = EntityType;
	_DisplayHits = true;

	_EntityIcons = NULL;

	_CloseUpDisplayDistance = 10.f;

	std::fill(_CloseUpFlags, _CloseUpFlags + sizeofarray(_CloseUpFlags), true);

	_ConnectState = Disconnected;

}

// ***************************************************************************

CPlugin::~CPlugin()
{
	if (m_Initialized)
	{
		_PluginAccess->deleteRootPluginPrimitive();
		_Root = NULL;
		m_Initialized = false;
		if (_PluginAccess)
		{
			_PluginAccess->deleteTexture(_EntityIcons);
		}
	}
}

// ***************************************************************************
void CPlugin::progress (float progressValue)
{
}

// ***************************************************************************

void CPlugin::removeEntity (CEntityEntry &entity)
{
	// Changes ?
	if (entity.Primitive)
	{
		std::map<NLLIGO::IPrimitive*, uint32>::iterator ite = _PrimitiveToString.find (entity.Primitive);
		if(ite != _PrimitiveToString.end())
		{
			uint32 stringId = (*ite).second;
			_PrimitiveToString.erase(entity.Primitive);

			std::multimap<uint32, NLLIGO::IPrimitive*>::iterator ite2 = _StringToPrimitive.find (stringId);
			while ((ite2 != _StringToPrimitive.end()) && (ite2->first == stringId))
			{
				std::multimap<uint32, NLLIGO::IPrimitive*>::iterator ite2delete = ite2;
				ite2++;
				_StringToPrimitive.erase(ite2delete);
			}
		}

		_PluginAccess->deletePluginPrimitive (entity.Primitive);
		entity.Primitive = NULL;
		entity.ColorDirty = true;
	}
}

/** Map a color of an entity to its aspect (color & icon)
  * \param di array giving the aspect for several value of the property.
  * \param value value for which aspect is queried
  * \param defaultValue value of the property that gives the default aspect
  */
static void getEntityColorFromValue(const TEntityDisplayInfoVect &di, uint value, uint defaultValue, CRGBA &destColor)
{
	CRGBA			defaultCol(127, 127, 127);		
	for(uint k = 0; k < di.size(); ++k)
	{
		if (di[k].Value == defaultValue)
		{
			defaultCol = di[k].Color;			
		}
		else if (di[k].Value == value)
		{
			destColor = di[k].Color;			
			return;
		}
	}
	destColor = defaultCol;	
}	


static void getEntityIconFromValue(const TEntityDisplayInfoVect &di, uint value, uint defaultValue, CEntityIcon &destIcon)
{	
	CEntityIcon		defaultIcon;
	for(uint k = 0; k < di.size(); ++k)
	{
		if (di[k].Value == defaultValue)
		{
			defaultIcon = di[k].Icon;
		}
		else if (di[k].Value == value)
		{		
			destIcon = di[k].Icon;
			return;
		}
	}	
	destIcon  = defaultIcon;
}

static bool getEntityVisibilityFromValue(const TEntityDisplayInfoVect &di, uint value)
{	
	for(uint k = 0; k < di.size(); ++k)
	{		
		if (di[k].Value == value)
		{
			return di[k].Visible;
		}
	}
	return true;
}

// ***************************************************************************
void CPlugin::updateEntityAspect(CEntityEntry &entity)
{	
	if (!entity.Primitive) return;
	NLMISC::CRGBA color;
	bool visible = true;
	const TEntityDisplayInfoVect &di = _DialogFlag->getEntityDisplayInfos(_EntityDisplayMode);
	// update icons
	getEntityIconFromValue(_DialogFlag->getEntityDisplayInfos(EntityType), (uint) entity.EntityId.getType(), RYZOMID::unknown, entity.IconForEntityType);
	getEntityIconFromValue(_DialogFlag->getEntityDisplayInfos(EntityMode), (uint) entity.Mode, MBEHAV::UNKNOWN_MODE, entity.IconForEntityMode);
	//
	switch(_EntityDisplayMode)
	{
		case EntityType:	
			getEntityColorFromValue(di, (uint) entity.EntityId.getType(), RYZOMID::unknown, color);
			visible = getEntityVisibilityFromValue(di, (uint) entity.EntityId.getType());
		break;
		case EntityMode:
			getEntityColorFromValue(di, (uint) entity.Mode, MBEHAV::UNKNOWN_MODE, color);
			visible = getEntityVisibilityFromValue(di, (uint) entity.EntityId.getType());
		break;
		case EntityHitPoints:
		{			
			if (entity.MaxHitPoints == 0)
			{
				color = di[3].Color;
			}
			else
			{
				float ratio = (float) entity.HitPoints / (float) entity.MaxHitPoints;
				if (ratio > 0.5f)
				{
					color.blendFromui(di[1].Color, di[2].Color, (uint8) (255.f * 2.f * (ratio - 0.5f)));
				}
				else
				{
					color.blendFromui(di[0].Color, di[1].Color, (uint8) (255.f * 2.f * ratio));
				}
			}
		}
		break;
		case EntityAlive:
		{			
			if (entity.MaxHitPoints == 0)
			{
				color = di[2].Color;
			}
			else
			{
				if (entity.HitPoints == 0) color = di[0].Color;
				else color = di[1].Color;
			}
		}
		break;
		default:
			nlassert(0);
		break;
	}
	if (color != entity.CurrentColor || entity.ColorDirty)
	{
		nlassert(entity.Primitive);				
		entity.Primitive->removePropertyByName ("Color");
		entity.Primitive->addPropertyByName ("Color", new CPropertyColor(color));	
		_PluginAccess->invalidatePluginPrimitive (entity.Primitive, QuadTree);
		entity.CurrentColor = color;
		entity.ColorDirty = false;		
	}
	entity.Hidden = !visible;
	_PluginAccess->setPrimitiveHideFlag(*entity.Primitive, !visible);

}


// ***************************************************************************
void serverSentAdd (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Get the plugin
	CPlugin *plugin = (CPlugin *)(from->appId());

	// Read the message version
	// Version 1 : added sheet id
	sint version = msgin.serialVersion(1);

	// Called when the server sent a ADD message
	uint32 count;
	msgin.serial(count);

	for (uint i = 0;i < count; i++)
	{	
		uint32 id;
		uint32 stringId;
		CEntityId entityId;
		NLMISC::CSheetId sheetID;
		msgin.serial(id);
		msgin.serial(stringId);
		msgin.serial (entityId);
		if (version >= 1)
		{
			msgin.serial(sheetID);
		}		

		// Resize the player array
		if (id >= plugin->_Entites.size())
			plugin->_Entites.resize (id+1);
		
		std::vector<CPrimitiveClass::CInitParameters> Parameters;
		plugin->_Entites[id].Primitive = (CPrimPoint*)(plugin->_PluginAccess->createPluginPrimitive ("player", "", NLMISC::CVector::Null, 0, Parameters, plugin->_Root));		
		plugin->_Entites[id].EntityId = entityId;

		if (plugin->_Entites[id].Primitive)
		{
			if (stringId)
			{
				plugin->_PrimitiveToString.insert (std::map<NLLIGO::IPrimitive*, uint32>::value_type (plugin->_Entites[id].Primitive, stringId));
				plugin->_StringToPrimitive.insert (std::multimap<uint32, NLLIGO::IPrimitive*>::value_type (stringId, plugin->_Entites[id].Primitive));

				// Set the primitive name
				plugin->setPrimitiveName (plugin->_Entites[id].Primitive, stringId);
			}
			else
			{
				// Set the new name
				plugin->_Entites[id].Primitive->removePropertyByName ("name");
				plugin->_Entites[id].Primitive->addPropertyByName ("name", new CPropertyString (sheetID.toString().c_str()));

				// Invalidate the name
				plugin->_PluginAccess->invalidatePluginPrimitive (plugin->_Entites[id].Primitive, LogicTreeParam);
			}

			// Set the entity id
			plugin->_Entites[id].Primitive->removePropertyByName ("entity id");
			plugin->_Entites[id].Primitive->addPropertyByName ("entity id", new CPropertyString (entityId.toString().c_str()));

			// Set the sheet name
			plugin->_Entites[id].Primitive->removePropertyByName ("Sheet");
			plugin->_Entites[id].Primitive->addPropertyByName ("Sheet", new CPropertyString (sheetID.toString().c_str()));

			// set start color
			plugin->updateEntityAspect(plugin->_Entites[id]);
		}
	}
}

// ***************************************************************************

void serverSentPos (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Get the plugin
	CPlugin *plugin = (CPlugin *)(from->appId());

	// Read the message version
	sint version = msgin.serialVersion(0);

	// Called when the server sent a POS message
	uint32 count;
	msgin.serial(count);

	for (uint i = 0;i < count; i++)
	{	
		uint32 id;
		msgin.serial(id);

		// Read position
		CPlugin::CEntityEntry &entity = plugin->_Entites[id];
		float x, y, theta;
		msgin.serial (x);
		msgin.serial (y);
		msgin.serial (theta);

		// Changes ?
		if (entity.Primitive)
		{
			if ((entity.Primitive->Point.x != x) || (entity.Primitive->Point.y != y) || (entity.Primitive->Angle != theta))
			{
				entity.Primitive->Point.x = x;
				entity.Primitive->Point.y = y;
				entity.Primitive->Angle = theta;
				plugin->_PluginAccess->invalidatePluginPrimitive (entity.Primitive, QuadTree);
			}
		}
	}
}

// ***************************************************************************

void serverSentMiscProp (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Get the plugin
	CPlugin *plugin = (CPlugin *)(from->appId());

	// Read the message version
	sint version = msgin.serialVersion(0);

	// Called when the server sent a HIT_POINTS message
	uint32 count;
	msgin.serial(count);

	for (uint i = 0;i < count; i++)
	{	
		uint32 id;
		msgin.serial(id);

		// Read hitpoints
		CPlugin::CEntityEntry &entity = plugin->_Entites[id];
		sint32 hitPoints;
		sint32 maxHitPoints;		
		uint8  mode;
		uint8  behaviour;
		msgin.serial (hitPoints);
		msgin.serial (maxHitPoints);
		msgin.serial(mode);
		msgin.serial(behaviour);


		// Changes ?
		if (entity.Primitive)
		{		
			if (hitPoints < entity.HitPoints)
			{				
				CPlugin::CHit hit;
				hit.EntityId = id;
				hit.HitTime = plugin->_CurrentTime;
				plugin->_HitList.push_front(hit);
			}
			entity.HitPoints = hitPoints;
			entity.MaxHitPoints = maxHitPoints;	
			entity.Mode = (MBEHAV::EMode) mode;

			plugin->updateEntityAspect(entity);
			plugin->_Entites[id].Primitive->removePropertyByName ("HitPoints");
			plugin->_Entites[id].Primitive->addPropertyByName ("HitPoints", new CPropertyString (toString(hitPoints)));
			plugin->_Entites[id].Primitive->removePropertyByName ("MaxHitPoints");
			plugin->_Entites[id].Primitive->addPropertyByName ("MaxHitPoints", new CPropertyString (toString(maxHitPoints)));
			plugin->_Entites[id].Primitive->removePropertyByName ("Mode");
			plugin->_Entites[id].Primitive->addPropertyByName ("Mode", new CPropertyString (MBEHAV::modeToString((MBEHAV::EMode) mode)));
			plugin->_Entites[id].Primitive->removePropertyByName ("Behaviour");
			plugin->_Entites[id].Primitive->addPropertyByName ("Behaviour", new CPropertyString (MBEHAV::behaviourToString((MBEHAV::EBehaviour) mode)));
			plugin->_PluginAccess->invalidatePluginPrimitive (plugin->_Entites[id].Primitive, QuadTree);

			// if the primitive is the only one that is selected, then update the property dialog
			const std::list<NLLIGO::IPrimitive*> &currSel = plugin->_PluginAccess->getCurrentSelection();
			if (currSel.size() == 1)
			{
				if (currSel.front() == plugin->_Entites[id].Primitive)
				{
					// properties for this primitive are currently displayed -> refresh
					plugin->_PluginAccess->refreshPropertyDialog();
				}
			}
		}
	}
}

void serverSentParams(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	CPlugin *plugin = (CPlugin *)(from->appId());
	CConnectionMsg cm;
	cm.MsgType = CConnectionMsg::ServerParamsMsg;
	msgin.serial(cm.ServerParams.Version);
	msgin.serial(cm.ServerParams.LoginRequired);
	plugin->setConnectionMsg(cm);
}

void serverSentAuthentValid(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	CPlugin *plugin = (CPlugin *)(from->appId());
	CConnectionMsg cm;
	cm.MsgType = CConnectionMsg::AuthentValid;	
	plugin->setConnectionMsg(cm);
}

void serverSentAuthentInvalid(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	CPlugin *plugin = (CPlugin *)(from->appId());
	CConnectionMsg cm;
	cm.MsgType = CConnectionMsg::AuthentInvalid;
	plugin->setConnectionMsg(cm);
}


// ***************************************************************************
void CPlugin::authentValid()
{
	if (_ConnectState == WaitingLoginConfirmation)
	{
		_ConnectState = Authentificated;
		updateConnectionState();
	}
}

// ***************************************************************************
void CPlugin::authentInvalid()
{
	if (_ConnectState == WaitingLoginConfirmation)
	{		
		if (_Client->connected())
		{
			connectDisconnect();
			updateConnectionState();			
		}
		MessageBox(NULL, getStringRsc(IDS_LOGIN_FAILED), getStringRsc(IDS_ERROR), MB_ICONEXCLAMATION);
	}
}

// ***************************************************************************
void CPlugin::serverParamsReceived(const CServerParams &sp)
{
	nlassert(sp.Version == 0); // version system not used now (since first version ...)
	if (_ConnectState != WaitingServerParams) return;
	if (sp.LoginRequired)
	{
		CDialogLogin dl;
		if (dl.DoModal() == IDOK)
		{
			CMessage msg;
			msg.setType("AUTHENT");
			std::string login = (LPCTSTR)dl.m_Login;
			std::string password = (LPCTSTR)dl.m_Password;
			sint ver = msg.serialVersion(0);
			msg.serial(login);
			msg.serial(password);
			_Client->send(msg);
			_ConnectState = WaitingLoginConfirmation;
			updateConnectionState();
		}
		else
		{
			if (_Client->connected())
			{
				connectDisconnect();
				updateConnectionState();			
			}
		}
	}
	else
	{
		// no need for a password, so we are already authentificated
		_ConnectState = Authentificated;
		updateConnectionState();
	}	
}


// ***************************************************************************
void CPlugin::setEntityDisplayMode(TEntityDisplayMode edm)
{
	if (edm == _EntityDisplayMode) return;
	_EntityDisplayMode = edm;
	updateDisplay();	
}

// ***************************************************************************
void CPlugin::updateDisplay()
{
	for(uint k = 0; k < _Entites.size(); ++k)
	{
		CPlugin::CEntityEntry &entity = _Entites[k];
		if (entity.Primitive)
		{					
			updateEntityAspect(entity);
			_PluginAccess->invalidatePluginPrimitive (entity.Primitive, QuadTree);
		}
	}
}

// ***************************************************************************
void serverSentString (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Get the plugin
	CPlugin *plugin = (CPlugin *)(from->appId());

	// Read the message version
	sint version = msgin.serialVersion(0);

	// Called when the server sent a POS message
	uint32 count;
	msgin.serial(count);

	for (uint i = 0;i < count; i++)
	{	
		uint32 stringId;
		string str;
		msgin.serial(stringId);
		msgin.serial(str);

		// Set the string
		plugin->_StringIdToString.insert (std::map<uint32, std::string>::value_type (stringId, str));

		// Modify entites

		// Changes ?
		std::multimap<uint32, NLLIGO::IPrimitive*>::iterator ite = plugin->_StringToPrimitive.find (stringId);
		while ((ite != plugin->_StringToPrimitive.end()) && (ite->first == stringId))
		{
			// Set the name
			plugin->setPrimitiveName (ite->second, stringId);

			ite++;
		}
	}
}

// ***************************************************************************

void serverSentRemove (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Get the plugin
	CPlugin *plugin = (CPlugin *)(from->appId());

	// Read the message version
	sint version = msgin.serialVersion(0);

	// Called when the server sent a POS message
	uint32 count;
	msgin.serial(count);

	for (uint i = 0;i < count; i++)
	{	
		uint32 id;
		msgin.serial(id);

		plugin->removeEntity (plugin->_Entites[id]);
	}
}

// ***************************************************************************
void serverSentInfo (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Called when the server sent a INFO message
	string text;
	msgin.serial(text);
	printf("%s\n", text.c_str());
}

// ***************************************************************************
// All messages handled by this server
#define NB_CB 9
TCallbackItem CallbackArray[NB_CB] =
{
	{ "ADD", serverSentAdd },
	{ "RMV", serverSentRemove },
	{ "POS", serverSentPos },
	{ "STR", serverSentString },
	{ "INFO", serverSentInfo },
	{ "MISC_PROP", serverSentMiscProp }, // Misc. properties
	{ "SERVER_PARAMS", serverSentParams },
	{ "AUTHENT_VALID", serverSentAuthentValid },
	{ "AUTHENT_INVALID", serverSentAuthentInvalid },
};

// ***************************************************************************

void CPlugin::init(IPluginAccess *pluginAccess)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	AfxEnableControlContainer();
	try
	{				
		_PluginAccess = pluginAccess;

		// Add the search paths
		NLMISC::CConfigFile::CVar *path = _PluginAccess->getConfigFile().getVarPtr("ShardMonitorPath");
		if (path)
		{
			for(uint k = 0; k < (uint) path->size(); ++k)
			{
				NLMISC::CPath::addSearchPath(path->asString(k), false, false);
			}
		}

		// connect to the server here...
		
		// open the dialog flag window
		_DialogFlag->init(this);
		_DialogFlag->Create(IDD_DIALOG_FLAGS, CWnd::FromHandle(_PluginAccess->getMainWindow()->m_hWnd));		
		_DialogFlag->ShowWindow(TRUE);
		
		// Read the host where to connect in the client.cfg file		
		_PluginActive=true;
		updateConnectionState();		
		
		// init the sheets
		std::string sheetIDPath = CPath::lookup("sheet_id.bin", false, false);
		if (sheetIDPath.empty())
		{
			_PluginAccess->errorMessage(NULL, "Couldn't find sheet_id.bin, Please check shard monitor paths in world_editor_plugin.cfg.", "Warning", MB_OK|MB_ICONEXCLAMATION);		
		}	
		else
		{			
			NLMISC::CSheetId::init(false);
		}
	
	}
	catch (Exception &e)
	{
		errorMessage (e.what ());
	}
}

// ***************************************************************************

void CPlugin::connectDisconnect()
{	
	try
	{
		_DialogFlag->UpdateData();
		_DialogFlag->Download = 0;
		_DialogFlag->UpdateData(FALSE);
		if (!_Client || !_Client->connected())
		{
			// Create the socket
			if (_Client)
				delete _Client;
			_Client = new CCallbackClient;

			// Init and Connect the client to the server located on port 3333
			_Client->addCallbackArray (CallbackArray, NB_CB);
			_Client->getSockId ()->setAppId ((uint64)this);

			// connect to the server here...
		
			// Erase all the primitives
			uint i;
			for (i=0; i<_Entites.size(); i++)
			{
				removeEntity (_Entites[i]);
			}

			// Erase string cache
			_PrimitiveToString.clear ();
			_StringToPrimitive.clear ();
			_StringIdToString.clear ();

			// Read the host where to connect in the client.cfg file
			nlassert (_DialogFlag);
			_DialogFlag->UpdateData (TRUE);

			_SHost = (const char*)_DialogFlag->ShardCtrl.getCurrString();

			_DialogFlag->ShardCtrl.onOK();
			
			try
			{
				CInetAddress addr(_SHost+":48888");
				_Client->connect(addr);
			}
			catch(ESocket &e)
			{
				errorMessage (e.what ());
				return;
			}
			
			if (!_Client->connected())
			{
				errorMessage ("The connection failed");
				return;
			}

			// todo move the looking window
			{
				/*
				CMessage msg;
				msg.setType("WINDOW");
				float xmin = 0;
				float xmax = 10000;
				float ymin = -10000;
				float ymax = 0;
				msg.serial (xmin);
				msg.serial (ymin);
				msg.serial (xmax);
				msg.serial (ymax);
				_Client->send(msg);
				*/
				_WMin = CVector(+0.0f,     -10000.0f, 0.0f);
				_WMax = CVector(+10000.0f, +0.0f,     0.0f);
			}

			_ConnectState = WaitingServerParams;
		}
		else
		{			
			//
			_Client->disconnect();
			if (_Client->connected())
			{
				errorMessage ("The disconnection failed");
				delete _Client;
				_Client = NULL;
				return;
			}
			delete _Client;
			_Client = NULL;
		}
		updateConnectionState();
	}
	catch (Exception &e)
	{
		errorMessage (e.what ());
		delete _Client;
		_Client = NULL;
	}
}

// ***************************************************************************

/// The current region has changed.
void CPlugin::primitiveChanged(const NLLIGO::IPrimitive *root)
{
}

// ***************************************************************************

/// The listener has been moved on the map.
void CPlugin::positionMoved(const NLMISC::CVector &position)
{
}

// ***************************************************************************

void CPlugin::lostPositionControl()
{
}

// ***************************************************************************

void CPlugin::onIdle()
{	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	
	static bool in = false;
	if (!in)
	{
		in = true;		

		// during the first call, we create a Root primitive to store all players
		if (!m_Initialized)
		{
			m_Initialized = true;
			_Root = _PluginAccess->createRootPluginPrimitive("Shard Monitor");

			// Set the new name
			_Root->removePropertyByName ("name");
			_Root->addPropertyByName ("name", new CPropertyString ("Shard Monitor"));

			// Invalidate the name
			_PluginAccess->invalidatePluginPrimitive (_Root, LogicTreeParam);
		}
		else
		{
			if (_Client && _Client->connected())
			{
				_ConnectionMsg.MsgType = CConnectionMsg::NoMsg;
				// first, we receive the stack of messages, which is composed of players information
				_Client->update();
				// See if there's aconnection relarted msg
				switch(_ConnectionMsg.MsgType)
				{
					case CConnectionMsg::ServerParamsMsg:
						serverParamsReceived(_ConnectionMsg.ServerParams);
					break;
					case CConnectionMsg::AuthentValid:
						authentValid();
					break;
					case CConnectionMsg::AuthentInvalid:
						authentInvalid();
					break;
					default:
						// no-op
					break;
				}

				
				if (_ConnectState == Authentificated)
				{
					// here check window coordinates
					CVector	vmin, vmax;
					_PluginAccess->getWindowCoordinates(vmin, vmax);
					if (vmin != _WMin || vmax != _WMax)
					{
						CMessage msg;
						msg.setType("WINDOW");
						float xmin = vmin.x;
						float xmax = vmax.x;
						float ymin = vmin.y;
						float ymax = vmax.y;
						msg.serial (xmin);
						msg.serial (ymin);
						msg.serial (xmax);
						msg.serial (ymax);
						_Client->send(msg);
						_WMin = vmin;
						_WMax = vmax;
					}

					if (_CtrlRequestedDownload != _RequestedDownload)
					{
						_RequestedDownload = _CtrlRequestedDownload;
						CMessage msg;
						msg.setType("BANDW");
						uint32	bandw = _RequestedDownload;
						msg.serial(bandw);
						_Client->send(msg);
					}
				}
			}
		}

		// Update strings
		updateConnectionState();

		in = false;
		
	}
}

const uint HIT_TIME_IN_MS = 1000;

// ***************************************************************************
void CPlugin::displayHits(CDisplay &display)
{
	std::list<CHit>::iterator it = _HitList.begin();
	while (it != _HitList.end())
	{
		std::list<CHit>::iterator tmpIt = it;
		++ it;
		if (_CurrentTime - tmpIt->HitTime  >= HIT_TIME_IN_MS)
		{			
			_HitList.erase(tmpIt);
		}
		else
		{		

			CEntityEntry &entity = _Entites[tmpIt->EntityId];
			if (!entity.Hidden && _DisplayHits)
			{
				const CPrimPoint *point = dynamic_cast<const CPrimPoint *>(entity.Primitive);
				if (point)
				{			
					// Clip ?
					if (!display.isClipped (&point->Point, 1))
					{
						const float HIT_SIZE = 10.f;
						uint dt = (uint) (_CurrentTime - tmpIt->HitTime);
						float currHitSize = HIT_SIZE * (float) dt / HIT_TIME_IN_MS;

						// Position in world
						CVector center = point->Point;
						display.worldToPixel (center);

						// Dot
						CVector corners[] = 
						{
							CVector(0.5f, 1.f, 0.f),
							CVector(1.0f, 0.5f, 0.f),
							CVector(1.0f, -0.5f, 0.f),
							CVector(0.5f, -1.f, 0.f),
							CVector(-0.5f, -1.f, 0.f),
							CVector(-1.f, -0.5f, 0.f),
							CVector(-1.f, 0.5f, 0.f),
							CVector(-0.5f, 1.f, 0.f),
						};
						const uint numCorners = sizeofarray(corners);
						for(uint k = 0; k < numCorners; ++k)
						{
							corners[k] = currHitSize * corners[k] + center;
							display.pixelToWorld(corners[k]);
						}
						const CRGBA HIT_COLOR(255, 0, 0);
						for(uint k = 0; k < numCorners; ++k)
						{
							display.lineRenderProxy(HIT_COLOR, corners[k], corners[(k + 1) % numCorners], 0);
						}
					}
				}
			}
		}
	}
}


// ***************************************************************************
void CPlugin::pushIcon(CDisplay &display, sint stepX, sint stepY, NLMISC::CVector &currPos, const CEntityIcon &icon, const CPrimTexture &pt)
{
	// if icon is NULL then no-op
	if (icon.X < 0 || icon.Y < 0) return;
	CVector center = currPos;
	display.worldToPixel(center);
	center.x += (float) stepX;
	center.y += (float) stepY;
	currPos = center;
	display.pixelToWorld(currPos);
	CVector tl(center.x - ENTITY_ICON_SIZE / 2, center.y + ENTITY_ICON_SIZE / 2, 0.f);
	CVector br(center.x + ENTITY_ICON_SIZE / 2, center.y - ENTITY_ICON_SIZE / 2, 0.f);
	display.pixelToWorld(tl);
	display.pixelToWorld(br);
	NLMISC::CQuadColorUV quvc;
	quvc.V0.set(tl.x, tl.y, 0.f);
	quvc.V1.set(br.x, tl.y, 0.f);
	quvc.V2.set(br.x, br.y, 0.f);
	quvc.V3.set(tl.x, br.y, 0.f);
	quvc.Color0 = quvc.Color1 = quvc.Color2 = quvc.Color3 = CRGBA::White;
	if (pt.getWidth() == 0	|| pt.getHeight() == 0)
	{
		// if texture width is 0, then texture hasn't been found, so display the whole 'not found' texture
		quvc.Uv0.set(0.f, 0.f);
		quvc.Uv1.set(1.f, 0.f);
		quvc.Uv2.set(1.f, 1.f);
		quvc.Uv3.set(0.f, 1.f);
	}
	else
	{
		float invWidth  = 1.f / pt.getWidth();
		float invHeight = 1.f / pt.getHeight();
		sint srcX = icon.X * ENTITY_ICON_SIZE;
		sint srcY = icon.Y * ENTITY_ICON_SIZE;
		quvc.Uv0.set(srcX * invWidth, srcY * invHeight);
		quvc.Uv1.set((srcX + ENTITY_ICON_SIZE) * invWidth, srcY * invHeight);
		quvc.Uv2.set((srcX + ENTITY_ICON_SIZE) * invWidth, (srcY  + ENTITY_ICON_SIZE) * invHeight);
		quvc.Uv3.set(srcX * invWidth, (srcY  + ENTITY_ICON_SIZE) * invHeight);
	}
	display.texQuadRenderProxy(quvc, 0);
}

// ***************************************************************************
void CPlugin::displayCloseUp(CDisplay &display)
{
	// init icon texture if not already done
	if (!_EntityIcons)
	{	
		static bool createFailed = false;
		if (createFailed) return;
		HRSRC rsc = FindResource(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ENTITY_ICONS_TGA), "TGA");
		if (rsc == NULL)
		{
			createFailed = true;
			return;
		}
		NLMISC::CBitmap bm;
		if (!_PluginAccess->buildNLBitmapFromTGARsc(rsc, AfxGetInstanceHandle(), bm))
		{
			createFailed = true;
			return;
		}
		_EntityIcons = _PluginAccess->createTexture();
		nlassert(_EntityIcons);
		_EntityIcons->buildFromNLBitmap(bm);
	}
	display.flush(); // must flush before we assign a next texture
	display.setLayerTexture(0, _EntityIcons);
	for(uint k = 0; k < _Entites.size(); ++k)
	{		
		if (!_Entites[k].Hidden)
		{
			CEntityEntry &entity = _Entites[k];
			const CPrimPoint *point = dynamic_cast<const CPrimPoint *>(entity.Primitive);
			if (point)
			{			
				// Clip ?
				if (!display.isClipped (&point->Point, 1))
				{					
					CVector currPos = point->Point;
					// display type of entity
					if (_CloseUpFlags[CloseUpEntityType]) // want to display the type as an icon ?
					{
						// see if there's an icon associated with current type
						pushIcon(display, ENTITY_ICON_SIZE / 2 + 4, 0, currPos, entity.IconForEntityType, *_EntityIcons);
					}
					if (_CloseUpFlags[CloseUpEntityMode]) // want to display the type as an icon ?
					{
						// see if there's an icon associated with current type
						pushIcon(display, ENTITY_ICON_SIZE + 4, 0, currPos, entity.IconForEntityMode, *_EntityIcons);
					}
					if (_CloseUpFlags[CloseUpEntityHP]) // want to display the type as an icon ?
					{
						if (entity.MaxHitPoints != 0)
						{
							uint iconIndex = 0;
							if (entity.HitPoints != 0)
							{
								iconIndex = 1 + (3 * entity.HitPoints) / entity.MaxHitPoints;
							}
							// icons position is hardcoded for now...
							pushIcon(display, ENTITY_ICON_SIZE + 4, 0, currPos, CEntityIcon(iconIndex, 2), *_EntityIcons);
						}
					}					
				}
			}
		}
	}
}

// ***************************************************************************
void CPlugin::postRender(CDisplay &display)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	_CurrentTime = NLMISC::CTime::getLocalTime();
	const uint HIT_TIME_IN_MS = 1000;
	// Display entities that are being hit
	displayHits(display);
	// Display close up details
	CVector oneMeter(1.f, 0.f, 0.f);
	display.worldVectorToFloatPixelVector(oneMeter);
	if (oneMeter.x >= _CloseUpDisplayDistance)
	{
		displayCloseUp(display);
	}
}

// ***************************************************************************

bool CPlugin::yesNoMessage (const char *format, ... )
{
	char buffer[1024];

	if (format)
	{
		va_list args;
		va_start( args, format );
		sint ret = vsnprintf( buffer, 1024, format, args );
		va_end( args );
	}
	else
	{
		strcpy(buffer, "Unknown error");
	}

	return _PluginAccess->yesNoMessage ("Plugin AI : %s", buffer);
}

// ***************************************************************************

void CPlugin::errorMessage (const char *format, ... )
{
	char buffer[1024];

	if (format)
	{
		va_list args;
		va_start( args, format );
		sint ret = vsnprintf( buffer, 1024, format, args );
		va_end( args );
	}
	else
	{
		strcpy(buffer, "Unknown error");
	}

	_PluginAccess->errorMessage ("Plugin AI : %s", buffer);
}

// ***************************************************************************

void CPlugin::infoMessage (const char *format, ... )
{
	char buffer[1024];

	if (format)
	{
		va_list args;
		va_start( args, format );
		sint ret = vsnprintf( buffer, 1024, format, args );
		va_end( args );
	}
	else
	{
		strcpy(buffer, "Unknown error");
	}

	_PluginAccess->infoMessage ("Plugin AI : %s", buffer);
}

// ***************************************************************************

void CPlugin::updateConnectionState()
{			
	_DialogFlag->UpdateData ();
	_DialogFlag->Sent = "";
	_DialogFlag->Received = "";
	_DialogFlag->DownloadValue = (toString("%.1f kB/s", _DialogFlag->Download/1024.0)).c_str();
	if (!_Client || !_Client->connected())
	{
		_ConnectState = Disconnected;
		// Change the text
		_DialogFlag->ConnectCtrl.SetWindowText (getStringRsc(IDS_CONNECT));
		_DialogFlag->State = getStringRsc(IDS_NOT_CONNECTED);		
	}
	else
	{
		switch(_ConnectState)
		{
			case Authentificated:
			{
				static	deque< pair<TTime, uint64> >	dataSize;
				dataSize.push_back( make_pair(NLMISC::CTime::getLocalTime(), _Client->getBytesReceived()) );
				if (dataSize.size() > 20)
					dataSize.pop_front();

				double	bandwidth = 0.0;
				if (dataSize.size() > 1)
				{
					uint	dSize = (uint)(dataSize.back().second - dataSize.front().second);
					uint	dt = (uint)(dataSize.back().first - dataSize.front().first);
					bandwidth = (double)dSize*1000.0 / dt;
				}

				// Change the text
				_DialogFlag->ConnectCtrl.SetWindowText (getStringRsc(IDS_DISCONNECT));
				_DialogFlag->State = (LPCTSTR) (getStringRsc(IDS_CONNECTED_TO) + _SHost.c_str());
				_DialogFlag->Sent = (toString ((uint32)_Client->getBytesSent ()) + (LPCTSTR) getStringRsc(IDS_BYTE_SENT)).c_str();
				//_DialogFlag->Received = (toString ((uint32)_Client->getBytesReceived ()) + " bytes received").c_str();
				_DialogFlag->Received = (toString("%.1f", bandwidth/1024.0) + " kB/s").c_str();
				_DialogFlag->DownloadValue = (toString("%.1f kB/s", _DialogFlag->Download/1024.0)).c_str();
				_CtrlRequestedDownload = _DialogFlag->Download;
			}
			break;
			case WaitingServerParams:				
				_DialogFlag->ConnectCtrl.SetWindowText (getStringRsc(IDS_CANCEL_CONNECT));
				_DialogFlag->State = (LPCTSTR) (getStringRsc(IDS_WAITING_SERVER_PARAMS) + _SHost.c_str());
			break;
			case WaitingLoginConfirmation:
			{
				_DialogFlag->ConnectCtrl.SetWindowText (getStringRsc(IDS_CANCEL_CONNECT));				
				std::string label = (LPCTSTR) getStringRsc(IDS_WAITING_LOGIN_CONFIRMATION) +_SHost;
				switch((NLMISC::CTime::getLocalTime() / 200) % 4)
				{
					case 0: label+= "-";  break;
					case 1: label+= "/";  break;
					case 2: label+= "|";  break;
					case 3: label+= "\\"; break;				
				}
				_DialogFlag->State = label.c_str();
			}
			break;
			default:
				nlassert(0);
			break;
		}
	}
	_DialogFlag->UpdateData (FALSE);	
}

// ***************************************************************************

void CPlugin::setPrimitiveName (NLLIGO::IPrimitive *primitive, uint32 stringId)
{
	std::map<uint32, std::string>::iterator ite = _StringIdToString.find (stringId);
	if (ite != _StringIdToString.end ())
	{
		// Set the new name
		primitive->removePropertyByName ("name");
		primitive->addPropertyByName ("name", new CPropertyString (ite->second.c_str()));
	}
	else
	{
		// Set the new name
		primitive->removePropertyByName ("name");
		primitive->addPropertyByName ("name", new CPropertyString ("<unknown>"));
	}

	// Invalidate the name
	_PluginAccess->invalidatePluginPrimitive (primitive, LogicTreeParam);
}

// ***************************************************************************
bool CPlugin::isActive()
{
	return _PluginActive;
}

// ***************************************************************************
string& CPlugin::getName()
{
	static string ret="Shard Monitor";
	return ret;
}

// ***************************************************************************
bool CPlugin::activatePlugin()
{
	if(!_PluginActive)
	{
		_DialogFlag->ShowWindow(TRUE);	
		_PluginActive=true;
		return true;
	}

	return false;
}

// ***************************************************************************
bool CPlugin::closePlugin()
{
	if(_PluginActive)
	{
		_DialogFlag->ShowWindow(FALSE);

		_PluginActive=false;
		
		return true;
	}	
	return false;
}

// ***************************************************************************
void CPlugin::setCloseUpFlag(TCloseUpFlag flag, bool on)
{
	if (on == _CloseUpFlags[flag]) return;
	nlassert(flag < CloseUpFlagCount);
	_CloseUpFlags[flag] = on;
	updateDisplay();
}

CString getStringRsc(UINT strID)
{
	CString s;
	s.LoadString(strID);
	return s;
}
