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

// nel
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"

// game share
#include "game_share/deployment_configuration.h"
#include "game_share/utils.h"

// local
#include "module_admin_itf.h"
#include "deployment_configuration_synchroniser.h"
#include "patchman_constants.h"

#include "server_share/mysql_wrapper.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace PATCHMAN;
using namespace DEPCFG;


//-----------------------------------------------------------------------------
// constants
//-----------------------------------------------------------------------------

static CSString DomainListFileName("domain_versions.txt");
static CSString NamedVersionsFileName("named_versions.txt");


//-----------------------------------------------------------------------------
// handy utils
//-----------------------------------------------------------------------------

static void writeDomainListFile(const TDomains& domains)
{
	// setup a variable to hold the desired file contents
	CSString outputString="// List of domains with version numbers in format: <domain name> <launch version> <install version>\n\n";

	// run through the domains container adding entries to our output string
	for (TDomains::const_iterator it= domains.begin(); it!=domains.end();++it)
	{
		outputString+= it->first;
		outputString+= ' ';
		outputString+= NLMISC::toString(it->second.LaunchVersion);
		outputString+= ' ';
		outputString+= NLMISC::toString(it->second.InstallVersion);
		outputString+= '\n';
	}

	// write the result to the domain list file (for future refference)
	outputString.writeToFile(DomainListFileName);
}

static void readDomainListFile(TDomains& domains)
{
	// clear out the result container before we begin
	domains.clear();

	// load the domain list file
	nlinfo("Loading domain versions from file: %s",DomainListFileName.c_str());
	CSString domainListFile;
	domainListFile.readFromFile(DomainListFileName);
	CVectorSString domainListFileLines;
	domainListFile.splitLines(domainListFileLines);

	// treat the lines of the file one by one...
	for (uint32 i=0;i<domainListFileLines.size();++i)
	{
		// skip comments if there are any
		CSString line= domainListFileLines[i].splitTo("//").strip();
		if (line.empty()) continue;

		// decompose the line and make sure the result is valid
		CSString domainName= line.strtok(" \t");
		uint32 launchVersion= line.strtok(" \t").atoui();
		uint32 installVersion= line.atoui();
		DROP_IF(installVersion==0,DomainListFileName+NLMISC::toString(": %u: ",i)+"Skipping invalid line: "+domainListFileLines[i],continue);

		// store away the result and display an info message
		domains[domainName].LaunchVersion= launchVersion;
		domains[domainName].InstallVersion= installVersion;
		nlinfo("- Setting launch version to %u and install version to %u for domain: %s",launchVersion,installVersion,domainName.c_str());
	}
}

static void writeNamedVersionFile(const TNamedVersions& namedVersions)
{
	// setup a variable to hold the desired file contents
	CSString outputString="// List of named versions in format: <version name> <client version> <server version>\n\n";

	// run through the domains container adding entries to our output string
	for (TNamedVersions::const_iterator it= namedVersions.begin(); it!=namedVersions.end();++it)
	{
		outputString+= it->first;
		outputString+= ' ';
		outputString+= NLMISC::toString(it->second.ClientVersion);
		outputString+= ' ';
		outputString+= NLMISC::toString(it->second.ServerVersion);
		outputString+= '\n';
	}

	// write the result to the domain list file (for future refference)
	outputString.writeToFile(NamedVersionsFileName);
}

static void readNamedVersionFile(TNamedVersions& namedVersions)
{
	// clear out the result container before we begin
	namedVersions.clear();

	// load the domain list file
	nlinfo("Loading named versions from file: %s",NamedVersionsFileName.c_str());
	CSString namedVersionsFile;
	namedVersionsFile.readFromFile(NamedVersionsFileName);
	CVectorSString namedVersionsFileLines;
	namedVersionsFile.splitLines(namedVersionsFileLines);

	// treat the lines of the file one by one...
	for (uint32 i=0;i<namedVersionsFileLines.size();++i)
	{
		// skip comments if there are any
		CSString line= namedVersionsFileLines[i].splitTo("//").strip();
		if (line.empty()) continue;

		// decompose the line and make sure the result is valid
		CSString versionName= line.strtok(" \t");
		uint32 clientVersion= line.strtok(" \t").atoui();
		uint32 serverVersion= line.atoui();
		DROP_IF(serverVersion==0,NamedVersionsFileName+NLMISC::toString(": %u: ",i)+"Skipping invalid line: "+namedVersionsFileLines[i],continue);

		// store away the result and display an info message
		namedVersions[versionName].ClientVersion= clientVersion;
		namedVersions[versionName].ServerVersion= serverVersion;
		nlinfo("- Named version '%s': client=%u, server=%u",versionName.c_str(),clientVersion,serverVersion);
	}
}


