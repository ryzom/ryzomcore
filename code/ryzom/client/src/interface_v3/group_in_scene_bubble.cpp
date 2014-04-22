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
#include "group_in_scene_bubble.h"
#include "interface_manager.h"
#include "skill_manager.h"
#include "../character_cl.h"
#include "nel/gui/action_handler.h"
#include "../entities.h"
#include "nel/gui/group_paragraph.h" // For CCtrlLink
#include "../net_manager.h"
#include "../string_manager_client.h"
#include "../login.h"
#include "../main_loop.h"
#include "../bg_downloader_access.h"

#include "nel/gui/view_text_id.h"

using namespace std;
using namespace NLMISC;

#define RZ_NUM_IS_BUBBLES		4
#define RZ_NUM_POPUP_MESSAGES	5
#define RZ_FADE_TIME			1.0

extern CEntityManager EntitiesMngr;

// ***************************************************************************

void contextHelp (const std::string &name)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// User want context help ?
	if ( (ClientCfg.Local || !IngameDbMngr.initInProgress()) && NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:CONTEXT_HELP")->getValueBool())
	{
		// Look for the context help
		uint index = 0;
		for(;;)
		{
			string defineTarget = name+"_"+toString(index)+"_target";
			string defineUrl = name+"_"+toString(index)+"_url";
			if ( CWidgetManager::getInstance()->getParser()->isDefineExist(defineTarget) &&
				CWidgetManager::getInstance()->getParser()->isDefineExist(defineUrl))
			{
				string target = CWidgetManager::getInstance()->getParser()->getDefine(defineTarget);
				string url = CWidgetManager::getInstance()->getParser()->getDefine(defineUrl);

				CInterfaceElement *elementTarget = CWidgetManager::getInstance()->getElementFromId(target);
				if (elementTarget && elementTarget->getActive())
				{
					// Add the context help
					if(url.empty() == false)
					{
						// Localisation (translation)
						string completeURL = url;
						completeURL = completeURL.substr(0, completeURL.size()-5); // Substract the ".html"
						completeURL += "_" + ClientCfg.getHtmlLanguageCode() + ".html";
						// Add bubble
						InSceneBubbleManager.addContextHelpHTML(completeURL, target,
							CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionTimeoutContextHtml).getValSInt32());
					}

					// Found one help
					break;
				}
			}
			else
				break;

			index++;
		}
	}
}

// ***************************************************************************

CGroupInSceneBubbleManager InSceneBubbleManager;

// ***************************************************************************

std::set<std::string> CGroupInSceneBubbleManager::_IgnoreContextHelp;

// ***************************************************************************
// CGroupInSceneBubbleManager
// ***************************************************************************

// ***************************************************************************
// Helper
void CGroupInSceneBubbleManager::fadeWindow (CInterfaceGroup *group, double timeout)
{
	if (timeout < (TimeInSec+RZ_FADE_TIME))
	{
		// Fade value
		double alpha = 255.0 * (timeout - TimeInSec) / RZ_FADE_TIME;
		clamp (alpha, 0.0, 255.0);
		group->setAlpha((uint8)alpha);
	}
}

// ***************************************************************************
// Helper
bool CGroupInSceneBubbleManager::checkTimeOut(vector<CPopup> &rList)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	bool alignMessage = false;
	for (uint32 i = 0; i < rList.size(); i++)
	{
		// Time out ?
		if (rList[i].timeOut ())
		{
			CWidgetManager::getInstance()->unMakeWindow(rList[i].Group);
			if (rList[i].Group->getParent())
				rList[i].Group->getParent()->delGroup(rList[i].Group);
			else
				delete rList[i].Group;

			// Remove the entry
			rList.erase (rList.begin()+i);
			i--;
			alignMessage = true;
		}
		else
		{
			// Update fade
			fadeWindow (rList[i].Group, rList[i].TimeOut);
		}
	}
	return alignMessage;
}

// ***************************************************************************
// Helper
void CGroupInSceneBubbleManager::alignMessagePopup (vector<CPopup> &rList, bool bCenter)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// Remove if too old messages
	while (rList.size() > RZ_NUM_POPUP_MESSAGES)
	{
		CPopup popup = rList.front();
		rList.erase (rList.begin());
		CWidgetManager::getInstance()->unMakeWindow(popup.Group);
		if (popup.Group->getParent())
			popup.Group->getParent()->delGroup(popup.Group);
		else
			delete popup.Group;
	}

	// First message must be aligned from the screen
	if (!rList.empty())
	{
		CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
		if (pRoot)
		{
			sint i = (sint)rList.size ()-1;

			rList[i].Group->invalidateCoords();
			rList[i].Group->updateCoords();

			sint32 offsetY;
			if (bCenter)
			{
				fromString(CWidgetManager::getInstance()->getParser()->getDefine("popup_pos_y_center"), offsetY);
				offsetY = pRoot->getHReal() - offsetY;
				offsetY -= rList[i].Group->getH();
			}
			else
				fromString(CWidgetManager::getInstance()->getParser()->getDefine("popup_pos_y"), offsetY);

			rList[i].Group->setY(offsetY);
			rList[i].Group->invalidateCoords();
			rList[i].Group->updateCoords();

			offsetY += rList[i].Group->getH();
			i--;

			while(i >= 0)
			{
				// Link to next message
				rList[i].Group->setY(offsetY);
				rList[i].Group->invalidateCoords();
				rList[i].Group->updateCoords();

				offsetY += rList[i].Group->getH();

				i--;
			}
		}
	}
}

// ***************************************************************************

CGroupInSceneBubbleManager::CGroupInSceneBubbleManager()
{
	_CurrentBubble = 0;
	_PopupCount = 0;
}

// ***************************************************************************

void CGroupInSceneBubbleManager::init ()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	uint i;
	for (i=0; i<RZ_NUM_IS_BUBBLES; i++)
	{
		// Id
		string id = "in_scene_bubble_"+toString (i);

		// Create the instance
		std::vector<std::pair<std::string,std::string> > templateParams;
		templateParams.push_back (std::pair<std::string,std::string>("id", id));

		CInterfaceGroup *group = CWidgetManager::getInstance()->getParser()->createGroupInstance ("3dbulle_L",
			"ui:interface", templateParams.empty()?NULL:&(templateParams[0]), (uint)templateParams.size());
		if (group)
		{
			// Link to the interface
			CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", group);
			CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
			group->setParent(pRoot);
			if (pRoot)
				pRoot->addGroup (group);

			CGroupInSceneBubble *bubble = dynamic_cast<CGroupInSceneBubble*>(group);
			if (bubble)
			{
				// Add it
				_Bubbles.push_back(bubble);
				bubble->setActive (false);
			}
		}
	}

	// Register an observer on skill points

}

