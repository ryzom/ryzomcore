/*
 * Copyright (C) 2012  by Jan Boon (Kaetemi)
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
#include <nel/misc/common.h>

#include <gsf/gsf-outfile-msole.h>
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-utils.h>
#include <gsf/gsf-doc-meta-data.h>
#include <gsf/gsf-msole-utils.h>
#include <glib/gi18n.h>
#include <string.h>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <vector>
#include <utility>

#include <nel/misc/file.h>
#include <nel/misc/vector.h>
#include "nel/misc/dynloadlib.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/algo.h"
#include "nel/misc/file.h"
#include "nel/misc/mem_stream.h"

#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/class_data.h"
#include "../pipeline_max/config.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"

// Testing
#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"

#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/storage/geom_buffers.h"
#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/update1/editable_mesh.h"
#include "../pipeline_max/epoly/editable_poly.h"

#include <boost/algorithm/string.hpp>
using namespace std;
using namespace boost::algorithm;

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::BUILTIN::STORAGE;
using namespace PIPELINE::MAX::UPDATE1;
using namespace PIPELINE::MAX::EPOLY;

CSceneClassRegistry SceneClassRegistry;

// Never enable this
bool DebugParser = false;

bool DisplayStream = false;
bool DisplayReplaces = false;

bool ReplacePaths = true;
bool ReplaceMapExt = false;

bool WritePathChangesOnly = true;
bool WriteModified = true;
bool WriteDummy = false;

bool HaltOnIssue = false;

const char *DatabaseDirectory = "w:\\database\\";
const char *LinuxDatabaseDirectory = "/mnt/y/ryzom-assets/database/";
bool RunningLinux = true;

//const char *SrcDirectoryRecursive = "w:\\database\\interfaces\\";
//const char *SrcDirectoryRecursive = "w:\\database\\";
//const char *SrcDirectoryRecursive = "w:\\database\\stuff\\fyros\\city\\newpositionville\\";
const char *SrcDirectoryRecursiveInit = "w:\\database\\";
//const char *SrcDirectoryRecursiveHandle = "w:\\database\\stuff\\generique\\agents\\accessories\\";
//const char *SrcDirectoryRecursiveHandle = "w:\\database\\landscape\\ligo\\primes_racines\\max\\";
const char *SrcDirectoryRecursiveHandle = "w:\\database\\stuff\\fyros\\decors\\constructions\\";

bool UseFallbackTga = false;
const char *FallbackTga = "w:\\database\\stuff\\lod_actors\\texture_lod\\trame.png";

std::set<std::string> MissingFiles;
std::map<std::string, std::string> KnownFileCache;

std::string nativeDatabasePath(const std::string &standardizedPath)
{
	if (RunningLinux)
	{
		std::string result = standardizedPath;
		NLMISC::strFindReplace(result, DatabaseDirectory, LinuxDatabaseDirectory);
		while (NLMISC::strFindReplace(result, "\\", "/")) { }
		return result;
	}
	return standardizedPath;
}

std::string unnativeDatabasePath(const std::string &nativizedPath)
{
	if (RunningLinux)
	{
		std::string result = nativizedPath;
		NLMISC::strFindReplace(result, LinuxDatabaseDirectory, DatabaseDirectory);
		while (NLMISC::strFindReplace(result, "/", "\\")) { }
		return result;
	}
	return nativizedPath;
}

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

inline bool isCharacter(char c0)
{
	uint8 c = *(uint8 *)(void *)(&c0);
	return (32 <= c /*&& c <= 127) || (161 <= c*/ /*&& c <= 255*/);
}

inline char stripFrenchLocale(char c0)
{
	uint8 c = *(uint8 *)(void *)(&c0);
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
	return c0;
}

std::string rewritePath(const std::string &path, const std::string &databaseDirectory)
{
	// nldebug("Rewrite: %s", path.c_str());
	
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

	NLMISC::strFindReplace(stdPath, "fy-hof-gilet02-dos", "fy_hof_gilet02_arriere_n");
	NLMISC::strFindReplace(stdPath, "fy-hof-pantalon02-arriere", "fy_hof_pantalon02_arriere_n");
	NLMISC::strFindReplace(stdPath, "fy-hof-pantalon02-avant", "fy_hof_pantalon02_avant_n");
	NLMISC::strFindReplace(stdPath, "fy-hom-blouson02-avantbras", "fy_hom_blouson02_avbras_n");
	NLMISC::strFindReplace(stdPath, "fy-hom-blouson02-bras", "fy_hom_blouson02_bras_n");
	NLMISC::strFindReplace(stdPath, "fy-hom-blouson02-dos", "fy_hom_blouson02_arriere_n");
	NLMISC::strFindReplace(stdPath, "fy-hom-blouson02-torse", "fy_hom_blouson02_avant_n");
	NLMISC::strFindReplace(stdPath, "fy-hom-pantalon02-mollet", "fy_hom_pantalon02_mollet_n");
	NLMISC::strFindReplace(stdPath, "fy-hom-shoes02", "fy_hom_shoes02_shoes_n");
	NLMISC::strFindReplace(stdPath, "fyhof-gilet02-torse", "fy_hof_gilet02_torse_n");

	NLMISC::strFindReplace(stdPath, "ge_hom_armor02_armpad", "ge_hom_armor02_avbras");
	NLMISC::strFindReplace(stdPath, "ge_hom_armor02_bottes", "ge_hom_armor02_pied");
	NLMISC::strFindReplace(stdPath, "ge_hom_armor02_cuisse-arr", "ge_hom_armor02_cuisse_arr");
	NLMISC::strFindReplace(stdPath, "ge_hom_armor02_cuisse-avt", "ge_hom_armor02_cuisse_avt");
	NLMISC::strFindReplace(stdPath, "ge_hom_armor02_hand-down", "ge_hom_armor02_hand_down");
	NLMISC::strFindReplace(stdPath, "ge_hom_armor02_hand-up", "ge_hom_armor02_hand_up");
	NLMISC::strFindReplace(stdPath, "ge_hom_armor02_torse", "ge_hom_armor02_torso");

	NLMISC::strFindReplace(stdPath, "fy_lovejail.", "fy_lovejail_su.");
	NLMISC::strFindReplace(stdPath, "\\caissebonusaction.", "\\g_caissebonus.");
	NLMISC::strFindReplace(stdPath, "\\sactoile.", "\\g_sactoile.");
	NLMISC::strFindReplace(stdPath, "\\ge_mission_stele_kami0.", "\\ge_mission_stele_kami.");
	NLMISC::strFindReplace(stdPath, "\\ge_mission_reward_karavan_bigshield.", "\\ge_mission_reward_karavan_bigshield_c1.");

	NLMISC::strFindReplace(stdPath, "\\tr_hom_underwear_cuisse_arr.", "\\tr_hom_underwear_cuisse-arr.");
	NLMISC::strFindReplace(stdPath, "\\tr_hom_underwear_cuisse_avt.", "\\tr_hom_underwear_cuisse-avt.");

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
				//#nlwarning("File name not known: '%s' ('%s')", path.c_str(), stdPath.c_str());
				// MissingFiles.insert(path);
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

				//#nlwarning("Path not in database: '%s' ('%s')", path.c_str(), stdPath.c_str());
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
			if (!NLMISC::CFile::fileExists(nativeDatabasePath(stdPath)))
			{
				if (stdPath[stdPath.size() - 3] == 't' && stdPath[stdPath.size() - 2] == 'g' && stdPath[stdPath.size() - 1] == 'a')
				{
					stdPath[stdPath.size() - 3] = 'p';
					stdPath[stdPath.size() - 2] = 'n';
					stdPath[stdPath.size() - 1] = 'g';
					if (NLMISC::CFile::fileExists(nativeDatabasePath(stdPath)))
						break;
				}
				{
					std::string stdPathVv2 = standardizePath(NLMISC::CFile::getPath(stdPath) + "/vv2/" + NLMISC::CFile::getFilename(stdPath), false);
					bool vv2works = false;
					if (NLMISC::CFile::fileExists(nativeDatabasePath(stdPathVv2)))
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
							if (NLMISC::CFile::fileExists(nativeDatabasePath(stdPathVv2)))
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

				//#nlwarning("Path file does not exist: '%s' ('%s')", path.c_str(), stdPath.c_str());
				MissingFiles.insert(path);
				return stdPath;
			}
		}
	}

	fileNameCache.insert(NLMISC::CFile::getFilename(stdPath));
	return stdPath;
}

