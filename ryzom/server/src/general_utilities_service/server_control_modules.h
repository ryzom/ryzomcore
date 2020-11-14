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

#ifndef SERVER_CONTROL_MODULES_H
#define SERVER_CONTROL_MODULES_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"


//-----------------------------------------------------------------------------
// utility routines
//-----------------------------------------------------------------------------

NLMISC::CSString rrBusyMarkerFileName(const NLMISC::CSString& targetDirectory,const NLMISC::CSString& reName);
bool rrBusyMarkerFilesExist(const NLMISC::CSString& targetDirectory);


//-----------------------------------------------------------------------------
// class CServerDirectories
//-----------------------------------------------------------------------------

class CServerDirectories
{
public:
	CServerDirectories();
	CServerDirectories(const NLMISC::CSString& rootDirectory,const NLMISC::CSString& domainName);
	void init(const NLMISC::CSString& rootDirectory,const NLMISC::CSString& domainName);

	NLMISC::CSString getDomainName() const;
	NLMISC::CSString getRootDirectory() const;

	NLMISC::CSString patchDirectoryName() const;
	NLMISC::CSString liveDirectoryName() const;
	NLMISC::CSString installDirectoryName() const;

	NLMISC::CSString liveVersionFileName() const;
	NLMISC::CSString installVersionFileName() const;
	NLMISC::CSString launchRequestVersionFileName() const;
	NLMISC::CSString installRequestVersionFileName() const;

	bool isPatchDirectoryReady() const;
	bool isLaunchRequestValid() const;
	bool isLaunchRequestLive() const;
	bool isInstallRequestInstalled() const;

	uint32 getLiveVersion() const;
	uint32 getInstalledVersion() const;
	uint32 getLaunchRequestVersion() const;
	uint32 getInstallRequestVersion() const;

	void writeInstallVersion(uint32 version) const;

private:
	NLMISC::CSString _RootDirectory;
	NLMISC::CSString _DomainName;
	bool _Initialised;
};

#endif
