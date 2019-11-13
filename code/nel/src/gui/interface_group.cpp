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
#include "libxml/globals.h"
#include "nel/misc/debug.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/misc/stream.h"
#include "nel/gui/lua_manager.h"
#include "nel/gui/lua_ihm.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/interface_link.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/ctrl_scroll_base.h"
#include "nel/gui/lua_ihm.h"

using namespace std;
using namespace NL3D;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

#define IG_UNIQUE_ID(this) ((void*)&((this)->_GroupSizeRef)) // NB nico : use some pointer *inside* CInterfaceGroup as a unique id for lua registry (any field but
														     // the first), instead of using 'this'. 'this' is already used by
		                                                     //  CLuaIHM::pushReflectableOnStack as unique id to CInterfaceElement's ref pointers

namespace NLGUI
{

	REGISTER_UI_CLASS(CInterfaceGroup)

	// ------------------------------------------------------------------------------------------------
	NLMISC_REGISTER_OBJECT(CViewBase, CInterfaceGroup, std::string, "interface_group");

	CInterfaceGroup::CInterfaceGroup(const TCtorParam &param) : CCtrlBase(param)
	{
		_ParentSizeMax = NULL;
		_MaxW = _MaxH = std::numeric_limits<sint32>::max();
		_OffsetX = _OffsetY = 0;
		_Overlappable= true;
		_ResizeFromChildW= false;
		_ResizeFromChildH= false;
		_ResizeFromChildWMargin= 0;
		_ResizeFromChildHMargin= 0;
		_MaxWReal = _MaxHReal = std::numeric_limits<sint32>::max();
		_GroupSizeRef = 0;
		_Escapable= false;
		_Priority= WIN_PRIORITY_NORMAL;
		_UseCursor = true;
		_IsGroupContainer = false;
		_IsGroupScrollText = false;
		_IsGroupInScene = false;
		_IsGroupList = false;
		_AHOnActive = NULL;
		_AHOnDeactive = NULL;
		_AHOnLeftClick = NULL;
		_AHOnRightClick = NULL;
		_AHOnEnter = NULL;
		_AHOnEscape = NULL;
		_NeedFrameUpdatePos= false;
		_LUAEnvTableCreated= false;
		_DepthForZSort= 0.f;

	#ifdef AJM_DEBUG_TRACK_INTERFACE_GROUPS
		CInterfaceManager::getInstance()->DebugTrackGroupsCreated( this );
	#endif
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::setIdRecurse(const std::string &id)
	{
		CCtrlBase::setIdRecurse(id);
		for(std::vector<CInterfaceGroup*>::iterator it = _ChildrenGroups.begin(); it != _ChildrenGroups.end(); ++it)
		{
			(*it)->setIdRecurse((*it)->getShortId());
		}
		for(std::vector<CCtrlBase*>::iterator it = _Controls.begin(); it != _Controls.end(); ++it)
		{
			(*it)->setIdRecurse((*it)->getShortId());
		}
		for(std::vector<CViewBase*>::iterator it = _Views.begin(); it != _Views.end(); ++it)
		{
			(*it)->setIdRecurse((*it)->getShortId());
		}
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::notifyActiveCalled(const NLGUI::CEventDescriptorActiveCalledOnParent &desc)
	{
		// notify children that the 'active' state of this group has changed
		for(std::vector<CInterfaceGroup*>::iterator it = _ChildrenGroups.begin(); it != _ChildrenGroups.end(); ++it)
		{
			(*it)->handleEvent(desc);
		}
		for(std::vector<CCtrlBase*>::iterator ctrlIt = _Controls.begin(); ctrlIt != _Controls.end(); ++ctrlIt)
		{
			(*ctrlIt)->handleEvent(desc);
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::setActive(bool state)
	{
		if(state != getActive())
		{
			CCtrlBase::setActive(state);
			if (_AHOnActive != NULL && state)
			{
				CAHManager::getInstance()->runActionHandler (_AHOnActive, this, _AHOnActiveParams);
			}
			if (_AHOnDeactive != NULL && !state)
			{
				CAHManager::getInstance()->runActionHandler (_AHOnDeactive, this, _AHOnDeactiveParams);
			}

			notifyActiveCalled(NLGUI::CEventDescriptorActiveCalledOnParent(state));
		}
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceGroup::~CInterfaceGroup()
	{
		// delete any LUA group environnement
		deleteLUAEnvTable();
		// delete any LUA interface link "ondbchange"
		removeAllLUAOnDbChange();

	//	NLMISC::TTime initStart;
	//	initStart = ryzomGetLocalTime ();
		clearGroups();
	//	nlinfo ("%d seconds for clearGroups '%s'", (uint32)(ryzomGetLocalTime ()-initStart)/1000, _Id.c_str());
	//	initStart = ryzomGetLocalTime ();
		clearControls();
	//	nlinfo ("%d seconds for clearControls '%s'", (uint32)(ryzomGetLocalTime ()-initStart)/1000, _Id.c_str());
	//	initStart = ryzomGetLocalTime ();
		clearViews();
	//	nlinfo ("%d seconds for clearViews '%s'", (uint32)(ryzomGetLocalTime ()-initStart)/1000, _Id.c_str());
		CWidgetManager::getInstance()->removeRefOnGroup (this);

	#ifdef AJM_DEBUG_TRACK_INTERFACE_GROUPS
		// AJM DEBUG
		CInterfaceManager::getInstance()->DebugTrackGroupsDestroyed( this );
	#endif
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::clearViews()
	{
		// Yoyo: important to not Leave NULL in the array, because of ~CGroupHTML and LibWWW callback
		// that may call CInterfaceManager::getElementFromId() (and this method hates having NULL in the arrays ^^)
		while(!_Views.empty())
		{
			CViewBase *pVB = _Views.back();
			delEltOrder (pVB);
			delete pVB;
			// slower than a _Views.clear() out of loop, but we have to keep a clean array.
			_Views.pop_back();
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::clearControls()
	{
		// Yoyo: important to not Leave NULL in the array, because of ~CGroupHTML() and LibWWW callback
		// that may call CInterfaceManager::getElementFromId() (and this method hates having NULL in the arrays ^^)
		while(!_Controls.empty())
		{
			CCtrlBase *pCB = _Controls.back();
			delEltOrder (pCB);
			delete pCB;
			// slower than a _Controls.clear() out of loop, but we have to keep a clean array.
			_Controls.pop_back();
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::clearGroups()
	{
		// Yoyo: important to not Leave NULL in the array, because of ~CGroupHTML() and LibWWW callback
		// that may call CInterfaceManager::getElementFromId() (and this method hates having NULL in the arrays ^^)
		while(!_ChildrenGroups.empty())
		{
			CInterfaceGroup *pIG = _ChildrenGroups.back();
			delEltOrder (pIG);
			delete pIG;
			// slower than a _ChildrenGroups.clear() out of loop, but we have to keep a clean array.
			_ChildrenGroups.pop_back();
		}
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::moveSBTrackY (CInterfaceGroup *target, sint32 dy)
	{
		// Look if there is a vertical scrollbar with this target attached ...
		vector<CCtrlBase*>::iterator itc;
		for (itc = _Controls.begin(); itc !=  _Controls.end(); itc++)
		{
			CCtrlBase *pCB = *itc;
			CCtrlScrollBase *pSB = dynamic_cast<CCtrlScrollBase*>(pCB);
			if (pSB != NULL)
			{
				if (pSB->getTarget() == target)
				{
					pSB->moveTrackY(dy);
					return true;
				}
			}
		}
		return false;
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::moveSBTargetY(CInterfaceGroup *target,sint32 dy)
	{
		// Look if there is a vertical scrollbar with this target attached ...
		vector<CCtrlBase*>::iterator itc;
		for (itc = _Controls.begin(); itc !=  _Controls.end(); itc++)
		{
			CCtrlBase *pCB = *itc;
			CCtrlScrollBase *pSB = dynamic_cast<CCtrlScrollBase*>(pCB);
			if (pSB != NULL)
			{
				if (pSB->getTarget() == target)
				{
					pSB->moveTargetY(dy);
					return true;
				}
			}
		}
		return false;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::setPriority(uint8 nprio)
	{
		if (nprio != _Priority)
		{
			CWidgetManager::getInstance()->setWindowPriority(this, nprio);
		}
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::parse (xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		if ( !CCtrlBase::parse(cur,parentGroup) )
		{
			nlinfo ("cannot parse InterfaceElementLocalisable part");
			return false;
		}

		//determine if the group is a window. If the property is not specified, set it to false.
		CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"win" ));

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"overlappable" );
		if(ptr)
		{
			_Overlappable= convertBool(ptr);
		}

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"escapable" );
		if(ptr)	_Escapable= convertBool(ptr);

		// determine if the group must be sized according to his sons.
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"child_resize_w" );
		if(ptr)
		{
			_ResizeFromChildW= convertBool(ptr);
		}
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"child_resize_h" );
		if(ptr)
		{
			_ResizeFromChildH= convertBool(ptr);
		}
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"child_resize_wmargin" );
		if(ptr)
		{
			NLMISC::fromString((const char*)ptr, _ResizeFromChildWMargin);
		}
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"child_resize_hmargin" );
		if(ptr)
		{
			NLMISC::fromString((const char*)ptr, _ResizeFromChildHMargin);
		}
		CAHManager::getInstance()->parseAH(cur, "on_active", "on_active_params", _AHOnActive, _AHOnActiveParams);
		CAHManager::getInstance()->parseAH(cur, "on_deactive", "on_deactive_params", _AHOnDeactive, _AHOnDeactiveParams);

		// Read user max size
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"max_w" );
		if (ptr) NLMISC::fromString((const char*)ptr, _MaxW);
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"max_h" );
		if (ptr) NLMISC::fromString((const char*)ptr, _MaxH);
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"max_sizeref" );
		if (ptr)
		{
			parseMaxSizeRef(ptr);
		}

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"max_sizeparent" );
		if (ptr)
		{
			string idparent = NLMISC::toLower(ptr.str());
			if (idparent != "parent")
			{
				if (parentGroup)
					idparent = parentGroup->getId() +":" +  string((const char*)ptr);
				else
					idparent = "ui:" +  string((const char*)ptr);
			}
			else
			{
				if (parentGroup)
					idparent = parentGroup->getId();
			}
			CWidgetManager::getInstance()->getParser()->addParentSizeMaxAssociation (this, idparent);
		}

		// left & right clicks
		CAHManager::getInstance()->parseAH(cur, "group_onclick_r", "group_params_r", _AHOnRightClick, _AHOnRightClickParams);
		CAHManager::getInstance()->parseAH(cur, "group_onclick_l", "group_params_l", _AHOnLeftClick, _AHOnLeftClickParams);

		// Each window (modal and groupContainer) can be validated by Enter. if "" => no op.
		CAHManager::getInstance()->parseAH(cur, "on_enter", "on_enter_params", _AHOnEnter, _AHOnEnterParams);

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"win_priority" );
		if(ptr) NLMISC::fromString((const char*)ptr, _Priority);

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"use_cursor" );
		if(ptr)	_UseCursor= convertBool(ptr);

		// Each window (modal and groupContainer) can be escaped if "escapable" set
		// they can add an action handler before the hide
		CAHManager::getInstance()->parseAH(cur, "on_escape", "on_escape_params", _AHOnEscape, _AHOnEscapeParams);

		if( editorMode )
		{
			ptr = (char*) xmlGetProp( cur, BAD_CAST "on_active" );
			if( ptr )
				mapAHString( "on_active", std::string( (const char*)ptr ) );

			ptr = (char*) xmlGetProp( cur, BAD_CAST "on_deactive" );
			if( ptr )
				mapAHString( "on_deactive", std::string( (const char*)ptr ) );

			ptr = (char*) xmlGetProp( cur, BAD_CAST "group_onclick_r" );
			if( ptr )
				mapAHString( "group_onclick_r", std::string( (const char*)ptr ) );

			ptr = (char*) xmlGetProp( cur, BAD_CAST "group_onclick_l" );
			if( ptr )
				mapAHString( "group_onclick_l", std::string( (const char*)ptr ) );

			ptr = (char*) xmlGetProp( cur, BAD_CAST "on_enter" );
			if( ptr )
				mapAHString( "on_enter", std::string( (const char*)ptr ) );

			ptr = (char*) xmlGetProp( cur, BAD_CAST "on_escape" );
			if( ptr )
				mapAHString( "on_escape", std::string( (const char*)ptr ) );
		}


		// LuaClass script
		ptr = xmlGetProp (cur, (xmlChar*)"lua_class");
		if( ptr )
			CWidgetManager::getInstance()->getParser()->addLuaClassAssociation( this, (const char*)ptr );

		return true;
	}

