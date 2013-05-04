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
#include "nel/misc/types_nl.h"
#include "nel/misc/variable.h"
#include "nel/net/service.h"
#include <time.h>

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

NLMISC::CVariable<string>	NevraxArchiveFileName("pam","NevraxArchiveFileName","Name of the archive file containing the .bashrc, .emacs and other similar environment files","nevrax.tgz",0,true);
NLMISC::CVariable<string>	InstallArchiveFileName("pam","InstallArchiveFileName","Name of the archive file containing the patchman installation","admin_install.tgz",0,true);
NLMISC::CVariable<string>	InstallArchiveDirectory("pam","InstallArchiveDirectory","Directory where the archive file containing the patchman installation is to be saved","/home/nevrax/",0,true);
NLMISC::CVariable<string>	ShellScriptCmdLine("pam","ShellScriptName","Command line executed by the shellScript command","/bin/sh /home/nevrax/patchman/pam_shell_script.sh",0,true);
NLMISC::CVariable<string>	UpdateEnvironmentLiveCmdLine("pam","UpdateEnvironmentLiveCmdLine","Command line executed to unpack & install the contents of the NevraxArchiveFileName file","/bin/sh /home/nevrax/patchman/update_environment.sh",0,true);


//-----------------------------------------------------------------------------
// class CPatchmanAdminModule
//-----------------------------------------------------------------------------

class CPatchmanAdminModule:
	public CDeploymentConfigurationSynchroniser,
	public CAdministeredModuleBase,
	public CFileReceiver
{
public:
	// ctors & dtors
	CPatchmanAdminModule();

	// IModule specialisation implementation
	bool initModule(const TParsedCommandLine &initInfo);
	void onModuleUp(IModuleProxy *module);
	void onModuleDown(IModuleProxy *module);
	void onModuleUpdate();
	std::string buildModuleManifest() const;

	// dissable immediate message dispatching to allow modules on same service to send eachother messages on module up
	bool isImmediateDispatchingSupported() const { return false; }

	// CDeploymentConfigurationSynchroniser specialisation implementation
	void cbDeploymentConfigurationSynchronised(NLNET::IModuleProxy* sender);

	// CFileReceiver specialisation implementation
	void cbValidateRequestMatches(TFileRequestMatches& requestMatches);
	void cbFileDownloadSuccess(const CSString& fileName,const NLMISC::CMemStream& data);
	void cbFileInfoChange(const NLMISC::CSString& fileName);

private:
	// private data
	mutable CSString _Manifest;

	typedef std::map<CSString,CSString> TDownloadRequests; // map of requests to destination directories
	TDownloadRequests _DownloadRequests;		// the set of files that have been requested for download

	// some macros for setting up the possiblity of having my own NLMISC_COMMAND scope
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CPatchmanAdminModule, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CPatchmanAdminModule, dump, "Dump the current status", "no args")
		NLMISC_COMMAND_HANDLER_ADD(CPatchmanAdminModule, getFileInfo, "get info on a file", "<filespec>")
		NLMISC_COMMAND_HANDLER_ADD(CPatchmanAdminModule, download, "download file(s)", "<filespec> [<destination>]")
		NLMISC_COMMAND_HANDLER_ADD(CPatchmanAdminModule, installUpdate, "check whether the patchman install is up to date and fetch a new install archive if need be", "<file name>")
		NLMISC_COMMAND_HANDLER_ADD(CPatchmanAdminModule, shellScript, "run the pam shell script with given command line", "[<arguments for shell script>]")
		NLMISC_COMMAND_HANDLER_ADD(CPatchmanAdminModule, quit, "quit the application", "<file name>")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump);
	NLMISC_CLASS_COMMAND_DECL(getFileInfo);
	NLMISC_CLASS_COMMAND_DECL(download);
	NLMISC_CLASS_COMMAND_DECL(installUpdate);
	NLMISC_CLASS_COMMAND_DECL(shellScript);
	NLMISC_CLASS_COMMAND_DECL(quit);
};


//-----------------------------------------------------------------------------
// methods CPatchmanAdminModule - Basics
//-----------------------------------------------------------------------------

CPatchmanAdminModule::CPatchmanAdminModule()
{
}

