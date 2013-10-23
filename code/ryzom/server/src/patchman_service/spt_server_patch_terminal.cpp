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
#include "nel/misc/variable.h"
#include "nel/net/service.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"

// game share
#include "game_share/utils.h"
#include "game_share/deployment_configuration.h"

// local
#include "module_admin_itf.h"
#include "deployment_configuration_synchroniser.h"
#include "patchman_constants.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace PATCHMAN;
using namespace DEPCFG;


//-----------------------------------------------------------------------------
// private utilities
//-----------------------------------------------------------------------------

static CSString LastSPTMessage;
static CSString SPTMessage[16];
NLMISC::CVariablePtr<string> __varLastSPTMessage("spt","LastSPTMessage","The last message received by an SPT", &LastSPTMessage);
NLMISC::CVariablePtr<string> __varSPTMessage0("spt","SPT0","watch for displaying recent state updates", &SPTMessage[0x0]);
NLMISC::CVariablePtr<string> __varSPTMessage1("spt","SPT1","watch for displaying recent state updates", &SPTMessage[0x1]);
NLMISC::CVariablePtr<string> __varSPTMessage2("spt","SPT2","watch for displaying recent state updates", &SPTMessage[0x2]);
NLMISC::CVariablePtr<string> __varSPTMessage3("spt","SPT3","watch for displaying recent state updates", &SPTMessage[0x3]);
NLMISC::CVariablePtr<string> __varSPTMessage4("spt","SPT4","watch for displaying recent state updates", &SPTMessage[0x4]);
NLMISC::CVariablePtr<string> __varSPTMessage5("spt","SPT5","watch for displaying recent state updates", &SPTMessage[0x5]);
NLMISC::CVariablePtr<string> __varSPTMessage6("spt","SPT6","watch for displaying recent state updates", &SPTMessage[0x6]);
NLMISC::CVariablePtr<string> __varSPTMessage7("spt","SPT7","watch for displaying recent state updates", &SPTMessage[0x7]);
NLMISC::CVariablePtr<string> __varSPTMessage8("spt","SPT8","watch for displaying recent state updates", &SPTMessage[0x8]);
NLMISC::CVariablePtr<string> __varSPTMessage9("spt","SPT9","watch for displaying recent state updates", &SPTMessage[0x9]);
NLMISC::CVariablePtr<string> __varSPTMessageA("spt","SPTA","watch for displaying recent state updates", &SPTMessage[0xA]);
NLMISC::CVariablePtr<string> __varSPTMessageB("spt","SPTB","watch for displaying recent state updates", &SPTMessage[0xB]);
NLMISC::CVariablePtr<string> __varSPTMessageC("spt","SPTC","watch for displaying recent state updates", &SPTMessage[0xC]);
NLMISC::CVariablePtr<string> __varSPTMessageD("spt","SPTD","watch for displaying recent state updates", &SPTMessage[0xD]);
NLMISC::CVariablePtr<string> __varSPTMessageE("spt","SPTE","watch for displaying recent state updates", &SPTMessage[0xE]);
NLMISC::CVariablePtr<string> __varSPTMessageF("spt","SPTF","watch for displaying recent state updates", &SPTMessage[0xF]);
NLMISC::CVariable<uint32> NumSPTWatches("spt","NumSPTWatches","number of SPT watches used by this service",16,0,true);

NLMISC::CVariable<string> DevConfigDirectory("spt", "DevConfigDirectory", "Directory used to build developer configuration file", "../../", 0, true);
NLMISC::CVariable<string> DevWorkingDirectory("spt", "DevWorkingDirectory", "Directory set as default directory in generated startup batch", "../../", 0, true);