//-----------------------------------------------------------------------------
// class CServerPatchManager
//-----------------------------------------------------------------------------

class CServerPatchManager:
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public CServerPatchManagerSkel,
	public CDeploymentConfigurationSynchroniser
{
public:
	// ctor
	CServerPatchManager();

	// CModuleBase specialisation implementation
	bool initModule(const TParsedCommandLine &initInfo);
	std::string buildModuleManifest() const;

	void onModuleUp(IModuleProxy *module);
	void onModuleDown(IModuleProxy *module);
//	bool onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg);

	// dissable immediate message dispatching to allow modules on same service to send eachother messages on module up
	bool isImmediateDispatchingSupported() const { return false; }

	// CServerPatchManagerSkel specialisation implementation
	void registerAdministeredModule(IModuleProxy *sender,bool requiresApplierUpdates,bool requiresTerminalUpdates,bool requireDepCfgUpdates,bool isAdministered);
	void requestRefresh(NLNET::IModuleProxy *sender);
	void setInstallVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domain, uint32 version);
	void setLaunchVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domain, uint32 version);
	void declareState(NLNET::IModuleProxy *sender, const NLMISC::CSString &state);
	void declareVersionName(NLNET::IModuleProxy *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion);
	void executeCommandOnModules(NLNET::IModuleProxy *sender, const NLMISC::CSString &target, const NLMISC::CSString &commandline);
	void executedCommandResult(NLNET::IModuleProxy *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result);

	// CDeploymentConfigurationSynchroniser specialisation implementation
	void cbDeploymentConfigurationSynchronised(NLNET::IModuleProxy* sender);

	// remaining public interface

	// update the domain set from the domains in the CDeploymentConfiguration singleton
	void updateDomains();


private:
	// private data
	mutable NLMISC::CSString _Manifest;
	NLMISC::CSString _DatabaseHost;

	TModuleStates	_ModuleStates;
	TDomains		_Domains;
	TNamedVersions	_NamedVersions;

	struct SRegisteredModule
	{
		bool RequireDepCfgUpdates, RequiresTerminalUpdates, RequireApplierUpdates, IsAdministered;
		SRegisteredModule(): RequireDepCfgUpdates(false), RequiresTerminalUpdates(false), RequireApplierUpdates(false), IsAdministered(false)
		{}
	};

	// A set of currently connected ServerPatchTerminal modules
	typedef std::map<IModuleProxy*,SRegisteredModule> TRegisteredModules;
	TRegisteredModules _RegisteredModules;

protected:
	// some macros for setting up the possiblity of having my own NLMISC_COMMAND scope
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CServerPatchManager, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchManager, dump, "Dump the current emiter status", "no args")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchManager, setInstallVersion, "Set the install version for a given domain", "<domain name> <version>")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchManager, setLaunchVersion, "Set the launch versino for a given domain", "<domain name> <version>")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchManager, updateAESdataBase, "Update the database entries for the AES", "<domain name> <database host> <database login> <database password>")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump);
	NLMISC_CLASS_COMMAND_DECL(setInstallVersion);
	NLMISC_CLASS_COMMAND_DECL(setLaunchVersion);
	NLMISC_CLASS_COMMAND_DECL(updateAESdataBase);
};


//-----------------------------------------------------------------------------
// methods CServerPatchManager - basics
//-----------------------------------------------------------------------------

CServerPatchManager::CServerPatchManager()
{
}