	std::string CInterfaceGroup::getProperty( const std::string &name ) const
	{
		if( name == "overlappable" )
		{
			return NLMISC::toString( _Overlappable );
		}
		else
		if( name == "escapable" )
		{
			return NLMISC::toString( _Escapable );
		}
		else
		if( name == "child_resize_w" )
		{
			return NLMISC::toString( _ResizeFromChildW );
		}
		else
		if( name == "child_resize_h" )
		{
			return NLMISC::toString( _ResizeFromChildH );
		}
		else
		if( name == "child_resize_wmargin" )
		{
			return NLMISC::toString( _ResizeFromChildWMargin );
		}
		else
		if( name == "child_resize_hmargin" )
		{
			return NLMISC::toString( _ResizeFromChildHMargin );
		}
		else
		if( name == "on_active" )
		{
			return getAHString( name );
		}
		else
		if( name == "on_active_params" )
		{
			return getOnActiveParams();
		}
		else
		if( name == "on_deactive" )
		{
			return getAHString( name );
		}
		else
		if( name == "on_deactive_params" )
		{
			return getOnDeactiveParams();
		}
		else
		if( name == "max_w" )
		{
			return NLMISC::toString( _MaxW );
		}
		else
		if( name == "max_h" )
		{
			return NLMISC::toString( _MaxH );
		}
		else
		if( name == "max_sizeref" )
		{
			return getSizeRefAsString( _GroupSizeRef, _SizeDivW, _SizeDivH );
		}
		else
		if( name == "max_sizeparent" )
		{
			return CWidgetManager::getInstance()->getParser()->getParentSizeMaxAssociation( (CInterfaceElement*)this );
		}
		else
		if( name == "group_onclick_r" )
		{
			return getAHString( name );
		}
		else
		if( name == "group_params_r" )
		{
			return getRightClickHandlerParams();
		}
		else
		if( name == "group_onclick_l" )
		{
			return getAHString( name );
		}
		else
		if( name == "group_params_l" )
		{
			return getLeftClickHandlerParams();
		}
		else
		if( name == "on_enter" )
		{
			return getAHString( name );
		}
		else
		if( name == "on_enter_params" )
		{
			return getAHOnEnterParams();
		}
		else
		if( name == "win_priority" )
		{
			return NLMISC::toString( _Priority );
		}
		else
		if( name == "use_cursor" )
		{
			return NLMISC::toString( _UseCursor );
		}
		else
		if( name == "on_escape" )
		{
			return getAHString( name );
		}
		else
		if( name == "on_escape_params" )
		{
			return getAHOnEscapeParams();
		}
		else
		if( name == "lua_class" )
		{
			return CWidgetManager::getInstance()->getParser()->getLuaClassAssociation( (CInterfaceGroup*)this );
		}
		else
			return CCtrlBase::getProperty( name );
	}

