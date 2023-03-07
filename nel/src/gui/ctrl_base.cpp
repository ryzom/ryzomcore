// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/lua_ihm.h"
#include "nel/gui/ctrl_base.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/i18n.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	std::map< std::string, std::map< std::string, std::string > > CCtrlBase::AHCache;

	// ***************************************************************************
	CCtrlBase::~CCtrlBase()
	{
		CWidgetManager::getInstance()->removeRefOnCtrl (this);
	}

	// ***************************************************************************
	bool CCtrlBase::handleEvent(const NLGUI::CEventDescriptor &event)
	{
		if( CViewBase::handleEvent( event ) )
			return true;

		if (event.getType() == NLGUI::CEventDescriptor::system)
		{
			NLGUI::CEventDescriptorSystem &eds = (NLGUI::CEventDescriptorSystem&)event;
			if (eds.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::activecalledonparent)
			{
				if (!((NLGUI::CEventDescriptorActiveCalledOnParent &) eds).getActive())
				{
					// the mouse capture should be lost when the ctrl is hidden
					if (CWidgetManager::getInstance()->getCapturePointerLeft() == this)
					{
						CWidgetManager::getInstance()->setCapturePointerLeft(NULL);
					}
					if (CWidgetManager::getInstance()->getCapturePointerRight() == this)
					{
						CWidgetManager::getInstance()->setCapturePointerRight(NULL);
					}
					// NB : don't call return here because derived class may be interested
					// in handling event more speciffically
				}
			}
		}
		return false;
	}

	std::string CCtrlBase::tooltipParentToString( TToolTipParentType type )
	{
		switch( type )
		{
		case TTMouse:
			return "mouse";
			break;

		case TTWindow:
			return "win";
			break;

		case TTSpecialWindow:
			return "special";
			break;

		default:
			break;
		}

		return "control";
	}

	CCtrlBase::TToolTipParentType CCtrlBase::stringToToolTipParent( const std::string &str )
	{
		std::string s = toLowerAscii( str );

		if( s == "mouse" )
			return TTMouse;
		else
		if( s == "win" )
			return TTWindow;
		else
		if( s == "special" )
			return TTSpecialWindow;
		else
			return TTCtrl;
	}

	// ***************************************************************************
	bool CCtrlBase::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
	{
		if(!CViewBase::parse(cur, parentGroup))
			return false;

		CXMLAutoPtr prop;

		// get static toolTip
		prop = (char *)xmlGetProp(cur, (xmlChar *)"tooltip_i18n");
		if ((bool)prop && strlen((const char *)prop) > 0)
		{
			// Force I18N tooltip
			if (!editorMode)
				_ContextHelp = CI18N::get((const char *)prop);
			else
				_ContextHelp = (const char *)prop;
		}
		else
		{
			// get static toolTip
			prop = (char *)xmlGetProp(cur, (xmlChar *)"tooltip");
			if (prop)
			{
				if (!editorMode && NLMISC::startsWith((const char *)prop, "ui"))
					_ContextHelp = CI18N::get((const char *)prop);
				else
					_ContextHelp = (const char *)prop;
			}
		}

		// get dynamic toolTip ActionHandler
		prop = (char*) xmlGetProp( cur, (xmlChar*)"on_tooltip" );
		if (prop)
		{
			_OnContextHelp= (const char*)prop;
		}
		prop = (char*) xmlGetProp( cur, (xmlChar*)"on_tooltip_params" );
		if (prop)
		{
			_OnContextHelpParams= (const char*)prop;
		}

		// Tooltip parent
		prop = (char*) xmlGetProp( cur, (xmlChar*)"tooltip_parent" );
		_ToolTipParent= TTCtrl;
		if(prop)
		{
			_ToolTipParent = stringToToolTipParent( std::string( (const char*)prop ) );
		}

		// Tooltip special parent
		prop = (char*) xmlGetProp( cur, (xmlChar*)"tooltip_special_parent" );
		_ToolTipSpecialParent= CStringShared();
		if(prop)
		{
			_ToolTipSpecialParent= (const char*)prop;
		}

		// Tooltip posref
		THotSpot tmpParentHS, tmpChildHS;
		prop = (char*) xmlGetProp( cur, (xmlChar*)"tooltip_posref" );
		convertTooltipHotSpot(prop, tmpParentHS, tmpChildHS);
		_ToolTipParentPosRef= tmpParentHS;
		_ToolTipPosRef= tmpChildHS;

		// Alternative tooltip posref : this one will be chosen	
		prop = (char*) xmlGetProp( cur, (xmlChar*)"tooltip_posref_alt" );
		convertTooltipHotSpot(prop, tmpParentHS, tmpChildHS);
		_ToolTipParentPosRefAlt = tmpParentHS;
		_ToolTipPosRefAlt = tmpChildHS;

		// ToolTip instant
		prop = (char*) xmlGetProp( cur, (xmlChar*)"instant_help");
		_ToolTipInstant= true;
		if (prop) _ToolTipInstant = convertBool(prop);

		return true;
	}

	std::string CCtrlBase::getProperty( const std::string &name ) const
	{
		if( name == "tooltip" )
		{
			return _ContextHelp;
		}
		else
		if( name == "tooltip_i18n" )
		{
			return _ContextHelp;
		}
		else
		if( name == "on_tooltip" )
		{
			return _OnContextHelp.toString();
		}
		else
		if( name == "on_tooltip_params" )
		{
			return _OnContextHelpParams.toString();
		}
		else
		if( name == "tooltip_parent" )
		{
			return tooltipParentToString( _ToolTipParent );
		}
		else
		if( name == "tooltip_special_parent" )
		{
			return _ToolTipSpecialParent.toString();
		}
		else
		if( name == "tooltip_posref" )
		{
			return TooltipHotSpotToString( _ToolTipPosRef );
		}
		else
		if( name == "tooltip_parent_posref" )
		{
			return TooltipHotSpotToString( _ToolTipParentPosRef );
		}
		else
		if( name == "tooltip_posref_alt" )
		{
			return TooltipHotSpotToString( _ToolTipPosRefAlt );
		}
		else
		if( name == "tooltip_parent_posref_alt" )
		{
			return TooltipHotSpotToString( _ToolTipParentPosRefAlt );
		}
		else
		if( name == "instant_help" )
		{
			return toString( _ToolTipInstant );
		}
		else
			return CInterfaceElement::getProperty( name );
	}


	void CCtrlBase::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "tooltip" )
		{
			if (!editorMode && NLMISC::startsWith(value, "ui"))
				_ContextHelp = CI18N::get(value);
			else
				_ContextHelp = value;
			return;
		}
		else
		if( name == "tooltip_i18n" )
		{
			if (!editorMode)
				_ContextHelp = CI18N::get(value);
			else
				_ContextHelp = value;
			return;
		}
		else
		if( name == "on_tooltip" )
		{
			_OnContextHelp = value;
			return;
		}
		else
		if( name == "on_tooltip_params" )
		{
			_OnContextHelpParams = value;
			return;
		}
		else
		if( name == "tooltip_parent" )
		{
			_ToolTipParent = stringToToolTipParent( value );
			return;
		}
		else
		if( name == "tooltip_special_parent" )
		{
			_ToolTipSpecialParent = value;
			return;
		}
		else
		if( name == "tooltip_posref" )
		{
			THotSpot HS;
			convertTooltipHotSpot( value.c_str(), HS );
			_ToolTipPosRef = HS;

			// When auto is set, both of them need to be auto
			if( _ToolTipPosRef == Hotspot_TTAuto )
				_ToolTipParentPosRef = Hotspot_TTAuto;
			else
			if( _ToolTipParentPosRef == Hotspot_TTAuto )
				_ToolTipParentPosRef = _ToolTipPosRef;

			return;
		}
		else
		if( name == "tooltip_parent_posref" )
		{
			THotSpot HS;
			convertTooltipHotSpot( value.c_str(), HS );
			_ToolTipParentPosRef = HS;

			// When auto is set, both of them need to be auto
			if( _ToolTipParentPosRef == Hotspot_TTAuto )
				_ToolTipPosRef = Hotspot_TTAuto;
			else
			if( _ToolTipPosRef == Hotspot_TTAuto )
				_ToolTipPosRef = _ToolTipParentPosRef;

			return;
		}
		else
		if( name == "tooltip_posref_alt" )
		{
			THotSpot HS;
			convertTooltipHotSpot( value.c_str(), HS );
			_ToolTipPosRefAlt = HS;

			// When auto is set, both of them need to be auto
			if( _ToolTipPosRefAlt == Hotspot_TTAuto )
				_ToolTipParentPosRefAlt = Hotspot_TTAuto;
			else
			if( _ToolTipParentPosRefAlt == Hotspot_TTAuto )
				_ToolTipPosRefAlt = _ToolTipParentPosRefAlt;

			return;
		}
		else
		if( name == "tooltip_parent_posref_alt" )
		{
			THotSpot HS;
			convertTooltipHotSpot( value.c_str(), HS );
			_ToolTipParentPosRefAlt = HS;

			// When auto is set, both of them need to be auto
			if( _ToolTipParentPosRefAlt == Hotspot_TTAuto )
				_ToolTipPosRefAlt = Hotspot_TTAuto;
			else
			if( _ToolTipPosRefAlt == Hotspot_TTAuto )
				_ToolTipPosRefAlt = _ToolTipParentPosRefAlt;

			return;
		}
		else
		if( name == "instant_help" )
		{
			bool b;
			if( fromString( value, b ) )
				_ToolTipInstant = b;
			return;
		}
		else
			CInterfaceElement::setProperty( name, value );
	}


	xmlNodePtr CCtrlBase::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node =
			CInterfaceElement::serialize( parentNode, type );

		if( node == NULL )
			return NULL;

		xmlNewProp( node, BAD_CAST "tooltip", BAD_CAST _ContextHelp.c_str() );
		xmlNewProp( node, BAD_CAST "tooltip_i18n", BAD_CAST _ContextHelp.c_str() );
		xmlNewProp( node, BAD_CAST "on_tooltip", BAD_CAST _OnContextHelp.toString().c_str() );
		xmlNewProp( node, BAD_CAST "on_tooltip_params", BAD_CAST _OnContextHelpParams.toString().c_str() );
		xmlNewProp( node, BAD_CAST "tooltip_parent", BAD_CAST tooltipParentToString( _ToolTipParent ).c_str() );
		xmlNewProp( node, BAD_CAST "tooltip_special_parent", BAD_CAST _ToolTipSpecialParent.toString().c_str() );
		
		xmlNewProp( node, BAD_CAST "tooltip_posref",
			BAD_CAST TooltipHotSpotToString( _ToolTipParentPosRef, _ToolTipPosRef ).c_str() );
		
		xmlNewProp( node, BAD_CAST "tooltip_posref_alt",
			BAD_CAST TooltipHotSpotToString( _ToolTipParentPosRefAlt, _ToolTipPosRefAlt ).c_str() );

		xmlNewProp( node, BAD_CAST "instant_help", BAD_CAST toString( _ToolTipInstant ).c_str() );

		return node;
	}

	// ***************************************************************************
	void CCtrlBase::convertTooltipHotSpot(const char *prop, THotSpot &parentHS, THotSpot &childHS)
	{
		parentHS = Hotspot_TTAuto;
		childHS = Hotspot_TTAuto;
		if(prop)
		{
			const	char	*ptr= (const char*)prop;
			if(stricmp(ptr, "auto")==0)
			{
				parentHS = Hotspot_TTAuto;
				childHS = Hotspot_TTAuto;
			}
			// valid ref?
			else if(strlen(ptr)>=5)
			{
				THotSpot	parentPosRef;
				THotSpot	posRef;
				CInterfaceElement::convertHotSpotCouple(ptr, parentPosRef, posRef);
				parentHS = parentPosRef;
				childHS = posRef;
			}
		}
	}

	void CCtrlBase::convertTooltipHotSpot(const char *prop, THotSpot &HS )
	{
		if(prop)
		{
			const char *ptr = (const char*)prop;
			if(stricmp(ptr, "auto")==0)
			{
				HS = Hotspot_TTAuto;
			}
			else if(strlen(ptr)==2)
			{
				HS = convertHotSpot(ptr);
			}
		}
	}

	std::string CCtrlBase::TooltipHotSpotToString( THotSpot parent, THotSpot child )
	{
		std::string s;

		if( ( parent == Hotspot_TTAuto ) && ( child == Hotspot_TTAuto ) )
		{
			s = "auto";
		}
		else
		{
			s = CInterfaceElement::HotSpotToString( parent );
			s += " ";
			s += CInterfaceElement::HotSpotToString( child );
		}

		return s;
	}

	std::string CCtrlBase::TooltipHotSpotToString( THotSpot HS )
	{
		std::string s;
		if( HS == Hotspot_TTAuto )
		{
			s = "auto";
		}
		else
		{
			s = HotSpotToString( HS );
		}
		return s;
	}

	// ***************************************************************************
	bool CCtrlBase::emptyContextHelp() const
	{
		std::string help;
		getContextHelp(help);
		std::string sTmp = _OnContextHelp;
		return help.empty() && sTmp.empty();
	}

	// ***************************************************************************
	void CCtrlBase::visit(CInterfaceElementVisitor *visitor)
	{
		nlassert(visitor);
		visitor->visitCtrl(this);
		CInterfaceElement::visit(visitor);
	}

	// ***************************************************************************
	void CCtrlBase::serial(NLMISC::IStream &f)
	{
		CViewBase::serial(f);

		uint version = f.serialVersion(1);
		nlassert(version);

		f.serial(_ContextHelp);
		f.serial(_OnContextHelp);
		f.serial(_OnContextHelpParams);
		f.serial(_ToolTipSpecialParent);
		f.serialEnum(_ToolTipParent);

		THotSpot tmpToolTipParentPosRef = _ToolTipParentPosRef;
		THotSpot tmpToolTipPosRef = _ToolTipPosRef;
		THotSpot tmpToolTipParentPosRefAlt = _ToolTipParentPosRefAlt;
		THotSpot tmpToolTipPosRefAlt = _ToolTipPosRefAlt;

		f.serialEnum(tmpToolTipParentPosRef);
		f.serialEnum(tmpToolTipPosRef);
		f.serialEnum(tmpToolTipParentPosRefAlt);
		f.serialEnum(tmpToolTipPosRefAlt);

		_ToolTipParentPosRef = tmpToolTipParentPosRef;
		_ToolTipPosRef = tmpToolTipPosRef;
		_ToolTipParentPosRefAlt = tmpToolTipParentPosRefAlt;
		_ToolTipPosRefAlt = tmpToolTipPosRefAlt;	
		
		nlSerialBitBool(f, _ToolTipInstant);	
	}

	// ***************************************************************************
	std::string CCtrlBase::getContextHelpWindowName() const
	{
		return "context_help";
	}

	uint32 CCtrlBase::getDepth( CInterfaceGroup *group )
	{
		uint32 depth = 1;
		CInterfaceGroup *parent = getParent();

		while( parent != NULL )
		{
			if ( parent == group )
				break;
			else
				parent = parent->getParent();
			depth++;
		}
		// The Resizer Ctrls take the precedence over Sons controls.
		return depth + getDeltaDepth();
	}


	void CCtrlBase::mapAHString( const std::string &key, const std::string &value )
	{
		std::map< std::string, std::map< std::string, std::string > >::iterator itr = AHCache.find( getId() );
		if( itr == AHCache.end() )
		{
			AHCache[ getId() ];
			itr = AHCache.find( getId() );
		}

		std::map< std::string, std::string > &AHMap = itr->second;
		AHMap[ key ] = value;
	}

	std::string CCtrlBase::getAHString( const std::string &key ) const
	{
		std::map< std::string, std::map< std::string, std::string > >::const_iterator itr = AHCache.find( getId() );
		if( itr == AHCache.end() )
			return "";

		std::map< std::string, std::string >::const_iterator itr2 = itr->second.find( key );
		if( itr2 == itr->second.end() )
			return "";
		else
			return itr2->second;
	}

#ifdef RYZOM_LUA_UCSTRING
	// ***************************************************************************
	int CCtrlBase::luaSetTooltipUtf8(CLuaState &ls)
	{
		const char *funcName = "setTooltipUtf8";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		std::string tooltip = ls.toString(1);

		setDefaultContextHelp(tooltip);

		return 0;
	}
#endif
}

