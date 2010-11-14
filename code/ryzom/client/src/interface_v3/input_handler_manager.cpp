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



#include "stdpch.h"

#include "nel/misc/i_xml.h"
#include "nel/misc/file.h"
#include "nel/misc/game_device_events.h"

#include "game_share/xml_auto_ptr.h"

#include "input_handler_manager.h"
#include "interface_manager.h"
#include "../actions.h"
#include "../input.h"
#include "../client_cfg.h"
#include "../motion/user_controls.h"
#include "../init.h"
#include "../release.h"

#include "../r2/editor.h"


///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;

////////////
// GLOBAL //
////////////

extern CActionsManager	Actions;
extern CActionsManager	EditActions;

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Pump_Events )

CInputHandlerManager* CInputHandlerManager::_Instance = NULL;


// ***************************************************************************
CInputHandlerManager::CInputHandlerManager()
{
	_EventServer= NULL;
	_MouseButtonsReleased = noButton;
	_MouseButtonsDown = noButton;
	_MouseButtonsState = noButton;
	_MouseX = _MouseY = _MouseLastX = _MouseLastY = 0;
	_Focus = true;
	_MouseWheel = 0;
	_SkipInterfaceManager=false;
	_RecoverFocusLost = false;
}

// ***************************************************************************
CInputHandlerManager::~CInputHandlerManager()
{
}

// ********************************************************************************************
void CInputHandlerManager::releaseInstance()
{
	if( _Instance )
	{
		delete _Instance;
		_Instance = NULL;
	}
}

// ***************************************************************************
void CInputHandlerManager::addToServer(NLMISC::CEventServer * server)
{
	_EventServer = server;

	// System
	server->addListener(EventGDMouseMove,	this);
	server->addListener(EventDestroyWindowId,	this);
	server->addListener(EventSetFocusId,	this);
	server->addListener(EventDisplayChangeId,	this);

	// Mouse
	server->addListener(EventMouseMoveId,	this);
	server->addListener(EventMouseDownId,	this);
	server->addListener(EventMouseUpId,		this);
	server->addListener(EventMouseWheelId,	this);
	server->addListener(EventMouseDblClkId,	this);

	// Keyboard
	server->addListener(EventStringId,		this);
	server->addListener(EventCharId,		this);
	server->addListener(EventKeyDownId,		this);
	server->addListener(EventKeyUpId,		this);
}


// ***************************************************************************
void CInputHandlerManager::release()
{
	// System
	_EventServer->removeListener(EventGDMouseMove,	this);
	_EventServer->removeListener(EventDestroyWindowId,	this);
	_EventServer->removeListener(EventSetFocusId,	this);
	_EventServer->removeListener(EventDisplayChangeId,	this);

	// Mouse
	_EventServer->removeListener(EventMouseMoveId,	this);
	_EventServer->removeListener(EventMouseDownId,	this);
	_EventServer->removeListener(EventMouseUpId,	this);
	_EventServer->removeListener(EventMouseWheelId,	this);
	_EventServer->removeListener(EventMouseDblClkId,	this);

	// Keyboard
	_EventServer->removeListener(EventStringId,		this);
	_EventServer->removeListener(EventCharId,		this);
	_EventServer->removeListener(EventKeyDownId,	this);
	_EventServer->removeListener(EventKeyUpId,		this);

}


