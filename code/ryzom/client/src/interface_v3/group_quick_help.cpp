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

#include "group_quick_help.h"
#include "nel/gui/group_list.h"
#include "nel/gui/group_paragraph.h"
#include "nel/gui/libwww.h"
#include "interface_manager.h"
#include "nel/gui/action_handler.h"
#include "nel/misc/xml_auto_ptr.h"
#include "../actions.h"
#include "../client_cfg.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CGroupQuickHelp, std::string, "quick_help");

CGroupQuickHelp::CGroupQuickHelp(const TCtorParam &param)
:	CGroupHTML(param)
{
	_IsQuickHelp = false;
	initParameters();
	_UpdateParagraphNextUpdateCoords = true;
}

// ***************************************************************************

CGroupQuickHelp::~CGroupQuickHelp()
{
}

// ***************************************************************************

bool CGroupQuickHelp::submitEvent (const char *event)
{
	if(_IsQuickHelp==false)
		return false;

	if (_CurrentStep<_Steps.size())
	{
		const CStep &step = _Steps[_CurrentStep];
		if (step.EventToComplete.find (event) != step.EventToComplete.end())
		{
			// Next step
			_CurrentStep++;

			activateCurrentStep ();

			// Update the text
			updateParagraph ();
		}
	}
	return false;
}

// ***************************************************************************

void CGroupQuickHelp::updateParagraph ()
{
	// Get the group list of CGroupScrollText
	CGroupList *groupList = getList ();
	if (groupList && groupList->getNumChildren())
	{
		// Get the adaptator
		CViewBase *viewBase = groupList->getChild(0);
		if (viewBase)
		{
			CInterfaceGroup *adaptor = dynamic_cast<CInterfaceGroup*>(viewBase);
			if (adaptor)
			{
				// Get the groups
				const vector<CInterfaceGroup*> &groups = adaptor->getGroups();

				// For each paragraph
				uint i;
				for (i=0; i<groups.size(); i++)
				{
					if (groups[i])
					{
						// Get a paragraph
						CGroupParagraph *paragraph = dynamic_cast<CGroupParagraph*> (groups[i]);
						if (paragraph)
						{
							// Set the text size
							if (i==_CurrentStep)
								setGroupTextSize (paragraph, true);
							else
								setGroupTextSize (paragraph, false);
						}
					}
				}
			}
		}

	}
}

// ***************************************************************************

bool CGroupQuickHelp::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	// Reset parameters
	initParameters ();

	CXMLAutoPtr ptr;

	ptr = xmlGetProp (cur, (xmlChar*)"non_selected_color");
	if (ptr)
		_NonSelectedColor = convertColor(ptr);
	ptr = xmlGetProp (cur, (xmlChar*)"non_selected_link_color");
	if (ptr)
		_NonSelectedLinkColor = convertColor(ptr);
	ptr = xmlGetProp (cur, (xmlChar*)"non_selected_global_color");
	if (ptr)
		_NonSelectedGlobalColor = convertBool(ptr);
	ptr = xmlGetProp (cur, (xmlChar*)"non_selected_font_size");
	if (ptr)
		fromString((const char*)ptr, _NonSelectedSize);


	if (!CGroupHTML::parse (cur, parentGroup))
		return false;

	return true;
}

// ***************************************************************************

void CGroupQuickHelp::initParameters()
{
	_CurrentStep = 0;
	_NonSelectedSize = 10;
	_NonSelectedColor = CRGBA(128,128,128);
	_NonSelectedLinkColor = CRGBA(0,0,128);
	_NonSelectedGlobalColor = true;
}

// ***************************************************************************

