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
// local
#include "administered_module.h"
#include "file_repository.h"
#include "patchman_constants.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace PATCHMAN;


//-----------------------------------------------------------------------------
// class CClientPatchRepository
//-----------------------------------------------------------------------------

class CClientPatchRepository: 
	public CAdministeredModuleBase,
	public CFileRepository
{
public:
	// ctor
	CClientPatchRepository();

	// CModuleBase specialisation implementation
	bool initModule(const NLNET::TParsedCommandLine &initInfo);
	void onModuleUp(NLNET::IModuleProxy *module);
	void onModuleDown(NLNET::IModuleProxy *module);
//	void onProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &msg);
	void onModuleUpdate();
	std::string buildModuleManifest() const;
	static const std::string &getHelperString();

	// make sure module system doesn't misbehave when modules are on the same service
	bool isImmediateDispatchingSupported() const { return false; }

protected:
	// IFileRequestValidator specialisation implementation
	bool cbValidateFileInfoRequest(const NLNET::IModuleProxy *sender,const std::string &fileName);

private:
	// private data
	CSString _RootDirectory;
	uint32 _RepositoryVersion;
	uint32 _LastVersionFileTimeStamp;

	uint32 _ServiceUpdatesPerUpdate;
	uint32 _ServiceUpdatesSinceLastUpdate;
	uint32 _UpdateCount;

protected:
	// NLMISC_COMMANDs for this module
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CClientPatchRepository, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CClientPatchRepository, dump, "Dump the current status", "no args")
		NLMISC_COMMAND_HANDLER_ADD(CClientPatchRepository, ServiceUpdatesPerUpdate, "number of service loops per update", "[<num updates (default=10)>]")
		// add commands defined in base classes
		NLMISC_COMMAND_HANDLER_ADDS_FOR_FILE_REPOSITORY(CClientPatchRepository)
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump);
	NLMISC_CLASS_COMMAND_DECL(ServiceUpdatesPerUpdate);
};


//-----------------------------------------------------------------------------
// handy utilities
//-----------------------------------------------------------------------------

static uint32 getFileVersion(const NLMISC::CSString& fileName)
{
	// start at the back of the file name and scan forwards until we find a '/' or '\\' or ':' or a digit
	uint32 i= (uint32)fileName.size();
	while (i--)
	{
		char c= fileName[i];

		// if we've hit a directory name separator then we haven't found a version number so drop out
		if (c=='/' || c=='\\' || c==':')
			return ~0u;

		// if we've found a digit then construct the rest of the version number and return
		if (isdigit(c))
		{
			uint32 firstDigit= i;
			while (firstDigit!=0 && isdigit(fileName[firstDigit-1]))
			{
				--firstDigit;
			}
			return fileName.leftCrop(firstDigit).left(i-firstDigit+1).atoui();
		}
	}

	// default to our 'invalid' value
	return ~0u;
}


//-----------------------------------------------------------------------------
// methods CClientPatchRepository
//-----------------------------------------------------------------------------

CClientPatchRepository::CClientPatchRepository()
{
	_RepositoryVersion=0;
	_LastVersionFileTimeStamp=0;

	_ServiceUpdatesPerUpdate=50;
	_ServiceUpdatesSinceLastUpdate= _ServiceUpdatesPerUpdate;
	_UpdateCount=0;
}

bool CClientPatchRepository::initModule(const TParsedCommandLine &initInfo)
{
	// setup a variable to build the init message in...
	NLMISC::CSString logMsg;

	// setup the root directory
	const TParsedCommandLine *targetDir = initInfo.getParam("path");
	if (targetDir != NULL)
	{
		_RootDirectory= targetDir->ParamValue;
	}
	else
	{
		registerProgress("CPR: No path() argument found on initialisation - using current directory");
		_RootDirectory= NLMISC::CPath::getCurrentPath();
	}
	_RootDirectory= NLMISC::CPath::standardizePath(_RootDirectory);
	logMsg+=" Path: "+_RootDirectory;

	// initialise base classes...
	logMsg+= CAdministeredModuleBase::init(initInfo);
	CFileRepository::init(this,_RootDirectory);

	// we're all done so let the world know
	registerProgress("CPR Initialised: "+logMsg);
	setStateVariable("State","Initialised");
	broadcastStateInfo();

	return true;
}

void CClientPatchRepository::onModuleUp(IModuleProxy *module)
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

void CClientPatchRepository::onModuleDown(IModuleProxy *module)
{
	// allow the base classes a chance to do their stuff
	CAdministeredModuleBase::onModuleDown(module);
	CFileRepository::onModuleDown(module);
}

//void CClientPatchRepository::onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg)
//{
//	if (CAdministeredModuleBase::onDispatchMessage(sender, msg))
//		return;
//
//	if (CFileRepository::onDispatchMessage(sender, msg))
//		return;
//
//	registerError("ignoring MSG: "+msg.getName());
//	STOP("CClientPatchRepository::onProcessModuleMessage : received unhandled message '"<<msg.getName()<<"'");
//}
//	
void CClientPatchRepository::onModuleUpdate()
{
	H_AUTO(CClientPatchRepository_onModuleUpdate);
	// allow the base classes a chance to do their stuff
	CAdministeredModuleBase::onModuleUpdate();
	CFileRepository::onModuleUpdate();

	// no need to update too often...
	if (++_ServiceUpdatesSinceLastUpdate>=_ServiceUpdatesPerUpdate)
	{
		_ServiceUpdatesSinceLastUpdate= 0;
		setStateVariable("State",NLMISC::toString("Running(%u)",_UpdateCount));

		// if the file containing the version number exists and has changed since the last update then re-read it
		CSString fileName= _RootDirectory+ "version";
		if (!NLMISC::CFile::fileExists(fileName))
			return;

		uint32 versionFileTimeStamp= NLMISC::CFile::getFileModificationDate(fileName);
		if (versionFileTimeStamp==_LastVersionFileTimeStamp)
			return;

		// record the new version time stamp fro later use
		_LastVersionFileTimeStamp= versionFileTimeStamp;

		// extract the version number from the file...
		CSString fileContents;
		fileContents.readFromFile(fileName);
		uint32 newVersion= fileContents.strip().atoui();

		if (newVersion!=_RepositoryVersion)
		{
			_RepositoryVersion= newVersion;
			registerProgress("version => "+fileContents.strip().quoteIfNotAtomic());

			setStateVariable("State",NLMISC::toString("Scanning (%u)",_UpdateCount));
			broadcastStateInfo();
			CFileRepository::rescanFull();
			setStateVariable("State",NLMISC::toString("Running(%u)",_UpdateCount));
		}
	}
}

std::string CClientPatchRepository::buildModuleManifest() const
{
	return CFileRepository::buildModuleManifest();
}

bool CClientPatchRepository::cbValidateFileInfoRequest(const NLNET::IModuleProxy *sender,const std::string &fileName)
{
	// extract the version number from the file name, compare with our current version number ...
	return getFileVersion(fileName) <= _RepositoryVersion;
}


NLMISC_CLASS_COMMAND_IMPL(CClientPatchRepository, dump)
{
	if (args.size()!=0)
		return false;

	NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);
	CFileRepository::dump(log);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CClientPatchRepository, ServiceUpdatesPerUpdate)
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
// CClientPatchRepository registration
//-----------------------------------------------------------------------------

NLNET_REGISTER_MODULE_FACTORY(CClientPatchRepository, "ClientPatchRepository");

