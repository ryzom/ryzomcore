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

// game share
#include "game_share/utils.h"
#include "game_share/file_description_container.h"
#include "game_share/deployment_configuration.h"

// local
#include "administered_module.h"
#include "file_receiver.h"
#include "deployment_configuration_synchroniser.h"
#include "module_admin_itf.h"
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
// some NLMISC Variable
//-----------------------------------------------------------------------------

NLMISC::CVariable<string>	SpaPreCmdLineText("spa","SpaPreCmdLineText","Text to insert before the command line (eg launcher name)","",0,true);
NLMISC::CVariable<string>	SpaPostCmdLineText("spa","SpaPostCmdLineText","Text to insert after the command line (eg logger name)","",0,true);
NLMISC::CVariable<TTime>	UpdatePeriod("spa","UpdatePeriod","How long do we wait from update to update (ms)",2000,0,true);
NLMISC::CVariable<string>	DeploymentRootDirectory("spa","DeploymentRootDirectory","The directory where default aes and screen cfg files are stored","/home/nevrax/patchman/",0,true);
NLMISC::CVariable<string>	MakeInstalledVersionLiveCmdLine("spa","MakeInstalledVersionLiveCmdLine","Command line executed to make the next installed version live","/bin/sh /home/nevrax/patchman/make_next_live.sh",0,true);
NLMISC::CVariable<string>	SpaLaunchAESCmdLine("spa","SpaLaunchAESCmdLine","Command line executed to launch the AES","/bin/sh /home/nevrax/patchman/loop_aes.sh",0,true);


//-----------------------------------------------------------------------------
// some handy utils
//-----------------------------------------------------------------------------

static NLMISC::CSString defaultAdminExecutorServiceCfgFileName()
{
	return NLMISC::CPath::standardizePath(DeploymentRootDirectory)+"admin_executor_service_default.cfg";
}

static NLMISC::CSString defaultScreenCfgFileName()
{
	return NLMISC::CPath::standardizePath(DeploymentRootDirectory)+"screen.rc.default";
}

static NLMISC::CSString appDirectoryName(const NLMISC::CSString& domainDirectory,const NLMISC::CSString& appName)
{
	return domainDirectory+"service_"+appName+'/';
}

static NLMISC::CSString appExePath(const NLMISC::CSString& domainDirectory,const NLMISC::CSString& appName)
{
	return appDirectoryName(domainDirectory,appName)+appName;
}


//-----------------------------------------------------------------------------
// class CServerDirectories
//-----------------------------------------------------------------------------

class CServerDirectories
{
public:
	CServerDirectories();
	CServerDirectories(const NLMISC::CSString& rootDirectory,bool onlyCreatePatchDirectory);
	void init(const NLMISC::CSString& rootDirectory,bool onlyCreatePatchDirectory);

	NLMISC::CSString getRootDirectory() const;
	NLMISC::CSString getDomainDirectory(const NLMISC::CSString& domainName) const;

	NLMISC::CSString getNextPatchDirectory(const NLMISC::CSString& domainName) const;
	NLMISC::CSString getLivePatchDirectory(const NLMISC::CSString& domainName) const;
	NLMISC::CSString getLiveRootDirectory(const NLMISC::CSString& domainName) const;
	NLMISC::CSString getNextRootDirectory(const NLMISC::CSString& domainName) const;

	NLMISC::CSString getLiveVersionFileName(const NLMISC::CSString& domainName) const;
	NLMISC::CSString getNextVersionFileName(const NLMISC::CSString& domainName) const;

	NLMISC::CSString getAdminExecutorServiceCfgFileName(const NLMISC::CSString& domainName) const;
	NLMISC::CSString getScreenCfgFileName(const NLMISC::CSString& domainName) const;

	uint32 getLiveVersion(const NLMISC::CSString& domainName) const;
	uint32 getNextVersion(const NLMISC::CSString& domainName) const;

	void writeNextVersion(const NLMISC::CSString& domainName,uint32 version) const;

private:
	NLMISC::CSString _RootDirectory;
	bool _Initialised;
};


//-----------------------------------------------------------------------------
// class CServerPatchApplier
//-----------------------------------------------------------------------------

class CServerPatchApplier:
	public CDeploymentConfigurationSynchroniser,
	public CAdministeredModuleBase,
	public CFileReceiver
{
public:
	// ctors & dtors
	CServerPatchApplier();

	// IModule specialisation implementation
	bool initModule(const TParsedCommandLine &initInfo);
	void onModuleUp(IModuleProxy *module);
	void onModuleDown(IModuleProxy *module);
	void onModuleUpdate();
	std::string buildModuleManifest() const;

	// CAdministeredModuleBase specialisation implementation
	void installVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString& domainName, uint32 version);
	void launchVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString& domainName, uint32 version);

	// CDeploymentConfigurationSynchroniser specialisation implementation
	void cbDeploymentConfigurationSynchronised(NLNET::IModuleProxy* sender);

	// CFileReceiver specialisation implementation
	void cbValidateRequestMatches(TFileRequestMatches& requestMatches);
	void cbFileDownloadSuccess(const NLMISC::CSString& fileName,const NLMISC::CMemStream& data);

