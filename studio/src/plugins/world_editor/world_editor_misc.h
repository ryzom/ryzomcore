// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#ifndef WORLD_EDITOR_MISC_H
#define WORLD_EDITOR_MISC_H

// Project includes

// NeL includes
#include <nel/misc/vector.h>
#include <nel/ligo/primitive.h>
#include <nel/ligo/primitive_class.h>

// STL includes
#include <map>
#include <set>
#include <string>
#include <vector>

namespace NLLIGO
{
class CZoneRegion;
}

#define WORLD_EDITOR_FILE_VERSION 2
#define WORLD_EDITOR_DATABASE_SIZE 100

namespace WorldEditor
{
namespace Utils
{
enum ItemType
{
	DataDirectoryType = 0,
	ContextType,
	LandscapeType,
	PrimitiveType
};

typedef std::pair<ItemType, std::string> WorldEditItem;
typedef std::vector<WorldEditItem> WorldEditList;

// Generate unique identificator
uint32 getUniqueId();

std::string getLastError();

// Load *.worldedit file and return list primitives and landscapes.
bool loadWorldEditFile(const std::string &fileName, WorldEditList &worldEditList);

// The absolute name of a land or primitive file named in a *.worldedit, from the folder of that *.worldedit,
// without "." or ".." components
std::string worldEditFilePath(const std::string &fileName, const std::string &worldEditFolder);

// Whether two file paths name the same file: on Windows without regard to case, as its file systems do
bool sameFilePath(const std::string &a, const std::string &b);

// Orders file paths, two paths that sameFilePath takes for the same file being equivalent
struct FilePathLess
{
	bool operator()(const std::string &a, const std::string &b) const;
};

// A set of file paths, matched as sameFilePath matches them
typedef std::set<std::string, FilePathLess> FilePathSet;

// The content of a file; false when it cannot be read
bool readFile(const std::string &fileName, std::string &content);

// Writes a file through a temporary file, so that a failed write leaves it as it was; false with the
// reason, which names the file, in error
bool writeFile(const std::string &fileName, const std::string &content, std::string &error);

// The content of a primitive file, as NLLIGO::saveXmlPrimitiveFile writes it
std::string primitivesFileContent(NLLIGO::CPrimitives &primitives, const std::string &fileName);

// The content of a land file, as LandscapeEditor::ZoneRegionObject::save writes it
std::string landFileContent(NLLIGO::CZoneRegion &zoneRegion);

// The size and SHA-1 of a file content, to compare contents without keeping them
std::string fileContentKey(const std::string &content);

// What Save knows of a file of the world: the keys of the content the editor had for it and of the
// content on disk, when it was loaded or last written
struct FileBaseline
{
	std::string EditorKey;
	std::string DiskKey;
};

// The baselines of the files of the world, by file name
typedef std::map<std::string, FileBaseline, FilePathLess> FileBaselines;

// The baseline of a file, NULL when the editor neither loaded nor wrote it
const FileBaseline *findFileBaseline(const FileBaselines &baselines, const std::string &fileName);

// Records that the editor and the disk both have this content for a file
void setFileBaseline(FileBaselines &baselines, const std::string &fileName, const std::string &content);

enum FileAction
{
	// the editor did not change the file since it was loaded or written, what is on disk stays
	FileUnchanged,
	// the file on disk already has the content
	FileUpToDate,
	// the editor changed the file, which is on disk as it was loaded or written, or is not there and
	// was not expected
	FileToWrite,
	// the editor changed the file, which also changed on disk, or is there unexpected, or cannot be read
	FileConflict
};

// What saving content to a file needs, from its baseline, which is NULL when there is none
FileAction fileAction(const std::string &fileName, const std::string &content, const FileBaseline *baseline);

// Get root primitive
NLLIGO::IPrimitive *getRootPrimitive(NLLIGO::IPrimitive *primitive);

// Init a primitive parameters
void initPrimitiveParameters(const NLLIGO::CPrimitiveClass &primClass, NLLIGO::IPrimitive &primitive,
							 const std::vector<NLLIGO::CPrimitiveClass::CInitParameters> &initParameters);

NLLIGO::IPrimitive *createPrimitive(const char *className, const char *primName,
									const NLMISC::CVector &initPos, float deltaPos,
									const std::vector<NLLIGO::CPrimitiveClass::CInitParameters> &initParameters,
									NLLIGO::IPrimitive *parent);

void deletePrimitive(NLLIGO::IPrimitive *primitive);

bool updateDefaultValues(NLLIGO::IPrimitive *primitive);

bool recursiveUpdateDefaultValues(NLLIGO::IPrimitive *primitive);

NLLIGO::CLigoConfig	*ligoConfig();

} /* namespace Utils */
} /* namespace WorldEditor */

#endif // WORLD_EDITOR_MISC_H
