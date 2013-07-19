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
#include "nel/gui/group_list.h"

#include "nel/gui/interface_element.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/view_text_id.h"
#include "nel/gui/group_container_base.h"
#include "nel/gui/lua_ihm.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/view_pointer_base.h"
#include "nel/misc/i18n.h"

using namespace std;
using namespace NLMISC;

NLMISC_REGISTER_OBJECT(CViewBase, CGroupList, std::string, "list");

namespace NLGUI
{

	// ----------------------------------------------------------------------------
	CGroupList::CGroupList(const TCtorParam &param)
	:	CInterfaceGroup(param),
		_Templ(TCtorParam())
	{
		_IdCounter = 0;
		_MaxElements = 1024;
		_AddElt = Bottom;
		_Align = Left;
		_Space = 0;
		_DynamicDisplaySize = false;
		_MinW= 0;
		_MinH= 0;
		_Over = false;
		_OverColor = CRGBA(255,  255,  255,  32);
		_OverElt = -1;
		_IsGroupList = true;
		_TextId = 0;
	}

	// ----------------------------------------------------------------------------
	void CGroupList::addChild (CViewBase* child,   bool deleteOnRemove)
	{
		if (!child)
		{
			nlwarning("<CGroupList::addChild> : tried to add a NULL view");
			return;
		}

		// Make sure there's room for the element
		if ((sint32)_Elements.size() == _MaxElements)
		{
			removeHead();
		}

		// add child at last index
		addChildAtIndex(child,   (uint)_Elements.size(),   deleteOnRemove);
		if (_Elements.size() >= 2)
		{
			setOrder((uint)_Elements.size() - 1,   getOrder((uint)_Elements.size() - 2) + 1);
		}
	}

	// ----------------------------------------------------------------------------
	CGroupList::~CGroupList()
	{
		deleteAllChildren();
	}

	// ----------------------------------------------------------------------------
	// Set Hotspot of the first element in reference to the group
	void CGroupList::setHSGroup (CViewBase *child,   EAlign addElt,   EAlign align)
	{
		switch (addElt)
		{
			case Bottom:
				if (align == Left)
				{
					child->_ParentPosRef = Hotspot_TL;
					child->_PosRef = Hotspot_TL;
				}
				else // align == Right
				{
					child->_ParentPosRef = Hotspot_TR;
					child->_PosRef = Hotspot_TR;
				}
			break;
			case Left:
				if (align == Top)
				{
					child->_ParentPosRef = Hotspot_TR;
					child->_PosRef = Hotspot_TR;
				}
				else // align == Bottom
				{
					child->_ParentPosRef = Hotspot_BR;
					child->_PosRef = Hotspot_BR;
				}
			break;
			case Top:
				if (align == Left)
				{
					child->_ParentPosRef = Hotspot_BL;
					child->_PosRef = Hotspot_BL;
				}
				else // align == Right
				{
					child->_ParentPosRef = Hotspot_BR;
					child->_PosRef = Hotspot_BR;
				}
			break;
			case Right:
				if (align == Top)
				{
					child->_ParentPosRef = Hotspot_TL;
					child->_PosRef = Hotspot_TL;
				}
				else // align == Bottom
				{
					child->_ParentPosRef = Hotspot_BL;
					child->_PosRef = Hotspot_BL;
				}
			break;
			default:
				nlassert(false);
			break;
		}
	}

	// ----------------------------------------------------------------------------
	/** align an element towards its parent in the group
	  */
	void CGroupList::setHSParent(CViewBase *view,   EAlign addElt,   EAlign /* align */,   uint space)
	{
		if ((addElt == Top) || (addElt == Bottom))
		{
			if (addElt == Bottom)
			{
				if (_Align == Left)
					view->_ParentPosRef = Hotspot_BL;
				else // align == Right
					view->_ParentPosRef = Hotspot_BR;
				//view->_Y = -abs((sint32)space);
				view->_Y = - (sint32)space;
			}
			else if (addElt == Top)
			{
				if (_Align == Left)
					view->_ParentPosRef = Hotspot_TL;
				else // align == Right
					view->_ParentPosRef = Hotspot_TR;
				// view->_Y = abs((sint32)space);
				view->_Y = (sint32)space;
			}
		}
		else
		{
			if (addElt == Left)
			{
				if (_Align == Top)
					view->_ParentPosRef = Hotspot_TL;
				else // align == Bottom
					view->_ParentPosRef = Hotspot_BL;
				//view->_X = -abs((sint32)space);
				view->_X = -(sint32)space;
			}
			else if (addElt == Right)
			{
				if (_Align == Top)
					view->_ParentPosRef = Hotspot_TR;
				else // align == Bottom
					view->_ParentPosRef = Hotspot_BR;
				//view->_X = abs((sint32)space);
				view->_X = (sint32)space;
			}
		}
	}

