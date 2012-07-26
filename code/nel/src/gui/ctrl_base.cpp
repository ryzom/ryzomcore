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

#include "libxml/globals.h"
#include "nel/misc/debug.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/ctrl_base.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/i18n.h"

using namespace NLMISC;

namespace NLGUI
{

	// ***************************************************************************
	CCtrlBase::~CCtrlBase()
	{
		CWidgetManager::getInstance()->removeRefOnCtrl (this);
	}

	// ***************************************************************************
	bool CCtrlBase::handleEvent(const NLGUI::CEventDescriptor &event)
	{
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

	// ***************************************************************************
	bool CCtrlBase::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
	{
		if(!CViewBase::parse(cur, parentGroup))
			return false;

		CXMLAutoPtr prop;

		// get static toolTip
		prop = (char*) xmlGetProp( cur, (xmlChar*)"tooltip" );
		if (prop)
		{
			const char *propPtr = prop;

			_ContextHelp = ucstring(propPtr);

			if (strlen(propPtr) > 2)
			{
				if ((propPtr[0] == 'u') && (propPtr[1] == 'i'))
					_ContextHelp = CI18N::get ((const char *) prop);
			}
		}
		// Force I18N tooltip
		prop = (char*) xmlGetProp( cur, (xmlChar*)"tooltip_i18n" );
		if ((bool)prop && strlen((const char*)prop)>0)
		{
			_ContextHelp = CI18N::get ((const char *) prop);
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
			if(stricmp((const char*)prop, "win")==0)
				_ToolTipParent= TTWindow;
			else if(stricmp((const char*)prop, "mouse")==0)
				_ToolTipParent= TTMouse;
			else if(stricmp((const char*)prop, "special")==0)
				_ToolTipParent= TTSpecialWindow;
			else
				_ToolTipParent= TTCtrl;
		}

		// Tooltip special parent
		prop = (char*) xmlGetProp( cur, (xmlChar*)"tooltip_special_parent" );
		_ToolTipSpecialParent= CStringShared();
		if(prop)
		{
			_ToolTipSpecialParent= std::string((const char*)prop);
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
		if( name == "on_tooltip" )
		{
			return _OnContextHelp.toString();
		}
		else
		if( name == "on_tooltip_params" )
		{
			return _OnContextHelpParams.toString();
		}

		return CInterfaceElement::getProperty( name );
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


	// ***************************************************************************
	bool CCtrlBase::emptyContextHelp() const
	{
		ucstring help;
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
		f.serial(_ContextHelp);
		f.serial(_OnContextHelp);
		f.serial(_OnContextHelpParams);
		f.serial(_ToolTipSpecialParent);
		f.serialEnum(_ToolTipParent);
		//

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
		//
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

}

