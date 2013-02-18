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

#include "actions.h"
#include "events_listener.h"
#include "interface_v3/interface_manager.h"

using namespace std;
using namespace NLMISC;

extern CEventsListener EventsListener;

////////////
// GLOBAL //
////////////

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Actions_Context_Mngr_Update )

static bool getParam (CBaseAction::CParameter::TType type, ucstring &paramName, ucstring &paramValue, const std::string &argu, uint paramId);

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
// CAction //////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
//---------------------------------------------------
// CAction :
// Constructor
//---------------------------------------------------
CAction::CAction()
{
	Valide = false;
	Repeat = false;
	KeyDown = true;
	KeyUp = false;
}// CAction //


void CAction::runAction ()
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();
	if (IM)
	{
		CAHManager::getInstance()->runActionHandler (Name.Name, NULL, Name.Argu);
	}
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
// CActionsManager //////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
//---------------------------------------------------
// CActionsManager :
// Constructor
//---------------------------------------------------
CActionsManager::CActionsManager()
{
	clear ();
	_Enabled = false;
}// CActionsManager //


//---------------------------------------------------
// addAction :
// Add a new Action in the context.
//---------------------------------------------------
bool CActionsManager::addAction(const CAction::CName &name)
{
	// Try to allocate memory for action
	CAction action;
	action.Name = name;

	// Find the base action
	const CBaseAction *baseAction = getBaseAction(name);
	if (baseAction)
	{
		// Copy the repeat flag
		action.Repeat = baseAction->Repeat;
		action.KeyUp = baseAction->KeyUp;
		action.KeyDown = baseAction->KeyDown;
	}

	// Try to insert the new action.
	pair<TActionsMap::iterator, bool> p = _Actions.insert(TActionsMap::value_type(name, action));

	if (!p.second)
	{
		nlwarning ("Action (%s %s) already exist in the action manager.", name.Name.c_str (), name.Argu.c_str ());
	}

	return p.second;
}// addAction //

// ***************************************************************************

CAction* CActionsManager::getAction(const CAction::CName &name)
{
	TActionsMap::iterator it = _Actions.find(name);
	if (it == _Actions.end()) return NULL;
	return &(it->second);
}

// ***************************************************************************

void CActionsManager::clear ()
{
	_Actions.clear ();
	_ActionForceDisplay.clear ();
	_ActionCombo.clear ();
	_ComboAction.clear ();
	_KeyAction.clear ();
	_WatchedActions.clear ();
	_ActionCategory.clear ();
	_Categories.clear ();
}

// ***************************************************************************

void CActionsManager::removeCombo (const CCombo &combo)
{
	// Get the old action of the combo
	TComboActionMap::iterator itePreviousCombo = _ComboAction.find (combo);
	if (itePreviousCombo != _ComboAction.end ())
	{
		const CAction::CName oldName = itePreviousCombo->second;

		// Remove all affected keys
		TKeyActionMap::iterator ite = _KeyAction.find (combo.Key);
		while ((ite != _KeyAction.end ()) && (ite->first == combo.Key))
		{
			TKeyActionMap::iterator copyToDelete = ite;
			ite++;
			if (copyToDelete->second == oldName)
				_KeyAction.erase (copyToDelete);
		}

		// Remove the action
		_ActionCombo.erase (oldName);

		// Remove the combo
		_ComboAction.erase (itePreviousCombo);
	}
}

// ***************************************************************************
void CActionsManager::removeAllCombos()
{
	_Actions.clear ();
	_ActionCombo.clear ();
	_ComboAction.clear ();
	_KeyAction.clear ();
	_WatchedActions.clear ();
	_ActionForceDisplay.clear();
}

// ***************************************************************************

bool CActionsManager::addCombo(const CAction::CName &name, const CCombo &combo, bool createAction)
{
	// If createAction == "true" -> Create the Action before to add te combo.
	if(createAction)
		addAction(name);

	// Erase previous values
	TActionComboMap::iterator itePreviousCombo = _ActionCombo.find (name);
	if (itePreviousCombo != _ActionCombo.end ())
	{
		// Remove the old action affected to the combo
		removeCombo (itePreviousCombo->second);
	}

	// Remove the new combo
	removeCombo (combo);

	_ComboAction.insert (TComboActionMap::value_type (combo, name));
	_ActionCombo.insert (TActionComboMap::value_type (name, combo));
	_KeyAction.insert (TKeyActionMap::value_type (combo.Key, name));

	return true;
}// addCombo //

