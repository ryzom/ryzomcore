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
#include "encyclopedia_manager.h"
#include "interface_manager.h"
#include "nel/gui/group_tree.h"
#include "nel/gui/view_text_id.h"
#include "nel/gui/view_bitmap.h"
#include "action_handler_misc.h"
#include "../sheet_manager.h"

// ***************************************************************************

using namespace NLMISC;
using namespace std;

// ***************************************************************************

CEncyclopediaManager *CEncyclopediaManager::_Instance = NULL;

// ***************************************************************************
void CEncyclopediaManager::releaseInstance()
{
	if( _Instance )
		delete _Instance;
	_Instance = NULL;
}

// ***************************************************************************
CEncyclopediaManager::CEncyclopediaManager()
{
	_AlbumNameSelected = 0;
	_ThemaNameSelected = 0;
	_Initializing =  false;
	_CheckAllFrame = false;
}

// ***************************************************************************
void CEncyclopediaManager::updateAllFrame()
{
	if (_CheckAllFrame)
	{
		if (!isStringWaiting())
		{
			rebuildAlbumList();
			_CheckAllFrame = false;
		}
	}
}

// ***************************************************************************
void CEncyclopediaManager::update(const CEncyclopediaUpdateMsg &msg)
{
	// Update local albums

	switch (msg.Type)
	{
		case CEncyclopediaUpdateMsg::UpdateInit:
			_Initializing = true;
			for (uint32 i = 0; i < msg.AllAlbums.size(); ++i)
				updateAlbum(msg.AllAlbums[i]);

			if (!isStringWaiting())
				rebuildAlbumList();
			else
				_CheckAllFrame = true;

			_Initializing = false;
			break;
		case CEncyclopediaUpdateMsg::UpdateAlbum:
			updateAlbum(msg.Album);

			if (!isStringWaiting())
				rebuildAlbumList();
			else
				_CheckAllFrame = true;

		break;
		case CEncyclopediaUpdateMsg::UpdateThema:
			updateThema(msg.AlbumName, msg.Thema);
		break;
	}

	if ((_AlbumNameSelected != 0) && (_ThemaNameSelected == 0))
		rebuildAlbumPage(_AlbumNameSelected);

	if ((_AlbumNameSelected != 0) && (_ThemaNameSelected != 0))
		rebuildThemaPage(_ThemaNameSelected);
}

// ***************************************************************************
void CEncyclopediaManager::clickOnAlbum(uint32 albumName)
{
	if ((_AlbumNameSelected != albumName) || (_ThemaNameSelected != 0))
	{
		rebuildAlbumPage(albumName);
		_AlbumNameSelected = albumName;
		_ThemaNameSelected = 0;
	}
}

// ***************************************************************************
void CEncyclopediaManager::clickOnThema(uint32 themaName)
{
	if (_ThemaNameSelected != themaName)
	{
		rebuildThemaPage(themaName);
		_ThemaNameSelected = themaName;
	}
}

// ***************************************************************************
void CEncyclopediaManager::updateAlbum(const CEncyMsgAlbum &a)
{
	// Search for album...
	CEncyMsgAlbum *pA = getAlbum(a.Name);
	if (pA == NULL)
	{
		uint32 nBack = (uint32)_Albums.size();
		_Albums.push_back(CEncyMsgAlbum());
		pA = &_Albums[nBack];
	}
	nlassert(pA != NULL);
	pA->Name = a.Name;
	pA->RewardBrick = a.RewardBrick;
	for (uint32 i = 0; i < a.Themas.size(); ++i)
		updateThema(a.Name, a.Themas[i]);
}

