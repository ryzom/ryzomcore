// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

// Qt includes

namespace WorldEditor
{

// Generate unique identificator
uint32 getUniqueId();

// Get root primitive
NLLIGO::IPrimitive *getRootPrimitive(NLLIGO::IPrimitive *primitive);

// Init a primitive parameters
void initPrimitiveParameters(const NLLIGO::CPrimitiveClass &primClass, NLLIGO::IPrimitive &primitive,
							 const std::vector<NLLIGO::CPrimitiveClass::CInitParameters> &initParameters);

NLLIGO::IPrimitive *createPrimitive(const char *className, const char *primName,
									const NLMISC::CVector &initPos, float deltaPos,
									const std::vector<NLLIGO::CPrimitiveClass::CInitParameters> &initParameters,
									NLLIGO::IPrimitive *parent);

// Remove the primitive and don't delete it.
//void takeAtPrimitive(NLLIGO::IPrimitive *primitive);

void deletePrimitive(NLLIGO::IPrimitive *primitive);

} /* namespace WorldEditor */

#endif // WORLD_EDITOR_MISC_H