bool CServerPatchManager::initModule(const TParsedCommandLine &initInfo)
{
	CServerPatchManagerSkel::init(this);
	CDeploymentConfigurationSynchroniser::init(this);
	bool ret = CModuleBase::initModule(initInfo);
	nlinfo("SPM: Initialising");

	// setup the database host (if there is one)
	const TParsedCommandLine *databaseHostParamater = initInfo.getParam("dbhost");
	if (databaseHostParamater != NULL)
	{
		_DatabaseHost= databaseHostParamater->ParamValue;
	}
	nlinfo("Initialising SPM with database host = '%s'",_DatabaseHost.empty()? "<none>": _DatabaseHost.c_str());

	// load the domain list file
	readDomainListFile(_Domains);

	// load the latest save of the deployment cfg file
	if (NLMISC::CFile::fileExists("spm_depcfg.txt"))
	{
		CDeploymentConfiguration::getInstance().read("spm_depcfg.txt");

		// update the domain set from the domains in the CDeploymentConfiguration singleton
		updateDomains();
	}

	// load the named versions file
	readNamedVersionFile(_NamedVersions);
	
	// setup the module manifest
	_Manifest = ManifestEntryIsAdministrator;

	// add an info filter to prevent logging of database passwords!
	NLMISC::ICommand::execute("addNegativeFilterInfo updateAESdataBase",*NLMISC::DebugLog);

	return ret;
}

std::string CServerPatchManager::buildModuleManifest() const
{
	return _Manifest;
}

void CServerPatchManager::onModuleUp(IModuleProxy *module)
{
}

void CServerPatchManager::onModuleDown(IModuleProxy *module)
{
	// if this is a registsered module then remove it
	_RegisteredModules.erase(module);

	// if there is a state record for this module then remove it and tell all connected SPTs about the module down
	if (_ModuleStates.find(module->getModuleName())!=_ModuleStates.end())
	{
		// remove the state record
		_ModuleStates.erase(module->getModuleName());

		// send the info to all SPTs
		for (TRegisteredModules::iterator it= _RegisteredModules.begin(); it!=_RegisteredModules.end(); ++it)
		{
			if (it->second.RequiresTerminalUpdates)
			{
				CServerPatchTerminalProxy spt(it->first);
				spt.declareModuleDown(this,module->getModuleName());
			}
		}
	}
}

//void CServerPatchManager::onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg)
//{
////	if (CServerPatchManagerSkel::onDispatchMessage(sender, msg))
////		return;
////
////	if (CDeploymentConfigurationSynchroniserSkel::onDispatchMessage(sender, msg))
////		return;
////
////	// unhandled message....
////
////	BOMB("CServerPatchManager::onProcessModuleMessage : received unhandled message '"<<msg.getName()<<"'", return);
//
//}
	

//-----------------------------------------------------------------------------
// CServerPatchManager message callbacks
//-----------------------------------------------------------------------------

void CServerPatchManager::registerAdministeredModule(IModuleProxy *sender,bool requiresApplierUpdates,bool requiresTerminalUpdates,bool requireDepCfgUpdates,bool isAdministered)
{
	nlinfo("SPM_LOG: registering slave module: %s %s",sender->getModuleName().c_str(),sender->getModuleManifest().c_str());

	// add to the registered modules container
	_RegisteredModules[sender].RequireApplierUpdates= requiresApplierUpdates;
	_RegisteredModules[sender].RequiresTerminalUpdates= requiresTerminalUpdates;
	_RegisteredModules[sender].RequireDepCfgUpdates= requireDepCfgUpdates;
	_RegisteredModules[sender].IsAdministered= isAdministered;
	nldebug("CServerPatchManager: Connection established to module: %s %s",sender->getModuleName().c_str(),sender->getModuleManifest().c_str());

	if (requiresApplierUpdates)
	{
		// send the up to date version numbers to the module...
		CAdministeredModuleBaseProxy prox(sender);
		for (TDomains::iterator dit=_Domains.begin(); dit!=_Domains.end(); ++dit)
		{
			if (dit->second.InstallVersion!=~0u)
			{
				prox.installVersion(this,dit->first,dit->second.InstallVersion);
			}
			if (dit->second.LaunchVersion!=~0u)
			{
				prox.launchVersion(this,dit->first,dit->second.LaunchVersion);
			}
		}
	}
	if (requiresTerminalUpdates)
	{
		// this is an SPT so send it everything...
		requestRefresh(sender);
	}
	if (requireDepCfgUpdates)
	{
		// request a deployment configuration sync
		CDeploymentConfigurationSynchroniser::requestSync(sender);
	}
}