	std::string CGroupList::getProperty( const std::string &name ) const
	{
		if( name == "maxelements" )
		{
			return toString( _MaxElements );
		}
		else
		if( name == "addelt" )
		{
			switch( _AddElt )
			{
			case Top:
				return "T";
				break;

			case Left:
				return "L";
				break;

			case Right:
				return "R";
				break;
			}

			return "B";
		}
		else
		if( name == "align" )
		{
			switch( _Align )
			{
			case Top:
				return "T";
				break;

			case Left:
				return "L";
				break;

			case Right:
				return "R";
				break;
			}

			return "B";
		}
		else
		if( name == "space" )
		{
			return toString( _Space );
		}
		else
		if( name == "over" )
		{
			return toString( _Over );
		}
		else
		if( name == "dynamic_display_size" )
		{
			return toString( _DynamicDisplaySize );
		}
		else
		if( name == "col_over" )
		{
			return toString( _OverColor );
		}
		else
		if( name == "hardtext" )
		{
			return _HardText;
		}
		else
		if( name == "textid" )
		{
			return toString( _TextId );
		}
		else
			return CInterfaceGroup::getProperty( name );
	}

	void CGroupList::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "maxelements" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_MaxElements = i;
			return;
		}
		else
		if( name == "addelt" )
		{
			if( value == "T" )
				_AddElt = Top;
			else
			if( value == "L" )
				_AddElt = Left;
			else
			if( value == "R" )
				_AddElt = Right;
			else
			if( value == "B" )
				_AddElt = Bottom;

			setupSizes();
			return;
		}
		else
		if( name == "align" )
		{
			if( value == "T" )
				_Align = Top;
			else
			if( value == "L" )
				_Align = Left;
			else
			if( value == "R" )
				_Align = Right;
			else
			if( value == "B" )
				_Align = Bottom;

			return;
		}
		else
		if( name == "space" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_Space = i;
			return;
		}
		else
		if( name == "over" )
		{
			bool b;
			if( fromString( value, b ) )
				_Over = b;
			return;
		}
		else
		if( name == "dynamic_display_size" )
		{
			bool b;
			if( fromString( value, b ) )
				_DynamicDisplaySize = b;
			return;
		}
		else
		if( name == "col_over" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_OverColor = c;
			return;
		}
		else
		if( name == "hardtext" )
		{
			_HardText = value;
			_TextId = 0;
			onTextChanged();
			return;
		}
		else
		if( name == "textid" )
		{
			uint32 i;
			if( fromString( value, i ) )
				_TextId = i;
			_HardText = "";
			onTextChanged();
			return;
		}
		else
			CInterfaceGroup::setProperty( name, value );
	}


	xmlNodePtr CGroupList::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CInterfaceGroup::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "list" );
		xmlSetProp( node, BAD_CAST "maxelements", BAD_CAST toString( _MaxElements ).c_str() );

		std::string addelt;
		std::string align;

		switch( _AddElt )
		{
		case Top:
			addelt = "T";
			break;

		case Left:
			addelt = "L";
			break;

		case Right:
			addelt = "R";
			break;

		default:
			addelt = "B";
			break;
		}

		switch( _Align )
		{
		case Top:
			align = "T";
			break;

		case Left:
			align = "L";
			break;

		case Right:
			align = "R";
			break;

		default:
			align = "B";
			break;
		}

		xmlSetProp( node, BAD_CAST "addelt", BAD_CAST addelt.c_str() );
		xmlSetProp( node, BAD_CAST "align", BAD_CAST align.c_str() );
		xmlSetProp( node, BAD_CAST "space", BAD_CAST toString( _Space ).c_str() );
		xmlSetProp( node, BAD_CAST "over", BAD_CAST toString( _Over ).c_str() );
		xmlSetProp( node, BAD_CAST "dynamic_display_size", BAD_CAST toString( _DynamicDisplaySize ).c_str() );
		xmlSetProp( node, BAD_CAST "col_over", BAD_CAST toString( _OverColor ).c_str() );
		xmlSetProp( node, BAD_CAST "hardtext", BAD_CAST _HardText.c_str() );
		xmlSetProp( node, BAD_CAST "textid", BAD_CAST toString( _TextId ).c_str() );

		return node;
	}

	// ----------------------------------------------------------------------------
	bool CGroupList::parse (xmlNodePtr cur,   CInterfaceGroup * parentGroup)
	{
		if (!CInterfaceGroup::parse(cur,  parentGroup))
			return false;

		// Parse location. If these properties are not specified,   set them to 0

		CXMLAutoPtr ptr((const char*) xmlGetProp( cur,   (xmlChar*)"maxelements" ));
		_MaxElements = 1024;
		if (ptr)
		{
			if (!fromString((const char*)ptr, _MaxElements))
			{
				nlwarning("<CGroupList::parse> Can't parse the 'maxelements' field ");
			}
		}

		ptr = (char*) xmlGetProp( cur,   (xmlChar*)"addelt" );
		_AddElt = Bottom;
		if (ptr)
		{
			if (stricmp(ptr,  "B") == 0)
				_AddElt = Bottom;
			else if (stricmp(ptr,  "T") == 0)
				_AddElt = Top;
			else if (stricmp(ptr,  "L") == 0)
				_AddElt = Left;
			else if (stricmp(ptr,  "R") == 0)
				_AddElt = Right;
		}

		ptr = (char*) xmlGetProp( cur,   (xmlChar*)"align" );
		_Align = Left;
		if (ptr)
		{
			if (stricmp(ptr,  "B") == 0)
				_Align = Bottom;
			else if (stricmp(ptr,  "T") == 0)
				_Align = Top;
			else if (stricmp(ptr,  "L") == 0)
				_Align = Left;
			else if (stricmp(ptr,  "R") == 0)
				_Align = Right;
		}

		ptr = (char*) xmlGetProp( cur,   (xmlChar*)"space" );
		_Space = 0;
		if (ptr)
			fromString((const char*)ptr, _Space);

		setupSizes();

		ptr = (char*) xmlGetProp( cur,   (xmlChar*)"over" );
		_Over = false;
		if (ptr) _Over = convertBool(ptr);

		ptr = (char*) xmlGetProp( cur,   (xmlChar*)"dynamic_display_size" );
		_DynamicDisplaySize = false;
		if (ptr) _DynamicDisplaySize = convertBool(ptr);

		ptr = (char*) xmlGetProp( cur,   (xmlChar*)"col_over" );
		_OverColor = CRGBA(255,  255,  255,  32);
		if (ptr) _OverColor = convertColor(ptr);


		// TEMPLATE TEXT SETUP

		// justification parameters
		_Templ.parseTextOptions (cur);

		// initial text
		ptr = (char*) xmlGetProp( cur,   (xmlChar*)"hardtext" );
		if (ptr)
		{
			_HardText = std::string( (const char*)ptr );
			const char *propPtr = ptr;
			ucstring Text = ucstring(propPtr);
			if ((strlen(propPtr)>2) && (propPtr[0] == 'u') && (propPtr[1] == 'i'))
				Text = CI18N::get (propPtr);

			addTextChild(Text);
		}
		else
		{
			ptr = (char*) xmlGetProp( cur,   (xmlChar*)"textid" );
			if (ptr)
			{
				fromString((const char*)ptr, _TextId );
				addTextChildID( _TextId );
			}
		}

		return true;
	}



	// ----------------------------------------------------------------------------
	void CGroupList::addTextChild(const ucstring& line,   bool multiLine /*= true*/)
	{
		const string elid = _Id + ":el" + toString(_IdCounter); ++_IdCounter;
		CViewText *view= new CViewText (elid,   string(""),   _Templ.getFontSize(),   _Templ.getColor(),   _Templ.getShadow());
		view->setSerializable( false );
		view->_Parent = this;
		view->setMultiLine (multiLine);
		view->setTextMode(_Templ.getTextMode());
		if (multiLine) view->setMultiLineSpace (_Space);
		view->setText (line);
		// Herit global-coloring
		view->setModulateGlobalColor(getModulateGlobalColor());
		addChild(view);
		invalidateCoords();
	}



	// ----------------------------------------------------------------------------
	void CGroupList::addTextChild(const ucstring& line,   const CRGBA& textColor,   bool multiLine /*= true*/)
	{
		const string elid = _Id + ":el" + toString(_IdCounter); ++_IdCounter;
		CViewText *view= new CViewText (elid,   string(""),   _Templ.getFontSize(),   _Templ.getColor(),   _Templ.getShadow());
		view->setSerializable( false );
		view->_Parent = this;
		view->setMultiLine (multiLine);
		if (multiLine) view->setMultiLineSpace (_Space);
		view->setText (line);
		view->setColor (textColor);
		// Herit global-coloring
		view->setModulateGlobalColor(getModulateGlobalColor());
		addChild(view);
		invalidateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupList::addTextChildID (uint32 nID,   bool multiLine)
	{
		const string elid = _Id + ":el" + toString(_IdCounter); ++_IdCounter;
		CViewTextID *view= new CViewTextID (elid,   nID,   _Templ.getFontSize(),   _Templ.getColor(),   _Templ.getShadow());
		view->setSerializable( false );
		view->_Parent = this;
		view->setMultiLine (multiLine);
		if (multiLine) view->setMultiLineSpace (_Space);
		// Herit global-coloring
		view->setModulateGlobalColor(getModulateGlobalColor());
		addChild (view);
		invalidateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupList::addTextChildID(const std::string &dbPath,  bool multiLine /*=true*/)
	{
		const string elid = _Id + ":el" + toString(_IdCounter); ++_IdCounter;
		CViewTextID *view= new CViewTextID (elid,   dbPath,   _Templ.getFontSize(),   _Templ.getColor(),   _Templ.getShadow());
		view->setSerializable( false );
		view->_Parent = this;
		view->setMultiLine (multiLine);
		if (multiLine) view->setMultiLineSpace (_Space);
		// Herit global-coloring
		view->setModulateGlobalColor(getModulateGlobalColor());
		addChild (view);
		invalidateCoords();
	}

	// ----------------------------------------------------------------------------
	bool CGroupList::delChild (CViewBase* childToDel,   bool noWarning, bool forceDontDelete)
	{
		// Look for child
		uint posChildToDel = 0;
		for (posChildToDel = 0; posChildToDel < _Elements.size(); ++posChildToDel)
		{
			CElementInfo rEI = _Elements[posChildToDel];
			if (rEI.Element == childToDel)
				break;
		}

		if (posChildToDel == _Elements.size())
		{
			if (!noWarning)
				nlwarning("Can't del child %s,   it does not exist in the list",   childToDel->getId().c_str());
			return false;
		}
		return delChild(posChildToDel, forceDontDelete);
	}

	// ----------------------------------------------------------------------------
	bool CGroupList::delChild(uint posChildToDel, bool forceDontDelete)
	{
		if (posChildToDel >= (uint) _Elements.size())
		{
			nlwarning("<CGroupList::delChild> bad index");
			return false;
		}

		CViewBase* childToDel = _Elements[posChildToDel].Element;

		childToDel->_Parent = NULL;

		bool elementMustBeDeleted = _Elements[posChildToDel].EltDeleteOnRemove && !forceDontDelete;
		_Elements.erase (_Elements.begin()+posChildToDel);
		// Remove from drawing
		if (dynamic_cast<CInterfaceGroup *>(childToDel)) delGroup(static_cast<CInterfaceGroup *>(childToDel),   !elementMustBeDeleted);
		else if (dynamic_cast<CCtrlBase *>(childToDel)) delCtrl(static_cast<CCtrlBase *>(childToDel),   !elementMustBeDeleted);
		else delView(childToDel,   !elementMustBeDeleted);

		// Bind the new first element
		if (posChildToDel < _Elements.size())
		{
			CViewBase *pVB = _Elements[posChildToDel].Element;
			if (posChildToDel == 0)
			{
				pVB->_ParentPos = NULL;
				setHSGroup (pVB,   _AddElt,   _Align);
				if ((_AddElt == Top) || (_AddElt == Bottom))
					pVB->setY (0);
				else
					pVB->setX (0);
			}
			else
				pVB->_ParentPos = _Elements[posChildToDel-1].Element;
		}
		return true;
	}

	// ----------------------------------------------------------------------------
	void CGroupList::removeHead ()
	{
		if (_Elements.empty())
		{
			nlwarning("<CGroupList::removeHead> Can't remove head,   list is empty");
			return;
		}
		delChild (_Elements.begin()->Element);
		/*CViewBase *pVB = _Elements.begin()->Element;
		if ((_AddElt == Top) || (_AddElt == Bottom))
		{
			sint32 shift = _H - (pVB->getH() + _Space);
			_H = shift;
		}
		else
		{
			sint32 shift = _W - (pVB->getW() + _Space);
			_W = shift;
		}

		bool FirstElementMustBeDeleted = _Elements.begin()->EltDeleteOnRemove;
		if (FirstElementMustBeDeleted)
			delete pVB;
		_Elements.erase (_Elements.begin());
		// Remove from drawing
		for (vector<CInterfaceGroup*>::iterator itg = _ChildrenGroups.begin(); itg != _ChildrenGroups.end(); itg++)
			if(*itg == pVB)
			{
				_ChildrenGroups.erase (itg);
				break;
			}
		for (vector<CCtrlBase*>::iterator itc = _Controls.begin(); itc != _Controls.end(); itc++)
			if(*itc == pVB)
			{
				_Controls.erase (itc);
				break;
			}
		for (vector<CViewBase*>::iterator itv = _Views.begin(); itv != _Views.end(); itv++)
			if(*itv == pVB)
			{
				_Views.erase (itv);
				break;
			}
		delEltOrder (pVB);

		// Bind the new first element
		pVB = _Elements.begin()->Element;
		pVB->_ParentPos = NULL;
		setHSGroup (pVB,   _AddElt,   _Align);
		if ((_AddElt == Top) || (_AddElt == Bottom))
			pVB->setY (0);
		else
			pVB->setX (0);*/
	}

	// ----------------------------------------------------------------------------
	void CGroupList::setTextTemplate(const CViewText& templ)
	{
		_Templ = templ;
	}


	// ----------------------------------------------------------------------------
	void CGroupList::updateCoords()
	{
		if (!_Active) return;
		// Handle if elements are not active
		for (sint32 i = 0; i < ((sint32)_Elements.size()-1); ++i)
		{
			if (_Elements[i].Element->getActive())
				setHSParent(_Elements[i+1].Element,   _AddElt,   _Align,   _Space);
			else
				setHSParent(_Elements[i+1].Element,   _AddElt,   _Align,   0);
		}

		CInterfaceGroup::updateCoords();

		sint32 nCurrentX = 0; // Current offset of an element

		EAlign addElt = _AddElt;
		if ((addElt == Top) || (addElt == Bottom))
		{
			// Calculate size
			sint32 newH = 0,   newW = 0;
			bool bFirst = true;

			for (uint32 i = 0; i < _Elements.size(); ++i)
			if (_Elements[i].Element->getActive())
			{
				newH += _Elements[i].Element->getH();
				if (!bFirst)
					newH += _Space;
				bFirst = false;
				nCurrentX += _Elements[i].Element->getX();
				newW = max (newW,   _Elements[i].Element->getW()+(sint32)abs(nCurrentX));
			}
			_W = max(newW,   _MinW);
			_H = max(newH,   _MinH);
			if (_DynamicDisplaySize)
			{
				_MaxW = _W;
				_MaxH = _H;
			}
			if (_H < _MaxH) setOfsY(0);
		}
		else
		{
			sint32 newW = 0,   newH = 0;
			bool bFirst = true;

			for (uint32 i = 0; i < _Elements.size(); ++i)
			if (_Elements[i].Element->getActive())
			{
				newW += _Elements[i].Element->getW();
				if (!bFirst)
					newW += _Space;
				bFirst = false;
				newH = max (newH,   _Elements[i].Element->getH());
			}
			_W = max(newW,   _MinW);
			_H = max(newH,   _MinH);
			if (_DynamicDisplaySize)
			{
				_MaxW = _W;
				_MaxH = _H;
			}
			if (_W < _MaxW) setOfsX(0);
		}

		CInterfaceElement::updateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupList::draw ()
	{
		// TEMP TEMP
		//CViewRenderer &rVR = *CViewRenderer::getInstance();
		//rVR.drawRotFlipBitmap _RenderLayer,   (_XReal,   _YReal,   _WReal,   _HReal,   0,   false,   rVR.getBlankTextureId(),   CRGBA(0,  255,  0,  255) );
		if (_Over)
		{
			CViewRenderer &rVR = *CViewRenderer::getInstance();

			if (CWidgetManager::getInstance()->getModalWindow() == NULL)
			{
				sint32 x = CWidgetManager::getInstance()->getPointer()->getX();
				sint32 y = CWidgetManager::getInstance()->getPointer()->getY();

				CInterfaceGroup	*pIG = CWidgetManager::getInstance()->getWindowUnder(x,  y);
				CInterfaceGroup	*pParent = this;
				bool bFound = false;
				while (pParent != NULL)
				{
					if (pParent == pIG)
					{
						bFound = true;
						break;
					}
					pParent = pParent->getParent();
				}

				sint32 clipx,  clipy,  clipw,  cliph;
				getClip(clipx,  clipy,  clipw,  cliph);
				if ((x < clipx) ||
					(x > (clipx + clipw)) ||
					(y < clipy) ||
					(y > (clipy + cliph)) || !bFound)
				{
					_OverElt = -1;
				}
				else
				{
					for (uint32 i = 0; i < _Elements.size(); ++i)
					if (_Elements[i].Element->getActive())
					{
						CViewBase *pVB = _Elements[i].Element;
						if ((x >= pVB->getXReal()) &&
							(x < (pVB->getXReal() + pVB->getWReal()))&&
							(y >= pVB->getYReal()) &&
							(y < (pVB->getYReal() + pVB->getHReal())))
						{
							_OverElt = i;
						}
					}
				}
			}

			if (_OverElt != -1)
			{
				// Find the first container
				CInterfaceGroup *pIG = _Parent;
				CGroupContainerBase *pGC = dynamic_cast<CGroupContainerBase*>(pIG);
				while (pIG != NULL)
				{
					pIG = pIG->_Parent;
					if (pIG == NULL) break;
					if (dynamic_cast<CGroupContainerBase*>(pIG) != NULL)
						pGC = dynamic_cast<CGroupContainerBase*>(pIG);
				}

				bool bDisplayOverSelection = true;
				if (pGC != NULL)
				{
					if (pGC->isGrayed())
						bDisplayOverSelection = false;
				}

				if (bDisplayOverSelection)
				{
					CViewBase *pVB = _Elements[_OverElt].Element;
					CRGBA col = _OverColor;
					if(getModulateGlobalColor())
					{
						col.modulateFromColor (_OverColor,   CWidgetManager::getInstance()->getGlobalColorForContent());
					}
					else
					{
						col= _OverColor;
						col.A = (uint8)(((sint32)col.A*((sint32)CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);
					}
					rVR.drawRotFlipBitmap (_RenderLayer,   pVB->getXReal(),   pVB->getYReal(),
											pVB->getWReal(),   pVB->getHReal(),   0,   false,   rVR.getBlankTextureId(),
											col );
				}

			}
		}

		CInterfaceGroup::draw ();
	}

	// ----------------------------------------------------------------------------
	bool CGroupList::handleEvent (const NLGUI::CEventDescriptor& event)
	{
		if (!_Active)
			return false;

		bool bReturn = CInterfaceGroup::handleEvent(event);

		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;

			_OverElt = -1;
			if (!isIn(eventDesc.getX(),   eventDesc.getY()))
				return false;

			for (uint32 i = 0; i < _Elements.size(); ++i)
			if (_Elements[i].Element->getActive())
			{
				CViewBase *pVB = _Elements[i].Element;
				if ((eventDesc.getX() >= pVB->getXReal()) &&
					(eventDesc.getX() < (pVB->getXReal() + pVB->getWReal()))&&
					(eventDesc.getY() >= pVB->getYReal()) &&
					(eventDesc.getY() < (pVB->getYReal() + pVB->getHReal())))
				{
					_OverElt = i;
				}
			}
		}

		return bReturn;
	}



	// predicate to remove a view from the list of element
	struct CRemoveViewPred
	{
		bool operator()(const CGroupList::CElementInfo &info) const { return dynamic_cast<CViewBase *>(info.Element) != NULL; }
	};

	// predicate to remove a ctrl from the list of element
	struct CRemoveCtrlPred
	{
		bool operator()(const CGroupList::CElementInfo &info)  const { return dynamic_cast<CCtrlBase *>(info.Element) != NULL; }
	};

	// predicate to remove a group from the list of element
	struct CRemoveGroupPred
	{
		bool operator()(const CGroupList::CElementInfo &info)  const { return dynamic_cast<CInterfaceGroup *>(info.Element) != NULL; }
	};


	// ----------------------------------------------------------------------------
	void CGroupList::clearViews()
	{
		_IdCounter = 0;
		// remove views from the list of elements
		_Elements.erase(std::remove_if(_Elements.begin(),   _Elements.end(),   CRemoveViewPred()),   _Elements.end());
		CInterfaceGroup::clearViews();
		updateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupList::clearControls()
	{
		_IdCounter = 0;
		// remove views from the list of elements
		_Elements.erase(std::remove_if(_Elements.begin(),   _Elements.end(),   CRemoveCtrlPred()),   _Elements.end());
		CInterfaceGroup::clearControls();
		updateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupList::clearGroups()
	{
		_IdCounter = 0;
		// remove views from the list of elements
		_Elements.erase(std::remove_if(_Elements.begin(),   _Elements.end(),   CRemoveGroupPred()),   _Elements.end());
		CInterfaceGroup::clearGroups();
		updateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupList::forceSizeW (sint32 newSizeW)
	{
		_W = newSizeW;
		for (uint32 i = 0; i < _Elements.size(); ++i)
		{
			_Elements[i].Element->setW (_W);
			_Elements[i].Element->CInterfaceElement::updateCoords();
		}
	}

	// ----------------------------------------------------------------------------
	void CGroupList::forceSizeH (sint32 newSizeH)
	{
		_H = newSizeH;
		for (uint32 i = 0; i < _Elements.size(); ++i)
		{
			_Elements[i].Element->setH (_H);
			_Elements[i].Element->CInterfaceElement::updateCoords();
		}
	}

	// ----------------------------------------------------------------------------
	void	CGroupList::setMinW(sint32 minW)
	{
		_MinW= minW;
		invalidateCoords();
	}

	// ----------------------------------------------------------------------------
	void	CGroupList::setMinH(sint32 minH)
	{
		_MinH= minH;
		invalidateCoords();
	}

	// ----------------------------------------------------------------------------
	bool CGroupList::addChildAtIndex(CViewBase *child,   uint index,   bool deleteOnRemove /*=true*/)
	{
		if (!child)
		{
			nlwarning("<CGroupList::addChild> : tried to add a NULL view");
			return false;
		}
		if (index > _Elements.size())
		{
			return false;
		}
		child->_Parent = this;
		child->_ParentPos = NULL;
		child->_Active = true;
		child->_X = 0;
		child->_Y = 0;
		child->_RenderLayer = this->_RenderLayer;
		// Can't have sizeref on the coordinate corresponding to alignement
		switch(_AddElt)
		{
			case Top:
			case Bottom:
				child->_SizeRef &= 1; // sizeref on w is permitted
			break;
			case Left:
			case Right:
				child->_SizeRef &= 2; // sizeref on h is permitted
			break;
			default:
				nlwarning("<CGroupList::addChild> bad align");
				child->_SizeRef = 0;
			break;
		}

		child->_SizeDivW = 10;
		child->_SizeDivH = 10;

		// Position the element according to the list alignement
		setHSGroup (child,   _AddElt,   _Align);

		// update coords of the element (it may use child_resize_h or w)
		//child->updateCoords();
		child->invalidateCoords();

		// Update size
		if ((_AddElt == Top) || (_AddElt == Bottom))
		{
			// update the list size
			sint32 newH = _H + child->getH();
			if (_Elements.size() > 0)
				newH += _Space;
			_H = newH;

			if ((_SizeRef&1) == 0) // No parent size reference in W
			{
				sint32 newW = max (_W,   child->getW());
				_W = newW;
			}
		}
		else
		{
			// Update the list coords
			sint32 newW = _W + child->getW();
			if (_Elements.size() > 0)
				newW += _Space;
			_W = newW;

			if ((_SizeRef&2) == 0) // No parent size reference in H
			{
				sint32 newH = max (_H,   child->getH());
				_H = newH;
			}
		}

		CElementInfo ei;
		ei.Element = child;
		ei.EltDeleteOnRemove = deleteOnRemove;
		ei.Order = 0;

		if (index != 0)
		{
			// update alignement
			setHSParent(child,   _AddElt,   _Align,   _Space);
			child->_ParentPos = _Elements[index - 1].Element;
		}
		_Elements.insert(_Elements.begin() + index,   ei);
		// link next element to this one
		if (index < _Elements.size() - 1)
		{
			_Elements[index + 1].Element->_ParentPos = child;
			setHSParent(_Elements[index + 1].Element,   _AddElt,   _Align,   _Space);
		}

		// Add this element for drawing
		{
			child->setSerializable( false );
			CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(child);
			if (pIG != NULL)
			{
				addGroup (pIG,   (sint) index);
				return true;
			}
			CCtrlBase *pCB = dynamic_cast<CCtrlBase*>(child);
			if (pCB != NULL)
			{
				addCtrl (pCB,   (sint) index);
				return true;
			}
			CViewBase *pVB = dynamic_cast<CViewBase*>(child);
			if (pVB != NULL)
			{
				addView (pVB,   (sint) index);
				return true;
			}
			nlstop;
			return false;
		}
		return false;
	}

	// ----------------------------------------------------------------------------
	sint32 CGroupList::getElementIndex(CViewBase* child) const
	{
		for(uint k = 0; k < _Elements.size(); ++k)
		{
			if (_Elements[k].Element == child) return k;
		}
		return -1;
	}

	// ----------------------------------------------------------------------------
	int CGroupList::luaGetElementIndex(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "getElementIndex", 1);
		CViewBase * viewBase = dynamic_cast<CViewBase *>(CLuaIHM::getUIOnStack(ls, 1));
		ls.push((double) getElementIndex(viewBase));
		return 1;
	}

	// ----------------------------------------------------------------------------
	void CGroupList::swapChildren(uint index1,   uint index2)
	{
		if (index1 >= _Elements.size()
			|| index2 >= _Elements.size())
		{
			nlwarning("<CGroupList::swapChildren> bad indexes");
			return;
		}
		// prevent elements from being deleted
		bool oldMustDelete1 = _Elements[index1].EltDeleteOnRemove;
		bool oldMustDelete2 = _Elements[index2].EltDeleteOnRemove;

		uint order1 = _Elements[index1].Order;
		uint order2 = _Elements[index2].Order;

		_Elements[index1].EltDeleteOnRemove = false;
		_Elements[index2].EltDeleteOnRemove = false;

		CViewBase *v1 = _Elements[index1].Element;
		CViewBase *v2 = _Elements[index2].Element;


		if (index1 < index2)
		{
			delChild(index2);
			delChild(index1);
			addChildAtIndex(v2,   index1,   oldMustDelete2);
			setOrder(index1,   order2);
			addChildAtIndex(v1,   index2,   oldMustDelete1);
			setOrder(index2,   order1);
		}
		else
		{
			delChild(index1);
			delChild(index2);
			addChildAtIndex(v1,   index2,   oldMustDelete1);
			setOrder(index2,   order1);
			addChildAtIndex(v2,   index1,   oldMustDelete2);
			setOrder(index1,   order2);
		}
	}

	// ----------------------------------------------------------------------------
	int	CGroupList::luaUpChild(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "upChild", 1);
		CViewBase * viewBase = dynamic_cast<CViewBase *>(CLuaIHM::getUIOnStack(ls, 1));
		sint32 indexUpChild = getElementIndex(viewBase);
		if(indexUpChild > 0)
		{
			swapChildren(indexUpChild, indexUpChild-1);
		}
		return 0;
	}

	// ----------------------------------------------------------------------------
	int	CGroupList::luaDownChild(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "downChild", 1);
		CViewBase * viewBase = dynamic_cast<CViewBase *>(CLuaIHM::getUIOnStack(ls, 1));
		sint32 indexDownChild = getElementIndex(viewBase);
		if(indexDownChild < (sint32) (_Elements.size()-1))
		{
			swapChildren(indexDownChild, indexDownChild+1);
		}
		return 0;
	}

	// ----------------------------------------------------------------------------
	int	CGroupList::luaGetChild(CLuaState &ls)
	{
		const char *funcName = "getChild";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		sint index = (sint) ls.toNumber(1);
		if(index < 0 || index >= (sint) _Elements.size())
		{
			CLuaIHM::fails(ls, "getChild : trying to access element %d in list '%s', which has %d elements",
						   index, getId().c_str(), (int) _Elements.size());
		}
		CLuaIHM::pushUIOnStack(ls, getChild((uint) index));
		return 1;
	}

	// ----------------------------------------------------------------------------
	void CGroupList::deleteAllChildren()
	{
		uint numChildren = getNbElement();
		for(uint k = 0; k < numChildren; ++k)
		{
			delChild(numChildren - 1 - k); // delete in reverse order to avoid unnecessary vector copies
		}
	}

	// ----------------------------------------------------------------------------
	uint CGroupList::getNumActiveChildren() const
	{
		uint numChildren = 0;
		for(uint k = 0; k < _Elements.size(); ++k)
		{
			if (_Elements[k].Element->getActive()) ++numChildren;
		}
		return numChildren;
	}

	// ----------------------------------------------------------------------------
	void CGroupList::setDelOnRemove(uint index,   bool delOnRemove)
	{
		if (index >= _Elements.size())
		{
			nlwarning("bad index");
			return;
		}
		_Elements[index].EltDeleteOnRemove = delOnRemove;
	}

	// ----------------------------------------------------------------------------
	bool CGroupList::getDelOnRemove(uint index) const
	{
		if (index >= _Elements.size())
		{
			nlwarning("bad index");
			return false;
		}
		return _Elements[index].EltDeleteOnRemove;
	}


	// ----------------------------------------------------------------------------
	int CGroupList::luaAddTextChild(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "addTextChild", 1);
		ucstring text;
		if(CLuaIHM::pop(ls, text))
		{
			addTextChild(text);
		}
		return 0;
	}

	// ----------------------------------------------------------------------------
	int CGroupList::luaAddColoredTextChild(CLuaState &ls)
	{
		const char *funcName = "addColoredTextChild";
		CLuaIHM::checkArgCount(ls, funcName, 5);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);
		CLuaIHM::checkArgType(ls, funcName, 3, LUA_TNUMBER);
		CLuaIHM::checkArgType(ls, funcName, 4, LUA_TNUMBER);
		CLuaIHM::checkArgType(ls, funcName, 5, LUA_TNUMBER);
		string text = ls.toString(1);
		ucstring ucText;
		ucText.fromUtf8(text);

		uint r = (uint) ls.toNumber(2);
		uint g = (uint) ls.toNumber(3);
		uint b = (uint) ls.toNumber(4);
		uint a = (uint) ls.toNumber(5);

		addTextChild(ucText, CRGBA(r, g, b, a));

		return 0;
	}

	// ----------------------------------------------------------------------------
	int CGroupList::luaAddChild(CLuaState &ls)
	{
		const char *funcName = "addChild";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CViewBase *vb = dynamic_cast<CViewBase *>(CLuaIHM::getUIOnStack(ls, 1));
		if (!vb)
		{
			CLuaIHM::fails(ls, "%s requires a view, group or control", funcName);
		}
		else
		{
			addChild(vb);
		}
		return 0;
	}

	// ----------------------------------------------------------------------------
	int CGroupList::luaAddChildAtIndex(CLuaState &ls)
	{
		const char *funcName = "addChildAtIndex";
		CLuaIHM::checkArgCount(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);
		CViewBase *vb = dynamic_cast<CViewBase *>(CLuaIHM::getUIOnStack(ls, 1));
		if (!vb)
		{
			CLuaIHM::fails(ls, "%s requires a view, group or control", funcName);
		}
		else
		{
			addChildAtIndex(vb, (uint) ls.toNumber(2));
		}
		return 0;
	}

	// ----------------------------------------------------------------------------
	int CGroupList::luaDetachChild(CLuaState &ls)
	{
		const char *funcName = "detachChild";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CViewBase *vb = dynamic_cast<CViewBase *>(CLuaIHM::getUIOnStack(ls, 1));
		if (!vb)
		{
			nlwarning("%s requires a view, group or control", funcName);
			ls.push(false);
		}
		else
		{
			ls.push(delChild(vb, false, true));
		}
		return 1;
	}

	// ----------------------------------------------------------------------------
	int CGroupList::luaDelChild(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "CGroupList::delChild", 1);
		CViewBase *vb = dynamic_cast<CViewBase *>(CLuaIHM::getUIOnStack(ls, 1));
		if (vb) delChild(vb);
		updateCoords();
		return 0;
	}



	// ----------------------------------------------------------------------------
	int CGroupList::luaClear(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "clear", 0);
		deleteAllChildren();
		return 0;
	}

	void CGroupList::setupSizes()
	{
		_GroupSizeRef = _SizeRef;
		if ((_AddElt == Top) || (_AddElt == Bottom))
		{
			setMaxW (_W);
			setMaxH(_H);
			_H = 0;
			_SizeRef = _SizeRef & (~2);
		}
		else
		{
			setMaxW (_W);
			setMaxH (_H);
			_W = 0;
			_SizeRef = _SizeRef & (~1);
		}
	}

	void CGroupList::onTextChanged()
	{
		if( _Elements.size() == 0 )
			return;

		CElementInfo &e = _Elements[ 0 ];
		
		CViewText *t = dynamic_cast< CViewText* >( e.Element );
		if( t != NULL )
		{
			t->setText( _HardText );
			return;
		}
		else
		{
			CViewTextID *ti = dynamic_cast< CViewTextID* >( e.Element );
			if( ti != NULL )
			{
				ti->setTextId( _TextId );
			}
		}
	}
}