private:
	// private methods for applying patches
	bool _preparePatching(const NLMISC::CSString& domainName, uint32 installRequestVersion);
	void _requestDownload(const NLMISC::CSString& domainName,const NLMISC::CSString& fileName,const NLMISC::CSString& remotePath);
	bool _patchNextFile(const NLMISC::CSString& domainName,uint32 nextVersion);

	void _updateDomain(const NLMISC::CSString& domainName);

private:
	// private data
	NLMISC::CSString _HostName;
	mutable NLMISC::CSString _Manifest;
	CServerDirectories _Directories;
	NLMISC::TTime _LastUpdateTime;

	typedef std::map<TDomainName,uint32> TDomainVersions;
	TDomainVersions _VersionToInstall;	// the version I'm supposed to install (per domain)
	TDomainVersions _VersionToLaunch;	// the version I'm supposed to launch (per domain)
	TDomainVersions _InstallInProgress;	// the version I'm in the process of installing (per domain)

	typedef std::set<NLMISC::CSString> TRequiredFileSet;				// the set of required files for a given domain
	typedef std::map<NLMISC::CSString,TRequiredFileSet> TRequiredFiles;	// the map of required files for all domains
	TRequiredFiles _RequiredFiles;

	typedef std::map<NLMISC::CSString,NLMISC::CSString> TDownloadRequests; // map of requests to domains
	TDownloadRequests _DownloadRequests;		// the set of files that have been requested for download

	// some macros for setting up the possiblity of having my own NLMISC_COMMAND scope
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CServerPatchApplier, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchApplier, dump, "Dump the current status", "no args")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchApplier, getFileInfo, "get info on a file", "<file name>")
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchApplier, getDebug, "get debug archive(s) for a given domain", "<domain name> [<service file specs>] [<required version>]")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump);
	NLMISC_CLASS_COMMAND_DECL(getFileInfo);
	NLMISC_CLASS_COMMAND_DECL(getDebug);
};


//-----------------------------------------------------------------------------
// utility Routines
//-----------------------------------------------------------------------------

static bool untar(const NLMISC::CSString& tarFile,const NLMISC::CSString& destinationDirectory)
{
	bool ok= true;

	NLMISC::CSString oldPath= NLMISC::CPath::getCurrentPath();

	NLMISC::CFile::createDirectoryTree(destinationDirectory);
	ok= NLMISC::CPath::setCurrentPath(destinationDirectory.c_str());
	DROP_IF(!ok,"Patching error - failed to change directory to: "+destinationDirectory,return false);

	NLMISC::CSString cmd;
	cmd+= "tar xzfv "+tarFile;
	nldebug("- system: %s",cmd.c_str());
	ok= system(cmd.c_str())==0;

	ok= NLMISC::CPath::setCurrentPath(oldPath.c_str());
	DROP_IF(!ok,"Patching error - failed to change directory to: "+oldPath,return false);

	return ok;
}

uint32 readVersionFile(const NLMISC::CSString& fileName)
{
	if (!NLMISC::CFile::fileExists(fileName))
	{
		return 0;
	}

	NLMISC::CSString s;
	s.readFromFile(fileName);
	return s.strip().atoi();
}

void writeVersionFile(const NLMISC::CSString& fileName, uint32 version)
{
	NLMISC::CSString(NLMISC::toString(version)).writeToFile(fileName);
}


//-----------------------------------------------------------------------------
// methods CServerDirectories
//-----------------------------------------------------------------------------

CServerDirectories::CServerDirectories()
{
	_Initialised= false;
}

CServerDirectories::CServerDirectories(const NLMISC::CSString& rootDirectory,bool onlyCreatePatchDirectory)
{
	_Initialised= false;
	init(rootDirectory,onlyCreatePatchDirectory);
}

void CServerDirectories::init(const NLMISC::CSString& rootDirectory,bool onlyCreatePatchDirectory)
{
	// setup our data
	_RootDirectory= NLMISC::CPath::standardizePath(rootDirectory.strip());

	// flag ourselves as initialised
	_Initialised= true;
}


//-----------------------------------------------------------------------------

NLMISC::CSString CServerDirectories::getRootDirectory() const
{
	return _RootDirectory;
}

NLMISC::CSString CServerDirectories::getDomainDirectory(const NLMISC::CSString& domainName) const
{
	return NLMISC::CPath::standardizePath(_RootDirectory+domainName);
}


//-----------------------------------------------------------------------------

NLMISC::CSString CServerDirectories::getLivePatchDirectory(const NLMISC::CSString& domainName) const
{
	return getLiveRootDirectory(domainName)+"archive/";
}

NLMISC::CSString CServerDirectories::getNextPatchDirectory(const NLMISC::CSString& domainName) const
{
	return getNextRootDirectory(domainName)+"archive/";
}