bool CPatchmanAdminModule::initModule(const TParsedCommandLine &initInfo)
{
	// setup a variable to build the init message in...
	CSString logMsg;

	// initialise the module base classes...
	logMsg+= CAdministeredModuleBase::init(initInfo);
	CFileReceiver::init(this,"*/*");
	CDeploymentConfigurationSynchroniser::init(this);

	// now that the base classes have been initialised, we can cumulate the module manifests
	_Manifest= (CFileReceiver::getModuleManifest()+_Manifest);
	_Manifest = _Manifest.strip();

	// we're all done so let the world know
	registerProgress(string("PAM Initialised: ")+logMsg+" "+_Manifest);
	setStateVariable("State","Initialised");
	broadcastStateInfo();

	return true;
}

void CPatchmanAdminModule::onModuleUp(IModuleProxy *module)
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

void CPatchmanAdminModule::onModuleDown(IModuleProxy *module)
{
	// allow the base classes a chance to do their stuff
	CAdministeredModuleBase::onModuleDown(module);
	CFileReceiver::onModuleDown(module);
}

void CPatchmanAdminModule::onModuleUpdate()
{
	// allow the base class a chance to do it's stuff
	CAdministeredModuleBase::onModuleUpdate();
	CFileReceiver::onModuleUpdate();
}

std::string CPatchmanAdminModule::buildModuleManifest() const
{
	return _Manifest;
}


//-----------------------------------------------------------------------------
// methods CDeploymentConfigurationSynchroniser - callbacks
//-----------------------------------------------------------------------------

void CPatchmanAdminModule::cbDeploymentConfigurationSynchronised(NLNET::IModuleProxy* sender)
{
}


//-----------------------------------------------------------------------------
// methods CFileReceiver - callbacks
//-----------------------------------------------------------------------------

void CPatchmanAdminModule::cbValidateRequestMatches(CFileReceiver::TFileRequestMatches& requestMatches)
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

void CPatchmanAdminModule::cbFileDownloadSuccess(const CSString& fileName,const NLMISC::CMemStream& data)
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

	// treat the special case where the downloaded file is an installation update for the patchman...
	if (fileName==InstallArchiveFileName.get())
	{
		bool ok= CFileManager::getInstance().save(NLMISC::CPath::standardizePath(InstallArchiveDirectory.get())+fileName,data);
		setStateVariable("Install",ok? "READY TO RESTART": "UPDATE FAILED TO SAVE FILE");
		return;
	}

	// treat the special case where the downloaded file is an environment update for the patchman...
	if (fileName==NevraxArchiveFileName.get())
	{
		bool ok= CFileManager::getInstance().save(NLMISC::CPath::standardizePath(InstallArchiveDirectory.get())+fileName,data);
		if (!ok)
		{
			setStateVariable("Environment","UPDATE FAILED TO SAVE FILE");
			return;
		}

		// workout which directory to execute in and which command to execute
		CSString executionDirectory= InstallArchiveDirectory.get();
		CSString cmdLine= UpdateEnvironmentLiveCmdLine.get();

		// log the info to the centraliser and flush the log...
		registerProgress(NLMISC::toString("Updating environment - executing: '%s' in: %s",cmdLine.c_str(),executionDirectory.c_str()));

		// store away the current path and change directory to the execution directory (creating it if it doesn't exist)
		CSString oldDirectory= NLMISC::CPath::getCurrentPath();
		NLMISC::CPath::setCurrentPath(executionDirectory.c_str());

		// execute the command
		system(cmdLine.c_str());

		// restore the previous working directory
		NLMISC::CPath::setCurrentPath(oldDirectory.c_str());
		time_t rawtime;
		nl_time ( &rawtime );
		setStateVariable("Environment",NLMISC::toString("Updated %s",asctime (nl_localtime ( &rawtime ))));

		return;
	}

	// make sure that the downloaded file was requested recently (and is expected)
	DROP_IF(_DownloadRequests.find(fileName)==_DownloadRequests.end(),("Ignoring unexpected download of file: "+fileName).c_str(),return);

	// build the destination file name
	CSString fullFileName= _DownloadRequests[fileName]+ NLMISC::CFile::getFilename(fileName);

	// write the file to our install directory
	registerProgress(NLMISC::toString("Save (%u bytes) to: %s",data.size(),fileName.c_str()));
	bool ok= CFileManager::getInstance().save(fullFileName,data);

	if (!ok)
	{
		// broadcast an error message and bail out
		registerError("Failed to save received data for file: "+fullFileName);
		DROP("Failed to save received data for file: "+fullFileName,return);
	}

	// remove the 'download request' record
	_DownloadRequests.erase(fileName);
	setStateVariable("PendingDownloads",NLMISC::toString(_DownloadRequests.size()));
}

