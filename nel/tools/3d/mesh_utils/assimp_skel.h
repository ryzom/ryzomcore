// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2026  Winch Gate Property Limited
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

#ifndef NL_ASSIMP_SKEL_H
#define NL_ASSIMP_SKEL_H
#include <nel/misc/types_nl.h>

struct CMeshUtilsContext;

// Walk the skeleton root's subtree (auto-detected as the first "Bip01"-named descendant of the
// scene root if scene_meta doesn't declare a TBoneRoot node) and emit a .skel file using the
// same convertMatrix + root-reset conventions as pipeline_max_export_skel and NeL's Max exporter.
// Called from exportScene() after flagging bones.
void exportSkels(CMeshUtilsContext &context);

#endif /* NL_ASSIMP_SKEL_H */
