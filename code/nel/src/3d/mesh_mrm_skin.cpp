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

#include "std3d.h"

#include "nel/misc/bsphere.h"
#include "nel/misc/fast_mem.h"
#include "nel/misc/system_info.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/mrm_builder.h"
#include "nel/3d/mrm_parameters.h"
#include "nel/3d/mesh_mrm_instance.h"
#include "nel/3d/scene.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/stripifier.h"
#include "nel/3d/matrix_3x4.h"
#include "nel/3d/raw_skin.h"


using namespace NLMISC;
using namespace std;


namespace NL3D
{



// ***************************************************************************
// ***************************************************************************
// Simple (slow) skinning version with position only.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CMeshMRMGeom::applySkin(CLod &lod, const CSkeletonModel *skeleton)
{
	nlassert(_Skinned);
	if(_SkinWeights.size()==0)
		return;

	// get vertexPtr.
	//===========================
	CVertexBufferReadWrite vba;
	_VBufferFinal.lock (vba);
	uint8		*destVertexPtr= (uint8*)vba.getVertexCoordPointer();
	uint		flags= _VBufferFinal.getVertexFormat();
	sint32		vertexSize= _VBufferFinal.getVertexSize();
	// must have XYZ.
	nlassert(flags & CVertexBuffer::PositionFlag);


	// compute src array.
	CMesh::CSkinWeight	*srcSkinPtr;
	CVector				*srcVertexPtr;
	srcSkinPtr= &_SkinWeights[0];
	srcVertexPtr= &_OriginalSkinVertices[0];



	// Compute useful Matrix for this lod.
	//===========================
	// Those arrays map the array of bones in skeleton.
	static	vector<CMatrix3x4>			boneMat3x4;
	computeBoneMatrixes3x4(boneMat3x4, lod.MatrixInfluences, skeleton);


	// apply skinning.
	//===========================
	// assert, code below is written especially for 4 per vertex.
	nlassert(NL3D_MESH_SKINNING_MAX_MATRIX==4);
	for(uint i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
	{
		uint		nInf= (uint)lod.InfluencedVertices[i].size();
		if( nInf==0 )
			continue;
		uint32		*infPtr= &(lod.InfluencedVertices[i][0]);

		// apply the skin to the vertices
		switch(i)
		{
		//=========
		case 0:
			// Special case for Vertices influenced by one matrix. Just copy result of mul.
			//  for all InfluencedVertices only.
			for(;nInf>0;nInf--, infPtr++)
			{
				uint	index= *infPtr;
				CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
				CVector				*srcVertex= srcVertexPtr + index;
				uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
				CVector				*dstVertex= (CVector*)(dstVertexVB);


				// Vertex.
				boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, *dstVertex);
			}
			break;

		//=========
		case 1:
			//  for all InfluencedVertices only.
			for(;nInf>0;nInf--, infPtr++)
			{
				uint	index= *infPtr;
				CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
				CVector				*srcVertex= srcVertexPtr + index;
				uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
				CVector				*dstVertex= (CVector*)(dstVertexVB);


				// Vertex.
				boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			}
			break;

		//=========
		case 2:
			//  for all InfluencedVertices only.
			for(;nInf>0;nInf--, infPtr++)
			{
				uint	index= *infPtr;
				CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
				CVector				*srcVertex= srcVertexPtr + index;
				uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
				CVector				*dstVertex= (CVector*)(dstVertexVB);


				// Vertex.
				boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
			}
			break;

		//=========
		case 3:
			//  for all InfluencedVertices only.
			for(;nInf>0;nInf--, infPtr++)
			{
				uint	index= *infPtr;
				CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
				CVector				*srcVertex= srcVertexPtr + index;
				uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
				CVector				*dstVertex= (CVector*)(dstVertexVB);


				// Vertex.
				boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[3] ].mulAddPoint( *srcVertex, srcSkin->Weights[3], *dstVertex);
			}
			break;

		}
	}
}


// ***************************************************************************
// ***************************************************************************
// Old school Template skinning: SSE or not.
// ***************************************************************************
// ***************************************************************************


// RawSkin Cache constants
//===============
// The number of byte to process per block
const	uint	NL_BlockByteL1= 4096;

// Number of vertices per block to process with 1 matrix.
uint	CMeshMRMGeom::NumCacheVertexNormal1= NL_BlockByteL1 / sizeof(CRawVertexNormalSkin1);
// Number of vertices per block to process with 2 matrix.
uint	CMeshMRMGeom::NumCacheVertexNormal2= NL_BlockByteL1 / sizeof(CRawVertexNormalSkin2);
// Number of vertices per block to process with 3 matrix.
uint	CMeshMRMGeom::NumCacheVertexNormal3= NL_BlockByteL1 / sizeof(CRawVertexNormalSkin3);
// Number of vertices per block to process with 4 matrix.
uint	CMeshMRMGeom::NumCacheVertexNormal4= NL_BlockByteL1 / sizeof(CRawVertexNormalSkin4);


/* Old School template: include the same file with define switching,
	Was used before to reuse same code for and without SSE.
	Useless now because SSE removed, but keep it for possible future work on it.
*/
#define ADD_MESH_MRM_SKIN_TEMPLATE
#include "mesh_mrm_skin_template.cpp"



// ***************************************************************************
// ***************************************************************************
// Misc.
// ***************************************************************************
// ***************************************************************************


} // NL3D

