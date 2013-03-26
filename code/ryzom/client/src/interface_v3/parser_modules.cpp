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
#include "parser_modules.h"
#include "nel/gui/view_text.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/group_list.h"
#include "interface_ddx.h"
#include "macrocmd_manager.h"
#include "../commands.h"
#include "interface_3d_scene.h"
#include "nel/misc/i_xml.h"

using namespace NLMISC;

#ifdef FINAL_VERSION
#include "../client_cfg.h"
#endif

CIF3DSceneParser::CIF3DSceneParser()
{
	parsingStage |= ( Resolved | GroupChildren );
}

CIF3DSceneParser::~CIF3DSceneParser()
{
}

bool CIF3DSceneParser::parse( xmlNodePtr cur, NLGUI::CInterfaceGroup *parentGroup )
{
	CInterface3DScene *pScene;
	CXMLAutoPtr ptr;

	pScene = new CInterface3DScene(CViewBase::TCtorParam());

	// parse the group attributes
	if (!pScene->parse(cur,parentGroup))
	{
		delete pScene;
		// todo hulud interface syntax error
		nlinfo ("cannot parse 3d scene attributes");
		return false;
	}

	if (parentGroup)
	{
		CGroupList *pList = dynamic_cast<CGroupList*>(parentGroup);
		if (pList != NULL)
			pList->addChild (pScene);
		else
			parentGroup->addGroup (pScene);
	}
	else
	{
		std::string tmp = "no parent for "+pScene->getId();
		// todo hulud interface syntax error
		nlinfo (tmp.c_str());
		delete pScene;
		return false;
	}

	return true;
}



CIFDDXParser::CIFDDXParser()
{
	parsingStage |= ( Resolved | GroupChildren );
}

CIFDDXParser::~CIFDDXParser()
{
}

bool CIFDDXParser::parse( xmlNodePtr cur, NLGUI::CInterfaceGroup *parentGroup )
{
	CInterfaceDDX *pDDX = NULL;
	pDDX = new CInterfaceDDX;
	if (pDDX)
	{
		if (!pDDX->parse(cur,parentGroup))
		{
			delete pDDX;
			return false;
		}
		return true;
	}
	return false;
}




CActionCategoryParser::CActionCategoryParser()
{
	parsingStage |= Unresolved;
}

CActionCategoryParser::~CActionCategoryParser()
{
}