void doFileInitialize(const std::string &filePath)
{
	// nldebug("File: '%s'", filePath.c_str());
	// nldebug("Native: '%s'", nativeDatabasePath(filePath).c_str());

	KnownFileCache[NLMISC::CFile::getFilename(filePath)] = standardizePath(filePath, false);
}

// maxRewritePaths W:/database/interfaces/anims_max

void doDirectoryInitialize(const std::string &directoryPath)
{
	nldebug("Directory: '%s'", directoryPath.c_str());

	std::string dirPath = standardizePath(directoryPath, true);
	std::vector<std::string> dirContents;

	NLMISC::CPath::getPathContent(nativeDatabasePath(dirPath), false, true, true, dirContents);

	// nldebug("Native: '%s'", nativeDatabasePath(dirPath).c_str());
	// nldebug("Contents: %i", dirContents.size());

	for (std::vector<std::string>::iterator it = dirContents.begin(), end = dirContents.end(); it != end; ++it)
	{
		std::string subPath = standardizePath(unnativeDatabasePath(*it), false);
		// nldebug("Path: '%s'", subPath.c_str());
		// nldebug("Native: '%s'", nativeDatabasePath(subPath + " ").c_str());

		if (NLMISC::CFile::isDirectory(nativeDatabasePath(subPath)))
		{
			if (subPath.find("\\.") == std::string::npos)
				doDirectoryInitialize(subPath);
		}
		else
			doFileInitialize(subPath);
	}
}

void runInitialize()
{
	nlinfo("DatabaseDirectory: '%s'", DatabaseDirectory);

	doDirectoryInitialize(std::string(SrcDirectoryRecursiveInit));
}