// ***************************************************************************

void CGroupInSceneBubbleManager::release ()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	uint i;

	// Remove bubbles
	for (i=0; i<_Bubbles.size(); i++)
	if (_Bubbles[i])
	{
		CWidgetManager::getInstance()->unMakeWindow(_Bubbles[i]);
		if (_Bubbles[i]->getParent())
		{
			_Bubbles[i]->getParent()->delGroup(_Bubbles[i]);
		}
		else
		{
			delete _Bubbles[i];
		}
	}
	_Bubbles.clear ();

	// Remove messages
	for (i=0; i<_MessagePopup.size(); i++)
	{
		CWidgetManager::getInstance()->unMakeWindow(_MessagePopup[i].Group);
		if (_MessagePopup[i].Group->getParent())
			_MessagePopup[i].Group->getParent()->delGroup(_MessagePopup[i].Group);
		else
			delete _MessagePopup[i].Group;
	}
	_MessagePopup.clear();

	// Remove messages
	for (i=0; i<_MessagePopupCentered.size(); i++)
	{
		CWidgetManager::getInstance()->unMakeWindow(_MessagePopupCentered[i].Group);
		if (_MessagePopupCentered[i].Group->getParent())
			_MessagePopupCentered[i].Group->getParent()->delGroup(_MessagePopupCentered[i].Group);
		else
			delete _MessagePopupCentered[i].Group;
	}
	_MessagePopupCentered.clear();

	// Remove messages
	for (i=0; i<_BubblePopup.size(); i++)
	{
		CWidgetManager::getInstance()->unMakeWindow(_BubblePopup[i].Group);
		if (_BubblePopup[i].Group->getParent())
			_BubblePopup[i].Group->getParent()->delGroup(_BubblePopup[i].Group);
		else
			delete _BubblePopup[i].Group;
	}
	_BubblePopup.clear();

	_CurrentBubble = 0;
	_PopupCount = 0;
}

// ***************************************************************************

void CGroupInSceneBubbleManager::update ()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	uint i;
	for (i=0; i<_Bubbles.size(); i++)
	{
		// Time out ?
		if (_Bubbles[i]->timeOut ())
		{
			_Bubbles[i]->setActive (false);
			_Bubbles[i]->unlink();
		}

		// Update fade
		fadeWindow (_Bubbles[i], _Bubbles[i]->getTimeout());
	}

	// Do we have to align messages ?
	if (checkTimeOut(_MessagePopup))
		alignMessagePopup (_MessagePopup, false);

	if (checkTimeOut(_MessagePopupCentered))
		alignMessagePopup (_MessagePopupCentered, true);

	for (i=0; i<_BubblePopup.size(); i++)
	{
		// Time out ?
		if (_BubblePopup[i].timeOut ())
		{
			CWidgetManager::getInstance()->unMakeWindow(_BubblePopup[i].Group);
			if (_BubblePopup[i].Group->getParent())
				_BubblePopup[i].Group->getParent()->delGroup(_BubblePopup[i].Group);
			else
				delete _BubblePopup[i].Group;

			// Remove the entry
			_BubblePopup.erase (_BubblePopup.begin()+i);
			i--;
		}
		else
		{
			if (!_BubblePopup[i].Target.empty())
			{
				// Get the target
				CInterfaceElement *target = CWidgetManager::getInstance()->getElementFromId(_BubblePopup[i].Target);
				if (target)
				{
					// Target is good ?
					if (!target->getActive() ||
						(target->getXReal() != _BubblePopup[i].TargetX) ||
						(target->getYReal() != _BubblePopup[i].TargetY) ||
						(target->getWReal() != _BubblePopup[i].TargetW) ||
						(target->getHReal() != _BubblePopup[i].TargetH))
					{
						CWidgetManager::getInstance()->unMakeWindow(_BubblePopup[i].Group);
						if (_BubblePopup[i].Group->getParent())
							_BubblePopup[i].Group->getParent()->delGroup(_BubblePopup[i].Group);
						else
							delete _BubblePopup[i].Group;

						// Remove the entry
						_BubblePopup.erase (_BubblePopup.begin()+i);
						i--;
						continue;
					}
				}

			}

			// Update fade
			fadeWindow (_BubblePopup[i].Group, _BubblePopup[i].TimeOut);
		}
	}

	// Some group to delete ?
	for (i=0; i<_GroupToDelete.size(); i++)
	{
		uint j;
		for (j=0; j<_BubblePopup.size(); j++)
		{
			if (_BubblePopup[j].Group == _GroupToDelete[i])
			{
				CWidgetManager::getInstance()->unMakeWindow(_BubblePopup[j].Group);
				if (_BubblePopup[j].Group->getParent())
					_BubblePopup[j].Group->getParent()->delGroup(_BubblePopup[j].Group);
				else
					delete _BubblePopup[j].Group;

				// Remove the entry
				_BubblePopup.erase (_BubblePopup.begin()+j);
				j--;
			}
		}
		for (j=0; j<_DynBubbles.size(); j++)
		{
			if (_DynBubbles[j].Bubble == _GroupToDelete[i])
			{
				//nlinfo ("dynchat bubble erase BotUID:%d",_DynBubbles[j].BotUID);

				// Unlink from character
				CEntityCL *pEntity = EntitiesMngr.getEntityByCompressedIndex(_DynBubbles[j].BotUID);
				CCharacterCL *pChar = dynamic_cast<CCharacterCL*>(pEntity);
				if (pChar != NULL)
					pChar->setBubble(NULL);

				CWidgetManager::getInstance()->unMakeWindow(_DynBubbles[j].Bubble);
				if (_DynBubbles[j].Bubble->getParent())
					_DynBubbles[j].Bubble->getParent()->delGroup(_DynBubbles[j].Bubble);
				else
					delete _DynBubbles[j].Bubble;

				_DynBubbles.erase(_DynBubbles.begin()+j);
				j--;
			}
		}
	}
	_GroupToDelete.clear();

	// DynChat bubbles to update ?
	for (i = 0; i < _DynBubbles.size(); ++i)
	{
		if (_DynBubbles[i].DescWaiting != 0)
		{
			ucstring res;
			STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
			if (pSMC->getDynString(_DynBubbles[i].DescWaiting,res))
			{
				//nlinfo ("dynchat bubble setText BotUID:%d",_DynBubbles[i].BotUID);

				_DynBubbles[i].DescWaiting = 0;
				_DynBubbles[i].Bubble->setText(res);
				_DynBubbles[i].Bubble->setActive(true);

				if (_DynBubbles[i].Bubble->isOnLastPage())
					_DynBubbles[i].displayOptions(true);

				// Send to the around me window
				// TODO must get the name of the bot etc...
				/*
				ucstring finalString = res;
				for(;;)
				{
					std::string::size_type index = finalString.find (ucstring("{break}"));
					if (index == ucstring::npos) break;
					finalString = finalString.substr (0, index) + finalString.substr(index+7,finalString.size());
				}

				PeopleInterraction.ChatInput.AroundMe.displayMessage(finalString);*/
			}
		}
	}
}