// ***************************************************************************
void CEncyclopediaManager::updateThema(uint32 nAlbumName, const CEncyMsgThema &t)
{
	CEncyMsgAlbum *pA = getAlbum(nAlbumName);
	nlassert(pA != NULL);
	CEncyMsgThema *pT = pA->getThema(t.Name);
	// Thema not found add it !
	if (pT == NULL)
	{
		uint32 nBack = (uint32)pA->Themas.size();
		pA->Themas.push_back(CEncyMsgThema());
		pT = &pA->Themas[nBack];

		// If we are not initializing so we must open the encyclopedia on the new thema
		if ( ! _Initializing )
		{
			_AlbumNameSelected = nAlbumName;
			_ThemaNameSelected = t.Name;
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			CInterfaceElement *pContainer = dynamic_cast<CInterfaceElement*>(CWidgetManager::getInstance()->getElementFromId(CONT_ENCY));
			if (pContainer != NULL)
				pContainer->setActive(true);
		}
	}
	nlassert(pT != NULL);
	*pT = t;
}

// ***************************************************************************
CEncyMsgAlbum *CEncyclopediaManager::getAlbum(uint32 nName)
{
	for (uint32 i = 0; i < _Albums.size(); ++i)
		if (_Albums[i].Name == nName)
			return &_Albums[i];
	return NULL;
}

// ***************************************************************************
void CEncyclopediaManager::rebuildAlbumList()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();

	CGroupTree *pTree = dynamic_cast<CGroupTree*>(CWidgetManager::getInstance()->getElementFromId(LIST_ENCY_ALBUM));
	nlassert(pTree != NULL);

	CGroupTree::SNode *pRoot = new CGroupTree::SNode;
	ucstring res;

	// Add all albums
	for (uint32 i = 0; i < _Albums.size(); ++i)
	{
		CGroupTree::SNode *pAlb = new CGroupTree::SNode;
		pAlb->Id = "a_" + toString(i);
		pAlb->AHName = "ency_click_album";
		pAlb->AHParams = toString(_Albums[i].Name);
		if (_Albums[i].Name == _AlbumNameSelected)
			pAlb->Opened = true;
		if (pSMC->getDynString(_Albums[i].Name, res))
			pAlb->Text = res;
		else
			nlwarning("try to construct album without name");

		// Add all themas
		for (uint32 j = 0; j < _Albums[i].Themas.size(); ++j)
		{
			CGroupTree::SNode *pThm = new CGroupTree::SNode;
			pThm->Id = "t_" + toString(i) + "_" + toString(j);
			pThm->AHName = "ency_click_thema";
			pThm->AHParams = toString(_Albums[i].Themas[j].Name);
			if (pSMC->getDynString(_Albums[i].Themas[j].Name, res))
				pThm->Text = res;
			else
				nlwarning("try to construct thema without name");

			pAlb->addChild(pThm);
		}

		pRoot->addChild(pAlb);
	}

	pTree->setRootNode(pRoot);

	// if previously selected album
	if ((_AlbumNameSelected != 0) && (_ThemaNameSelected == 0))
	{
		for (uint32 i = 0; i < _Albums.size(); ++i)
		{
			if (_Albums[i].Name == _AlbumNameSelected)
			{
				pTree->selectNodeById("a_" + toString(i));
				break;
			}
		}
	}
	// if previously selected thema
	if ((_AlbumNameSelected != 0) && (_ThemaNameSelected != 0))
	{
		for (uint32 i = 0; i < _Albums.size(); ++i)
		{
			if (_Albums[i].Name == _AlbumNameSelected)
			{
				for (uint32 j = 0; j < _Albums[i].Themas.size(); ++j)
				{
					if (_Albums[i].Themas[j].Name == _ThemaNameSelected)
					{
						pTree->selectNodeById("t_" + toString(i) + "_" + toString(j));
						break;
					}
				}
			}
		}
	}
}

