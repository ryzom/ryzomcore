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
#include "dbgroup_list_sheet_text_share.h"
#include "../client_sheets/sbrick_sheet.h"
#include "nel/misc/xml_auto_ptr.h"
#include "interface_manager.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/ctrl_text_button.h"
#include "../net_manager.h"
#include "../client_sheets/item_sheet.h"

using namespace std;
using namespace NLMISC;


// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheetTextShare, std::string, "list_sheet_share");

CDBGroupListSheetTextShare::CDBGroupListSheetTextShare(const TCtorParam &param)
:	CDBGroupListSheetText(param)
{
	_XWanted = 0;
	_YWanted = 0;
	_XNbMember = 0;
	_YNbMember = 0;
	_XChance = 0;
	_YChance = 0;
	_CheckCoordAccelerated = false;
}


// ***************************************************************************
bool CDBGroupListSheetTextShare::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	if(!CDBGroupListSheetText::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr prop;

	// Parse X/Y
	prop = (char*) xmlGetProp( cur, (xmlChar*)"icon_wanted" );
	if(prop)	_WantedIcon= (const char*)prop;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"icon_not_wanted" );
	if(prop)	_NotWantedIcon= (const char*)prop;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"x_wanted" );
	if(prop)	fromString((const char*)prop, _XWanted);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"y_wanted" );
	if(prop)	fromString((const char*)prop, _YWanted);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"x_nbmember" );
	if(prop)	fromString((const char*)prop, _XNbMember);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"y_nbmember" );
	if(prop)	fromString((const char*)prop, _YNbMember);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"x_chance" );
	if(prop)	fromString((const char*)prop, _XChance);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"y_chance" );
	if(prop)	fromString((const char*)prop, _YChance);

	return true;
}


// ***************************************************************************
CDBGroupListSheetTextShare::CSheetChildShare::CSheetChildShare()
{
	NbMember = NULL;
	Chance = NULL;
	Wanted = NULL;

	CacheNbMember = 0;
	CacheChance = 0;
	CacheWanted = false;
}

// ***************************************************************************
CDBGroupListSheetTextShare::CSheetChildShare::~CSheetChildShare()
{
	NbMember = NULL;
	Chance = NULL;
	Wanted = NULL;
}


// ***************************************************************************
void CDBGroupListSheetTextShare::CSheetChildShare::init(CDBGroupListSheetText *pFather, uint index)
{
	// init my parent
	CSheetChild::init(pFather, index);

	CDBGroupListSheetTextShare	*compoList= (CDBGroupListSheetTextShare*)pFather;
	// get the scrolled list.
	CInterfaceGroup		*parentList= compoList->getList();

	// **** Init the NbMember view
	CViewText *text= new CViewText(CViewBase::TCtorParam());
	text->setId(parentList->getId()+":nb_member"+toString(index));
	text->setParent (parentList);
	text->setParentPos (parentList);
	text->setParentPosRef (Hotspot_TL);
	text->setPosRef (Hotspot_TL);
	text->setActive(true);
	// set text aspect
	text->setFontSize(compoList->getTextTemplate().getFontSize());
	text->setColor(compoList->getTextTemplate().getColor());
	text->setShadow(compoList->getTextTemplate().getShadow());
	text->setShadowOutline(compoList->getTextTemplate().getShadowOutline());
	text->setMultiLine(false);
	text->setModulateGlobalColor(compoList->getTextTemplate().getModulateGlobalColor());
	// Add it to the scrolled list.
	NbMember= text;
	parentList->addView(NbMember);

	// **** Init the Chance view
	text= new CViewText(CViewBase::TCtorParam());
	text->setId(parentList->getId()+":chance"+toString(index));
	text->setParent (parentList);
	text->setParentPos (parentList);
	text->setParentPosRef (Hotspot_TL);
	text->setPosRef (Hotspot_TL);
	text->setActive(true);
	// set text aspect
	text->setFontSize(compoList->getTextTemplate().getFontSize());
	text->setColor(compoList->getTextTemplate().getColor());
	text->setShadow(compoList->getTextTemplate().getShadow());
	text->setShadowOutline(compoList->getTextTemplate().getShadowOutline());
	text->setMultiLine(false);
	text->setModulateGlobalColor(compoList->getTextTemplate().getModulateGlobalColor());
	// Add it to the scrolled list.
	Chance= text;
	parentList->addView(Chance);

	// **** Init the Wanted icon
	Wanted = new CViewBitmap(CViewBase::TCtorParam());
	Wanted->setId(parentList->getId()+":wanted"+toString(index));
	Wanted->setParent (parentList);
	Wanted->setParentPos (parentList);
	Wanted->setParentPosRef (Hotspot_TL);
	Wanted->setPosRef (Hotspot_TL);
	parentList->addView(Wanted);

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf *pNL;
	string sTmp= Ctrl->getSheet()+":NB_MEMBER";
	sTmp = "LOCAL" + sTmp.substr(6,sTmp.size());
	pNL = NLGUI::CDBManager::getInstance()->getDbProp(sTmp, false);
	nlassert(pNL != NULL);
	CurrentNbMember.link ( sTmp.c_str() );

	sTmp= Ctrl->getSheet()+":CHANCE";
	sTmp = "LOCAL" + sTmp.substr(6,sTmp.size());
	pNL = NLGUI::CDBManager::getInstance()->getDbProp(sTmp, false);
	nlassert(pNL != NULL);
	CurrentChance.link ( sTmp.c_str() );

	sTmp= Ctrl->getSheet()+":WANTED";
	sTmp = "LOCAL" + sTmp.substr(6,sTmp.size());
	pNL = NLGUI::CDBManager::getInstance()->getDbProp(sTmp, false);
	nlassert(pNL != NULL);
	CurrentWanted.link ( sTmp.c_str() );
}