// ***************************************************************************

CGroupInSceneBubble *CGroupInSceneBubbleManager::newBubble (const ucstring &text)
{
	if (!text.empty() && _Bubbles.size ())
	{
		// Get a bubble
		CGroupInSceneBubble *bubble = _Bubbles[_CurrentBubble];
		if (bubble)
		{
			// Next bubble
			_CurrentBubble++;
			_CurrentBubble %= _Bubbles.size();

			// Reset the bubble
			bubble->setText (text);
			bubble->setAlpha(255);

			return bubble;
		}
	}
	return NULL;
}

// ***************************************************************************

void CGroupInSceneBubbleManager::addSkillPopup (uint skillId, sint delta, uint time)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CSkillManager	  *pSM = CSkillManager::getInstance();

	// Create a skill popup
	string id = "skill_popup_"+toString(_PopupCount++);

	// Create the instance
	std::vector<std::pair<std::string,std::string> > templateParams;
	templateParams.push_back (std::pair<std::string,std::string>("id", id));
	templateParams.push_back (std::pair<std::string,std::string>("skillid", toString(skillId)));
	templateParams.push_back (std::pair<std::string,std::string>("delta", toString(delta)));

	CInterfaceGroup *group = CWidgetManager::getInstance()->getParser()->createGroupInstance ("skill_popup",
		"ui:interface", templateParams.empty()?NULL:&(templateParams[0]), (uint)templateParams.size());
	if (group)
	{
		// Skill name
		const ucstring sSkillName(STRING_MANAGER::CStringManagerClient::getSkillLocalizedName((SKILLS::ESkills)skillId));
		CViewText *pViewSkillName = dynamic_cast<CViewText*>(group->getView("name"));
		if (pViewSkillName != NULL)
			pViewSkillName->setText (sSkillName);

		// Skill value
		CCDBNodeLeaf *skillLeaf = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SKILLS:"+toString(skillId)+":BaseSKILL", false);
		if (skillLeaf)
		{
			pViewSkillName = dynamic_cast<CViewText*>(group->getView("lvl"));
			if (pViewSkillName != NULL)
				pViewSkillName->setText (toString(skillLeaf->getValue32()));
		}

		// Delta
		pViewSkillName = dynamic_cast<CViewText*>(group->getView("delta"));
		if (pViewSkillName != NULL)
			pViewSkillName->setText (toString("%+d",delta));

		// MaxValue
		CViewText *pViewSkillMax = dynamic_cast<CViewText*>(group->getView("max"));
		if (pViewSkillMax != NULL)
			pViewSkillMax->setText (toString(pSM->getMaxSkillValue((SKILLS::ESkills)skillId)));

		// Link to the interface
		CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", group);
		CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
		group->setParent(pRoot);
		if (pRoot)
			pRoot->addGroup (group);

		group->setActive (true);

		// Add the skill popup
		CPopup popup;
		popup.Group = group;
		popup.TimeOut = TimeInSec + time;
		_MessagePopup.push_back(popup);
	}

	// Align messages
	alignMessagePopup (_MessagePopup, false);
}

// ***************************************************************************

void CGroupInSceneBubbleManager::addMessagePopup (const ucstring &message, CRGBA color, uint time)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// default timeout?
	if(time==0)
		time=CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionTimeoutMessages).getValSInt32();

	// Create a skill popup
	string id = "message_popup_"+toString(_PopupCount++);

	// Create the instance
	std::vector<std::pair<std::string,std::string> > templateParams;
	templateParams.push_back (std::pair<std::string,std::string>("id", id));

	CInterfaceGroup *group = CWidgetManager::getInstance()->getParser()->createGroupInstance ("message_popup",
		"ui:interface", templateParams.empty()?NULL:&(templateParams[0]), (uint)templateParams.size());
	if (group)
	{
		// Skill name
		CViewText *pViewName = dynamic_cast<CViewText*>(group->getView("name"));
		if (pViewName != NULL)
		{
			pViewName->setText (message);
			pViewName->setColor (color);
		}

		// Link to the interface
		CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", group);
		CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
		group->setParent(pRoot);
		if (pRoot)
			pRoot->addGroup (group);

		group->setActive (true);

		// Add the skill popup
		CPopup popup;
		popup.Group = group;
		popup.TimeOut = TimeInSec + time;
		_MessagePopup.push_back(popup);
	}

	// Align messages
	alignMessagePopup (_MessagePopup, false);
}

// ***************************************************************************

void CGroupInSceneBubbleManager::addMessagePopupCenter (const ucstring &message, CRGBA color, uint time)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// default timeout?
	if(time==0)
		time= CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionTimeoutMessages).getValSInt32();

	// Create a skill popup
	string id = "message_popup_"+toString(_PopupCount++);

	// Create the instance
	std::vector<std::pair<std::string,std::string> > templateParams;
	templateParams.push_back (std::pair<std::string,std::string>("id", id));

	CInterfaceGroup *group = CWidgetManager::getInstance()->getParser()->createGroupInstance ("message_popup_center",
		"ui:interface", templateParams.empty()?NULL:&(templateParams[0]), (uint)templateParams.size());
	if (group)
	{
		// Skill name
		CViewText *pViewName = dynamic_cast<CViewText*>(group->getView("name"));
		if (pViewName != NULL)
		{
			pViewName->setTextFormatTaged(message);
			pViewName->setColor (color);
		}

		// Link to the interface
		CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", group);
		CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
		group->setParent(pRoot);
		if (pRoot)
			pRoot->addGroup (group);

		group->setActive (true);

		// Add the skill popup
		CPopup popup;
		popup.Group = group;
		popup.TimeOut = TimeInSec + time;
		_MessagePopupCentered.push_back(popup);
	}

	// Align messages
	alignMessagePopup (_MessagePopupCentered, true);
}

