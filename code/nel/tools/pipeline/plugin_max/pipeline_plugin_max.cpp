/**
 * \file pipeline_plugin_max.cpp
 * \brief CPipelinePluginMax
 * \date 2012-02-25 10:39GMT
 * \author Jan Boon (Kaetemi)
 * CPipelinePluginMax
 */

/*
 * Copyright (C) 2012  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RYZOM CORE PIPELINE; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "pipeline_plugin_max.h"

// STL includes
#ifdef NL_OS_WINDOWS
#	include <windows.h>
#endif

// NeL includes
#include "nel/misc/dynloadlib.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/algo.h"
#include "nel/misc/file.h"

// Project includes
#include "../plugin_library/pipeline_interface.h"
#include "process_max_shape.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

// ******************************************************************

class CPipelinePluginMaxNelLibrary : public NLMISC::INelLibrary
{
	void onLibraryLoaded(bool /* firstTime */)
	{
		nldebug("Library loaded: CPipelinePluginMax");
		PIPELINE_REGISTER_CLASS(CProcessMaxShape);
	}
	void onLibraryUnloaded(bool /* lastTime */)
	{
		nldebug("Library unloaded: CPipelinePluginMax");
	}
};
NLMISC_DECL_PURE_LIB(CPipelinePluginMaxNelLibrary)

#ifdef NL_OS_WINDOWS
HINSTANCE CPipelinePluginMaxDllHandle = NULL;
BOOL WINAPI DllMain(HANDLE hModule, DWORD /* ul_reason_for_call */, LPVOID /* lpReserved */)
{
	CPipelinePluginMaxDllHandle = (HINSTANCE)hModule;
	return TRUE;
}
#endif

// ******************************************************************
/*
CPipelinePluginMax::CPipelinePluginMax()
{

}

CPipelinePluginMax::~CPipelinePluginMax()
{

}*/