NLMISC::CSString CServerDirectories::getLiveRootDirectory(const NLMISC::CSString& domainName) const
{
	nlassert(_Initialised);

	return getDomainDirectory(domainName)+"live/";
}

NLMISC::CSString CServerDirectories::getNextRootDirectory(const NLMISC::CSString& domainName) const
{
	nlassert(_Initialised);

	return getDomainDirectory(domainName)+"next/";
}


//-----------------------------------------------------------------------------

NLMISC::CSString CServerDirectories::getLiveVersionFileName(const NLMISC::CSString& domainName) const
{
	nlassert(_Initialised);

	return getLiveRootDirectory(domainName)+"version";
}

NLMISC::CSString CServerDirectories::getNextVersionFileName(const NLMISC::CSString& domainName) const
{
	nlassert(_Initialised);

	return getNextRootDirectory(domainName)+"version";
}

NLMISC::CSString CServerDirectories::getAdminExecutorServiceCfgFileName(const NLMISC::CSString& domainName) const
{
	nlassert(_Initialised);

	return getDomainDirectory(domainName)+"admin_executor_service.cfg";
}

NLMISC::CSString CServerDirectories::getScreenCfgFileName(const NLMISC::CSString& domainName) const
{
	nlassert(_Initialised);

	return getDomainDirectory(domainName)+domainName+".screen.rc";
}

//-----------------------------------------------------------------------------

uint32 CServerDirectories::getLiveVersion(const NLMISC::CSString& domainName) const
{
	nlassert(_Initialised);

	return readVersionFile(getLiveVersionFileName(domainName));
}

uint32 CServerDirectories::getNextVersion(const NLMISC::CSString& domainName) const
{
	nlassert(_Initialised);

	return readVersionFile(getNextVersionFileName(domainName));
}


//-----------------------------------------------------------------------------

void CServerDirectories::writeNextVersion(const NLMISC::CSString& domainName,uint32 version) const
{
	writeVersionFile(getNextVersionFileName(domainName),version);
}


//-----------------------------------------------------------------------------
// methods CServerPatchApplier - Basics
//-----------------------------------------------------------------------------

CServerPatchApplier::CServerPatchApplier()
{
	_LastUpdateTime= 0;
}

bool CServerPatchApplier::initModule(const TParsedCommandLine &initInfo)
{
	// setup a variable to build the init message in...
	NLMISC::CSString logMsg;

	// deal with the obligatory 'path' argument
	const TParsedCommandLine *rootArg = initInfo.getParam("path");
	DROP_IF(rootArg==NULL,"path() parameter not found in command line",return false);
	NLMISC::CSString root= NLMISC::CPath::standardizePath(rootArg->ParamValue);
	_Directories.init(root,false);
	_Manifest += " root="+ _Directories.getRootDirectory();

	// if there's a 'host' parameter then deal with it
	const TParsedCommandLine *hostArg = initInfo.getParam("host");
	_HostName= (hostArg!=NULL)? hostArg->ParamValue : IService::getInstance()->getHostName();
	_Manifest += " host="+ _HostName;

	// initialise the module base classes...
	logMsg+= CAdministeredModuleBase::init(initInfo);
	CFileReceiver::init(this,"*/*.tgz");
	CDeploymentConfigurationSynchroniser::init(this);

	// now that the base classes have been initialised, we can cumulate the module manifests
	_Manifest= (CFileReceiver::getModuleManifest()+_Manifest);
	_Manifest = _Manifest.strip();

	// we're all done so let the world know
	registerProgress(string("SPA Initialised: ")+logMsg+" "+_Manifest);
	setStateVariable("State","Initialised");
	broadcastStateInfo();

	return true;
}

void CServerPatchApplier::onModuleUp(IModuleProxy *module)
{
	// allow the base classes a chance to do their stuff
	CAdministeredModuleBase::onModuleUp(module);
	CFileReceiver::onModuleUp(module);

	// treat patch manager update specially to get the latest depcfg update
	if (CSString(module->getModuleManifest()).contains(ManifestEntryIsAdministrator))
	{
		// register with the administrator
		CServerPatchManagerProxy manager(module);
		manager.registerAdministeredModule(this,true,false,true,true);
		registerProgress("registering with manager: "+module->getModuleName());
	}
}

void CServerPatchApplier::onModuleDown(IModuleProxy *module)
{
	// allow the base classes a chance to do their stuff
	CAdministeredModuleBase::onModuleDown(module);
	CFileReceiver::onModuleDown(module);
}

void CServerPatchApplier::onModuleUpdate()
{
	H_AUTO(CServerPatchApplier_onModuleUpdate);

	// allow the base class a chance to do it's stuff
	CAdministeredModuleBase::onModuleUpdate();
	CFileReceiver::onModuleUpdate();

	// only trigger a test every few seconds
	NLMISC::TTime localTime= NLMISC::CTime::getLocalTime();
	if (localTime-_LastUpdateTime<UpdatePeriod)
		return;

	// register the new 'last update time'
	_LastUpdateTime= localTime;
	setStateVariable("State","Idle");


	//---------------------------------------------------------------------------------
	// update the domains (patch / go live)

	// get the vector of domain names for our machine from the host description record
	SHostDescription hostDescription;
	CDeploymentConfiguration::getInstance().getHost(_HostName,hostDescription);
	TDomainNames domainNames= hostDescription.Domains;

	// iterate over the domain for this machine
	for (TDomainNames::iterator dit=domainNames.begin(); dit!= domainNames.end(); ++dit)
	{
		_updateDomain(*dit);
	}
}