void CGroupQuickHelp::setGroupTextSize (CInterfaceGroup *group, bool selected)
{
	bool globalColor = selected ? TextColorGlobalColor : _NonSelectedGlobalColor;
	bool linkGlobalColor = selected ? LinkColorGlobalColor : _NonSelectedGlobalColor;
	uint fontSize = selected ? TextFontSize : _NonSelectedSize;_NonSelectedSize;
	NLMISC::CRGBA color = selected ? TextColor : _NonSelectedColor;
	NLMISC::CRGBA linkColor = selected ? LinkColor : _NonSelectedLinkColor;

	// Look for text in this group
	const vector<CViewBase*> &views = group->getViews();
	uint i;
	for (i=0; i<views.size(); i++)
	{
		// View text ?
		CViewLink *viewText = dynamic_cast<CViewLink *>(views[i]);
		if (viewText)
		{
			bool link = !(viewText->getText().empty()) && viewText->getUnderlined ();

			// Set text attributes
			viewText->setFontSize(fontSize);
			viewText->setColor(link ? linkColor : color);
			viewText->setModulateGlobalColor(link ? linkGlobalColor : globalColor);
		}
	}

	// Active / desactive link ctrl
	const vector<CCtrlBase*> &ctrls = group->getControls();
	for (i=0; i<ctrls.size(); i++)
	{
		CCtrlLink *ctrlLink = dynamic_cast<CCtrlLink*>(ctrls[i]);
		if (ctrlLink)
		{
			ctrlLink->setActive(selected);
		}
	}

	// Look into subgroups
	const vector<CInterfaceGroup*> &groups = group->getGroups();
	for (i=0; i<groups.size(); i++)
	{
		if (groups[i])
		{
			setGroupTextSize (groups[i], selected);
		}
	}
	CGroupParagraph *p = dynamic_cast<CGroupParagraph*>(group);
	if (p)
		p->setTopSpace(0);
	group->setActive(selected);
}

// ***************************************************************************

extern CActionsContext ActionsContext;

void CGroupQuickHelp::beginElement (uint element_number, const BOOL *present, const char **value)
{
	CGroupHTML::beginElement (element_number, present, value);

	// Paragraph ?
	switch(element_number)
	{
	case HTML_A:
			// Quick help
			if (_TrustedDomain && present[MY_HTML_A_Z_ACTION_SHORTCUT] && value[MY_HTML_A_Z_ACTION_SHORTCUT])
			{
				// Get the action category
				string category;
				if (present[MY_HTML_A_Z_ACTION_CATEGORY] && value[MY_HTML_A_Z_ACTION_CATEGORY])
					category = value[MY_HTML_A_Z_ACTION_CATEGORY];

				// Get the action params
				string params;
				if (present[MY_HTML_A_Z_ACTION_PARAMS] && value[MY_HTML_A_Z_ACTION_PARAMS])
					params = value[MY_HTML_A_Z_ACTION_PARAMS];

				// Get the action descriptor
				CActionsManager *actionManager = ActionsContext.getActionsManager (category);
				if (actionManager)
				{
					const CActionsManager::TActionComboMap &actionCombo = actionManager->getActionComboMap ();
					CActionsManager::TActionComboMap::const_iterator ite = actionCombo.find (CAction::CName (value[MY_HTML_A_Z_ACTION_SHORTCUT], params.c_str()));
					if (ite != actionCombo.end())
					{
						addString (ite->second.toUCString());
					}
				}
			}
			break;

	case HTML_P:
		// Get the action name
		if (present[MY_HTML_P_QUICK_HELP_EVENTS])
		{
			// This page is a quick help
			_IsQuickHelp = true;
			// Add a step
			_Steps.push_back (CStep());
			CStep &step = _Steps.back();

			if (value[MY_HTML_P_QUICK_HELP_EVENTS])
			{
				// Get the event names
				string events = value[MY_HTML_P_QUICK_HELP_EVENTS];
				if (!events.empty())
				{
					uint first = 0;
					while (first < events.size())
					{
						// String end
						string::size_type last = events.find_first_of(" ", first);
						if (last == string::npos)
							last = events.size();

						// Extract the string
						step.EventToComplete.insert (events.substr (first, last-first));
						first = (uint)last+1;
					}
				}
			}

			// Get the condition
			if (present[MY_HTML_P_QUICK_HELP_CONDITION] && value[MY_HTML_P_QUICK_HELP_CONDITION])
			{
				step.Condition = value[MY_HTML_P_QUICK_HELP_CONDITION];
			}

			// Get the action handlers to run
			if (present[MY_HTML_P_QUICK_HELP_LINK] && value[MY_HTML_P_QUICK_HELP_LINK])
			{
				step.URL = value[MY_HTML_P_QUICK_HELP_LINK];
			}
		}
		break;
	}
}

