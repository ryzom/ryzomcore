// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2015  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2016  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
//
// Author: Jan BOON (Kaetemi) <jan.boon@kaetemi.be>

#ifndef NLPIPELINE_PROJECT_CONFIG_H
#define NLPIPELINE_PROJECT_CONFIG_H
#include <nel/misc/types_nl.h>
#include <vector>

namespace NLMISC {
	class CConfigFile;
}

#ifdef NL_OS_WINDOWS
#include <nel/misc/sstring.h>
typedef NLMISC::CSString TPathString;
#else
typedef std::string TPathString;
#endif

namespace NLPIPELINE {

/// Asset project configuration. Used to configure lookup directories for tools and buildsite specific setup. Do not use for pipeline build settings
class CProjectConfig
{
public:
	enum Flags
	{
		DatabaseTextureSearchPaths =  0x0001,
		DatabaseMaterialSearchPaths = 0x0002,
		RuntimeTextureSearchPaths =   0x0100,
		RuntimeShapeSearchPaths =     0x0200,
	};

public:
	~CProjectConfig();

	/// Searches for the configuration for the specified asset path by recursively going through all parent directories looking for 'nel.cfg', matches it to a project cfg if partial is not set, initializes and applies the configuration.
	static bool init(const std::string &asset, Flags flags, bool partial = false);
	/// Undo init
	static void release();

private:
	static void cleanup();
	static void searchDirectories(const char *var);

	static CProjectConfig s_Instance;

	static uint32 s_AssetConfigModification;
	static uint32 s_ProjectConfigModification;

	static TPathString s_AssetConfigPath;
	static TPathString s_ProjectConfigPath;

	static std::string s_ProjectName;
	static CProjectConfig::Flags s_InitFlags;

	static std::vector<TPathString> s_ConfigPaths;
	static std::vector<NLMISC::CConfigFile *> s_ConfigFiles;

};

} /* namespace NLPIPELINE */

#endif /* #ifndef NLPIPELINE_PROJECT_CONFIG_H */

/* end of file */