namespace {

std::set<std::string> MissingFiles;
std::map<std::string, std::string> KnownFileCache;

// COPY FROM PIPELINE_SERVICE.CPP WITH BACKSLASHES INSTEAD OF FORWARD SLASHES
std::string standardizePath(const std::string &path, bool addFinalSlash)
{
	// check empty path
	if (path.empty())
		return "";

	std::string newPath;
	newPath.resize(path.size() + 1);

	std::string::size_type j = 0;
	for (std::string::size_type i = 0; i < path.size(); ++i)
	{
		if (path[i] == '\\' || path[i] == '/')
		{
			if (j <= 1 && path[i] == '\\')
			{
				// for windows network
				newPath[j] = '\\';
				++j;
			}
			else if (j == 0 || newPath[j - 1] != '\\')
			{
				newPath[j] = '\\';
				++j;
			}
		}
		else
		{
			newPath[j] = path[i];
			++j;
		}
	}
	newPath[j] = 0;
	newPath.resize(j);

	// add terminal slash
	if (addFinalSlash && newPath[newPath.size()-1] != '\\')
		newPath += '\\';

	return newPath;
}

inline bool isCharacter(uint8 c)
{
	return (32 <= c /*&& c <= 127) || (161 <= c*/ /*&& c <= 255*/);
}

inline uint8 stripFrenchLocale(uint8 c)
{
	if (192 <= c && c <= 197) return 'a';
	if (200 <= c && c <= 203) return 'e';
	if (204 <= c && c <= 207) return 'i';
	if (210 <= c && c <= 214) return 'o';
	if (217 <= c && c <= 220) return 'u';
	if (c == 221) return 'y';
	if (224 <= c && c <= 229) return 'a';
	if (232 <= c && c <= 235) return 'e';
	if (236 <= c && c <= 239) return 'i';
	if (242 <= c && c <= 246) return 'o';
	if (249 <= c && c <= 252) return 'u';
	if (c == 253 || c == 255) return 'y';
	return c;
}

// maxRewritePaths W:/database/interfaces/anims_max

std::string rewritePath(const std::string &path, const std::string &databaseDirectory)
{
	static std::set<std::string> fileNameCache;

	std::string stdPath = standardizePath(path, false);
	for (std::string::size_type i = 0; i < stdPath.size(); ++i)
		stdPath[i] = stripFrenchLocale(stdPath[i]);
	stdPath = NLMISC::toLower(stdPath);

	// TODO: remove ./stuff/caravan/agents/_textures/actors/trame.png

	NLMISC::strFindReplace(stdPath, "w:\\database\\", databaseDirectory);
	NLMISC::strFindReplace(stdPath, "\\\\amiga\\3d\\database\\", databaseDirectory);
	NLMISC::strFindReplace(stdPath, "ma_hom_armor_01", "ma_hom_armor01");
	NLMISC::strFindReplace(stdPath, "ma_hof_armor_01", "ma_hof_armor01");
	NLMISC::strFindReplace(stdPath, "ma_hom_armor_00", "ma_hom_armor00");
	NLMISC::strFindReplace(stdPath, "ma_hof_armor_00", "ma_hof_armor00");
	NLMISC::strFindReplace(stdPath, "zo_hom_armor_00", "zo_hom_armor00");
	NLMISC::strFindReplace(stdPath, "zo_hof_armor_00", "zo_hof_armor00");
	NLMISC::strFindReplace(stdPath, "fy_hof_cheveux_shave01", "fy_hof_cheveux_shave");

	NLMISC::strFindReplace(stdPath, "matis\\agents\\_textures\\actors\\ma_hom_underwear_avtbras", "tryker\\agents\\_textures\\actors\\tr_hom_underwear_avtbras");
	NLMISC::strFindReplace(stdPath, "matis\\agents\\_textures\\actors\\ma_hom_underwear_epaule", "tryker\\agents\\_textures\\actors\\tr_hom_underwear_epaule");
	NLMISC::strFindReplace(stdPath, "matis\\agents\\_textures\\actors\\ma_hom_underwear_hand-downside", "tryker\\agents\\_textures\\actors\\tr_hom_underwear_hand-downside");
	NLMISC::strFindReplace(stdPath, "matis\\agents\\_textures\\actors\\ma_hom_underwear_hand-upside", "tryker\\agents\\_textures\\actors\\tr_hom_underwear_hand-upside");
	NLMISC::strFindReplace(stdPath, "matis\\agents\\_textures\\actors\\ma_hom_underwear_molet", "tryker\\agents\\_textures\\actors\\tr_hom_underwear_molet");
	NLMISC::strFindReplace(stdPath, "matis\\agents\\_textures\\actors\\ma_hof_underwear_hand-downside", "tryker\\agents\\_textures\\actors\\tr_hof_underwear_hand-downside");
	NLMISC::strFindReplace(stdPath, "matis\\agents\\_textures\\actors\\ma_hof_underwear_hand-upside", "tryker\\agents\\_textures\\actors\\tr_hof_underwear_hand-upside");

	NLMISC::strFindReplace(stdPath, "tr_hof_underwear_torso", "tr_hof_underwear_torse");
	NLMISC::strFindReplace(stdPath, "tr_hof_underwear_hand-", "tr_hof_underwear_hand_");

	NLMISC::strFindReplace(stdPath, "fy_hom_armor00_avbras.", "fy_hom_armor00_avbras_c1.");
	NLMISC::strFindReplace(stdPath, "fy_hom_armor00_bottes.", "fy_hom_armor00_bottes_c1.");
	NLMISC::strFindReplace(stdPath, "fy_hom_armor00_bras.", "fy_hom_armor00_bras_c1.");
	NLMISC::strFindReplace(stdPath, "fy_hom_armor00_cuissear.", "fy_hom_armor00_cuissear_c1.");
	NLMISC::strFindReplace(stdPath, "fy_hom_armor00_cuisseav.", "fy_hom_armor00_cuisseav_c1.");
	NLMISC::strFindReplace(stdPath, "fy_hom_armor00_dos.", "fy_hom_armor00_dos_c1.");
	NLMISC::strFindReplace(stdPath, "fy_hom_armor00_hand-downside.", "fy_hom_armor00_hand-downside_c1.");
	NLMISC::strFindReplace(stdPath, "fy_hom_armor00_hand-upside.", "fy_hom_armor00_hand-upside_c1.");
	NLMISC::strFindReplace(stdPath, "fy_hom_armor00_pieds.", "fy_hom_armor00_pieds_c1.");
	NLMISC::strFindReplace(stdPath, "fy_hom_armor00_torse.", "fy_hom_armor00_torse_c1.");

	NLMISC::strFindReplace(stdPath, "interfaces_visage", "visage_interface");

	if (stdPath.find("\\trame.") != std::string::npos)
		stdPath = standardizePath(databaseDirectory + "/stuff/lod_actors/texture_lod/trame.png", false);
	if (stdPath.find("\\tr_hof_visage_c1.") != std::string::npos)
		stdPath = standardizePath(databaseDirectory + "/stuff/tryker/agents/_textures/actors/tr_hof_visage_c1.png", false);
	if (stdPath.find("\\tr_hof_visage_c2.") != std::string::npos)
		stdPath = standardizePath(databaseDirectory + "/stuff/tryker/agents/_textures/actors/tr_hof_visage_c2.png", false);
	if (stdPath.find("\\tr_hof_visage_c3.") != std::string::npos)
		stdPath = standardizePath(databaseDirectory + "/stuff/tryker/agents/_textures/actors/tr_hof_visage_c3.png", false);
	if (stdPath.find("\\ma_hof_cheveux_medium02.") != std::string::npos)
		stdPath = standardizePath(databaseDirectory + "/stuff/matis/agents/_textures/actors/ma_hof_cheveux_medium02.png", false);

	/*if (stdPath.size() > path.size())
	{
		nlwarning("Path size becomes too large: '%s' -> '%s'", path.c_str(), stdPath.c_str());
		return path;
	}*/

	if (NLMISC::CFile::getFilename(stdPath) == stdPath)
	{
		breakable
		{
			if (fileNameCache.find(stdPath) == fileNameCache.end() || KnownFileCache.find(stdPath) == KnownFileCache.end())
			{
				if (stdPath[stdPath.size() - 3] == 't' && stdPath[stdPath.size() - 2] == 'g' && stdPath[stdPath.size() - 1] == 'a')
				{
					stdPath[stdPath.size() - 3] = 'p';
					stdPath[stdPath.size() - 2] = 'n';
					stdPath[stdPath.size() - 1] = 'g';
					if (fileNameCache.find(stdPath) != fileNameCache.end())
						break;
				}
				nlwarning("File name not known: '%s' ('%s')", path.c_str(), stdPath.c_str());
				return stdPath;
			}
		}
		return stdPath;
	}
	else
	{
		if (stdPath.find(databaseDirectory) == std::string::npos)
		{
			if (stdPath[1] == ':' || (stdPath[0] == '\\' && stdPath[1] == '\\'))
			{
				if (KnownFileCache.find(NLMISC::CFile::getFilename(stdPath)) != KnownFileCache.end())
					return KnownFileCache[NLMISC::CFile::getFilename(stdPath)];

				if (stdPath[stdPath.size() - 3] == 't' && stdPath[stdPath.size() - 2] == 'g' && stdPath[stdPath.size() - 1] == 'a')
				{ stdPath[stdPath.size() - 3] = 'p'; stdPath[stdPath.size() - 2] = 'n'; stdPath[stdPath.size() - 1] = 'g'; }
				if (KnownFileCache.find(NLMISC::CFile::getFilename(stdPath)) != KnownFileCache.end())
					return KnownFileCache[NLMISC::CFile::getFilename(stdPath)];

				if (stdPath[stdPath.size() - 3] == 'p' && stdPath[stdPath.size() - 2] == 'n' && stdPath[stdPath.size() - 1] == 'g')
				{ stdPath[stdPath.size() - 3] = 't'; stdPath[stdPath.size() - 2] = 'g'; stdPath[stdPath.size() - 1] = 'a'; }
				if (KnownFileCache.find(NLMISC::CFile::getFilename(stdPath)) != KnownFileCache.end())
					return KnownFileCache[NLMISC::CFile::getFilename(stdPath)];

				nlwarning("Path not in database: '%s' ('%s')", path.c_str(), stdPath.c_str());
				MissingFiles.insert(path);
				return stdPath;
			}
			else
			{
				// invalid path, don't care too much
				return stdPath;
			}
		}

		breakable
		{
			if (!NLMISC::CFile::fileExists(stdPath))
			{
				if (stdPath[stdPath.size() - 3] == 't' && stdPath[stdPath.size() - 2] == 'g' && stdPath[stdPath.size() - 1] == 'a')
				{
					stdPath[stdPath.size() - 3] = 'p';
					stdPath[stdPath.size() - 2] = 'n';
					stdPath[stdPath.size() - 1] = 'g';
					if (NLMISC::CFile::fileExists(stdPath))
						break;
				}
				{
					std::string stdPathVv2 = standardizePath(NLMISC::CFile::getPath(stdPath) + "/vv2/" + NLMISC::CFile::getFilename(stdPath), false);
					bool vv2works = false;
					if (NLMISC::CFile::fileExists(stdPathVv2))
					{
						vv2works = true;
					}
					else
					{
						if (stdPathVv2[stdPathVv2.size() - 3] == 'p' && stdPathVv2[stdPathVv2.size() - 2] == 'n' && stdPathVv2[stdPathVv2.size() - 1] == 'g')
						{
							stdPathVv2[stdPathVv2.size() - 3] = 't';
							stdPathVv2[stdPathVv2.size() - 2] = 'g';
							stdPathVv2[stdPathVv2.size() - 1] = 'a';
							if (NLMISC::CFile::fileExists(stdPathVv2))
							{
								vv2works = true;
							}
						}
					}
					if (vv2works)
					{
						// try with vv2
						/*if (stdPathVv2.size() > path.size())
						{
							nlwarning("Path with vv2 size becomes too large: '%s' -> '%s'", path.c_str(), stdPathVv2.c_str());
							return stdPath;
						}
						else
						{*/
							stdPath = stdPathVv2;
							break;
						//}
					}
				}
				// try find
				if (KnownFileCache.find(NLMISC::CFile::getFilename(stdPath)) != KnownFileCache.end())
					return KnownFileCache[NLMISC::CFile::getFilename(stdPath)];

				if (stdPath[stdPath.size() - 3] == 't' && stdPath[stdPath.size() - 2] == 'g' && stdPath[stdPath.size() - 1] == 'a')
				{ stdPath[stdPath.size() - 3] = 'p'; stdPath[stdPath.size() - 2] = 'n'; stdPath[stdPath.size() - 1] = 'g'; }
				if (KnownFileCache.find(NLMISC::CFile::getFilename(stdPath)) != KnownFileCache.end())
					return KnownFileCache[NLMISC::CFile::getFilename(stdPath)];

				if (stdPath[stdPath.size() - 3] == 'p' && stdPath[stdPath.size() - 2] == 'n' && stdPath[stdPath.size() - 1] == 'g')
				{ stdPath[stdPath.size() - 3] = 't'; stdPath[stdPath.size() - 2] = 'g'; stdPath[stdPath.size() - 1] = 'a'; }
				if (KnownFileCache.find(NLMISC::CFile::getFilename(stdPath)) != KnownFileCache.end())
					return KnownFileCache[NLMISC::CFile::getFilename(stdPath)];

				nlwarning("Path file does not exist: '%s' ('%s')", path.c_str(), stdPath.c_str());
				MissingFiles.insert(path);
				return stdPath;
			}
		}
	}

	fileNameCache.insert(NLMISC::CFile::getFilename(stdPath));
	return stdPath;
}

class CMaxRewritePathsCommand : public NLMISC::IRunnable
{
public:
	NLMISC::CLog *Log;
	std::string SrcDirectoryRecursive;
	std::string DatabaseDirectory;

