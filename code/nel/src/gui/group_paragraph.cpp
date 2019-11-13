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
#include "nel/gui/group_paragraph.h"
#include "nel/gui/group_html.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/interface_element.h"
#include "nel/gui/view_pointer_base.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/view_text_id.h"
#include "nel/gui/group_container.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/i18n.h"
#include "nel/misc/xml_auto_ptr.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

NLMISC_REGISTER_OBJECT(CViewBase, CCtrlLink, std::string, "button_link");

namespace NLGUI
{

	// ----------------------------------------------------------------------------
	CGroupParagraph::CGroupParagraph(const TCtorParam &param)
	:	CInterfaceGroup(param),
		_Templ(TCtorParam())
	{
		_IdCounter = 0;
		_AddElt = Bottom;
		_Align = Left;
		_Space = 0;
		_MinW= 0;
		_MinH= 0;
		_Over = false;
		_TempOver = false;
		_OverColor = CRGBA(255,255,255,32);
		_OverElt = -1;
		_LastW = 0;
		invalidateContent();
		_TopSpace = 0;
		_Indent = 0;
		_FirstViewIndentView = false;
		_TextId = 0;
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::addChild (CViewBase* child, bool deleteOnRemove)
	{
		if (!child)
		{
			nlwarning("<CGroupParagraph::addChild> : tried to add a NULL view");
			return;
		}

		// add child at last index
		addChildAtIndex(child, (uint)_Elements.size(), deleteOnRemove);
		if (_Elements.size() >= 2)
		{
			setOrder((uint)_Elements.size() - 1, getOrder((uint)_Elements.size() - 2) + 1);
		}
		invalidateContent();
	}

	// ----------------------------------------------------------------------------
	CGroupParagraph::~CGroupParagraph()
	{
		deleteAllChildren();
	}

	// ----------------------------------------------------------------------------
	// Set Hotspot of the first element in reference to the group
	/*void CGroupParagraph::setHSGroup (CViewBase *child, EAlign addElt, EAlign align)
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
	}*/

	// ----------------------------------------------------------------------------
	/** align an element towards its parent in the group
	  */
	/*void CGroupParagraph::setHSParent(CViewBase *view, EAlign addElt, EAlign align, uint space)
	{
		if ((addElt == Top) || (addElt == Bottom))
		{
			if (addElt == Bottom)
			{
				if (_Align == Left)
					view->_ParentPosRef = Hotspot_BL;
				else // align == Right
					view->_ParentPosRef = Hotspot_BR;
				view->_Y = -abs(space);
			}
			else if (addElt == Top)
			{
				if (_Align == Left)
					view->_ParentPosRef = Hotspot_TL;
				else // align == Right
					view->_ParentPosRef = Hotspot_TR;
				view->_Y = abs(space);
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
				view->_X = -abs(space);
			}
			else if (addElt == Right)
			{
				if (_Align == Top)
					view->_ParentPosRef = Hotspot_TR;
				else // align == Bottom
					view->_ParentPosRef = Hotspot_BR;
				view->_X = abs(space);
			}
		}
	}*/


	std::string CGroupParagraph::getProperty( const std::string &name ) const
	{
		if( name == "addelt" )
		{
			switch( _AddElt )
			{
			case Top:
				return "T";

			case Left:
				return "L";

			case Right:
				return "R";

			case Bottom:
				return "B";
			}

			nlassert(false);

			return "";
		}
		else
		if( name == "align" )
		{
			switch( _Align )
			{
			case Top:
				return "T";

			case Left:
				return "L";

			case Right:
				return "R";

			case Bottom:
				return "B";
			}

			nlassert(false);

			return "";
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

	void CGroupParagraph::setProperty( const std::string &name, const std::string &value )
	{
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
		if( name == "col_over" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_OverColor = c;
			return;
		}
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
			{
				_TextId = i;
				_HardText.clear();
			}
			onTextChanged();
			return;

		}
		else
			CInterfaceGroup::setProperty( name, value );
	}


	xmlNodePtr CGroupParagraph::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CInterfaceGroup::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

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
		xmlSetProp( node, BAD_CAST "col_over", BAD_CAST toString( _OverColor ).c_str() );
		xmlSetProp( node, BAD_CAST "hardtext", BAD_CAST _HardText.c_str() );
		xmlSetProp( node, BAD_CAST "textid", BAD_CAST toString( _TextId ).c_str() );

		return node;
	}

	// ----------------------------------------------------------------------------
	bool CGroupParagraph::parse (xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		if (!CInterfaceGroup::parse(cur,parentGroup))
			return false;

		// Parse location. If these properties are not specified, set them to 0

		CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"addelt" ));
		_AddElt = Bottom;
		if (ptr)
		{
			if (stricmp(ptr,"B") == 0)
				_AddElt = Bottom;
			else if (stricmp(ptr,"T") == 0)
				_AddElt = Top;
			else if (stricmp(ptr,"L") == 0)
				_AddElt = Left;
			else if (stricmp(ptr,"R") == 0)
				_AddElt = Right;
		}

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"align" );
		_Align = Left;
		if (ptr)
		{
			if (stricmp(ptr,"B") == 0)
				_Align = Bottom;
			else if (stricmp(ptr,"T") == 0)
				_Align = Top;
			else if (stricmp(ptr,"L") == 0)
				_Align = Left;
			else if (stricmp(ptr,"R") == 0)
				_Align = Right;
		}

		setupSizes();

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"space" );
		_Space = 0;
		if (ptr)
			fromString((const char*)ptr, _Space);

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"over" );
		_Over = false;
		if (ptr) _Over = convertBool(ptr);

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"col_over" );
		_OverColor = CRGBA(255,255,255,32);
		if (ptr) _OverColor = convertColor(ptr);


		// TEMPLATE TEXT SETUP

		// justification parameters
		_Templ.parseTextOptions (cur);

		// initial text
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"hardtext" );
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
			ptr = (char*) xmlGetProp( cur, (xmlChar*)"textid" );
			if (ptr)
			{
				fromString((const char*)ptr, _TextId );
				addTextChildID( _TextId );
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::addTextChild(const ucstring& line, bool multiLine /*= true*/)
	{
		const string elid = _Id + ":el" + toString(_IdCounter); ++_IdCounter;
		CViewText *view= new CViewText (elid, string(""), _Templ.getFontSize(), _Templ.getColor(), _Templ.getShadow());
		view->_Parent = this;
		view->setMultiLine (multiLine);
		view->setTextMode(_Templ.getTextMode());
		if (multiLine) view->setMultiLineSpace (_Space);
		view->setText (line);
		// Herit global-coloring
		view->setModulateGlobalColor(getModulateGlobalColor());
		addChild (view);
		invalidateCoords();
	}



	// ----------------------------------------------------------------------------
	void CGroupParagraph::addTextChild(const ucstring& line, const CRGBA& textColor, bool multiLine /*= true*/)
	{
		const string elid = _Id + ":el" + toString(_IdCounter); ++_IdCounter;
		CViewText *view= new CViewText (elid, string(""), _Templ.getFontSize(), _Templ.getColor(), _Templ.getShadow());
		view->_Parent = this;
		view->setMultiLine (multiLine);
		if (multiLine) view->setMultiLineSpace (_Space);
		view->setText (line);
		view->setColor (textColor);
		// Herit global-coloring
		view->setModulateGlobalColor(getModulateGlobalColor());
		addChild (view);
		invalidateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::addTextChildID (uint32 nID, bool multiLine)
	{
		const string elid = _Id + ":el" + toString(_IdCounter); ++_IdCounter;
		CViewTextID *view= new CViewTextID (elid, nID, _Templ.getFontSize(), _Templ.getColor(), _Templ.getShadow());
		view->_Parent = this;
		view->setMultiLine (multiLine);
		if (multiLine) view->setMultiLineSpace (_Space);
		// Herit global-coloring
		view->setModulateGlobalColor(getModulateGlobalColor());
		addChild (view);
		invalidateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::addTextChildID(const std::string &dbPath,bool multiLine /*=true*/)
	{
		const string elid = _Id + ":el" + toString(_IdCounter); ++_IdCounter;
		CViewTextID *view= new CViewTextID (elid, dbPath, _Templ.getFontSize(), _Templ.getColor(), _Templ.getShadow());
		view->_Parent = this;
		view->setMultiLine (multiLine);
		if (multiLine) view->setMultiLineSpace (_Space);
		// Herit global-coloring
		view->setModulateGlobalColor(getModulateGlobalColor());
		addChild (view);
		invalidateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::delChild (CViewBase* childToDel)
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
			nlwarning("Can't del child %s, it does not exist in the list", childToDel->getId().c_str());
			return;
		}
		delChild(posChildToDel);
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::delChild(uint posChildToDel)
	{
		if (posChildToDel >= (uint) _Elements.size())
		{
			nlwarning("<CGroupParagraph::delChild> bad index");
			return;
		}

		CViewBase* childToDel = _Elements[posChildToDel].Element;
		bool ElementMustBeDeleted = _Elements[posChildToDel].EltDeleteOnRemove;
		_Elements.erase (_Elements.begin()+posChildToDel);
		// Remove from drawing
		if (dynamic_cast<CInterfaceGroup *>(childToDel)) delGroup(static_cast<CInterfaceGroup *>(childToDel), !ElementMustBeDeleted);
		else if (dynamic_cast<CCtrlBase *>(childToDel)) delCtrl(static_cast<CCtrlBase *>(childToDel), !ElementMustBeDeleted);
		else delView(childToDel, !ElementMustBeDeleted);

		// Bind the new first element
		if (posChildToDel < _Elements.size())
		{
			CViewBase *pVB = _Elements[posChildToDel].Element;
			if (posChildToDel == 0)
			{
				pVB->_ParentPos = NULL;
				// setHSGroup (pVB, _AddElt, _Align);
				if ((_AddElt == Top) || (_AddElt == Bottom))
					pVB->setY (0);
				else
					pVB->setX (0);
			}
			else
				pVB->_ParentPos = _Elements[posChildToDel-1].Element;
		}
	}

	// ----------------------------------------------------------------------------
	/*void CGroupParagraph::removeHead ()
	{
		if (_Elements.empty())
		{
			nlwarning("<CGroupParagraph::removeHead> Can't remove head, list is empty");
			return;
		}
		delChild (_Elements.begin()->Element);*/
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
		setHSGroup (pVB, _AddElt, _Align);
		if ((_AddElt == Top) || (_AddElt == Bottom))
			pVB->setY (0);
		else
			pVB->setX (0);*/
	//}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::setTextTemplate(const CViewText& templ)
	{
		_Templ = templ;
	}


	// ----------------------------------------------------------------------------
	void CGroupParagraph::updateCoords()
	{
		if (_Parent)
		{
			if (!_ContentValidated)
			{
				// Update W
				CInterfaceElement::updateCoords();

				// Current X and Y
				sint32 x=_Indent;
				sint32 y=-(sint32)_TopSpace;

				// Current W
				sint width = std::min(getMaxWReal(), getWReal());

				// For each elements, place them
				uint firstElementOnLine = 0;
				const uint elmCount = (uint)_Elements.size();
				uint i;
				for (i = 0; i < elmCount+1; ++i)
				{
					// Force flush for the last element
					bool changeLine = (i == elmCount);
					uint lastLineElement = i+1;

					// Not the end of the element
					if (i < elmCount)
					{
						// Active element ?
						if (_Elements[i].Element->getActive())
						{
							_Elements[i].Element->updateCoords();
							// Is a view text ?
							CViewText *viewText = dynamic_cast<CViewText*>(_Elements[i].Element);
							if (viewText)
							{
								viewText->setFirstLineX(x + ((i==0)?_FirstViewIndentView:0));
								viewText->setX(0);
								viewText->updateTextContext();
							}
							else
							{
								_Elements[i].Element->setX(x + ((i==0)?_FirstViewIndentView:0));
							}
							_Elements[i].Element->setPosRef(Hotspot_TL);
							_Elements[i].Element->setParentPosRef(Hotspot_TL);

							// Update coords for this element
							_Elements[i].Element->updateCoords();

							// Does we balance the last line height ?
							if (viewText)
							{
								changeLine = viewText->getNumLine() > 1;
								if (!viewText->getText().empty() && *(viewText->getText().rbegin()) == (ucchar) '\n')
								{
									changeLine = true;
								}
							}
							else
							{
								// If this element is too big for the line, place it next time
								if ((i!=firstElementOnLine) && ((_Elements[i].Element->getX() + _Elements[i].Element->getW()) > width))
								{
									lastLineElement--;
									changeLine = true;
									i--;
								}
							}


							// New x coordinate
							x = _Elements[i].Element->getX() + _Elements[i].Element->getWReal();
						}
					}
					else
						lastLineElement = i;

					// Balance line height
					if (changeLine)
					{
						// Eval line height
						uint j;
						uint lineHeight = 0;
						uint multiLineHeight = 0;
						uint legHeight = 0;
						uint lastLineWidth = _Indent;

						for (j=firstElementOnLine; j<lastLineElement; j++)
						{
							// Is a view text ?
							uint newHeight;
							CViewText *viewTextOnLine = dynamic_cast<CViewText*>(_Elements[j].Element);

							// Element height
							if (viewTextOnLine)
							{
								// Height is just under the first line letter
								newHeight = viewTextOnLine->getFontHeight() - viewTextOnLine->getFontLegHeight();

								// Leg height for this view
								legHeight = std::max(legHeight, viewTextOnLine->getFontLegHeight());

								// Last element is a text multi line
								if (j==(lastLineElement-1))
								{
									const uint numLine = viewTextOnLine->getNumLine();
									if (numLine>1)
									{
										multiLineHeight = (numLine-2) * viewTextOnLine->getFontHeight() + (numLine-1) * viewTextOnLine->getMultiLineSpace();
										lastLineWidth = _Indent + viewTextOnLine->getLastLineW ();
									}
								}
							}
							else
							{
								newHeight = _Elements[j].Element->getH();
								CGroupHTMLInputOffset *inputOffset = dynamic_cast<CGroupHTMLInputOffset *>(_Elements[j].Element);
								if (inputOffset && inputOffset->Offset < 0)
								{
									newHeight += inputOffset->Offset;
									legHeight = max(-(inputOffset->Offset), sint32(legHeight));
								}
							}

							if (lineHeight < newHeight)
								lineHeight = newHeight;
						}

						// Repos element on the line
						for (j=firstElementOnLine; j<lastLineElement; j++)
						{
							// Is a view text ?
							uint newHeight;
							CViewText *viewTextOnLine = dynamic_cast<CViewText*>(_Elements[j].Element);

							sint32 offsetY=0;
							CGroupHTMLInputOffset *inputOffset = dynamic_cast<CGroupHTMLInputOffset *>(_Elements[j].Element);
							if (inputOffset)
								offsetY = inputOffset->Offset;
								// Element height
							if (viewTextOnLine)
								newHeight = viewTextOnLine->getFontHeight() - viewTextOnLine->getFontLegHeight() - offsetY;
							else
								newHeight = _Elements[j].Element->getH();

							sint32 posY = y-lineHeight+newHeight+offsetY;
							_Elements[j].Element->setY(posY);
						}

						if (i < elmCount)
						{
							nlassert(lastLineElement>0);
							nlassert(lastLineElement-1<elmCount);
							x = lastLineWidth;

							// Next line
							y -= lineHeight + multiLineHeight + legHeight;
						}

						// New first element
						firstElementOnLine = lastLineElement;
					}
				}

				// Update control view
				for (i = 0; i<_Links.size(); ++i)
				{
					// Link
					CLink &link = _Links[i];

					// Number of link needed
					uint links = std::min((uint)3, link.Link->getNumLine());

					// Number of line..
					uint j;
					for (j=0; j<3; j++)
					{
						if (j<links)
						{
							// Create the control ?
							CCtrlLink *ctrl = link.CtrlLink[j];
							if (ctrl == NULL)
							{
								// Control button
								ctrl = new CCtrlLink(CViewBase::TCtorParam());
								link.CtrlLink[j] = ctrl;
								ctrl->setId(getId()+":"+"links"+toString(i)+"-"+toString(j));
								ctrl->setParent (this);
								ctrl->setParentSize (this);
								ctrl->setParentPos (this);
								ctrl->setParentPosRef (Hotspot_TL);
								ctrl->setPosRef (Hotspot_TL);
								ctrl->setActive(true);
								ctrl->setActionOnLeftClick(link.Link->getActionOnLeftClick());
								ctrl->setParamsOnLeftClick(link.Link->getParamsOnLeftClick());
								ctrl->setScale(true);
								addCtrl(ctrl);
							}

							// Pos the links
							int x, y, width, height;
							int fontSize = link.Link->getFontHeight();
							int fontLineSpace = link.Link->getMultiLineSpace();

							// X
							x = (j == 0) ? link.Link->getFirstLineX() : link.Link->getX();

							// Y
							if (j < 2)
								y = link.Link->getY() - (fontSize+fontLineSpace) * j;
							else
								y = link.Link->getY() - ((fontSize+fontLineSpace) * (link.Link->getNumLine()-1));

							// Width

							// Last line ?
							if (j == (links-1))
								width = link.Link->getLastLineW ();
							else
							{
								width = link.Link->getW();

								// First line ?
								if (j == 0)
									width -= (link.Link->getFirstLineX() - link.Link->getX());
							}

							if ((j == 1) && (links==3))
								height = (fontSize+fontLineSpace)*(link.Link->getNumLine()-2);
							else
								height = fontSize;

							ctrl->setX(x);
							ctrl->setY(y);
							ctrl->setW(width);
							ctrl->setH(height);
						}
						else
						{
							if (link.CtrlLink[j])
							{
								delCtrl (link.CtrlLink[j]);
								link.CtrlLink[j] = NULL;
							}
						}
					}
				}
			}
		}
		CInterfaceGroup::updateCoords();

		// Validated
		_ContentValidated = true;
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::checkCoords ()
	{
		if (_Parent != NULL)
		{
			sint parentWidth = std::min(_Parent->getMaxWReal(), _Parent->getWReal());
			if (_LastW != (sint) parentWidth)
			{
				CCtrlBase *pCB = CWidgetManager::getInstance()->getCapturePointerLeft();
				if (pCB != NULL)
				{
					CCtrlResizer *pCR = dynamic_cast<CCtrlResizer*>(pCB);
					if (pCR != NULL)
					{
						// We are resizing !!!!
					}
					else
					{
						_LastW = parentWidth;
						 invalidateContent();
					}
				}
				else
				{
					_LastW = parentWidth;
					invalidateContent();
				}
			}
		}
		CInterfaceGroup::checkCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::draw ()
	{
		// TEMP TEMP
		//CViewRenderer &rVR = *CViewRenderer::getInstance();
		//rVR.drawRotFlipBitmap _RenderLayer, (_XReal, _YReal, _WReal, _HReal, 0, false, rVR.getBlankTextureId(), CRGBA(0,255,0,255) );
		if (_Over || _TempOver)
		{
			CViewRenderer &rVR = *CViewRenderer::getInstance();

			if (CWidgetManager::getInstance()->getModalWindow() == NULL)
			{
				sint32 x = CWidgetManager::getInstance()->getPointer()->getX();
				sint32 y = CWidgetManager::getInstance()->getPointer()->getY();

				CInterfaceGroup	*pIG = CWidgetManager::getInstance()->getWindowUnder(x,y);
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

				sint32 clipx,clipy,clipw,cliph;
				getClip(clipx,clipy,clipw,cliph);
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
				CViewBase *pVB = _Elements[_OverElt].Element;
				CRGBA col = _OverColor;
				if(getModulateGlobalColor())
				{
					col.modulateFromColor (_OverColor, CWidgetManager::getInstance()->getGlobalColorForContent());
				}
				else
				{
					col= _OverColor;
					col.A = (uint8)(((sint32)col.A*((sint32)CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);
				}
				rVR.drawRotFlipBitmap (_RenderLayer, pVB->getXReal(), pVB->getYReal(),
										pVB->getWReal(), pVB->getHReal(), 0, false, rVR.getBlankTextureId(),
										col );

			}
		}

		CInterfaceGroup::draw ();
	}

	// ----------------------------------------------------------------------------
	bool CGroupParagraph::handleEvent (const NLGUI::CEventDescriptor& event)
	{
		if (!_Active)
			return false;

		bool bReturn = CInterfaceGroup::handleEvent(event);

		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;

			_OverElt = -1;
			if (!isIn(eventDesc.getX(), eventDesc.getY()))
			{
				_TempOver = false;
				return false;
			}

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
		bool operator()(const CGroupParagraph::CElementInfo &info) const { return dynamic_cast<CViewBase *>(info.Element) != NULL; }
	};

	// predicate to remove a ctrl from the list of element
	struct CRemoveCtrlPred
	{
		bool operator()(const CGroupParagraph::CElementInfo &info)  const { return dynamic_cast<CCtrlBase *>(info.Element) != NULL; }
	};

	// predicate to remove a group from the list of element
	struct CRemoveGroupPred
	{
		bool operator()(const CGroupParagraph::CElementInfo &info)  const { return dynamic_cast<CInterfaceGroup *>(info.Element) != NULL; }
	};


	// ----------------------------------------------------------------------------
	void CGroupParagraph::clearViews()
	{
		_IdCounter = 0;
		// remove views from the list of elements
		_Elements.erase(std::remove_if(_Elements.begin(), _Elements.end(), CRemoveViewPred()), _Elements.end());
		CInterfaceGroup::clearViews();
		updateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::clearControls()
	{
		_IdCounter = 0;
		// remove views from the list of elements
		_Elements.erase(std::remove_if(_Elements.begin(), _Elements.end(), CRemoveCtrlPred()), _Elements.end());
		CInterfaceGroup::clearControls();
		updateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::clearGroups()
	{
		_IdCounter = 0;
		// remove views from the list of elements
		_Elements.erase(std::remove_if(_Elements.begin(), _Elements.end(), CRemoveGroupPred()), _Elements.end());
		CInterfaceGroup::clearGroups();
		updateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::forceSizeW (sint32 newSizeW)
	{
		_W = newSizeW;
		for (uint32 i = 0; i < _Elements.size(); ++i)
		{
			_Elements[i].Element->setW (_W);
			_Elements[i].Element->CInterfaceElement::updateCoords();
		}
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::forceSizeH (sint32 newSizeH)
	{
		_H = newSizeH;
		for (uint32 i = 0; i < _Elements.size(); ++i)
		{
			_Elements[i].Element->setH (_H);
			_Elements[i].Element->CInterfaceElement::updateCoords();
		}
	}

	// ----------------------------------------------------------------------------
	void	CGroupParagraph::setMinW(sint32 minW)
	{
		_MinW= minW;
		invalidateCoords();
	}

	// ----------------------------------------------------------------------------
	void	CGroupParagraph::setMinH(sint32 minH)
	{
		_MinH= minH;
		invalidateCoords();
	}

	// ----------------------------------------------------------------------------
	bool CGroupParagraph::addChildAtIndex(CViewBase *child, uint index, bool deleteOnRemove /*=true*/)
	{
		if (!child)
		{
			nlwarning("<CGroupParagraph::addChild> : tried to add a NULL view");
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
				nlwarning("<CGroupParagraph::addChild> bad align");
				child->_SizeRef = 0;
			break;
		}

		child->_SizeDivW = 10;
		child->_SizeDivH = 10;

		// Position the element according to the list alignement
		// setHSGroup (child, _AddElt, _Align);

		// update coords of the element (it may use child_resize_h or w)
		//child->updateCoords();
		child->invalidateCoords();

		// Update size
		if ((_AddElt == Top) || (_AddElt == Bottom))
		{
			// update the list size
			sint32 newH = _H + child->getH();
			if (!_Elements.empty())
				newH += _Space;
			_H = newH;

			if ((_SizeRef&1) == 0) // No parent size reference in W
			{
				sint32 newW = max (_W, child->getW());
				_W = newW;
			}
		}
		else
		{
			// Update the list coords
			sint32 newW = _W + child->getW();
			if (!_Elements.empty())
				newW += _Space;
			_W = newW;

			if ((_SizeRef&2) == 0) // No parent size reference in H
			{
				sint32 newH = max (_H, child->getH());
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
			// setHSParent(child, _AddElt, _Align, _Space);
			// child->_ParentPos = _Elements[index - 1].Element;
		}
		_Elements.insert(_Elements.begin() + index, ei);
		// link next element to this one
		if (index < _Elements.size() - 1)
		{
			// _Elements[index + 1].Element->_ParentPos = child;
			// setHSParent(_Elements[index + 1].Element, _AddElt, _Align, _Space);
		}

		// Add this element for drawing
		{
			CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(child);
			if (pIG != NULL)
			{
				addGroup (pIG, (sint) index);
				return true;
			}
			CCtrlBase *pCB = dynamic_cast<CCtrlBase*>(child);
			if (pCB != NULL)
			{
				addCtrl (pCB, (sint) index);
				return true;
			}
			CViewBase *pVB = dynamic_cast<CViewBase*>(child);
			if (pVB != NULL)
			{
				addView (pVB, (sint) index);
				return true;
			}
			nlstop;
			return false;
		}
		return false;
	}

	// ----------------------------------------------------------------------------
	sint32 CGroupParagraph::getElementIndex(CViewBase* child) const
	{
		for(uint k = 0; k < _Elements.size(); ++k)
		{
			if (_Elements[k].Element == child) return k;
		}
		return -1;
	}

	// ----------------------------------------------------------------------------
	/*void CGroupParagraph::swapChildren(uint index1, uint index2)
	{
		if (index1 >= _Elements.size()
			|| index2 >= _Elements.size())
		{
			nlwarning("<CGroupParagraph::swapChildren> bad indexes");
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
			addChildAtIndex(v2, index1, oldMustDelete2);
			setOrder(index1, order2);
			addChildAtIndex(v1, index2, oldMustDelete1);
			setOrder(index2, order1);
		}
		else
		{
			delChild(index1);
			delChild(index2);
			addChildAtIndex(v1, index2, oldMustDelete1);
			setOrder(index2, order1);
			addChildAtIndex(v2, index1, oldMustDelete2);
			setOrder(index1, order2);
		}
	}*/

	// ----------------------------------------------------------------------------
	void CGroupParagraph::deleteAllChildren()
	{
		uint numChildren = getNbElement();
		for(uint k = 0; k < numChildren; ++k)
		{
			delChild(numChildren - 1 - k); // delete in reverse order to avoid unnecessary vector copies
		}
	}

	// ----------------------------------------------------------------------------
	uint CGroupParagraph::getNumActiveChildren() const
	{
		uint numChildren = 0;
		for(uint k = 0; k < _Elements.size(); ++k)
		{
			if (_Elements[k].Element->getActive()) ++numChildren;
		}
		return numChildren;
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::setDelOnRemove(uint index, bool delOnRemove)
	{
		if (index >= _Elements.size())
		{
			nlwarning("bad index");
			return;
		}
		_Elements[index].EltDeleteOnRemove = delOnRemove;
	}

	// ----------------------------------------------------------------------------
	bool CGroupParagraph::getDelOnRemove(uint index) const
	{
		if (index >= _Elements.size())
		{
			nlwarning("bad index");
			return false;
		}
		return _Elements[index].EltDeleteOnRemove;
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::addChildLink (CViewLink* child, bool deleteOnRemove)
	{
		// Add the child
		addChild (child, deleteOnRemove);

		// Add the link
		_Links.push_back (CLink(child));
	}

	// ----------------------------------------------------------------------------
	CGroupParagraph::CLink::CLink (CViewLink *link)
	{
		Link = link;
		CtrlLink[0] = NULL;
		CtrlLink[1] = NULL;
		CtrlLink[2] = NULL;
	}

	// ----------------------------------------------------------------------------
	void CGroupParagraph::onInvalidateContent()
	{
		_ContentValidated = false;
		invalidateCoords();
	}

	// ----------------------------------------------------------------------------
	sint32 CGroupParagraph::getMaxUsedW() const
	{
		// The paragraph max is the sum of the components max
		sint maxWidth = 0;
		for (uint k = 0; k < _Elements.size(); ++k)
		{
			// Get the child width
			maxWidth += _Elements[k].Element->getMaxUsedW();
		}
		return maxWidth + _MarginLeft;
	}

	// ----------------------------------------------------------------------------
	sint32 CGroupParagraph::getMinUsedW() const
	{
		// The paragraph min is the max of the components min
		sint32 minWidth = 0;
		for (uint k = 0; k < _Elements.size(); ++k)
		{
			// Get the child width
			sint32 width = _Elements[k].Element->getMinUsedW();
			if (width > minWidth)
				minWidth = width;
		}
		return minWidth + _MarginLeft;
	}


	void CGroupParagraph::setupSizes()
	{
		EAlign addElt = _AddElt;
		_GroupSizeRef = _SizeRef;
		if ((addElt == Top) || (addElt == Bottom))
		{
			setMaxW (_W);
			setMaxH(_H);
			_H = 0;
			_SizeRef = _SizeRef&(~2);
		}
		else
		{
			setMaxW (_W);
			setMaxH (_H);
			_W = 0;
			_SizeRef = _SizeRef&(~1);
		}
	}


	void CGroupParagraph::onTextChanged()
	{
		if( _Elements.empty() )
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