// ***************************************************************************
void CInputHandlerManager::operator ()(const NLMISC::CEvent &event)
{
	HandleSystemCursorCapture(event);

	if (event == EventDisplayChangeId)
	{
	}

	// Process message to InterfaceManager
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	if (event==EventSetFocusId)
	{
		CEventSetFocus *pEvent=(CEventSetFocus *)&event;
		if (!pEvent->Get)
		{
			// Deactivate all keys
			_MouseButtonsDown = noButton;
			_MouseButtonsReleased = noButton;
			_MouseButtonsState = noButton;
			_Focus = false;

			if (!_SkipInterfaceManager)
			{
				// if there was some control capturing the mouse, warn them that they lost the focus
				if (pIM->getCapturePointerLeft())
				{
					pIM->getCapturePointerLeft()->handleEvent(CEventDescriptorSetFocus(pEvent->Get));
				}
				pIM->setCapturePointerLeft(NULL);
				if (pIM->getCapturePointerRight())
				{
					pIM->getCapturePointerRight()->handleEvent(CEventDescriptorSetFocus(pEvent->Get));
				}
				pIM->setCapturePointerRight(NULL);
				UserControls.stopFreeLook();
			}
			// be nice with other app : let the mouse reappear (useful in direct 3D mode with no hardware cursor)
			Driver->showCursor(true);
//			Driver->setSystemArrow();
		}
		else
		{
			_RecoverFocusLost = true; // force to update mouse pos on next click or move
			Driver->showCursor(IsMouseCursorHardware());
			_Focus = true;
		}

		if(!_SkipInterfaceManager)
		{
			//
			if (R2::getEditor().isInitialized()
					&& (ClientCfg.R2EDEnabled || R2::getEditor().getCurrentTool())
				   )
			{
				R2::getEditor().handleEvent(CEventDescriptorSetFocus(pEvent->Get));
			}
		}

		// re-post
		FilteredEventServer.postEvent( event.clone() );
	}

	// If want to skip the interface Manager
	if(_SkipInterfaceManager)
	{
		// redirect to FilteredEventServer
		FilteredEventServer.postEvent( event.clone() );
		/*	Invalidate Actions managers by simulate keyUp on all. Else can have this scenario:
			- keyDown ("forward" action valide) before teleporting
			- keyUp while teleporting (=> _SkipInterfaceManager==true)
			- action still valide when teleported
		*/
		EditActions.releaseAllKeyNoRunning();
		Actions.releaseAllKeyNoRunning();
		return;
	}


	// **** Event Focus

	// **** Event Keyboard
	if( event == EventKeyDownId ||
			 event == EventKeyUpId ||
			 event == EventCharId ||
			 event == EventStringId)
	{
		// if not handled, post to Action Manager
		if( !pIM->handleEvent( CEventDescriptorKey((const CEventKey &) event) ) )
		{
			// See if handled by editor
			bool handled = false;
			if (R2::getEditor().isInitialized()
			    && (ClientCfg.R2EDEnabled || R2::getEditor().getCurrentTool())
			   )
			{
				handled = R2::getEditor().handleEvent(CEventDescriptorKey((const CEventKey &) event) );
			}
			if (!handled)
			{
				// Event from the Keyboard (DOWN KEYS)
				if(event == EventKeyDownId)
				{
					CEventKeyDown* downEvent=(CEventKeyDown*)&event;
					if (!pIM->getCaptureKeyboard () || !EditActions.keyPushed (*downEvent))
						Actions.keyPushed (*downEvent);
				}
				// Event from the Keyboard (UP KEYS)
				else if(event == EventKeyUpId)
				{
					CEventKeyUp* upEvent=(CEventKeyUp*)&event;
					EditActions.keyReleased (*upEvent);
					Actions.keyReleased (*upEvent);
				}
			}
		}
		/* if handled, still post released key for the following bug:
				- I press 'Z' to go 'forward' before entering chat
				- I enter chat, keeping Z pressed
				- I release 'Z' in chat (thus handled by pIM->handleEvent())
				- the player continue running...
		*/
		else
		{
			// don't send keydown to action managers
			if(event==EventKeyUpId)
			{
				CEventKeyUp* upEvent=(CEventKeyUp*)&event;
				EditActions.keyReleased (*upEvent);
				Actions.keyReleased (*upEvent);
			}
		}
	}
	// **** Event Mouse
	else if(pIM->getPointer() && _Focus /* && pIM->isMouseHandlingEnabled() */ &&
			( event == EventMouseMoveId ||
			  event == EventMouseDownId ||
			  event == EventMouseUpId ||
			  event == EventMouseWheelId ||
			  event == EventMouseDblClkId )
			)
	{


		CViewPointer &rIP = *pIM->getPointer();

		CEventDescriptorMouse eventDesc;
		sint32	x,y;
		rIP.getPointerDispPos (x, y);
		eventDesc.setX (x);
		eventDesc.setY (y);

		bool	handled= false;

		// button down ?
		static volatile bool doTest = false;

		if (!doTest || (doTest && pIM->isMouseHandlingEnabled()))
		{
			if (event==EventMouseDownId)
			{
				if (_RecoverFocusLost)
				{
					handled |= updateMousePos((CEventMouse&)event, eventDesc); // must update mouse pos here,
																			 // because when app window focus is gained by a mouse click, this is
																			 // the only place where we can retrieve mouse pos before a mouse move
					_RecoverFocusLost = false;
				}
				if (!handled)
				{
					if (R2::getEditor().isInitialized()
						&& (R2::isEditionCurrent() || R2::getEditor().getCurrentTool())
					   )
					{
						handled |= R2::getEditor().handleEvent(eventDesc);
					}
				}

				CEventMouseDown *pEvent=(CEventMouseDown*)&event;

				// update states
				_MouseButtonsDown = (TMouseButton)  (_MouseButtonsDown | pEvent->Button);
				_MouseButtonsReleased =(TMouseButton) (_MouseButtonsReleased & ~(pEvent->Button));
				_MouseButtonsState = (TMouseButton)  (_MouseButtonsState | pEvent->Button);

				rIP.setButtonState(_MouseButtonsState);

				// handle Event
				if(pEvent->Button & leftButton)
				{
					eventDesc.setEventTypeExtended(CEventDescriptorMouse::mouseleftdown);
					handled|= pIM->handleEvent (eventDesc);
				}
				if(pEvent->Button & rightButton)
				{
					eventDesc.setEventTypeExtended(CEventDescriptorMouse::mouserightdown);
					handled|= pIM->handleEvent (eventDesc);
				}
			}
			// button up ?
			else if (event==EventMouseUpId)
			{
				CEventMouseUp *pEvent=(CEventMouseUp*)&event;

				// update states
				_MouseButtonsReleased = (TMouseButton)  (_MouseButtonsReleased | pEvent->Button);
				_MouseButtonsDown =(TMouseButton) (_MouseButtonsDown & ~(pEvent->Button));
				_MouseButtonsState = (TMouseButton) (_MouseButtonsState & ~(pEvent->Button));

				rIP.setButtonState(_MouseButtonsState);

				// handle Event
				if(pEvent->Button & leftButton)
				{
					eventDesc.setEventTypeExtended(CEventDescriptorMouse::mouseleftup);
					handled|= pIM->handleEvent (eventDesc);
				}
				if(pEvent->Button & rightButton)
				{
					eventDesc.setEventTypeExtended(CEventDescriptorMouse::mouserightup);
					handled|= pIM->handleEvent (eventDesc);
				}
			}
			// db click ?
			else if (event == EventMouseDblClkId )
			{
				// TODO: yoyo make it work if needed (for now, seems preferable to manage in each ActionHandler)

				CEventMouseDblClk* pEvent=(CEventMouseDblClk*)&event;

				// handle Event
				if(pEvent->Button & leftButton)
				{
					eventDesc.setEventTypeExtended(CEventDescriptorMouse::mouseleftdblclk);
					handled|= pIM->handleEvent (eventDesc);
				}
				if(pEvent->Button & rightButton)
				{
					eventDesc.setEventTypeExtended(CEventDescriptorMouse::mouserightdblclk);
					handled|= pIM->handleEvent (eventDesc);
				}

			}
			// mouse move?
			else if(event == EventMouseMoveId)
			{
				handled |= updateMousePos((CEventMouse&)event, eventDesc);
			}
			else if (event == EventMouseWheelId)
			{
				CEventMouseWheel *pEvent=(CEventMouseWheel*)&event;
				if (pEvent->Direction)
					_MouseWheel += 1;
				else
					_MouseWheel -= 1;

				// handle Event now.
				if (_MouseWheel != 0)
				{
					eventDesc.setEventTypeExtended(CEventDescriptorMouse::mousewheel);
					eventDesc.setWheel(_MouseWheel);
					handled|= pIM->handleEvent (eventDesc);
					_MouseWheel = 0;
				}
			}
		}

		// if Event not handled, post to Action Manager
		if( !handled )
		{
			bool handled = false;
			if (R2::getEditor().isInitialized()
			    && (R2::isEditionCurrent() || R2::getEditor().getCurrentTool())
			   )
			{
				handled = R2::getEditor().handleEvent(eventDesc);
			}
			if (!handled)
			{
				// post to Action Manager
				FilteredEventServer.postEvent( event.clone() );
			}
		}
	}
	// **** Others
	else
	{
		FilteredEventServer.postEvent( event.clone() );
	}
}