bool CActionCategoryParser::parse( xmlNodePtr cur, NLGUI::CInterfaceGroup *parentGroup )
{
	// The category
   	CCategory category;

	// Name
	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"name" ));
	if (ptr)
		category.Name = (const char*)ptr;

	// Localized string
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"hardtext" );
	if (ptr)
		category.LocalizedName = (const char*)ptr;

	// macroisable (per category)
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"macroisable" );
	if (ptr)
		category.Macroisable= CInterfaceElement::convertBool(ptr);

	// Count number of action
	uint ns = CIXml::countChildren(cur, "action");
	category.BaseActions.resize( ns );


	std::string actionCategoryContext = "game";

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"contexts" );
	if (ptr)
		actionCategoryContext = (const char *) ptr;

	uint actionIndex = 0;
	xmlNodePtr actionNode = CIXml::getFirstChildNode(cur, "action");
	if (actionNode)
	{
		do
		{
			// The action
			CBaseAction &action = category.BaseActions[actionIndex];

			// list of contexts in which this action is valid
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"contexts" );
			if (ptr)
				action.Contexts = (const char *) ptr;
			else
				action.Contexts = actionCategoryContext; // inherit from action category

			// Repeat flag
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"repeat" );
			if (ptr)
				fromString((const char*)ptr, action.Repeat);

			// KeyDown flag
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"keydown" );
			if (ptr)
				fromString((const char*)ptr, action.KeyDown);

			// KeyUp flag
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"keyup" );
			if (ptr)
				fromString((const char*)ptr, action.KeyUp);

			// WaitForServer flag (wait an answer from server before continuing)
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"waitforserver" );
			if (ptr)
				fromString((const char*)ptr, action.WaitForServer);

			// Action name
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"name" );
			if (ptr)
				action.Name = (const char*)ptr;


			// Action localized name
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"hardtext" );
			if (ptr)
				action.LocalizedName = (const char*)ptr;

			// macroisable (per action)
			action.Macroisable= true;
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"macroisable" );
			if (ptr)
				action.Macroisable = CInterfaceElement::convertBool(ptr);


			// Read the parameters
			action.Parameters.resize (CIXml::countChildren(actionNode, "parameter"));

			uint parameterIndex = 0;
			xmlNodePtr paramNode = CIXml::getFirstChildNode(actionNode, "parameter");
			if (paramNode)
			{
				do
				{
					// The parameter
					CBaseAction::CParameter &parameter = action.Parameters[parameterIndex];

					// Parameter type
					ptr = (char*) xmlGetProp( paramNode, (xmlChar*)"type" );
					if (ptr)
					{
						sint32 tType;
						fromString((const char*)ptr, tType);
						parameter.Type = (CBaseAction::CParameter::TType)tType;
					}

					// Parameter name
					ptr = (char*) xmlGetProp( paramNode, (xmlChar*)"name" );
					if (ptr)
						parameter.Name = (const char*)ptr;

					// Parameter localized name
					ptr = (char*) xmlGetProp( paramNode, (xmlChar*)"hardtext" );
					if (ptr)
						parameter.LocalizedName = (const char*)ptr;

					// Default value
					ptr = (char*) xmlGetProp( paramNode, (xmlChar*)"value" );
					if (ptr)
						parameter.DefaultValue = (const char*)ptr;

					// Visible flag
					//ptr = (char*) xmlGetProp( paramNode, (xmlChar*)"visible" );
					//if (ptr)
					//	fromString((const char*)ptr, parameter.Visible);

					// Parse instance
					xmlNodePtr instanceNode = CIXml::getFirstChildNode(paramNode, "instance");
					if (instanceNode)
					{
						do
						{
							if (!parser->parseInstance(instanceNode))
							{
								// todo hulud interface syntax error
								nlwarning("<CInterfaceParser::parseActionCategory> cannot create instance from template");
							}
						}
						while((instanceNode = CIXml::getNextChildNode(instanceNode, "instance")));
					}

					parameter.Values.resize (CIXml::countChildren(paramNode, "value"));

					uint valueIndex = 0;
					xmlNodePtr valueNode = CIXml::getFirstChildNode(paramNode, "value");
					if (valueNode)
					{
						do
						{
							// The value
							CBaseAction::CParameter::CValue &value = parameter.Values[valueIndex];

							// Value
							ptr = (char*) xmlGetProp( valueNode, (xmlChar*)"value" );
							if (ptr)
								value.Value = (const char*)ptr;

							// list of contexts in which this value is valid
							ptr = (char*) xmlGetProp( valueNode, (xmlChar*)"contexts" );
							if (ptr) value.Contexts = (const char*) ptr;
							else value.Contexts = action.Contexts; // inherit context from action

							// Localized value
							ptr = (char*) xmlGetProp( valueNode, (xmlChar*)"hardtext" );
							if (ptr)
								value.LocalizedValue = (const char*)ptr;

							valueIndex++;
						}
						while((valueNode = CIXml::getNextChildNode(valueNode, "value")));
					}

					parameterIndex++;
				}
				while((paramNode = CIXml::getNextChildNode(paramNode, "parameter")));
			}

			// Next action
			actionIndex++;
		}
		while((actionNode = CIXml::getNextChildNode(actionNode, "action")));
	}

	// Add this category to the action manager
	CActionsManager *actionManager = ActionsContext.getActionsManager (category.Name);
	if (actionManager)
	{
// They want to display debug shortcut in final version
#if FINAL_VERSION
		if ((category.Name != "debug") || ClientCfg.AllowDebugCommands)
#else // FINAL_VERSION
		if (1)
#endif // FINAL_VERSION
		{
			actionManager->removeCategory (category.Name);
			actionManager->addCategory (category);
		}
		else
		{
			// Remove thoses actions from the manager
			CAHManager *pAHFM = CAHManager::getInstance();
			uint i;
			for (i=0; i<category.BaseActions.size(); i++)
			{
				CAHManager::TFactoryMap::iterator ite = pAHFM->FactoryMap.find (category.BaseActions[i].Name);
				if (ite != pAHFM->FactoryMap.end())
				{
					IActionHandler *ah = ite->second;
					pAHFM->FactoryMap.erase (ite);
					pAHFM->NameMap.erase (ah);
				}
			}
		}
	}
	return true;
}



CCommandParser::CCommandParser()
{
	parsingStage |= Unresolved;
}

CCommandParser::~CCommandParser()
{
}

bool CCommandParser::parse( xmlNodePtr cur, NLGUI::CInterfaceGroup *parentGroup )
{
	// Parse the key
	bool ret = false;

	// Localized string
	CXMLAutoPtr ptrName((const char*) xmlGetProp( cur, (xmlChar*)"name" ));
	if (ptrName)
	{
		// Does the action exist ?
		std::string name = ptrName;
		if (!ICommand::exists (name) || (CUserCommand::CommandMap.find(name) != CUserCommand::CommandMap.end()))
		{
			// Get the action
			CXMLAutoPtr ptrAction((const char*) xmlGetProp( cur, (xmlChar*)"action" ));
			if (ptrAction)
			{
				// Get the params
				CXMLAutoPtr ptrParams((const char*) xmlGetProp( cur, (xmlChar*)"params" ));
				if (ptrParams)
				{
					CUserCommand::createCommand (ptrName, ptrAction, ptrParams);

					// if prop "ctrlchar" is declared with false, then disable ctrlchar for this command
					CXMLAutoPtr prop((const char*) xmlGetProp( cur, (xmlChar*)"ctrlchar" ));
					if( (const char*)prop && (CInterfaceElement::convertBool((const char*)prop)==false) )
						ICommand::enableControlCharForCommand(ptrName, false);

					// Done
					ret = true;
				}
			}
			else
			{
				// todo hulud interface syntax error
				nlwarning("<CInterfaceParser::parseCommand> No action for command : %s", (const char*)ptrName);
			}
		}
	}
	else
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::parseCommand> No name for a key");
	}

	return ret;
}