// Scary stuff
void doFileScanner(const std::string &filePath)
{
	if (filePath[filePath.size() - 3] == 'm' && filePath[filePath.size() - 2] == 'a' && filePath[filePath.size() - 1] == 'x')
	{
		nldebug("File: '%s'", filePath.c_str());

		std::vector<char> buffer;
		buffer.resize(NLMISC::CFile::getFileSize(nativeDatabasePath(filePath)));

		// read
		{
			NLMISC::CIFile ifile;
			ifile.open(nativeDatabasePath(filePath), false);
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

void doDirectoryScanner(const std::string &directoryPath)
{
	nldebug("Directory: '%s'", directoryPath.c_str());

	std::string dirPath = standardizePath(directoryPath, true);
	std::vector<std::string> dirContents;

	NLMISC::CPath::getPathContent(nativeDatabasePath(dirPath), false, true, true, dirContents);

	for (std::vector<std::string>::iterator it = dirContents.begin(), end = dirContents.end(); it != end; ++it)
	{
		std::string subPath = standardizePath(unnativeDatabasePath(*it), false);

		if (NLMISC::CFile::isDirectory(nativeDatabasePath(subPath)))
		{
			if (subPath.find("\\.") == std::string::npos)
				doDirectoryScanner(subPath);
		}
		else
			doFileScanner(subPath);
	}
}

void runScanner()
{
	nlinfo("DatabaseDirectory: '%s'", DatabaseDirectory);

	doDirectoryScanner(SrcDirectoryRecursiveHandle);

	for (std::set<std::string>::iterator it = MissingFiles.begin(), end = MissingFiles.end(); it != end; ++it)
		nlinfo("Missing: '%s'", (*it).c_str());
}

void handleFile(const std::string &path);

void doFileHandler(const std::string &filePath)
{
	if (filePath[filePath.size() - 3] == 'm' && filePath[filePath.size() - 2] == 'a' && filePath[filePath.size() - 1] == 'x')
	{
		nldebug("File: '%s'", filePath.c_str());

		handleFile(nativeDatabasePath(filePath));
	}
}

void doDirectoryHandler(const std::string &directoryPath)
{
	nldebug("Directory: '%s'", directoryPath.c_str());

	std::string dirPath = standardizePath(directoryPath, true);
	std::vector<std::string> dirContents;

	NLMISC::CPath::getPathContent(nativeDatabasePath(dirPath), false, true, true, dirContents);

	for (std::vector<std::string>::iterator it = dirContents.begin(), end = dirContents.end(); it != end; ++it)
	{
		std::string subPath = standardizePath(unnativeDatabasePath(*it), false);

		if (NLMISC::CFile::isDirectory(nativeDatabasePath(subPath)))
		{
			if (subPath.find("\\.") == std::string::npos)
				doDirectoryHandler(subPath);
		}
		else
			doFileHandler(subPath);
	}
}

void runHandler()
{
	nlinfo("DatabaseDirectory: '%s'", DatabaseDirectory);

	doDirectoryHandler(SrcDirectoryRecursiveHandle);
}

void serializeStorageContainer(PIPELINE::MAX::CStorageContainer *storageContainer, GsfInfile *infile, const char *streamName)
{
	GsfInput *input = gsf_infile_child_by_name(infile, streamName);
	if (!input)
	{
		nlerror("GSF Could not read stream %s", streamName);
		return;
	}
	{
		PIPELINE::MAX::CStorageStream instream(input);
		storageContainer->serial(instream);
	}
	g_object_unref(input);
}

void serializeStorageContainer(PIPELINE::MAX::CStorageContainer *storageContainer, GsfOutfile *outfile, const char *streamName)
{
	//nldebug("write");
	GsfOutput *output = GSF_OUTPUT(gsf_outfile_new_child(outfile, streamName, false));
	if (!output)
	{
		nlerror("GSF Could not write stream %s", streamName);
		return;
	}
	{
		PIPELINE::MAX::CStorageStream outstream(output);
		storageContainer->serial(outstream);
	}
	gsf_output_close(output);
	g_object_unref(G_OBJECT(output));
}

void serializeRaw(std::vector<uint8> &rawOutput, GsfInfile *infile, const char *streamName)
{
	GsfInput *input = gsf_infile_child_by_name(infile, streamName);
	if (!input)
	{
		nlerror("GSF Could not read stream %s", streamName);
		return;
	}
	{
		PIPELINE::MAX::CStorageStream instream(input);
		rawOutput.resize(instream.size());
		instream.serialBuffer(&rawOutput[0], rawOutput.size());
	}
	g_object_unref(input);
}

void serializeRaw(std::vector<uint8> &rawOutput, GsfOutfile *outfile, const char *streamName)
{
	GsfOutput *output = GSF_OUTPUT(gsf_outfile_new_child(outfile, streamName, false));
	if (!output)
	{
		nlerror("GSF Could not write stream %s", streamName);
		return;
	}
	{
		PIPELINE::MAX::CStorageStream outstream(output);
		outstream.serialBuffer(&rawOutput[0], rawOutput.size());
	}
	gsf_output_close(output);
	g_object_unref(G_OBJECT(output));
}

std::string cleanString(const std::string &str)
{
	std::string res = str;
	trim(res);
	// \\Amiga\3D (10 chars)
	if (res.size() > 10)
	{
		if (res.substr(res.size() - 10) == "\\\\Amiga\\3D")
			res = res.substr(0, res.size() - 10);
	}
	if (res.size() > 1)
	{
		if (res.substr(res.size() - 1) == "0")
			res = res.substr(0, res.size() - 1);
	}
	if (res.size() > 4 && res[res.size() - 1] == '.')
	{
		if (res.substr(res.size() - 4) == "max.") // fix a stupid typo
			res = res.substr(0, res.size() - 1);
	}
	return res;
}

std::string rewritePathFinal(const std::string &str)
{
	std::string strtrimmed = cleanString(str);
	std::string result = rewritePath(strtrimmed, DatabaseDirectory);
	if (NLMISC::CFile::getFilename(result) != result && !NLMISC::CFile::fileExists(nativeDatabasePath(result)) &&
		((result[result.size() - 3] == 't' && result[result.size() - 2] == 'g' && result[result.size() - 1] == 'a') || (result[result.size() - 3] == 'p' && result[result.size() - 2] == 'n' && result[result.size() - 1] == 'g'))
		)
	{
		// nlwarning("Replacing missing '%s' with '%s'", result.c_str(), FallbackTga);
		if (UseFallbackTga)
		{
			return FallbackTga;
		}
		else
		{
			return NLMISC::CFile::getFilename(result);
		}
	}
	if (DisplayReplaces)
	{
		nldebug("Replacing '%s' with '%s'", str.c_str(), result.c_str());
	}
	return result;
}

bool isImportantFilePath(const std::string &str)
{
	// nldebug("Is important? %s", str.c_str());
	
	std::string strtrimmed = cleanString(str);
	if (strtrimmed.size() >= 4)
	{
		std::string strlw = NLMISC::toLower(strtrimmed);
		return (strlw[strlw.size() - 3] == 'm' && strlw[strlw.size() - 2] == 'a' && strlw[strlw.size() - 1] == 'x')
			|| (strlw[strlw.size() - 3] == 't' && strlw[strlw.size() - 2] == 'g' && strlw[strlw.size() - 1] == 'a')
			|| (strlw[strlw.size() - 3] == 'p' && strlw[strlw.size() - 2] == 'n' && strlw[strlw.size() - 1] == 'g');
	}
	return false;
}

bool hasImportantFilePath(CStorageRaw *raw)
{
	if (raw->Value.size() >= 4)
	{
		// Find any occurences of .max, .png or .tga in ascii or utf16
		for (uint i = 0; i < raw->Value.size() - 3; ++i)
		{
			if (NLMISC::toLower(((char *)&raw->Value[0])[i]) == '.'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 1]) == 'm'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 2]) == 'a'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 3]) == 'x')
				return true;
			if (NLMISC::toLower(((char *)&raw->Value[0])[i]) == '.'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 1]) == 't'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 2]) == 'g'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 3]) == 'a')
				return true;
			if (NLMISC::toLower(((char *)&raw->Value[0])[i]) == '.'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 1]) == 'p'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 2]) == 'n'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 3]) == 'g')
				return true;
		}
	}
	if (raw->Value.size() >= 6)
	{
		for (uint i = 0; i < raw->Value.size() - 6; ++i)
		{
			if (NLMISC::toLower(((char *)&raw->Value[0])[i]) == '.'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 2]) == 'm'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 4]) == 'a'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 6]) == 'x')
				return true;
			if (NLMISC::toLower(((char *)&raw->Value[0])[i]) == '.'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 2]) == 't'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 4]) == 'g'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 6]) == 'a')
				return true;
			if (NLMISC::toLower(((char *)&raw->Value[0])[i]) == '.'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 2]) == 'p'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 4]) == 'n'
				&& NLMISC::toLower(((char *)&raw->Value[0])[i + 6]) == 'g')
				return true;
		}
	}
	return false;
}