// ***************************************************************************
bool CActionsManager::valide(const CAction::CName &name) const
{
	// Recover the pointer on "actionName" if it exists.
	TActionsMap::const_iterator it = _Actions.find(name);
	if(it != _Actions.end())
	{
		return it->second.Valide;
	}
	// No action of this name found.
	else
		return false;
}// valide //

// ***************************************************************************
bool CActionsManager::isActionPresentInContext(const CAction::CName &name) const
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	if (!im->isInGame()) return true; // no filtering done when outgame, because actions.xml not loaded yet (no base actions where added)
	{
		const CBaseAction *baseAction = getBaseAction(name);
		if (!baseAction) return false;
		// see if action valid in current context
		if (!ActionsContext.matchContext(baseAction->Contexts)) return false;
		// all parameters must be valid in current context
		for (uint i=0; i<baseAction->Parameters.size (); i++)
		{
			const CBaseAction::CParameter &parameter = baseAction->Parameters[i];
			if (parameter.Type == CBaseAction::CParameter::Constant)
			{
				ucstring paramName;
				ucstring paramValue = parameter.DefaultValue;

				// Get the param from the argu
				getParam (parameter.Type, paramName, paramValue, name.Argu, i);

				bool found = true;
				for (uint k = 0; k < parameter.Values.size(); ++k)
				{
					if (parameter.Values[k].Value == paramValue.toUtf8())
					{
						if (!ActionsContext.matchContext(parameter.Values[k].Contexts)) return false;
						found = true;
						break;
					}
				}
				if (!found) return false;
			}
		}
		return true;
	}
}// valide //


// ***************************************************************************

bool CActionsManager::keyPushed (const CEventKeyDown &keyDown)
{
	bool actionExist = false;
	if (_Enabled)
	{
		CCombo combo;
		combo.Key = keyDown.Key;
		combo.KeyButtons = keyDown.Button;

		// Scan action binded to this key
		TKeyActionMap::iterator iteKeyAction = _KeyAction.find (keyDown.Key);
		while ((iteKeyAction != _KeyAction.end ()) && (iteKeyAction->first == keyDown.Key))
		{
			if (isActionPresentInContext(iteKeyAction->second))
			{
				// Add it to the set of actions to watch
				_WatchedActions.insert (*iteKeyAction);
			}
			iteKeyAction++;
		}

		// Update special valide actions
		updateKeyButton (keyDown.Button);

		// Get the key down
		TComboActionMap::iterator ite = _ComboAction.find (combo);
		if (ite != _ComboAction.end ())
		{
			// Get the action
			TActionsMap::iterator iteAction = _Actions.find (ite->second);
			nlassert (iteAction != _Actions.end ());

			if (isActionPresentInContext(iteAction->first))
			{
				// Run the action
				if (iteAction->second.KeyDown && (iteAction->second.Repeat || keyDown.FirstTime))
				{
					iteAction->second.runAction ();
				}

				// The action exist
				actionExist = true;
			}
			else
			{
				// The action exist
				actionExist = false;
			}
		}
	}
	return actionExist;
}

// ***************************************************************************

void CActionsManager::keyReleased (const CEventKeyUp &keyUp)
{
	if (_Enabled)
	{
		// Update special keys state
		updateKeyButton (keyUp.Button);

		// For each watched actions
		TKeyActionMap::iterator iteWatchedAction = _WatchedActions.begin ();
		while (iteWatchedAction != _WatchedActions.end ())
		{
			TKeyActionMap::iterator iteToDelete = iteWatchedAction++;

			// Get the combo for this action
			TActionComboMap::iterator iteCombo = _ActionCombo.find (iteToDelete->second);
			nlassert (iteCombo != _ActionCombo.end());

			// This combo released ?
			if (iteCombo->second.Key == keyUp.Key)
			{
				// Get the action
				TActionsMap::iterator iteAction = _Actions.find (iteToDelete->second);
				nlassert (iteAction != _Actions.end());

				// Remove this action from watching
				_WatchedActions.erase (iteToDelete);

				// Invalidate the action
				bool LastValid = iteAction->second.Valide;
				iteAction->second.Valide = false;

				if ((LastValid == true) && (iteAction->second.Valide == false))
				{
					// Run the action
					if (iteAction->second.KeyUp)
					{
						iteAction->second.runAction ();
					}
				}
			}
		}
	}
}