void CServerPatchManager::requestRefresh(NLNET::IModuleProxy *sender)
{
	nlinfo("SPM_LOG: requestRefresh: %s",sender->getModuleName().c_str());

	// this is an SPT so send it everything...
	CServerPatchTerminalProxy spt(sender);

	// send the module states
	for (TModuleStates::iterator it=_ModuleStates.begin(); it!=_ModuleStates.end(); ++it )
	{
		spt.declareState(this,it->first,it->second);
	}

	// send the domain info
	for (TDomains::iterator it=_Domains.begin(); it!=_Domains.end(); ++it )
	{
		spt.declareDomainInfo(this,it->first,it->second.InstallVersion,it->second.LaunchVersion);
	}

	// send the named version info
	for (TNamedVersions::iterator it=_NamedVersions.begin(); it!=_NamedVersions.end(); ++it )
	{
		spt.declareVersionName(this,it->first,it->second.ClientVersion,it->second.ServerVersion);
	}
}

void CServerPatchManager::setInstallVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domain, uint32 version)
{
	bool domainExists= _Domains.find(domain)!=_Domains.end();
	CSString comment;

	// if the domain exists...
	if (domainExists)
	{
		// store away the new version number
		_Domains[domain].InstallVersion= version;

		// initialise the comment & associated counter
		comment= NLMISC::toString("Installing version %u on domain %s",version,domain.c_str());
		uint32 count=0;

		// iterate over all registered modules
		for (TRegisteredModules::iterator it= _RegisteredModules.begin(); it!=_RegisteredModules.end(); ++it)
		{
			if (it->second.RequireApplierUpdates)
			{
				// get a proxy to send messages to the module...
				CAdministeredModuleBaseProxy prox(it->first);
				prox.installVersion(this,domain,version);

				// add an entry to the comment and increment the counter...
				comment+= NLMISC::toString("\n- Dispatching to Module: %s",(it->first)->getModuleName().c_str());
				++count;
			}
			if (it->second.RequiresTerminalUpdates)
			{
				// get a proxy to send messages to the module...
				CServerPatchTerminalProxy prox(it->first);
				prox.setInstallVersion(this,domain,version);
			}
		}

		// finish off the comment
		comment+= NLMISC::toString("\n- Messages forwarded to %u registered modules",count);
	}
	else
	{
		comment= NLMISC::toString("Failed to install version %u on domain %s because domain name not recognised",version,domain.c_str());
	}

	// update the domain list file (for future refference)
	writeDomainListFile(_Domains);

	// if this is a command line command then just dump the comment to the output and bail out
	if (sender==NULL)
	{
		CVectorSString lines;
		comment.splitLines(lines);
		for (uint32 i=0;i<lines.size();++i)
		{
			nlinfo("%s",lines[i].c_str());
		}
		nlinfo("SPM_LOG: setInstallVersion() called from command line for domain: %s with version: %d",domain.c_str(),version);
		return;
	}

	// send a reply message back to the requestor
	nlinfo("SPM_LOG: setInstallVersion() called by module '%s' for domain: %s with version: %d",sender->getModuleName().c_str(),domain.c_str(),version);
	DROP_IF(sender->getModuleClassName()!="ServerPatchTerminal","setInstallVersion message recieved from a module that isn't a CServerPatchTerminal",return);
	CServerPatchTerminalProxy spt(sender);
	spt.ackVersionChange(this,domain,domainExists,comment);
}