	void CInterfaceGroup::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "overlappable" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				_Overlappable = b;
			return;
		}
		else
		if( name == "escapable" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				_Escapable = b;
			return;
		}
		else
		if( name == "child_resize_w" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				_ResizeFromChildW = b;
			return;
		}
		else
		if( name == "child_resize_h" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				_ResizeFromChildH = b;
			return;
		}
		else
		if( name == "child_resize_wmargin" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				_ResizeFromChildWMargin = b;
			return;
		}
		else
		if( name == "child_resize_hmargin" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				_ResizeFromChildHMargin = b;
			return;
		}
		else
		if( name == "on_active" )
		{
			std::string dummy;
			_AHOnActive = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "on_active_params" )
		{
			_AHOnActiveParams = value;
			return;
		}
		else
		if( name == "on_deactive" )
		{
			std::string dummy;
			_AHOnDeactive = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "on_deactive_params" )
		{
			_AHOnDeactiveParams = value;
		}
		else
		if( name == "max_w" )
		{
			sint32 i;
			if( NLMISC::fromString( value, i ) )
				_MaxW = i;
			return;
		}
		else
		if( name == "max_h" )
		{
			sint32 i;
			if( NLMISC::fromString( value, i ) )
				_MaxH = i;
			return;
		}
		else
		if( name == "max_sizeref" )
		{
			parseMaxSizeRef( value.c_str() );
			return;
		}
		else
		if( name == "max_sizeparent" )
		{
			std::string parentId;

			if( value != "parent" ){
				if( _Parent != NULL )
					parentId = _Parent->getId() + ":" + value;
				else
					parentId = _Parent->getId();
			}
			CWidgetManager::getInstance()->getParser()->addParentSizeMaxAssociation( this, parentId );
			return;
		}
		else
		if( name == "group_onclick_r" )
		{
			std::string dummy;
			_AHOnRightClick = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "group_params_r" )
		{
			_AHOnRightClickParams = value;
			return;
		}
		else
		if( name == "group_onclick_l" )
		{
			std::string dummy;
			_AHOnLeftClick = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "group_params_l" )
		{
			_AHOnLeftClickParams = value;
			return;
		}
		else
		if( name == "on_enter" )
		{
			std::string dummy;
			_AHOnEnter = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "on_enter_params" )
		{
			_AHOnEnterParams = value;
			return;
		}
		else
		if( name == "win_priority" )
		{
			sint8 i;
			if( NLMISC::fromString( value, i ) )
				_Priority = i;
			return;
		}
		else
		if( name == "use_cursor" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				_UseCursor = b;
			return;
		}
		else
		if( name == "on_escape" )
		{
			std::string dummy;
			_AHOnEscape = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "on_escape_params" )
		{
			_AHOnEscapeParams = value;
			return;
		}
		else
		if( name == "lua_class" )
		{
			CWidgetManager::getInstance()->getParser()->addLuaClassAssociation( this, value );
			return;
		}
		else
			CCtrlBase::setProperty( name, value );
	}

	xmlNodePtr CInterfaceGroup::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = serializeGroup( parentNode, type );
		if( node == NULL )
			return NULL;

		serializeSubGroups( node );
		serializeControls( node );
		serializeViews( node );
		serializeLinks( node );

		return node;
	}

	xmlNodePtr CInterfaceGroup::serializeGroup( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CCtrlBase::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlNewProp( node, BAD_CAST "overlappable", BAD_CAST NLMISC::toString( _Overlappable ).c_str() );
		xmlNewProp( node, BAD_CAST "escapable", BAD_CAST NLMISC::toString( _Escapable ).c_str() );
		xmlNewProp( node, BAD_CAST "child_resize_w", BAD_CAST NLMISC::toString( _ResizeFromChildW ).c_str() );
		xmlNewProp( node, BAD_CAST "child_resize_h", BAD_CAST NLMISC::toString( _ResizeFromChildH ).c_str() );
		xmlNewProp( node, BAD_CAST "child_resize_wmargin", BAD_CAST NLMISC::toString( _ResizeFromChildWMargin ).c_str() );
		xmlNewProp( node, BAD_CAST "child_resize_hmargin", BAD_CAST NLMISC::toString( _ResizeFromChildHMargin ).c_str() );

		xmlNewProp( node, BAD_CAST "on_active", BAD_CAST getAHString( "on_active" ).c_str() );
		xmlNewProp( node, BAD_CAST "on_active_params", BAD_CAST getOnActiveParams().c_str() );
		xmlNewProp( node, BAD_CAST "on_deactive", BAD_CAST getAHString( "on_deactive" ).c_str() );
		xmlNewProp( node, BAD_CAST "on_deactive_params", BAD_CAST getOnDeactiveParams().c_str() );

		xmlNewProp( node, BAD_CAST "max_w", BAD_CAST NLMISC::toString( _MaxW ).c_str() );
		xmlNewProp( node, BAD_CAST "max_h", BAD_CAST NLMISC::toString( _MaxH ).c_str() );
		xmlNewProp( node, BAD_CAST "max_sizeref",
			BAD_CAST getSizeRefAsString( _GroupSizeRef, _SizeDivW, _SizeDivH ).c_str() );
		xmlNewProp( node, BAD_CAST "max_sizeparent",
			BAD_CAST CWidgetManager::getInstance()->getParser()->getParentSizeMaxAssociation( (CInterfaceElement*)this ).c_str() );
		xmlNewProp( node, BAD_CAST "group_onclick_r", BAD_CAST getAHString( "group_onclick_r" ).c_str() );
		xmlNewProp( node, BAD_CAST "group_params_r", BAD_CAST getRightClickHandlerParams().c_str() );
		xmlNewProp( node, BAD_CAST "group_onclick_l", BAD_CAST getAHString( "group_onclick_l" ).c_str() );
		xmlNewProp( node, BAD_CAST "group_params_l", BAD_CAST getLeftClickHandlerParams().c_str() );
		xmlNewProp( node, BAD_CAST "on_enter", BAD_CAST getAHString( "on_enter" ).c_str() );
		xmlNewProp( node, BAD_CAST "on_enter_params", BAD_CAST getAHOnEnterParams().c_str() );
		xmlNewProp( node, BAD_CAST "win_priority", BAD_CAST NLMISC::toString( _Priority ).c_str() );
		xmlNewProp( node, BAD_CAST "use_cursor", BAD_CAST NLMISC::toString( _UseCursor ).c_str() );
		xmlNewProp( node, BAD_CAST "on_escape", BAD_CAST getAHString( "on_escape" ).c_str() );
		xmlNewProp( node, BAD_CAST "on_escape_params", BAD_CAST getAHOnEscapeParams().c_str() );
		xmlNewProp( node, BAD_CAST "lua_class",
			BAD_CAST CWidgetManager::getInstance()->getParser()->getLuaClassAssociation( (CInterfaceGroup*)this ).c_str() );

		return node;
	}

	xmlNodePtr CInterfaceGroup::serializeSubGroups( xmlNodePtr parentNode ) const
	{
		std::vector< CInterfaceGroup* >::const_iterator itr;
		for( itr = _ChildrenGroups.begin(); itr != _ChildrenGroups.end(); ++itr )
		{
			if( !(*itr)->IsSerializable() )
				continue;

			(*itr)->serialize( parentNode, "group" );
		}

		return parentNode;
	}

	xmlNodePtr CInterfaceGroup::serializeControls( xmlNodePtr parentNode ) const
	{
		std::vector< CCtrlBase* >::const_iterator itr;
		for( itr = _Controls.begin(); itr != _Controls.end(); ++itr )
		{
			if( !(*itr)->IsSerializable() )
				continue;

			(*itr)->serialize( parentNode, "ctrl" );
		}

		return parentNode;
	}

	xmlNodePtr CInterfaceGroup::serializeViews( xmlNodePtr parentNode ) const
	{
		std::vector< CViewBase* >::const_iterator itr;
		for( itr = _Views.begin(); itr != _Views.end(); ++itr )
		{
			if( !(*itr)->IsSerializable() )
				continue;

			(*itr)->serialize( parentNode, "view" );
		}

		return parentNode;
	}

	xmlNodePtr CInterfaceGroup::serializeTreeData( xmlNodePtr parentNode ) const
	{
		if( parentNode == NULL )
			return NULL;

		xmlNodePtr node = xmlNewNode( NULL, BAD_CAST "tree" );
		if( node == NULL )
			return NULL;

		xmlAddChild( parentNode, node );

		xmlSetProp( node, BAD_CAST "node", BAD_CAST CInterfaceElement::stripId( getId() ).c_str() );

		return node;
	}


	bool CInterfaceGroup::serializeLinks( xmlNodePtr parentNode ) const
	{
		if( parentNode == NULL )
			return false;

		const std::map< uint32, SLinkData > &linkMap =
			CWidgetManager::getInstance()->getParser()->getLinkMap();

		xmlNodePtr node = NULL;

		std::map< uint32, SLinkData >::const_iterator itr;
		for( itr = linkMap.begin(); itr != linkMap.end(); ++itr )
		{
			if( itr->second.parent != getId() )
				continue;

			const SLinkData &data = itr->second;

			node = xmlNewNode( NULL, BAD_CAST "link" );
			if( node == NULL )
				return false;

			xmlAddChild( parentNode, node );

			xmlSetProp( node, BAD_CAST "expr", BAD_CAST data.expr.c_str() );

			if( !data.target.empty() )
				xmlSetProp( node, BAD_CAST "target", BAD_CAST data.target.c_str() );

			if( !data.action.empty() )
			{
				xmlSetProp( node, BAD_CAST "action", BAD_CAST data.action.c_str() );

				if( !data.params.empty() )
					xmlSetProp( node, BAD_CAST "params", BAD_CAST data.params.c_str() );

				if( !data.cond.empty() )
					xmlSetProp( node, BAD_CAST "cond", BAD_CAST data.cond.c_str() );
			}

		}


		return true;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::parseMaxSizeRef(const char *ptr)
	{
		parseSizeRef(ptr, _GroupSizeRef, _SizeDivW, _SizeDivH);
	}

	// ------------------------------------------------------------------------------------------------
	uint32 CInterfaceGroup::getMemory ()
	{
		uint32 Mem = sizeof(*this);
		/*vector<CInterfaceGroup*>::const_iterator itg;
		for (itg = _ChildrenGroups.begin() ; itg != _ChildrenGroups.end(); itg++)
		{
			CInterfaceGroup *pIG = *itg;
			Mem += pIG->getMemory();
		}*/

		for (vector<CViewBase*>::const_iterator itv = _Views.begin() ; itv != _Views.end(); itv++)
		{
			CViewBase *pVB = *itv;
			Mem += pVB->getMemory();
		}

		for (vector<CCtrlBase*>::const_iterator itc = _Controls.begin() ; itc != _Controls.end(); itc++)
		{
			CCtrlBase* ctrl = *itc;
			Mem += ctrl->getMemory();
		}
		return Mem;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::addToEltOrder(CViewBase *view, sint order)
	{
		if (!view) return;
		if (order == -1)
		{
			_EltOrder.push_back(view);
		}
		else
		{
			if (order > (sint) _EltOrder.size()) return;
			_EltOrder.insert(_EltOrder.begin() + order, view);
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::addElement (CInterfaceElement *child, sint eltOrder /*= -1*/)
	{
		if (!child)
		{
			nlwarning("<CInterfaceGroup::addView> : tried to add a NULL view");
			return;
		}

		if( child->isGroup() )
		{
			addGroup( static_cast< CInterfaceGroup* >( child ), eltOrder );
		}
		else
		if( child->isCtrl() )
		{
			addCtrl( static_cast< CCtrlBase* >( child ), eltOrder );
		}
		else
		if( child->isView() )
		{
			addView( static_cast< CViewBase* >( child ), eltOrder );
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::addView (CViewBase *child, sint eltOrder /*= -1*/)
	{
		if (!child)
		{
			nlwarning("<CInterfaceGroup::addView> : tried to add a NULL view");
			return;
		}
		_Views.push_back(child);
		addToEltOrder(child, eltOrder);

		// elt callBack.
		child->onAddToGroup();
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::addCtrl (CCtrlBase *child, sint eltOrder /*= -1*/)
	{
		if (!child)
		{
			nlwarning("<CInterfaceGroup::addCtrl> : tried to add a NULL ctrl");
			return;
		}
		_Controls.push_back(child);
		addToEltOrder(child, eltOrder);

		// elt callBack.
		child->onAddToGroup();
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::addGroup (CInterfaceGroup *child, sint eltOrder /*= -1*/)
	{
		if (!child)
		{
			nlwarning("<CInterfaceGroup::addGroup> : tried to add a NULL group");
			return;
		}
		_ChildrenGroups.push_back(child);
		addToEltOrder(child, eltOrder);

		// elt callBack.
		child->onAddToGroup();
	}

	// ------------------------------------------------------------------------------------------------
	int CInterfaceGroup::luaAddGroup (CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "CInterfaceGroup::addTab", 1);
		CInterfaceGroup * group = dynamic_cast<CInterfaceGroup *>(CLuaIHM::getUIOnStack(ls, 1));
		if(group)
		{
			group->setParent(this);
			group->setParentPos(this);
			addGroup(group);
		}
		return 0;
	}
	// ------------------------------------------------------------------------------------------------
	// id = incomplete path (text:list:a) lid complete one (ui:interface:content:text:list:a)
	static bool reverseCheckPath(const string &id, const string &lid)
	{
		string idTmp = id, lidTmp = lid;
	//	bool isFound = true;
		while (!idTmp.empty())
		{
			string tokid, toklid;

			string::size_type posid = idTmp.rfind (":");
			if (posid == string::npos)
			{
				posid = 0;
				tokid = idTmp;
			}
			else
			{
				tokid = idTmp.substr (posid+1);
			}

			string::size_type poslid = lidTmp.rfind (":");
			if (poslid == string::npos)
			{
				poslid = 0;
				toklid = lidTmp;
			}
			else
			{
				toklid = lidTmp.substr (poslid+1);
			}

			if (tokid != toklid)
				return false;

			if (posid > 0)
				idTmp = idTmp.substr (0, posid);
			else
				idTmp.clear();

			if (poslid > 0)
				lidTmp = lidTmp.substr (0, poslid);
			else
				lidTmp.clear();
		}
		return true;
	}

	// ------------------------------------------------------------------------------------------------
	CViewBase*		CInterfaceGroup::getView  (const std::string &id)
	{
		vector<CViewBase*>::const_iterator itv;
		for (itv = _Views.begin(); itv != _Views.end(); ++itv)
		{
			CViewBase *pVB = (*itv);
			if (reverseCheckPath(id, pVB->getId()))
				return *itv;
		}
		// search in sons
		std::vector<CInterfaceGroup*>::const_iterator itg;
		for (itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end(); ++itg)
		{
			CViewBase* view = (*itg)->getView(id);
			if (view)
				return view;
		}
		return NULL;
	}

	// ------------------------------------------------------------------------------------------------
	CCtrlBase*		CInterfaceGroup::getCtrl  (const std::string &id)
	{
		vector<CCtrlBase*>::const_iterator itv;
		for (itv = _Controls.begin(); itv != _Controls.end(); ++itv)
		{
			CCtrlBase *pCB = (*itv);
			if (reverseCheckPath(id, pCB->getId()))
				return *itv;
		}
		// search in sons
		std::vector<CInterfaceGroup*>::const_iterator itg;
		for (itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end(); ++itg)
		{
			CCtrlBase*pCtrl = (*itg)->getCtrl (id);
			if (pCtrl)
				return pCtrl;
		}
		return NULL;
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceGroup*CInterfaceGroup::getGroup (const std::string &id) const
	{
		std::vector<CInterfaceGroup*>::const_iterator itg;
		for (itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end();itg++)
		{
			CInterfaceGroup *pIG = (*itg);
			if (reverseCheckPath(id, pIG->getId()))
				return *itg;
		}
		// search in sons
		for (itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end(); ++itg)
		{
			CInterfaceGroup *pCtrl = (*itg)->getGroup (id);
			if (pCtrl)
				return pCtrl;
		}
		return NULL;
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::delView  (CViewBase *child, bool dontDelete /* = false*/)
	{
		for (sint32 i = 0; i < (sint32)_Views.size(); ++i)
		{
			if (_Views[i] == child)
			{
				CViewBase *v = _Views[i];
				_Views.erase(_Views.begin()+i);
				delEltOrder (child);
				child->onRemoved();
				if (!dontDelete) delete v;
				return true;
			}
		}
		return false;
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::delCtrl  (CCtrlBase *child, bool dontDelete /* = false*/)
	{
		for (sint32 i = 0; i < (sint32)_Controls.size(); ++i)
		{
			if (_Controls[i] == child)
			{
				CCtrlBase *c = _Controls[i];
				_Controls.erase(_Controls.begin()+i);
				delEltOrder (child);
				child->onRemoved();
				if (!dontDelete) delete c;
				return true;
			}
		}
		return false;
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::delGroup (CInterfaceGroup *child, bool dontDelete /* = false*/)
	{
		for (sint32 i = 0; i < (sint32)_ChildrenGroups.size(); ++i)
		{
			if (_ChildrenGroups[i] == child)
			{
				CInterfaceGroup *g = _ChildrenGroups[i];
				_ChildrenGroups.erase(_ChildrenGroups.begin()+i);
				delEltOrder (child);
				child->onRemoved();
				if (!dontDelete) delete g;
				return true;
			}
		}
		return false;
	}

	// ------------------------------------------------------------------------------------------------
	int CInterfaceGroup::luaDelGroup (CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "CInterfaceGroup::delTab", 1);
		CInterfaceGroup * group = dynamic_cast<CInterfaceGroup *>(CLuaIHM::getUIOnStack(ls, 1));
		if(group)
		{
			delGroup(group);
		}
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	int CInterfaceGroup::luaGetNumGroups(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "CInterfaceGroup::getNumGroups", 0);
		ls.push((uint)_ChildrenGroups.size());
		return 1;
	}

	// ------------------------------------------------------------------------------------------------
	int CInterfaceGroup::luaGetGroup(CLuaState &ls)
	{
		const char *funcName = "CInterfaceGroup::getGroup";
		CLuaIHM::checkArgCount(ls, "CInterfaceGroup::getGroup", 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		uint index = (uint) ls.toInteger(1);
		if (index >= _ChildrenGroups.size())
		{
			CLuaIHM::fails(ls, "getGroup : try to index group %s, but there are only %d son groups", ls.toString(1), (int) _ChildrenGroups.size());
		}
		CLuaIHM::pushUIOnStack(ls, _ChildrenGroups[index]);
		return 1;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::delEltOrder (CViewBase *pElt)
	{
		for (sint32 i = 0; i < (sint32)_EltOrder.size(); ++i)
		{
			if (_EltOrder[i] == pElt)
			{
				_EltOrder.erase (_EltOrder.begin()+i);
				return;
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::delView  (const std::string &id, bool dontDelete /* = false*/)
	{
		return delView(getView(id), dontDelete);
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::delCtrl  (const std::string &id, bool dontDelete /* = false*/)
	{
		return delCtrl(getCtrl(id), dontDelete);
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::delGroup (const std::string &id, bool dontDelete /* = false*/)
	{
		return delGroup(getGroup(id), dontDelete);
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::isChildGroup(const CInterfaceGroup *group) const
	{
		return std::find(_ChildrenGroups.begin(), _ChildrenGroups.end(), group) != _ChildrenGroups.end();
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::handleEvent (const NLGUI::CEventDescriptor &event)
	{
		if (CCtrlBase::handleEvent(event)) return true;
		if (!_Active)
			return false;

		if (event.getType() == NLGUI::CEventDescriptor::system)
		{
			NLGUI::CEventDescriptorSystem &eds = (NLGUI::CEventDescriptorSystem&)event;
			if (eds.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::activecalledonparent)
			{
				// notify all childrens
				notifyActiveCalled((NLGUI::CEventDescriptorActiveCalledOnParent &) eds);
			}
		}

		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;

			if (!isIn(eventDesc.getX(), eventDesc.getY()))
				return false;

			bool taken = false;

			// For each control in the group...
			vector<CCtrlBase*>::const_iterator itc;
			for (itc = _Controls.begin(); itc != _Controls.end(); itc++)
			{
				CCtrlBase *pCB = *itc;
				if (pCB->getActive())
					taken = taken || pCB->handleEvent(eventDesc);
			}
			if (taken)
				return true;

			// For each child group
			for (sint i = (sint)_ChildrenGroups.size()-1; i >= 0; --i)
			{
				CInterfaceGroup *pIG = _ChildrenGroups[i];
				if (pIG->getActive())
					if (pIG->handleEvent(eventDesc))
						return true;
			}


			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown)
			{
				if (_AHOnLeftClick != NULL)
				{
					CAHManager::getInstance()->runActionHandler(_AHOnLeftClick, this, _AHOnLeftClickParams);
					return true;
				}
			}

			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightup)
			{
				if (_AHOnRightClick != NULL)
				{
					CAHManager::getInstance()->runActionHandler(_AHOnRightClick, this, _AHOnRightClickParams);
					return true;
				}
			}
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousewheel)
			{
				// handle the Mouse Wheel only if interesting
				if (_H>_MaxH)
				{
					CInterfaceGroup *currParent = _Parent;
					while (currParent)
					{
						if (currParent->moveSBTargetY (this, -(eventDesc.getWheel()*12)))
							return true;
						currParent = currParent->getParent();
					}
				}
			}
		}
		return false;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::executeControl (const std::string &/* sControlName */)
	{
	//	bool taken = false;
	//	CCtrlBase *pIC = getCtrl (sControlName);
	//	if (pIC != NULL)
	//		pIC->callback(taken);
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::draw ()
	{
		sint32 oldSciX, oldSciY, oldSciW, oldSciH;
		makeNewClip (oldSciX, oldSciY, oldSciW, oldSciH);

		// Display sons only if not total clipped
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		if( !rVR.isClipWindowEmpty() )
		{
			// Draw all decorative elements
			vector<CViewBase*>::const_iterator ite;
			for (ite = _EltOrder.begin() ; ite != _EltOrder.end(); ite++)
			{
				CViewBase *pVB = *ite;
				if( pVB->getName() == "=MARKED=" )
				{
					nlinfo( "=MARKED=" );
				}

				if (pVB->getActive())
					pVB->draw();
			}
		}

		restoreClip (oldSciX, oldSciY, oldSciW, oldSciH);
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::drawNoClip()
	{
		// Display sons only if not total clipped
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		if( !rVR.isClipWindowEmpty() )
		{
			// Draw all decorative elements
			vector<CViewBase*>::const_iterator ite;
			for (ite = _EltOrder.begin() ; ite != _EltOrder.end(); ite++)
			{
				CViewBase *pVB = *ite;
				if (pVB->getActive())
					pVB->draw();
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::renderWiredQuads(CInterfaceElement::TRenderWired type, const std::string &uiFilter)
	{
		if (!_Active) return;
		CInterfaceElement::renderWiredQuads(type, uiFilter);
		sint32 oldSciX, oldSciY, oldSciW, oldSciH;
		makeNewClip (oldSciX, oldSciY, oldSciW, oldSciH);
		for(std::vector<CViewBase*>::iterator it = _EltOrder.begin(); it != _EltOrder.end(); ++it)
		{
			if (*it) (*it)->renderWiredQuads(type, uiFilter);
		}
		restoreClip (oldSciX, oldSciY, oldSciW, oldSciH);
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::drawElement (CViewBase *el)
	{
		sint32 oldSciX, oldSciY, oldSciW, oldSciH;
		makeNewClip (oldSciX, oldSciY, oldSciW, oldSciH);

		if(el)
			el->draw();

		restoreClip (oldSciX, oldSciY, oldSciW, oldSciH);
	}

	// ------------------------------------------------------------------------------------------------
	sint32 CInterfaceGroup::getInnerWidth() const
	{
		sint width = CInterfaceElement::getInnerWidth();
		return std::min(width, _MaxWReal - _MarginLeft);
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::checkCoords()
	{
		// Make XReal same as in updateCoords() as some elements (CViewText) depends on it
		_XReal += _MarginLeft;

		//update all children elements
		vector<CViewBase*>::const_iterator ite;
		for (ite = _EltOrder.begin() ; ite != _EltOrder.end(); ite++)
		{
			CViewBase *pIE = *ite;
			if(pIE->getActive())
				pIE->checkCoords();
		}

		_XReal -= _MarginLeft;
		executeLuaScriptOnDraw();
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::executeLuaScriptOnDraw()
	{
		// If some LUA script attached to me, execute it
		if(!_LUAOnDraw.empty())
			CAHManager::getInstance()->runActionHandler("lua", this, _LUAOnDraw);

	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::doUpdateCoords()
	{
		_MaxWReal = _MaxW;
		_MaxHReal = _MaxH;

		CInterfaceElement *el = NULL;

		if (_ParentSizeMax != NULL)
		{
			el = _ParentSizeMax;
		}
		else
		{
			if (_ParentSize != NULL)
			{
				el = _ParentSize;
			}
			else
			{
				if (_ParentPos != NULL)
					el = _ParentPos;
				else
					el = _Parent;
			}
		}

		if (el != NULL)
		{
			if (_GroupSizeRef&1)
				_MaxWReal += _SizeDivW * el->getWReal() / 10;

			if (_GroupSizeRef&2)
				_MaxHReal += _SizeDivH * el->getHReal() / 10;
		}

		CViewBase::updateCoords();
		_XReal += _OffsetX + _MarginLeft;
		_YReal += _OffsetY;

		//update all children elements
		vector<CViewBase*>::const_iterator ite;
		for (ite = _EltOrder.begin() ; ite != _EltOrder.end(); ite++)
		{
			CViewBase *pIE = *ite;
			pIE->updateCoords();
		}

		_XReal -= (_OffsetX + _MarginLeft);
		_YReal -= _OffsetY;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::updateCoords()
	{
		// update basis and sons one time.
		doUpdateCoords();

		// if the group must resize from children
		if(_ResizeFromChildH || _ResizeFromChildW)
		{
			// compute BBox of all childrens
			sint	width, height;
			evalChildrenBBox(_ResizeFromChildW, _ResizeFromChildH, width, height);
			// set forced size.
			if(_ResizeFromChildW)
			{
				_W= _ResizeFromChildWMargin + width;
			}
			if(_ResizeFromChildH)
			{
				_H= _ResizeFromChildHMargin + height;
			}
		}

		CInterfaceElement::updateCoords();
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::evalChildrenBBox(bool resizeFromChildW, bool resizeFromChildH, sint &width, sint &height) const
	{
		sint	yMin=INT_MAX, yMax=INT_MIN;
		sint	xMin=INT_MAX, xMax=INT_MIN;
		vector<CViewBase*>::const_iterator ite;
		for (ite = _EltOrder.begin() ; ite != _EltOrder.end(); ite++)
		{
			CViewBase *pIE = *ite;
			if (pIE->getActive())
			{
				const CInterfaceElement *el = pIE->getParentPos() ? pIE->getParentPos() : pIE->getParent();
				if (el == this)
				{
					// to compute the bbox, don't use direct sons that have a sizeref on the resizing coordinate
					if ((resizeFromChildW && (pIE->getSizeRef() & 1))
						|| (resizeFromChildH && (pIE->getSizeRef() & 2))
					   )
					{
						continue;
					}
				}
				// avoid also some interface elements
				if(pIE->avoidResizeParent())
					continue;
				// get the real coords bounds.
				sint32	x0,y0,x1,y1;
				// If it is a group, minimize with MaxHReal / MaxWReal
				const CInterfaceGroup	*sonGroup= dynamic_cast<const CInterfaceGroup*>(pIE);
				// \todo yoyo: do not know why but don't work if this==scroll_text
				if(sonGroup && !isGroupScrollText())
				{
					sint32	w, h;
					sonGroup->computeClipContribution(x0, y0, w, h);
					x1= x0 + w;
					y1= y0 + h;
				}
				else
				{
					x0= pIE->getXReal();
					y0= pIE->getYReal();
					x1= x0 + pIE->getWReal();
					y1= y0 + pIE->getHReal();
				}
				// enlarge
				if(x0<xMin)
					xMin= x0;
				if(y0<yMin)
					yMin= y0;
				if(x1>xMax)
					xMax= x1;
				if(y1>yMax)
					yMax= y1;
			}
		}
		width = xMax - xMin;
		height = yMax - yMin;
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceElement* CInterfaceGroup::getElement (const std::string &id)
	{
		if (_Id == id)
			return this;

		if (id.compare(0, _Id.size(), _Id) != 0)
			return NULL;

		vector<CViewBase*>::const_iterator itv;
		for (itv = _Views.begin(); itv != _Views.end(); itv++)
		{
			CViewBase *pVB = *itv;
	#if !FINAL_VERSION		// For SpeedUp in final version
			nlassert(pVB);	// The element must not be NULL
	#endif
			if (pVB->getId() == id)
				return pVB;
		}

		vector<CCtrlBase*>::const_iterator itc;
		for (itc = _Controls.begin(); itc != _Controls.end(); itc++)
		{
			CCtrlBase* ctrl = *itc;
	#if !FINAL_VERSION		// For SpeedUp in final version
			nlassert(ctrl);	// The element must not be NULL
	#endif
			if (ctrl->getId() == id)
				return ctrl;
		}

		vector<CInterfaceGroup*>::const_iterator itg;
		for (itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end(); itg++)
		{
			CInterfaceGroup *pIG = *itg;
	#if !FINAL_VERSION		// For SpeedUp in final version
			nlassert(pIG);	// The element must not be NULL
	#endif
			CInterfaceElement *pIEL = pIG->getElement(id);
			if (pIEL != NULL)
				return pIEL;
		}
		return NULL;
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::delElement (const std::string &id, bool noWarning)
	{
		if (id.substr(0, _Id.size()) != _Id)
			return false;

		vector<CViewBase*>::const_iterator itv;
		for (itv = _Views.begin(); itv != _Views.end(); itv++)
		{
			CViewBase *pVB = *itv;
			if (pVB->getId() == id)
			{
				delView  (pVB, false);
				return true;
			}
		}

		vector<CCtrlBase*>::const_iterator itc;
		for (itc = _Controls.begin(); itc != _Controls.end(); itc++)
		{
			CCtrlBase* ctrl = *itc;
			if (ctrl->getId() == id)
			{
				delCtrl (ctrl, false);
				return true;
			}
		}

		vector<CInterfaceGroup*>::const_iterator itg;
		for (itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end(); itg++)
		{
			CInterfaceGroup *pIG = *itg;
			if (pIG->getId() == id)
			{
				// If this is a root window
				if (pIG->getRootWindow () == pIG)
					CWidgetManager::getInstance()->unMakeWindow(pIG, noWarning);
				delGroup (pIG, false);
				return true;
			}
		}
		return false;
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::delElement (CInterfaceElement *pIE, bool noWarning)
	{
		// delete correct type of element
		if(pIE->isGroup())
		{
			CInterfaceGroup		*pIG= static_cast<CInterfaceGroup*>(pIE);
			// unmake window if it is
			if (pIG->getRootWindow () == pIG)
				CWidgetManager::getInstance()->unMakeWindow(pIG, noWarning);
			return delGroup(pIG);
		}
		else if(pIE->isCtrl())
		{
			return delCtrl(static_cast<CCtrlBase*>(pIE));
		}

		nlassert(pIE->isView());
		return delView(static_cast<CViewBase*>(pIE));
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceElement* CInterfaceGroup::takeElement( CInterfaceElement *e )
	{
		bool ok = false;

		if( e->isGroup() )
		{
			ok = delGroup( static_cast< CInterfaceGroup* >( e ), true );
		}
		else
		if( e->isCtrl() )
		{
			ok = delCtrl( static_cast< CCtrlBase* >( e ), true );
		}
		else
		if( e->isView() )
		{
			ok = delView( static_cast< CViewBase* >( e ), true );
		}

		if( ok )
			return e;
		else
			return NULL;
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::isWindowUnder (sint32 x, sint32 y)
	{
		return ((x >= _XReal) &&
				(x < (_XReal + _WReal))&&
				(y > _YReal) &&
				(y <= (_YReal + _HReal)));
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceGroup* CInterfaceGroup::getGroupUnder (sint32 x, sint32 y)
	{
		// Begins by the children
		std::vector<CInterfaceGroup*>::const_iterator itg;
		for (itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end(); itg++)
		{
			CInterfaceGroup *pChild = *itg;
			CInterfaceGroup *pChildUnder = pChild->getGroupUnder (x-_XReal, y-_YReal);
			if (pChildUnder != NULL)
			{
				if ( (x >= _XReal) &&
					(x < (_XReal + _WReal))&&
					(y > _YReal) &&
					(y <= (_YReal+ _HReal)))
					return pChildUnder;
			}
		}

		// If not found in childs then try in the parent one
		if ( (x >= _XReal) &&
			 (x < (_XReal + _WReal))&&
			 (y > _YReal) &&
			 (y <= (_YReal+ _HReal)))
			 return this;
		return NULL;
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::getViewsUnder (sint32 x, sint32 y, sint32 clipX, sint32 clipY, sint32 clipW, sint32 clipH, std::vector<CViewBase*> &vVB)
	{
		if (!((x >= _XReal) &&
			(x < (_XReal + _WReal))&&
			(y > _YReal) &&
			(y <= (_YReal+ _HReal))))
			return false;
		// test against current clip
		computeCurrentClipContribution(clipX, clipY, clipW, clipH,
									   clipX, clipY, clipW, clipH);

		if (!((x > clipX) &&
			(x < (clipX + clipW))&&
			(y > clipY) &&
			(y < (clipY + clipH))))
			return false;



		// same as draw order: start with parent
		std::vector<CViewBase*>::const_iterator itc;
		for (itc = _EltOrder.begin(); itc != _EltOrder.end(); itc++)
		{
			if (!(*itc)->isCtrl() && !(*itc)->isGroup()) // must be a view
			{
				CViewBase *pVB = *itc;
				if (pVB != NULL)
				if (pVB->getActive())
				if ( ((x) > pVB->getXReal()) &&
					 ((x) < (pVB->getXReal() + pVB->getWReal()))&&
					 ((y) > pVB->getYReal()) &&
					 ((y) < (pVB->getYReal() + pVB->getHReal())))
				{
					vVB.push_back (pVB);
				}
			}
		}

		// same as draw order: continue with children
		std::vector<CInterfaceGroup*>::const_iterator itg;
		for (itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end(); itg++)
		{
			CInterfaceGroup *pChild = *itg;
			if (pChild->getActive())
			{
	//		bool bUnder =
				pChild->getViewsUnder (x, y, clipX, clipY, clipW, clipH, vVB);
	//			if (bUnder && !vICL.empty())
	//				return true;
			}
		}
		return true;
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::getCtrlsUnder (sint32 x, sint32 y, sint32 clipX, sint32 clipY, sint32 clipW, sint32 clipH, std::vector<CCtrlBase*> &vICL)
	{
		if (!((x >= _XReal) &&
			(x < (_XReal + _WReal))&&
			(y > _YReal) &&
			(y <= (_YReal+ _HReal))))
			return false;
		// test against current clip
		computeCurrentClipContribution(clipX, clipY, clipW, clipH,
									   clipX, clipY, clipW, clipH);

		if (!((x >= clipX) &&
			(x < (clipX + clipW))&&
			(y > clipY) &&
			(y <= (clipY + clipH))))
			return false;



		// same as draw order: start with parent
		std::vector<CViewBase*>::const_iterator itc;
		for (itc = _EltOrder.begin(); itc != _EltOrder.end(); itc++)
		{
			if ((*itc)->isCtrl() && !(*itc)->isGroup()) // must be a ctrl but not a group (parsed later)
			{
				CCtrlBase *pICL = (CCtrlBase *) *itc;
				if (pICL != NULL)
				if (pICL->getActive())
				if ( ((x) >= pICL->getXReal()) &&
					 ((x) < (pICL->getXReal() + pICL->getWReal()))&&
					 ((y) > pICL->getYReal()) &&
					 ((y) <= (pICL->getYReal() + pICL->getHReal())))
				{
					vICL.push_back (pICL->getSubCtrl(x,y));
				}
			}
		}

		// same as draw order: continue with children
		std::vector<CInterfaceGroup*>::const_iterator itg;
		for (itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end(); itg++)
		{
			CInterfaceGroup *pChild = *itg;
			if (pChild->getActive())
			{
	//		bool bUnder =
				pChild->getCtrlsUnder (x, y, clipX, clipY, clipW, clipH, vICL);
	//			if (bUnder && !vICL.empty())
	//				return true;
			}
		}
		return true;
	}


	// ------------------------------------------------------------------------------------------------
	bool CInterfaceGroup::getGroupsUnder (sint32 x, sint32 y, sint32 clipX, sint32 clipY, sint32 clipW, sint32 clipH, std::vector<CInterfaceGroup *> &vIGL)
	{
		if (!((x >= _XReal) &&
			(x < (_XReal + _WReal))&&
			(y > _YReal) &&
			(y <= (_YReal+ _HReal))))
			return false;
		// test against current clip
		computeCurrentClipContribution(clipX, clipY, clipW, clipH,
									   clipX, clipY, clipW, clipH);

		if (!((x >= clipX) &&
			(x < (clipX + clipW))&&
			(y > clipY) &&
			(y <= (clipY + clipH))))
			return false;



		// same as draw order: start with parent
		std::vector<CViewBase*>::const_iterator itc;
		for (itc = _EltOrder.begin(); itc != _EltOrder.end(); itc++)
		{
			if ((*itc)->isGroup()) // must be a group
			{
				CInterfaceGroup *pIGL = (CInterfaceGroup *) *itc;
				if (pIGL != NULL)
				if (pIGL->getActive())
				if ( ((x) >= pIGL->getXReal()) &&
					 ((x) < (pIGL->getXReal() + pIGL->getWReal()))&&
					 ((y) > pIGL->getYReal()) &&
					 ((y) <= (pIGL->getYReal() + pIGL->getHReal())))
				{
					vIGL.push_back (pIGL);
				}
			}
		}

		// same as draw order: continue with children
		std::vector<CInterfaceGroup*>::const_iterator itg;
		for (itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end(); itg++)
		{
			CInterfaceGroup *pChild = *itg;
			if (pChild->getActive())
			{
	//		bool bUnder =
				pChild->getGroupsUnder (x, y, clipX, clipY, clipW, clipH, vIGL);
	//			if (bUnder && !vICL.empty())
	//				return true;
			}
		}
		return true;
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::absoluteToRelative (sint32 &x, sint32 &y)
	{
		CInterfaceGroup *curGrp = _Parent;
		while (curGrp != NULL)
		{
			x = x - curGrp->_XReal;
			y = y - curGrp->_YReal;
			curGrp = curGrp->_Parent;
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::computeClipContribution(sint32 &newSciXDest, sint32 &newSciYDest, sint32 &newSciWDest, sint32 &newSciHDest) const
	{
		sint32 newSciX = _XReal;
		sint32 newSciY = _YReal;
		sint32 newSciW = _WReal;
		sint32 newSciH = _HReal;
		// If there is width size limiter
		if (_MaxWReal < _WReal)
		{
			if ((_PosRef == Hotspot_TR) || (_PosRef == Hotspot_MR) || (_PosRef == Hotspot_BR))
				newSciX = _XReal + _WReal - _MaxWReal;
			newSciW = _MaxWReal;
		}
		// If there is height size limiter
		if (_MaxHReal < _HReal)
		{
			if ((_PosRef == Hotspot_TL) || (_PosRef == Hotspot_TM) || (_PosRef == Hotspot_TR))
				newSciY = _YReal + _HReal - _MaxHReal;
			newSciH = _MaxHReal;
		}
		// Don't apply margins because HTML list marker is drawn outside group paragraph inner content.
		// Should not be an issue because horizontal scolling not used.
		newSciXDest = newSciX/* + _MarginLeft*/;
		newSciYDest = newSciY;
		newSciWDest = newSciW/* - _MarginLeft*/;
		newSciHDest = newSciH;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::computeCurrentClipContribution(sint32 oldSciX, sint32 oldSciY, sint32 oldSciW, sint32 oldSciH,
														 sint32 &newSciXDest, sint32 &newSciYDest, sint32 &newSciWDest, sint32 &newSciHDest) const
	{
		sint32 newSciX, newSciY, newSciW, newSciH;
		computeClipContribution(newSciX, newSciY, newSciW, newSciH);

		// Clip Left
		if (newSciX < oldSciX)
		{
			newSciW = newSciW - (oldSciX - newSciX);
			newSciX = oldSciX;
		}
		// Clip Right
		if ((newSciX+newSciW) > (oldSciX+oldSciW))
		{
			newSciW = newSciW - ((newSciX+newSciW)-(oldSciX+oldSciW));
		}
		// Clip Bottom
		if (newSciY < oldSciY)
		{
			newSciH = newSciH - (oldSciY - newSciY);
			newSciY = oldSciY;
		}
		// Clip Top
		if ((newSciY+newSciH) > (oldSciY+oldSciH))
		{
			newSciH = newSciH - ((newSciY+newSciH)-(oldSciY+oldSciH));
		}

		// Don't apply margins because HTML list marker is drawn outside group paragraph inner content.
		// Should not be an issue because horizontal scolling not used.
		newSciXDest = newSciX/* + _MarginLeft*/;
		newSciYDest = newSciY;
		newSciWDest = newSciW/* - _MarginLeft*/;
		newSciHDest = newSciH;

	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::makeNewClip (sint32 &oldSciX, sint32 &oldSciY, sint32 &oldSciW, sint32 &oldSciH)
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		rVR.getClipWindow (oldSciX, oldSciY, oldSciW, oldSciH);

		sint32 newSciX, newSciY, newSciW, newSciH;
		computeCurrentClipContribution(oldSciX, oldSciY, oldSciW, oldSciH, newSciX, newSciY, newSciW, newSciH);
		rVR.setClipWindow (newSciX, newSciY, newSciW, newSciH);
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::getClip(sint32 &x, sint32 &y, sint32 &w, sint32 &h) const
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		uint32 sw, sh;
		rVR.getScreenSize(sw, sh);
		sint32 sciX = 0, sciY = 0, sciW = sw, sciH =sh;
		const CInterfaceGroup *currGroup = this;
		do
		{
			currGroup->computeCurrentClipContribution(sciX, sciY, sciW, sciH, sciX, sciY, sciW, sciH);
			currGroup = currGroup->_Parent;
		} while(currGroup);
		x = sciX;
		y = sciY;
		w = sciW;
		h = sciH;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::restoreClip (sint32 oldSciX, sint32 oldSciY, sint32 oldSciW, sint32 oldSciH)
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		rVR.setClipWindow (oldSciX, oldSciY, oldSciW, oldSciH);
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::launch ()
	{
		// launch all elements
		vector<CViewBase*>::const_iterator ite;
		for (ite = _EltOrder.begin() ; ite != _EltOrder.end(); ite++)
		{
			CViewBase *pIE = *ite;
			pIE->launch();
		}
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceGroup *CInterfaceGroup::getGroup(uint index) const
	{
		if (index > _ChildrenGroups.size())
		{
			nlwarning("<CInterfaceGroup::getGroup()> bad index;");
			return NULL;
		}
		return _ChildrenGroups[index];
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::updateAllLinks()
	{
		CCtrlBase::updateAllLinks();
		{
			for(std::vector<CInterfaceGroup*>::iterator it = _ChildrenGroups.begin(); it != _ChildrenGroups.end(); ++it)
			{
				(*it)->updateAllLinks();
			}
		}
		{
			for(std::vector<CCtrlBase*>::iterator it = _Controls.begin(); it != _Controls.end(); ++it)
			{
				(*it)->updateAllLinks();
			}
		}
		{
			for(std::vector<CViewBase*>::iterator it = _Views.begin(); it != _Views.end(); ++it)
			{
				(*it)->updateAllLinks();
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	sint32 CInterfaceGroup::getAlpha() const
	{
		vector<CViewBase*>::const_iterator itv;
		for (itv = _Views.begin(); itv != _Views.end(); itv++)
		{
			CViewBase *pVB = *itv;
			sint32 a = pVB->getAlpha();
			if (a != -1)
				return a;
		}

		vector<CCtrlBase*>::const_iterator itc;
		for (itc = _Controls.begin(); itc != _Controls.end(); itc++)
		{
			CCtrlBase *pCB = *itc;
			sint32 a = pCB->getAlpha();
			if (a != -1)
				return a;
		}

		vector<CInterfaceGroup*>::const_iterator itg;
		for (itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end(); itg++)
		{
			CInterfaceGroup *pIG = *itg;
			sint32 a = pIG->getAlpha();
			if (a != -1)
				return a;
		}
		return -1;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::setAlpha (sint32 a)
	{
		vector<CViewBase*>::const_iterator itv;
		for (itv = _Views.begin(); itv != _Views.end(); itv++)
		{
			CViewBase *pVB = *itv;
			pVB->setAlpha(a);
		}

		vector<CCtrlBase*>::const_iterator itc;
		for (itc = _Controls.begin(); itc != _Controls.end(); itc++)
		{
			CCtrlBase *pCB = *itc;
			pCB->setAlpha(a);
		}

		vector<CInterfaceGroup*>::const_iterator itg;
		for (itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end(); itg++)
		{
			CInterfaceGroup *pIG = *itg;
			pIG->setAlpha(a);
		}
	}

	// ------------------------------------------------------------------------------------------------
	void	CInterfaceGroup::setLeftClickHandler(const std::string &handler)
	{
		_AHOnLeftClick = CAHManager::getInstance()->getAH(handler, _AHOnLeftClickParams);
	}

	// ------------------------------------------------------------------------------------------------
	void	CInterfaceGroup::setRightClickHandler(const std::string &handler)
	{
		_AHOnRightClick = CAHManager::getInstance()->getAH(handler, _AHOnRightClickParams);
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceGroup* CInterfaceGroup::getEnclosingContainer()
	{
		CInterfaceGroup *ig = this;
		do
		{
			if( ig->isGroupContainer() )
				return ig;
			ig = ig->getParent();
		}
		while( ig != NULL );

		return NULL;
	}

	// ------------------------------------------------------------------------------------------------
	int CInterfaceGroup::luaDumpSize(CLuaState &ls)
	{
		const char *funcName = "dumpSize";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		dumpSize();
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	int CInterfaceGroup::luaDumpEltsOrder(CLuaState &ls)
	{
		const char *funcName = "dumpEltsOrder";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		dumpEltsOrder();
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	int CInterfaceGroup::luaDumpGroups(CLuaState &ls)
	{
		const char *funcName = "dumpGroups";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		dumpGroups();
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::dumpGroups() const
	{
		nlinfo("Num groups = %d", (int) _ChildrenGroups.size());
		for(uint k = 0; k < _ChildrenGroups.size(); ++k)
		{
			std::string typeName = "???";
			if (_ChildrenGroups[k])
			{
				NLGUI::CInterfaceGroup *group = _ChildrenGroups[k];
				const type_info &ti = typeid(*group);
				typeName = ti.name();
			}
			nlinfo("Group %d, name = %s, type=%s", k,  _ChildrenGroups[k] ? _ChildrenGroups[k]->getId().c_str() : "???", typeName.c_str());
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::dumpEltsOrder() const
	{
		nlinfo("Num elements = %d, num groups = %d", (int) _EltOrder.size(), _ChildrenGroups.size());
		for(uint k = 0; k < _EltOrder.size(); ++k)
		{
			CInterfaceElement *el = _EltOrder[k];
			if (el)
			{
				std::string typeName;
				NLGUI::CViewBase *view = _EltOrder[k];
				const type_info &ti = typeid(*view);
				typeName = ti.name();
				nlinfo("Element %d, name = %s, type=%s, x=%d, y=%d, parent_name=%s parentposname=%s xreal=%d, yreal=%d, wreal=%d, hreal=%d",
					   k, el->getId().c_str(), typeName.c_str(), el->getX(), el->getY(), el->getParent() ? el->getParent()->getId().c_str() : "no parent",
					   el->getParentPos() ? el->getParentPos()->getId().c_str() : "parent",
					   (int) el->getXReal(),
					   (int) el->getYReal(),
					   (int) el->getWReal(),
					   (int) el->getHReal()
					  );
			}
			else
			{
				nlinfo("Element %d = NULL", (int) k);
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	sint CInterfaceGroup::getInsertionOrder(CViewBase *vb) const
	{
		for(uint k = 0; k < _EltOrder.size(); ++k)
		{
			if (_EltOrder[k] == vb) return (sint) k;
		}
		return -1;
	}

	// ------------------------------------------------------------------------------------------------
	sint32 CInterfaceGroup::getMaxUsedW() const
	{
		sint maxWidth = 0;
		for (uint k = 0; k < _EltOrder.size(); ++k)
		{
			// Get the child width
			sint32 width = _EltOrder[k]->getMaxUsedW()+_EltOrder[k]->getXReal() - getXReal();
			if (width > maxWidth)
				maxWidth = width;
		}
		return maxWidth;
	}

	// ------------------------------------------------------------------------------------------------
	sint32 CInterfaceGroup::getMinUsedW() const
	{
		sint32 minWidth = 0;
		for (uint k = 0; k < _EltOrder.size(); ++k)
		{
			// Get the child width
			sint32 width = _EltOrder[k]->getMinUsedW()+_EltOrder[k]->getXReal() - getXReal();
			if (width > minWidth)
				minWidth = width;
		}
		return minWidth;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::clearAllEditBox()
	{
		for(uint k = 0; k < _ChildrenGroups.size(); ++k)
		{
			if (_ChildrenGroups[k]) _ChildrenGroups[k]->clearAllEditBox();
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::restoreAllContainersBackupPosition()
	{
		for(uint k = 0; k < _ChildrenGroups.size(); ++k)
		{
			if (_ChildrenGroups[k]) _ChildrenGroups[k]->restoreAllContainersBackupPosition();
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::dumpSize(uint depth /*=0*/) const
	{
		CViewBase::dumpSize(depth);
		for(uint k = 0; k < _ChildrenGroups.size(); ++k)
		{
			_ChildrenGroups[k]->dumpSize(depth + 1);
		}
		for(uint k = 0; k < _Controls.size(); ++k)
		{
			_Controls[k]->dumpSize(depth + 1);
		}
		for(uint k = 0; k < _Views.size(); ++k)
		{
			_Views[k]->dumpSize(depth + 1);
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::visit(CInterfaceElementVisitor *visitor)
	{
		nlassert(visitor);
		for(uint k = 0; k < _ChildrenGroups.size(); ++k)
		{
			_ChildrenGroups[k]->visit(visitor);
		}
		for(uint k = 0; k < _Controls.size(); ++k)
		{
			_Controls[k]->visit(visitor);
		}
		for(uint k = 0; k < _Views.size(); ++k)
		{
			_Views[k]->visit(visitor);
		}
		visitor->visitGroup(this);
		CInterfaceElement::visit(visitor);
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::visitGroupAndChildren( CInterfaceElementVisitor *visitor )
	{
		nlassert( visitor != 0 );
		for( uint i = 0; i < _ChildrenGroups.size(); i++ )
		{
			_ChildrenGroups[ i ]->visitGroupAndChildren( visitor );
		}

		visitor->visitGroup( this );
	}


	// ------------------------------------------------------------------------------------------------

	void CInterfaceGroup::setUseCursor(bool use)
	{
		_UseCursor=use;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::onFrameUpdateWindowPos(sint dx, sint dy)
	{
		// Move me.
		_XReal+= dx;
		_YReal+= dy;

		// Move all my sons.
		vector<CViewBase*>::const_iterator ite;
		for (ite = _EltOrder.begin() ; ite != _EltOrder.end(); ite++)
		{
			CViewBase *pIE = *ite;
			pIE->onFrameUpdateWindowPos(dx, dy);
		}
	}

	// ------------------------------------------------------------------------------------------------
	void	CInterfaceGroup::pushLUAEnvTable()
	{
		CLuaState *lua= CLuaManager::getInstance().getLuaState();
		nlassert(lua);

		if(!_LUAEnvTableCreated)
		{
			CLuaStackChecker lsc(lua);

			// Create a table and assign it in the REGISTRY."__ui_envtable"  table, with a userdata ptr
			lua->push(IHM_LUA_ENVTABLE);
			lua->getTable(LUA_REGISTRYINDEX);		// Stack: __ui_envtable
			lua->pushLightUserData(IG_UNIQUE_ID(this));
			lua->newTable();
			lua->setTable(-3);						// Stack: __ui_envtable  (with .this={})
			lua->pop();

			// Created!
			_LUAEnvTableCreated= true;
		}

		// Get the table from registry, and push it on stack
		CLuaStackChecker lsc(lua, 1);
		lua->push(IHM_LUA_ENVTABLE);
		lua->getTable(LUA_REGISTRYINDEX);		// __ui_envtable
		lua->pushLightUserData(IG_UNIQUE_ID(this));
		lua->getTable(-2);						// __ui_envtable   envtable
		lua->remove(-2);						// envtable
		nlassert(lua->isTable());
	}

	// ------------------------------------------------------------------------------------------------
	void	CInterfaceGroup::deleteLUAEnvTable(bool recurse)
	{
		if(_LUAEnvTableCreated)
		{
			CLuaState *lua= CLuaManager::getInstance().getLuaState();
			nlassert(lua);

			// replace simply the table with Nil, letting LUA Garbage collector do the realease stuff
			CLuaStackChecker lsc(lua);
			lua->push(IHM_LUA_ENVTABLE);
			lua->getTable(LUA_REGISTRYINDEX);		// __ui_envtable
			lua->pushLightUserData(IG_UNIQUE_ID(this)); // NB nico : use some pointer *inside* that object as a unique id (any field but
																   // the first), instead of using 'this'. 'this' is already used by
																   //  CLuaIHM::pushReflectableOnStack
			lua->pushNil();
			lua->setTable(-3);						// __ui_envtable
			lua->pop();

			_LUAEnvTableCreated= false;
		}
		if (recurse)
		{
			for(uint k = 0; k < _ChildrenGroups.size(); ++k)
			{
				_ChildrenGroups[k]->deleteLUAEnvTable(true);
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	void	CInterfaceGroup::setLuaScriptOnDraw(const std::string &script)
	{
		_LUAOnDraw= script;
	}

	// ------------------------------------------------------------------------------------------------
	void	CInterfaceGroup::addLuaScriptOnDBChange(const std::string &dbList, const std::string &script)
	{
		// remove any existing
		removeLuaScriptOnDBChange(dbList);

		// create and attach the link
		NLMISC::CSmartPtr<CInterfaceLink>	newLink= new CInterfaceLink;
		_LUAOnDbChange[dbList]= newLink;
		// Init and attach to list of untargeted links
		std::vector<CInterfaceLink::CTargetInfo> noTargets;
		std::vector<CInterfaceLink::CCDBTargetInfo> noCdbTargets;
		newLink->init(noTargets, noCdbTargets, NLMISC::toString("depends(%s)", dbList.c_str()), "lua", script, "", this);
	}

	// ------------------------------------------------------------------------------------------------
	void	CInterfaceGroup::removeLuaScriptOnDBChange(const std::string &dbList)
	{
		TLUAOnDbChange::iterator	it= _LUAOnDbChange.find(dbList);
		if(it!=_LUAOnDbChange.end())
		{
			nlassert(it->second!=NULL);
			// Remove from link of untargeted Links
			it->second->uninit();
			// erase from map (thus the ptr should be deleted)
			_LUAOnDbChange.erase(it);
		}
	}

	// ------------------------------------------------------------------------------------------------
	void	CInterfaceGroup::removeAllLUAOnDbChange()
	{
		TLUAOnDbChange::iterator	it= _LUAOnDbChange.begin();
		for(;it!=_LUAOnDbChange.end();)
		{
			// since deleted, must not do it++, but it= begin()
			removeLuaScriptOnDBChange(it->first);
			it= _LUAOnDbChange.begin();
		}
	}

	// ------------------------------------------------------------------------------------------------
	int CInterfaceGroup::luaFind(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "CInterfaceGroup::find", 1);
		CLuaIHM::checkArgType(ls, "CInterfaceGroup::find", 1, LUA_TSTRING);
		std::string id = ls.toString(1);
		CInterfaceElement* element = findFromShortId(id);
		if (!element)
		{
			ls.pushNil();
		}
		else
		{
			CLuaIHM::pushUIOnStack(ls, element);
		}
		return 1;
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceElement* CInterfaceGroup::findFromShortId(const std::string &id)
	{
		CInterfaceElement* element = NULL;
		element = getView(id);
		if (!element) element = getCtrl(id);
		if (!element) element = getGroup(id);
		return element;
	}

	// ------------------------------------------------------------------------------------------------
	int CInterfaceGroup::luaGetEnclosingContainer(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "CInterfaceGroup::getEnclosingContainer", 0);
		CLuaIHM::pushUIOnStack(ls, getEnclosingContainer());
		return 1;
	}

	// ------------------------------------------------------------------------------------------------
	int CInterfaceGroup::luaDeleteLUAEnvTable(CLuaState &ls)
	{
		const char *funcName = "deleteLUAenvTable";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TBOOLEAN); // is delete recursive
		deleteLUAEnvTable(ls.toBoolean(1));
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::serial(NLMISC::IStream &f)
	{
		CCtrlBase::serial(f);
		f.serialContPolyPtr(_ChildrenGroups);
		f.serialContPolyPtr(_Controls);
		f.serialContPolyPtr(_Views);
		f.serialContPolyPtr(_EltOrder);
		f.serial(_MaxW, _MaxH);
		f.serial(_MaxWReal, _MaxHReal);
		f.serial(_OffsetX, _OffsetY);
		f.serial(_Priority);

		nlSerialBitBool(f, _Overlappable);
		nlSerialBitBool(f, _ResizeFromChildW);
		nlSerialBitBool(f, _ResizeFromChildH);
		nlSerialBitBool(f, _Escapable);
		nlSerialBitBool(f, _UseCursor);
		nlSerialBitBool(f, _IsGroupContainer);
		nlSerialBitBool(f, _NeedFrameUpdatePos);

		f.serial(_ResizeFromChildWMargin);
		f.serial(_ResizeFromChildHMargin);
		f.serial(_GroupSizeRef);

		serialAH(f, _AHOnActive);
		f.serial(_AHOnActiveParams);
		serialAH(f, _AHOnDeactive);
		f.serial(_AHOnDeactiveParams);

		// right & left clicks
		serialAH(f, _AHOnLeftClick);
		f.serial(_AHOnLeftClickParams);
		serialAH(f, _AHOnRightClick);
		f.serial(_AHOnRightClickParams);

		// enter params.
		serialAH(f, _AHOnEnter);
		f.serial(_AHOnEnterParams);

		// escape AH
		serialAH(f, _AHOnEscape);
		f.serial(_AHOnEscapeParams);
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceElement *CInterfaceGroup::clone()
	{
		CInterfaceElement *oldParentSizeMax = _ParentSizeMax;
		if (_ParentSizeMax == _Parent)
		{
			_ParentSizeMax = NULL;
		}
		CInterfaceElement *ret = CCtrlBase::clone();
		_ParentSizeMax	 = oldParentSizeMax;
		return ret;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceGroup::setMaxSizeRef(const std::string &maxSizeRef)
	{
		parseMaxSizeRef(maxSizeRef.c_str());
	}

	// ------------------------------------------------------------------------------------------------
	std::string CInterfaceGroup::getMaxSizeRefAsString() const
	{
		return "IMPLEMENT ME!";
	}

	void CInterfaceGroup::onWidgetDeleted( CInterfaceElement *e )
	{
		for( std::vector< CViewBase* >::iterator itr = _Views.begin(); itr != _Views.end(); ++itr )
			(*itr)->onWidgetDeleted( e );

		for( std::vector< CCtrlBase* >::iterator itr = _Controls.begin(); itr != _Controls.end(); ++itr )
			(*itr)->onWidgetDeleted( e );

		for( std::vector< CInterfaceGroup* >::iterator itr = _ChildrenGroups.begin(); itr != _ChildrenGroups.end(); ++itr )
			(*itr)->onWidgetDeleted( e );
	}

	void CInterfaceGroup::moveBy(sint32 x, sint32 y)
	{
		CInterfaceElement::moveBy(x, y);

		for(uint i = 0; i < _EltOrder.size(); ++i)
		{
			CViewBase *v = _EltOrder[i];
			v->updateCoords();
		}
	}

	bool CInterfaceGroup::explode()
	{
		CInterfaceGroup *p = getParent();
		if( p == NULL )
			return false;

		std::string oldId;

		// Reparent children
		for(uint i = 0; i < _EltOrder.size(); ++i)
		{
			CInterfaceElement *e = _EltOrder[i];

			oldId = e->getId();

			e->setW(e->getWReal());
			e->setH(e->getHReal());
			e->setSizeRef("");

			e->setParent(p);

			e->setParentPos(p);
			e->setParentSize(p);
			e->alignTo(p);

			p->addElement(e);
			e->setIdRecurse(e->getShortId());

			CWidgetManager::getInstance()->onWidgetMoved(oldId, e->getId());
		}

		_EltOrder.clear();
		_Views.clear();
		_Controls.clear();
		_ChildrenGroups.clear();

		return true;
	}

	void CInterfaceGroup::spanElements()
	{
		sint32 minx = std::numeric_limits< sint32 >::max();
		sint32 miny = std::numeric_limits< sint32 >::max();
		sint32 maxx = std::numeric_limits< sint32 >::min();
		sint32 maxy = std::numeric_limits< sint32 >::min();

		sint32 tlx, tly, brx, bry;

		// Find the min and max coordinates of the elements
		for(uint i = 0; i < _EltOrder.size(); ++i)
		{
			CViewBase *v = _EltOrder[i];

			v->getHSCoords(Hotspot_TL, tlx, tly);
			v->getHSCoords(Hotspot_BR, brx, bry);

			if (tlx < minx)
				minx = tlx;
			if (brx > maxx)
				maxx = brx;
			if (bry < miny)
				miny = bry;
			if (tly > maxy)
				maxy = tly;
		}

		// Set the position and the width and height based on these coords
		setW(maxx - minx);
		setH(maxy - miny);
		_WReal = getW();
		_HReal = getH();
		_XReal = minx;
		_YReal = miny;
	}

	void CInterfaceGroup::alignElements()
	{
		for(uint i = 0; i < _EltOrder.size(); ++i)
		{
			CViewBase *v = _EltOrder[i];
			v->alignTo(this);
		}
	}

}