// ***************************************************************************
void CActionsManager::releaseAllKeyNoRunning()
{
	// For each watched actions
	TKeyActionMap::iterator iteWatchedAction = _WatchedActions.begin ();
	while (iteWatchedAction != _WatchedActions.end ())
	{
		TKeyActionMap::iterator iteToDelete = iteWatchedAction++;

		// Invalidate the action
		TActionsMap::iterator iteAction = _Actions.find (iteToDelete->second);
		nlassert (iteAction != _Actions.end());
		iteAction->second.Valide = false;

		// Remove this action from watching
		_WatchedActions.erase (iteToDelete);
	}
}


// ***************************************************************************

sint getMatchingNote (sint keyButton, sint newButton)
{
	// At least all the needed key
	if ((keyButton & newButton) != keyButton)
		return -1;

	// If exactly the same, we want it
	if (keyButton == newButton)
		return 10;

	// Else count the number of bits used
	const uint flags[3] = { ctrlKeyButton, shiftKeyButton, altKeyButton };
	sint count = 0;
	for (uint i=0; i<3; i++)
	{
		if (keyButton & flags[i])
			count++;
	}
	return count;
}

// ***************************************************************************

void CActionsManager::updateKeyButton (NLMISC::TKeyButton newButtons)
{
	// For each watched actions
	TKeyActionMap::iterator iteWatchedAction = _WatchedActions.begin ();
	while (iteWatchedAction != _WatchedActions.end ())
	{
		// First action for this key
		TKeyActionMap::iterator iteWatchedActionBegin = iteWatchedAction;

		// Current Key
		NLMISC::TKey key = iteWatchedAction->first;

		// Best matching action
		CAction		*bestMatching = NULL;
		sint		bestMatchingNote = -1;

		// For each action with the same key, search the best combo
		while ((iteWatchedAction != _WatchedActions.end ()) && (key == iteWatchedAction->first))
		{
			// Get the action combo
			TActionComboMap::iterator iteCombo = _ActionCombo.find (iteWatchedAction->second);
			if (iteCombo == _ActionCombo.end())
				nlwarning("try to find Name:%s , Argu:%s",iteWatchedAction->second.Name.c_str(), iteWatchedAction->second.Argu.c_str());
			nlassert (iteCombo != _ActionCombo.end());

			// Get the matching note
			sint matchingNote = getMatchingNote (iteCombo->second.KeyButtons, newButtons);
			if (matchingNote > bestMatchingNote)
			{
				// Get the action
				TActionsMap::iterator iteAction = _Actions.find (iteWatchedAction->second);
				nlassert (iteAction != _Actions.end());

				// Memorise the best action
				bestMatching = &(iteAction->second);
				bestMatchingNote = matchingNote;
			}

			iteWatchedAction++;
		}

		// Invalide or valide actions
		while (iteWatchedActionBegin != iteWatchedAction)
		{
			// Get the action
			TActionsMap::iterator iteAction = _Actions.find (iteWatchedActionBegin->second);
			nlassert (iteAction != _Actions.end());

			// Valide or invalide it
			bool LastValid = iteAction->second.Valide;
			iteAction->second.Valide = (&(iteAction->second) == bestMatching);
			if ((LastValid == true) && (iteAction->second.Valide == false))
			{
				// Run the action on keyup
				if (iteAction->second.KeyUp)
				{
					iteAction->second.runAction ();
				}
			}


			iteWatchedActionBegin++;
		}
	}
}

// ***************************************************************************

