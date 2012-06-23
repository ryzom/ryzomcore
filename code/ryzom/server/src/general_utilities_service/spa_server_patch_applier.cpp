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
#include "game_share/file_description_container.h"

// local
#include "gus_module_manager.h"
#include "server_control_modules.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUS;


//-------------------------------------------------------------------------------------------------
// constants
//-------------------------------------------------------------------------------------------------

// how long do we wait from update to update
static NLMISC::TTime UpdatePeriod= 2*1000;


//-----------------------------------------------------------------------------
// class CServerPatchApplier
//-----------------------------------------------------------------------------

class CServerPatchApplier: public GUS::IModule
{
public:
	// IModule specialisation implementation
	bool initialiseModule(const NLMISC::CSString& rawArgs);
	void serviceUpdate(NLMISC::TTime localTime);
	NLMISC::CSString getState() const;
	NLMISC::CSString getName() const;
	NLMISC::CSString getParameters() const;
	void displayModule() const;

public:
	// remaining public interface
	CServerPatchApplier();
	void patchFromLive(uint32 liveVersion, uint32 installRequestVersion);

private:
	void _patchUpFromLive(uint32 liveVersion, uint32 installRequestVersion);
	void _patchDownFromLive(uint32 liveVersion, uint32 installRequestVersion);

private:
	// private data
	CServerDirectories _Directories;
	NLMISC::TTime _LatsUpdateTime;
};


//-----------------------------------------------------------------------------
// utility Routines
//-----------------------------------------------------------------------------

bool applyFirstPatch(const NLMISC::CSString& patchFile,const NLMISC::CSString& destFile)
{
	nlinfo("APPLY FIRST PATCH: %s => %s",patchFile.c_str(),destFile.c_str());
	NLMISC::CSString cmd= "xdelta patch "+patchFile+" /dev/null "+destFile;
	bool ok= system(cmd.c_str())==0;
	return ok;
}

bool applyPatch(const NLMISC::CSString& patchFile,const NLMISC::CSString& srcFile,const NLMISC::CSString& destFile)
{
	nlinfo("APPLY PATCH: %s to file %s => %s",patchFile.c_str(),srcFile.c_str(),destFile.c_str());
	NLMISC::CSString cmd= "xdelta patch "+patchFile+" "+srcFile+" "+destFile;
	bool ok= system(cmd.c_str())==0;
	return ok;
}

bool untar(const NLMISC::CSString& destFile)
{
	bool ok;
	nlinfo("SPA UNPACK TAR BALL: %s",destFile.c_str());

	NLMISC::CSString oldPath= NLMISC::CPath::getCurrentPath();
	NLMISC::CSString newPath= NLMISC::CFile::getPath(destFile);

	ok= NLMISC::CPath::setCurrentPath(newPath.c_str());
	DROP_IF(!ok,"Patching error - failed to change directory to: "+newPath,return false);

	NLMISC::CSString cmd;
	cmd+= "tar xzfv "+NLMISC::CFile::getFilename(destFile);
	nldebug("- system: %s",cmd.c_str());
	ok= system(cmd.c_str())==0;

	ok= NLMISC::CPath::setCurrentPath(oldPath.c_str());
	DROP_IF(!ok,"Patching error - failed to change directory to: "+oldPath,return false);

	return ok;
}

bool untarIfNeeded(const NLMISC::CSString& destFile)
{
	if (destFile.right(4)==".tgz" || destFile.right(7)==".tar.gz")
	{
		return untar(destFile);
	}
	return true;
}

bool finalisePatch(const NLMISC::CSString& installDir,const NLMISC::CSString& fileName,uint32 patchCount)
{
	bool ok=true;

	// seup a few file names
	NLMISC::CSString srcFile=   installDir+ fileName+ ((patchCount&1)==0?".tmp1":".tmp0");
	NLMISC::CSString destFile=  installDir+ fileName;
	NLMISC::CSString oldFile=	installDir+ fileName+ ((patchCount&1)==0?".tmp0":".tmp1");

	// rename the final patched file
	nldebug("SPA RENAME: %s => %s",srcFile.c_str(),destFile.c_str());
	NLMISC::CFile::moveFile(destFile.c_str(),srcFile.c_str());

	// delete the temp file used in patch generation (if there was one)
	if (NLMISC::CFile::fileExists(oldFile))
	{
		nldebug("SPA DELETE: %s",oldFile.c_str());
		NLMISC::CFile::deleteFile(oldFile);
	}

	// if the patched file is a TAR ball then unpack it
	return untarIfNeeded(destFile);
}


//-----------------------------------------------------------------------------
// methods CServerPatchApplier
//-----------------------------------------------------------------------------

CServerPatchApplier::CServerPatchApplier()
{
	_LatsUpdateTime= 0;
}