CKeyParser::CKeyParser()
{
	parsingStage |= Unresolved;
}

CKeyParser::~CKeyParser()
{
}

bool CKeyParser::parse( xmlNodePtr cur, NLGUI::CInterfaceGroup *parentGroup )
{
	// Parse the key
	bool ret = false;

	// Localized string
	TKey key;
	CXMLAutoPtr ptrKey((const char*) xmlGetProp( cur, (xmlChar*)"name" ));
	if (ptrKey)
	{
		bool isNA = std::string((const char*)ptrKey) == std::string("N/A");
		// Get the key from the string
		key = CEventKey::getKeyFromString ((const char*)ptrKey);
		if (key != KeyCount || isNA)
		{
			// Get the action
			CXMLAutoPtr ptrAction((const char*) xmlGetProp( cur, (xmlChar*)"action" ));
			if (ptrAction)
			{
				// Get the params
				CXMLAutoPtr ptrParams((const char*) xmlGetProp( cur, (xmlChar*)"params" ));

				// Get the modifiers
				bool shift=false;
				bool ctrl=false;
				bool menu=false;
				CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"shift" ));
				if (ptr)
					fromString((const char*)ptr, shift);
				ptr = (char*) xmlGetProp( cur, (xmlChar*)"ctrl" );
				if (ptr)
					fromString((const char*)ptr, ctrl);
				ptr = (char*) xmlGetProp( cur, (xmlChar*)"menu" );
				if (ptr)
					fromString((const char*)ptr, menu);

				// Repeat flag
				bool repeat=false;
				ptr = (char*) xmlGetProp( cur, (xmlChar*)"repeat" );
				if (ptr)
					fromString((const char*)ptr, repeat);

				// Get the context
				CXMLAutoPtr ptrContext((const char*) xmlGetProp( cur, (xmlChar*)"context" ));
				std::string context = (const char*)ptrContext?(const char*)ptrContext:"";

				// Add the action
				CCombo combo;
				combo.init(key, (TKeyButton)((shift?shiftKeyButton:noKeyButton)|(ctrl?ctrlKeyButton:noKeyButton)|(menu?altKeyButton:noKeyButton)));
				::CAction::CName actionName ((const char*)ptrAction, ptrParams?(const char*)ptrParams:"");

				// Get the actions context manager
				CActionsManager *actionManager = ActionsContext.getActionsManager(context);
				if (actionManager)
				{
					bool canAdd= true;

					// for keys.xml, don't replace already defined keys
					if( parser->getDefine("key_def_no_replace")=="1" )
					{
						// if this combo key is already used for any action,
						// or if this action is already bound to any key
						if(isNA || actionManager->isComboAssociated(combo) || actionManager->isActionAssociated(actionName))
							// don't replace
							canAdd= false;
					}

					// add/replace the combo?
					if(canAdd)
					{
						actionManager->addCombo(actionName, combo);
						::CAction *action = actionManager->getAction(actionName);
						if (action && repeat) action->Repeat = true;
					}

					// if the action is to be shown in the Key interface
					if( parser->getDefine("key_def_force_display")=="1" )
						actionManager->forceDisplayForAction(actionName, true);
				}

				// Done
				ret = true;
			}
			else
			{
				// todo hulud interface syntax error
				nlwarning("<CInterfaceParser::parseKey> No action for key : %s", (const char*)ptrKey);
			}
		}
		else
		{
			// todo hulud interface syntax error
			nlwarning("<CInterfaceParser::parseKey> Unknown key : %s", (const char*)ptrKey);
		}
	}
	else
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::parseKey> No name for a key");
	}

	return ret;
}



CMacroParser::CMacroParser()
{
	parsingStage |= Unresolved;
}

CMacroParser::~CMacroParser()
{
}

bool CMacroParser::parse( xmlNodePtr cur, NLGUI::CInterfaceGroup *parentGroup )
{
	H_AUTO(parseMacro)

	CMacroCmd cmd;
	if (cmd.readFrom(cur))
		CMacroCmdManager::getInstance()->addMacro(cmd);
	else
		return false;
	return true;
}