static void addSPTMessage(const CSString& moduleName, const CSString& msgText)
{
	static uint32 count=0;
	LastSPTMessage= NLMISC::toString("%u: ",++count)+moduleName+": "+msgText;
	nldebug("SPTMSG_VERBOSE %d: %s: %s",count,moduleName.c_str(),msgText.c_str());

	// look for a slot to stick the message in
	CSString cleanModuleName= (moduleName.splitTo(':')+'/'+moduleName.splitFrom(':').splitFrom(':'));
	cleanModuleName = cleanModuleName.strip();

	uint32 oldest=0;
	uint32 oldestTime=~0u;
	for (uint32 i=0;i<sizeof(SPTMessage)/sizeof(SPTMessage[0]) && i<NumSPTWatches.get();++i)
	{
		// extract important fields from the message in this slot
		CSString hold= SPTMessage[i];
		uint32 msgTime= hold.splitTo(':',true).strip().atoui();
		CSString msgModuleName= hold.splitTo(':',true).strip();

		// if we have found a previous message from the same module then replace it
		if (msgModuleName==cleanModuleName)
		{
			SPTMessage[i]= NLMISC::toString("%u: ",++count)+cleanModuleName+": "+msgText;
			return;
		}
		// if we've found an older message than the best so far then update it
		if (msgTime<oldestTime)
		{
			oldestTime= msgTime;
			oldest= i;
		}
	}

	// pack down the existing messages to get rid of the one we chose to overwrite
	for (uint32 i=oldest;i>0;--i)
	{
		SPTMessage[i]= SPTMessage[i-1];
	}

	// store away the new message in the first history slot
	SPTMessage[0]= NLMISC::toString("%u: ",++count)+cleanModuleName+": "+msgText;
}


//-----------------------------------------------------------------------------
// class CServerPatchTerminal
//-----------------------------------------------------------------------------

class CServerPatchTerminal:
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public CServerPatchTerminalSkel,
	public CDeploymentConfigurationSynchroniser
{
public:
	// ctor
	CServerPatchTerminal();

	// CModuleBase specialisation implementation
	bool initModule(const TParsedCommandLine &initInfo);
	std::string buildModuleManifest() const;

	void onModuleUp(IModuleProxy *module);
	void onModuleDown(IModuleProxy *module);
	void onModuleUpdate();
// 	void onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg);

	// dissable immediate message dispatching to allow modules on same service to send eachother messages on module up
	bool isImmediateDispatchingSupported() const { return false; }

	// CServerPatchTerminalSkel specialisation
	void declareState(NLNET::IModuleProxy *sender, const NLMISC::CSString &moduleName, const NLMISC::CSString &state);
	void declareModuleDown(NLNET::IModuleProxy *sender, const NLMISC::CSString &moduleName);
	void declareVersionName(NLNET::IModuleProxy *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion);
	void declareDomainInfo(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, uint32 installVersion, uint32 launchVersion);
	void ackVersionChange(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, bool success, const NLMISC::CSString &comment);
	void setInstallVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, uint32 version);
	void setLaunchVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, uint32 version);
	void executedCommandAck(NLNET::IModuleProxy *sender, const NLMISC::CSString &result);
	void executedCommandResult(NLNET::IModuleProxy *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result);