// ***************************************************************************
void CDBGroupListSheetTextShare::CSheetChildShare::updateViewText(CDBGroupListSheetText *pFather)
{
	CDBGroupListSheetTextShare	*compoList= (CDBGroupListSheetTextShare*)pFather;

	// common method to update text as item
	updateViewTextAsItem();

	// Update the Number of member of the team that wants the item/phrase
	if(NbMember != NULL)
	{
		NbMember->setActive(true);
		NbMember->setX( compoList->getXNbMember() );
		NbMember->setY( compoList->getYNbMember() - YItem*compoList->getHSlot() );
		NbMember->setText( toString(CacheNbMember) );
	}

	// Update the chance percentage to obtain the item
	if(Chance != NULL)
	{
		Chance->setActive(true);
		Chance->setX( compoList->getXChance() );
		Chance->setY( compoList->getYChance() - YItem*compoList->getHSlot() );
		Chance->setText( toString(CacheChance) );
	}

	if(Wanted != NULL)
	{
		if (CacheWanted)
			Wanted->setTexture(compoList->getWantedIcon());
		else
			Wanted->setTexture(compoList->getNotWantedIcon());

		Wanted->setActive(true);
		Wanted->setX( compoList->getXWanted() );
		Wanted->setY( compoList->getYWanted() - YItem*compoList->getHSlot() );
	}
}

// ***************************************************************************
void CDBGroupListSheetTextShare::CSheetChildShare::hide(CDBGroupListSheetText *pFather)
{
	CSheetChild::hide(pFather);

	// hide additional views
	if(NbMember != NULL) NbMember->setActive(false);
	if(Chance != NULL) Chance->setActive(false);
	if(Wanted != NULL) Wanted->setActive(false);
}

// ***************************************************************************
bool CDBGroupListSheetTextShare::CSheetChildShare::isInvalidated(CDBGroupListSheetText * /* pFather */)
{
	uint8 newNbMember = (uint8)CurrentNbMember.getSInt32();
	uint8 newChance = (uint8)CurrentChance.getSInt32();
	bool newWanted = (CurrentWanted.getSInt32()!=0);

	bool ret = false;

	if ((newNbMember != CacheNbMember) ||
		(newChance != CacheChance) ||
		(newWanted != CacheWanted))
		ret = true;

	return ret;
}

// ***************************************************************************
void CDBGroupListSheetTextShare::CSheetChildShare::update(CDBGroupListSheetText *pFather)
{
	uint8 newNbMember = (uint8)CurrentNbMember.getSInt32();
	uint8 newChance = (uint8)CurrentChance.getSInt32();
	bool newWanted = (CurrentWanted.getSInt32()!=0);
	CacheNbMember = newNbMember;
	CacheChance = newChance;
	CacheWanted = newWanted;
	CSheetChild::update(pFather);
}

// ***************************************************************************
class CHandlerTeamShareChoose : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CBitMemStream out;

		CCtrlTextButton *pTB = dynamic_cast<CCtrlTextButton*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:team_share:content:ok"));
		if (pTB == NULL) return;
		if (pTB->getActive() == false) return;

		uint8 index = 0;
		string strMsgName, localDB;
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(pCaller);

		localDB = pCS->getSheet();
		localDB = localDB.substr(localDB.rfind(':')+1,localDB.size());
		fromString(localDB, index);
		localDB = pCS->getSheet() + ":WANTED";
		localDB = "LOCAL" + localDB.substr(6,localDB.size());

		// Retrieve index
		if (NLGUI::CDBManager::getInstance()->getDbProp(localDB)->getValue8() == 0)
		{
			strMsgName = "TEAM:SHARE_VALID_ITEM";
			NLGUI::CDBManager::getInstance()->getDbProp(localDB)->setValue8(1); // We can do it because it is in local
		}
		else
		{
			strMsgName = "TEAM:SHARE_INVALID_ITEM";
			NLGUI::CDBManager::getInstance()->getDbProp(localDB)->setValue8(0); // We can do it because it is in local
		}

		if (!GenericMsgHeaderMngr.pushNameToStream(strMsgName, out))
		{
			nlwarning ("don't know message name %s", strMsgName.c_str());
		}
		else
		{
			// Serialize item/phrase index
			out.serial(index);
			// Serialize session id
			uint8 sessionid = NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:SHARE:SESSION")->getValue8();
			out.serial(sessionid);
			NetMngr.push (out);
			//nlinfo("impulseCallBack : %s %d %d sent", strMsgName.c_str(), index, sessionid);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerTeamShareChoose, "team_share_choose" );

// ***************************************************************************
class CHandlerTeamShareValid : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CBitMemStream out;
		string strMsgName = "TEAM:SHARE_VALID";

		CCtrlTextButton *pTB = dynamic_cast<CCtrlTextButton*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:team_share:content:ok"));
		if (pTB != NULL)
			pTB->setActive(false);

		if (!GenericMsgHeaderMngr.pushNameToStream(strMsgName, out))
		{
			nlwarning ("don't know message name %s", strMsgName.c_str());
		}
		else
		{
			// Serialize session id
			uint8 sessionid = NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:SHARE:SESSION")->getValue8();
			out.serial(sessionid);
			NetMngr.push (out);
			//nlinfo("impulseCallBack : %s %d sent", strMsgName.c_str(), sessionid);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerTeamShareValid, "team_share_valid" );