void CPatchmanAdminModule::cbFileInfoChange(const NLMISC::CSString& fileName)
{
	if (fileName==NevraxArchiveFileName.get())
	{
		// setup info on the environment file that's reported by our connected repository(s)
		SFileInfo newInfo;
		bool found= getSingleFileInfo(fileName,newInfo);

		// if no file could be found then giveup
		if (!found)
		{
			setStateVariable("Environment","NOT FOUND");
			return;
		}

		// setup info on the file that's currently installed
		SFileInfo currentInfo;
		currentInfo.updateFileInfo(fileName,NLMISC::CPath::standardizePath(InstallArchiveDirectory.get())+fileName);

		// if the files match then we're all done
		if ( (currentInfo.FileSize==newInfo.FileSize) &&
			 (currentInfo.Checksum==newInfo.Checksum) )
		{
			setStateVariable("Environment","up to date");
			return;
		}

		// put in a request for the new install file
		requestFile(fileName);
		setStateVariable("Environment","DOWNLOADING");
		nlinfo("Update fetching new environment archive because current one is out of date");
	}
	else if (fileName==InstallArchiveFileName.get())
	{
		// setup info on the environment file that's reported by our connected repository(s)
		SFileInfo newInfo;
		bool found= getSingleFileInfo(fileName,newInfo);

		// if no file could be found then giveup
		if (!found)
		{
			setStateVariable("Install","NOT FOUND");
			return;
		}

		// setup info on the file that's currently installed
		SFileInfo currentInfo;
		currentInfo.updateFileInfo(fileName,NLMISC::CPath::standardizePath(InstallArchiveDirectory.get())+fileName);

		// if the files match then we're all done
		if ( (currentInfo.FileSize!=newInfo.FileSize) ||
			 (currentInfo.Checksum!=newInfo.Checksum) )
		{
			setStateVariable("Install","out of date");
		}
	}
}


//-----------------------------------------------------------------------------
// methods CPatchmanAdminModule - NLMISC_COMMANDs
//-----------------------------------------------------------------------------

NLMISC_CLASS_COMMAND_IMPL(CPatchmanAdminModule, dump)
{
	NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

	CFileReceiver::dump(log);

	log.displayNL("");
	log.displayNL("-----------------------------------");
	log.displayNL("PAM Info");
	log.displayNL("-----------------------------------");
	log.displayNL("state: %s",getStateString().c_str());

	// display any pending downloads with their target directories
	for (TDownloadRequests::const_iterator it= _DownloadRequests.begin(); it!=_DownloadRequests.end(); ++it)
	{
		log.displayNL("Wating for download: %s",(it->second+it->first).c_str());
	}

	// setup info on the install archive that's currently in place
	SFileInfo currentInstallInfo;
	currentInstallInfo.updateFileInfo(InstallArchiveFileName.get(),NLMISC::CPath::standardizePath(InstallArchiveDirectory.get())+InstallArchiveFileName.get());
	log.displayNL("Local  %s: size=%d checksum=%s",InstallArchiveFileName.get().c_str(),currentInstallInfo.FileSize,currentInstallInfo.Checksum.toString().c_str());

	// get info concerning the latest install archive available for download
	SFileInfo requiredInstallInfo;
	bool found= getSingleFileInfo(InstallArchiveFileName,requiredInstallInfo);
	if (found)
	{
		// see if the two archives match
		if ( (currentInstallInfo.FileSize==requiredInstallInfo.FileSize) &&
			 (currentInstallInfo.Checksum==requiredInstallInfo.Checksum) )
		{
			// the archives match
			log.displayNL("%s is up to date",InstallArchiveFileName.get().c_str());
		}
		else
		{
			// the archives don't match so display info on the archive ready for download
			log.displayNL("Server %s: size=%d checksum=%s",InstallArchiveFileName.get().c_str(),requiredInstallInfo.FileSize,requiredInstallInfo.Checksum.toString().c_str());
		}
	}
	else
	{
		// the local copy of the archive file couldn't be located 
		log.displayNL("File not found %s: Download required!",InstallArchiveFileName.get().c_str());
	}

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CPatchmanAdminModule, getFileInfo)
{
	if (args.size()!=1)
		return false;

	dumpFileInfo(args[0],log);

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CPatchmanAdminModule, download)
{
	CSString destination= InstallArchiveDirectory.get();
	switch (args.size())
	{
	case 2:
		destination= args[1];

	case 1:
		{
			// setup info on the install file that's reported by our connected repository(s)
			TFileInfoVector fileInfo;
			getFileInfo(args[0],fileInfo);

			// if no file could be found then giveup
			if (fileInfo.empty())
			{
				log.displayNL("No files found matching file spec %s",args[0].c_str());
				return true;
			}

			// iterate over matching files, adding them to the download list
			for (uint i=(uint)fileInfo.size();i--;)
			{
				_DownloadRequests[fileInfo[i].FileName]= NLMISC::CPath::standardizePath(destination);
				requestFile(fileInfo[i].FileName);
				log.displayNL("Requesting download: %s",fileInfo[i].FileName.c_str());
			}

			// set the 'downloads' state variable to the number of pending downloads
			setStateVariable("PendingDownloads",NLMISC::toString(_DownloadRequests.size()));
			return true;
		}

	default:
		return false;
	}
}