	virtual void getName(std::string &result) const
	{ result = "CMaxRewritePathsCommand"; }

	void doFile(const std::string &filePath)
	{
		if (filePath[filePath.size() - 3] == 'm' && filePath[filePath.size() - 2] == 'a' && filePath[filePath.size() - 1] == 'x')
		{
			nldebug("File: '%s'", filePath.c_str());

			std::vector<char> buffer;
			buffer.resize(NLMISC::CFile::getFileSize(filePath));

			// read
			{
				NLMISC::CIFile ifile;
				ifile.open(filePath, false);
				ifile.serialBuffer((uint8 *)(&buffer[0]), buffer.size());
				ifile.close();
			}

			// find paths
			for (std::vector<char>::size_type i = 256; i < buffer.size(); ++i) // skip the first 256 lol :)
			{
				if (((NLMISC::toLower(buffer[i - 1]) == 'x' && NLMISC::toLower(buffer [i - 2]) == 'a' && NLMISC::toLower(buffer[i - 3]) == 'm')
					|| (NLMISC::toLower(buffer[i - 1]) == 'a' && NLMISC::toLower(buffer [i - 2]) == 'g' && NLMISC::toLower(buffer[i - 3]) == 't')
					|| (NLMISC::toLower(buffer[i - 1]) == 'g' && NLMISC::toLower(buffer [i - 2]) == 'n' && NLMISC::toLower(buffer[i - 3]) == 'p'))
					&& (NLMISC::toLower(buffer[i - 4]) == '.'))
				{
					// buffer[i] is the character after the path! :)
					std::vector<char>::size_type beginPath = 0;
					for (std::vector<char>::size_type j = i - 4; j > 0; --j)
					{
						if (!isCharacter(buffer[j]))
						{
							if ((buffer[j + 1] == '\\' && buffer[j + 2] == '\\') || buffer[j] == 0)
							{
								beginPath = j + 1;
								break;
							}
							// nlwarning("Invalid characters '%i' in path at %i, len %i!", (uint32)buffer[j], (uint32)j, (uint32)(i - j));
							// beginPath = j + 1; // test
							break;
						}
						if (buffer[j] == ':')
						{
							beginPath = j - 1;
							break;
						}
					}
					if (beginPath != 0)
					{
						std::vector<char>::size_type sizePath = i - beginPath;
						std::string foundPath = std::string(&buffer[beginPath], sizePath); //std::string(buffer.at(beginPath), buffer.at(i));
						//nldebug("Found path: '%s' from %i to %i", foundPath.c_str(), (uint32)beginPath, (uint32)i);
						std::string fixedPath = rewritePath(foundPath, DatabaseDirectory);
						//nldebug("Rewrite to: '%s'", fixedPath.c_str());
					}
				}
			}

			//NLMISC::CFile::re

			// ...
		}
	}