void CServerPatchManager::setLaunchVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domain, uint32 version)
{
	bool domainExists= _Domains.find(domain)!=_Domains.end();
	CSString comment;

	// if the domain exists...
	if (domainExists)
	{
		// store away the new version number
		_Domains[domain].LaunchVersion= version;

		// initialise the comment & associated counter
		comment= NLMISC::toString("Launching version %u on domain %s",version,domain.c_str());
		uint32 count=0;

		// iterate over all registered modules
		for (TRegisteredModules::iterator it= _RegisteredModules.begin(); it!=_RegisteredModules.end(); ++it)
		{
			if (it->second.RequireApplierUpdates)
			{
				// get a proxy to send messages to the module...
				CAdministeredModuleBaseProxy prox(it->first);
				prox.launchVersion(this,domain,version);

				// add an entry to the comment and increment the counter...
				comment+= NLMISC::toString("\n- Dispatching to Module: %s",(it->first)->getModuleName().c_str());
				++count;
			}
			if (it->second.RequiresTerminalUpdates)
			{
				// get a proxy to send messages to the module...
				CServerPatchTerminalProxy prox(it->first);
				prox.setLaunchVersion(this,domain,version);
			}
		}

		// finish off the comment
		comment+= NLMISC::toString("\n- Messages forwarded to %u modules",count);
	}
	else
	{
		comment= NLMISC::toString("Failed to launch version %u on domain %s because domain name not recognised",version,domain.c_str());
	}

	// update the domain list file (for future refference)
	writeDomainListFile(_Domains);

	// if this is a command line command then just dump tje comment ot the output and bail out
	if (sender==NULL)
	{
		CVectorSString lines;
		comment.splitLines(lines);
		for (uint32 i=0;i<lines.size();++i)
		{
			nlinfo("%s",lines[i].c_str());
		}
		nlinfo("SPM_LOG: setLaunchVersion() called from command line for domain: %s with version: %d",domain.c_str(),version);
		return;
	}

	// send a reply message back to the requestor
	nlinfo("SPM_LOG: setLaunchVersion() called by module '%s' for domain: %s with version: %d",sender->getModuleName().c_str(),domain.c_str(),version);
	DROP_IF(sender->getModuleClassName()!="ServerPatchTerminal","setLaunchVersion message recieved from a module that isn't a CServerPatchTerminal",return);
	CServerPatchTerminalProxy spt(sender);
	spt.ackVersionChange(this,domain,domainExists,comment);
}

void CServerPatchManager::declareState(NLNET::IModuleProxy *sender, const NLMISC::CSString &state)
{
	// get a copy of the module name for the module that we're dealing with
	CSString moduleName= sender->getModuleName();

	// keep a record of the state for this module...
	_ModuleStates[moduleName]= state;

	// send the info to all SPTs
	for (TRegisteredModules::iterator it= _RegisteredModules.begin(); it!=_RegisteredModules.end(); ++it)
	{
		if (it->second.RequiresTerminalUpdates)
		{
			CServerPatchTerminalProxy spt(it->first);
			spt.declareState(this,moduleName,state);
		}
	}
}

void CServerPatchManager::declareVersionName(NLNET::IModuleProxy *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion)
{
	nlinfo("SPM_LOG: declareVersionName() called by module '%s' for name: %s with client: %d and server: %d",
		sender->getModuleName().c_str(),versionName.c_str(),clientVersion,serverVersion);

	// check whether the named version already exists...
	if (_NamedVersions.find(versionName)!=_NamedVersions.end())
	{
		// if the version numbers matched then we have nothing to do so drop out
		if (_NamedVersions[versionName].ClientVersion== clientVersion && _NamedVersions[versionName].ServerVersion== serverVersion)
		{
			return;
		}

		// display a warning to let people know that there is a change
		nlwarning("Replacing version %s old(server %u, client %u) with new(server %u, client %u)",
			versionName.c_str(),
			_NamedVersions[versionName].ServerVersion,
			_NamedVersions[versionName].ClientVersion,
			serverVersion,
			clientVersion);
	}

	// set the values for the named version
	_NamedVersions[versionName].ServerVersion= serverVersion;
	_NamedVersions[versionName].ClientVersion= clientVersion;

	// save the version name away to the versino name file for future use
	writeNamedVersionFile(_NamedVersions);

	// send the info to all SPTs (including the request orriginator)
	for (TRegisteredModules::iterator it= _RegisteredModules.begin(); it!=_RegisteredModules.end(); ++it)
	{
		if (it->second.RequiresTerminalUpdates)
		{
			CServerPatchTerminalProxy spt(it->first);
			spt.declareVersionName(this,versionName,serverVersion,clientVersion);
		}
	}
}

