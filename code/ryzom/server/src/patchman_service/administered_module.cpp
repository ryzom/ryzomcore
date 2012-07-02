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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "administered_module.h"
#include "module_admin_itf.h"
#include "patchman_tester.h"
#include "patchman_constants.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace PATCHMAN;


//-----------------------------------------------------------------------------
// methods CAdministeredModuleBase - ctors / dtors
//-----------------------------------------------------------------------------

CAdministeredModuleBase::CAdministeredModuleBase()
{
	_ErrorCount= 0;
	_Initialised= false;
}

NLMISC::CSString CAdministeredModuleBase::init(const TParsedCommandLine &initInfo)
{
	CAdministeredModuleBaseSkel::init(this);
	// prevent double initialialisation (given that we are using virtual inherritance, 
	// this init can be called more than once for the same module)
	if (_Initialised)
		return "";
	_Initialised= true;

	// initialise the module base...
	CModuleBase::initModule(initInfo);

	// initialise the state variables
	setStateVariable("State","Initialising");
	broadcastStateInfo();

	return "";
}


//-----------------------------------------------------------------------------
// methods CAdministeredModuleBase - hooks for methods that this interface implements and that must be called from the parent class
//-----------------------------------------------------------------------------

void CAdministeredModuleBase::onModuleUp(IModuleProxy *module)
{
	// if the module coming up is an SPM module then we call it 'dad'
	if (CSString(module->getModuleManifest()).contains(ManifestEntryIsAdministrator))
	{
		_PatchManagers.insert(module);
		registerProgress("ServerPatchManager Connected: "+module->getModuleName()+" ("+module->getModuleManifest()+")");
	}
}

void CAdministeredModuleBase::onModuleDown(IModuleProxy *module)
{
	// if the module going down is an SPM module then remove it from our lists
	if (_PatchManagers.find(module)!=_PatchManagers.end())
	{
		_PatchManagers.erase(module);
		registerProgress("ServerPatchManager Disconnected: "+module->getModuleName()+" ("+module->getModuleManifest()+")");
	}
}

void CAdministeredModuleBase::onModuleUpdate()
{
	H_AUTO(CAdministeredModuleBase_onModuleUpdate);
	// if the state has changed then broadcast the new state info
	if (_LastBroadcastState != getStateString())
	{
		broadcastStateInfo();
	}
}

//bool CAdministeredModuleBase::onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
//{
//	return CAdministeredModuleBaseSkel::onDispatchMessage(sender,message);
//}


//-----------------------------------------------------------------------------
// methods CAdministeredModuleBase - callbackf for treating module messages
//-----------------------------------------------------------------------------

void CAdministeredModuleBase::executeCommand(NLNET::IModuleProxy *sender, const NLMISC::CSString &cmdline, const NLMISC::CSString &originator)
{
	// create a displayer to gather the output of the command
	class SStringDisplayer: public IDisplayer
	{
	public:
		CSString Data;
		void doDisplay( const CLog::TDisplayInfo& args, const char *message)
		{
			Data += message;
		}
	};
	CLog log;
	SStringDisplayer stringDisplayer;
	log.addDisplayer(&stringDisplayer);

	// execute the command
	registerProgress(NLMISC::toString("exec '%s' (via '%s'): '%s'", originator.c_str(), sender->getModuleName().c_str(), cmdline.c_str()));
	ICommand::execute(this->getModuleName()+'.'+cmdline.strip(),log);

	// send a reply message to the originating service
	CServerPatchManagerProxy manager(sender);
	manager.executedCommandResult(this,originator,cmdline,stringDisplayer.Data);
}


//-----------------------------------------------------------------------------
// methods CAdministeredModuleBase - methods for managing state variables
//-----------------------------------------------------------------------------

void CAdministeredModuleBase::registerError(const NLMISC::CSString& value) const
{
	++_ErrorCount;
	setStateVariable("Error",NLMISC::toString("[%d]",_ErrorCount)+value);
	nlwarning("%s: ERROR: %s",CModuleBase::getModuleName().c_str(),value.c_str());
}

void CAdministeredModuleBase::registerProgress(const NLMISC::CSString& value) const
{
	setStateVariable("Latest",value);
	nlinfo("%s: %s",CModuleBase::getModuleName().c_str(),value.c_str());
}

void CAdministeredModuleBase::setStateVariable(const NLMISC::CSString& variableName, const NLMISC::CSString& value) const
{
	// if the state variable is in fact unchanged then just drop out as there is nothing real to do
	if (_StateVariables[variableName] == value)
	{
		return;
	}

	// set the state variable value
	_StateVariables[variableName]= value;
}