// ***************************************************************************

void CGroupQuickHelp::browse (const char *url)
{
	// Reset the data
	_Steps.clear ();
	_CurrentStep = 0;

	_IsQuickHelp = false;

	string completeURL = url;
	if (completeURL.substr(completeURL.size()-5, 5) == ".html")
	{
		completeURL = completeURL.substr(0, completeURL.size()-5); // Substract the ".html"
		completeURL += "_" + ClientCfg.getHtmlLanguageCode() + ".html";
	}

	CGroupHTML::browse (completeURL.c_str());
}

// ***************************************************************************

std::string	CGroupQuickHelp::home()
{
	string completeURL = Home;
	completeURL = completeURL.substr(0, completeURL.size()-5); // Substract the ".html"
	completeURL += "_" + ClientCfg.getHtmlLanguageCode() + ".html";

	return completeURL;
}

// ***************************************************************************

void CGroupQuickHelp::endBuild ()
{
	CGroupHTML::endBuild ();

	if(_IsQuickHelp==false)
		return;

	// First step must run an action handler ?
	activateCurrentStep ();
	_UpdateParagraphNextUpdateCoords = true;
}

// ***************************************************************************

void CGroupQuickHelp::activateCurrentStep ()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	for(;;)
	{
		if (_CurrentStep < _Steps.size())
		{
			// Current step
			CStep &step = _Steps[_CurrentStep];

			// A link to follow ?
			if (!step.URL.empty())
				CAHManager::getInstance()->runActionHandler("browse", NULL, "url="+step.URL);

			// Test a skip condition
			if (!step.Condition.empty() && evalExpression (step.Condition))
			{
				// Next action handler
				_CurrentStep++;
				continue;
			}
		}
		break;
	}
}

// ***************************************************************************

bool CGroupQuickHelp::evalExpression (const std::string &condition)
{
	// Add your conditions here :

	if (condition == "always")
		return true;

	return false;
}

// ***************************************************************************
/** Submit a quick help action
 */
class CHandlerSubmitQuickHelp : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceElement *element = CWidgetManager::getInstance()->getElementFromId("ui:interface:quick_help:content:html");
		if (element)
		{
			// Group HTML ?
			CGroupQuickHelp *groupQH = dynamic_cast<CGroupQuickHelp*>(element);
			if (groupQH)
			{
				// Submit the form the url
				groupQH->submitEvent (sParams.c_str());
			}
		}
		element = CWidgetManager::getInstance()->getElementFromId("ui:interface:help_browser:content:html");
		if (element)
		{
			// Group HTML ?
			CGroupQuickHelp *groupQH = dynamic_cast<CGroupQuickHelp*>(element);
			if (groupQH)
			{
				// Submit the form the url
				groupQH->submitEvent (sParams.c_str());
			}
		}

	}
};
REGISTER_ACTION_HANDLER( CHandlerSubmitQuickHelp, "submit_quick_help");

// ***************************************************************************
/** Run a quick help
 */
class CHandlerRunQuickHelp : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		// Get the quick help radio buttons base id
		string buttonId = CWidgetManager::getInstance()->getParser()->getDefine("quick_help_buttons");
		if (!buttonId.empty())
		{
			// Get the button id
			CInterfaceElement *element = CWidgetManager::getInstance()->getElementFromId(buttonId+":"+sParams);
			if (element)
			{
				// Button Ctrl ?
				CCtrlBaseButton *button = dynamic_cast<CCtrlBaseButton*>(element);
				if (button)
				{
					// Push the button
					button->setPushed(true);

					// Run the left click action handler
					CAHManager::getInstance()->runActionHandler(button->getActionOnLeftClick(), button, button->getParamsOnLeftClick());
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerRunQuickHelp, "run_quick_help");

// ***************************************************************************

void CGroupQuickHelp::updateCoords()
{
	CGroupHTML::updateCoords();
	if(_IsQuickHelp==false)
		return;
	if (_UpdateParagraphNextUpdateCoords)
	{
		_UpdateParagraphNextUpdateCoords = false;
		updateParagraph ();
	}
}

// ***************************************************************************