bool CServerPatchApplier::initialiseModule(const NLMISC::CSString& rawArgs)
{
	NLMISC::CSString root= extractNamedPathParameter("root",rawArgs)+'/';
	DROP_IF(root.empty(),"root() parameter not found in command line",return false);

	NLMISC::CSString domain= extractNamedParameter("domain",rawArgs);
	DROP_IF(domain.empty(),"domain() parameter not found in command line",return false);

	nlinfo("SPA: Initialising with root '%s' and domain '%s'",root.c_str(),domain.c_str());
	_Directories.init(root,domain);

	return true;
}

void CServerPatchApplier::serviceUpdate(NLMISC::TTime localTime)
{
	// only trigger a test every few seconds
	if (localTime-_LatsUpdateTime<UpdatePeriod)
		return;
	_LatsUpdateTime= localTime;

	// if there is a download in progress then wait for it to finish
	if (rrBusyMarkerFilesExist(_Directories.patchDirectoryName()))
		return;

	// read the current and required version numbers from their respective files
	uint32 liveVersion= _Directories.getLiveVersion();
	uint32 installedVersion= _Directories.getInstalledVersion();
	uint32 liveRequestVersion= _Directories.getLaunchRequestVersion();
	uint32 installRequestVersion= _Directories.getInstallRequestVersion();

	// if we need to build a new version then do it
	if (installedVersion!=installRequestVersion && liveVersion!=installRequestVersion)
	{
		patchFromLive(liveVersion,installRequestVersion);
	}

	// if the right version is installled but not yet live then make it live...
	if (installedVersion==liveRequestVersion && liveVersion!=liveRequestVersion)
	{
		// launch a script to make the shard live
		nlinfo("SPA: Making installed patch live (version: %d)",liveRequestVersion);
		system(("cd "+_Directories.getRootDirectory()+" && /bin/sh next/scripts/make_installed_shard_live.sh && cd -").c_str());
	}
}

void CServerPatchApplier::patchFromLive(uint32 liveVersion, uint32 installRequestVersion)
{
	// scan the temp directory to build a file list and delete all of the files we find there
	CFileDescriptionContainer tempFiles;
	tempFiles.addFileSpec(_Directories.installDirectoryName()+"*",true);
	for (uint32 i=0;i<tempFiles.size();++i)
	{
		NLMISC::CFile::deleteFile(tempFiles[i].FileName);
	}

	// make sure the temp directory is now empty
	tempFiles.clear();
	tempFiles.addFileSpec(_Directories.installDirectoryName()+"*",true);
	DROP_IF(!tempFiles.empty(),"Failed to delete all of the contents of the directory: "+_Directories.installDirectoryName(),return);

	// choose a patch direction depending on the whether we're patching forwards or backwards
	if (liveVersion<installRequestVersion)
	{
		_patchUpFromLive(liveVersion,installRequestVersion);
	}
	else if (liveVersion>installRequestVersion)
	{
		_patchDownFromLive(liveVersion,installRequestVersion);
	}
}