void CCombo::init (NLMISC::TKey key, NLMISC::TKeyButton keyButtons)
{
	Key = key;
	KeyButtons = keyButtons;
}

// ***************************************************************************
ucstring CCombo::toUCString() const
{
	ucstring ret;
	if ((KeyButtons & shiftKeyButton) && (Key != 0x10))
		ret += CI18N::get("uiKeySHIFT") + "+";
	if ((KeyButtons & ctrlKeyButton) && (Key != 0x11))
		ret += CI18N::get("uiKeyCONTROL") + "+";
	if ((KeyButtons & altKeyButton) && (Key != 0x12))
		ret += CI18N::get("uiKeyMENU") + "+";
	if (CI18N::hasTranslation("ui"+CEventKey::getStringFromKey(Key)))
		ret += CI18N::get("ui"+CEventKey::getStringFromKey(Key));
	else
		ret += CEventKey::getStringFromKey(Key);
	return ret;
}

// ***************************************************************************

const std::vector<CCategory>	&CActionsManager::getCategories () const
{
	return _Categories;
}

// ***************************************************************************

void	CActionsManager::reserveCategories (uint space)
{
	_Categories.reserve (space);
}

// ***************************************************************************

void	CActionsManager::addCategory (const CCategory &category)
{
	_Categories.push_back (category);

	// Add an entry in the map to get the base action by the action name
	uint i;
	for (i=0; i<category.BaseActions.size (); i++)
	{
		CCategoryLocator locator;
		locator.CategoryId = (uint)_Categories.size ()-1;
		locator.BaseActionId = i;
		_ActionCategory.insert (TActionBaseActionMap::value_type (category.BaseActions[i].Name, locator));
	}
}

// ***************************************************************************

void	CActionsManager::removeCategory (const string &catName)
{
	// Search the category
	uint i, catNb;
	for (catNb=0; catNb < _Categories.size(); ++catNb)
		if (_Categories[catNb].Name == catName)
			break;

	if (catNb == _Categories.size()) return;

	// Remove all entries in the map to get the base action by the action name
	for (i=0; i<_Categories[catNb].BaseActions.size (); i++)
	{
		CCategoryLocator locator;
		locator.CategoryId = catNb;
		locator.BaseActionId = i;
		_ActionCategory.erase (_Categories[catNb].BaseActions[i].Name);
	}
	_Categories.erase (_Categories.begin()+catNb);
}

// ***************************************************************************

void	CActionsManager::enable (bool enable)
{
	_Enabled = enable;
}

// ***************************************************************************
// CBaseAction
// ***************************************************************************

CBaseAction::CBaseAction ()
{
	Repeat = false;
	KeyDown = true;
	KeyUp = false;
	WaitForServer = false;
	Macroisable= true;
}

// ***************************************************************************
bool CBaseAction::isUsableInCurrentContext() const
{
	if (ActionsContext.matchContext(Contexts))
	{
		bool cteParamFound = false;
		bool matchingParamFound = false;
		// now, see if for all the constant parameter for this action,
		// at least one is valid in current context
		for (uint l = 0; l < Parameters.size(); ++l)
		{
			const CParameter &param = Parameters[l];
			if (param.Type == CParameter::Constant)
			{
				cteParamFound = true;
				for (uint m = 0; m < param.Values.size(); ++m)
				{
					if (ActionsContext.matchContext(param.Values[m].Contexts))
					{
						matchingParamFound = true;
						break;
					}
				}
			}
			if (matchingParamFound) break;
		}
		if (!cteParamFound || matchingParamFound) return true;
	}
	return false;
}

// ***************************************************************************

CBaseAction::CParameter::CParameter ()
{
//	Visible = true;
	Type = Constant;
}

// ***************************************************************************