bool fixChunk(uint16 id, IStorageObject *chunk)
{
	if (id == 4656)
	{
		// nldebug("testing 4656: %s", chunk->toString().c_str());
	}
	bool changed = false;
	CStorageValue<std::string> *asString = dynamic_cast<CStorageValue<std::string> *>(chunk);
	if (asString)
	{
		// nldebug("String: %s", asString->Value.c_str());
		if (isImportantFilePath(asString->Value))
		{
			std::string rewritten = rewritePathFinal(asString->Value);
			if (rewritten != asString->Value)
			{
				asString->Value = rewritten;
				changed = true;
			}
		}
		return changed;
	}
	CStorageValue<ucstring> *asUCString = dynamic_cast<CStorageValue<ucstring> *>(chunk);
	if (asUCString)
	{
		// nldebug("UCString: %s", asUCString->Value.toUtf8().c_str());
		if (isImportantFilePath(asUCString->Value.toString()))
		{
			std::string rewritten = rewritePathFinal(asUCString->Value.toString());
			if (rewritten != asUCString->Value.toString())
			{
				asUCString->Value.fromUtf8(rewritten);
				changed = true;
			}
		}
		return changed;
	}
	CStorageRaw *asRaw = dynamic_cast<CStorageRaw *>(chunk);
	if (asRaw)
	{
		switch (id)
		{
		case 4656:
			// nldebug("4656: %s", chunk->toString().c_str());
		case 256:
		case 16385:
		case 288: // .max xref in classdata
			if (hasImportantFilePath(asRaw))
			{
				// generic ucstring really
				nlassert(asRaw->Value.size() % 2 == 0);
				ucstring str;
				str.resize(asRaw->Value.size() / 2);
				memcpy(&str[0], &asRaw->Value[0], asRaw->Value.size());
				// nldebug("[%s]", str.toString().c_str());
				nlassert(isImportantFilePath(str.toString()));
				std::string rewritten = rewritePathFinal(str.toString());
				if (rewritten != str.toString())
				{
					str.fromUtf8(rewritten);
					asRaw->Value.resize(str.size() * 2);
					memcpy(&asRaw->Value[0], &str[0], asRaw->Value.size());
					changed = true;
				}
				break;
			}
		case 10:
			if (hasImportantFilePath(asRaw))
			{
				// 10 00 08 00 00 00 02 00 80 00 40 // 11 bytes O_O
				// 4d 00 00 00
				// 57 3a 5c 44 61 74 61 62 61 73
				// 65 5c 73 74 75 66 66 5c 74 72
				// 79 6b 65 72 5c 61 67 65 6e 74
				// 73 5c 5f 74 65 78 74 75 72 65
				// 73 5c 61 63 74 6f 72 73 5c 54
				// 52 5f 48 4f 46 5f 63 69 76 69
				// 6c 30 31 5f 74 6f 72 73 6f 5f
				// 43 31 2e 74 67 61 00
				bool overrideFF = true; // Patch for some ligo max files
				if (overrideFF && asRaw->Value.size() > 4 && asRaw->Value[asRaw->Value.size() - 4] == 0xFF)
				{
					nlwarning("0xFFFFFFFF");
				}
				else if (!(asRaw->Value[asRaw->Value.size() - 1] == 0))
				{
					nlinfo("Id: %i, size: %i", (uint32)id, asRaw->Value.size());
					asRaw->toString(std::cout);
					nldebug("x");
					nlwarning("not null term");
					if (HaltOnIssue)
					{
						std::string x;
						std::cin >> x;
					}
					break;
				}
				uint32 size = ((uint32 *)&asRaw->Value[11])[0];
				if (!(asRaw->Value.size() == size + 4 + 11))
				{
					nlinfo("Id: %i, size: %i", (uint32)id, asRaw->Value.size());
					asRaw->toString(std::cout);
					nldebug("x");
					nlwarning("size '%i' does not match '%i', use different algo :)", size, asRaw->Value.size() - 4 - 11);
					uint8 nonsense[11];
					uint32 counter;
					std::vector<std::string> strings;
					{
						NLMISC::CMemStream mem;
						asRaw->serial(mem);
						mem.invert();
						mem.serialBuffer(nonsense, 11);
						mem.serial(counter);
						uint i = 0;
						while ((sint)mem.getPos() != (sint)mem.size())
						{
							//nldebug("pos %i", mem.getPos());
							//nldebug("size %i", mem.size());
							char funny;
							mem.serial(funny);
							nlassert(funny == '@');
							sint32 size;
							mem.serial(size);
							//nldebug("size %i", size);
							if (!overrideFF && size == -1)
							{
								nldebug("size %i", size);
								nlwarning("bad size");
								if (HaltOnIssue)
								{
									std::string x;
									std::cin >> x;
								}
								return changed;
							}
							std::string v;
							if (overrideFF && size == -1)
							{
								v.resize(1);
								*(uint8 *)&v[0] = 0;
							}
							else
							{
								v.resize(size);
								mem.serialBuffer((uint8 *)&v[0], size);
							}
							if (!(v[v.size() - 1] == 0))
							{
								nlinfo("Id: %i, size: %i", (uint32)id, asRaw->Value.size());
								asRaw->toString(std::cout);
								nldebug("x");
								nlwarning("not null term inside array stuff %i '%s'", i, v.c_str());
								if (HaltOnIssue)
								{
									std::string x;
									std::cin >> x;
								}
								return changed;
							}
							v.resize(v.size() - 1);
							// nldebug("%s", v.c_str());
							strings.push_back(v);
							++i;
							// nldebug("ok");
						}
						nlassert(strings.size() == counter);
						asRaw->Value.resize(0);
					}
					bool foundone = false;
					for (uint i = 0; i < strings.size(); ++i)
					{
						if (isImportantFilePath(strings[i]))
						{
							foundone = true;
							std::string rewritten = rewritePathFinal(strings[i]);
							if (rewritten != strings[i])
							{
								strings[i] = rewritten;
								changed = true;
							}
						}
					}
					nlassert(foundone);
					{
						//nldebug("go");
						NLMISC::CMemStream mem;
						mem.serialBuffer(nonsense, 11);
						mem.serial(counter);
						for (uint i = 0; i < strings.size(); ++i)
						{
							//nldebug("one");
							char funny = '@';
							mem.serial(funny);
							strings[i].resize(strings[i].size() + 1);
							strings[i][strings[i].size() - 1] = 0;
							uint32 size = strings[i].size();
							mem.serial(size);
							mem.serialBuffer((uint8 *)&strings[i][0], size);
						}
						asRaw->setSize(mem.getPos());
						mem.invert();
						asRaw->serial(mem);
					}
					//std::string x;
					//std::cin >> x;
					return changed;
				}
				std::string str;
				str.resize(size - 1);
				memcpy(&str[0], &asRaw->Value[15], str.size());
				// nldebug("test '%s'", str.c_str());
				// asRaw->toString(std::cout);
				if (!isImportantFilePath(str))
				{
					nlinfo("Id: %i, size: %i", (uint32)id, asRaw->Value.size());
					asRaw->toString(std::cout);
					nldebug("x");
					nlwarning("not important");
					if (HaltOnIssue)
					{
						std::string x;
						std::cin >> x;
					}
					break;
				}
				std::string rewritten = rewritePathFinal(str);
				if (rewritten != str)
				{
					str = rewritten;
					asRaw->Value.resize(str.size() + 11 + 4 + 1);
					memcpy(&asRaw->Value[15], &str[0], str.size());
					((uint32 *)&asRaw->Value[11])[0] = str.size() + 1;
					asRaw->Value[asRaw->Value.size() - 1] = 0;
					changed = true;
				}
				break;
			}
		case 304:
			if (hasImportantFilePath(asRaw))
			{
				// null term c string
				nlassert(asRaw->Value[asRaw->Value.size() - 1] == 0);
				std::string str;
				str.resize(asRaw->Value.size() - 1);
				memcpy(&str[0], &asRaw->Value[0], str.size());
				if (!isImportantFilePath(str))
				{
					nlinfo("Id: %i", (uint32)id);
					asRaw->toString(std::cout);
					nlerror("not important");
				}
				std::string rewritten = rewritePathFinal(str);
				if (rewritten != str)
				{
					str = rewritten;
					asRaw->Value.resize(str.size() + 1);
					memcpy(&asRaw->Value[0], &str[0], str.size());
					asRaw->Value[asRaw->Value.size() - 1] = 0;
					changed = true;
				}
				break;
			}
		case 9730:
			if (asRaw->Value.size() > 0 && asRaw->Value[0] == 'I')
			{
				// ignore Init.max
				break;
			}
		default:
			if (hasImportantFilePath(asRaw))
			{
				nlinfo("Id: %i", (uint32)id);
				asRaw->toString(std::cout);
				nlwarning("Found important file path");
				if (HaltOnIssue)
				{
					std::string x;
					std::cin >> x;
				}
				return changed;
			}
			break;
		}
	}
	return changed;
}