// ***************************************************************************
bool		CInputHandlerManager::updateMousePos(NLMISC::CEventMouse &event, CEventDescriptorMouse &eventDesc)
{
	if (!IsMouseFreeLook())
	{
		CEventMouseMove* mouseEvent=(CEventMouseMove*)&event;
		uint32 w, h;
		CInterfaceManager::getInstance()->getViewRenderer().getScreenSize(w, h);

		// compute new coords
		_MouseLastX = _MouseX;
		_MouseLastY = _MouseY;
		_MouseX = (sint32)(mouseEvent->X*w + 0.5f);
		_MouseY = (sint32)(mouseEvent->Y*h + 0.5f);

		// Process Move message only if not Null move
		if(_MouseX!=_MouseLastX || _MouseY!=_MouseLastY)
		{
			// Move the pointer
			//pIM->movePointer (_MouseX-_MouseLastX, _MouseY-_MouseLastY);

			CInterfaceManager *pIM = CInterfaceManager::getInstance();

			pIM->movePointerAbs(_MouseX, _MouseY);

			CViewPointer &rIP = *pIM->getPointer();
			// get new pointer pos.
			sint32	x,y;
			rIP.getPointerDispPos (x, y);
			eventDesc.setX (x);
			eventDesc.setY (y);

			// handle Event now.
			eventDesc.setEventTypeExtended(CEventDescriptorMouse::mousemove);
			return pIM->handleEvent (eventDesc);
		}
	}
	return false;
}