// ***************************************************************************

CGroupInSceneBubbleManager::CPopupContext *CGroupInSceneBubbleManager::buildContextHelp (const string &templateName, const string &targetName, CInterfaceElement *&target, uint time)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	string v="m";
	string h="m";
	target = CWidgetManager::getInstance()->getElementFromId(targetName);
	if (target)
	{
		// Find a position
		NL3D::UDriver *Driver = CViewRenderer::getInstance()->getDriver();
		const uint width = Driver->getWindowWidth();
		const uint height = Driver->getWindowHeight();
		h = (target->getXReal() < ((sint)width-target->getXReal()-target->getWReal()))?"l":"r";
		v = (target->getYReal() < ((sint)height-target->getYReal()-target->getHReal()))?"b":"t";
		target->setActive(true);

		// Look for a parent container
		CInterfaceGroup *parent = target->getParent();
		while (parent)
		{
			if (parent->getParent() && (parent->getParent()->getId() == "ui:interface"))
			{
				parent->setActive(true);
				break;
			}

			parent = parent->getParent();
		}
	}

	// Id
	string id = "context_help_"+toString(_PopupCount++);

	// Create the instance
	std::vector<std::pair<std::string,std::string> > templateParams;
	templateParams.push_back (std::pair<std::string,std::string>("id", id));

	CInterfaceGroup *group = CWidgetManager::getInstance()->getParser()->createGroupInstance (templateName+v+h,
		"ui:interface", templateParams.empty()?NULL:&(templateParams[0]), (uint)templateParams.size());
	if (group)
	{
		// Target available ?
		if (target)
		{
			group->setParentPos(target);
			group->setPosRef((h=="r")?(v=="t")?Hotspot_TR:Hotspot_BR:(v=="t")?Hotspot_TL:Hotspot_BL);
			group->setParentPosRef((h=="l")?(v=="b")?Hotspot_TR:Hotspot_BR:(v=="b")?Hotspot_TL:Hotspot_BL);
			group->setX((h=="r")?16:-16);
			group->setY((v=="t")?16:-16);
		}

		// Link to the interface
		CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", group);
		CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
		group->setParent(pRoot);
		if (pRoot)
			pRoot->addGroup (group);

		// To create the list
		group->updateCoords();

		// Add the popup
		CPopupContext popup;
		popup.Group = group;
		popup.TimeOut = TimeInSec + time;
		if (target)
		{
			// Backup bubble target
			target->updateCoords();
			popup.Target = target->getId();
			popup.TargetX = target->getXReal();
			popup.TargetY = target->getYReal();
			popup.TargetW = target->getWReal();
			popup.TargetH = target->getHReal();
		}
		_BubblePopup.push_back(popup);
		return &_BubblePopup.back();
	}
	return NULL;
}

// ***************************************************************************

void CGroupInSceneBubbleManager::addContextHelp (const ucstring &message, const string &targetName, uint time)
{
	ucstring finalMessage = message;
	CInterfaceElement *target;
	CPopupContext *context = CGroupInSceneBubbleManager::buildContextHelp ("context_help_", targetName, target, time);
	if (context)
	{
		CViewBase *view = context->Group->getView("text");
		if (view)
		{
			CViewText *text = dynamic_cast<CViewText*>(view);
			if (text)
			{
				// All text empty ?
				if (finalMessage.empty() && target)
				{
					CGroupContainer	*container = dynamic_cast<CGroupContainer*>(target);
					if (container)
						finalMessage = container->getTitle();
					if (finalMessage.empty())
					{
						// Try the tool tip
						CCtrlBase *ctrlBase = dynamic_cast<CCtrlBase*>(target);
						if (ctrlBase)
						{
							ctrlBase->getContextHelp(finalMessage);
						}
					}
				}

				text->setText(finalMessage);
			}
		}
		context->Group->setActive(true);
	}
}

// ***************************************************************************

void CGroupInSceneBubbleManager::addContextHelpHTML (const string &url, const string &targetName, uint time)
{
	// Ignore this
	if (_IgnoreContextHelp.find (url) == _IgnoreContextHelp.end())
	{
		CInterfaceElement *target;
		CPopupContext *context = CGroupInSceneBubbleManager::buildContextHelp ("context_help_html_", targetName, target, time);
		if (context)
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			CAHManager::getInstance()->runActionHandler("browse", NULL, "name="+context->Group->getId()+":header_opened:window:html|url="+url);

			// Add the URL
			context->Url = url;
		}
	}
}

// ***************************************************************************

void CGroupInSceneBubbleManager::ignoreContextHelp (CInterfaceGroup *groupToRemove)
{
	uint i;
	for (i=0; i<_BubblePopup.size(); i++)
	{
		if (_BubblePopup[i].Group == groupToRemove)
		{
			// Delete it
			_GroupToDelete.push_back(_BubblePopup[i].Group);

			// Ignore it
			_IgnoreContextHelp.insert (_BubblePopup[i].Url);
			return;
		}
	}
}

// ***************************************************************************