void CAdministeredModuleBase::appendStateVariable(const NLMISC::CSString& variableName, const NLMISC::CSString& value) const
{
	// if the state variable is in fact unchanged then just drop out as there is nothing real to do
	if (value.empty())
	{
		return;
	}

	// if the state variable isn't currently empty then add a separating space before the new value
	if (!_StateVariables[variableName].empty())
	{
		_StateVariables[variableName]+= ' ';
	}

	// append the new value to the state variable
	_StateVariables[variableName]+= value;
}

const NLMISC::CSString& CAdministeredModuleBase::getStateVariable(const NLMISC::CSString& variableName) const
{
	// setup a static 'emptyString' to return a refference to if requested variables are not found
	static NLMISC::CSString emptyString;

	return (_StateVariables.find(variableName)==_StateVariables.end())? emptyString: _StateVariables[variableName];
}

NLMISC::CSString CAdministeredModuleBase::getStateString() const
{
	// setup a result variable to build the output string in
	NLMISC::CSString result;

	// add each variable to the result string in turn
	for (TStateVariables::iterator it= _StateVariables.begin(); it!=_StateVariables.end(); ++it)
	{
		if (!result.empty())
		{
			result+=", ";
		}
		result+= it->first;
		result+= "=";
		result+= it->second.quoteIfNotAtomic();
	}

	// all done so return the constructed string
	return result;
}

void CAdministeredModuleBase::clearStateVariable(const NLMISC::CSString& variableName) const
{
	_StateVariables.erase(variableName);
}

void CAdministeredModuleBase::clearAllStateVariables() const
{
	_StateVariables.clear();
}

void CAdministeredModuleBase::broadcastStateInfo() const
{
	// generate the state string that we need to dispatch to any listening modules
	CSString stateString= getStateString();

	// send an update message to any connected SPM modules
	for (TPatchManagers::const_iterator it=_PatchManagers.begin(); it!=_PatchManagers.end(); ++it)
	{
		CServerPatchManagerProxy spm(*it);
		spm.declareState(const_cast<CAdministeredModuleBase*>(this),stateString);
	}

	// after broadcasting the state info the 'state has changed' flag can be set to false
	_LastBroadcastState= stateString;
}

//-----------------------------------------------------------------------------
// class CAdministeredModuleWrapper
//-----------------------------------------------------------------------------

CAdministeredModuleWrapper::CAdministeredModuleWrapper()
{
	_AdministeredModuleBase= NULL;
}

NLMISC::CSString CAdministeredModuleWrapper::init(CAdministeredModuleBase* administeredModuleBase)
{
	_AdministeredModuleBase= administeredModuleBase;
	return CSString();
}

void CAdministeredModuleWrapper::registerError(const NLMISC::CSString& value) const
{
	if (_AdministeredModuleBase!=NULL)
	{
		_AdministeredModuleBase->registerError(value);
	}
	else
	{
		nlwarning("Error: %s",value.c_str());
	}
}

void CAdministeredModuleWrapper::registerProgress(const NLMISC::CSString& value) const
{
	if (_AdministeredModuleBase!=NULL)
	{
		_AdministeredModuleBase->registerProgress(value);
	}
	else
	{
		nlinfo("Latest: %s",value.c_str());
	}
}

void CAdministeredModuleWrapper::setStateVariable(const NLMISC::CSString& variableName, const NLMISC::CSString& value) const
{
	if (_AdministeredModuleBase!=NULL)
	{
		_AdministeredModuleBase->setStateVariable(variableName,value);
	}
	else
	{
		nldebug("Setting: %s = %s",variableName.c_str(),value.c_str());
	}
}

void CAdministeredModuleWrapper::appendStateVariable(const NLMISC::CSString& variableName, const NLMISC::CSString& value) const
{
	if (_AdministeredModuleBase!=NULL)
	{
		_AdministeredModuleBase->appendStateVariable(variableName,value);
	}
	else
	{
		nldebug("Appending: %s += %s",variableName.c_str(),value.c_str());
	}
}

void CAdministeredModuleWrapper::clearStateVariable(const NLMISC::CSString& variableName) const
{
	if (_AdministeredModuleBase!=NULL)
	{
		_AdministeredModuleBase->clearStateVariable(variableName);
	}
	else
	{
		nldebug("Clearing: %s",variableName.c_str());
	}
}

void CAdministeredModuleWrapper::clearAllStateVariables() const
{
	if (_AdministeredModuleBase!=NULL)
	{
		_AdministeredModuleBase->clearAllStateVariables();
	}
	else
	{
		nlinfo("Clearing all state variables...");
	}
}

void CAdministeredModuleWrapper::broadcastStateInfo() const
{
	if (_AdministeredModuleBase!=NULL)
	{
		_AdministeredModuleBase->broadcastStateInfo();
	}
}