static bool getParam (CBaseAction::CParameter::TType type, ucstring &paramName, ucstring &paramValue, const std::string &argu, uint paramId)
{
	const string separator = "|";
	const string equal_separator = "=";

	// Go to param start
	uint index = 0;
	string::size_type pos = 0;
	while (index != paramId)
	{
		// Not found ?
		pos = argu.find_first_of(separator, pos);
		if (pos == string::npos)
			return false;

		pos++;
		index++;
	}

	// Pos is valid ?
	if (pos < argu.size ())
	{
		// End
		string::size_type end = argu.find_first_of(separator, pos);
		if (end == string::npos)
			end = argu.size();

		// Look for a '='
		string::size_type equal = argu.find_first_of(equal_separator, pos);
		if ((equal != string::npos) && (equal >= end))
			equal = string::npos;

		// Equal is present ?
		if (equal != string::npos)
		{
			// Extract parameter name
			paramName = argu.substr(pos, equal-pos);
			pos = equal+1;
		}

		// Value ?
		if(type==CBaseAction::CParameter::User || type==CBaseAction::CParameter::UserName)
			paramValue.fromUtf8(argu.substr(pos, end-pos));
		else
			paramValue = argu.substr(pos, end-pos);

		// Ok
		return true;
	}
	return false;
}

ucstring CBaseAction::getActionLocalizedText (const CAction::CName &name) const
{
	// Action base name
	ucstring temp = CI18N::get(LocalizedName);

	// Get the parameter
	uint i;
	for (i=0; i<Parameters.size (); i++)
	{
		bool parameterOk = false;
		const CParameter &parameter = Parameters[i];
		ucstring paramName;
		ucstring paramValue;

		// Get the param from the argu
		if (getParam (parameter.Type, paramName, paramValue, name.Argu, i))
		{
			switch (parameter.Type)
			{
			case CParameter::Hidden:
				if ((ucstring (parameter.DefaultValue) == paramValue) && (ucstring (parameter.Name) == paramName))
					parameterOk = true;
				break;
			case CParameter::Constant:
				{
					uint j;
					for (j=0; j<parameter.Values.size (); j++)
					{
						// This value ?
						const CParameter::CValue &value = parameter.Values[j];
						if (ucstring(value.Value) == paramValue)
						{
							temp += " ";

							if ((value.LocalizedValue.size() >= 2) &&
								(value.LocalizedValue[0]=='u') && (value.LocalizedValue[1]=='i'))
								temp += CI18N::get(value.LocalizedValue);
							else
								temp += value.LocalizedValue;
							parameterOk = true;
							break;
						}
					}
				}
				break;
			case CParameter::User:
			case CParameter::UserName:
				temp += " ";
				temp += paramValue;
				parameterOk = true;
				break;
			}

			// Parameter not found ? Next base action..
			if (!parameterOk)
				break;
		}
	}

	// Found ?
	if (i==Parameters.size ())
		return temp;

	return ucstring("");
}

// ***************************************************************************
// CActionsManager
// ***************************************************************************

const CActionsManager::TComboActionMap &CActionsManager::getComboActionMap () const
{
	return _ComboAction;
}

// ***************************************************************************

const CActionsManager::TActionComboMap &CActionsManager::getActionComboMap () const
{
	return _ActionCombo;
}

// ***************************************************************************

const CActionsManager::CCategoryLocator *CActionsManager::getActionLocator (const CAction::CName &name) const
{
	// Look for the base action
	TActionBaseActionMap::const_iterator ite = _ActionCategory.find (name.Name);
	while ((ite != _ActionCategory.end ()) && (ite->first == name.Name))
	{
		// Ref on the base action
		const CCategory &cat = _Categories[ite->second.CategoryId];
		uint baseActionId = ite->second.BaseActionId;
		uint baseActionSize = cat.BaseActions.size();

		if( ite->second.BaseActionId >= cat.BaseActions.size() )
			return NULL;

		const CBaseAction &baseAction = cat.BaseActions[ite->second.BaseActionId];

		// Check parameters
		uint i;
		uint s = baseAction.Parameters.size();

		for (i=0; i<s; i++)
		{
			bool parameterOk = false;
			const CBaseAction::CParameter &parameter = baseAction.Parameters[i];
			ucstring paramName;
			ucstring paramValue;

			// Get the param from the argu
			if (getParam (parameter.Type, paramName, paramValue, name.Argu, i))
			{
				switch (parameter.Type)
				{
				case CBaseAction::CParameter::Hidden:
					if ((ucstring (parameter.DefaultValue) == paramValue) && (ucstring (parameter.Name) == paramName))
						parameterOk = true;
					break;
				case CBaseAction::CParameter::Constant:
					{
						// If the value of the action param match with one of the values of the base action param so its ok
						uint j;
						for (j=0; j<parameter.Values.size (); j++)
						{
							const CBaseAction::CParameter::CValue &value = parameter.Values[j];
							if (ucstring(value.Value) == paramValue)
							{
								parameterOk = true;
								break;
							}
						}
					}
					break;
				case CBaseAction::CParameter::User:
				case CBaseAction::CParameter::UserName:
					parameterOk = true;
					break;
				}
			}

			// Parameter not found ? Next base action..
			if (!parameterOk)
				break;
		}

		// Found ?
		if (i==baseAction.Parameters.size ())
			return &ite->second;

		ite++;
	}

	return NULL;
}

