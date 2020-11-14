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

#ifndef ADMINISTERED_MODULE_BASE_H
#define ADMINISTERED_MODULE_BASE_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// nel
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"

#include "nel/misc/sstring.h"

// local
#include "module_admin_itf.h"


//-----------------------------------------------------------------------------
// class CAdministeredModuleBase
//-----------------------------------------------------------------------------

class CAdministeredModuleBase:
	public NLNET::CEmptyModuleServiceBehav<NLNET::CEmptyModuleCommBehav<NLNET::CEmptySocketBehav<NLNET::CModuleBase> > >,
	public PATCHMAN::CAdministeredModuleBaseSkel
{
public:
	//---------------------------------------------------------------------------------------------
	// ctors, dtors & init
	CAdministeredModuleBase();

	// initialise the module - must be called at start of derived modules' initModule() method
	NLMISC::CSString init(const NLNET::TParsedCommandLine &initInfo);

	//---------------------------------------------------------------------------------------------
	// hooks for methods that this interface implements and that must be called from the parent class
	void onModuleUp(NLNET::IModuleProxy *module);
	void onModuleDown(NLNET::IModuleProxy *module);
	void onModuleUpdate();
//	bool onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);

	// dissable immediate message dispatching to allow modules on same service to send eachother messages on module up
	bool isImmediateDispatchingSupported() const { return false; }


	//---------------------------------------------------------------------------------------------
	// callbacks on receipt of module messages

	void installVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString& domainName, uint32 version) {}
	void launchVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString& domainName, uint32 version) {}
	void executeCommand(NLNET::IModuleProxy *sender, const NLMISC::CSString &cmdline, const NLMISC::CSString &originator);

	
	//---------------------------------------------------------------------------------------------
	// methods for managing state variables

	// display an nlwarning and call setStateVariable("Error",...) - keeps track of the number of errors encountered
	void registerError(const NLMISC::CSString& value) const;

	// display an nlinfo and call setStateVariable("Latest",...)
	void registerProgress(const NLMISC::CSString& value) const;

	// write accessor for state variables
	void setStateVariable(const NLMISC::CSString& variableName, const NLMISC::CSString& value) const;

	// concatenating write accessor for state variables
	// on return, if state variable was not empty then variable = <old value>' '<new value> else variable = <new value>
	void appendStateVariable(const NLMISC::CSString& variableName, const NLMISC::CSString& value) const;

	// read accessor for state variables
	const NLMISC::CSString& getStateVariable(const NLMISC::CSString& variableName) const;

	// read accessor for getting state as a single complete string
	// the state variables are each listed with their respective values
	NLMISC::CSString getStateString() const;

	// method for clearing state variables
	void clearStateVariable(const NLMISC::CSString& variableName) const;

	// general reset for deleting all state variables
	void clearAllStateVariables() const;

	// broadcast state info to all connected SPM modules
	void broadcastStateInfo() const;

private:
	// state variables map (variable name to value)
	typedef std::map<NLMISC::CSString,NLMISC::CSString> TStateVariables;
	mutable TStateVariables _StateVariables;

	// A set of currently connected ServerPatchManager modules
	typedef std::set<NLNET::IModuleProxy*> TPatchManagers;
	TPatchManagers _PatchManagers;

	// a couple of useful context variables
	bool _Initialised;
	mutable NLMISC::CSString _LastBroadcastState;
	mutable uint32 _ErrorCount;
};


//-----------------------------------------------------------------------------
// class CAdministeredModuleWrapper
//-----------------------------------------------------------------------------

class CAdministeredModuleWrapper
{
public:
	//---------------------------------------------------------------------------------------------
	// ctors, dtors & init
	CAdministeredModuleWrapper();

	// initialise the module - must be called at start of derived modules' initModule() method
	NLMISC::CSString init(CAdministeredModuleBase* administeredModuleBase);

	//---------------------------------------------------------------------------------------------
	// methods for managing state variables

	// display an nlwarning and call setStateVariable("Error",...) - keeps track of the number of errors encountered
	void registerError(const NLMISC::CSString& value) const;

	// display an nlinfo and call setStateVariable("Latest",...)
	void registerProgress(const NLMISC::CSString& value) const;

	// write accessor for state variables
	void setStateVariable(const NLMISC::CSString& variableName, const NLMISC::CSString& value) const;

	// concatenating write accessor for state variables
	// on return, if state variable was not empty then variable = <old value>' '<new value> else variable = <new value>
	void appendStateVariable(const NLMISC::CSString& variableName, const NLMISC::CSString& value) const;

	// method for clearing state variables
	void clearStateVariable(const NLMISC::CSString& variableName) const;

	// general reset for deleting all state variables
	void clearAllStateVariables() const;

	// broadcast state info to all connected SPM modules
	void broadcastStateInfo() const;

private:
	// private data
	CAdministeredModuleBase* _AdministeredModuleBase;
};

#endif