private:
	// private data
	mutable NLMISC::CSString _Manifest;

	TModuleStates	_ModuleStates;
	TDomains		_Domains;
	TNamedVersions	_NamedVersions;

	uint32 _LastUpdateTime;		// time in seconds since 1970
	uint32 _UpdatePeriod;		// period in seconds
	CSString _CommandFileName;	// name of the command file (if there is one)

	// A set of currently connected ServerPatchManager modules
	typedef std::set<IModuleProxy*> TPatchManagers;
	TPatchManagers _PatchManagers;

	// some macros for setting up the possiblity of having my own NLMISC_COMMAND scope
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CServerPatchTerminal, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchTerminal, dump, "Dump the current emiter status", "no args")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchTerminal, install, "Set the install version for a given domain", "<domain name> <version>")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchTerminal, launch, "Set the launch versino for a given domain", "<domain name> <version>")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchTerminal, nameVersion, "Create a named version", "<version name> <client version> <server version>")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchTerminal, listNamedVersions, "List named versions (within an optional range)", "[<first> [<last>]]")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchTerminal, state, "Display the state of subset of connected modules", "[<module id wildcard> [[!]<varname wildcard> [[!]...]]]")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchTerminal, uploadDepCfg, "Upload a given file as the new deployment config for the server park", "<file name>")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchTerminal, on, "Execute a command line on given set of remote modules", "<module name spec>[';'<>[;...]] <cmdline>")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchTerminal, depDevCfg, "Deploy the a set of configuration file for the local developer machine", "")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump);
	NLMISC_CLASS_COMMAND_DECL(install);
	NLMISC_CLASS_COMMAND_DECL(launch);
	NLMISC_CLASS_COMMAND_DECL(nameVersion);
	NLMISC_CLASS_COMMAND_DECL(listNamedVersions);
	NLMISC_CLASS_COMMAND_DECL(state);
	NLMISC_CLASS_COMMAND_DECL(uploadDepCfg);
	NLMISC_CLASS_COMMAND_DECL(on);
	NLMISC_CLASS_COMMAND_DECL(depDevCfg);
};


//-----------------------------------------------------------------------------
// methods CServerPatchTerminal - basics
//-----------------------------------------------------------------------------

CServerPatchTerminal::CServerPatchTerminal()
{
	_LastUpdateTime=0;	// force update at first call
	_UpdatePeriod=10;	// update every 10 seconds
}

bool CServerPatchTerminal::initModule(const TParsedCommandLine &initInfo)
{
	// allow base classes to do their stuff
	CServerPatchTerminalSkel::init(this);
	CDeploymentConfigurationSynchroniser::init(this);
	bool ret = CModuleBase::initModule(initInfo);

	// if there's a 'cmdFile' parameter then deal with it
	const TParsedCommandLine *cmdFileArg = initInfo.getParam("cmdFile");
	if (cmdFileArg!=NULL)
	{
		_CommandFileName= cmdFileArg->ParamValue;
	}

	// if there's an 'updatePeriod' parameter then deal with it
	const TParsedCommandLine *updatePeriodArg = initInfo.getParam("updatePeriod");
	if (updatePeriodArg!=NULL)
	{
		CSString updatePeriodStr= updatePeriodArg->ParamValue;
		uint32 updatePeriod= updatePeriodStr.atoui();
		DROP_IF(updatePeriod==0 && updatePeriodStr!="0","Invalid updatePeriod: "+updatePeriodStr,return false);
		_UpdatePeriod= updatePeriod;
	}

	// display a fancy log message
	nlinfo("SPM: Initialised: cmdFile='%s' updatePeriod=%u seconds",_CommandFileName.c_str(),_UpdatePeriod);

	return ret;
}

std::string CServerPatchTerminal::buildModuleManifest() const
{
	return "";
}

void CServerPatchTerminal::onModuleUp(IModuleProxy *module)
{
	// if the module coming up is an SPM module then we call it 'dad'
	if (CSString(module->getModuleManifest()).contains(ManifestEntryIsAdministrator))
	{
		_PatchManagers.insert(module);
		addSPTMessage(module->getModuleName(),"** connection **");
		nlinfo("ServerPatchManager Connected: %s (%s)",module->getModuleName().c_str(),module->getModuleManifest().c_str());

		// register with the administrator
		CServerPatchManagerProxy manager(module);
		manager.registerAdministeredModule(this,false,true,true,false);
	}
}

void CServerPatchTerminal::onModuleDown(IModuleProxy *module)
{
	// if the module going down is an SPM module then remove it from our lists
	if (_PatchManagers.find(module)!=_PatchManagers.end())
	{
		_PatchManagers.erase(module);
		addSPTMessage(module->getModuleName(),"** disconnection **");
		nlinfo("ServerPatchManager Disconnected: %s (%s)",module->getModuleName().c_str(),module->getModuleManifest().c_str());
	}

	// erase all of our current info (module related)
	_ModuleStates.clear();

	// request an update from connected managers
	for (TPatchManagers::iterator it=_PatchManagers.begin();it!=_PatchManagers.end();++it)
	{
		CServerPatchManagerProxy spm(*it);
		spm.requestRefresh(this);
	}
}

