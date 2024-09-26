// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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



#ifndef NL_TANGENT_SPACE_BUILD_H
#define NL_TANGENT_SPACE_BUILD_H

#include "nel/misc/types_nl.h"
#include "nel/3d/mesh.h"

namespace NL3D
{

class CVertexBuffer;
class CIndexBuffer;

/** This add tangent space information to a vertex buffer. The result is an added 3d texture coordinate
  * that contains the T vector of the tangent space basis (T is oriented toward increasing s coordinates). The binormal can be computed in a vertex program
  * by doing B = N ^ T. The input mesh build must provide a normal component.
  * Moreover it must have at least one texture coordinate applied (this is needed to build coherent tangent space basis)
  * \return true if the conversion was possible
  */


bool	BuildTangentSpace(CMesh::CMeshBuild &outMeshBuild, const CMesh::CMeshBuild &inMeshBuild);




} // NL3D

#endif