	// maxRewritePaths W:/database/interfaces/anims_max

	void doDirectory(const std::string &directoryPath)
	{
		nldebug("Directory: '%s'", directoryPath.c_str());

		std::string dirPath = standardizePath(directoryPath, true);
		std::vector<std::string> dirContents;

		NLMISC::CPath::getPathContent(dirPath, false, true, true, dirContents);

		for (std::vector<std::string>::iterator it = dirContents.begin(), end = dirContents.end(); it != end; ++it)
		{
			const std::string &subPath = standardizePath(*it, false);

			if (NLMISC::CFile::isDirectory(subPath))
			{
				if (subPath.find("\\.") == std::string::npos)
					doDirectory(subPath);
			}
			else
				doFile(subPath);

			if (PIPELINE::IPipelineInterface::getInstance()->isExiting())
				return;
		}
	}

	virtual void run()
	{
		std::string tempDirectory = PIPELINE::IPipelineProcess::getInstance()->getTempDirectory();
		DatabaseDirectory = standardizePath(PIPELINE::IPipelineInterface::getInstance()->getConfigFile().getVar("DatabaseDirectory").asString(0), true);
		nlinfo("DatabaseDirectory: '%s'", DatabaseDirectory.c_str());

		doDirectory(SrcDirectoryRecursive);

		for (std::set<std::string>::iterator it = MissingFiles.begin(), end = MissingFiles.end(); it != end; ++it)
			nlinfo("Missing: '%s'", (*it).c_str());

		PIPELINE::IPipelineInterface::getInstance()->endedRunnableTask();
	}
};
CMaxRewritePathsCommand s_MaxRewritePathsCommand;

class CMaxRewriteInitCacheCommand : public NLMISC::IRunnable
{
public:
	NLMISC::CLog *Log;
	std::string SrcDirectoryRecursive;
	std::string DatabaseDirectory;