void CServerPatchTerminal::onModuleUpdate()
{
	H_AUTO(CServerPatchTerminal_onModuleUpdate);

	// if it's been a while since we last checked for presence of a command file and we have a non-null
	// command file name then look and see if the command file is present / has changed
	uint32 currentTime= CTime::getSecondsSince1970();
	if (currentTime-_LastUpdateTime > _UpdatePeriod && !_CommandFileName.empty())
	{
		_LastUpdateTime= currentTime;
		if (NLMISC::CFile::fileExists(_CommandFileName))
		{
			// move the command file to a tmp file...
			CSString tmpFileName= _CommandFileName+"__patchman__spt__.tmp";
			bool ok= NLMISC::CFile::moveFile(tmpFileName.c_str(),_CommandFileName.c_str());
			DROP_IF(!ok,"Attempt to move file '"+_CommandFileName+"' to '"+tmpFileName+"' FAILED",return);

			// read the tmp file and delete it
			CSString commandFileContents;
			commandFileContents.readFromFile(tmpFileName);
			NLMISC::CFile::deleteFile(tmpFileName);

			// split the commands into lines
			CVectorSString commands;
			commandFileContents.splitLines(commands);

			// execute the commands...
			for (uint32 i=0;i<commands.size();++i)
			{
				CSString& command= commands[i];
				ICommand::execute(getModuleName()+"."+command.strip(),*InfoLog);
			}
		}
	}
}

//void CServerPatchTerminal::onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg)
//{
//	if (CServerPatchTerminalSkel::onDispatchMessage(sender, msg))
//		return;
//
//	if (CDeploymentConfigurationSynchroniserSkel::onDispatchMessage(sender, msg))
//		return;
//
//	// unhandled message....
//
//	BOMB("CServerPatchTerminal::onProcessModuleMessage : received unhandled message '"<<msg.getName()<<"'", return);
//
//}

	
//-----------------------------------------------------------------------------
// CServerPatchTerminal message callbacks
//-----------------------------------------------------------------------------

void CServerPatchTerminal::declareState(NLNET::IModuleProxy *sender, const NLMISC::CSString &moduleName, const NLMISC::CSString &state)
{
	addSPTMessage(moduleName,state);
	_ModuleStates[moduleName]= state;
}

void CServerPatchTerminal::declareModuleDown(NLNET::IModuleProxy *sender, const NLMISC::CSString &moduleName)
{
	addSPTMessage(moduleName,"** module down **");
	_ModuleStates.erase(moduleName);
}

void CServerPatchTerminal::declareVersionName(NLNET::IModuleProxy *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion)
{
	_NamedVersions[versionName].ServerVersion= serverVersion;
	_NamedVersions[versionName].ClientVersion= clientVersion;
}

void CServerPatchTerminal::declareDomainInfo(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, uint32 installVersion, uint32 launchVersion)
{
	_Domains[domainName].InstallVersion= installVersion;
	_Domains[domainName].LaunchVersion= launchVersion;
	nlinfo("Domain %s: Install: %d, Launch: %d", domainName.c_str(), installVersion, launchVersion);
}

void CServerPatchTerminal::ackVersionChange(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, bool success, const NLMISC::CSString &comment)
{
	nlinfo("Received ack message for version change for domain %s: %s",domainName.c_str(),success?"SUCCESS":"FAILED");
	CVectorSString lines;
	comment.splitLines(lines);
	for (uint32 i=0;i<lines.size();++i)
	{
		nlinfo("- %s",lines[i].c_str());
	}
}

void CServerPatchTerminal::setInstallVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, uint32 version)
{
	_Domains[domainName].InstallVersion= version;
	nlinfo("Domain %s: Install: %d",domainName.c_str(),version);
}