void CGroupInSceneBubbleManager::chatOpen (uint32 nUID, const ucstring &ucsText, uint bubbleTimer)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	CCharacterCL *pChar = dynamic_cast<CCharacterCL*>(EntitiesMngr.getEntityByCompressedIndex(nUID));
	if (pChar == NULL || nUID==CLFECOMMON::INVALID_CLIENT_DATASET_INDEX) return;
	if (bubbleTimer == 0) bubbleTimer = CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionTimeoutBubbles).getValSInt32();

	// Output the message in a bubble

	bool show = false;
	if (pChar->isUser())
		show = NLGUI::CDBManager::getInstance()->getDbProp ("UI:SAVE:INSCENE:USER:MESSAGES")->getValueBool();
	else if (pChar->isFriend())
		show = NLGUI::CDBManager::getInstance()->getDbProp ("UI:SAVE:INSCENE:FRIEND:MESSAGES")->getValueBool();
	else
		show = NLGUI::CDBManager::getInstance()->getDbProp ("UI:SAVE:INSCENE:ENEMY:MESSAGES")->getValueBool();

	if (show)
	{
		// Check that we can create the new bubble (if a dynamic bubble is already present do not replace it !
		CGroupInSceneBubble *pCharBubble = pChar->getBubble();
		if (pCharBubble != NULL)
			if (strnicmp(pCharBubble->getId().c_str(), "ui:interface:in_scene_dyn_bubble", 32) == 0)
				return;

		// Get a bubble
		CGroupInSceneBubble *bubble = newBubble (ucsText);
		if (bubble)
		{
			// Link the bubble
			bubble->link (pChar, bubbleTimer);
			bubble->setActive (true);
			pChar->setBubble(bubble);
		}
	}
}

// ***************************************************************************

void CGroupInSceneBubbleManager::dynChatOpen (uint32 nBotUID, uint32 nBotName, const vector<uint32> &DynStrs)
{
	nlassert( (DynStrs.size() >= 1) && (DynStrs.size() <= 9));
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// If the character doesn't exist in view field -> do not display the bubble

	CEntityCL *pEntity = EntitiesMngr.getEntityByCompressedIndex(nBotUID);
	if (ClientCfg.Local) pEntity = EntitiesMngr.entity(1);
	CCharacterCL *pChar = dynamic_cast<CCharacterCL*>(pEntity);
	if (pChar == NULL)
	{
		nlwarning("character probably too far");
		return;
	}

	// Look if we get a bubble with this nBotUID ?

	uint32 pos, j;
	for (pos = 0; pos < _DynBubbles.size(); ++pos)
	{
		if (_DynBubbles[pos].BotUID == nBotUID)
			break;
	}

	// If the bubble doesn't exist -> create
	CGroupInSceneBubble *bubble = NULL;
	string id;
	if (pos == _DynBubbles.size())
	{
		uint32 i = 0;
		while (getDynBubble(i) != NULL) i++;
		id = "in_scene_dyn_bubble_" + toString(i);
		// Create the instance
		std::vector<std::pair<std::string,std::string> > templateParams;
		templateParams.push_back (std::pair<std::string,std::string>("id", id));

		CInterfaceGroup *group = CWidgetManager::getInstance()->getParser()->createGroupInstance ("dyn_3dbulle_L", "ui:interface", templateParams);
		if (group == NULL)
		{
			nlwarning("cannot create dyn_3dbulle_L");
			return;
		}
		// Link to the interface
		CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", group);
		CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
		group->setParent(pRoot);
		if (pRoot)
			pRoot->addGroup (group);
		group->setActive(false);

		bubble = dynamic_cast<CGroupInSceneBubble*>(group);
		if (bubble == NULL)
		{
			nlwarning("cannot cast to CGroupInSceneBubble");
			return;
		}
		CDynBubble dynBubble;
		dynBubble.BotName = nBotName;
		dynBubble.BotUID = nBotUID;
		dynBubble.Bubble = bubble;
		_DynBubbles.push_back(dynBubble);
	}
	else
	{
		bubble = _DynBubbles[pos].Bubble;
		// Remove from group to delete if in the same frame
		for (j=0; j<_GroupToDelete.size(); j++)
		if (_GroupToDelete[j] == bubble)
		{
			_GroupToDelete.erase(_GroupToDelete.begin()+j);
			break;
		}
	}

	// Update the bubble's texts

	id = bubble->getId() + ":header_opened:window:";
	CViewTextID *pVT;
	CCtrlLink *pCL;

	_DynBubbles[pos].DescWaiting = DynStrs[0];

	for (j = 0; j < 8; ++j)
	{
		pVT = dynamic_cast<CViewTextID*>(bubble->getElement(id+"opt"+toString(j)));
		if (pVT != NULL)
		{
			pVT->setActive(false);
			pVT->setTextId(0);
		}
		pCL = dynamic_cast<CCtrlLink*>(bubble->getElement(id+"optb"+toString(j)));
		if (pCL != NULL) pCL->setActive(false);
	}

	for (j = 0; j < (DynStrs.size() -1); ++j)
	{
		pVT = dynamic_cast<CViewTextID*>(bubble->getElement(id+"opt"+toString(j)));
		if (pVT != NULL)
		{
			pVT->setActive(true);
			pVT->setTextId(DynStrs[j+1]);
			pCL = dynamic_cast<CCtrlLink*>(bubble->getElement(id+"optb"+toString(j)));
			if (pCL != NULL) pCL->setActive(true);
		}
	}
	_DynBubbles[pos].displayOptions(false);

	// Link bubble to the character
	pChar->setBubble(bubble);

	// Make the npc face the character
	UserEntity->interlocutor( pChar->slot() );
}

// ***************************************************************************