void CServerPatchApplier::_updateDomain(const NLMISC::CSString& domainName)
{
	// make sure that I have version numbers setup for this domain (if not then fill in default values)
	if (_VersionToInstall.find(domainName)==_VersionToInstall.end()) _VersionToInstall[domainName]= ~0u;
	if (_VersionToLaunch.find(domainName)==_VersionToLaunch.end()) _VersionToLaunch[domainName]= ~0u;

	// take local copies of the requested versions for this domain
	uint32 versionToInstall= _VersionToInstall[domainName];
	uint32 versionToLaunch= _VersionToLaunch[domainName];

	// read the current and required version numbers from their respective files
	uint32 nextVersion= _Directories.getNextVersion(domainName);
	uint32 liveVersion= _Directories.getLiveVersion(domainName);

	// set a state variable for the domain, giving patch numbers
	CSString liveVersionName= liveVersion==~0u? "n/a": NLMISC::toString("%u",liveVersion);
	CSString nextVersionName= nextVersion==~0u? "n/a": NLMISC::toString("%u",nextVersion);
	CSString versionToLaunchName=	(versionToLaunch==~0u)  || (versionToLaunch==liveVersion) ?	"": NLMISC::toString("(%u)",versionToLaunch);
	CSString versionToInstallName=	(versionToInstall==~0u) || (versionToInstall==nextVersion) || (versionToInstall==liveVersion) ?	"": NLMISC::toString("(%u)",versionToInstall);
	setStateVariable("_"+domainName,"live "+liveVersionName+versionToLaunchName+" inst "+nextVersionName+versionToInstallName);

	// if we need to initiate a new patching run then do so
	if (nextVersion!=versionToInstall && liveVersion!=versionToInstall && _InstallInProgress[domainName]!=versionToInstall && versionToInstall!=~0u)
	{
		setStateVariable("State","Working");
		setStateVariable("_"+domainName,"Initiating patch: "+versionToInstallName);
		broadcastStateInfo();
		bool prepOK= _preparePatching(domainName,versionToInstall);
		if (prepOK)
		{
			_InstallInProgress[domainName]= versionToInstall;
		}
		return;
	}

	// if the installation isn't finished then do the next bit...
	if (nextVersion!=versionToInstall && liveVersion!=versionToInstall && versionToInstall!=~0u)
	{
		// patch the next file for this domain ... this can take some time ...
		bool isPatchFinished= _patchNextFile(domainName,versionToInstall);

		if (!isPatchFinished)
			return;

		// we're done patching so update the version number in the target directory
		_Directories.writeNextVersion(domainName,versionToInstall);
	}

	// if the right version is installled but not yet live then make it live...
	if (nextVersion==versionToLaunch && liveVersion!=versionToLaunch && versionToLaunch!=~0u)
	{
		// workout which directory to execute in and which command to execute
		CSString executionDirectory= _Directories.getDomainDirectory(domainName);
		CSString cmdLine= MakeInstalledVersionLiveCmdLine.get();

		// log the info to the centraliser and flush the log...
		registerProgress(NLMISC::toString("Next(%d) => live: '%s' in: %s",versionToLaunch,cmdLine.c_str(),executionDirectory.c_str()));
		setStateVariable("State","Waiting for shutdown");
		broadcastStateInfo();

		// store away the current path and change directory to the execution directory (creating it if it doesn't exist)
		CSString oldDirectory= NLMISC::CPath::getCurrentPath();
		NLMISC::CFile::createDirectoryTree(executionDirectory);
		NLMISC::CPath::setCurrentPath(executionDirectory.c_str());

		// execute the command
		system(cmdLine.c_str());

		// restore the previous working directory
		NLMISC::CPath::setCurrentPath(oldDirectory.c_str());
	}
}

std::string CServerPatchApplier::buildModuleManifest() const
{
	return _Manifest;
}


//-----------------------------------------------------------------------------
// methods CServerPatchApplier - Message handler callbacks
//-----------------------------------------------------------------------------

void CServerPatchApplier::installVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString& domainName, uint32 version)
{
	_VersionToInstall[domainName]= version;
	registerProgress(NLMISC::toString("Setting %s INSTALL=%d (for module %s)",domainName.c_str(),version,sender->getModuleName().c_str()));
}

void CServerPatchApplier::launchVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString& domainName, uint32 version)
{
	_VersionToLaunch[domainName]= version;
	registerProgress(NLMISC::toString("Setting %s LAUNCH=%d (for module %s)",domainName.c_str(),version,sender->getModuleName().c_str()));
}