// ***************************************************************************
void CEncyclopediaManager::rebuildAlbumPage(uint32 albumName)
{
	uint32 i;
	CEncyMsgAlbum *pAlbum = NULL;
	// Select the right album
	for (i = 0; i < _Albums.size(); ++i)
	{
		if (_Albums[i].Name == albumName)
		{
			_AlbumNameSelected = _Albums[i].Name;
			pAlbum = &_Albums[i];
			break;
		}
	}

	if (pAlbum == NULL)
		return;

	// Update the right page
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// Hide and show good group
	CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId(PAGE_ENCY_ALBUM);
	nlassert(pIE != NULL);
	pIE->setActive(true);
	pIE = CWidgetManager::getInstance()->getElementFromId(PAGE_ENCY_HELP);
	pIE->setActive(false);
	pIE = CWidgetManager::getInstance()->getElementFromId(PAGE_ENCY_THEMA);
	pIE->setActive(false);

	// Setup title
	CViewTextID *pVT = dynamic_cast<CViewTextID*>(CWidgetManager::getInstance()->getElementFromId(PAGE_ENCY_ALBUM ":title"));
	nlassert(pVT != NULL);
	pVT->setTextId(pAlbum->Name);

	// Setup brick reward
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:ENCY:ALBUMBRICK:SHEET")->setValue32(pAlbum->RewardBrick);
	CViewText *pRBVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(PAGE_ENCY_ALBUM ":reward:desc"));
	if (pRBVT != NULL)
	{
		STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
		const ucstring desc(pSMC->getSBrickLocalizedDescription(CSheetId(pAlbum->RewardBrick)));
		pRBVT->setText(desc);
	}
}

// ***************************************************************************
void CEncyclopediaManager::rebuildThemaPage(uint32 themaName)
{
	uint32 i;
	CEncyMsgThema *pThema = NULL;
	// Select the right album
	for (i = 0; i < _Albums.size(); ++i)
	{
		pThema = _Albums[i].getThema(themaName);
		if (pThema != NULL)
		{
			_AlbumNameSelected = _Albums[i].Name;
			break;
		}
	}

	if (pThema == NULL)
		return;

	// Update the right page
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// Hide and show good group
	CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId(PAGE_ENCY_ALBUM);
	nlassert(pIE != NULL);
	pIE->setActive(false);
	pIE = CWidgetManager::getInstance()->getElementFromId(PAGE_ENCY_HELP);
	pIE->setActive(false);
	pIE = CWidgetManager::getInstance()->getElementFromId(PAGE_ENCY_THEMA);
	pIE->setActive(true);

	// Setup title
	CViewTextID *pVT = dynamic_cast<CViewTextID*>(CWidgetManager::getInstance()->getElementFromId(PAGE_ENCY_THEMA ":title"));
	nlassert(pVT != NULL);
	pVT->setTextId(pThema->Name);

	// Setup rewards
	pVT = dynamic_cast<CViewTextID*>(CWidgetManager::getInstance()->getElementFromId(PAGE_ENCY_THEMA ":reward_text:desc"));
	nlassert(pVT != NULL);
	pVT->setTextId(pThema->RewardText);

	// Setup brick reward
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:ENCY:REWARDBRICK:SHEET")->setValue32(pThema->RewardSheet);
	CViewText *pRBVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(PAGE_ENCY_THEMA ":reward:desc"));
	nlassert(pRBVT != NULL);
	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
	CEntitySheet *pES = SheetMngr.get(CSheetId(pThema->RewardSheet));
	if (pES != NULL)
	{
		if (pES->type() == CEntitySheet::ITEM)
		{
			const ucstring desc(pSMC->getItemLocalizedDescription(CSheetId(pThema->RewardSheet)));
			pRBVT->setText(desc);
		}
		else if (pES->type() == CEntitySheet::SBRICK)
		{
			const ucstring desc(pSMC->getSBrickLocalizedDescription(CSheetId(pThema->RewardSheet)));
			pRBVT->setText(desc);
		}
		else if (pES->type() == CEntitySheet::SPHRASE)
		{
			const ucstring desc(pSMC->getSPhraseLocalizedDescription(CSheetId(pThema->RewardSheet)));
			pRBVT->setText(desc);
		}
	}

	// Setup the total number of steps
	uint32 nNbSteps = pThema->NbTask - 1; // 0th is the rite
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:ENCY:STEPS")->setValue32(nNbSteps);

	// Count number of tasks done
	uint32 nNbTaskDone = 0;
	for (i = 0; i < pThema->NbTask; ++i)
		if (pThema->getTaskState((uint8)i) == 2) // 2 == finished
			++nNbTaskDone;
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:ENCY:DONE")->setValue32(nNbTaskDone);

	// setup rite & tasks
	for (i = 0; i < pThema->NbTask; ++i)
	{
		string sTmp;
		if (i == 0)
			sTmp = PAGE_ENCY_THEMA ":todo2:rite";
		else
			sTmp = PAGE_ENCY_THEMA ":todo:task" + toString(i);

		// setup task description
		CViewTextID *pText = dynamic_cast<CViewTextID*>(CWidgetManager::getInstance()->getElementFromId(sTmp+":desc"));
		nlassert(pText != NULL);
		pText->setTextId(pThema->TaskName[i]);

		// setup task NPC name
		CStringPostProcessNPCRemoveTitle *pSPPRT = new CStringPostProcessNPCRemoveTitle;
		pIM->addServerID (sTmp+":npc:uc_hardtext", pThema->TaskNPCName[i], pSPPRT);

		// If the task is not known gray it
		if (pThema->getTaskState((uint8)i) == 0)
			pText->setAlpha(80);
		else
			pText->setAlpha(160);

		// If the task is finished toggle it
		CViewBitmap *pBitmap = dynamic_cast<CViewBitmap*>(CWidgetManager::getInstance()->getElementFromId(sTmp+":done"));
		nlassert(pBitmap != NULL);
		if (pThema->getTaskState((uint8)i) == 2)
			pBitmap->setActive(true);
		else
			pBitmap->setActive(false);
	}

}