void CGroupInSceneBubbleManager::webIgChatOpen (uint32 nBotUID, string text, const vector<string> &strs, const vector<string> &links)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	// If the character doesn't exist in view field -> do not display the bubble

	CEntityCL *pEntity = EntitiesMngr.getEntityByCompressedIndex(nBotUID);
	CCharacterCL *pChar = dynamic_cast<CCharacterCL*>(pEntity);
	if (pChar == NULL)
	{
		nlwarning("character probably too far");
		return;
	}

	// Look if we get a bubble with this nBotUID ?
	uint32 pos, j;
	for (pos = 0; pos < _DynBubbles.size(); ++pos)
	{
		if (_DynBubbles[pos].BotUID == nBotUID)
			break;
	}

	// If the bubble doesn't exist -> create
	CGroupInSceneBubble *bubble = NULL;
	string id;
	if (pos == _DynBubbles.size())
	{
		uint32 i = 0;
		while (getDynBubble(i) != NULL) i++;
		id = "in_scene_webig_bubble_" + toString(nBotUID);
		// Create the instance
		std::vector<std::pair<std::string,std::string> > templateParams;
		templateParams.push_back (std::pair<std::string,std::string>("id", id));

		CInterfaceGroup *group = CWidgetManager::getInstance()->getParser()->createGroupInstance ("webig_3dbulle_L", "ui:interface", templateParams);
		if (group == NULL)
		{
			nlwarning("cannot create webig_3dbulle_L");
			return;
		}
		// Link to the interface
		CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", group);
		CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
		group->setParent(pRoot);
		if (pRoot)
			pRoot->addGroup (group);
		group->setActive(false);

		bubble = dynamic_cast<CGroupInSceneBubble*>(group);
		if (bubble == NULL)
		{
			nlwarning("cannot cast to CGroupInSceneBubble");
			return;
		}
		CDynBubble dynBubble;
		dynBubble.BotName = 0;
		dynBubble.BotUID = nBotUID;
		dynBubble.Bubble = bubble;
		_DynBubbles.push_back(dynBubble);
	}
	else
	{
		bubble = _DynBubbles[pos].Bubble;
		// Remove from group to delete if in the same frame
		for (j=0; j<_GroupToDelete.size(); j++)
		if (_GroupToDelete[j] == bubble)
		{
			_GroupToDelete.erase(_GroupToDelete.begin()+j);
			break;
		}
	}

	// Update the bubble's texts

	ucstring ucText;
	ucText.fromUtf8(text);
	bubble->setText(ucText);
	id = bubble->getId() + ":header_opened:window:";
	CViewText *pVT;
	CCtrlLink *pCL;

	_DynBubbles[pos].DescWaiting = 0;

	for (j = 0; j < 8; ++j)
	{
		pVT = dynamic_cast<CViewText*>(bubble->getElement(id+"opt"+toString(j)));
		if (pVT != NULL)
		{
			pVT->setActive(false);
			pVT->setText(ucstring(""));
		}
		pCL = dynamic_cast<CCtrlLink*>(bubble->getElement(id+"optb"+toString(j)));
		if (pCL != NULL) pCL->setActive(false);
	}

	for (j = 0; j < strs.size(); ++j)
	{
		pVT = dynamic_cast<CViewText*>(bubble->getElement(id+"opt"+toString(j)));
		if (pVT != NULL)
		{
			pVT->setActive(true);
			ucstring optionText;
			optionText.fromUtf8(strs[j]);
			pVT->setText(optionText);
			pCL = dynamic_cast<CCtrlLink*>(bubble->getElement(id+"optb"+toString(j)));
			if (pCL != NULL)
			{
				pCL->setActionOnLeftClick("browse");
				pCL->setParamsOnLeftClick("name=ui:interface:web_transactions:content:html|show=0|url="+links[j]);
				pCL->setActive(true);

			}
		}
	}

	// Link bubble to the character
	pChar->setBubble(bubble);

	// Make the npc face the character
	UserEntity->interlocutor( pChar->slot() );
}


uint32 CGroupInSceneBubbleManager::CDynBubble::getOptionStringId(uint option)
{
	if (!Bubble) return 0;
	std::string id = Bubble->getId() + ":header_opened:window:";
	CViewTextID *pVT = dynamic_cast<CViewTextID*>(Bubble->getElement(id + "opt" + toString(option)));
	if (!pVT) return 0;
	return pVT->getTextId();
}



#if !FINAL_VERSION

// TEMP TEMP TEMP
NLMISC_COMMAND(testDynChatOpen1, "", "")
{
	CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());
	if (selection != NULL)
	{
		vector<uint32> DescAndOptions;
		DescAndOptions.push_back(654);
		DescAndOptions.push_back(478);
		DescAndOptions.push_back(13598);
		InSceneBubbleManager.dynChatOpen (selection->dataSetId(), 315,  DescAndOptions);
	}
	return true;
}

NLMISC_COMMAND(testDynChatOpen2, "", "")
{
	CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());
	if (selection != NULL)
	{
		vector<uint32> DescAndOptions;
		DescAndOptions.push_back(654);
		DescAndOptions.push_back(13598);
		InSceneBubbleManager.dynChatOpen (selection->dataSetId(), 315,  DescAndOptions);
	}
	return true;
}

#endif

// ***************************************************************************

void CGroupInSceneBubbleManager::dynChatClose (uint32 nBotUID)
{
	// Look if we get a bubble with this nBotUID ?
	uint32 pos;
	for (pos = 0; pos < _DynBubbles.size(); ++pos)
	{
		if (_DynBubbles[pos].BotUID == nBotUID)
			break;
	}
	if (pos != _DynBubbles.size())
	{
		_GroupToDelete.push_back(_DynBubbles[pos].Bubble);
	}

	// The NPC now won't have to face the user entity anymore
	// Note: if we receive the impulsion close after a new impulsion open, the NPC could fail to
	// turn towards the user entity if there was no frame in between (because the turning is done
	// by CUserEntity::updatePreCamera()).
	UserEntity->interlocutor( CLFECOMMON::INVALID_SLOT );
}

// ***************************************************************************
CGroupInSceneBubbleManager::CDynBubble *CGroupInSceneBubbleManager::getDynBubble(uint32 nDynBubbleNb)
{
	uint32 value;
	for (uint32 i = 0; i < _DynBubbles.size(); ++i)
		if (_DynBubbles[i].Bubble != NULL)
		{
			string id = _DynBubbles[i].Bubble->getId();
			id = id.substr(33, id.size());
			fromString(id, value);
			if (value == nDynBubbleNb)
				return &_DynBubbles[i];
		}
	return NULL;
}

// ***************************************************************************

void CGroupInSceneBubbleManager::dynChatNext (uint32 nBubbleNb)
{
	CDynBubble *pDB = getDynBubble(nBubbleNb);
	if (pDB != NULL)
		pDB->next();
	else
		nlwarning("cannot get dyn bubble %d", nBubbleNb);
}

// ***************************************************************************

void CGroupInSceneBubbleManager::dynChatSkip (uint32 nBubbleNb)
{
	CDynBubble *pDB = getDynBubble(nBubbleNb);
	if (pDB != NULL)
		pDB->skip();
	else
		nlwarning("cannot get dyn bubble %d", nBubbleNb);
}

// ***************************************************************************
uint32 CGroupInSceneBubbleManager::dynChatGetBotUID (uint32 nBubbleNb)
{
	CDynBubble *pDB = getDynBubble(nBubbleNb);
	if (pDB != NULL)
		return pDB->BotUID;

	nlwarning("cannot get dyn bubble %d", nBubbleNb);
	return 0;
}

// ***************************************************************************
uint32 CGroupInSceneBubbleManager::dynChatGetOptionStringId(uint32 nBubbleNb, uint option)
{
	CDynBubble *pDB = getDynBubble(nBubbleNb);
	if (pDB == NULL) return 0;
	return pDB->getOptionStringId(option);
}