//-----------------------------------------------------------------------------
// methods CServerPatchApplier - Message handler callbacks
//-----------------------------------------------------------------------------

void CServerPatchApplier::cbDeploymentConfigurationSynchronised(NLNET::IModuleProxy* sender)
{
	registerProgress("dep cfg sync from "+sender->getModuleName());

	// get the host description that concerns us
	SHostDescription theHost;
	CDeploymentConfiguration::getInstance().getHost(_HostName,theHost);

	// iterate over the domains that we're supposed to support...
	for (TDomainNames::iterator dit= theHost.Domains.begin(); dit!= theHost.Domains.end(); ++dit)
	{
		// clear out the set of required files for this domain
		_RequiredFiles[*dit].clear();

		// setup the base for the <domain>.screen.rc config file
		CSString screenConfig;
		screenConfig.readFromFile(defaultScreenCfgFileName());

		// setup the base for the admin_executor_service config file
		CSString adminExecutorConfig;
		adminExecutorConfig.readFromFile(defaultAdminExecutorServiceCfgFileName());
		adminExecutorConfig+="\n\nShardName=\""+*dit+"\";\n";
		CSString aesRegisteredServices;
		CSString aesAddRegisteredServices;

		// get hold of this list of apps for this domain on this host
		TAppNames appNames;
		CDeploymentConfiguration::getInstance().getAppNames(_HostName,*dit,appNames);

		// register a progress message...
		registerProgress(NLMISC::toString("dep cfg update for domain: %s (%d apps)",dit->c_str(),appNames.size()));

		// iterate over the apps that we're supposed to run for this domain...
		for (TAppNames::iterator ait= appNames.begin(); ait!= appNames.end(); ++ait)
		{
			// get hold of the app description for this app
			SAppDescription theApp;
			CDeploymentConfiguration::getInstance().getApp(*dit,*ait,theApp);

			// if the app is a dummy that serves only for retrieving a given data set then skip it
			if (theApp.CmdLine=="none")
			{
				_RequiredFiles[*dit].insert(theApp.DataPacks.begin(),theApp.DataPacks.end());
				continue;
			}

			// make a directory for the app
			CSString dirName= _Directories.getDomainDirectory(*dit)+theApp.AppName;
			NLMISC::CFile::createDirectoryTree(dirName);

			// build a 'Paths' clause to append to cfg file
			const CSString dataPath= _Directories.getLiveRootDirectory(*dit);
			CSString paths;
			paths.join(theApp.DataPacks,"\",\n\t\""+dataPath);
			if (!paths.empty()) paths= 
				"\nPaths = {\n"
				"\t\""+_Directories.getDomainDirectory(*dit)+"local/\",\n"
				"\t\""+appDirectoryName(_Directories.getLiveRootDirectory(*dit),theApp.CmdLine.firstWord())+"\",\n"
				"\t\""+dataPath+paths+"\"\n};\n";

			// copy cfg file to the new directroy
			CSString fullCmd= theApp.CmdLine.splitToOneOfSeparators(" \t");
			CSString exeName= NLMISC::CFile::getFilenameWithoutExtension(fullCmd);
			CSString cfgFileName= dirName+'/'+theApp.CmdLine.firstWord()+".cfg";
			NLMISC::CSString(theApp.CfgFile+paths).writeToFileIfDifferent(cfgFileName);

			// add an entry to the aes cfg file
			CSString exePath= appExePath(_Directories.getLiveRootDirectory(*dit),theApp.CmdLine.firstWord()).quote();
			CSString runPath= NLMISC::CSString(_Directories.getDomainDirectory(*dit)+theApp.AppName).quote();
			CSString exeArgs= NLMISC::CSString(theApp.CmdLine.tailFromFirstWord().strip()).quote();
			CSString aesEntry=theApp.AppName+" = { "+runPath+", "+exePath+", "+exeArgs+" };\n";
			adminExecutorConfig+= aesEntry;
			aesRegisteredServices+= "\n\t\""+theApp.AppName+"\",";
			aesAddRegisteredServices << "\n\t\"" << "aes.addRegisteredService " << theApp.AppName << " " << theApp.ShardName << "\",";

			// add an entry to the screen.rc cfg file
			CSString screenEntry=	"# "+theApp.AppName+"\nchdir "+runPath+"\nscreen -t "+theApp.AppName+" "+
									SpaPreCmdLineText.get()+" "+exePath.unquote()+" "+exeArgs.unquote()+" "+SpaPostCmdLineText.get()+"\n\n";
			screenConfig+= screenEntry;

			// add entries to the requiredFiles set
			CSString exeArchive= "service_"+exeName;
			_RequiredFiles[*dit].insert(exeArchive);
			_RequiredFiles[*dit].insert(theApp.DataPacks.begin(),theApp.DataPacks.end());
		}

		// write the <domain>.screen.rc file
		screenConfig.writeToFileIfDifferent(_Directories.getScreenCfgFileName(*dit));

		// append the registered service list to the aes config file contents
		adminExecutorConfig+= "\nRegisteredServices=\n{"+aesRegisteredServices+"\n};\n";
		adminExecutorConfig << "\nStartCommands += \n{\n" << aesAddRegisteredServices << "\n};\n";

		// write the admin_executor_service.cfg file
		adminExecutorConfig.writeToFileIfDifferent(_Directories.getAdminExecutorServiceCfgFileName(*dit));
	}
}