void CServerPatchManager::executeCommandOnModules(NLNET::IModuleProxy *sender, const NLMISC::CSString &target, const NLMISC::CSString &commandline)
{
	// initialiase a result string to hold info on what we've done - for return to the sender
	CSString result;

	// iterate over all registered modules
	for (TRegisteredModules::iterator it= _RegisteredModules.begin(); it!=_RegisteredModules.end(); ++it)
	{
		if (it->second.IsAdministered && testWildCard((it->first)->getModuleName(),target))
		{
			// get a proxy to send messages to the module...
			CAdministeredModuleBaseProxy prox(it->first);
			prox.executeCommand(this,commandline,sender->getModuleName());

			// add an entry to the comment and increment the counter...
			result+= NLMISC::toString("\nExec: %s: %s",(it->first)->getModuleName().c_str(),commandline.c_str());
		}
	}

	// if the result string is empt then we've found nothing to do ...
	if (result.empty())
	{
		result= "No target found matching: '"+target+"' to execute commandline: "+commandline;
	}

	CServerPatchTerminalProxy spt(sender);
	spt.executedCommandAck(this,result);

	// treat the special case where I match the spec of the target for the command...
	if (testWildCard(getModuleFullyQualifiedName(),target))
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
		nlinfo("exec '%s': '%s'", sender->getModuleName().c_str(), commandline.c_str());
		ICommand::execute(getModuleName()+'.'+commandline.strip(),log);

		// send a reply message to the originating service
		CServerPatchTerminalProxy spt(sender);
		spt.executedCommandResult(this,getModuleFullyQualifiedName(),commandline,stringDisplayer.Data);
	}
}


void CServerPatchManager::executedCommandResult(NLNET::IModuleProxy *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result)
{
	// iterate over all connected SPTs, looking for the originator
	for (TRegisteredModules::iterator it= _RegisteredModules.begin(); it!=_RegisteredModules.end(); ++it)
	{
		if (originator==(it->first)->getModuleName())
		{
			// send off a module message to this domain...
			CServerPatchTerminalProxy spt(it->first);
			spt.executedCommandResult(this,sender->getModuleName(),commandline,result);
			return;
		}
	}
	nlwarning("Ignoring command result from '%s' for unknown module: %s",sender->getModuleName().c_str(),originator.c_str());
}


//-----------------------------------------------------------------------------
// CDeploymentConfigurationSynchroniser specialisation implementation
//-----------------------------------------------------------------------------

void CServerPatchManager::cbDeploymentConfigurationSynchronised(NLNET::IModuleProxy* sender)
{
	// save the updated deployment configuration
	CDeploymentConfiguration::getInstance().write("spm_depcfg.txt");

	// update the domain set from the domains in the CDeploymentConfiguration singleton
	updateDomains();

	// displatch the configuration to any connected spt modules
	for (TRegisteredModules::iterator it= _RegisteredModules.begin(); it!=_RegisteredModules.end(); ++it)
	{
		if (it->second.RequireDepCfgUpdates)
		{
			// simulate a request by the patch applier for a sync
			CDeploymentConfigurationSynchroniser::requestSync(it->first);
		}
	}
}


//-----------------------------------------------------------------------------
// remaining public interface
//-----------------------------------------------------------------------------

void CServerPatchManager::updateDomains()
{
	// get the vector of domain names
	TDomainNames domainNames;
	CDeploymentConfiguration::getInstance().getDomainNames(domainNames);

	// iterate over the domains
	for (TDomainNames::iterator dit=domainNames.begin(); dit!= domainNames.end(); ++dit)
	{
		// if the domain doesn't exist then create it
		if (_Domains.find(*dit)==_Domains.end())
		{
			_Domains[*dit];
		}
	}
}


//-----------------------------------------------------------------------------
// CServerPatchManager NLMISC_COMMANDs
//-----------------------------------------------------------------------------