NLMISC_CLASS_COMMAND_IMPL(CPatchmanAdminModule, installUpdate)
{
	if (args.size()!=0)
		return false;

	// setup info on the install file that's reported by our connected repository(s)
	SFileInfo requiredInstallInfo;
	bool found= getSingleFileInfo(InstallArchiveFileName,requiredInstallInfo);

	// if no file could be found then giveup
	if (!found)
	{
		setStateVariable("Install","NOT FOUND");
		log.displayNL("Update FAILED because install archive not found in repository");
		return true;
	}

	// setup info on the file that's currently installed
	SFileInfo currentInstallInfo;
	currentInstallInfo.updateFileInfo(InstallArchiveFileName.get(),NLMISC::CPath::standardizePath(InstallArchiveDirectory.get())+InstallArchiveFileName.get());

	// if the files match then we're all done
	if ( (currentInstallInfo.FileSize==requiredInstallInfo.FileSize) &&
		 (currentInstallInfo.Checksum==requiredInstallInfo.Checksum) )
	{
		log.displayNL("Update IGNORED because installation is already up to date");
		if (getStateVariable("Install").empty())
		{
			setStateVariable("Install","up to date");
		}

		return true;
	}

	// put in a request for the new install file
	requestFile(InstallArchiveFileName);
	setStateVariable("Install","DOWNLOADING");
	log.displayNL("Update fetching new install archive because current one is not up to date");

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CPatchmanAdminModule, shellScript)
{
	// setup the path for our temporary files (use the current directory on windows systems and the /tmp/ on linux / unix)
	CSString path;
#ifdef NL_OS_UNIX
	path = "/tmp/";
#endif

	// setup names of files to hold stdout and stderr output
	CSString stdoutFile = path+CFile::findNewFile("patchman_pam_stdout.tmp");
	CSString stderrFile = path+CFile::findNewFile("patchman_pam_stderr.tmp");
	
	// extract the command line from the raw arguments string
	CSString cmdLine= rawCommandString;
	cmdLine.strtok(" \t");

	// execute the command
	CSString cmd = ShellScriptCmdLine.get() + " " + cmdLine + " >" + stdoutFile + " 2>" + stderrFile;
	nlinfo("Executing: '%s' in directory '%s'\n", cmd.c_str(), CPath::getCurrentPath().c_str());
	bool ok= system(cmd.c_str())==0;

	// retreive the stdout and stderr results
	CSString stdoutResult;
	stdoutResult.readFromFile(stdoutFile);
	CSString stderrResult;
	stderrResult.readFromFile(stderrFile);

	// display stdout output (add a header if stderr output was generated too)
	if (!stderrResult.empty())	log.displayNL("<<< stdout >>>");
	NLMISC::CVectorSString outputLines;
	stdoutResult.splitLines(outputLines);
	for (uint32 i=0;i<outputLines.size();++i) log.displayNL("%s",outputLines[i].c_str());
	
	// display stderr output with a separating header (if there is any)
	if (!stderrResult.empty())
	{
		log.displayNL("<<< stderr >>>");
		outputLines.clear();
		stderrResult.splitLines(outputLines);
		for (uint32 i=0;i<outputLines.size();++i) log.displayNL("%s",outputLines[i].c_str());
	}

	// log any extra info that might be important
	if (ok && stdoutResult.empty() && stderrResult.empty()) log.displayNL( "<<< Command Executed OK >>>");
	if (!ok) log.displayNL("Command Execution Failed");

	// time to party!
	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CPatchmanAdminModule, quit)
{
	if (args.size()!=0)
		return false;

	ICommand::execute("quit",log,quiet,human);

	return true;
}


//-----------------------------------------------------------------------------
// CPatchmanAdminModule registration
//-----------------------------------------------------------------------------

NLNET_REGISTER_MODULE_FACTORY(CPatchmanAdminModule,"PatchmanAdminModule");