void CServerPatchTerminal::setLaunchVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, uint32 version)
{
	_Domains[domainName].LaunchVersion= version;
	nlinfo("Domain %s: Launch: %d",domainName.c_str(),version);
}

void CServerPatchTerminal::executedCommandAck(NLNET::IModuleProxy *sender, const NLMISC::CSString &result)
{
	nlinfo("Ack Command Execution (from %s):",sender->getModuleName().c_str());
	CVectorSString lines;
	result.splitLines(lines);
	for (uint32 i=0;i<lines.size(); ++i)
	{
		nlinfo("- %s",lines[i].c_str());
	}
}

void CServerPatchTerminal::executedCommandResult(NLNET::IModuleProxy *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result)
{
	nlinfo("Command Execution Result: (from %s via %s)",originator.c_str(),sender->getModuleName().c_str());
	CVectorSString lines;
	result.splitLines(lines);
	for (uint32 i=0;i<lines.size(); ++i)
	{
		nlinfo("- %s",lines[i].c_str());
	}
}


//-----------------------------------------------------------------------------
// CServerPatchTerminal utils routines for NLMISC_COMMANDs
//-----------------------------------------------------------------------------

static bool parseArgsForDomainNameAndVersionNumber(const vector<string>& args, const TNamedVersions& namedVersions, CSString& domainName, uint32& version)
{
	// checkout the args...
	DROP_IF(args.size()!=2, "Wrong number of command line arguments found (expecting 2)", return false);
	domainName= args[0];
	CSString versionName= args[1];

	// work out the version number
	version= ~0u;
	if (namedVersions.find(versionName)!= namedVersions.end())
	{
		// the version name was found in our versino names table so just extract the version number from the table
		version= namedVersions.find(versionName)->second.ServerVersion;
	}
	else
	{
		// the version name wasn't found in the version name table so it has to be a number
		version= versionName.atoui();
		DROP_IF(version==0 && args[1]!="0", "Invalid <version> argument (should be a number)", return false);
	}

	return true;
}


//-----------------------------------------------------------------------------
// CServerPatchTerminal NLMISC_COMMANDs
//-----------------------------------------------------------------------------

