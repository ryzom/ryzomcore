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

#ifndef NL_RAW_SKINNED_H
#define NL_RAW_SKINNED_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/uv.h"
#include "nel/misc/object_vector.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mrm_mesh.h"


namespace NL3D
{


using	NLMISC::CVector;
using	NLMISC::CUV;


/// Vertices influenced by 1 matrix only.
class	CRawVertexNormalSkinned1
{
public:
	// The id of the matrix to use.
	uint32		MatrixId[1];
	CVector		Vertex;
	CVector		Normal;
	CUV			UV;
};

/// Vertices influenced by 2 matrix only.
class	CRawVertexNormalSkinned2
{
public:
	// The id of the matrix to use.
	uint32		MatrixId[2];
	float		Weights[2];
	CVector		Vertex;
	CVector		Normal;
	CUV			UV;
};

/// Vertices influenced by 3 matrix only.
class	CRawVertexNormalSkinned3
{
public:
	uint32		MatrixId[3];
	float		Weights[3];
	CVector		Vertex;
	CVector		Normal;
	CUV			UV;
};

/// Vertices influenced by 4 matrix only.
class	CRawVertexNormalSkinned4
{
public:
	uint32		MatrixId[4];
	float		Weights[4];
	CVector		Vertex;
	CVector		Normal;
	CUV			UV;
};

/// The array per lod.
class	CRawSkinnedNormalCache
{
public:
	// The vertices influenced by 1 matrix.
	NLMISC::CObjectVector<CRawVertexNormalSkinned1, false>	Vertices1;
	// The vertices influenced by 2 matrix.
	NLMISC::CObjectVector<CRawVertexNormalSkinned2, false>	Vertices2;
	// The vertices influenced by 3 matrix.
	NLMISC::CObjectVector<CRawVertexNormalSkinned3, false>	Vertices3;
	// The vertices influenced by 4 matrix.
	NLMISC::CObjectVector<CRawVertexNormalSkinned4, false>	Vertices4;

	// For Each array, set the max number of vertices to copy in VBSoft (not VBHard directly)
	uint32							SoftVertices[4];
	uint32							HardVertices[4];
	// Total Of SoftVertices
	uint32							TotalSoftVertices;
	uint32							TotalHardVertices;

	// The RawSkin Geomorphs.
	std::vector<CMRMWedgeGeom>		Geomorphs;
	// The Raw Primitives.
	std::vector<CIndexBuffer>	RdrPass;

	/// What RawSkin lod this cache represent. -1 if NULL
	sint					LodId;
	/// To see if same Data than in the CMeshMRMSkinnedGeom
	uint					MeshDataId;

	// free up the memory
	void				clearArrays();
};



} // NL3D


#endif // NL_RAW_SKINNED_H

/* End of raw_skinned.h */
