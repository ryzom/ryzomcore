// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef WORLD_EDITOR_MISC_H
#define WORLD_EDITOR_MISC_H

// Project includes

// NeL includes
#include <nel/misc/vector.h>
#include <nel/ligo/primitive.h>
#include <nel/ligo/primitive_class.h>

// STL includes
#include <string>
#include <vector>

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

// Load *.worldedit file and return list primitives and landscapes.
bool loadWorldEditFile(const std::string &fileName, WorldEditList &worldEditList);

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
