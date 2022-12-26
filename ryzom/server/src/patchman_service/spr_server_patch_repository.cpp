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

// game share
#include "game_share/utils.h"

// local
#include "spr_server_patch_repository.h"
#include "patchman_constants.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace PATCHMAN;


//-----------------------------------------------------------------------------
// methods CServerPatchRepository
//-----------------------------------------------------------------------------

CServerPatchRepository::CServerPatchRepository()
{
	_NoUpdate= false;

	_AcceptFileRequests= true;
	_AcceptFileInfoRequests= true;

	_ServiceUpdatesPerUpdate= 10;
	_ServiceUpdatesSinceLastUpdate= 0;
	_UpdateCount= 0;
}

bool CServerPatchRepository::initModule(const TParsedCommandLine &initInfo)
{
	// setup a variable to build the init message in...
	NLMISC::CSString logMsg;

	// setup the root directory
	NLMISC::CSString rootDirectory;
	const TParsedCommandLine *targetDir = initInfo.getParam("path");
	if (targetDir != NULL)
	{
		rootDirectory= targetDir->ParamValue;
	}
	else
	{
		registerProgress("SPR: No path() argument found on initialisation - using current directory");
		rootDirectory= NLMISC::CPath::getCurrentPath();
	}
	logMsg+=" Path: "+rootDirectory;

	// look to see if there's a 'noUpdate' argument to deal with
	_NoUpdate= (initInfo.getParam("noUpdate")!=NULL);
	logMsg+= _NoUpdate?" - Auto update dissabled":" - Auto update enabled";

	// initialise base classes...
	logMsg+= CAdministeredModuleBase::init(initInfo);
	CFileRepository::init(this,rootDirectory);

	// we're all done so let the world know
	registerProgress("SPR Initialised: "+logMsg);
	setStateVariable("State","Initialised");
	broadcastStateInfo();
	return true;
}

void CServerPatchRepository::onModuleUp(IModuleProxy *module)
{
	// allow the base classes a chance to do their stuff
	CAdministeredModuleBase::onModuleUp(module);
	CFileRepository::onModuleUp(module);

	if (CSString(module->getModuleManifest()).contains(ManifestEntryIsAdministrator))
	{
		// register with the administrator
		CServerPatchManagerProxy manager(module);
		manager.registerAdministeredModule(this,false,false,false,true);
		registerProgress("registering with manager: "+module->getModuleName());
	}
}

void CServerPatchRepository::onModuleDown(IModuleProxy *module)
{
	// allow the base classes a chance to do their stuff
	CAdministeredModuleBase::onModuleDown(module);
	CFileRepository::onModuleDown(module);
}

//void CServerPatchRepository::onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg)
//{
//	if (CAdministeredModuleBase::onDispatchMessage(sender, msg))
//		return;
//
//	if (CFileRepository::onDispatchMessage(sender, msg))
//		return;
//
//	registerError("ignoring MSG: "+msg.getName());
//	STOP("CServerPatchRepository::onProcessModuleMessage : received unhandled message '"<<msg.getName()<<"'");
//}
	
void CServerPatchRepository::onModuleUpdate()
{
	H_AUTO(CServerPatchRepository_onModuleUpdate);

	// allow the base classes a chance to do their stuff
	CAdministeredModuleBase::onModuleUpdate();
	CFileRepository::onModuleUpdate();

	if (++_ServiceUpdatesSinceLastUpdate>=_ServiceUpdatesPerUpdate)
	{
		if (!_NoUpdate)
		{
			setStateVariable("State",NLMISC::toString("[%d]Scanning",_UpdateCount));
			broadcastStateInfo();
			CFileRepository::rescanPartial();
		}

		setStateVariable("State",NLMISC::toString("[%d]Idle",_UpdateCount));
		_ServiceUpdatesSinceLastUpdate= 0;
	}
}

std::string CServerPatchRepository::buildModuleManifest() const
{
	return CFileRepository::buildModuleManifest();
}

bool CServerPatchRepository::cbValidateDownloadRequest(const NLNET::IModuleProxy *sender,const std::string &fileName)
{
	// by default we return true...
	return _AcceptFileRequests;
}

bool CServerPatchRepository::cbValidateFileInfoRequest(const NLNET::IModuleProxy *sender,const std::string &fileName)
{
	// by default we return true...
	return _AcceptFileInfoRequests;
}

void CServerPatchRepository::cbFileInfoRescanning(const NLMISC::CSString& fileName,uint32 fileSize)
{
	registerProgress("SPR: Calculate Checksum: "+fileName+toString(" (%u bytes)",fileSize));
	setStateVariable("State","Scanning");
	broadcastStateInfo();
}


NLMISC_CLASS_COMMAND_IMPL(CServerPatchRepository, dump)
{
	if (args.size()!=0)
		return false;

	NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);
	CFileRepository::dump(log);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchRepository, AcceptGetFileRequests)
{
	switch(args.size())
	{
	case 1:
		if (args[0]=="1" || args[0]=="true")
		{
			_AcceptFileRequests= true;
		}
		else if (args[0]=="0" || args[0]=="false")
		{
			_AcceptFileRequests= false;
		}
		else 
			break;
		// drop through...

	case 0:
		log.displayNL("AcceptGetFileRequests %s",_AcceptFileRequests?"true":"false");
		return true;
	}

	return false;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchRepository, AcceptGetInfoRequests)
{
	switch(args.size())
	{
	case 1:
		if (args[0]=="1" || args[0]=="true")
		{
			_AcceptFileInfoRequests= true;
		}
		else if (args[0]=="0" || args[0]=="false")
		{
			_AcceptFileInfoRequests= false;
		}
		else 
			break;
		// drop through...

	case 0:
		log.displayNL("AcceptGetInfoRequests %s",_AcceptFileInfoRequests?"true":"false");
		return true;
	}

	return false;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchRepository, ServiceUpdatesPerUpdate)
{
	switch(args.size())
	{
	case 1:
		{
			uint32 newVal= NLMISC::CSString(args[0]).atoui();
			if (newVal==0 && args[0]!="0")
				break;
			_ServiceUpdatesPerUpdate= newVal;
		}
		// drop through...

	case 0:
		log.displayNL("ServiceUpdatesPerUpdate %u",_ServiceUpdatesPerUpdate);
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------
// CServerPatchRepository registration
//-----------------------------------------------------------------------------

NLNET_REGISTER_MODULE_FACTORY(CServerPatchRepository, "ServerPatchRepository");

