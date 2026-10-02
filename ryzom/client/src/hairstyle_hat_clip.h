// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2018  Winch Gate Property Limited
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

#ifndef RY_HAIRSTYLE_HAT_CLIP_H
#define RY_HAIRSTYLE_HAT_CLIP_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_instance.h"

namespace NL3D
{
	class CScene;
	class USkeleton;
}

/** Builds a private (not shape-bank-shared) instance of hairstyleShapeName, with the
 *  part of its geometry that falls inside hatShapeName's volume cut away at the exact
 *  intersection, bound to skeleton exactly like a normal skinned hairstyle.
 *
 *  Returns an empty UInstance (isEmpty() == true) if either shape can't be loaded or
 *  isn't a plain CMesh -- the caller must fall back to the normal shared-shape equip
 *  path in that case.
 */
NL3D::UInstance buildClippedHairstyleInstance(NL3D::CScene &scene, NL3D::USkeleton &skeleton,
	const std::string &hairstyleShapeName, const std::string &hatShapeName);

#endif // RY_HAIRSTYLE_HAT_CLIP_H