void CServerPatchApplier::_patchUpFromLive(uint32 liveVersion, uint32 installRequestVersion)
{
	nlinfo("SPA %s: Patching up from version %d to %d",_Directories.getDomainName().c_str(),liveVersion,installRequestVersion);

	// scan the patch directory to build a file list
	CFileDescriptionContainer patchFiles;
	patchFiles.addFileSpec(_Directories.patchDirectoryName()+"*",true);

	// build map of dest file name to patch vector, filtering out patches later than 'version'
	typedef std::map<NLMISC::CSString,std::vector<uint32> > TFilePatches;
	typedef std::set<NLMISC::CSString> TRequiredPaths;
	TFilePatches filePatches;
	TRequiredPaths requiredPaths;
	for (uint32 i=0;i<patchFiles.size();++i)
	{
		CSString name= patchFiles[i].FileName.leftCrop(_Directories.patchDirectoryName().size()).splitTo(".patch_");
		uint32 patchNum= patchFiles[i].FileName.splitFrom(".patch_").atoi();
		DROP_IF(patchNum==0,"Failed to identify patch number in file name: "+patchFiles[i].FileName,continue);
		if (patchNum<=installRequestVersion)
		{
			filePatches[name].push_back(patchNum);
		}
		requiredPaths.insert(NLMISC::CFile::getPath(name));
	}

	// create any paths that we're going to need
	for (TRequiredPaths::iterator it= requiredPaths.begin(); it!=requiredPaths.end(); ++it)
	{
		NLMISC::CFile::createDirectoryTree(_Directories.installDirectoryName()+*it);
	}

	// run through the file list copying or patching as required
	TFilePatches::iterator it= filePatches.begin();
	TFilePatches::iterator itEnd= filePatches.end();
	for (;it!=itEnd;++it)
	{
		// skip files that don't exist in the requested patch version
		if (it->second.empty())
		{
			continue;
		}

		// sort the patch vector
		std::sort(it->second.begin(),it->second.end());

		// identify the first patch beyond the patch in this file beyond 'liveVersion'
		uint32 patchIdx=0;
		while (patchIdx< it->second.size() && it->second[patchIdx]<=liveVersion)
		{
			++patchIdx;
		}

		// if there is no change between the live file and install file then copy the live file
		if (patchIdx==it->second.size())
		{
			nlinfo("COPY: %s from %s",(_Directories.installDirectoryName()+it->first).c_str(),(_Directories.liveDirectoryName()+it->first).c_str());
			NLMISC::CFile::copyFile(_Directories.installDirectoryName()+it->first,_Directories.liveDirectoryName()+it->first);
			untarIfNeeded(_Directories.installDirectoryName()+it->first);
			continue;
		}

		// setup a little boolean to flag errors
		bool ok= true;

		// try to apply the first patch in the patch vector by using the file in the current version directory
		if (patchIdx!=0)
		{
			ok=	applyPatch( NLMISC::toString("%s%s.patch_%d",_Directories.patchDirectoryName().c_str(),it->first.c_str(),it->second[patchIdx]),
							_Directories.liveDirectoryName()+it->first,
							_Directories.installDirectoryName()+it->first+((patchIdx&1)==0?".tmp0":".tmp1"));
			DROP_IF(!ok,"Failed to apply patch ... rewinding and trying to buid file from 0 for file: "+it->first,patchIdx=0);
		}

		// if the patch index is 0 then start by applying the first patch...
		if (patchIdx ==0)
		{
			ok= applyFirstPatch(NLMISC::toString("%s%s.patch_%d",_Directories.patchDirectoryName().c_str(),it->first.c_str(),it->second[0]),
							_Directories.installDirectoryName()+it->first+".tmp0");
			DROP_IF(!ok,"Error: skipping file because failed to apply fist patch: "+it->first,continue);
		}

		// run through the remaining patches aplying them iteratively...
		while(++patchIdx < it->second.size())
		{
			ok=	applyPatch( NLMISC::toString("%s%s.patch_%d",_Directories.patchDirectoryName().c_str(),it->first.c_str(),it->second[patchIdx]),
							_Directories.installDirectoryName()+it->first+((patchIdx&1)==0?".tmp1":".tmp0"),
							_Directories.installDirectoryName()+it->first+((patchIdx&1)==0?".tmp0":".tmp1"));
			DROP_IF(!ok,NLMISC::toString("Failed to apply patch %d ... for file: ",it->second[patchIdx])+it->first,break);
		}

		// if we bombed out during patch apply then skip this file and continue
		if (!ok)
			continue;

		// we're done so rename the last file generated and delete it's twin 
		finalisePatch(_Directories.installDirectoryName(), it->first, it->second.size());
	}

	// write the new version number to the 'last patch version' file
	_Directories.writeInstallVersion(installRequestVersion);
}