// ***************************************************************************
void		CInputHandlerManager::CComboKey::init(const CEventDescriptorKey &rDK)
{
	Key= rDK.getKey();
	CtrlFlags= 0;
	if( rDK.getKeyCtrl() )
		CtrlFlags|= CtrlKey;
	if( rDK.getKeyShift() )
		CtrlFlags|= ShiftKey;
	if( rDK.getKeyAlt() )
		CtrlFlags|= AltKey;
}

// ***************************************************************************
bool		CInputHandlerManager::CComboKey::operator<(const CComboKey &c) const
{
	if(Key!=c.Key)
		return Key<c.Key;
	else
		return CtrlFlags<c.CtrlFlags;
}


// ***************************************************************************
bool CInputHandlerManager::isComboKeyChat(const CEventDescriptorKey &edk) const
{
	CComboKey	ckey;
	ckey.init(edk);
	return _ComboKeyChat.find(ckey)!=_ComboKeyChat.end();
}


// ***************************************************************************
bool CInputHandlerManager::readInputConfigFile(const std::string & fileName)
{
	// parse the XML input config file to build the events to actions map
	CIXml read;
	CIFile file;
	xmlNodePtr root;
	if (!file.open(fileName))
	{
		nlinfo(" could not open file %s",fileName.c_str());
	}
	try
	{
		read.init (file);
		root = read.getRootNode();

	}
	catch (Exception &e)
	{
		// Output error
		root = NULL;
		nlwarning ("CFormLoader: Error while loading the xml input file : %s", e.what());
	}
	if (!root)
	{
		nlinfo ("no root element in xml file %s",fileName.c_str());
		return false;
	}
	//the root node name should be input_config
	if (strcmp( (char*)root->name,"input_config") )
	{
		nlinfo("in a xml input config, the root node must be <input_config>");
		return false;
	}

	//get all system nodes
	xmlNodePtr cur = root->xmlChildrenNode;
	while (cur)
	{
		// Read all combo_key_chat setup.
		if ( !strcmp((char*)cur->name,"combo_key_chat") )
			parseComboKeyChat(cur);

		cur= cur->next;
	}

	return true;
}


