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



#ifndef CL_ACTIONS_H
#define CL_ACTIONS_H


/////////////
// INCLUDE //
/////////////

// client
#include "events_listener.h"

///////////
// CLASS //
class CCombo;
class CActionsManager;
class CAction;

/**
 * The goal of CCombo is to gather together Inputs that will validate an Action.
 * For now, CCombo is composed of keyboard inputs or mouse inputs
 * (not Keyboard and mouse inputs in the same CCombo).
 *
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 *
 * \todo Load/Save.
 * \todo Check if there is memory leak.
 * \todo Check comments.
 */
class CCombo
{
public:

	// The key
	NLMISC::TKey					Key;

	// The CTRL - SHIFT - ALT state
	NLMISC::TKeyButton				KeyButtons;

public:

	/// Init the combo
	void init (NLMISC::TKey key, NLMISC::TKeyButton keyButtons);

	/// Get the combo in human readable form
	ucstring toUCString() const;

	// For maps
	bool operator<(const CCombo &other) const
	{
		if (Key < other.Key)
			return true;
		if (Key > other.Key)
			return false;
		return KeyButtons < other.KeyButtons;
	}
};


/**
 * The Goal of CAction is to know the state of an Action.
 *
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 *
 * \todo Check comments.
 */
class CAction
{
public:

	// The name of an action
	class CName
	{
	public:
		std::string		Name;
		std::string		Argu;

		CName()
		{
		}

		CName(const char *name)
		{
			Name = name;
		}

		CName(const char *name, const char *argu)
		{
			Name = name;
			Argu = argu;
		}


		// For the maps
		bool operator<(const CName &other) const
		{
			if (Name < other.Name)
				return true;
			if (Name > other.Name)
				return false;
			return Argu < other.Argu;
		}

		// For the maps
		bool operator== (const CName &other) const
		{
			return (Name == other.Name) && (Argu == other.Argu);
		}
	};

	// The icon of an action
	class CIcon
	{
	public:

		// The background color
		NLMISC::CRGBA	Color;

		// The bitmaps (1 over 0)
		std::string		Bitmaps[2];

		// The text (6 letters, only A-Z 0-9)
		std::string		Text;
	};

	/// Constructor
	CAction();

	// Execute the action
	void	runAction ();

	// True if the action has begun	(key down..)
	bool	Valide;

	// Repetition for this action ?
	bool	Repeat;

	// Do we have to run an AH on key up ?
	bool	KeyUp;

	// Do we have to run an AH on key down ?
	bool	KeyDown;

	// The name of the action
	CName	Name;
};

/*
 *	The base action is a class used to generate actions
 */
class CBaseAction
{
public:

	// Init default values
	CBaseAction ();

	// The parameter descriptor of an action
	class CParameter
	{
	public:
		enum TType
		{
			Hidden=0,
			Constant,	// The parameter is a list of const string
			User,		// The parameter is a user string
			UserName,	// The parameter is a user "name" string
		}				Type;

		// Init default values
		CParameter ();

		// The parameter name (optional)
		std::string		Name;

		// The parameter localized name (optional)
		std::string		LocalizedName;

		// Default value
		std::string		DefaultValue;

		// The parameter constant value is Type == Constant
		class CValue
		{
		public:
			// The value
			std::string		Value;

			// contexts in which this value is possible
			std::string		Contexts;

			// The localized value
			std::string		LocalizedValue;
		};
		std::vector<CValue>	Values;
	};

	// The action parameter descritors
	std::vector<CParameter>		Parameters;

	// Repetition for this action ?
	bool	Repeat;

	// Do we have to run an AH on key up ?
	bool	KeyUp;

	// Do we have to run an AH on key down ?
	bool	KeyDown;

	// Do we have to wait for an answer from the server before continuing execution in macros ?
	bool	WaitForServer;

	// Is this action can be macroized?
	bool	Macroisable;

	// The name of the action handler used by the action
	std::string		Name;

	// The localized name
	std::string		LocalizedName;

	// Contexts in which this action exists
	std::string		Contexts;

	/// Get an action localized text
	ucstring getActionLocalizedText (const CAction::CName &name) const;

	// see if there's at least one set of parameters for which this action is usable in current context
	bool	isUsableInCurrentContext() const;

	// The action icon
	// CAction::CIcon	Icon;
};

/*
 * Category of action
 */
class CCategory
{
public:
	// The category name
	std::string					Name;

	// The category localized name
	std::string					LocalizedName;

	// The set of base action for this category
	std::vector<CBaseAction>	BaseActions;

	// Is this whole category can be macroized?
	bool	Macroisable;

public:
	CCategory()
	{
		Macroisable= true;
	}
};

// HashMapTraits for NLMISC::TKey
struct CTKeyHashMapTraits
{
	static const size_t bucket_size = 4;
	static const size_t min_buckets = 8;
	CTKeyHashMapTraits() { }
	size_t operator() (NLMISC::TKey key) const
	{
		return (size_t)key;
	}
	bool operator() (NLMISC::TKey key1, NLMISC::TKey key2) const
	{
		return key1 < key2;
	}
};

