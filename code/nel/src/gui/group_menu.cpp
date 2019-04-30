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
#include "nel/gui/interface_options.h"
#include "nel/gui/interface_expr.h"
#include "nel/gui/group_menu.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/action_handler.h" // Just for getAllParams
#include "nel/gui/lua_ihm.h"
#include "nel/misc/i18n.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/group_list.h"
#include "nel/gui/ctrl_scroll.h"
#include "nel/gui/view_pointer_base.h"


using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace
{
	const std::string ID_MENU_CHECKBOX  = "menu_cb";
	const std::string ID_MENU_SEPARATOR = "menu_separator";
	const std::string ID_MENU_SUBMENU = "menu_sb";
	const uint MENU_WIDGET_X = 2;
	const uint LEFT_MENU_WIDGET_X = 4;
}

namespace NLGUI
{

	// ------------------------------------------------------------------------------------------------
	// CGroupSubMenu
	// ------------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------------
	bool CViewTextMenu::getGrayed() const
	{
		return _Grayed;
	}

	// ------------------------------------------------------------------------------------------------
	void CViewTextMenu::setGrayed (bool g)
	{
		_Grayed = g;
	}

	// ------------------------------------------------------------------------------------------------
	void CViewTextMenu::setCheckable(bool c)
	{
		if (!c)
		{
			_Checkable = false;
			_CheckBox = NULL;
		}
		else
		{
			_Checkable = true;
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CViewTextMenu::setChecked(bool c)
	{
		if (_CheckBox)
		{
			CInterfaceOptions *pIO = CWidgetManager::getInstance()->getOptions("menu_checkbox");
			if (!pIO) return;
			_CheckBox->setTexture(pIO->getValStr(c ? "checked_bitmap" : "unchecked_bitmap"));
		}
		_Checked = c;
	}

	// ------------------------------------------------------------------------------------------------
	sint32 CViewTextMenu::getAlpha() const
	{
		if (_Grayed)
		{
			return OldColorGrayed.A;
		}
		else
		{
			if (Over)
				return OldColorOver.A;
			else
				return OldColor.A;
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CViewTextMenu::setAlpha (sint32 a)
	{
		OldShadowColor.A = OldColor.A = (uint8)a;
		OldShadowColorOver.A = OldColorOver.A = (uint8)a;
		OldShadowColorGrayed.A = OldColorGrayed.A = (uint8)a;
	}

	// ------------------------------------------------------------------------------------------------
	// CGroupSubMenu
	// ------------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------------
	CGroupSubMenu::CGroupSubMenu(const TCtorParam &param)
	: CGroupSubMenuBase(param)
	{
		_SelectionView = NULL;
		_GroupList = NULL;
		_GroupMenu = NULL;
		_Selected = -1;
		_MaxVisibleLine = -1;
		_ScrollBar = NULL;
	}

	// ------------------------------------------------------------------------------------------------
	CGroupSubMenu::~CGroupSubMenu()
	{
		removeAllUserGroups();
	}

	// ------------------------------------------------------------------------------------------------
	sint CGroupSubMenu::getLineFromId(const std::string &id)
	{
		for (uint k = 0; k < _Lines.size(); ++k)
		{
			if (_Lines[k].Id == id)
			{
				return (sint) k;
			}
		}
		return -1;
	}

	// ------------------------------------------------------------------------------------------------
	CGroupSubMenu *CGroupSubMenu::getSubMenu(uint index) const
	{
		if (index >= _SubMenus.size())
		{
			nlassert("bad index");
			return NULL;
		}
		return _SubMenus[index];
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::setSubMenu(uint index, CGroupSubMenu *sub)
	{
		nlassert(sub != NULL);
		nlassert(index < _SubMenus.size());
		sub->setSerializable( false );

		if (_SubMenus[index] != NULL)
		{
			// must delete from the group menu (if not found, just delete)
			if( !_GroupMenu || !_GroupMenu->delGroup(_SubMenus[index]) )
				delete _SubMenus[index];
			_SubMenus[index] = NULL;
			delView(_Lines[index].RightArrow);
		}

		if (_Lines[index].CheckBox)
			_Lines[index].RightArrow = createRightArrow(_Lines[index].CheckBox, true);
		else
			_Lines[index].RightArrow = createRightArrow(_GroupList, false);

		sub->_GroupMenu = _GroupMenu;
		sub->initOptions(this);
		_GroupMenu->addGroup (sub);
		sub->_DispType = _GroupMenu->_DispType;
		_SubMenus[index] = sub;
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::initOptions(CInterfaceGroup *parent)
	{
		// Initialization
		// me
		_Parent = _GroupMenu;
		if (parent == NULL)
		{
			setParentPos (_GroupMenu);
			setParentPosRef (Hotspot_TL);
			setPosRef (Hotspot_TL);
		}
		else
		{
			setParentPos (parent);
			setParentPosRef (Hotspot_BR);
			setPosRef (Hotspot_BL);
		}
		_DisplayFrame = true;
		_ResizeFromChildH = true;
		_ResizeFromChildW = true;
		_ResizeFromChildHMargin = 8;
		_ResizeFromChildWMargin = 8;
		_ModulateGlobalColor = _GroupMenu->_ModulateGlobalColor;
		// the selection
		if (_SelectionView == NULL)
		{
			_SelectionView = new CViewBitmap(CViewBase::TCtorParam());
	//		CInterfaceManager *pIM = CInterfaceManager::getInstance();
	//		CViewRenderer &rVR = *CViewRenderer::getInstance();
			_SelectionView->setId( getId() + ":selection" );
			_SelectionView->setParent (this);
			_SelectionView->setActive (false);
			_SelectionView->setTexture ("blank.tga");
			_SelectionView->setScale (true);
			_SelectionView->setX (4);
			_SelectionView->setSizeRef(1); // sizeref on W
			_SelectionView->setW (-8);
			_SelectionView->setSerializable( false );
			addView (_SelectionView, 0);
		}
		// the group list
		if (_GroupList == NULL)
		{
			_GroupList = new CGroupList(CViewBase::TCtorParam());
			_GroupList->setId( getId() + ":list" );
			_GroupList->setParent (this);
			_GroupList->setParentPos (this);
			_GroupList->setX (4);
			_GroupList->setY (4);
			_GroupList->setSpace (_GroupMenu->_Space);
			_GroupList->setSerializable( false );
			_GroupList->setResizeFromChildW(true);
			addGroup (_GroupList);
		}
	}

	// ------------------------------------------------------------------------------------------------
	bool CGroupSubMenu::parse (xmlNodePtr cur,  CInterfaceGroup *parent)
	{
		initOptions(parent);
		// the children
		while (cur)
		{
			CViewTextMenu *pV = NULL;

			CXMLAutoPtr id((const char*) xmlGetProp (cur,  (xmlChar*)"id"));

			if (stricmp((char*)cur->name, "separator") == 0)
			{
				if (id)
				{
					addSeparator((const char *) id);
				}
				else
				{
					addSeparator();
				}
			}
			else
			if (stricmp((char*)cur->name, "action") == 0)
			{
				string		strId,  strAh,  strParams,  strCond, strTexture;
				ucstring	ucstrName;

				if (id)		strId = (const char*)id;
				CXMLAutoPtr name((const char*) xmlGetProp (cur,  (xmlChar*)"name"));

				if (name)
				{
					const char *ptrName = (const char*)name;
					ucstrName = ucstring(ptrName);
					if ((strlen(ptrName)>2) && (ptrName[0] == 'u') && (ptrName[1] == 'i'))
						ucstrName = CI18N::get (ptrName);
				}

				CXMLAutoPtr ah((const char*) xmlGetProp (cur,  (xmlChar*)"handler"));
				if (ah)		strAh = (const char*)ah;
				CXMLAutoPtr cond((const char*) xmlGetProp (cur,  (xmlChar*)"cond"));
				if (cond)	strCond = (const char*)cond;
				CXMLAutoPtr params((const char*) xmlGetProp (cur,  (xmlChar*)"params"));
				if (params)	strParams = (const char*)params;
				CXMLAutoPtr strCheckable((const char*) xmlGetProp (cur,  (xmlChar*)"checkable"));
				bool bCheckable = false;
				if (strCheckable)	bCheckable = convertBool (strCheckable);
				CXMLAutoPtr strChecked((const char*) xmlGetProp (cur,  (xmlChar*)"checked"));
				bool bChecked = false;
				if (strChecked)	bChecked = convertBool (strChecked);
				bool bFormatted = false;
				CXMLAutoPtr strFormatted((const char*) xmlGetProp (cur,  (xmlChar*)"formatted"));
				if (strFormatted)	bFormatted = convertBool (strFormatted);

				pV = addLine (ucstrName, strAh, strParams, strId, strCond, strTexture, bCheckable, bChecked, bFormatted);
				pV->setSerializable( false );

				CXMLAutoPtr strSelectable((const char*) xmlGetProp (cur,  (xmlChar*)"selectable"));
				bool bSelectable = true;
				if (strSelectable)	bSelectable = convertBool (strSelectable);
				_Lines.back().Selectable = bSelectable;


				CXMLAutoPtr grayed((const char*) xmlGetProp (cur,  (xmlChar*)"grayed"));
				bool bGrayed = false;
				if (grayed)	bGrayed = convertBool (grayed);
				pV->setGrayed(bGrayed);

				// Is this line has a sub menu ?
				xmlNodePtr child = cur->children;
				if (child != NULL)
				{
					if (_Lines.back().CheckBox)
					{
						_Lines.back().RightArrow = createRightArrow(_Lines.back().CheckBox, true);
					}
					else
					{
						_Lines.back().RightArrow = createRightArrow(_GroupList, false);
					}
					// and create the sub menu
					CGroupSubMenu *childMenu = new CGroupSubMenu(CViewText::TCtorParam());
					childMenu->_GroupMenu = _GroupMenu;
					childMenu->setSerializable( false );
					childMenu->parse (child,  this);

					CXMLAutoPtr MVL((const char*) xmlGetProp(cur,  (xmlChar*)"max_visible_line"));
					if (MVL)
					{
						sint32 maxVisibleLine;
						fromString((const char*)MVL, maxVisibleLine);
						childMenu->setMaxVisibleLine(maxVisibleLine);
					}

					_SubMenus.back() = childMenu;
				}

				// Add user groups
				// Left
				CXMLAutoPtr usergroup((const char*) xmlGetProp (cur,  (xmlChar*)"usergroup_l"));
				if (usergroup)
				{
					vector< pair<string,string> > vparams;
					CXMLAutoPtr ugparams((const char*) xmlGetProp (cur,  (xmlChar*)"usergroup_params_l"));
					if (ugparams)
					{
						IActionHandler::getAllParams((const char*)ugparams, vparams);
					}

					string completeId = _Parent->getId() + ":" + _Lines[_Lines.size()-1].Id;
					CInterfaceGroup *pUGLeft = CWidgetManager::getInstance()->getParser()->createGroupInstance((const char*)usergroup, completeId, vparams);
					if (pUGLeft)
						setUserGroupLeft((uint)_Lines.size()-1, pUGLeft, true);
				}
				usergroup = (char*) xmlGetProp (cur,  (xmlChar*)"usergroup_r");
				if (usergroup)
				{
					vector< pair<string,string> > vparams;
					CXMLAutoPtr ugparams((const char*) xmlGetProp (cur,  (xmlChar*)"usergroup_params_r"));
					if (ugparams)
					{
						IActionHandler::getAllParams((const char*)ugparams, vparams);
					}

					string completeId = _Parent->getId() + ":" + _Lines[_Lines.size()-1].Id;
					CInterfaceGroup *pUG = CWidgetManager::getInstance()->getParser()->createGroupInstance((const char*)usergroup, completeId, vparams);
					if (pUG)
						setUserGroupRight((uint)_Lines.size()-1, pUG, true);
				}
			}
			cur = cur->next;
		}

		_GroupMenu->addGroup (this);
		this->_DispType = _GroupMenu->_DispType;

		return true;
	}

	// ------------------------------------------------------------------------------------------------
	CViewBitmap *CGroupSubMenu::createIcon(CInterfaceElement *parentPos, const string &texture)
	{
		// Add an icon to the line
		CViewBitmap *pVB = new CViewBitmap(CViewBase::TCtorParam());
		pVB->setSerializable( false );
		pVB->setParent (this);
		pVB->setParentPos (parentPos);
		pVB->setParentPosRef (Hotspot_ML);
		pVB->setPosRef (Hotspot_MR);
		pVB->setTexture(texture);
		pVB->setModulateGlobalColor(false);
		pVB->setX (-2);
		addView (pVB);
		return pVB;
	}

	// ------------------------------------------------------------------------------------------------
	CViewBitmap *CGroupSubMenu::createCheckBox(bool checked)
	{
		// Put the left arrow to the line
		CViewBitmap *pVB = new CViewBitmap(CViewBase::TCtorParam());
		pVB->setSerializable( false );
		pVB->setParent (this);
		pVB->setParentPos (_GroupList);
		pVB->setParentPosRef (Hotspot_BR);
		pVB->setPosRef (Hotspot_BL);
		CInterfaceOptions *pIO = CWidgetManager::getInstance()->getOptions("menu_checkbox");
		if (pIO)
		{
			pVB->setTexture(pIO->getValStr(checked ? "checked_bitmap" : "unchecked_bitmap"));
		}
		pVB->setX (MENU_WIDGET_X);
		pVB->setId (ID_MENU_CHECKBOX); // always rescale to parent in update coords
		addView (pVB);
		return pVB;
	}


	// ------------------------------------------------------------------------------------------------
	CViewBitmap	   *CGroupSubMenu::createRightArrow(CInterfaceElement *parentPos, bool center)
	{
		// Put the left arrow to the line
		CViewBitmap *pVB = new CViewBitmap(CViewBase::TCtorParam());
		pVB->setSerializable( false );
		pVB->setParent (this);
		pVB->setParentPos (parentPos);
		if (!center)
		{
			pVB->setParentPosRef (Hotspot_BR);
			pVB->setPosRef (Hotspot_BL);
		}
		else
		{
			pVB->setParentPosRef (Hotspot_MR);
			pVB->setPosRef (Hotspot_ML);
		}
		pVB->setTexture("w_arrow_right_3.tga");
		pVB->setX (MENU_WIDGET_X);
		pVB->setId (ID_MENU_SUBMENU); // rescale to parent in update coords if asked (not needed if there's already on the left a checkbox)
		addView (pVB);
		return pVB;
	}

	// ------------------------------------------------------------------------------------------------
	#define GET_REF_ELM(__index__)	\
		CInterfaceElement	*refElm;						\
		sint32				refElmYReal= 0;					\
		sint32				refElmHReal= 0;					\
		refElm = _Lines[__index__].ViewText;				\
		if(refElm)											\
		{													\
			refElmYReal= refElm->getYReal() - _Lines[__index__].TextDY;	\
			refElmHReal= _Lines[__index__].HReal;			\
		}

	void CGroupSubMenu::updateCoords ()
	{
		if (_ParentPos == _GroupMenu)
		{
			// Root Menu
			setX(_GroupMenu->SpawnMouseX);
			setY(_GroupMenu->SpawnMouseY);
			CGroupFrame::updateCoords();

			CViewRenderer &rVR = *CViewRenderer::getInstance();
			uint32 screenW, screenH;
			rVR.getScreenSize(screenW, screenH);
			if ((_XReal+_WReal) > (sint32)screenW)
				setX(screenW-_WReal);
			if (_YReal < 0)
				setY(_HReal);
		}
		else
		// The sub menu may go outside the screen in Y. => clamp it as possible
		{
			/* X/Y coords have normally been updated before by "parent" sub menus
				Why? because setSubMenu() is typically called in "parent first" order (or it is exactly what is done in ::parse())
				=> Parent CGroupSubMenu::updateCoords() are called before their sons in CGroupMenu::updateCoords() !!!
				=> No Need to call _SubMenus[RALineNb]->updateCoords() below !  (else would call too much time because of recursion!!)
			*/

			// must udpate correct Real coords
			CGroupFrame::updateCoords();

			// get screen size
			CViewRenderer &rVR = *CViewRenderer::getInstance();
			uint32 screenW, screenH;
			rVR.getScreenSize(screenW, screenH);

			sint32	hReal= getHReal();
			// If the H is too big, abort.. can't do anything
			if(hReal<=(sint32)screenH)
			{
				sint32	yReal= getYReal();

				// compute the shift to apply to the Y of the sub menu, to respect (as possible) the screen
				sint32	dyClamp= 0;
				if(yReal<0)
					dyClamp= - yReal;
				if(yReal+hReal>(sint32)screenH)
					dyClamp= screenH - (yReal+hReal);
				// change then the Y
				if(dyClamp!=0)
				{
					setY(getY()+dyClamp);
					CGroupFrame::updateCoords();
				}
			}
		}

		// not yet setuped?
		if (!_GroupList) return;

		// get text dy position
		sint32	textDYPos= 0;
		textDYPos= -(1+_GroupList->getSpace())/2;

		// Set the arrows at the right positions (in _Views we have selection and the right arrows)
		sint32 CBLineNb = 0; // check box
		sint32 RALineNb = 0; // right arrow
		uint32 i;

		sint32 maxUserGroupWidth = 0;
		// compute max width of user groups, & adapt max height for each line
		uint k;

		// update all left user groups to get their width
		sint32 maxLeftUGWidth = 0;
		for(k = 0; k < _Lines.size(); ++k)
		{
			if (_Lines[k].UserGroupLeft)
			{
				_Lines[k].UserGroupLeft->updateCoords();
				maxLeftUGWidth = std::max(_Lines[k].UserGroupLeft->getWReal(), maxLeftUGWidth);
			}
		}

		_GroupList->setX(LEFT_MENU_WIDGET_X + maxLeftUGWidth);

		// active separators when needed
		bool activeLineSeen = false;
		for (i = 0; i < _Lines.size(); ++i)
		{
			if (_Lines[i].Separator != NULL)
			{
				if (i == _Lines.size() - 1)
				{
					_Lines[i].Separator->setActive(false);
					break;
				}
				_Lines[i].Separator->setActive(activeLineSeen);
				activeLineSeen = false;
			}
			else
			{
				if (_Lines[i].ViewText && _Lines[i].ViewText->getActive()) activeLineSeen = true;
			}
		}

		CGroupFrame::updateCoords();

		bool mustUpdate = false;

		if (_MaxVisibleLine > 0 && sint32(_Lines.size())>_MaxVisibleLine)
		{
			for(k = 0; k < _Lines.size(); ++k)
			if (_Lines[k].ViewText)
			{
				// compute max height of widgets on the left of text
				sint32 widgetMaxH = 0;
				if (_Lines[k].UserGroupRight)	widgetMaxH = _Lines[k].UserGroupRight->getHReal();
				if (_Lines[k].UserGroupLeft)	widgetMaxH = std::max(widgetMaxH, _Lines[k].UserGroupLeft->getHReal());
				if (_Lines[k].CheckBox)		widgetMaxH = std::max(widgetMaxH, _Lines[k].CheckBox->getHReal());
				if (_Lines[k].RightArrow)	widgetMaxH = std::max(widgetMaxH, _Lines[k].RightArrow->getHReal());
				widgetMaxH = std::max(widgetMaxH, _Lines[k].ViewText->getHReal());
				_GroupList->setMaxH(widgetMaxH*_MaxVisibleLine+_GroupList->getSpace()*(_MaxVisibleLine-1));
				if (_ScrollBar == NULL)
				{
					_ScrollBar = new CCtrlScroll(CViewBase::TCtorParam());
					_ScrollBar->setParent (this);
					_ScrollBar->setParentPos (_GroupList);
					_ScrollBar->setPosRef (Hotspot_BL);
					_ScrollBar->setParentPosRef (Hotspot_BR);
					_ScrollBar->setX (4);
					_ScrollBar->setY (0);
					_ScrollBar->setW (8);
					_ScrollBar->setTextureBottomOrLeft	("w_scroll_l123_b.tga");
					_ScrollBar->setTextureMiddle		("w_scroll_l123_m.tga");
					_ScrollBar->setTextureTopOrRight	("w_scroll_l123_t.tga");
					_ScrollBar->setTarget(_GroupList);
					_SelectionView->setW (-8-8-2);
					_ScrollBar->setSerializable( false );
					addCtrl(_ScrollBar);
					mustUpdate = true;
				}
				break;
			}
		}
		else
		{
			_SelectionView->setW(-8);
		}


		// *** Setup Text
		for(k = 0; k < _Lines.size(); ++k)
		{
			CInterfaceGroup *ig = _Lines[k].UserGroupRight;
			if (ig)
			{
				ig->updateCoords();
				maxUserGroupWidth = std::max(maxUserGroupWidth, ig->getWReal());
			}
			if (_Lines[k].ViewText)
			{
				// compute max height of widgets on the left of text
				sint32 widgetMaxH = 0;
				if (_Lines[k].UserGroupRight) widgetMaxH = _Lines[k].UserGroupRight->getHReal();
				if (_Lines[k].UserGroupLeft) widgetMaxH = std::max(widgetMaxH, _Lines[k].UserGroupLeft->getHReal());
				if (_Lines[k].CheckBox)  widgetMaxH = std::max(widgetMaxH, _Lines[k].CheckBox->getHReal());
				if (_Lines[k].RightArrow)  widgetMaxH = std::max(widgetMaxH, _Lines[k].RightArrow->getHReal());

				sint32	textHReal= _Lines[k].ViewText->getHReal();
				_Lines[k].HReal= max(widgetMaxH, textHReal);
				_Lines[k].TextDY= textDYPos;
				if(widgetMaxH>textHReal)
					_Lines[k].TextDY+= (widgetMaxH-textHReal) / 2;
			}
		}


		// *** Update Text Positions
	//	sint32 currX = 0;
		for(k = 0; k < _Lines.size(); ++k)
		{
			if (_Lines[k].ViewText)
			{
				// Setup Y
				_Lines[k].ViewText->setY(_Lines[k].TextDY);
			}
		}



		if (mustUpdate)
		{
			CGroupFrame::updateCoords();
		}


		// *** Setup SubMenus and CheckBoxes Positions
		sint32 maxViewW = 0;
		for (i = 1; i < _Views.size(); ++i)
		{
			CViewBitmap *pVB = dynamic_cast<CViewBitmap *>(_Views[i]);
			if (pVB == NULL) continue;
			if (pVB->getId() == ID_MENU_SUBMENU)
			{
				// Look for the next line of the menu that contains a sub menu
				for(;;)
				{
					nlassert (RALineNb < (sint32)_SubMenus.size());
					if (_SubMenus[RALineNb] != NULL) // has a check box or an arrow to indicate submenu ?
					{
						break;
					}
					++RALineNb;
				}

				// get refElm and refElmYReal
				GET_REF_ELM(RALineNb)

				// if there is a check box, y is 0
				if (_Lines[RALineNb].CheckBox || _Lines[RALineNb].UserGroupRight)
				{
					pVB->setY(0);
					pVB->setX(MENU_WIDGET_X);
				}
				else
				{
					sint32 limY = refElmYReal + refElmHReal/2 - _GroupList->getYReal();
					// Setup the arrow at the right pos
					if(_GroupList->getMaxH()>=limY && limY>=0)
					{
						pVB->setY(refElmYReal + (refElmHReal - pVB->getHReal()) / 2 - _GroupList->getYReal());
						pVB->setColor(_Lines[RALineNb].ViewText->getColor());
						pVB->setActive(_Lines[RALineNb].ViewText->getActive());
						pVB->setX(maxUserGroupWidth + MENU_WIDGET_X);
					}
					else
					{
						pVB->setY(0);
						pVB->setActive(false);
					}
				}

				if (_GroupMenu->SpawnOnMousePos)
				{
					_SubMenus[RALineNb]->setParentPos (this);

					// According to mouse position, set the sub menu on the left or right, begin at top or bottom
					CViewRenderer &rVR = *CViewRenderer::getInstance();
					uint32 screenW, screenH;
					rVR.getScreenSize(screenW, screenH);
					if ((_GroupMenu->SpawnMouseX <= ((sint32)screenW/2)) && (_GroupMenu->SpawnMouseY <= ((sint32)screenH/2)))
					{
						_SubMenus[RALineNb]->setParentPosRef(Hotspot_BR);
						_SubMenus[RALineNb]->setPosRef(Hotspot_BL);
						_SubMenus[RALineNb]->setY (refElmYReal - _GroupList->getYReal());
					}
					if ((_GroupMenu->SpawnMouseX <= ((sint32)screenW/2)) && (_GroupMenu->SpawnMouseY > ((sint32)screenH/2)))
					{
						_SubMenus[RALineNb]->setParentPosRef(Hotspot_TR);
						_SubMenus[RALineNb]->setPosRef(Hotspot_TL);
						_SubMenus[RALineNb]->setY (refElmHReal+(refElmYReal - _GroupList->getYReal()) - _GroupList->getHReal());
					}
					if ((_GroupMenu->SpawnMouseX > ((sint32)screenW/2)) && (_GroupMenu->SpawnMouseY <= ((sint32)screenH/2)))
					{
						_SubMenus[RALineNb]->setParentPosRef(Hotspot_BL);
						_SubMenus[RALineNb]->setPosRef(Hotspot_BR);
						_SubMenus[RALineNb]->setY (refElmYReal - _GroupList->getYReal());
					}
					if ((_GroupMenu->SpawnMouseX > ((sint32)screenW/2)) && (_GroupMenu->SpawnMouseY > ((sint32)screenH/2)))
					{
						_SubMenus[RALineNb]->setParentPosRef(Hotspot_TL);
						_SubMenus[RALineNb]->setPosRef(Hotspot_TR);
						_SubMenus[RALineNb]->setY (refElmHReal+(refElmYReal - _GroupList->getYReal()) - _GroupList->getHReal());
					}
					_SubMenus[RALineNb]->setX(0);
				}
				else
				{
					// Setup sub menu
					_SubMenus[RALineNb]->setParentPos (this);
					_SubMenus[RALineNb]->setParentPosRef (Hotspot_BR);
					_SubMenus[RALineNb]->setPosRef (Hotspot_BL);
					_SubMenus[RALineNb]->setY (16+refElmYReal - _GroupList->getYReal() - _SubMenus[RALineNb]->getHReal());
				}

				++RALineNb;
			}
			else if (pVB->getId() == ID_MENU_CHECKBOX)
			{
				for(;;)
				{
					nlassert (CBLineNb < (sint32)_SubMenus.size());
					if (_Lines[CBLineNb].CheckBox != NULL) // has a check box or an arrow to indicate submenu ?
					{
						break;
					}
					++CBLineNb;
				}
				// Setup the arrow at the right pos
				if (!_Lines[CBLineNb].UserGroupRight)
				{
					// get refElm and refElmYReal
					GET_REF_ELM(CBLineNb)

					pVB->setX(maxUserGroupWidth + 2 * MENU_WIDGET_X);

					sint32 limY = refElmYReal + refElmHReal/2 - _GroupList->getYReal();
					// Setup the arrow at the right pos
					if(_GroupList->getMaxH()>=limY && limY>=0)
					{
						pVB->setY(refElmYReal + (refElmHReal - pVB->getHReal()) / 2 - _GroupList->getYReal());
						pVB->setActive(_Lines[CBLineNb].ViewText->getActive());
					}
					else
					{
						pVB->setY(0);
						pVB->setActive(false);
					}
				}
				else
				{
					pVB->setY(0);
					pVB->setX(MENU_WIDGET_X);
				}
				pVB->setColor (_Lines[CBLineNb].ViewText->getColor());
				//
				++CBLineNb;
			}

			if (maxViewW<(pVB->getWReal()+pVB->getX())) maxViewW = pVB->getWReal()+pVB->getX();
		}

		// setup scrollbar position in function of views width
		if(maxViewW>0 && _ScrollBar)
			_ScrollBar->setX(4 + maxViewW);

		// *** Setup user groups positions
		for(k = 0; k < _Lines.size(); ++k)
		{
			CInterfaceGroup *igr = _Lines[k].UserGroupRight;
			CInterfaceGroup *igl = _Lines[k].UserGroupLeft;
			if (igr || igl)
			{
				// get refElm and refElmYReal
				GET_REF_ELM(k)

				if (refElm)
				{
					if (igr)
					{
						igr->setX(MENU_WIDGET_X + maxUserGroupWidth - igr->getWReal());

						sint32 limY = refElmYReal + refElmHReal/2 - _GroupList->getYReal();
						if(_GroupList->getMaxH()>=limY && limY>=0)
						{
							igr->setY(refElmYReal + (refElmHReal - igr->getHReal()) / 2 - _GroupList->getYReal());
							igr->setActive (refElm->getActive());
						}
						else
						{
							igr->setY(0);
							igr->setActive(false);
						}
					}

					if (igl)
					{
						sint32 limY = refElmYReal + refElmHReal/2 - _GroupList->getYReal();
						if(_GroupList->getMaxH()>=limY && limY>=0)
						{
							igl->setY(refElmYReal + (refElmHReal - igl->getHReal()) / 2 - this->getYReal());
							igl->setActive(refElm->getActive());
						}
						else
						{
							igl->setY(0);
							igl->setActive(false);
						}
					}
				}
			}
		}


		sint32 SepLineNb = 0;
		// set separator at the right position
		for (i = 0; i < _ChildrenGroups.size(); ++i)
		{
			CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup *>(_ChildrenGroups[i]);
			if (pIG == NULL) continue;
			if (pIG->getId() != ID_MENU_SEPARATOR) continue; // is it a separator ?

			// set good width
			/*sint32 sw = getW() - _LeftBorder - _RightBorder;
			sw = std::max(sw, (sint32) 0);
			pIG->setW(sw);*/

			// Look for the next line of the menu that contains a separator
			CInterfaceGroup *sep = NULL;
			do
			{
				nlassert (SepLineNb < (sint32)_Lines.size());
				sep = _Lines[SepLineNb].Separator;
				++SepLineNb;
			}
			while (sep == NULL);

			// Setup the arrow at the right pos
			pIG->setY (sep->getYReal() - getYReal());
			pIG->setActive(sep->getActive());
		}
		CGroupFrame::updateCoords();

		//_SelectionView->setW (this->getW());
		_SelectionView->setH (8);
		_SelectionView->setY (4);


		if (_Selected != -1 && _Lines[_Selected].ViewText != NULL)
		{
			CRGBA col= _GroupMenu->_HighLightOver;

			_SelectionView->setColor (col);
			_SelectionView->setModulateGlobalColor(getModulateGlobalColor());

			// get refElm and refElmYReal
			GET_REF_ELM(_Selected)

			_SelectionView->setH (refElmHReal);
			_SelectionView->setY (refElmYReal - this->getYReal());
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::checkCoords()
	{
		if (!_Active) return;
		if (_GroupMenu == NULL) return;

		// if the mouse goes out the window,  unselect all (because handleEvent may not be called)
		sint	xMouse= CWidgetManager::getInstance()->getPointer()->getX();
		sint	yMouse= CWidgetManager::getInstance()->getPointer()->getY();
		if (!((xMouse >= _XReal) &&
			(xMouse < (_XReal + _WReal))&&
			(yMouse > _YReal) &&
			(yMouse <= (_YReal+ _HReal))))
			_Selected= -1;

	//	CViewRenderer &rVR = *CViewRenderer::getInstance();

		// Highlight (background under the selection)
		if (_Selected != -1)
		{
			// display hightlight
			if(_GroupMenu->_HighLightOver.A > 0)
			{
				_SelectionView->setActive (true);
				_SelectionView->invalidateCoords();
			}
			else
			{
				_SelectionView->setActive (false);
			}
		}
		else // no selection
		{
			_SelectionView->setActive (false);
		}

		// Text color if grayed or not
		for (sint32 i = 0; i < (sint32)_Lines.size(); ++i)
		{
			if (_Lines[i].ViewText)
			{
				if (_Lines[i].ViewText->getGrayed()) // Colors when the text is grayed
				{
					_Lines[i].ViewText->setColor (_Lines[i].ViewText->OldColorGrayed);
					_Lines[i].ViewText->setShadowColor (_Lines[i].ViewText->OldShadowColorGrayed);
				}
				else
				{
					if (i == _Selected) // Colors when the text is selected
					{
						_Lines[i].ViewText->Over = true;
						_Lines[i].ViewText->setColor (_Lines[i].ViewText->OldColorOver);
						_Lines[i].ViewText->setShadowColor (_Lines[i].ViewText->OldShadowColorOver);
					}
					else // Or finally normal colors
					{
						_Lines[i].ViewText->Over = false;
						_Lines[i].ViewText->setColor (_Lines[i].ViewText->OldColor);
						_Lines[i].ViewText->setShadowColor (_Lines[i].ViewText->OldShadowColor);
					}
				}
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::draw()
	{
		if (!_Active) return;
		if (_GroupMenu == NULL) return;
		CGroupFrame::draw();
	}

	// ------------------------------------------------------------------------------------------------
	bool CGroupSubMenu::handleEvent (const NLGUI::CEventDescriptor &event)
	{
		if (!_Active)
			return false;

		sint32	textDYPos= 0;
		if(_GroupList)
			textDYPos= -(1+_GroupList->getSpace())/2;

		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
			_Selected = -1;

			// TODO First check sub menus that can be not in the area of this menu

			if (!((eventDesc.getX() >= _XReal) &&
				(eventDesc.getX() < (_XReal + _WReal))&&
				(eventDesc.getY() > _YReal) &&
				(eventDesc.getY() <= (_YReal+ _HReal))))
				return false;

			uint32 i = 0;
			for (i = 0; i < _Lines.size(); ++i)
			{
				if (_Lines[i].Selectable)
				{
					// get refElm and refElmYReal
					GET_REF_ELM(i)

					if (refElm)
					{
						if (refElm->getActive() == true)
						if ((eventDesc.getY() > refElmYReal) &&
							(eventDesc.getY() <= (refElmYReal + refElmHReal + _GroupList->getSpace())))
						{
							_Selected = i;
							break;
						}
					}
				}
			}

			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
			{
				// If a line is selected and the line is not grayed
				if ((_Selected != -1) && (!_Lines[i].ViewText->getGrayed()))
				{

					CAHManager::getInstance()->runActionHandler (	_Lines[_Selected].AHName,
											CWidgetManager::getInstance()->getCtrlLaunchingModal(),
											_Lines[_Selected].AHParams );

					if (_SubMenus[_Selected] != NULL)
					{
						openSubMenu (_Selected);
					}
					else
					{
						// if the menu hasn't triggered a new modal window,  disable it
						if (CWidgetManager::getInstance()->getModalWindow() == _GroupMenu)
						{
							if(_GroupMenu && _GroupMenu->getCloseSubMenuUsingPopModal())
								CWidgetManager::getInstance()->popModalWindow();
							else
								CWidgetManager::getInstance()->disableModalWindow ();
						}
					}
				}
			}

			if (event.getType() == NLGUI::CEventDescriptor::mouse)
			{
				const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
				//
				if (_GroupList && _ScrollBar)
				{
					if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousewheel)
					{
						if (isIn(eventDesc.getX(), eventDesc.getY()))
						{
							sint32 h = 0;
							for (uint32 k = 0; k < _Lines.size(); ++k)
							if (_Lines[k].ViewText)
							{
								// compute max height of widgets on the left of text
								sint32 widgetMaxH = 0;
								if (_Lines[k].UserGroupRight)	widgetMaxH = _Lines[k].UserGroupRight->getHReal();
								if (_Lines[k].UserGroupLeft)	widgetMaxH = std::max(widgetMaxH, _Lines[k].UserGroupLeft->getHReal());
								if (_Lines[k].CheckBox)		widgetMaxH = std::max(widgetMaxH, _Lines[k].CheckBox->getHReal());
								if (_Lines[k].RightArrow)	widgetMaxH = std::max(widgetMaxH, _Lines[k].RightArrow->getHReal());
								widgetMaxH = std::max(widgetMaxH, _Lines[k].ViewText->getHReal());
								h = widgetMaxH+_GroupList->getSpace();
							}
							if (h == 0) h = 1;
							_ScrollBar->moveTargetY(- eventDesc.getWheel() * h);

							hideSubMenus();

							return true;
						}
					}
				}
			}

			return true;
		}
		return false;
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceElement* CGroupSubMenu::getElement (const std::string &id)
	{
		string sTmp = id.substr(0,  _GroupMenu->getId().size());
		if (sTmp != _GroupMenu->getId()) return NULL;

		string sRest = id.substr(_GroupMenu->getId().size()+1,  id.size());

		// Iterate through the tree to see if sRest is present
		CGroupSubMenu *pCurGSM = this;
		while (!sRest.empty())
		{
			// Get the first element of the sRest
			string::size_type posid = sRest.find (":");

			if (posid == string::npos)	// Is there just one token to test ?
			{
				for (uint32 i = 0; i < pCurGSM->_Lines.size(); ++i)
					if (sRest == pCurGSM->_Lines[i].Id)
						return pCurGSM->_Lines[i].ViewText;
				sRest.clear();
			}
			else // no a lot of token left
			{
				string sTok = sRest.substr (0,  posid);
				uint32 i = 0;
				for (i = 0; i < pCurGSM->_Lines.size(); ++i)
					if (sTok == pCurGSM->_Lines[i].Id)
						break;
				if (i == pCurGSM->_Lines.size())
					return NULL;

				// No sub-menus
				if (pCurGSM->_SubMenus[i] == NULL)
				{
					// Get next token
					sRest = sRest.substr (posid+1);
					posid = sRest.find (":");
					if (posid == string::npos)
						sTok = sRest;
					else
						sTok = sRest.substr (0,  posid);
					// Do we want left or right user group ?
					if (pCurGSM->_Lines[i].UserGroupRight)
					{
						string sUGid = pCurGSM->_Lines[i].UserGroupRight->getId();
						sUGid = sUGid.substr(sUGid.rfind(':')+1,sUGid.size());
						if (sUGid == sTok)
						{
							CInterfaceElement *pIE = pCurGSM->_Lines[i].UserGroupRight->getElement(id);
							return pIE;
						}
					}
					if (pCurGSM->_Lines[i].UserGroupLeft)
					{
						string sUGid = pCurGSM->_Lines[i].UserGroupLeft->getId();
						sUGid = sUGid.substr(sUGid.rfind(':')+1,sUGid.size());
						if (sUGid == sTok)
						{
							CInterfaceElement *pIE = pCurGSM->_Lines[i].UserGroupLeft->getElement(id);
							return pIE;
						}
					}

					return NULL;
				}
				else
				{
					pCurGSM = pCurGSM->_SubMenus[i];
				}
			}
			sRest = sRest.substr (posid+1);
		}

		return NULL;
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::addSeparator(const std::string &id)
	{
		addSeparatorAtIndex((uint)_Lines.size(), id);
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::addSeparatorAtIndex(uint index, const std::string &id)
	{
		if (index > _Lines.size())
		{
			nlwarning("Bad index");
			return;
		}

			// create the real separator. It may be larger than the group list, this is why we create a separate group
		CInterfaceGroup *separator = CWidgetManager::getInstance()->getParser()->createGroupInstance("menu_separator", "", NULL, 0);
		if (!separator) return;
		separator->setSerializable( false );
		separator->setId(ID_MENU_SEPARATOR);
		separator->setSerializable( false );
		addGroup(separator);
		separator->setParent(this);
		// create place holder group
		CInterfaceGroup *ph = CWidgetManager::getInstance()->getParser()->createGroupInstance("menu_separator_empty", "", NULL, 0);
		if (!ph)
		{
			delGroup(separator);
			return;
		}
		_GroupList->addChildAtIndex(ph, index);
		SSubMenuEntry tmp;
		tmp.Id = id;
		tmp.Separator = ph;
		tmp.ViewText = NULL;
		tmp.CheckBox = NULL;
		tmp.RightArrow = NULL;
		_Lines.insert(_Lines.begin() + index, tmp);
		_SubMenus.insert(_SubMenus.begin() + index, (CGroupSubMenu*)NULL);
		_GroupMenu->invalidateCoords();
	}


	// ------------------------------------------------------------------------------------------------
	CViewTextMenu* CGroupSubMenu::addLine (const ucstring &name,  const std::string &ah,
										   const std::string &params,  const std::string &id,
										   const std::string &cond, const std::string &texture,
										   bool checkable /*= false*/,  bool checked /*= false*/, bool formatted /*= false */
										  )
	{
		SSubMenuEntry tmp;

		CViewTextMenu *pV = new CViewTextMenu(CViewBase::TCtorParam());


		pV->setCaseMode(_GroupMenu->getCaseMode());
		if (formatted)
		{
			pV->setMultiLine (true);
			pV->setMultiLineMaxWOnly (true);
			pV->setTextFormatTaged (name);
		}
		else
		{
			pV->setText (name);
		}
		pV->setColor (_GroupMenu->_Color);
		pV->setFontSize (_GroupMenu->_FontSize);
		pV->setShadow (_GroupMenu->_Shadow);
		pV->setShadowOutline (_GroupMenu->_ShadowOutline);
		pV->setCheckable(checkable);
		pV->setChecked(checked);
		pV->setModulateGlobalColor(_GroupMenu->_ModulateGlobalColor);

		pV->OldColor =				_GroupMenu->_Color;
		pV->OldShadowColor =		_GroupMenu->_ShadowColor;
		pV->OldColorOver =			_GroupMenu->_ColorOver;
		pV->OldShadowColorOver =	_GroupMenu->_ShadowColorOver;
		pV->OldColorGrayed =		_GroupMenu->_ColorGrayed;
		pV->OldShadowColorGrayed =	_GroupMenu->_ShadowColorGrayed;

		_GroupList->addChild (pV);

		CViewBitmap *checkBox = NULL;

		if (checkable)
		{
			checkBox = createCheckBox(checked);
			checkBox->setTexture(texture);
			pV->setCheckBox(checkBox);
		}

		CViewBitmap *icon = NULL;
		if (!texture.empty())
		{	
			if (_GroupList->getNumChildren() == 1)
				pV->setX(20);
			icon = createIcon(pV, texture);
		}
		

		tmp.ViewText = pV;
		tmp.Separator = NULL;
		tmp.AHName = ah;
		tmp.AHParams = params;
		tmp.Cond = cond;
		tmp.CheckBox = checkBox;
		tmp.RightArrow = icon;
		if (id.empty())
			tmp.Id = NLMISC::toString (_Lines.size());
		else
			tmp.Id = id;

		pV->setId(_GroupMenu->getId()+":"+tmp.Id);

		_Lines.push_back (tmp);

		// Add an empty sub menu by default
		_SubMenus.push_back (NULL);

		_GroupMenu->invalidateCoords();

		return pV;
	}

	CViewTextMenu* CGroupSubMenu::addLineAtIndex(uint index,  const ucstring &name,  const std::string &ah,
												 const std::string &params,  const std::string &id /*=""*/,
												 const std::string &cond /*=std::string()*/, const std::string &texture,
												 bool checkable /*= false*/,  bool checked /*= false*/, bool formatted /*= false */
												)
	{
		if (index > _Lines.size())
		{
			nlwarning("Bad index");
			return NULL;
		}
		SSubMenuEntry tmp;
		CViewTextMenu *pV = new CViewTextMenu(CViewBase::TCtorParam());
		pV->setSerializable( false );


		pV->setCaseMode(_GroupMenu->getCaseMode());


		if (formatted)
		{
			pV->setMultiLine (true);
			pV->setMultiLineMaxWOnly (true);
			pV->setTextFormatTaged (name);
		}
		else
		{
			pV->setText (name);
		}

		pV->setColor (_GroupMenu->_Color);
		pV->setFontSize (_GroupMenu->_FontSize);
		pV->setShadow (_GroupMenu->_Shadow);
		pV->setShadowOutline (_GroupMenu->_ShadowOutline);
		pV->setCheckable(checkable);
		pV->setChecked(checked);
		pV->setModulateGlobalColor(_GroupMenu->_ModulateGlobalColor);

		pV->OldColor =				_GroupMenu->_Color;
		pV->OldShadowColor =		_GroupMenu->_ShadowColor;
		pV->OldColorOver =			_GroupMenu->_ColorOver;
		pV->OldShadowColorOver =	_GroupMenu->_ShadowColorOver;
		pV->OldColorGrayed =		_GroupMenu->_ColorGrayed;
		pV->OldShadowColorGrayed =	_GroupMenu->_ShadowColorGrayed;

		_GroupList->addChildAtIndex(pV,  index);

		CViewBitmap *checkBox = NULL;
		if (checkable)
		{
			checkBox = createCheckBox(checked);
			checkBox->setTexture(texture);
			pV->setCheckBox(checkBox);
		}

		tmp.ViewText = pV;
		tmp.Separator = NULL;
		tmp.AHName = ah;
		tmp.AHParams = params;
		tmp.Cond = cond;
		tmp.CheckBox = checkBox;
		tmp.RightArrow = NULL;

		if (id.empty())
			tmp.Id = NLMISC::toString (_Lines.size());
		else
			tmp.Id = id;

		pV->setId(getId()+":"+tmp.Id);

		_Lines.insert(_Lines.begin() + index,  tmp);

		// Add an empty sub menu by default
		_SubMenus.insert(_SubMenus.begin() + index,  (CGroupSubMenu*)NULL);

		_GroupMenu->invalidateCoords();

		return pV;
	}


	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::removeLine(uint index)
	{
		if (index >= _Lines.size())
		{
			nlwarning("Bad index");
			return;
		}
		setUserGroupRight(index, NULL, false); // remove user group
		setUserGroupLeft(index, NULL, false); // remove user group
		// remove view (right arrow & checkbox)
		if (_Lines[index].RightArrow) delView(_Lines[index].RightArrow);
		if (_Lines[index].CheckBox) delView(_Lines[index].CheckBox);
		if (_Lines[index].Separator)
		{
			// remove one separator group
			for(uint k = 0; k < _ChildrenGroups.size(); ++k)
			{
				if (_ChildrenGroups[k]->getId() == ID_MENU_SEPARATOR)
				{
					delGroup(_ChildrenGroups[k]);
					break;
				}
			}
		}
		//
		_GroupList->setDelOnRemove(index, true);
		_GroupList->delChild(index);
		_Lines.erase(_Lines.begin() + index);

		//invalidate selection
		_Selected = -1;

		if(_SubMenus[index])
		{
			// reset it and his sons (recurs)
			_SubMenus[index]->reset();
			// then delete it
			_GroupMenu->delGroup(_SubMenus[index]);
		}
		_SubMenus.erase(_SubMenus.begin() + index);
		_GroupMenu->invalidateCoords();
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::openSubMenu (sint32 nb)
	{
		hideSubMenus ();
		_SubMenus[nb]->setActive (true);
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::hideSubMenus ()
	{
		for (uint32 i = 0; i < _SubMenus.size(); ++i)
			if (_SubMenus[i] != NULL)
			{
				_SubMenus[i]->setActive (false);
				_SubMenus[i]->hideSubMenus ();
			}
	}

	// ------------------------------------------------------------------------------------------------
	void	CGroupSubMenu::reset()
	{
		uint lineCount = (uint)_Lines.size();
		for(sint k = lineCount - 1; k >= 0; --k)
		{
			removeLine(k);
		}
	}


	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::removeAllUserGroups()
	{
		for(uint k = 0; k < _Lines.size(); ++k)
		{
			setUserGroupRight(k, NULL, false);
			setUserGroupLeft(k, NULL, false);
		}
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceGroup *CGroupSubMenu::getUserGroupRight(uint line) const
	{
		if (line >= _Lines.size())
		{
			nlwarning("bad index");
			return NULL;
		}
		return _Lines[line].UserGroupRight;
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceGroup *CGroupSubMenu::getUserGroupLeft(uint line) const
	{
		if (line >= _Lines.size())
		{
			nlwarning("bad index");
			return NULL;
		}
		return _Lines[line].UserGroupLeft;
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::setUserGroupRight(uint line, CInterfaceGroup *group, bool ownership)
	{
		if (line >= _Lines.size())
		{
			nlwarning("bad index");
			return;
		}
		if (group && isChildGroup(group))
		{
			nlwarning("Group inserted twice");
			return;
		}
		if (_Lines[line].UserGroupRight)
		{
			delGroup(_Lines[line].UserGroupRight, !_Lines[line].UserGroupRightOwnership);
		}
		_Lines[line].UserGroupRight = group;
		_Lines[line].UserGroupRightOwnership = ownership;
		if (group)
		{
			CViewBase *prevElem = _Lines[line].CheckBox ? _Lines[line].CheckBox : _Lines[line].RightArrow;
			if (prevElem)
			{
				prevElem->setParentPosRef (Hotspot_MR);
				prevElem->setPosRef (Hotspot_ML);
				prevElem->setParentPos(group);
				prevElem->setX(MENU_WIDGET_X);
				prevElem->setY(0);
			}
			sint insertionOrder;
			if (prevElem)
			{
				insertionOrder = getInsertionOrder(prevElem);
			}
			else
			{
				insertionOrder = -1;
			}
			addGroup(group, insertionOrder);
			group->setParent(this);
			group->setParentPos(_GroupList);
			group->setParentPosRef (Hotspot_BR);
			group->setPosRef (Hotspot_BL);
		}
		else
		{
			// restore all posref..
			CViewBase *prevElem = _Lines[line].CheckBox ? _Lines[line].CheckBox : _Lines[line].RightArrow;
			if (prevElem)
			{
				prevElem->setParent (this);
				prevElem->setParentPos (_GroupList);
				prevElem->setParentPosRef (Hotspot_BR);
				prevElem->setPosRef (Hotspot_BL);
				prevElem->setX (MENU_WIDGET_X);
			}
		}
		_GroupMenu->invalidateCoords();
	}


	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::setUserGroupLeft(uint line, CInterfaceGroup *group, bool ownership)
	{
		if (line >= _Lines.size())
		{
			nlwarning("bad index");
			return;
		}
		if (group && isChildGroup(group))
		{
			nlwarning("Group inserted twice");
			return;
		}
		if (_Lines[line].UserGroupLeft)
		{
			delGroup(_Lines[line].UserGroupLeft, !_Lines[line].UserGroupLeftOwnership);
		}
		_Lines[line].UserGroupLeft = group;
		_Lines[line].UserGroupLeftOwnership = ownership;
		if (group)
		{
			addGroup(group);
			group->setParent(this);
			group->setParentPos(this);
			group->setParentPosRef (Hotspot_BL);
			group->setPosRef (Hotspot_BL);
			group->setX(LEFT_MENU_WIDGET_X);
		}

		_GroupMenu->invalidateCoords();
	}


	// ------------------------------------------------------------------------------------------------
	CGroupSubMenu *CGroupSubMenu::cloneMenu(CGroupSubMenu *appendToMenu,  CGroupMenu	*newFather,  CInterfaceGroup *initGroup /* = NULL */) const
	{
		CGroupSubMenu *copyMenu = appendToMenu ? appendToMenu : new CGroupSubMenu(CViewText::TCtorParam());
		uint startSize = (uint)copyMenu->_Lines.size();
		copyMenu->setSerializable( false );
		copyMenu->_GroupMenu = newFather;
		copyMenu->initOptions(initGroup);
		copyMenu->_Lines.reserve(_Lines.size() + startSize);
		copyMenu->_SubMenus.reserve(_SubMenus.size() + startSize);
		// copy childrens
		for(uint k = 0; k < _Lines.size(); ++k)
		{
			if (_Lines[k].Separator)
			{
				copyMenu->addSeparator(_Lines[k].Id);
			}
			else
			{
				std::string texture = std::string();
				if(_Lines[k].ViewText->getCheckBox())
				{
					texture = _Lines[k].ViewText->getCheckBox()->getTexture();
				}
				CViewTextMenu *pV = NULL;
				pV = copyMenu->addLine (_Lines[k].ViewText->getText(),  _Lines[k].AHName,  _Lines[k].AHParams,  _Lines[k].Id,  _Lines[k].Cond,
						 texture, _Lines[k].ViewText->getCheckable(),  _Lines[k].ViewText->getChecked(), _Lines[k].ViewText->getFormatted ());
				copyMenu->_Lines[k].Selectable = _Lines[k].Selectable;
				pV->setGrayed(_Lines[k].ViewText->getGrayed());
			}

			// sub menu copy if there's one
			if (_SubMenus[k] != NULL)
			{

				if (copyMenu->_Lines.back().CheckBox)
				{
					copyMenu->_Lines.back().RightArrow = copyMenu->createRightArrow(copyMenu->_Lines.back().CheckBox, true);
				}
				else
				{
					copyMenu->_Lines.back().RightArrow = copyMenu->createRightArrow(copyMenu->_GroupList, false);
				}


				// and create the sub menu
				copyMenu->_SubMenus[k + startSize] = _SubMenus[k]->cloneMenu(NULL,  newFather,  copyMenu);
			}
		}
		if (!appendToMenu)
		{
			newFather->addGroup(copyMenu);
		}
		return copyMenu;
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::setActive(bool state)
	{
		// check conditions
		for(uint k = 0; k < _Lines.size(); ++k)
		{
			if (!_Lines[k].Cond.empty())
			{
				CInterfaceExprValue result;
				if (CInterfaceExpr::eval(_Lines[k].Cond,  result))
				{
					if (result.toBool())
					{
						_Lines[k].ViewText->setGrayed(!result.getBool());
					}
				}
			}
		}

		if(_ScrollBar && _GroupList)
			_ScrollBar->setTrackPos(_GroupList->getHReal());

		CGroupFrame::setActive(state);
	}

	// ------------------------------------------------------------------------------------------------
	const std::string CGroupSubMenu::getActionHandler(uint lineIndex) const
	{
		if (lineIndex > _Lines.size())
		{
			nlwarning("Bad index");
			return 0;
		}
		return _Lines[lineIndex].AHName;
	}

	// ------------------------------------------------------------------------------------------------
	const std::string CGroupSubMenu::getActionHandlerParam(uint lineIndex) const
	{
		if (lineIndex > _Lines.size())
		{
			nlwarning("Bad index");
			return 0;
		}
		return _Lines[lineIndex].AHParams;
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::setActionHandler(uint lineIndex, const std::string &ah)
	{
		if (lineIndex > _Lines.size())
		{
			nlwarning("Bad index");
			return;
		}
		_Lines[lineIndex].AHName = ah;
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::setActionHandlerParam(uint lineIndex, const std::string &params)
	{
		if (lineIndex > _Lines.size())
		{
			nlwarning("Bad index");
			return;
		}
		_Lines[lineIndex].AHParams = params;
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::setSelectable(uint lineIndex, bool selectable)
	{
		if (lineIndex > _Lines.size())
		{
			nlwarning("Bad index");
			return;
		}
		_Lines[lineIndex].Selectable = selectable;
	}

	// ------------------------------------------------------------------------------------------------
	bool CGroupSubMenu::getSelectable(uint lineIndex) const
	{
		if (lineIndex > _Lines.size())
		{
			nlwarning("Bad index");
			return 0;
		}
		return _Lines[lineIndex].Selectable;
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::setMaxVisibleLine(sint32 mvl)
	{
		_MaxVisibleLine = mvl;
	}

	// ------------------------------------------------------------------------------------------------
	const std::string &CGroupSubMenu::getLineId(uint index)
	{
		if(index>=_Lines.size())
		{
			static string nullString;
			return nullString;
		}
		else
			return _Lines[index].Id;
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::setGrayedLine(uint line,  bool g)
	{
		if(line<_Lines.size())
		{
			if (_Lines[line].ViewText)
			{
				_Lines[line].ViewText->setGrayed(g);
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupSubMenu::setHiddenLine(uint line,  bool h)
	{
		if(line<_Lines.size())
		{
			if (_Lines[line].ViewText)
			{
				_Lines[line].ViewText->setActive(!h);
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaGetNumLine(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "getNumLine", 0);
		ls.push(getNumLine());
		return 1;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaGetSubMenu(CLuaState &ls)
	{
		const char *funcName = "getSubMenu";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		CLuaIHM::pushUIOnStack(ls, getSubMenu((uint) ls.toInteger(1)));
		return 1;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaAddSubMenu(CLuaState &ls)
	{
		const char *funcName = "addSubMenu";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		setSubMenu((uint) ls.toInteger(1), new CGroupSubMenu(CViewText::TCtorParam()));
		CLuaIHM::pushUIOnStack(ls, getSubMenu((uint) ls.toInteger(1)));
		return 1;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaGetLineId(CLuaState &ls)
	{
		const char *funcName = "getLineId";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		std::string id = getLineId((uint) ls.toInteger(1));
		CLuaIHM::push(ls, id);
		return 1;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaGetLineFromId(CLuaState &ls)
	{
		const char *funcName = "getLineFromId";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		ls.push(getLineFromId(ls.toString(1)));
		return 1;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaIsSeparator(CLuaState &ls)
	{
		const char *funcName = "isSeparator";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		ls.push(isSeparator((uint) ls.toInteger(1)));
		return 1;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaAddLine(CLuaState &ls)
	{
		const char *funcName = "addLine";
		CLuaIHM::checkArgCount(ls, funcName, 4);
		CLuaIHM::checkArgTypeUCString(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 3, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 4, LUA_TSTRING);
		ucstring arg1;
		nlverify(CLuaIHM::getUCStringOnStack(ls, 1, arg1));
		addLine(arg1, ls.toString(2), ls.toString(3), ls.toString(4));
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaAddIconLine(CLuaState &ls)
	{
		const char *funcName = "addIconLine";
		CLuaIHM::checkArgCount(ls, funcName, 5);
		CLuaIHM::checkArgTypeUCString(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 3, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 4, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 5, LUA_TSTRING);
		ucstring arg1;
		nlverify(CLuaIHM::getUCStringOnStack(ls, 1, arg1));
		addLine(arg1, ls.toString(2), ls.toString(3), ls.toString(4), string(), ls.toString(5));
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaAddLineAtIndex(CLuaState &ls)
	{
		const char *funcName = "addLineAtIndex";
		CLuaIHM::checkArgCount(ls, funcName, 5);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		CLuaIHM::checkArgTypeUCString(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 3, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 4, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 5, LUA_TSTRING);
		ucstring arg2;
		nlverify(CLuaIHM::getUCStringOnStack(ls, 2, arg2));
		addLineAtIndex((uint) ls.toInteger(1), arg2, ls.toString(3), ls.toString(4), ls.toString(5));
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaAddSeparator(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "addSeparator", 0);
		addSeparator();
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaAddSeparatorAtIndex(CLuaState &ls)
	{
		const char *funcName = "addSeparatorAtIndex";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		addSeparatorAtIndex((uint) ls.toInteger(1));
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaRemoveLine(CLuaState &ls)
	{
		const char *funcName = "removeLine";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		removeLine((uint) ls.toInteger(1));
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaSetUserGroupRight(CLuaState &ls)
	{
		const char *funcName = "setUserGroupRight";
		CLuaIHM::checkArgCount(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		if (!(CLuaIHM::isUIOnStack(ls, 2) || ls.isNil(2)))
		{
			CLuaIHM::fails(ls, "%s :  Group required as argument 2", funcName);
		}
		CInterfaceElement *el = CLuaIHM::getUIOnStack(ls, 2);
		CInterfaceGroup *group = dynamic_cast<CInterfaceGroup *>(el);
		if (el && !group)
		{
			CLuaIHM::fails(ls, "%s :  Group required as argument 2", funcName);
		}
		setUserGroupRight((uint) ls.toInteger(1), group, true);
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaSetUserGroupLeft(CLuaState &ls)
	{
		const char *funcName = "setUserGroupLeft";
		CLuaIHM::checkArgCount(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		if (!(CLuaIHM::isUIOnStack(ls, 2) || ls.isNil(2)))
		{
			CLuaIHM::fails(ls, "%s :  Group required as argument 2", funcName);
		}
		CInterfaceElement *el = CLuaIHM::getUIOnStack(ls, 2);
		CInterfaceGroup *group = dynamic_cast<CInterfaceGroup *>(el);
		if (el && !group)
		{
			CLuaIHM::fails(ls, "%s :  Group required as argument 2", funcName);
		}
		setUserGroupLeft((uint) ls.toInteger(1), group, true);
		return 0;
	}


	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaGetUserGroupRight(CLuaState &ls)
	{
		const char *funcName = "getUserGroupRight";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		CLuaIHM::pushUIOnStack(ls, getUserGroupRight((uint) ls.toInteger(1)));
		return 1;
	}


	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaGetUserGroupLeft(CLuaState &ls)
	{
		const char *funcName = "getUserGroupLeft";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		CInterfaceElement *pIE = getUserGroupLeft((uint) ls.toInteger(1));
		if (pIE)
		{
			CLuaIHM::pushUIOnStack(ls, pIE);
			return 1;
		}
		else return 0;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaSetMaxVisibleLine(CLuaState &ls)
	{
		const char *funcName = "setMaxVisibleLine";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		setMaxVisibleLine((uint) ls.toInteger(1));
		return 0;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupSubMenu::luaReset(CLuaState &ls)
	{
		const char *funcName = "reset";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		reset();
		return 0;
	}


	// ------------------------------------------------------------------------------------------------
	// CGroupMenu
	// ------------------------------------------------------------------------------------------------

	NLMISC_REGISTER_OBJECT(CViewBase, CGroupMenu, std::string, "menu");

	// ------------------------------------------------------------------------------------------------
	CGroupMenu::CGroupMenu(const TCtorParam &param)
	:CGroupModal(param)
	{
		_Active = false;
		_Color = CRGBA::White;
		_ColorOver = CRGBA::White;
		_ColorGrayed = CRGBA(128, 128, 128, 255);
		_ShadowColor = CRGBA::Black;
		_ShadowColorOver = CRGBA::Black;
		_ShadowColorGrayed = CRGBA::Black;
		_HighLightOver.set(128, 0, 0, 255);
		_FontSize = 12;
		_Shadow = false;
		_ShadowOutline = false;
		_ResizeFromChildH = _ResizeFromChildW = true;
		_DisplayFrame = false;
		_RootMenu = NULL;
		_Space = 3;
		_CaseMode = CaseUpper;
		_Formatted = false;
	}

	// ------------------------------------------------------------------------------------------------
	CGroupMenu::~CGroupMenu()
	{
	}

	std::string CGroupMenu::getProperty( const std::string &name ) const
	{
		if( name == "extends" )
		{
			return _Extends;
		}
		else
		if( name == "case_mode" )
		{
			uint32 cm = _CaseMode;
			return toString( cm );
		}
		else
		if( name == "color" )
		{
			return toString( _Color );
		}
		else
		if( name == "shadow_color" )
		{
			return toString( _ShadowColor );
		}
		else
		if( name == "color_over" )
		{
			return toString( _ColorOver );
		}
		else
		if( name == "shadow_color_over" )
		{
			return toString( _ShadowColorOver );
		}
		else
		if( name == "highlight_over" )
		{
			return toString( _HighLightOver );
		}
		else
		if( name == "color_grayed" )
		{
			return toString( _ColorGrayed );
		}
		else
		if( name == "shadow_color_grayed" )
		{
			return toString( _ShadowColorGrayed );
		}
		else
		if( name == "space" )
		{
			return toString( _Space );
		}
		else
		if( name == "fontsize" )
		{
			return toString( _FontSize );
		}
		else
		if( name == "shadow" )
		{
			return toString( _Shadow );
		}
		else
		if( name == "shadow_outline" )
		{
			return toString( _ShadowOutline );
		}
		else
		if( name == "formatted" )
		{
			return toString( _Formatted );
		}
		else
		if( name == "max_visible_line" )
		{
			if( _RootMenu == NULL )
				return "0";
			else
				return toString( _RootMenu->getMaxVisibleLine() );
		}
		else
			return CGroupModal::getProperty( name );
	}

	void CGroupMenu::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "extends" )
		{
			_Extends = value;
			return;
		}
		else
		if( name == "case_mode" )
		{
			uint32 cm;
			if( fromString( value, cm ) )
				_CaseMode = (TCaseMode)cm;
			return;
		}
		else
		if( name == "color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_Color = c;
			return;
		}
		else
		if( name == "shadow_color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_ShadowColor = c;
			return;
		}
		else
		if( name == "color_over" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_ColorOver = c;
			return;
		}
		else
		if( name == "shadow_color_over" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_ShadowColorOver = c;
			return;
		}
		else
		if( name == "highlight_over" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_HighLightOver = c;
			return;
		}
		else
		if( name == "color_grayed" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_ColorGrayed = c;
			return;
		}
		else
		if( name == "shadow_color_grayed" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_ShadowColorGrayed = c;
			return;
		}
		else
		if( name == "space" )
		{
			uint8 i;
			if( fromString( value, i ) )
				_Space = i;
			return;
		}
		else
		if( name == "fontsize" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_FontSize = i;
			return;
		}
		else
		if( name == "shadow" )
		{
			bool b;
			if( fromString( value, b ) )
				_Shadow = b;
			return;
		}
		else
		if( name == "shadow_outline" )
		{
			bool b;
			if( fromString( value, b ) )
				_ShadowOutline = b;
			return;
		}
		else
		if( name == "formatted" )
		{
			bool b;
			if( fromString( value, b ) )
				_Formatted = b;
			return;
		}
		else
		if( name == "max_visible_line" )
		{
			if( _RootMenu != NULL )
			{
				sint32 i;
				if( fromString( value, i ) )
					_RootMenu->setMaxVisibleLine( i );
			}
			return;
		}
		else
			CGroupModal::setProperty( name, value );
	}


	xmlNodePtr CGroupMenu::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CGroupModal::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "menu" );
		xmlSetProp( node, BAD_CAST "extends", BAD_CAST _Extends.c_str() );
		xmlSetProp( node, BAD_CAST "case_mode", BAD_CAST toString( uint32( _CaseMode ) ).c_str() );
		xmlSetProp( node, BAD_CAST "color", BAD_CAST toString( _Color ).c_str() );
		xmlSetProp( node, BAD_CAST "shadow_color", BAD_CAST toString( _ShadowColor ).c_str() );
		xmlSetProp( node, BAD_CAST "color_over", BAD_CAST toString( _ColorOver ).c_str() );
		xmlSetProp( node, BAD_CAST "shadow_color_over", BAD_CAST toString( _ShadowColorOver ).c_str() );
		xmlSetProp( node, BAD_CAST "highlight_over", BAD_CAST toString( _HighLightOver ).c_str() );
		xmlSetProp( node, BAD_CAST "color_grayed", BAD_CAST toString( _ColorGrayed ).c_str() );
		xmlSetProp( node, BAD_CAST "shadow_color_grayed", BAD_CAST toString( _ShadowColorGrayed ).c_str() );
		xmlSetProp( node, BAD_CAST "space", BAD_CAST toString( _Space ).c_str() );
		xmlSetProp( node, BAD_CAST "fontsize", BAD_CAST toString( _FontSize ).c_str() );
		xmlSetProp( node, BAD_CAST "shadow", BAD_CAST toString( _Shadow ).c_str() );
		xmlSetProp( node, BAD_CAST "shadow_outline", BAD_CAST toString( _ShadowOutline ).c_str() );
		xmlSetProp( node, BAD_CAST "formatted", BAD_CAST toString( _Formatted ).c_str() );
		
		if( _RootMenu == NULL )
			xmlSetProp( node, BAD_CAST "max_visible_line", BAD_CAST "0" );
		else
			xmlSetProp( node, BAD_CAST "max_visible_line",
				BAD_CAST toString( _RootMenu->getMaxVisibleLine() ).c_str() );

		return NULL;
	}

	// ------------------------------------------------------------------------------------------------
	bool CGroupMenu::parse (xmlNodePtr in,  CInterfaceGroup *parentGroup)
	{
		CXMLAutoPtr prop;

		_FontSize = 12;

		// override source menu options (if there's one)
		if (!CGroupModal::parse(in, parentGroup))
			return false;


		// see if this menu extends another menu
		prop= (char*) xmlGetProp( in,  (xmlChar*)"extends" );
		CGroupSubMenu *gmExtended = NULL;
		if (prop)
		{
			if( editorMode )
				_Extends = std::string( (const char*)prop );

			CGroupMenu *gm = dynamic_cast<CGroupMenu *>(CWidgetManager::getInstance()->getElementFromId(prop.str()));
			if (!gm)
			{
				gm = dynamic_cast<CGroupMenu *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:" + std::string((const char*)prop)));
			}
			if (gm)
			{
				gmExtended = gm->_RootMenu;
				// copy group frame parameters
				CGroupFrame::copyOptionFrom(*gm);
				// copy basic datas
				_Color = gm->_Color;
				_ShadowColor = gm->_ShadowColor;
				_Shadow = gm->_Shadow;
				_ShadowOutline = gm->_ShadowOutline;
				_FontSize = gm->_FontSize;
				_ColorOver = gm->_ColorOver;
				_ShadowColorOver = gm->_ShadowColorOver;
				_HighLightOver = gm->_HighLightOver;
				_ColorGrayed = gm->_ColorGrayed;
				_ShadowColorGrayed = gm->_ShadowColorGrayed;
				_Priority = gm->_Priority;
				_ModulateGlobalColor = gm->_ModulateGlobalColor;
				_Space = gm->_Space;
				_CaseMode = gm->_CaseMode;
			}
			else
			{
				nlwarning("Can't get menu %s or bad type",  (const char *) prop);
			}
		}


		// Override the modal behaviour because of sub menus
		ExitClickOut = true;
		ExitClickL = false;
		ExitClickR = false;
		ExitKeyPushed = true;
		_ResizeFromChildH = _ResizeFromChildW = true;
		_DisplayFrame = false;
		_Active = false;
		_Formatted = false;

		// text colors
		prop= (char*) xmlGetProp( in,  (xmlChar*)"color" );
		if (prop)	_Color = convertColor(prop);

		prop = (char*) xmlGetProp( in, (xmlChar*)"case_mode" );
		if (prop)
		{
			sint32 caseMode;
			fromString((const char*)prop, caseMode);
			_CaseMode = (TCaseMode)caseMode;
		}

		prop= (char*) xmlGetProp( in,  (xmlChar*)"shadow_color" );
		if (prop)	_ShadowColor = convertColor(prop);

		prop= (char*) xmlGetProp( in,  (xmlChar*)"color_over" );
		if (prop)	_ColorOver = convertColor(prop);

		prop= (char*) xmlGetProp( in,  (xmlChar*)"shadow_color_over" );
		if (prop)	_ShadowColorOver = convertColor(prop);

		prop= (char*) xmlGetProp( in,  (xmlChar*)"highlight_over" );
		if (prop)	_HighLightOver = convertColor(prop);

		prop= (char*) xmlGetProp( in,  (xmlChar*)"color_grayed" );
		if (prop)	_ColorGrayed = convertColor(prop);

		prop= (char*) xmlGetProp( in,  (xmlChar*)"shadow_color_grayed" );
		if (prop)	_ShadowColorGrayed = convertColor(prop);

		prop = (char*) xmlGetProp (in,  (xmlChar*)"space");
		if (prop) fromString((const char*)prop, _Space);

		// Text props
		prop = (char*) xmlGetProp( in,  (xmlChar*)"fontsize" );
		if (prop) fromString((const char*)prop, _FontSize);

		prop = (char*) xmlGetProp( in,  (xmlChar*)"shadow" );
		if (prop)
			_Shadow = convertBool(prop);

		prop = (char*) xmlGetProp( in,  (xmlChar*)"shadow_outline" );
		if (prop)
			_ShadowOutline = convertBool(prop);

		prop = (char*) xmlGetProp( in,  (xmlChar*)"formatted" );
		if (prop)
			_Formatted = convertBool(prop);


		// Read sons
		xmlNodePtr cur;
		cur = in->children;
		if (_RootMenu != NULL) delete _RootMenu;
		_RootMenu = new CGroupSubMenu(CViewText::TCtorParam());
		_RootMenu->setId( getId() + ":header" );
		_RootMenu->setSerializable( false );
		_RootMenu->_GroupMenu = this;
		_RootMenu->parse (cur);

		prop = (char*) xmlGetProp( in,  (xmlChar*)"max_visible_line" );
		if (prop)
		{
			sint32 maxVisibleLine;
			fromString((const char*)prop, maxVisibleLine);
			_RootMenu->setMaxVisibleLine(maxVisibleLine);
		}

		if (gmExtended)
		{
			gmExtended->cloneMenu(_RootMenu,  this);
		}
		return true;
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::recurseDraw(CGroupSubMenu *pSubMenu)
	{
		pSubMenu->draw();

	//	const vector<CInterfaceGroup*> &rGroups = pSubMenu->getGroups();

		for (uint32 i = 0; i < pSubMenu->getNumLines(); i++)
		{
			CGroupSubMenu *pGSM = pSubMenu->getSubMenu(i);
			if (pGSM != NULL)
			{
				recurseDraw(pGSM);
				CViewRenderer::getInstance()->flush();
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::draw ()
	{
		if (!_Active) return;

		// TEMP TEMP
		//CViewRenderer &rVR = *CViewRenderer::getInstance();
		//rVR.drawRotFlipBitmap _RenderLayer,  (_XReal,  _YReal,  _WReal,  _HReal,  0,  false,  rVR.getBlankTextureId(),  CRGBA(255, 0, 0, 255) );

		_RootMenu->_Active = true;

		if (SpawnOnMousePos)
			recurseDraw(_RootMenu);
		else
			CGroupModal::draw();
	}

	// ------------------------------------------------------------------------------------------------
	bool CGroupMenu::handleEvent (const NLGUI::CEventDescriptor &event)
	{
		if (!_Active)
			return false;
		return CGroupModal::handleEvent (event);
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceElement* CGroupMenu::getElement (const std::string &id)
	{
		if (id == getId()) return this;
		CInterfaceElement *pIE = _RootMenu->getElement(id);
		if (pIE != NULL)
			return pIE;
		return CGroupModal::getElement(id);
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::setActive (bool state)
	{
		if (SpawnOnMousePos)
		{
			CViewRenderer &rVR = *CViewRenderer::getInstance();
			uint32 w,h;
			rVR.getScreenSize(w,h);
			setW(w);
			setH(h);
			setX(0);
			setY(0);
			_ResizeFromChildH = _ResizeFromChildW = false;
			_RootMenu->_PosRef = Hotspot_TL;
			_RootMenu->_ParentPosRef = Hotspot_BL;
		}

		CGroupFrame::setActive (state);

		// must recompute now the pos of the menu
		uint32 i;
		for (i = 0; i < _ChildrenGroups.size(); ++i)
		{
			_ChildrenGroups[i]->setActive (true);
		}

		CGroupModal::updateCoords();

		// hide sub menus
		_RootMenu->hideSubMenus();
	}

	// ------------------------------------------------------------------------------------------------
	bool CGroupSubMenu::isSeparator(uint i) const
	{
		if (i >= _SubMenus.size())
		{
			nlassert("bad index");
			return false;
		}
		return _Lines[i].Separator != NULL;
	}
	// ------------------------------------------------------------------------------------------------
	bool CGroupMenu::isWindowUnder (sint32 x,  sint32 y)
	{
		for (uint32 i = 0; i < _ChildrenGroups.size(); ++i)
			if (_ChildrenGroups[i]->getActive ())
				if (_ChildrenGroups[i]->isWindowUnder(x, y))
					return true;

		return false;
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::addLine (const string &name,  const string &ah,  const string &params,
							  const std::string &id/*=std::string()*/,
							  const std::string &cond /*= std::string()*/, const std::string &texture,
							  bool checkable /*= false*/,  bool checked /*= false*/
							 )
	{
		if (_RootMenu == NULL)
		{
			_RootMenu = new CGroupSubMenu(CViewText::TCtorParam());
			_RootMenu->_GroupMenu = this;
			_RootMenu->setSerializable( false );
			addGroup (_RootMenu);
		}

		_RootMenu->addLine (name,  ah,  params,  id,  cond,  texture,  checkable,  checked, _Formatted);
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::addLine(const ucstring &name,  const std::string &ah,  const std::string &params,
							 const std::string &id /* = std::string()*/,
							 const std::string &cond /*= std::string()*/, const std::string &texture,
							 bool checkable /*= false*/,  bool checked /*= false*/
							)
	{
		if (_RootMenu == NULL)
		{
			_RootMenu = new CGroupSubMenu(CViewText::TCtorParam());
			_RootMenu->_GroupMenu = this;
			_RootMenu->setSerializable( false );
			addGroup (_RootMenu);
		}
		_RootMenu->addLine (name,  ah,  params,  id,  cond,  texture,  checkable,  checked, _Formatted);
	}
	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::addLineAtIndex(uint index, const ucstring &name, const std::string &ah,
									const std::string &params, const std::string &id /*=std::string()*/,
									const std::string &cond /*=std::string()*/, const std::string &texture,
									bool checkable /*=false*/, bool checked /*=false*/)
	{
		if (_RootMenu == NULL)
		{
			_RootMenu = new CGroupSubMenu(CViewText::TCtorParam());
			_RootMenu->_GroupMenu = this;
			_RootMenu->setSerializable( false );
			addGroup (_RootMenu);
		}
		_RootMenu->addLineAtIndex(index, name,  ah,  params,  id,  cond,  texture,  checkable,  checked, _Formatted);
	}


	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::reset ()
	{
		if ( _RootMenu )
		{
			_RootMenu->reset();
			invalidateCoords();
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::setMinW(sint32 minW)
	{
		if ( _RootMenu )
		{
			_RootMenu->_GroupList->setMinW(minW-_RootMenu->getResizeFromChildWMargin());
			_RootMenu->_GroupList->setW(minW-_RootMenu->getResizeFromChildWMargin());
			_RootMenu->setW(minW-_RootMenu->getResizeFromChildWMargin());
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::setMinH(sint32 minH)
	{
		if ( _RootMenu )
		{
			_RootMenu->_GroupList->setMinH(minH-_RootMenu->getResizeFromChildHMargin());
			_RootMenu->_GroupList->setH(minH-_RootMenu->getResizeFromChildHMargin());
			_RootMenu->setH(minH-_RootMenu->getResizeFromChildHMargin());
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::setGrayedLine(uint line,  bool g)
	{
		if ( _RootMenu )
		{
			_RootMenu->setGrayedLine(line, g);
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::setFontSize(uint fontSize)
	{
		_FontSize = fontSize;
	}

	// ------------------------------------------------------------------------------------------------
	uint CGroupMenu::getNumLine() const
	{
		return _RootMenu ? _RootMenu->getNumLine() : 0;
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::deleteLine(uint index)
	{
		if (index > getNumLine())
		{
			nlwarning("bad index");
			return;
		}
		_RootMenu->removeLine(index);
	}

	// ------------------------------------------------------------------------------------------------
	const std::string CGroupMenu::getActionHandler(uint lineIndex) const
	{
		return _RootMenu ? _RootMenu->getActionHandler(lineIndex) : "";
	}

	// ------------------------------------------------------------------------------------------------
	const std::string CGroupMenu::getActionHandlerParam(uint lineIndex) const
	{
		return _RootMenu ? _RootMenu->getActionHandlerParam(lineIndex) : "";
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::setActionHandler(uint lineIndex, const std::string &ah)
	{
		if (_RootMenu)
			_RootMenu->setActionHandler(lineIndex, ah);
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::setActionHandlerParam(uint lineIndex, const std::string &params)
	{
		if (_RootMenu)
			_RootMenu->setActionHandlerParam(lineIndex, params);
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::setUserGroupRight(uint line, CInterfaceGroup *gr, bool ownerShip /*=true*/)
	{
		if (_RootMenu)
		{
			_RootMenu->setUserGroupRight(line, gr, ownerShip);
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CGroupMenu::setUserGroupLeft(uint line, CInterfaceGroup *gr, bool ownerShip /*=true*/)
	{
		if (_RootMenu)
		{
			_RootMenu->setUserGroupLeft(line, gr, ownerShip);
		}
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupMenu::luaGetRootMenu(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "getRootMenu", 0);
		CLuaIHM::pushUIOnStack(ls, getRootMenu());
		return 1;
	}

	// ------------------------------------------------------------------------------------------------
	int CGroupMenu::luaSetMinW(CLuaState &ls)
	{
		const char *funcName = "setMinW";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		setMinW((sint32) ls.toInteger(1));
		return 0;
	}
}