	virtual void getName(std::string &result) const
	{ result = "CMaxRewriteInitCacheCommand"; }

	void doFile(const std::string &filePath)
	{
		KnownFileCache[NLMISC::CFile::getFilename(filePath)] = standardizePath(filePath, true);
	}

	// maxRewritePaths W:/database/interfaces/anims_max

	void doDirectory(const std::string &directoryPath)
	{
		nldebug("Directory: '%s'", directoryPath.c_str());

		std::string dirPath = standardizePath(directoryPath, true);
		std::vector<std::string> dirContents;

		NLMISC::CPath::getPathContent(dirPath, false, true, true, dirContents);

		for (std::vector<std::string>::iterator it = dirContents.begin(), end = dirContents.end(); it != end; ++it)
		{
			const std::string &subPath = standardizePath(*it, false);

			if (NLMISC::CFile::isDirectory(subPath))
			{
				if (subPath.find("\\.") == std::string::npos)
					doDirectory(subPath);
			}
			else
				doFile(subPath);

			if (PIPELINE::IPipelineInterface::getInstance()->isExiting())
				return;
		}
	}

	virtual void run()
	{
		std::string tempDirectory = PIPELINE::IPipelineProcess::getInstance()->getTempDirectory();
		DatabaseDirectory = standardizePath(PIPELINE::IPipelineInterface::getInstance()->getConfigFile().getVar("DatabaseDirectory").asString(0), true);
		nlinfo("DatabaseDirectory: '%s'", DatabaseDirectory.c_str());

		doDirectory(SrcDirectoryRecursive);

		PIPELINE::IPipelineInterface::getInstance()->endedRunnableTask();
	}
};
CMaxRewriteInitCacheCommand s_MaxRewriteInitCacheCommand;

} /* anonymous namespace */

} /* namespace PIPELINE */

