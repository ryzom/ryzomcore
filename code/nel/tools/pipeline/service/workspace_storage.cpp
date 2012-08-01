/**
 * \file workspace_storage.cpp
 * \brief CWorkspaceStorage
 * \date 2012-07-30 14:34GMT
 * \author Jan Boon (Kaetemi)
 * CWorkspaceStorage
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "workspace_storage.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/net/service.h>

// Project includes
#include "pipeline_service.h"

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

namespace {

/// Input must be normalized path
bool isInRootDirectoryFast(std::string &rootDirectoryName, std::string &rootDirectoryPath, const std::string &path)
{
	//return path.find(g_DatabaseDirectory) == 0;
	NLMISC::CConfigFile::CVar &rootDirectories = NLNET::IService::getInstance()->ConfigFile.getVar("RootDirectories");
	for (uint i = 0; i < rootDirectories.size(); ++i)
	{
		rootDirectoryName = rootDirectories.asString(i);
		NLMISC::CConfigFile::CVar &dir = NLNET::IService::getInstance()->ConfigFile.getVar(rootDirectoryName);
		rootDirectoryPath = standardizePath(dir.asString(), true);
		if (path.find(rootDirectoryPath) == 0) return true;
	}
	return false;
}

bool isInSheetsDirectoryFast(std::string &sheetDirectoryName, std::string &sheetDirectoryPath, const std::string &path)
{
	{
		sheetDirectoryName = "WorkspaceDfnDirectory";
		NLMISC::CConfigFile::CVar &dir = NLNET::IService::getInstance()->ConfigFile.getVar(sheetDirectoryName);
		sheetDirectoryPath = standardizePath(dir.asString(), true);
		if (path.find(sheetDirectoryPath) == 0) return true;
	}
	{
		sheetDirectoryName = "WorkspaceSheetDirectory";
		NLMISC::CConfigFile::CVar &dir = NLNET::IService::getInstance()->ConfigFile.getVar(sheetDirectoryName);
		sheetDirectoryPath = standardizePath(dir.asString(), true);
		if (path.find(sheetDirectoryPath) == 0) return true;
	}
	return false;
}

/// Input must be normalized path
bool isInWorkspaceDirectoryFast(const std::string &path)
{
	return path.find(g_WorkDir) == 0;
}

/// Input must be normalized path in database directory
std::string dropRootDirectoryFast(const std::string &path, const std::string &rootDirectoryPath)
{
	return path.substr(rootDirectoryPath.length());
}

/// Input must be normalized path in sheets directory
std::string dropSheetDirectoryFast(const std::string &path, const std::string &sheetDirectoryPath)
{
	return path.substr(sheetDirectoryPath.length());
}

/// Input must be normalized path in pipeline directory
std::string dropWorkspaceDirectoryFast(const std::string &path)
{
	return path.substr(g_WorkDir.length());
}

} /* anonymous namespace */

std::string CWorkspaceStorage::getMetaFilePath(const std::string &path, const std::string &dotSuffix)
{
	std::string stdPath = standardizePath(path, false);
	if (isInWorkspaceDirectoryFast(stdPath))
	{
		// TODO_TEST
		std::string relPath = dropWorkspaceDirectoryFast(stdPath);
		std::string::size_type slashPos = relPath.find_first_of('/');
		std::string proProName = relPath.substr(0, slashPos);
		std::string subPath = relPath.substr(slashPos);
		return g_WorkDir + proProName + PIPELINE_DATABASE_META_SUFFIX + subPath + dotSuffix;
	}
	else
	{
		std::string rootDirectoryName;
		std::string rootDirectoryPath;
		if (isInSheetsDirectoryFast(rootDirectoryName, rootDirectoryPath, stdPath))
		{
			std::string relPath = dropSheetDirectoryFast(stdPath, rootDirectoryPath);
			return g_WorkDir + PIPELINE_DIRECTORY_PREFIX_SHEET + NLMISC::toLower(rootDirectoryName) + PIPELINE_DATABASE_META_SUFFIX + "/" + relPath + dotSuffix;
		}
		else
		{
			if (isInRootDirectoryFast(rootDirectoryName, rootDirectoryPath, stdPath))
			{
				std::string relPath = dropRootDirectoryFast(stdPath, rootDirectoryPath);
				return g_WorkDir + PIPELINE_DIRECTORY_PREFIX_ROOT + NLMISC::toLower(rootDirectoryName) + PIPELINE_DATABASE_META_SUFFIX + "/" + relPath + dotSuffix;
			}
			else
			{
				nlerror("Path is not in database or pipeline (%s)", path.c_str());
				return path + dotSuffix;
			}
		}
	}
}

std::string CWorkspaceStorage::getMetaDirectoryPath(const std::string &path)
{
	// Not very sane, but works.
	return standardizePath(CWorkspaceStorage::getMetaFilePath(path, ""), true);
}

std::string CWorkspaceStorage::getProjectDirectory(const std::string &projectName)
{
	std::string lwProjectName = NLMISC::toLower(projectName);
	return g_WorkDir + PIPELINE_DIRECTORY_PREFIX_PROJECT + lwProjectName + "/";
}

} /* namespace PIPELINE */

/* end of file */