// ***************************************************************************

void CGroupInSceneBubbleManager::CDynBubble::displayOptions (bool bShow)
{
	CViewTextID *pVT;
	CCtrlLink *pCL;

	string id = Bubble->getId() + ":header_opened:window:";

	for (uint32 j = 0; j < 8; ++j)
	{
		pVT = dynamic_cast<CViewTextID*>(Bubble->getElement(id+"opt"+toString(j)));
		if (pVT != NULL)
		{
			if (pVT->getTextId() != 0)
			{
				pVT->setActive(bShow);
				pCL = dynamic_cast<CCtrlLink*>(Bubble->getElement(id+"optb"+toString(j)));
				if (pCL != NULL) pCL->setActive(bShow);
			}
		}
	}
}

// ***************************************************************************

void CGroupInSceneBubbleManager::CDynBubble::next()
{
	Bubble->next();
	if (Bubble->isOnLastPage())
		displayOptions(true);
}

// ***************************************************************************

void CGroupInSceneBubbleManager::CDynBubble::skip()
{
	while (!Bubble->isOnLastPage())
		Bubble->next();
	displayOptions(true);
	// Count the number of options
	uint32 nNbOptions = 0;
	CViewTextID *pVT;
	string id = Bubble->getId() + ":header_opened:window:";
	for (uint32 j = 0; j < 8; ++j)
	{
		pVT = dynamic_cast<CViewTextID*>(Bubble->getElement(id+"opt"+toString(j)));
		if ((pVT != NULL) && (pVT->getTextId() != 0))
			nNbOptions++;
	}
	// If only one option validate it
	if (nNbOptions == 1)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CAHManager::getInstance()->runActionHandler("dynchat_click_option", Bubble, "0");
	}
}

// ***************************************************************************

class CAHDynChatClickOption : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		if (pCaller == NULL) return;
		// Get the bot UID
		string id = pCaller->getId();
		id = id.substr(33, id.size());
		id = id.substr(0, id.find(':'));
		sint32 nBubbleNb;
		fromString(id, nBubbleNb);
		uint32 nBotUID = InSceneBubbleManager.dynChatGetBotUID(nBubbleNb);
		uint8 nOpt;
		fromString(Params, nOpt);
		uint32 optStrId = InSceneBubbleManager.dynChatGetOptionStringId(nBubbleNb, nOpt);
		if (!optStrId) return;

		if (isBGDownloadEnabled())
		{
			STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
			ucstring result;
			if (!pSMC->getDynString(optStrId, result))
			{
				return; // shouldn't happen since the button isn't visible as long as the text has not been received ...
			}
			static volatile bool forceWarning = false; // for debug
			ucstring::size_type	pos= result.find(ucstring("{ros_exit}"));
			if(pos != ucstring::npos || forceWarning)
			{
				if (AvailablePatchs != 0)
				{
					inGamePatchUncompleteWarning();
					return;
				}
			}
		}

		const string sMsg = "BOTCHAT:DYNCHAT_SEND";
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
		{
			out.serial(nBotUID);
			out.serial(nOpt);
			NetMngr.push(out);
			//nlinfo("impulseCallBack : %s %d %d sent", sMsg.c_str(), nBotUID, nOpt);
		}
		else
		{
			nlwarning("unknown message name '%s'", sMsg.c_str());
		}

		InSceneBubbleManager.dynChatClose(nBotUID);
	}
};
REGISTER_ACTION_HANDLER( CAHDynChatClickOption, "dynchat_click_option" );

// ***************************************************************************

void CGroupInSceneBubbleManager::serialInSceneBubbleInfo(NLMISC::IStream &f)
{
	f.serialCont(_IgnoreContextHelp);
}


// ***************************************************************************
// CGroupInSceneBubble
// ***************************************************************************

// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CGroupInSceneBubble, std::string, "in_scene_bubble");

CGroupInSceneBubble::CGroupInSceneBubble(const TCtorParam &param)
:	CGroupInScene(param)
{
	_Character = NULL;
	_CanBeShown = false;
	_ZBias= ClientCfg.BubbleZBias;
}

// ***************************************************************************

CGroupInSceneBubble::~CGroupInSceneBubble()
{
}

// ***************************************************************************

bool CGroupInSceneBubble::timeOut () const
{
	// timeOut if we are displaying the last part of the text
	if (isOnLastPage())
		return _Character && (TimeInSec>=_TimeOut);
	else
		return false;
}

// ***************************************************************************

double CGroupInSceneBubble::getTimeout () const
{
	if (isOnLastPage())
		return _TimeOut;
	else
		return TimeInSec+RZ_FADE_TIME;
}

// ***************************************************************************

void CGroupInSceneBubble::link (CCharacterCL	*entity, uint duration)
{
	unlink ();
	_Character = entity;
	_Duration = duration;
	_TimeOut = TimeInSec + duration;
}

// ***************************************************************************

void CGroupInSceneBubble::unlink ()
{
	if (_Character)
		_Character->setBubble (NULL);
	_Character = NULL;
	setActive (false);
}

// ***************************************************************************

void CGroupInSceneBubble::setText (const ucstring &text)
{
	if (text.empty()) return;
	_TextParts.clear();

	// Look for "{break}" in the message
	ucstring finalMsg = text;
	ucstring tmpMsg;

	for(;;)
	{
		ucstring::size_type index = finalMsg.find (ucstring("{break}"));
		if (index == ucstring::npos) break;
		tmpMsg = finalMsg.substr (0, index);
		if (!tmpMsg.empty())
			_TextParts.push_back(tmpMsg);
		finalMsg = finalMsg.substr(index+7, finalMsg.size());
	}
	if (!finalMsg.empty())
		_TextParts.push_back(finalMsg);

	_CurrentPart = 0;

	setRawText(_TextParts[_CurrentPart]);
	// If more than one part display the 2 buttons next and skip
	displayNextAndSkip( !isOnLastPage() );

	invalidateCoords();
}

// ***************************************************************************

void CGroupInSceneBubble::next()
{
	if (_TextParts.empty()) return;
	if (_CurrentPart >= (_TextParts.size()-1)) return;
	_CurrentPart++;
	setRawText(_TextParts[_CurrentPart]);
	displayNextAndSkip( !isOnLastPage() );
	_TimeOut = TimeInSec + _Duration;
}

