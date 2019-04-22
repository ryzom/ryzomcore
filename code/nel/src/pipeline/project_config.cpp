// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2015  Winch Gate Property Limited
// Author: Jan Boon <jan.boon@kaetemi.be>
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

#include <nel/misc/types_nl.h>
#include "nel/pipeline/project_config.h"

#ifdef NL_OS_WINDOWS
#	include <Windows.h>
#else
#	include <stdlib.h>
#endif

#include <algorithm>

#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/misc/config_file.h>

using namespace std;
using namespace NLMISC;

namespace NLPIPELINE {

TPathString CProjectConfig::s_AssetConfigPath;
TPathString CProjectConfig::s_ProjectConfigPath;
std::vector<NLMISC::CConfigFile *> CProjectConfig::s_ConfigFiles;
std::vector<TPathString> CProjectConfig::s_ConfigPaths;
CProjectConfig CProjectConfig::s_Instance;
uint32 CProjectConfig::s_AssetConfigModification;
uint32 CProjectConfig::s_ProjectConfigModification;
CProjectConfig::Flags CProjectConfig::s_InitFlags = (CProjectConfig::Flags)0;
std::string CProjectConfig::s_ProjectName;

static std::set<TPathString> s_SearchPaths;

void CProjectConfig::cleanup()
{
	for (std::vector<NLMISC::CConfigFile *>::iterator it(s_ConfigFiles.begin()), end(s_ConfigFiles.end()); it != end; ++it)
		delete *it;
	s_ConfigFiles.clear();
}

CProjectConfig::~CProjectConfig()
{
	cleanup();
}

bool CProjectConfig::init(const std::string &asset, Flags flags, bool partial)
{
	TPathString rootPath = NLMISC::CPath::standardizePath(asset, false);
	TPathString configPath = rootPath + "/nel.cfg";
	while (!CFile::fileExists(configPath))
	{
		std::string::size_type sep = CFile::getLastSeparator(rootPath);
		if (sep == string::npos)
			return false;

		rootPath = rootPath.substr(0, sep);
		if (rootPath.empty())
			return false;

		configPath = rootPath + "/nel.cfg";
	}

	rootPath += "/";
	uint32 configFileModification = CFile::getFileModificationDate(configPath);
	bool assetConfigSame = configPath == s_AssetConfigPath && s_AssetConfigModification == configFileModification && s_InitFlags == flags;
	
	std::vector<TPathString> configRootPaths;
	TPathString projectConfigPath;
	uint32 projectConfigModification = 0;
	std::string projectName;
	if (partial)
	{
		if (assetConfigSame && s_ProjectConfigPath.empty())
			return true; // Do not reload
	}
	else
	{
		if (assetConfigSame && !s_ProjectConfigPath.empty() && CFile::fileExists(s_ProjectConfigPath))
		{
			projectConfigModification = CFile::getFileModificationDate(s_ProjectConfigPath);

			if (s_ProjectConfigModification == projectConfigModification)
				return true; // Do not reload
		}

		// Search for project and load up all root paths
		std::vector<std::string> files;
		CPath::getPathContent(CPath::getApplicationDirectory("NeL", true) + "/projects", false, false, true, files);
		for (std::vector<std::string>::iterator it(files.begin()), end(files.end()); it != end; ++it)
		{
			const std::string& file = *it;
			if (file.length() >= 4 && (file.compare(file.length() - 4, 4, ".cfg") == 0))
			{
				CConfigFile project;
				project.load(file);
				CConfigFile::CVar &directories = project.getVar("Directories");
				bool isProject = false;
				for (uint i = 0; i < directories.size(); ++i)
				{
					if (rootPath == CPath::standardizePath(directories.asString(i), true))
					{
						isProject = true;
						break;
					}
				}
				if (isProject)
				{
					projectConfigModification = CFile::getFileModificationDate(file);
					projectConfigPath = file;

					for (uint i = 0; i < directories.size(); ++i)
					{
						std::string dir = CPath::standardizePath(directories.asString(i), true);
						std::string cfgPath = dir + "nel.cfg";
						if (CFile::fileExists(cfgPath))
							configRootPaths.push_back(dir);
					}

					projectName = project.getVar("ProjectName").asString();

					break;
				}
			}
		}
	}

	if (projectConfigPath.empty())
	{
		projectName = "NeL Project";
		configRootPaths.push_back(rootPath);
		projectConfigModification = 0;
	}

	nldebug("Initializing project config '%s'", projectConfigPath.empty() ? configPath.c_str() : projectConfigPath.c_str());
	release();

	s_InitFlags = flags;
	s_AssetConfigPath = configPath;
	s_AssetConfigModification = configFileModification;
	s_ProjectConfigPath = projectConfigPath;
	s_ProjectConfigModification = projectConfigModification;
	s_ProjectName = projectName;
	s_ConfigPaths = configRootPaths;

	std::map<std::string, CConfigFile *> configFiles;
	for (std::vector<TPathString>::iterator it(configRootPaths.begin()), end(configRootPaths.end()); it != end; ++it)
	{
		const std::string &dir = *it;
		const std::string &cfgPath = *it + "nel.cfg";
		CConfigFile *cfgFile = new CConfigFile();
		cfgFile->load(cfgPath);
		std::string identifier = cfgFile->getVar("Identifier").asString();
		if (configFiles.find(identifier) != configFiles.end()) // Identifier already exists
		{
			if (dir == rootPath)
			{
				// Replace config that was already added, asset root gets priority
				std::vector<NLMISC::CConfigFile *>::iterator old = std::find(s_ConfigFiles.begin(), s_ConfigFiles.end(), configFiles[identifier]);
				uint idx = old - s_ConfigFiles.begin();
				s_ConfigFiles.erase(old);
				s_ConfigPaths.erase(s_ConfigPaths.begin() + idx);
			}
			else
			{
				// Skip, first listed config gets priority
				s_ConfigPaths.erase(s_ConfigPaths.begin() + s_ConfigFiles.size());
				continue;
			}
		}
#ifdef NL_OS_WINDOWS
		SetEnvironmentVariableA(identifier.c_str(), dir.c_str());
#else
		setenv(identifier.c_str(), dir.c_str(), 1);
#endif
		configFiles[identifier] = cfgFile;
		s_ConfigFiles.push_back(cfgFile);
	}

	nlassert(s_ConfigFiles.size() == s_ConfigPaths.size());

	if (flags & DatabaseTextureSearchPaths) 
	{
		searchDirectories("DatabaseTextureSearchPaths");
	}

	return true;
}

void CProjectConfig::searchDirectories(const char *var)
{
	for (uint i = 0; i < s_ConfigFiles.size(); ++i)
	{
		CConfigFile *cfg = s_ConfigFiles[i];
		const TPathString &dir = s_ConfigPaths[i];
		CConfigFile::CVar *paths = cfg->getVarPtr(var);
		if (paths)
		{
			for (uint i = 0; i < paths->size(); i++)
			{
				TPathString path = paths->asString(i);
				if (!CPath::isAbsolutePath(path)) path = dir + path;
				path = CPath::standardizePath(path);
				if (s_SearchPaths.find(path) == s_SearchPaths.end())
				{
					CPath::addSearchPath(path);
					s_SearchPaths.insert(path);
				}
			}
		}
	}
}

void CProjectConfig::release()
{
	s_SearchPaths.clear();
	CPath::clearMap();
	cleanup();
}

} /* namespace NLPIPELINE */

/* end of file */