// ***************************************************************************
const CBaseAction *CActionsManager::getBaseAction (const CAction::CName &name) const
{
	const CCategoryLocator *pCL = getActionLocator(name);
	if (pCL == NULL) return NULL;
	return &_Categories[pCL->CategoryId].BaseActions[pCL->BaseActionId];
}

// ***************************************************************************
void CActionsManager::removeBaseAction(const CAction::CName &name)
{
	const CCategoryLocator *pCL = getActionLocator(name);
	if (pCL == NULL)
	{
		nlwarning("Action %s %s not found.", name.Name.c_str(), name.Argu.c_str());
		return;
	}
	std::vector<CBaseAction> &baseActions = _Categories[pCL->CategoryId].BaseActions;
	baseActions.erase(baseActions.begin() + pCL->BaseActionId);
}

// ***************************************************************************
const CCategory *CActionsManager::getCategory (const CAction::CName &name) const
{
	const CCategoryLocator *pCL = getActionLocator(name);
	if (pCL == NULL) return NULL;
	return &_Categories[pCL->CategoryId];
}


// ***************************************************************************
void	CActionsManager::forceDisplayForAction(const CAction::CName &name, bool state)
{
	if(state)
		_ActionForceDisplay.insert(name);
	else
		_ActionForceDisplay.erase(name);
}

// ***************************************************************************
bool	CActionsManager::isActionDisplayForced(const CAction::CName &name) const
{
	return _ActionForceDisplay.find(name)!=_ActionForceDisplay.end();
}

// ***************************************************************************
ucstring CActionsManager::getActionLocalizedText (const CAction::CName &name) const
{
	const CBaseAction *baseAction= getBaseAction(name);
	if(!baseAction)
		return ucstring();
	return baseAction->getActionLocalizedText(name);
}


// ***************************************************************************
// CActionsContext
// ***************************************************************************


CActionsContext::CActionsContext()
{
	_Context = "game";
}

bool CActionsContext::addActionsManager (CActionsManager *actionManager, const std::string &category)
{
	return _ActionsManagers.insert (TActionsManagerMap::value_type(category, actionManager)).second;
}

// ***************************************************************************
CActionsManager *CActionsContext::getActionsManager (const std::string &category) const
{
	TActionsManagerMap::const_iterator ite = _ActionsManagers.find (category);
	if (ite != _ActionsManagers.end())
		return ite->second;
	else
	{
		ite = _ActionsManagers.find ("");
		if (ite != _ActionsManagers.end())
			return ite->second;
		return NULL;
	}
}

// ***************************************************************************
bool CActionsContext::matchContext(const std::string &contexts) const
{
	std::vector<std::string> contextList;
	splitString(contexts, ",", contextList);
	for (uint k = 0; k < contextList.size(); ++k)
	{
		std::string currContext = contextList[k];
		while(strFindReplace(currContext, " ", ""));
		while(strFindReplace(currContext, "\t", ""));
		if (nlstricmp(currContext, _Context) == 0) return true;
	}
	return false;
}

// ***************************************************************************
void CActionsContext::removeAllCombos()
{
	for (TActionsManagerMap::iterator it = _ActionsManagers.begin(); it != _ActionsManagers.end(); ++it)
	{
		it->second->removeAllCombos();
	}
}

// ***************************************************************************