// ***************************************************************************

void CGroupInSceneBubble::skip()
{
	if (_TextParts.empty()) return;
	_CurrentPart = (uint32)_TextParts.size()-1;
	_TimeOut = TimeInSec;
}

// ***************************************************************************

void CGroupInSceneBubble::setRawText (const ucstring &text)
{
	_CanBeShown = !text.empty();
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CInterfaceElement *pVTIE = CWidgetManager::getInstance()->getElementFromId(getId()+":header_opened:window:text");
	CViewText *pVT= dynamic_cast<CViewText*>(pVTIE);
	if (pVT != NULL)
		pVT->setText(text);
}

// ***************************************************************************

void CGroupInSceneBubble::displayNextAndSkip(bool show)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId(getId()+":header_opened:window:but_next");
	if (pIE != NULL) pIE->setActive(show);
	pIE = CWidgetManager::getInstance()->getElementFromId(getId()+":header_opened:window:but_skip");
	if (pIE != NULL) pIE->setActive(show);
	pIE = CWidgetManager::getInstance()->getElementFromId(getId()+":header_opened:window:text");
	if (pIE != NULL)
	{
		if (show)
			pIE->setY(-24);
		else
			pIE->setY(-5);
	}
}

// ***************************************************************************

class CHandlerBubbleNext : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		// Find the bubble containing the caller
		if (pCaller == NULL) return;
		CInterfaceGroup *pParent = pCaller->getParent();
		CGroupInSceneBubble *pBubble = NULL;
		while (pParent != NULL)
		{
			pBubble = dynamic_cast<CGroupInSceneBubble*>(pParent);
			if (pBubble != NULL) break;
			pParent = pParent->getParent();
		}
		if (pBubble == NULL) return;

		// Check if its a dynamic bubble
		if (strnicmp(pBubble->getId().c_str(), "ui:interface:in_scene_dyn_bubble", 32) == 0)
		{
			string nb = pBubble->getId().substr(33, pBubble->getId().size());
			sint32 nBubbleNb;
			fromString(nb, nBubbleNb);
			InSceneBubbleManager.dynChatNext(nBubbleNb);
		}
		else
		{
			pBubble->next();
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerBubbleNext, "bubble_next");

// ***************************************************************************

class CHandlerBubbleSkip : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		// Find the bubble containing the caller
		if (pCaller == NULL) return;
		CInterfaceGroup *pParent = pCaller->getParent();
		CGroupInSceneBubble *pBubble = NULL;
		while (pParent != NULL)
		{
			pBubble = dynamic_cast<CGroupInSceneBubble*>(pParent);
			if (pBubble != NULL) break;
			pParent = pParent->getParent();
		}
		if (pBubble == NULL) return;

		// Check if its a dynamic bubble
		if (strnicmp(pBubble->getId().c_str(), "ui:interface:in_scene_dyn_bubble", 32) == 0)
		{
			string nb = pBubble->getId().substr(33, pBubble->getId().size());
			sint32 nBubbleNb;
			fromString(nb, nBubbleNb);
			InSceneBubbleManager.dynChatSkip(nBubbleNb);
		}
		else
		{
			pBubble->skip();
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerBubbleSkip, "bubble_skip");

// ***************************************************************************

class CHandlerCharacterBubble : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		uint entityId;
		fromString(getParam (sParams, "entity"), entityId);
		ucstring text;
		text.fromUtf8(getParam (sParams, "text"));
		string sTime = getParam (sParams, "time");
		uint duration;
		if (sTime.empty())
			duration = CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionTimeoutBubbles).getValSInt32();
		else
			fromString(sTime, duration);

		CEntityCL *entity = EntitiesMngr.entity(entityId);
		if (entity)
			InSceneBubbleManager.chatOpen (entity->dataSetId(), text, duration);
	}
};
REGISTER_ACTION_HANDLER( CHandlerCharacterBubble, "character_bubble");

// ***************************************************************************

class CHandlerSkillPopup : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// User Skill Up
		UserEntity->skillUp();
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		uint skillId;
		fromString(getParam (sParams, "skillid"), skillId);
		sint delta;
		fromString(getParam (sParams, "delta"), delta);
		string sTime = getParam (sParams, "time");
		uint duration;
		if (sTime.empty())
			duration = CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionTimeoutMessages).getValSInt32();
		else
			fromString(sTime, duration);

		// Add the skill message
		InSceneBubbleManager.addSkillPopup(skillId, delta, duration);
	}
};
REGISTER_ACTION_HANDLER( CHandlerSkillPopup, "skill_popup");

// ***************************************************************************

class CHandlerMessagePopup : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		ucstring text0;
		text0.fromUtf8(getParam (sParams, "text0").c_str());
		ucstring text1;
		text1.fromUtf8(getParam (sParams, "text1").c_str());
		string sTime = getParam (sParams, "time");
		uint duration;
		if (sTime.empty())
			duration = CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionTimeoutMessages).getValSInt32();
		else
			fromString(sTime, duration);

		// Add the skill message
		InSceneBubbleManager.addMessagePopup(text0 + text1, CRGBA::White, duration);
	}
};
REGISTER_ACTION_HANDLER( CHandlerMessagePopup, "message_popup");

// ***************************************************************************

class CHandlerContextHelp : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		string targetName = getParam (sParams, "target");
		string text = getParam (sParams, "text");
		ucstring itext;
		itext.fromUtf8 (getParam (sParams, "itext"));
		if (itext.empty())
			itext = CI18N::get(text);

		InSceneBubbleManager.addContextHelp (itext, targetName, CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionTimeoutContext).getValSInt32());
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextHelp, "context_help");

// ***************************************************************************

class CHandlerContextHelpHTML : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		contextHelp (sParams);
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextHelpHTML, "context_help_html");

// ***************************************************************************

class CHandlerContextHelpIgnore : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		if (pCaller)
		{
			// Find the base group
			CInterfaceGroup *parent = pCaller->getParent();
			while (parent)
			{
				if (parent->getParent() && (parent->getParent()->getId() == "ui:interface"))
				{
					parent->setActive(true);
					break;
				}

				parent = parent->getParent();
			}

			if (parent)
			{
				// Look for the group
				InSceneBubbleManager.ignoreContextHelp (parent);
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextHelpIgnore, "context_help_ignore");