NLMISC_CLASS_COMMAND_IMPL(CServerPatchTerminal, dump)
{
	NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

	log.displayNL("-------------------------------------------");
	log.displayNL("SPT - Known Module States");
	log.displayNL("-------------------------------------------");
	for (TModuleStates::iterator it= _ModuleStates.begin(); it!=_ModuleStates.end();++it)
	{
		log.displayNL("%s: %s",it->first.c_str(),it->second.c_str());
	}

	log.displayNL("-------------------------------------------");
	log.displayNL("SPT - Domain Versions");
	log.displayNL("-------------------------------------------");
	for (TDomains::iterator it= _Domains.begin(); it!=_Domains.end();++it)
	{
		log.displayNL("%s: Install Requested: %d  Launch Requested: %d",it->first.c_str(),it->second.InstallVersion,it->second.LaunchVersion);
	}

	log.displayNL("-------------------------------------------");
	log.displayNL("SPT - Connected Managers");
	log.displayNL("-------------------------------------------");
	for (TPatchManagers::iterator it= _PatchManagers.begin(); it!=_PatchManagers.end();++it)
	{
		log.displayNL("%s %s",(*it)->getModuleName().c_str(),(*it)->getModuleManifest().c_str());
	}

	log.displayNL("-------------------------------------------");

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchTerminal, install)
{
	// checkout the args...
	CSString domainName;
	uint32 version=~0u;
	bool argsOK= parseArgsForDomainNameAndVersionNumber(args,_NamedVersions,domainName,version);
	if (!argsOK) return false;

	// if there are no patch managers connected then say so...
	DROP_IF(_PatchManagers.empty(),"This command has no effect because no ServerPatchManager module is connected",return true);

	// dispatch the request to the different connected SPMs
	for (TPatchManagers::iterator it=_PatchManagers.begin();it!=_PatchManagers.end();++it)
	{
		CServerPatchManagerProxy spm(*it);
		spm.setInstallVersion(this,domainName,version);
	}

	// all done so return with success
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchTerminal, launch)
{
	// checkout the args...
	CSString domainName;
	uint32 version=~0u;
	bool argsOK= parseArgsForDomainNameAndVersionNumber(args,_NamedVersions,domainName,version);
	if (!argsOK) return false;

	// if there are no patch managers connected then say so...
	DROP_IF(_PatchManagers.empty(),"This command has no effect because no ServerPatchManager module is connected",return true);

	// dispatch the request to the different connected SPMs
	for (TPatchManagers::iterator it=_PatchManagers.begin();it!=_PatchManagers.end();++it)
	{
		CServerPatchManagerProxy spm(*it);
		spm.setLaunchVersion(this,domainName,version);
	}

	// all done so return with success
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchTerminal, nameVersion)
{
	// checkout the args...
	if (args.size()!=3) return false;
	CSString versionName= args[0];
	uint32 clientVersion= NLMISC::CSString(args[1]).atoui();
	uint32 serverVersion= NLMISC::CSString(args[2]).atoui();
	if (serverVersion==0 || clientVersion==0) return false;

	// if there are no patch managers connected then say so...
	DROP_IF(_PatchManagers.empty(),"This command has no effect because no ServerPatchManager module is connected",return true);

	// dispatch the request to the different connected SPMs
	for (TPatchManagers::iterator it=_PatchManagers.begin();it!=_PatchManagers.end();++it)
	{
		CServerPatchManagerProxy spm(*it);
		spm.declareVersionName(this,versionName,clientVersion,serverVersion);
	}

	// all done so return with success
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchTerminal, listNamedVersions)
{
	uint32 first=0, last=~0u;
	switch(args.size())
	{
	case 2:
		last= NLMISC::CSString(args[1]).atoui();
		DROP_IF(NLMISC::toString(last)!=args[1],"Second argument is not a valid number!",return false);
		// drop through ...

	case 1:
		first= NLMISC::CSString(args[0]).atoui();
		DROP_IF(NLMISC::toString(first)!=args[0],"First argument is not a valid number!",return false);
		// drop through ...

	case 0:
		// we found valid arguments so break out and continue
		break;

	default:
		// invalid arguments so return false
		return false;
	}

	// display the named version info
	log.displayNL("---------------------------------------------------------------");
	log.displayNL("Named versions with server version between %u and %u:",first,last);
	log.displayNL("---------------------------------------------------------------");
	for (TNamedVersions::iterator it=_NamedVersions.begin(); it!=_NamedVersions.end(); ++it )
	{
		uint32 serverVersion= it->second.ServerVersion;
		if (serverVersion>=first && serverVersion<=last)
		{
			log.displayNL("- Named Versions: %s: client=%u  server=%u",it->first.c_str(),it->second.ClientVersion,it->second.ServerVersion);
		}
	}
	log.displayNL("---------------------------------------------------------------");

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchTerminal, state)
{
	// setup the module wildcard (default to '*')
	NLMISC::CSString moduleWildcard="*";
	if (!args.empty())
	{
		moduleWildcard= args[0];
	}

	// setup the vector of requested variable names (an empty vector implies 'all variables')
	NLMISC::CVectorSString requestedVarNames;
	for (uint32 i=1; i<args.size(); ++i)
	{
		requestedVarNames.push_back(args[i]);
	}

	// setup a few containers to hold the states that we want to display
	typedef std::set<NLMISC::CSString> TModuleNames;
	typedef std::map<NLMISC::CSString,uint32> TVarLengths;
	typedef std::map<NLMISC::CSString,NLMISC::CSString> TVarValues;
	typedef std::map<NLMISC::CSString,TVarValues> TModuleVars;
	TModuleNames moduleNames;	// the set of modules that match the module name wildcard
	TVarLengths varLengths;		// map of maximum lengths of values indexed by variable names
	TModuleVars moduleVars;		// map of state values indexed by [moduleName][varName]
	uint32 maxModuleNameLen=0;	// the length of the longest module name found

	// build the contents of the above container set
	for (TModuleStates::iterator it= _ModuleStates.begin(); it!=_ModuleStates.end();++it)
	{
		// check whether the module name matches trhe given wildcard
		NLMISC::CSString moduleName= it->first;
		if (!testWildCard(moduleName,moduleWildcard))
			continue;

		// add the module to our set of module names
		moduleNames.insert(moduleName);
		maxModuleNameLen= std::max(maxModuleNameLen,(uint32)moduleName.size());

		// decompose the state variable into component parts
		NLMISC::CVectorSString states;
		it->second.splitBySeparator(',',states);

		// scan the state variables for matches
		for (uint32 i=0;i<states.size();++i)
		{
			// decompose the state into variable name and value components
			NLMISC::CSString varName= states[i].splitTo('=').strip();
			NLMISC::CSString value= states[i].splitFrom('=').strip().unquoteIfQuoted();

			// if the we have a specifice list of request variable names then check to see whether this one is in the list
			if (!requestedVarNames.empty())
			{
				bool found= false;
				for (uint32 i=0; i<requestedVarNames.size(); ++i)
				{
					// check whether this is a positive or negative wildcard
					if (requestedVarNames[i].left(1)!="!")
					{
						// this is a positive wildcard so found becomes true if wildcard matches
						found|= testWildCard(varName,requestedVarNames[i]);
					}
					else
					{
						// this is a negative wildcard so found becomes false if wildcard matches
						found&= !testWildCard(varName,requestedVarNames[i].leftCrop(1));
					}
				}

				// if we hven't found this var name in the requested var names vector then skip it
				if (!found) continue;
			}

			// insert the value into the map of values...
			moduleVars[moduleName][varName]= value;

			// update the maximum length value for the variable
			varLengths[varName]= std::max((uint32)varLengths[varName],std::max((uint32)varName.size(),(uint32)value.size()));
		}
	}
	DROP_IF(moduleNames.empty(),"No modules found matching wildcard: "+moduleWildcard,return true);

	// display a header row
	log.display("%-*s",maxModuleNameLen,"");
	for (TVarLengths::iterator it= varLengths.begin(); it!=varLengths.end();++it)
	{
		log.display(" %-*s",it->second,it->first.c_str());
	}
	log.displayNL("");

	// run through the containers we've just built, displaying their contents
	for (TModuleVars::iterator it= moduleVars.begin(); it!=moduleVars.end();++it)
	{
		log.display("%-*s",maxModuleNameLen,it->first.c_str());
		for (TVarLengths::iterator it2= varLengths.begin(); it2!=varLengths.end();++it2)
		{
			log.display(" %-*s",it2->second,it->second[it2->first].c_str());
		}
		log.displayNL("");
	}

	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerPatchTerminal, uploadDepCfg)
{
	NLMISC::CSString fileName= "server_park_database.txt";
	switch(args.size())
	{
		case 1:
			fileName= args[0];
		case 0:
			break;							// we found valid arguments so break out and continue
		default:
			return false;					// invalid arguments so return false
	}

	// read the file and ensure that the read succeeded
	bool ok=	CDeploymentConfiguration::getInstance().read(fileName);
	DROP_IF(!ok,"Aborting operation due to errors while reading file: "+fileName,return true);

	// bundle the CDeploymentConfiguration singleton into a data blob
	CMemStream blob;
	blob.serial(CDeploymentConfiguration::getInstance());

	// iterate over connected managers to send them the info
	for (TPatchManagers::iterator it= _PatchManagers.begin(); it!=_PatchManagers.end(); ++it)
	{
		// get a proxy for the manager and send them the message
		CDeploymentConfigurationSynchroniserProxy manager(*it);
		manager.sync(this,NLNET::TBinBuffer(blob.buffer(),blob.size()));
	}

	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerPatchTerminal, on)
{
	if (args.size()<2)
		return false;

	// setup the command line from the raw command string, stripping off the command name
	CSString cmdLine= CSString(rawCommandString).strip();
	cmdLine.strtok(" \t");

	// extract the targets from the command line and split them up (cmdLine then only includes the command tail)
	CSString targetSpec= cmdLine.strtok(" \t");
	CVectorSString targets;
	targetSpec.splitBySeparator(';',targets);

	// iterate over connected managers to send them the command
	for (TPatchManagers::iterator it= _PatchManagers.begin(); it!=_PatchManagers.end(); ++it)
	{
		// get a proxy for the manager and send them the message
		CServerPatchManagerProxy manager(*it);
		for (uint32 i=0;i<targets.size();++i)
		{
			manager.executeCommandOnModules(this,targets[i],cmdLine);
		}
	}

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchTerminal, depDevCfg)
{
	NLMISC::CSString fileName= "server_park_database.txt";
	switch(args.size())
	{
		case 1:
			fileName= args[0];
		case 0:
			break;							// we found valid arguments so break out and continue
		default:
			return false;					// invalid arguments so return false
	}

	// read the file and ensure that the read succeeded
	bool ok=	CDeploymentConfiguration::getInstance().read(fileName);
	DROP_IF(!ok,"Aborting operation due to errors while reading file: "+fileName,return true);

	vector<CSString> appNames;
	CDeploymentConfiguration::getInstance().getAppNames(IService::getInstance()->getHostName(), "dev", appNames);

	for (uint i=0; i<appNames.size(); ++i)
	{
		SAppDescription appDesc;
		CDeploymentConfiguration::getInstance().getApp("dev", appNames[i], appDesc);

		string configDirectory = DevConfigDirectory.get()+"/"+appDesc.ShardName+"/";
		// create a directory for each shard configuration files
		if (!CFile::isExists(configDirectory))
			CFile::createDirectoryTree(configDirectory);

		// ok, write the configuration file in the config directory
		CSString cfgFileName= appDesc.CmdLine.firstWord()+".cfg";

		string fileName = configDirectory+cfgFileName;
		FILE *fp = fopen(fileName.c_str(), "wt");
		nlassert(fp != NULL);
		fwrite(appDesc.CfgFile.data(), appDesc.CfgFile.size(), 1, fp);
		fclose(fp);

		string fullConfigPath = CPath::getFullPath(configDirectory);

		// generate a batch starter
		CSString batch;
		// hack the cmd line
		CVectorSString cmdParams;
		explode(string(appDesc.CmdLine), string(" "), reinterpret_cast<vector<string>&>(cmdParams), true);
		for (uint i=0; i<cmdParams.size(); ++i)
		{
			string &p = cmdParams[i];
			if (i == 0)
				p+="_rd.exe";
			else
			{
				if (p == "-C.")
					p = string("-C")+fullConfigPath;
				else if (p == "-L."
					|| p == "--nobreak"
					|| p == "--writepid")
					p = "";
			}
		}
		CSString cmdLine;
		cmdLine.join(cmdParams, " ");
		batch << "cd \""<<DevWorkingDirectory.get()<<"\"\n";
		batch << "start " << cmdLine;

		fileName = DevConfigDirectory.get()+"/start_"+appDesc.ShardName+"_"+appDesc.StartOrder+"_"+appDesc.AppName+".bat";
		fp = fopen(fileName.c_str(), "wt");
		nlassert(fp != NULL);
		fwrite(batch.data(), batch.size(), 1, fp);
		fclose(fp);
	}

	return true;
}


//-----------------------------------------------------------------------------
// CServerPatchTerminal registration
//-----------------------------------------------------------------------------

NLNET_REGISTER_MODULE_FACTORY(CServerPatchTerminal,"ServerPatchTerminal");