//-----------------------------------------------------------------------------
// methods CFileReceiver - callbacks
//-----------------------------------------------------------------------------

void CServerPatchApplier::cbValidateRequestMatches(CFileReceiver::TFileRequestMatches& requestMatches)
{
	// if there's nothing to do then do nothing!
	if (requestMatches.empty())
		return;

	// make sure that the file and checksum info for all of the files is ok
	typedef CFileReceiver::TFileRequestMatches::iterator TIt;
	TIt ref= requestMatches.begin();
	for (TIt it= requestMatches.begin(); (++it)!=requestMatches.end();)
	{
		if (it->second.FileSize!=ref->second.FileSize || it->second.Checksum!=ref->second.Checksum)
		{
			registerError("Differing matches found for file: "+ref->second.FileName);
			requestMatches.clear();
			return;
		}
	}

	// all ok so just return
}

void CServerPatchApplier::cbFileDownloadSuccess(const NLMISC::CSString& fileName,const NLMISC::CMemStream& data)
{
	// if the latest info for the file (eg size) doesn't match with the data that we've received then consider that there's a problem and go round again
	SFileInfo fileInfo;
	CFileReceiver::getSingleFileInfo(fileName,fileInfo);
	if (fileInfo.FileSize!=data.size())
	{
		registerError(NLMISC::toString("Re-requesting file because data size (%d != %d): %s",data.size(),fileInfo.FileSize,fileName.c_str()));
		CFileReceiver::requestFile(fileName);
		return;
	}

	// make sure that the downloaded file was requested recently (and is expected)
	DROP_IF(_DownloadRequests.find(fileName)==_DownloadRequests.end(),("Ignoring unexpected download of file: "+fileName).c_str(),return);

	// treat the special case where the downloaded file is a debug archive...
	if (_DownloadRequests[fileName].right(6)==" debug")
	{
		// determine the path to store the debug archive in (deduced from the domain name...)
		CSString domainName= _DownloadRequests[fileName].rightCrop(6);
		NLMISC::CSString fileNameInLive= _Directories.getLivePatchDirectory(domainName)+ NLMISC::CFile::getFilename(fileName);

		//remove the entry from our pending download request map
		_DownloadRequests.erase(fileName);

		// write the file to our live directory
		registerProgress(NLMISC::toString("Save (%u bytes) to: %s",data.size(),fileName.c_str()));
		bool ok= CFileManager::getInstance().save(fileNameInLive,data);
		if (!ok)
		{
			// broadcast an error message and bail out
			registerError("Failed to save received data for file: "+fileName);
			STOP("Failed to save received data for file: "+fileName);
		}

		// untar the archive...
		registerProgress("Uncompressing debug archive: "+fileNameInLive);
		untar(fileNameInLive,_Directories.getLiveRootDirectory(domainName));
		registerProgress("Uncompressed debug archive: "+fileNameInLive);

		// we've treated the debug case so we're done...
		return;
	}

	// build the destination file name
	NLMISC::CSString fileNameInNext= _Directories.getNextPatchDirectory(_DownloadRequests[fileName])+ NLMISC::CFile::getFilename(fileName);

	// write the file to our install directory
	registerProgress(NLMISC::toString("Save (%u bytes) to: %s",data.size(),fileName.c_str()));
	bool ok= CFileManager::getInstance().save(fileNameInNext,data);

	if (!ok)
	{
		// broadcast an error message, wait a few seconds, repeat the file request and bail out
		registerError("Failed to save received data for file: "+fileName);
		STOP("Failed to save received data for file: "+fileName);
		do
		{
			registerProgress("Trying to save file: "+fileNameInNext);
			nlSleep(3*1000);
			ok= CFileManager::getInstance().save(fileNameInNext,data);
		}
		while (!ok);
	}

	// remove the 'download request' record
	_DownloadRequests.erase(fileName);
}


//-----------------------------------------------------------------------------
// methods CServerPatchApplier - applying patches
//-----------------------------------------------------------------------------