// ***************************************************************************
void	CInputHandlerManager::pumpEvents()
{
	H_AUTO_USE ( RZ_Client_Pump_Events )

	nlassert(_EventServer);

	// pump the src EventServer
	_EventServer->pump(true);

	// pump the filtered ones
	FilteredEventServer.pump();
}


// ***************************************************************************
void	CInputHandlerManager::pumpEventsNoIM()
{
	nlassert(_EventServer);

	// Set Mode to skip InterfaceManager Handling
	_SkipInterfaceManager= true;

	// pump the src EventServer
	_EventServer->pump(true);

	// pump the filtered ones
	FilteredEventServer.pump(true);

	// reset
	_SkipInterfaceManager= false;
}


// ***************************************************************************
void	CInputHandlerManager::resetPos (sint x, sint y)
{
	_MouseLastX = x;
	_MouseX = x;
	_MouseY = y;
	_MouseLastY = y;

}


// ***************************************************************************
void CInputHandlerManager::parseComboKeyChat(xmlNodePtr cur)
{
	xmlNodePtr keySon = cur->xmlChildrenNode;
	while (keySon)
	{
		// check its a key
		if ( strcmp((char*)keySon->name,"key") )
		{
			keySon= keySon->next;
			continue;
		}

		vector<CComboKey> v;
		parseKey(keySon, v);

		for (uint i = 0; i < v.size(); ++i)
			_ComboKeyChat.insert(v[i]);

		// next
		keySon= keySon->next;
	}
}


// ***************************************************************************
void CInputHandlerManager::parseKey(xmlNodePtr cur, std::vector<CComboKey> &out)
{
	// read it
	CXMLAutoPtr prop;
	string	keyStr;
	string	flagStr;

	// read id
	prop= (char*) xmlGetProp( cur, (xmlChar*) "id" );
	if(prop)
		keyStr= (const char*)prop;

	// read flags
	prop= (char*) xmlGetProp( cur, (xmlChar*) "mod" );
	if(prop)
		flagStr= (const char*)prop;


	// Build the Key
	CComboKey	comboKey;
	// If an hexa code, translate
	if( keyStr.find("0x")!=string::npos )
	{
		sint	value;
		sscanf(keyStr.c_str(), "%x", &value);
		comboKey.Key= (TKey)value;
	}
	// else translate from enum
	else
	{
		comboKey.Key= CEventKey::getKeyFromString(keyStr);
	}

	// If valid key
	if(comboKey.Key==NLMISC::KeyCount)
	{
		nlwarning("Unknown Key Id in xml input file: %s", keyStr.c_str());
	}
	else
	{
		// Test All flags if match flagStr
		for(uint i=0;i<8;i++)
		{
			comboKey.CtrlFlags= i;

			// Test if Control match
			if(comboKey.CtrlFlags & CComboKey::CtrlKey)
			{
				// If don't find c or C, abort
				if( flagStr.find('c')== string::npos && flagStr.find('C')== string::npos )
					continue;
			}
			else
			{
				// if find 'c', then abort
				if( flagStr.find('c')!= string::npos )
					continue;
			}

			// Test if Shift match
			if(comboKey.CtrlFlags & CComboKey::ShiftKey)
			{
				// If don't find s or S, abort
				if( flagStr.find('s')== string::npos && flagStr.find('S')== string::npos )
					continue;
			}
			else
			{
				// if find 's', then abort
				if( flagStr.find('s')!= string::npos )
					continue;
			}

			// Test if Alt match
			if(comboKey.CtrlFlags & CComboKey::AltKey)
			{
				// If don't find a or A, abort
				if( flagStr.find('a')== string::npos && flagStr.find('A')== string::npos )
					continue;
			}
			else
			{
				// if find 'a', then abort
				if( flagStr.find('a')!= string::npos )
					continue;
			}

			// If pass, Insert this combo
			out.push_back(comboKey);
		}
	}
}