NLMISC_CLASS_COMMAND_IMPL(CServerPatchManager, dump)
{
	NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

	// display a fancy title
	log.displayNL("----------------------------------");
	log.displayNL("Server Patch Manager");
	log.displayNL("----------------------------------");

	// display the registered modules
	for (TRegisteredModules::iterator it= _RegisteredModules.begin(); it!=_RegisteredModules.end(); ++it)
	{
		log.displayNL("- Registered Module: '%s'%s%s%s",
			(it->first)->getModuleName().c_str(),
			it->second.RequiresTerminalUpdates?" Terminal":"",
			it->second.RequireApplierUpdates?" Applier":"",
			it->second.RequireDepCfgUpdates?" TrackDepCfg":""
			);
	}
	log.displayNL("----------------------------------");

	// display the module states
	for (TModuleStates::iterator it=_ModuleStates.begin(); it!=_ModuleStates.end(); ++it )
	{
		log.displayNL("- Module State: %s: %s",it->first.c_str(),it->second.c_str());
	}
	log.displayNL("----------------------------------");

	// display the domain info
	for (TDomains::iterator it=_Domains.begin(); it!=_Domains.end(); ++it )
	{
		log.displayNL("- Domain Versions: %s: install=%u  launch=%u",it->first.c_str(),it->second.InstallVersion,it->second.LaunchVersion);
	}
	log.displayNL("----------------------------------");

	// display the named version info
	for (TNamedVersions::iterator it=_NamedVersions.begin(); it!=_NamedVersions.end(); ++it )
	{
		log.displayNL("- Named Versions: %s: client=%u  server=%u",it->first.c_str(),it->second.ClientVersion,it->second.ServerVersion);
	}
	log.displayNL("----------------------------------");

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchManager, setInstallVersion)
{
	// identify the command line argguments
	if (args.size()!=2) return false;
	CSString domainName= args[0];
	CSString versionName= args[1];

	// try to make sense of the version name or version number argument
	uint32 version=~0u;
	if (_NamedVersions.find(versionName)!=_NamedVersions.end())
	{
		version= _NamedVersions.find(versionName)->second.ServerVersion;
	}
	else
	{
		version= NLMISC::CSString(versionName).atoui();
		if (version==0 && args[1]!="0") return false;
	}

	// delegate to the message handler routine to do the real work
	setInstallVersion(NULL, domainName, version);

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchManager, setLaunchVersion)
{
	// identify the command line argguments
	if (args.size()!=2) return false;
	CSString domainName= args[0];
	CSString versionName= args[1];

	// try to make sense of the version name or version number argument
	uint32 version=~0u;
	if (_NamedVersions.find(versionName)!=_NamedVersions.end())
	{
		version= _NamedVersions.find(versionName)->second.ServerVersion;
	}
	else
	{
		version= NLMISC::CSString(versionName).atoui();
		if (version==0 && args[1]!="0") return false;
	}

	// delegate to the message handler routine to do the real work
	setLaunchVersion(NULL, domainName, version);

	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerPatchManager, updateAESdataBase)
{
	// check for command syntax
	if (args.size()!=4)
		return false;

	// extract our command arguments
	CSString domainName	= args[0];
	CSString dbHost		= args[1];
	CSString dbLogin	= args[2];
	CSString dbPassword	= args[3];

	// if the domain doesn't exist then bomb out
	DROP_IF(_Domains.find(domainName)==_Domains.end(),"Domain not recognised: "+domainName, return true);

	// prepare our sql database connection...
	MSW::CConnection conn;
	conn.connect(dbHost, dbLogin, dbPassword, "nel_tool");

	// get hold of of the domain description object
	SDomainDescription domainDescription;
	CDeploymentConfiguration::getInstance().getDomain(domainName,domainDescription);

	// create the query to delete all app objects for the domain
	NLMISC::CSString theEraseQuery= NLMISC::toString("DELETE FROM service WHERE `shard` = '%s' ;",domainName.c_str());
	nldebug("SQL: %s",theEraseQuery.c_str());
	conn.query(theEraseQuery);

	// iterate over the domain's hosts...
	for (THostNames::iterator hit= domainDescription.Hosts.begin(); hit!= domainDescription.Hosts.end(); ++hit)
	{
		// get hold of the list of apps that run on this host for this domain
		TAppNames appNames;
		CDeploymentConfiguration::getInstance().getAppNames(*hit,domainName,appNames);

		// for each app...
		for (TAppNames::iterator ait= appNames.begin(); ait!= appNames.end(); ++ait)
		{
			// get hold of the app object
			SAppDescription theApp;
			CDeploymentConfiguration::getInstance().getApp(domainName,*ait,theApp);

			// make sure this is a real app and not a dummy
			if (theApp.CmdLine=="none")
				continue;

			// create the query for the app object...
			NLMISC::CSString theQuery= NLMISC::toString("INSERT INTO service (`service_id`,`shard`,`server`,`name`) VALUES ('','%s','%s','%s') ;",domainName.c_str(),hit->splitTo('.').c_str(),theApp.AppName.c_str());
			nldebug("SQL: %s",theQuery.c_str());
			conn.query(theQuery);
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
// CServerPatchManager registration
//-----------------------------------------------------------------------------

NLNET_REGISTER_MODULE_FACTORY(CServerPatchManager,"ServerPatchManager");