bool CServerPatchApplier::_preparePatching(const NLMISC::CSString& domainName, uint32 installRequestVersion)
{
	uint32 currentVersion=0;

	// have a look to see if the 'current_version' file exists in the install directory
	NLMISC::CSString currentVersionFileName= _Directories.getNextPatchDirectory(domainName)+"current_version";
	if (NLMISC::CFile::fileExists(currentVersionFileName))
	{
		// read the current version from the file in question
		CSString fileData;
		fileData.readFromFile(currentVersionFileName);
		currentVersion= fileData.atoui();
	}

	// if the cureent version matches the insall request then we can  assume that the directory has been prepared previously
	if (currentVersion==installRequestVersion)
	{
		registerProgress(NLMISC::toString("Resuming patch for version %u",installRequestVersion));
		return true;
	}

	// update the state variables and force a flush of state variable changes to listening SPM modules
	setStateVariable("_"+domainName,NLMISC::toString("Patching to %u - scanning temp directories",installRequestVersion));
	broadcastStateInfo();

	// scan the temp directory to build a file list and delete all of the files we find there
	CFileDescriptionContainer tempFiles;
	tempFiles.addFileSpec(_Directories.getNextRootDirectory(domainName)+"*",true);
	for (uint32 i=0;i<tempFiles.size();++i)
	{
		NLMISC::CFile::deleteFile(tempFiles[i].FileName);
	}

	// update the state variables and force a flush of state variable changes to listening SPM modules
	setStateVariable("_"+domainName,NLMISC::toString("Patching to %u - setting up temp directory",installRequestVersion));
	broadcastStateInfo();

	// make sure the temp directory is now empty
	tempFiles.clear();
	tempFiles.addFileSpec(_Directories.getNextRootDirectory(domainName)+"*",true);
	DROP_IF(!tempFiles.empty(),"Failed to delete all of the contents of the directory: "+_Directories.getNextRootDirectory(domainName),return false);

	// make sure the temp directory exists...
	NLMISC::CFile::createDirectoryTree(_Directories.getNextPatchDirectory(domainName));

	// write the version number to the tag file to prevent deletion of directory content on patch resume
	CSString fileData= NLMISC::toString(currentVersion);
	fileData.writeToFile(currentVersionFileName);

	return true;
}

void CServerPatchApplier::_requestDownload(const NLMISC::CSString& domainName,const NLMISC::CSString& fileName,const NLMISC::CSString& remotePath)
{
	// if the file already exists in the set of files awaiting download then return...
	if (_DownloadRequests.find(remotePath+fileName)!=_DownloadRequests.end())	return;

	// see if we can retrieve the file as-is from the live branch...

	// setup some handy paths
	NLMISC::CSString fileNameInLive= _Directories.getLivePatchDirectory(domainName)+fileName;
	NLMISC::CSString fileNameInNext= _Directories.getNextPatchDirectory(domainName)+fileName;

	// ask the download system for info on the file that we want (file size & checksum)
	SFileInfo fileInfo;
	bool infoFound= CFileReceiver::getSingleFileInfo(remotePath+fileName,fileInfo);

	// have a look to see if the file exists in our live patch directory (and is the same size)
	if (infoFound && NLMISC::CFile::fileExists(fileNameInLive) && NLMISC::CFile::getFileSize(fileNameInLive)==fileInfo.FileSize)
	{
		// get checksum for the file in live
		SFileInfo liveFileInfo;
		liveFileInfo.updateFileInfo(fileName,fileNameInLive,SFileInfo::FORCE_RECALCULATE,NULL);

		// see if our files match...
		if (fileInfo.Checksum==liveFileInfo.Checksum)
		{
			// try copying the file and return if we succeeded (we don't need to send a download request)
			bool ok=NLMISC::CFile::copyFile(fileNameInNext,fileNameInLive);
			if (ok)
				return;
		}
	}

	// add the file to the set of download requests
	_DownloadRequests[remotePath+fileName]= domainName;
	CFileReceiver::requestFile(remotePath+fileName);
}

bool CServerPatchApplier::_patchNextFile(const NLMISC::CSString& domainName,uint32 nextVersion)
{
	uint32 count= 0;

	// for each file in the required file set
	for (TRequiredFileSet::iterator rit= _RequiredFiles[domainName].begin(); rit!= _RequiredFiles[domainName].end(); ++rit)
	{
		// count the number of requests that we're passing
		++count;

		// compose the file names
		NLMISC::CSString buildNumber= NLMISC::toString("%06u",nextVersion);
		NLMISC::CSString baseFileName= *rit;
		NLMISC::CSString tagFileName= baseFileName+".tag";
		NLMISC::CSString tgzFileName= baseFileName+".tgz";

		// if the archive file is missing then dispatch a request for it and give up
		if (!NLMISC::CFile::fileExists(_Directories.getNextPatchDirectory(domainName)+tgzFileName))
		{
			setStateVariable("State","Working");
			setStateVariable("_"+domainName,"Getting: "+*rit+NLMISC::toString("(%d/%d)",count,_RequiredFiles[domainName].size()));
			_requestDownload(domainName,tgzFileName,buildNumber+'/');
			return false;
		}

		// skip files that we've already treated
		if (NLMISC::CFile::fileExists(_Directories.getNextPatchDirectory(domainName)+tagFileName))
		{
			continue;
		}

		// unpack the file
		setStateVariable("State","Working");
		setStateVariable("_"+domainName,"Unpacking: "+*rit+NLMISC::toString("(%d/%d)",count,_RequiredFiles[domainName].size()));
		broadcastStateInfo();
		untar(_Directories.getNextPatchDirectory(domainName)+tgzFileName,_Directories.getNextRootDirectory(domainName));
		registerProgress("untared: "+*rit);

		// flag the file as done
		("unpack operation complete tag for archive: "+tgzFileName).writeToFile(_Directories.getNextPatchDirectory(domainName)+tagFileName);

		// we're all done for this round...
		return false;
	}

	// there's nothing left to patch here - we're ready!
	clearStateVariable("_"+domainName);
	return true;
}