bool fixChunks(CStorageContainer *container)
{
	bool changed = false;
	for (CStorageContainer::TStorageObjectConstIt it = container->chunks().begin(), end = container->chunks().end(); it != end; ++it)
	{
		if (it->second->isContainer())
		{
			changed = (changed || fixChunks(static_cast<CStorageContainer *>(it->second)));
		}
		else
		{
			changed = (changed || fixChunk(it->first, it->second));
		}
	}
	return changed;
}

void handleFile(const std::string &path)
{
	GError *err = NULL;

	GsfDocMetaData *metadata = gsf_doc_meta_data_new();
	nlassert(metadata);

	GsfInput *src = gsf_input_stdio_new(path.c_str(), &err);
	if (err) { nlerror("GSF Failed to open %s", path.c_str()); return; }

	GsfInfile *infile = gsf_infile_msole_new(src, NULL);
	if (!infile) { nlerror("GSF Failed to recognize %s", path.c_str()); return; }
	g_object_unref(src);

	uint8 classId[16];
	if (!gsf_infile_msole_get_class_id((GsfInfileMSOle *)infile, classId))
		nlerror("GSF Did not find classId");

	PIPELINE::MAX::CStorageContainer videoPostQueue;
	serializeStorageContainer(&videoPostQueue, infile, "VideoPostQueue");
	PIPELINE::MAX::CStorageContainer config;
	serializeStorageContainer(&config, infile, "Config");
	PIPELINE::MAX::CStorageContainer classData;
	serializeStorageContainer(&classData, infile, "ClassData");

	std::vector<uint8> summaryInformation;
	serializeRaw(summaryInformation, infile, "\05SummaryInformation");

	std::vector<uint8> documentSummaryInformation;
	serializeRaw(documentSummaryInformation, infile, "\05DocumentSummaryInformation"); // Can't read this, don't care.

	PIPELINE::MAX::CDllDirectory dllDirectory;
	serializeStorageContainer(&dllDirectory, infile, "DllDirectory");
	dllDirectory.parse(VersionUnknown);

	PIPELINE::MAX::CClassDirectory3 classDirectory3(&dllDirectory);
	serializeStorageContainer(&classDirectory3, infile, "ClassDirectory3");
	classDirectory3.parse(VersionUnknown);

	PIPELINE::MAX::CScene scene(&SceneClassRegistry, &dllDirectory, &classDirectory3);
	serializeStorageContainer(&scene, infile, "Scene");

	if (DebugParser)
	{
		// Not parsing the scene for this function.
		scene.parse(VersionUnknown);
		scene.clean();
	}

	/*
	PIPELINE::MAX::CStorageContainer dllDirectory;
	serializeStorageContainer(&dllDirectory, infile, "DllDirectory");

	PIPELINE::MAX::CStorageContainer classDirectory3;
	serializeStorageContainer(&classDirectory3, infile, "ClassDirectory3");

	PIPELINE::MAX::CStorageContainer scene;
	serializeStorageContainer(&scene, infile, "Scene");
	*/

	if (DisplayStream)
	{
		videoPostQueue.toString(std::cout);
		config.toString(std::cout);
		classData.toString(std::cout);
		dllDirectory.toString(std::cout);
		classDirectory3.toString(std::cout);
		scene.toString(std::cout);
	}

	if (DebugParser)
	{
		// Not parsing the scene for this function.
		scene.build(VersionUnknown);
		scene.disown();
	}

	g_object_unref(infile);

	bool pathsChanged = !WritePathChangesOnly;
	if (ReplacePaths)
	{
		pathsChanged = pathsChanged || fixChunks(&classData);
		pathsChanged = pathsChanged || fixChunks(&scene);
	}
	
	if (ReplaceMapExt)
	{
		// Parse the scene
		scene.parse(VersionUnknown);
		NLMISC::CClassId editMeshClassId = NLMISC::CClassId(0x00000050, 0x00000000);
		NLMISC::CClassId editableMeshClassId = NLMISC::CClassId(0xe44f10b3, 0x00000000);
		// from: (0x2ec82081, 0x045a6271)
		NLMISC::CClassId fromClassId = NLMISC::CClassId(0x2ec82081, 0x045a6271);
		// Find all object space modifier derived containing map extender
		for (CStorageContainer::TStorageObjectConstIt it = scene.container()->chunks().begin(), end = scene.container()->chunks().end(); it != end; ++it)
		{
			if (it->first == 0x2032) // Derived Object
			{
				// nldebug("Found derived object");
				CReferenceMaker *derivedObject = dynamic_cast<CReferenceMaker *>(it->second);
				nlassert(derivedObject);

				// Find map extender in the modifier stack
				uint mapExtenderIndex = 0;
				CReferenceMaker *mapExtender = NULL;
				for (uint i = 0; i < derivedObject->nbReferences(); ++i)
				{
					if (derivedObject->getReference(i) && derivedObject->getReference(i)->classDesc()->classId() == fromClassId)
					{
						nldebug("Found map extender at '%i' / '%i'!", i, derivedObject->nbReferences());
						mapExtenderIndex = i;
						mapExtender = derivedObject->getReference(i);

						bool deleteDerivedGeom = false;
						CStorageContainer *derivedData = dynamic_cast<CStorageContainer *>(derivedObject->findStorageObject(0x2500, mapExtenderIndex));
						nlassert(derivedData);
						CStorageContainer *derivedGeom = dynamic_cast<CStorageContainer *>(derivedData->findStorageObject(0x2512));
						if (!derivedGeom)
						{
							CStorageRaw *derivedGeomRaw = dynamic_cast<CStorageRaw *>(derivedData->findStorageObject(0x2512));
							if (derivedGeomRaw)
							{
								nlwarning("Derived geometry raw instead of as container");
								NLMISC::CMemStream memGeom;
								derivedGeomRaw->serial(memGeom);
								uint size = memGeom.getPos();
								memGeom.invert();
								derivedGeom = new CStorageContainer();
								deleteDerivedGeom = true;
								derivedGeom->serial(memGeom, size);
							}
							else
							{
								derivedData->toString(std::cout);
								nlwarning("derived geometry missing!!!");
								if (HaltOnIssue)
								{
									std::string x;
									std::cin >> x;
								}
								continue;
							}
						}
						CStorageRaw *derivedVertices = dynamic_cast<CStorageRaw *>(derivedGeom->findStorageObject(0x03e9));
						if (!derivedVertices)
						{
							derivedGeom->toString(std::cout);
							nlwarning("derived vertices missing!!!");
							if (HaltOnIssue)
							{
								std::string x;
								std::cin >> x;
							}
							continue;
						}
						CStorageRaw *derivedIndices = dynamic_cast<CStorageRaw *>(derivedGeom->findStorageObject(0x03eb));
						if (!derivedIndices)
						{
							derivedGeom->toString(std::cout);
							nlwarning("derived indices missing!!!");
							if (HaltOnIssue)
							{
								std::string x;
								std::cin >> x;
							}
							continue;
						}

						CStorageContainer::TStorageObjectContainer &mapChunks = const_cast<CStorageContainer::TStorageObjectContainer &>(mapExtender->chunks());

						// CStorageContainer::TStorageObjectWithId

						nlassert(derivedVertices->Value.size() % 12 == 0);
						nlassert(derivedIndices->Value.size() % 12 == 0);
						uint32 nbVertices = derivedVertices->Value.size() / 12;
						uint32 nbFaces = derivedIndices->Value.size() / 12;
						nldebug("Vertices: %i", nbVertices);
						nldebug("Faces: %i", nbFaces);

						// Write vertex chunks
						CStorageValue<uint32> *chunkVertCount = new CStorageValue<uint32>();
						chunkVertCount->Value = nbVertices;
						mapChunks.push_back(CStorageContainer::TStorageObjectWithId(0x0100, chunkVertCount));

						CStorageRaw *chunkVerts = new CStorageRaw();
						NLMISC::CMemStream memVerts;
						derivedVertices->serial(memVerts);
						chunkVerts->setSize(memVerts.getPos());
						memVerts.invert();
						chunkVerts->serial(memVerts);
						mapChunks.push_back(CStorageContainer::TStorageObjectWithId(0x0110, chunkVerts));

						// Write face chunks
						CStorageValue<uint32> *chunkFaceCount = new CStorageValue<uint32>();
						chunkFaceCount->Value = nbFaces;
						mapChunks.push_back(CStorageContainer::TStorageObjectWithId(0x0230, chunkFaceCount));

						CStorageRaw *chunkFaces = new CStorageRaw();
						NLMISC::CMemStream memFaces;
						derivedIndices->serial(memFaces);
						chunkFaces->setSize(memFaces.getPos());
						memFaces.invert();
						chunkFaces->serial(memFaces);
						mapChunks.push_back(CStorageContainer::TStorageObjectWithId(0x0240, chunkFaces));

						// Write geom points
						CStorageValue<uint32> *chunkGeomPointCount = new CStorageValue<uint32>();
						chunkGeomPointCount->Value = nbVertices; // derived vertices?
						mapChunks.push_back(CStorageContainer::TStorageObjectWithId(0x0320, chunkGeomPointCount));

						CStorageRaw *chunkGeomPoints = new CStorageRaw();
						NLMISC::CMemStream memGeomPoints;
						derivedVertices->serial(memGeomPoints); // derived vertices?
						chunkGeomPoints->setSize(memGeomPoints.getPos());
						memGeomPoints.invert();
						chunkGeomPoints->serial(memGeomPoints);
						mapChunks.push_back(CStorageContainer::TStorageObjectWithId(0x0330, chunkGeomPoints));

						if (deleteDerivedGeom)
							delete derivedGeom;

						// /*break;*/
						nldebug("Converted!");
					}
				}

				if (mapExtender)
				{
					uint editMeshIndex;
					CReferenceMaker *editMesh = NULL;
					for (uint i = 0; i < derivedObject->nbReferences(); ++i)
					{
						if (derivedObject->getReference(i) && derivedObject->getReference(i)->classDesc()->classId() == editMeshClassId)
						{
							nldebug("Found edit mesh at '%i' / '%i'!", i, derivedObject->nbReferences());
							editMeshIndex = i;
							editMesh = derivedObject->getReference(i);

							// Ensure no selection

							CStorageContainer::TStorageObjectContainer &meshChunks = const_cast<CStorageContainer::TStorageObjectContainer &>(editMesh->chunks());

							// Ensure no selection
							for (CStorageContainer::TStorageObjectIterator it = meshChunks.begin(), end = meshChunks.end(); it != end; )
							{
								CStorageContainer::TStorageObjectIterator next = it; ++next;
								switch (it->first)
								{
									case 0x2800:
										nldebug("Override selection");
										{
											delete it->second;
											CStorageValue<uint16> *v = new CStorageValue<uint16>();
											v->Value = 0;
											it->second = v;
										}
										break;
								}
								it = next;
							}

							// /*break;*/
						}
					}

					uint editableMeshIndex;
					CReferenceMaker *editableMesh = NULL;
					for (uint i = 0; i < derivedObject->nbReferences(); ++i)
					{
						if (derivedObject->getReference(i) && derivedObject->getReference(i)->classDesc()->classId() == editableMeshClassId)
						{
							nldebug("Found editable mesh at '%i' / '%i'!", i, derivedObject->nbReferences());
							editableMeshIndex = i;
							editableMesh = derivedObject->getReference(i);

							CStorageContainer::TStorageObjectContainer &meshChunks = const_cast<CStorageContainer::TStorageObjectContainer &>(editableMesh->chunks());

							// Ensure no selection
							for (CStorageContainer::TStorageObjectIterator it = meshChunks.begin(), end = meshChunks.end(); it != end; )
							{
								CStorageContainer::TStorageObjectIterator next = it; ++next;
								switch (it->first)
								{
									case 0x2845:
									case 0x2846:
									case 0x2847:
									case 0x2849:
									case 0x2850:
									case 0x3001:
									case 0x3003:
									case 0x3004:
										nldebug("Erase selection");
										meshChunks.erase(it);
										break;
									case 0x4038:
										nldebug("Override selection");
										{
											delete it->second;
											CStorageValue<uint32> *v = new CStorageValue<uint32>();
											v->Value = 0;
											it->second = v;
										}
										break;
								}
								it = next;
							}

							// /*break;*/
						}
					}

					if (editableMesh && editableMeshIndex + 1 != derivedObject->nbReferences())
					{
						nlwarning("Editable mesh not at bottom of the stack, this is unpossible!");
						std::string x;
						std::cin >> x;
						continue;
					}
/*
					if (editableMesh == NULL || editMesh == NULL)
					{
						derivedObject->toString(std::cout);
						nlwarning("editable mesh or edit mesh not found");
						std::string x;
						std::cin >> x;
						continue;
					}

					//
						//derivedObject->toString(std::cout);
						//editableMesh->toString(std::cout);

					// derivedObject -> 0x2500 -> 0x2512 -> 0x03e9 / 0x03eb
					// editableMesh -> 0x08fe (geom) -> 0x0916 / 0x0918

					CStorageContainer *editableMeshGeometry = dynamic_cast<CStorageContainer *>(editableMesh->findStorageObject(0x08fe));
					// nlassert(editableMeshGeometry);
					if (!editableMeshGeometry)
					{
						nlwarning("broken");
						std::string x;
						std::cin >> x;
						continue;
					}
					CStorageRaw *editableVertices = dynamic_cast<CStorageRaw *>(editableMeshGeometry->findStorageObject(0x0916));
					nlassert(editableVertices);
					CStorageRaw *editableIndices = dynamic_cast<CStorageRaw *>(editableMeshGeometry->findStorageObject(0x0918));
					nlassert(editableIndices);

	//return new CStorageArraySizePre<NLMISC::CVector>();
					if (derivedIndices->Value.size() != editableIndices->Value.size())
					{
						editableMesh->toString(std::cout);
						derivedObject->toString(std::cout);
						nlwarning("size mismatch");
						std::string x;
						std::cin >> x;
						continue;
					}

					editableVertices->Value.clear();
					editableIndices->Value.clear();

					nlassert(derivedVertices->Value.size() % 12 == 0);
					uint32 nbVertices = derivedVertices->Value.size() / 12;
					NLMISC::CMemStream memVertices;
					memVertices.serial(nbVertices);
					derivedVertices->serial(memVertices);
					memVertices.invert();
					editableVertices->setSize(derivedVertices->Value.size() + 4);
					editableVertices->serial(memVertices);

					NLMISC::CMemStream memIndices;
					derivedIndices->serial(memIndices);
					memIndices.invert();
					editableIndices->setSize(derivedIndices->Value.size());
					editableIndices->serial(memIndices);
					nldebug("ok!");*/
				}
			}
		}
		// to: (0x02df2e3a, 0x72ba4e1f)
		NLMISC::CClassId toClassId = NLMISC::CClassId(0x02df2e3a, 0x72ba4e1f); // uvw unwrap
		// NLMISC::CClassId toClassId = NLMISC::CClassId(0x5c5b50f7,0x60397ca1); // converttomesh :|
		// NLMISC::CClassId toClassId = NLMISC::CClassId(0x31f9c666, 0x03b4a577); // uvw mapping add
		// dllname to: uvwunwrap.dlm
		for (CStorageContainer::TStorageObjectConstIt it = classDirectory3.chunks().begin(), end = classDirectory3.chunks().end(); it != end; ++it)
		{
			CClassEntry *classEntry = dynamic_cast<CClassEntry *>(it->second);
			if (classEntry)
			{
				//nldebug("class entry %s", classEntry->classId().toString().c_str());
				if (classEntry->classId() == fromClassId)
				{
					nldebug("Found class id to replace!");
					classEntry->overrideClassId(toClassId);
					CDllEntry *dllEntry = const_cast<CDllEntry *>(dllDirectory.get(classEntry->dllIndex()));
					dllEntry->overrideDllFilename(ucstring("uvwunwrap.dlm"));
				}
			}
			//else nldebug("not class entry");
		}
		// Disown the scene
		scene.disown();
	}

	dllDirectory.disown();
	classDirectory3.disown();
	if (WriteModified && (ReplaceMapExt || pathsChanged))
	{
		const char *outpath = (WriteDummy ? "testdummy.max" : path.c_str());
		GsfOutput  *output;
		GsfOutfile *outfile;

		output = gsf_output_stdio_new(outpath, &err);
		if (err) { nlerror("GSF Failed to create %s", outpath); return; }
		outfile = gsf_outfile_msole_new(output);
		g_object_unref(G_OBJECT(output));

		serializeStorageContainer(&videoPostQueue, outfile, "VideoPostQueue");
		serializeStorageContainer(&scene, outfile, "Scene");
		serializeStorageContainer(&dllDirectory, outfile, "DllDirectory");
		serializeStorageContainer(&config, outfile, "Config");
		serializeStorageContainer(&classDirectory3, outfile, "ClassDirectory3");
		serializeStorageContainer(&classData, outfile, "ClassData");
		serializeRaw(summaryInformation, outfile, "\05SummaryInformation");
		serializeRaw(documentSummaryInformation, outfile, "\05DocumentSummaryInformation");

		if (!gsf_outfile_msole_set_class_id((GsfOutfileMSOle *)outfile, classId))
			nlerror("GSF Cannot write class id");

		gsf_output_close(GSF_OUTPUT(outfile));
		g_object_unref(G_OBJECT(outfile));

		if (WriteDummy)
		{
			nlinfo("Dummy written, press key for next");
			std::string x;
			std::cin >> x;
		}
	}

	g_object_unref(metadata);
}