NLMISC_CATEGORISED_COMMAND(max, maxRewriteInitCache, "Find all .tga, .png and .max files", "<srcDirectoryRecursive>")
{
	if(args.size() != 1) return false;
	PIPELINE::s_MaxRewriteInitCacheCommand.Log = &log;
	PIPELINE::s_MaxRewriteInitCacheCommand.SrcDirectoryRecursive = args[0];
	if (!PIPELINE::IPipelineInterface::getInstance()->tryRunnableTask("COMMAND_MAX_REWRITE_INIT_CACHE", &PIPELINE::s_MaxRewriteInitCacheCommand))
	{
		log.displayNL("Busy.");
		return false;
	}
	return true;
}

NLMISC_CATEGORISED_COMMAND(max, maxRewritePaths, "Rewrite all paths to .tga, .png and .max files found in a max file", "<srcDirectoryRecursive>")
{
	if(args.size() != 1) return false;
	PIPELINE::s_MaxRewritePathsCommand.Log = &log;
	PIPELINE::s_MaxRewritePathsCommand.SrcDirectoryRecursive = args[0];
	if (!PIPELINE::IPipelineInterface::getInstance()->tryRunnableTask("COMMAND_MAX_REWRITE_PATHS", &PIPELINE::s_MaxRewritePathsCommand))
	{
		log.displayNL("Busy.");
		return false;
	}
	return true;
}

/* end of file */