/**
 * The aims of CActionsManager ...
 *
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 *
 * \todo DelCombo(...);
 * \todo Load/Save.
 * \todo Check if there is memory leak.
 * \todo Check comments.
 */
class CActionsManager
{
public:

	/// Typedef to use the map easily.
	typedef std::map<CAction::CName, CAction>	TActionsMap;
	typedef std::set<CAction::CName>			TActionsForceDisplaySet;
	typedef std::map<CAction::CName, CCombo>	TActionComboMap;
	typedef std::map<CCombo, CAction::CName>	TComboActionMap;
	typedef CHashMultiMap<NLMISC::TKey, CAction::CName, CTKeyHashMapTraits>	TKeyActionMap;

	// Category locator
	class CCategoryLocator
	{
	public:
		uint	CategoryId;
		uint	BaseActionId;
	};

	typedef CHashMultiMap<std::string, CCategoryLocator>	TActionBaseActionMap;

	/// Constructor
	CActionsManager();

	/// Add a new Action in the context.
	bool addAction(const CAction::CName &name);

	/// Return an action from its name
	CAction* getAction(const CAction::CName &name);

	/// Clear actions
	void clear ();

	/// Enable / disable combos
	void enable (bool enable);

	/// Remove a combo
	void removeCombo (const CCombo &combo);

	// Remove all the combos
	void removeAllCombos();

	/// Add a combo in an Action.
	bool addCombo(const CAction::CName &name, const CCombo &combo, bool createAction = true);

	/// Get the base action and category of an action
	const CCategoryLocator *getActionLocator (const CAction::CName &name) const;

	/// Get the base action of an action
	const CBaseAction *getBaseAction (const CAction::CName &name) const;

	// remove an action from the action manager
	void removeBaseAction(const CAction::CName &name);

	/// Get the category of an action
	const CCategory *getCategory (const CAction::CName &name) const;

	/// Return if the Action is valide.
	bool valide(const CAction::CName &name) const;

	// Return true if the action is present in current (global) context
	bool isActionPresentInContext(const CAction::CName &name) const;

	/// A key has been pushed. Return true if an action handler is associated with this key.
	bool keyPushed (const NLMISC::CEventKeyDown &keyDown);

	/// A key has been released
	void keyReleased (const NLMISC::CEventKeyUp &keyUp);

	/// Get the category array
	const std::vector<CCategory>	&getCategories () const;

	/// Reserve space in the category array
	void	reserveCategories (uint space);

	/// Add a category
	void	addCategory (const CCategory &category);

	/// Remove a category
	void	removeCategory (const std::string &catName);

	/// Get combo / action map
	const TComboActionMap	&getComboActionMap () const;

	/// Get action / combo map
	const TActionComboMap	&getActionComboMap () const;

	/// Release All keys, without running any AH
	void	releaseAllKeyNoRunning();

	/// true if a combo is already associated to an action
	bool	isComboAssociated(const CCombo &combo) const {return _ComboAction.find(combo)!=_ComboAction.end();}

	/// true if an action is already associated to a combo
	bool	isActionAssociated(const CAction::CName &name) const {return _ActionCombo.find(name)!=_ActionCombo.end();}

	/// Force the action to be always displayed in the "Keys" interface, even if the key is unbound. default is false
	void	forceDisplayForAction(const CAction::CName &name, bool state);

	/// see forceDisplayForAction
	bool	isActionDisplayForced(const CAction::CName &name) const;

	/// see forceDisplayForAction
	const TActionsForceDisplaySet	&getActionsForceDisplaySet() const {return _ActionForceDisplay;}

	/// Get an action localized text
	ucstring getActionLocalizedText (const CAction::CName &name) const;

	//@}

private:

	/// Update key buttons
	void updateKeyButton (NLMISC::TKeyButton);

	/// Map with all Actions in the context. the string = Action name.
	TActionsMap				_Actions;
	TActionsForceDisplaySet	_ActionForceDisplay;
	TActionComboMap			_ActionCombo;
	TComboActionMap			_ComboAction;
	TKeyActionMap			_KeyAction;
	TKeyActionMap			_WatchedActions;
	TActionBaseActionMap	_ActionCategory;
	std::vector<CCategory>	_Categories;

	bool					_Enabled;

};// CActionsManager //

/**
 * Action context
 */
class CActionsContext
{
public:
	CActionsContext();

	// Add an action manager.
	bool addActionsManager (CActionsManager *actionManager, const std::string &category="");

	// Get an action manager. Returns NULL if not found.
	CActionsManager *getActionsManager (const std::string &category="") const;

	void setContext(const std::string &context) { _Context = context; }
	bool matchContext(const std::string &contexts) const;

	void removeAllCombos();

private:
	typedef std::map<std::string, CActionsManager *>	TActionsManagerMap;
	TActionsManagerMap	_ActionsManagers;
	std::string			_Context;
};

extern CActionsContext ActionsContext;

#endif // CL_ACTIONS_H

/* End of actions.h */