// int __stdcall WinMain(void *, void *, void *, int)
int main(int argc, char **argv)
{
	// Initialise gsf
	printf("Pipeline Max Rewrite Assets\n");
	char const *me = (argv[0] ? argv[0] : "pipeline_max_rewrite_assets");
	g_set_prgname(me);
	gsf_init();

	// Register all plugin classes
	CBuiltin::registerClasses(&SceneClassRegistry);
	CUpdate1::registerClasses(&SceneClassRegistry);
	CEPoly::registerClasses(&SceneClassRegistry);

	//handleFile("/srv/work/database/interfaces/anims_max/cp_fy_hof_species.max");
	runInitialize();
	runHandler();
	//handleFile(nativeDatabasePath("w:\\database\\landscape\\ligo\\desert\\max\\zonematerial-converted-brandon.max")); // overrideFF
	//handleFile(nativeDatabasePath("w:\\database\\landscape\\ligo\\desert\\max\\zonematerial-converted-154_dz.max")); // overrideFF
	//handleFile(nativeDatabasePath("w:\\database\\landscape\\ligo\\lacustre\\max\\zonematerial-converted-village_a.max"));
	//handleFile(nativeDatabasePath("w:\\database\\stuff\\generique\\agents\\accessories\\ge_mission_reward_karavan_bigshield.max"));
	//handleFile(nativeDatabasePath("w:\\database\\stuff\\generique\\agents\\accessories\\ge_acc_pick_o.max"));
	//handleFile(nativeDatabasePath("w:\\database\\stuff\\generique\\agents\\accessories\\ge_zo_wea_trib_masse1m.max"));
	//handleFile(nativeDatabasePath("w:\\database\\stuff\\generique\\agents\\accessories\\ge_fy_wea_trib_grand_bouclier.max"));
	//handleFile(nativeDatabasePath("w:\\database\\stuff\\generique\\agents\\accessories\\ge_mission_entrepot.max"));
	//handleFile(nativeDatabasePath("w:\\database\\stuff\\generique\\agents\\accessories\\mesh_wip\\all_trib_weapons.max"));
	//handleFile(nativeDatabasePath("w:\\database\\stuff\\fyros\\decors\\constructions\\fy_cn_smokehouse.max"));
	//handleFile("/srv/work/database/landscape/ligo/jungle/pipeline_max/zonematerial-foret-ruine_boss.max");
	//handleFile("/srv/work/database/stuff/fyros/agents/actors/male/animation/anims_non_utilisees/fy_hom_assis_boire_verre.max");
	//handleFile("/home/kaetemi/3dsMax/scenes/test_clear_add_uvw.max");
	//runScanner();

	for (std::set<std::string>::iterator it = MissingFiles.begin(), end = MissingFiles.end(); it != end; ++it)
		nlinfo("Missing: '%s'", (*it).c_str());

	gsf_shutdown();

	return 0;
}