void CServerPatchApplier::_patchDownFromLive(uint32 liveVersion, uint32 installRequestVersion)
{
	nlinfo("SPA %s: Patching down from version %d to %d",_Directories.getDomainName().c_str(),liveVersion,installRequestVersion);

	// scan the patch directory to build a file list
	CFileDescriptionContainer patchFiles;
	patchFiles.addFileSpec(_Directories.patchDirectoryName()+"*");

	// build map of dest file name to patch vector, filtering out patches later than 'version'
	// and a set of files that are going to need back patching
	typedef std::map<NLMISC::CSString,std::vector<uint32> > TFilePatches;
	TFilePatches filePatches;
	std::set<NLMISC::CSString> needBackPatchSet;
	for (uint32 i=0;i<patchFiles.size();++i)
	{
		CSString name= patchFiles[i].FileName.leftCrop(_Directories.patchDirectoryName().size()).splitTo(".patch_");
		uint32 patchNum= patchFiles[i].FileName.splitFrom(".patch_").atoi();
		DROP_IF(patchNum==0,"Failed to identify patch number in file name: "+patchFiles[i].FileName,continue);
		if (patchNum<=installRequestVersion)
		{
			filePatches[name].push_back(patchNum);
		}
		else if (patchNum<=liveVersion)
		{
			needBackPatchSet.insert(name);
		}
	}

	// run through the file list copying or patching as required
	TFilePatches::iterator it= filePatches.begin();
	TFilePatches::iterator itEnd= filePatches.end();
	for (;it!=itEnd;++it)
	{
		// skip files that don't exist in the requested patch version
		if (it->second.empty())
		{
			continue;
		}

		// sort the patch vector
		std::sort(it->second.begin(),it->second.end());

		// if there is no change between the live file and install file then copy the live file
		if (needBackPatchSet.find(it->first)==needBackPatchSet.end())
		{
			nlinfo("COPY: %s => %s",(_Directories.installDirectoryName()+it->first).c_str(),(_Directories.liveDirectoryName()+it->first).c_str());
			NLMISC::CFile::copyFile(_Directories.installDirectoryName()+it->first,_Directories.liveDirectoryName()+it->first);
			continue;
		}

		// setup a little boolean to flag errors
		bool ok= true;

		// start by applying the first patch...
		ok= applyFirstPatch(NLMISC::toString("%s%s.patch_%d",_Directories.patchDirectoryName().c_str(),it->first.c_str(),it->second[0]),
						_Directories.installDirectoryName()+it->first+".tmp0");
		DROP_IF(!ok,"Error: skipping file because failed to apply fist patch: "+it->first,continue);

		// run through the remaining patches aplying them iteratively...
		for(uint32 patchIdx=1; patchIdx< it->second.size(); ++patchIdx)
		{
			ok=	applyPatch( NLMISC::toString("%s%s.patch_%d",_Directories.patchDirectoryName().c_str(),it->first.c_str(),it->second[patchIdx]),
							_Directories.installDirectoryName()+it->first+((patchIdx&1)==0?".tmp1":".tmp0"),
							_Directories.installDirectoryName()+it->first+((patchIdx&1)==0?".tmp0":".tmp1"));
			DROP_IF(!ok,NLMISC::toString("Failed to apply patch %d ... for file: ",it->second[patchIdx])+it->first,break);
		}

		// if we bombed out during patch apply then skip this file and continue
		if (!ok)
			continue;

		// we're done so rename the last file generated and delete it's twin
		finalisePatch(_Directories.installDirectoryName(), it->first, it->second.size());
	}

	// write the new version number to the 'last patch version' file
	_Directories.writeInstallVersion(installRequestVersion);
}

NLMISC::CSString CServerPatchApplier::getState() const
{
	return getName()+" "+getParameters()+
		NLMISC::toString(": live(cur=%d, req=%d) install(cur=%d, req=%d)",
						_Directories.getLiveVersion(),_Directories.getLaunchRequestVersion(),
						_Directories.getInstalledVersion(),_Directories.getInstallRequestVersion())+": "+
						(_Directories.isPatchDirectoryReady()?"Patch directory is ready":"Downloading new patch files");
}

NLMISC::CSString CServerPatchApplier::getName() const
{
	return "SPA";
}

NLMISC::CSString CServerPatchApplier::getParameters() const
{
	return "root(\""+_Directories.getRootDirectory()+"\") domain(\""+_Directories.getDomainName()+"\")";
}

void CServerPatchApplier::displayModule() const
{
	NLMISC::InfoLog->displayNL("%s %s",getName().c_str(),getParameters().c_str());

	NLMISC::InfoLog->displayNL("- Domain             : %s",_Directories.getDomainName().c_str());
	NLMISC::InfoLog->displayNL("- Root Directory     : %s",_Directories.getRootDirectory().c_str());

	NLMISC::InfoLog->displayNL("- Patch Directory    : %s",_Directories.patchDirectoryName().c_str());
	NLMISC::InfoLog->displayNL("- Live Directory     : %s",_Directories.liveDirectoryName().c_str());
	NLMISC::InfoLog->displayNL("- Install Directory  : %s",_Directories.installDirectoryName().c_str());

	NLMISC::InfoLog->displayNL("- Live Version       : %d (from file %s)",_Directories.getLiveVersion(),_Directories.liveVersionFileName().c_str());
	NLMISC::InfoLog->displayNL("- Live Request       : %d (from file %s)",_Directories.getLaunchRequestVersion(),_Directories.launchRequestVersionFileName().c_str());

	NLMISC::InfoLog->displayNL("- Install Version    : %d (from file %s)",_Directories.getInstalledVersion(),_Directories.installVersionFileName().c_str());
	NLMISC::InfoLog->displayNL("- Install Request    : %d (from file %s)",_Directories.getInstallRequestVersion(),_Directories.installRequestVersionFileName().c_str());

	NLMISC::InfoLog->displayNL("- %s - %s - %s - %s",
		_Directories.isPatchDirectoryReady()?"patch directory is ready":"patch directory NOT ready",
		_Directories.isLaunchRequestValid()?"live request is valid":"live request NOT valid",
		_Directories.isLaunchRequestLive()?"requested version is live":"requested version NOT live",
		_Directories.isInstallRequestInstalled()?"requested version is installed":"requested version NOT installed" );

}


//-----------------------------------------------------------------------------
// CServerPatchApplier registration
//-----------------------------------------------------------------------------

REGISTER_GUS_MODULE(CServerPatchApplier,"SPA","root(<root directory>) unitfier(<domain name>)","Shard Patch Applier")
