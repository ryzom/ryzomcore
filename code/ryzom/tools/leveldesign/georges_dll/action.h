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

#ifndef GEORGES_EDIT_ACTION_H
#define GEORGES_EDIT_ACTION_H

#include "nel/misc/types_nl.h"

// Document modification action
class IAction
{
	friend class CGeorgesEditDoc;
public:

	// Type of modification performed
	enum TTypeAction
	{
		HeaderVersion,
		HeaderState,
		HeaderComments,
		TypeType,
		TypeUI,
		TypeDefault,
		TypeMin,
		TypeMax,
		TypeIncrement,
		TypePredef,
		DfnParents,
		DfnStructure,
		FormParents,
		FormArraySize,
		FormArrayRename,
		/*FormArrayReplace,
		FormArrayAppend,*/
		FormArrayInsert,
		FormArrayDelete,
		FormVirtualDfnName,
		FormValue,
		FormTypeValue,
		FormPaste,
	};

protected:

	// Constructor
	IAction (TTypeAction type, uint selId, uint slot);

	// Init log label
	void setLabel (const char *logLabel, CGeorgesEditDoc &doc);

public:

	// Virtual destructor
	virtual ~IAction () {};

protected:

	// Action type
	TTypeAction		_Type;

	// Original value ?
	bool			_Original;

	// Selection Id of the Undo
	uint			_SelId;

	// Action slot
	uint			_Slot;

	// Old and new value
	bool			_LogPresent[2];
	std::string		_Log[2];
	std::string		_LogLabel;

	// Do the modification
	virtual bool	doAction (CGeorgesEditDoc &doc, bool redo, bool &modified, bool firstTime);

	// Update the views
	enum TUpdateRightView
	{
		DoNothing,
		UpdateLabels,
		UpdateValues,
		Redraw
	};
	void			update (bool updateLeftView, TUpdateRightView rightView, CGeorgesEditDoc &doc, const char *_FormName);
};

// String modification action
class CActionString : public IAction
{
public:

	// Constructor
	CActionString (IAction::TTypeAction type, const char *newValue, CGeorgesEditDoc &doc, const char *formName, const char *userData, uint selId, uint slot);

protected:

	// Old and new value
	std::string		_Value[2];

	// The form name
	std::string		_FormName;

	// The form name
	std::string		_UserData;

	// Do the modification
	virtual bool	doAction (CGeorgesEditDoc &doc, bool redo, bool &modified, bool firstTime);
};

// Vector string modification action
class CActionStringVector : public IAction
{
public:

	// Constructor
	CActionStringVector (IAction::TTypeAction type, const std::vector<std::string> &stringVector, CGeorgesEditDoc &doc, const char *formName, uint selId, uint slot);

protected:

	// Old and new value
	std::vector<std::string>	_Value[2];

	// The form name
	std::string		_FormName;

	// Do the modification
	virtual bool	doAction (CGeorgesEditDoc &doc, bool redo, bool &modified, bool firstTime);
};

// Vector vector string modification action
class CActionStringVectorVector : public IAction
{
public:

	// Constructor
	CActionStringVectorVector (IAction::TTypeAction type, const std::vector<std::vector<std::string> > &stringVectorVector, CGeorgesEditDoc &doc, uint selId, uint slot);

protected:

	// Old and new value
	std::vector<std::vector<std::string> >	_Value[2];

	// Do the modification
	virtual bool	doAction (CGeorgesEditDoc &doc, bool redo, bool &modified, bool firstTime);
};

// Vector vector string modification action
class CActionBuffer : public IAction
{
public:

	// Constructor
	CActionBuffer (IAction::TTypeAction type, const uint8 *buffer, uint bufferSize, CGeorgesEditDoc &doc, const char *formName, const char *userData, uint selId, uint slot);

protected:

	// Old and new value
	std::vector<uint8>		_Value[2];

	// The form name
	std::string		_FormName;

	// User data
	std::string		_UserData;

	// Do the modification
	virtual bool	doAction (CGeorgesEditDoc &doc, bool redo, bool &modified, bool firstTime);
};

#endif GEORGES_EDIT_ACTION_H
