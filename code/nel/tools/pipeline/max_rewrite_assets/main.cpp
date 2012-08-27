
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

#include "../max/storage_stream.h"
#include "../max/storage_object.h"
#include "../max/dll_directory.h"
#include "../max/class_directory_3.h"
#include "../max/class_data.h"
#include "../max/config.h"
#include "../max/scene.h"
#include "../max/scene_class_registry.h"

// Testing
#include "../max/builtin/builtin.h"
#include "../max/update1/update1.h"
#include "../max/epoly/epoly.h"

#include "../max/builtin/storage/app_data.h"
#include "../max/builtin/storage/geom_buffers.h"
#include "../max/builtin/scene_impl.h"
#include "../max/builtin/i_node.h"
#include "../max/update1/editable_mesh.h"
#include "../max/epoly/editable_poly.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::BUILTIN::STORAGE;
using namespace PIPELINE::MAX::UPDATE1;
using namespace PIPELINE::MAX::EPOLY;

CSceneClassRegistry SceneClassRegistry;

// Never enable this
bool DebugParser = true;

bool DisplayStream = false;
bool WriteModified = false;
bool WriteDummy = true;

const char *DatabaseDirectory = "w:\\database\\";
const char *LinuxDatabaseDirectory = "/srv/work/database/";
bool RunningLinux = true;

//const char *SrcDirectoryRecursive = "w:\\database\\interfaces\\";
const char *SrcDirectoryRecursive = "w:\\database\\";

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

				nlwarning("Path file does not exist: '%s' ('%s')", path.c_str(), stdPath.c_str());
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

	KnownFileCache[NLMISC::CFile::getFilename(filePath)] = standardizePath(filePath, true);
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

	doDirectoryInitialize(std::string(SrcDirectoryRecursive));
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

	doDirectoryScanner(SrcDirectoryRecursive);

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

	doDirectoryHandler(SrcDirectoryRecursive);

	for (std::set<std::string>::iterator it = MissingFiles.begin(), end = MissingFiles.end(); it != end; ++it)
		nlinfo("Missing: '%s'", (*it).c_str());
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
		scene.build(VersionUnknown);
		scene.disown();
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

	g_object_unref(infile);

	dllDirectory.disown();
	classDirectory3.disown();
	if (WriteModified)
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
	//runInitialize();
	runHandler();
	//runScanner();

	gsf_shutdown();

	return 0;
}