// ***************************************************************************
// For the group tree we have to check the incoming phrase because no view id stored
bool CEncyclopediaManager::isStringWaiting()
{
	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();

	for (uint32 i = 0; i < _Albums.size(); ++i)
	{
		ucstring res;
		if (!pSMC->getDynString(_Albums[i].Name, res))
			return true;
		for (uint32 j = 0; j < _Albums[i].Themas.size(); ++j)
		{
			if (!pSMC->getDynString(_Albums[i].Themas[j].Name, res))
				return true;
		}
	}

	return false;
}

// ***************************************************************************
// ACTION HANDLERS
// ***************************************************************************

// ***************************************************************************
class CAHEncyClickAlbum : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		nlinfo("ency_click_album called");

		uint32 albumName;
		fromString(Params, albumName);
		CEncyclopediaManager::getInstance()->clickOnAlbum(albumName);
	}
};
REGISTER_ACTION_HANDLER( CAHEncyClickAlbum, "ency_click_album" );

// ***************************************************************************
class CAHEncyClickThema : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		nlinfo("ency_click_thema called");

		uint32 themaName;
		fromString(Params, themaName);
		CEncyclopediaManager::getInstance()->clickOnThema(themaName);
	}
};
REGISTER_ACTION_HANDLER( CAHEncyClickThema, "ency_click_thema" );

// ***************************************************************************
// DEBUG
// ***************************************************************************

NLMISC_COMMAND(testEncyclopedia, "Temp : Simulate the server message","")
{
	CEncyclopediaUpdateMsg msg;
	msg.Type = CEncyclopediaUpdateMsg::UpdateAlbum;

	msg.Album.Name = 654645;
	msg.Album.RewardBrick = CSheetId("bmov00030.sbrick").asInt();
	msg.Album.State = 1;

	CEncyMsgThema thm;
	thm.Name = 3003;
	thm.RewardSheet = CSheetId("bmor00065.sbrick").asInt();
	thm.RewardText = 2002;
	thm.State = 1;

	thm.TaskName[0] = 65455;
	thm.TaskNPCName[0] = 321;
	thm.TaskName[1] = 65466;
	thm.TaskNPCName[1] = 335;
	thm.TaskName[2] = 65488;
	thm.TaskNPCName[2] = 348;
	thm.NbTask = 3;

	msg.Album.Themas.push_back(thm);

	CEncyclopediaManager::getInstance()->update(msg);

	return true;
}