//-----------------------------------------------------------------------------
// methods CServerPatchApplier - NLMISC_COMMANDs
//-----------------------------------------------------------------------------

NLMISC_CLASS_COMMAND_IMPL(CServerPatchApplier, dump)
{
	NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

	CFileReceiver::dump(log);

	log.displayNL("");
	log.displayNL("-----------------------------------");
	log.displayNL("SPA Info");
	log.displayNL("-----------------------------------");

	// get the vector of domain names for our machine from the host description record
	SHostDescription hostDescription;
	CDeploymentConfiguration::getInstance().getHost(_HostName,hostDescription);
	TDomainNames domainNames= hostDescription.Domains;

	// merge the domain names into a single string
	NLMISC::CSString domains;
	domains.join(domainNames,", ");

	// dump global info
	log.displayNL("- Domains            : %s",domains.c_str());
	log.displayNL("- Root Directory     : %s",_Directories.getRootDirectory().c_str());

	// dump info on each domain
	for (TDomainNames::iterator dit=domainNames.begin(); dit!= domainNames.end(); ++dit)
	{
		log.displayNL("- Domain ----------- : %s",dit->c_str());
		log.displayNL("   - Live Directory     : %s",_Directories.getLiveRootDirectory(*dit).c_str());
		log.displayNL("   - Install Directory  : %s",_Directories.getNextRootDirectory(*dit).c_str());

		log.displayNL("   - Live Version       : %d (from file %s)",_Directories.getLiveVersion(*dit),_Directories.getLiveVersionFileName(*dit).c_str());
		log.displayNL("   - Live Request       : %d",_VersionToLaunch[*dit]);

		log.displayNL("   - Install Version    : %d (from file %s)",_Directories.getNextVersion(*dit),_Directories.getNextVersionFileName(*dit).c_str());
		log.displayNL("   - Install Request    : %d",_VersionToInstall[*dit]);

		for (TRequiredFileSet::iterator rit= _RequiredFiles[*dit].begin(); rit!= _RequiredFiles[*dit].end(); ++rit)
		{
			log.displayNL("   - Uses               : %s",rit->c_str());
		}
	}
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchApplier, getFileInfo)
{
	if (args.size()!=1)
		return false;

	dumpFileInfo(args[0],log);

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchApplier, getDebug)
{
	// initialise the wildcard to use for the service name
	CSString serviceFileSpec="*";

	// initialise the version number to use for the domain
	uint32 requiredVersion= ~0u; 

	switch(args.size())
	{
		case 3: requiredVersion= CSString(args[2]).atoui();
				DROP_IF(requiredVersion==0 && args[2]!="0","Invalid version number: "+args[2],return false)
		case 2: serviceFileSpec= args[1];
		case 1: break;
		default: return false;
	}

	// get the domain name anmd version number
	CSString domainName= args[0];
	if (requiredVersion==~0u)
	{
		requiredVersion= _Directories.getLiveVersion(domainName);
	}
	NLMISC::CSString remotePath= NLMISC::toString("%06u/",requiredVersion);

	// get the version number for the live version in this domain

	// get the set of executables that we run for this domain that match the given file spec
	TAppNames appNames;
	CDeploymentConfiguration::getInstance().getAppNames(_HostName,domainName,appNames);

	// run through the apps looking for any that match our file specs
	for (TAppNames::iterator ait= appNames.begin(); ait!= appNames.end(); ++ait)
	{
		// get hold of the app description for this app
		SAppDescription theApp;
		CDeploymentConfiguration::getInstance().getApp(domainName,*ait,theApp);

		// if the app is a dummy that serves only for retrieving a given data set then skip it
		if (theApp.CmdLine=="none") continue;

		// determine the executable name for the app
		CSString exeName= theApp.CmdLine.firstWord();

		// if the app exe name doesn't match our file spec then continue...
		if ( !testWildCard(exeName,serviceFileSpec) && 
			 !testWildCard("service_"+exeName,serviceFileSpec) && 
			 !testWildCard("service_"+exeName+"_debug",serviceFileSpec) )
			 continue;

		// determine the execution path for the app
		// dispatch the file download request												  
		CSString fileName= "service_"+exeName+"_debug.tgz";
		_DownloadRequests[remotePath+fileName]= domainName+" debug";
		CFileReceiver::requestFile(remotePath+fileName);
	}

	return true;
}

//-----------------------------------------------------------------------------
// CServerPatchApplier registration
//-----------------------------------------------------------------------------

NLNET_REGISTER_MODULE_FACTORY(CServerPatchApplier,"ServerPatchApplier");
