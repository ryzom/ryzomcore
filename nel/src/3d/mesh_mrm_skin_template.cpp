/**
 * File not compiled. Included from mesh_mrm_skin.cpp. It is a "old school" template.
 */

// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

// ace: before including this, #define this define to use it
//      the goal is to be able to compile every .cpp file with no
//      special case (GNU/Linux needs)
#ifdef ADD_MESH_MRM_SKIN_TEMPLATE

// ***************************************************************************
// ***************************************************************************
// "Templates" for VertexSkinning with any input matrix type.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
static void	applyArraySkinNormalT(uint numMatrixes, uint32 *infPtr, CMesh::CSkinWeight *srcSkinPtr,
	CVector *srcVertexPtr, CVector *srcNormalPtr, uint normalOff,
	uint8 *destVertexPtr, vector<CMatrix3x4> &boneMat3x4, uint vertexSize, uint nInf)
{
	/* Prefetch all vertex/normal before, it is to be faster.
	*/
#ifdef NL_HAS_SSE2
	{
		uint	nInfTmp= nInf;
		uint32	*infTmpPtr= infPtr;
		for(;nInfTmp>0;nInfTmp--, infTmpPtr++)
		{
			uint	index= *infTmpPtr;
			CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
			CVector				*srcVertex= srcVertexPtr + index;
			CVector				*srcNormal= srcNormalPtr + index;

			_mm_prefetch((const char *)(void *)srcSkin, _MM_HINT_T1);
			_mm_prefetch((const char *)(void *)srcVertex, _MM_HINT_T1);
			_mm_prefetch((const char *)(void *)srcNormal, _MM_HINT_T1);
		}
	}
#elif defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
	{
		uint	nInfTmp= nInf;
		uint32	*infTmpPtr= infPtr;
		for(;nInfTmp>0;nInfTmp--, infTmpPtr++)
		{
			uint	index= *infTmpPtr;
			CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
			CVector				*srcVertex= srcVertexPtr + index;
			CVector				*srcNormal= srcNormalPtr + index;

			__asm
			{
				mov eax, srcSkin
				mov ebx, srcVertex
				mov ecx, srcNormal
				mov edx, [eax]
				mov edx, [ebx]
				mov edx, [ecx]
			}

		}
	}
#endif

	// Process vertices.
	switch(numMatrixes)
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
			CVector				*srcNormal= srcNormalPtr + index;
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, *dstVertex);
			// Normal.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, *dstNormal);
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
			CVector				*srcNormal= srcNormalPtr + index;
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			// Normal.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
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
			CVector				*srcNormal= srcNormalPtr + index;
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
			// Normal.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcNormal, srcSkin->Weights[2], *dstNormal);
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
			CVector				*srcNormal= srcNormalPtr + index;
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[3] ].mulAddPoint( *srcVertex, srcSkin->Weights[3], *dstVertex);
			// Normal.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcNormal, srcSkin->Weights[2], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[3] ].mulAddVector( *srcNormal, srcSkin->Weights[3], *dstNormal);
		}
		break;

	}
}



// ***************************************************************************
static void	applyArraySkinTangentSpaceT(uint numMatrixes, uint32 *infPtr, CMesh::CSkinWeight *srcSkinPtr,
	CVector *srcVertexPtr, CVector *srcNormalPtr, CVector *tgSpacePtr, uint normalOff, uint tgSpaceOff,
	uint8 *destVertexPtr, vector<CMatrix3x4> &boneMat3x4, uint vertexSize, uint nInf)
{
	/* Prefetch all vertex/normal/tgSpace before, it is faster.
	*/
#ifdef NL_HAS_SSE2
	{
		uint	nInfTmp= nInf;
		uint32	*infTmpPtr= infPtr;
		for(;nInfTmp>0;nInfTmp--, infTmpPtr++)
		{
			uint	index= *infTmpPtr;
			CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
			CVector				*srcVertex= srcVertexPtr + index;
			CVector				*srcNormal= srcNormalPtr + index;
			CVector				*srcTgSpace= tgSpacePtr + index;

			_mm_prefetch((const char *)(void *)srcSkin, _MM_HINT_T1);
			_mm_prefetch((const char *)(void *)srcVertex, _MM_HINT_T1);
			_mm_prefetch((const char *)(void *)srcNormal, _MM_HINT_T1);
			_mm_prefetch((const char *)(void *)srcTgSpace, _MM_HINT_T1);
		}
	}
#elif defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
	{
		uint	nInfTmp= nInf;
		uint32	*infTmpPtr= infPtr;
		for(;nInfTmp>0;nInfTmp--, infTmpPtr++)
		{
			uint	index= *infTmpPtr;
			CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
			CVector				*srcVertex= srcVertexPtr + index;
			CVector				*srcNormal= srcNormalPtr + index;
			CVector				*srcTgSpace= tgSpacePtr + index;

			__asm
			{
				mov eax, srcSkin
				mov ebx, srcVertex
				mov ecx, srcNormal
				mov esi, srcTgSpace
				mov edx, [eax]
				mov edx, [ebx]
				mov edx, [ecx]
				mov edx, [esi]
			}

		}
	}
#endif

	// Process vertices.
	switch(numMatrixes)
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
			CVector				*srcNormal= srcNormalPtr + index;
			CVector				*srcTgSpace= tgSpacePtr + index;
			//
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);
			CVector				*dstTgSpace= (CVector*)(dstVertexVB + tgSpaceOff);



			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, *dstVertex);
			// Normal.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, *dstNormal);
			// Tg space
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcTgSpace, *dstTgSpace);

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
			CVector				*srcNormal= srcNormalPtr + index;
			CVector				*srcTgSpace= tgSpacePtr + index;
			//
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);
			CVector				*dstTgSpace= (CVector*)(dstVertexVB + tgSpaceOff);

			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			// Normal.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
			// Tg space
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcTgSpace, srcSkin->Weights[0], *dstTgSpace);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcTgSpace, srcSkin->Weights[1], *dstTgSpace);
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
			CVector				*srcNormal= srcNormalPtr + index;
			CVector				*srcTgSpace= tgSpacePtr + index;
			//
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);
			CVector				*dstTgSpace= (CVector*)(dstVertexVB + tgSpaceOff);

			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
			// Normal.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcNormal, srcSkin->Weights[2], *dstNormal);
			// Tg space
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcTgSpace, srcSkin->Weights[0], *dstTgSpace);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcTgSpace, srcSkin->Weights[1], *dstTgSpace);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcTgSpace, srcSkin->Weights[2], *dstTgSpace);
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
			CVector				*srcNormal= srcNormalPtr + index;
			CVector				*srcTgSpace= tgSpacePtr + index;
			//
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);
			CVector				*dstTgSpace= (CVector*)(dstVertexVB + tgSpaceOff);

			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[3] ].mulAddPoint( *srcVertex, srcSkin->Weights[3], *dstVertex);
			// Normal.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcNormal, srcSkin->Weights[2], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[3] ].mulAddVector( *srcNormal, srcSkin->Weights[3], *dstNormal);
			// Tg space
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcTgSpace, srcSkin->Weights[0], *dstTgSpace);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcTgSpace, srcSkin->Weights[1], *dstTgSpace);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcTgSpace, srcSkin->Weights[2], *dstTgSpace);
			boneMat3x4[ srcSkin->MatrixId[3] ].mulAddVector( *srcTgSpace, srcSkin->Weights[3], *dstTgSpace);
		}
		break;

	}

}



// ***************************************************************************
// ***************************************************************************
// ApplySkin methods.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CMeshMRMGeom::applySkinWithNormal(CLod &lod, const CSkeletonModel *skeleton)
{
	nlassert(_Skinned);
	if(_SkinWeights.empty())
		return;

	// get vertexPtr / normalOff.
	//===========================
	CVertexBufferReadWrite vba;
	_VBufferFinal.lock (vba);
	uint8		*destVertexPtr= (uint8*)vba.getVertexCoordPointer();
	uint		flags= _VBufferFinal.getVertexFormat();
	sint32		vertexSize= _VBufferFinal.getVertexSize();
	// must have XYZ and Normal.
	nlassert((flags & CVertexBuffer::PositionFlag)
			 && (flags & CVertexBuffer::NormalFlag)
			);


	// Compute offset of each component of the VB.
	sint32		normalOff;
	normalOff= _VBufferFinal.getNormalOff();


	// compute src array.
	CMesh::CSkinWeight	*srcSkinPtr;
	CVector				*srcVertexPtr;
	CVector				*srcNormalPtr= NULL;
	srcSkinPtr= &_SkinWeights[0];
	srcVertexPtr= &_OriginalSkinVertices[0];
	srcNormalPtr= &(_OriginalSkinNormals[0]);



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

		// TestYoyo
		/*extern	uint TESTYOYO_NumStdSkinVertices;
		TESTYOYO_NumStdSkinVertices+= nInf;*/

		// apply the skin to the vertices
		applyArraySkinNormalT(i, infPtr, srcSkinPtr, srcVertexPtr, srcNormalPtr,
			normalOff, destVertexPtr,
			boneMat3x4, vertexSize, nInf);
	}
}


// ***************************************************************************
void	CMeshMRMGeom::applySkinWithTangentSpace(CLod &lod, const CSkeletonModel *skeleton,
	uint tangentSpaceTexCoord)
{
	nlassert(_Skinned);
	if(_SkinWeights.empty())
		return;

	// get vertexPtr / normalOff / tangent space offset.
	//===========================
	CVertexBufferReadWrite vba;
	_VBufferFinal.lock (vba);
	uint8		*destVertexPtr= (uint8*)vba.getVertexCoordPointer();
	uint		flags= _VBufferFinal.getVertexFormat();
	sint32		vertexSize= _VBufferFinal.getVertexSize();
	// must have XYZ.
	// if there's tangent space, there also must be a normal there.
	nlassert((flags & CVertexBuffer::PositionFlag)
			 && (flags & CVertexBuffer::NormalFlag)
			);


	// Compute offset of each component of the VB.
	sint32		normalOff;
	normalOff= _VBufferFinal.getNormalOff();

	// tg space offset
	sint32		tgSpaceOff = _VBufferFinal.getTexCoordOff((uint8) tangentSpaceTexCoord);

	// compute src array.
	CMesh::CSkinWeight	*srcSkinPtr;
	CVector				*srcVertexPtr;
	CVector				*srcNormalPtr;
	CVector				*tgSpacePtr;
	//
	srcSkinPtr= &_SkinWeights[0];
	srcVertexPtr= &_OriginalSkinVertices[0];
	srcNormalPtr= &(_OriginalSkinNormals[0]);
	tgSpacePtr = &(_OriginalTGSpace[0]);



	// Compute useful Matrix for this lod.
	//===========================
	// Those arrays map the array of bones in skeleton.
	static	vector<CMatrix3x4>			boneMat3x4;
	computeBoneMatrixes3x4(boneMat3x4, lod.MatrixInfluences, skeleton);


	// apply skinning (with tangent space added)
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
		applyArraySkinTangentSpaceT(i, infPtr, srcSkinPtr, srcVertexPtr, srcNormalPtr, tgSpacePtr,
			normalOff, tgSpaceOff, destVertexPtr,
			boneMat3x4, vertexSize, nInf);
	}
}



// ***************************************************************************
// ***************************************************************************
// Raw "Vertex/Normal only" ApplySkin methods.
// ***************************************************************************
// ***************************************************************************


#define	NL3D_RAWSKIN_NORMAL_OFF		12
#define	NL3D_RAWSKIN_UV_OFF			24
#define	NL3D_RAWSKIN_VERTEX_SIZE	32


/* Speed Feature test.
	Don't use precaching for now, cause its seems to be slower on some configs (P4-2.4Ghz),
	but maybe faster on other (P3-800)
	On a P4-2.4Ghz, for 40000 vertices skinned, both no precaching and asm
	saves 27% of execution time in the applyRawSkinNormal*() loop (ie 1 ms)
*/
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
//#define	NL3D_RAWSKIN_PRECACHE
#define	NL3D_RAWSKIN_ASM
#endif


// ***************************************************************************
void		CMeshMRMGeom::applyArrayRawSkinNormal1(CRawVertexNormalSkin1 *src, uint8 *destVertexPtr,
	CMatrix3x4 *boneMat3x4, uint nInf)
{
	// must write contigously in AGP, and ASM is hardcoded...
	nlctassert(NL3D_RAWSKIN_NORMAL_OFF==12);
	nlctassert(NL3D_RAWSKIN_UV_OFF==24);

	/*extern	uint TESTYOYO_NumRawSkinVertices1;
	TESTYOYO_NumRawSkinVertices1+= nInf;
	H_AUTO( TestYoyo_RawSkin1 );*/

#ifdef	NL3D_RAWSKIN_PRECACHE
	for(;nInf>0;)
	{
		// number of vertices to process for this block.
		uint	nBlockInf= min(NumCacheVertexNormal1, nInf);
		// next block.
		nInf-= nBlockInf;

		// cache the data in L1 cache.
		CFastMem::precache(src, nBlockInf * sizeof(CRawVertexNormalSkin1));
#else
	{
		uint	nBlockInf= nInf;
#endif


#ifndef NL3D_RAWSKIN_ASM
		//  for all InfluencedVertices only.
		for(;nBlockInf>0;nBlockInf--, src++, destVertexPtr+=NL3D_RAWSKIN_VERTEX_SIZE)
		{
			CVector				*dstVertex= (CVector*)(destVertexPtr);
			CVector				*dstNormal= (CVector*)(destVertexPtr + NL3D_RAWSKIN_NORMAL_OFF);

			// For 1 matrix, can write directly to AGP (if destVertexPtr is AGP...)
			// Vertex.
			boneMat3x4[ src->MatrixId[0] ].mulSetPoint( src->Vertex.Pos, *(CVector*)(destVertexPtr) );
			// Normal.
			boneMat3x4[ src->MatrixId[0] ].mulSetVector( src->Vertex.Normal, *(CVector*)(destVertexPtr + NL3D_RAWSKIN_NORMAL_OFF) );
			// UV copy.
			*(CUV*)(destVertexPtr + NL3D_RAWSKIN_UV_OFF)= src->Vertex.UV;
		}
#else
		// ASM harcoded for 36
		nlctassert(sizeof(CRawVertexNormalSkin1)==36);

		/*  116 cycles / loop typical
			58 cycles / loop in theory (no memory problem)
		*/
		__asm
		{
			mov		ecx, nBlockInf
			mov		esi, src
			mov		edi, destVertexPtr
			mov		edx, boneMat3x4
		theLoop:
			// Vertex.
			// **** boneMat3x4[ src->MatrixId[0] ].mulSetPoint( src->Vertex.Pos, *(CVector*)(destVertexPtr) );

			// eax= matrix
			mov		eax, [esi]src.MatrixId				// uop: 0/1
			lea		eax, [eax*2+eax]
			shl		eax, 4
			add		eax, edx							// uop: 1/0

			// load x y z
			fld		[esi]src.Vertex.Pos.x					// uop: 0/1
			fld		[esi]src.Vertex.Pos.y					// uop: 0/1
			fld		[esi]src.Vertex.Pos.z					// uop: 0/1
			// vout.x= (a11*vin.x + a12*vin.y + a13*vin.z + a14);
			fld		[eax]CMatrix3x4.a11				// uop: 0/1
			fmul	st, st(3)							// uop: 1/0 (5)
			fld		[eax]CMatrix3x4.a12				// uop: 0/1
			fmul	st, st(3)							// uop: 1/0 (5)
			faddp	st(1), st							// uop: 1/0 (3)
			fld		[eax]CMatrix3x4.a13				// uop: 0/1
			fmul	st, st(2)							// uop: 1/0 (5)
			faddp	st(1), st							// uop: 1/0 (3)
			fld		[eax]CMatrix3x4.a14				// uop: 0/1
			faddp	st(1), st							// uop: 1/0 (3)
			fstp	dword ptr[edi]						// uop: 0/0/1/1
			// vout.y= (a21*vin.x + a22*vin.y + a23*vin.z + a24);
			fld		[eax]CMatrix3x4.a21
			fmul	st, st(3)
			fld		[eax]CMatrix3x4.a22
			fmul	st, st(3)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a23
			fmul	st, st(2)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a24
			faddp	st(1), st
			fstp	dword ptr[edi+4]
			// vout.z= (a31*vin.x + a32*vin.y + a33*vin.z + a34);
			fld		[eax]CMatrix3x4.a31
			fmul	st, st(3)
			fld		[eax]CMatrix3x4.a32
			fmul	st, st(3)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a33
			fmul	st, st(2)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a34
			faddp	st(1), st
			fstp	dword ptr[edi+8]
			// free x y z
			fstp	st									// uop: 1/0
			fstp	st									// uop: 1/0
			fstp	st									// uop: 1/0


			// Normal
			// **** boneMat3x4[ src->MatrixId[0] ].mulSetVector( src->Vertex.Normal, *(CVector*)(destVertexPtr + NL3D_RAWSKIN_NORMAL_OFF) );

			// load x y z
			fld		[esi]src.Vertex.Normal.x
			fld		[esi]src.Vertex.Normal.y
			fld		[esi]src.Vertex.Normal.z
			// vout.x= (a11*vin.x + a12*vin.y + a13*vin.z + a14);
			fld		[eax]CMatrix3x4.a11				// uop: 0/1
			fmul	st, st(3)							// uop: 1/0 (5)
			fld		[eax]CMatrix3x4.a12				// uop: 0/1
			fmul	st, st(3)							// uop: 1/0 (5)
			faddp	st(1), st							// uop: 1/0 (3)
			fld		[eax]CMatrix3x4.a13				// uop: 0/1
			fmul	st, st(2)							// uop: 1/0 (5)
			faddp	st(1), st							// uop: 1/0 (3)
			fstp	dword ptr[edi+12]					// uop: 0/0/1/1
			// vout.y= (a21*vin.x + a22*vin.y + a23*vin.z + a24);
			fld		[eax]CMatrix3x4.a21
			fmul	st, st(3)
			fld		[eax]CMatrix3x4.a22
			fmul	st, st(3)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a23
			fmul	st, st(2)
			faddp	st(1), st
			fstp	dword ptr[edi+16]
			// vout.z= (a31*vin.x + a32*vin.y + a33*vin.z + a34);
			fld		[eax]CMatrix3x4.a31
			fmul	st, st(3)
			fld		[eax]CMatrix3x4.a32
			fmul	st, st(3)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a33
			fmul	st, st(2)
			faddp	st(1), st
			fstp	dword ptr[edi+20]
			// free x y z
			fstp	st
			fstp	st
			fstp	st


			// UV copy.
			// **** *(CUV*)(destVertexPtr + NL3D_RAWSKIN_UV_OFF)= src->Vertex.UV;
			mov		eax, [esi]src.Vertex.UV.U					// uop: 0/1
			mov		dword ptr[edi+24], eax				// uop: 0/0/1/1
			mov		eax, [esi]src.Vertex.UV.V					// uop: 0/1
			mov		dword ptr[edi+28], eax				// uop: 0/0/1/1


			// **** next
			add		esi, 36								// uop: 1/0
			add		edi, NL3D_RAWSKIN_VERTEX_SIZE		// uop: 1/0
			dec		ecx									// uop: 1/0
			jnz		theLoop								// uop: 1/1 (p1)

			mov		nBlockInf, ecx
			mov		src, esi
			mov		destVertexPtr, edi
		}
#endif
	}


}

// ***************************************************************************
void		CMeshMRMGeom::applyArrayRawSkinNormal2(CRawVertexNormalSkin2 *src, uint8 *destVertexPtr,
	CMatrix3x4 *boneMat3x4, uint nInf)
{
	// must write contigously in AGP, and ASM is hardcoded...
	nlctassert(NL3D_RAWSKIN_NORMAL_OFF==12);
	nlctassert(NL3D_RAWSKIN_UV_OFF==24);

	/*extern	uint TESTYOYO_NumRawSkinVertices2;
	TESTYOYO_NumRawSkinVertices2+= nInf;
	H_AUTO( TestYoyo_RawSkin2 );*/

	// Since VertexPtr may be a AGP Ram, MUST NOT read into it! (mulAdd*() do it!)
	CVector	tmpVert;

#ifdef	NL3D_RAWSKIN_PRECACHE
	for(;nInf>0;)
	{
		// number of vertices to process for this block.
		uint	nBlockInf= min(NumCacheVertexNormal2, nInf);
		// next block.
		nInf-= nBlockInf;

		// cache the data in L1 cache.
		CFastMem::precache(src, nBlockInf * sizeof(CRawVertexNormalSkin2));
#else
	{
		uint	nBlockInf= nInf;
#endif


#ifndef NL3D_RAWSKIN_ASM
		//  for all InfluencedVertices only.
		for(;nBlockInf>0;nBlockInf--, src++, destVertexPtr+=NL3D_RAWSKIN_VERTEX_SIZE)
		{
			// Vertex.
			boneMat3x4[ src->MatrixId[0] ].mulSetPoint( src->Vertex.Pos, src->Weights[0], tmpVert);
			boneMat3x4[ src->MatrixId[1] ].mulAddPoint( src->Vertex.Pos, src->Weights[1], tmpVert);
			*(CVector*)(destVertexPtr)= tmpVert;
			// Normal.
			boneMat3x4[ src->MatrixId[0] ].mulSetVector( src->Vertex.Normal, src->Weights[0], tmpVert);
			boneMat3x4[ src->MatrixId[1] ].mulAddVector( src->Vertex.Normal, src->Weights[1], tmpVert);
			*(CVector*)(destVertexPtr + NL3D_RAWSKIN_NORMAL_OFF)= tmpVert;
			// UV copy.
			*(CUV*)(destVertexPtr + NL3D_RAWSKIN_UV_OFF)= src->Vertex.UV;
		}
#else
		// ASM harcoded for 48
		nlctassert(sizeof(CRawVertexNormalSkin2)==48);

		/*  154 cycles / loop typical
			124 cycles / loop in theory (no memory problem)
		*/
		__asm
		{
			mov		ecx, nBlockInf
			mov		esi, src
			mov		edi, destVertexPtr
			mov		edx, boneMat3x4
		theLoop:
			// Vertex.
			// **** boneMat3x4[ src->MatrixId[0] ].mulSetPoint( src->Vertex.Pos, *(CVector*)(destVertexPtr) );

			// eax= matrix0
			mov		eax, [esi+0]src.MatrixId			// uop: 0/1
			lea		eax, [eax*2+eax]
			shl		eax, 4
			add		eax, edx							// uop: 1/0
			// ebx= matrix1
			mov		ebx, [esi+4]src.MatrixId			// uop: 0/1
			lea		ebx, [ebx*2+ebx]
			shl		ebx, 4
			add		ebx, edx							// uop: 1/0

			// load x y z
			fld		[esi]src.Vertex.Pos.x					// uop: 0/1
			fld		[esi]src.Vertex.Pos.y					// uop: 0/1
			fld		[esi]src.Vertex.Pos.z					// uop: 0/1

			// **** vout.x= (a11*vin.x + a12*vin.y + a13*vin.z + a14);
			// 1st Matrix
			fld		[eax]CMatrix3x4.a11				// uop: 0/1
			fmul	st, st(3)							// uop: 1/0 (5)
			fld		[eax]CMatrix3x4.a12				// uop: 0/1
			fmul	st, st(3)							// uop: 1/0 (5)
			faddp	st(1), st							// uop: 1/0 (3)
			fld		[eax]CMatrix3x4.a13				// uop: 0/1
			fmul	st, st(2)							// uop: 1/0 (5)
			faddp	st(1), st							// uop: 1/0 (3)
			fld		[eax]CMatrix3x4.a14				// uop: 0/1
			faddp	st(1), st							// uop: 1/0 (3)
			// mul by scale
			fmul	[esi+0]src.Weights

			// 2nd matrix
			fld		[ebx]CMatrix3x4.a11
			fmul	st, st(4)
			fld		[ebx]CMatrix3x4.a12
			fmul	st, st(4)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a13
			fmul	st, st(3)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a14
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+4]src.Weights
			faddp	st(1), st

			// store
			fstp	dword ptr[edi]						// uop: 0/0/1/1

			// **** vout.y= (a21*vin.x + a22*vin.y + a23*vin.z + a24);
			fld		[eax]CMatrix3x4.a21
			fmul	st, st(3)
			fld		[eax]CMatrix3x4.a22
			fmul	st, st(3)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a23
			fmul	st, st(2)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a24
			faddp	st(1), st
			// mul by scale
			fmul	[esi+0]src.Weights

			// 2nd matrix
			fld		[ebx]CMatrix3x4.a21
			fmul	st, st(4)
			fld		[ebx]CMatrix3x4.a22
			fmul	st, st(4)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a23
			fmul	st, st(3)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a24
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+4]src.Weights
			faddp	st(1), st

			// store
			fstp	dword ptr[edi+4]

			// **** vout.z= (a31*vin.x + a32*vin.y + a33*vin.z + a34);
			fld		[eax]CMatrix3x4.a31
			fmul	st, st(3)
			fld		[eax]CMatrix3x4.a32
			fmul	st, st(3)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a33
			fmul	st, st(2)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a34
			faddp	st(1), st
			// mul by scale
			fmul	[esi+0]src.Weights

			// 2nd matrix
			fld		[ebx]CMatrix3x4.a31
			fmul	st, st(4)
			fld		[ebx]CMatrix3x4.a32
			fmul	st, st(4)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a33
			fmul	st, st(3)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a34
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+4]src.Weights
			faddp	st(1), st

			// store
			fstp	dword ptr[edi+8]

			// free x y z
			fstp	st									// uop: 1/0
			fstp	st									// uop: 1/0
			fstp	st									// uop: 1/0


			// Normal
			// **** boneMat3x4[ src->MatrixId[0] ].mulSetVector( src->Vertex.Normal, *(CVector*)(destVertexPtr + NL3D_RAWSKIN_NORMAL_OFF) );

			// load x y z
			fld		[esi]src.Vertex.Normal.x
			fld		[esi]src.Vertex.Normal.y
			fld		[esi]src.Vertex.Normal.z

			// **** vout.x= (a11*vin.x + a12*vin.y + a13*vin.z + a14);
			fld		[eax]CMatrix3x4.a11				// uop: 0/1
			fmul	st, st(3)							// uop: 1/0 (5)
			fld		[eax]CMatrix3x4.a12				// uop: 0/1
			fmul	st, st(3)							// uop: 1/0 (5)
			faddp	st(1), st							// uop: 1/0 (3)
			fld		[eax]CMatrix3x4.a13				// uop: 0/1
			fmul	st, st(2)							// uop: 1/0 (5)
			faddp	st(1), st							// uop: 1/0 (3)
			// mul by scale
			fmul	[esi+0]src.Weights

			// 2nd matrix
			fld		[ebx]CMatrix3x4.a11
			fmul	st, st(4)
			fld		[ebx]CMatrix3x4.a12
			fmul	st, st(4)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a13
			fmul	st, st(3)
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+4]src.Weights
			faddp	st(1), st

			// store
			fstp	dword ptr[edi+12]					// uop: 0/0/1/1

			// **** vout.y= (a21*vin.x + a22*vin.y + a23*vin.z + a24);
			fld		[eax]CMatrix3x4.a21
			fmul	st, st(3)
			fld		[eax]CMatrix3x4.a22
			fmul	st, st(3)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a23
			fmul	st, st(2)
			faddp	st(1), st
			// mul by scale
			fmul	[esi+0]src.Weights

			// 2nd matrix
			fld		[ebx]CMatrix3x4.a21
			fmul	st, st(4)
			fld		[ebx]CMatrix3x4.a22
			fmul	st, st(4)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a23
			fmul	st, st(3)
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+4]src.Weights
			faddp	st(1), st

			// store
			fstp	dword ptr[edi+16]

			// **** vout.z= (a31*vin.x + a32*vin.y + a33*vin.z + a34);
			fld		[eax]CMatrix3x4.a31
			fmul	st, st(3)
			fld		[eax]CMatrix3x4.a32
			fmul	st, st(3)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a33
			fmul	st, st(2)
			faddp	st(1), st
			// mul by scale
			fmul	[esi+0]src.Weights

			// 2nd matrix
			fld		[ebx]CMatrix3x4.a31
			fmul	st, st(4)
			fld		[ebx]CMatrix3x4.a32
			fmul	st, st(4)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a33
			fmul	st, st(3)
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+4]src.Weights
			faddp	st(1), st

			// store
			fstp	dword ptr[edi+20]

			// free x y z
			fstp	st
			fstp	st
			fstp	st


			// UV copy.
			// **** *(CUV*)(destVertexPtr + NL3D_RAWSKIN_UV_OFF)= src->Vertex.UV;
			mov		eax, [esi]src.Vertex.UV.U					// uop: 0/1
			mov		dword ptr[edi+24], eax				// uop: 0/0/1/1
			mov		eax, [esi]src.Vertex.UV.V					// uop: 0/1
			mov		dword ptr[edi+28], eax				// uop: 0/0/1/1


			// **** next
			add		esi, 48								// uop: 1/0
			add		edi, NL3D_RAWSKIN_VERTEX_SIZE		// uop: 1/0
			dec		ecx									// uop: 1/0
			jnz		theLoop								// uop: 1/1 (p1)

			mov		nBlockInf, ecx
			mov		src, esi
			mov		destVertexPtr, edi
		}
#endif
	}

}

// ***************************************************************************
void		CMeshMRMGeom::applyArrayRawSkinNormal3(CRawVertexNormalSkin3 *src, uint8 *destVertexPtr,
	CMatrix3x4 *boneMat3x4, uint nInf)
{
	// must write contigously in AGP, and ASM is hardcoded...
	nlctassert(NL3D_RAWSKIN_NORMAL_OFF==12);
	nlctassert(NL3D_RAWSKIN_UV_OFF==24);

	/*extern	uint TESTYOYO_NumRawSkinVertices3;
	TESTYOYO_NumRawSkinVertices3+= nInf;
	H_AUTO( TestYoyo_RawSkin3 );*/

	// Since VertexPtr may be a AGP Ram, MUST NOT read into it! (mulAdd*() do it!)
	CVector	tmpVert;

#ifdef	NL3D_RAWSKIN_PRECACHE
	for(;nInf>0;)
	{
		// number of vertices to process for this block.
		uint	nBlockInf= min(NumCacheVertexNormal3, nInf);
		// next block.
		nInf-= nBlockInf;

		// cache the data in L1 cache.
		CFastMem::precache(src, nBlockInf * sizeof(CRawVertexNormalSkin3));
#else
	{
		uint	nBlockInf= nInf;
#endif


#ifndef NL3D_RAWSKIN_ASM
		//  for all InfluencedVertices only.
		for(;nBlockInf>0;nBlockInf--, src++, destVertexPtr+=NL3D_RAWSKIN_VERTEX_SIZE)
		{
			// Vertex.
			boneMat3x4[ src->MatrixId[0] ].mulSetPoint( src->Vertex.Pos, src->Weights[0], tmpVert);
			boneMat3x4[ src->MatrixId[1] ].mulAddPoint( src->Vertex.Pos, src->Weights[1], tmpVert);
			boneMat3x4[ src->MatrixId[2] ].mulAddPoint( src->Vertex.Pos, src->Weights[2], tmpVert);
			*(CVector*)(destVertexPtr)= tmpVert;
			// Normal.
			boneMat3x4[ src->MatrixId[0] ].mulSetVector( src->Vertex.Normal, src->Weights[0], tmpVert);
			boneMat3x4[ src->MatrixId[1] ].mulAddVector( src->Vertex.Normal, src->Weights[1], tmpVert);
			boneMat3x4[ src->MatrixId[2] ].mulAddVector( src->Vertex.Normal, src->Weights[2], tmpVert);
			*(CVector*)(destVertexPtr + NL3D_RAWSKIN_NORMAL_OFF)= tmpVert;
			// UV copy.
			*(CUV*)(destVertexPtr + NL3D_RAWSKIN_UV_OFF)= src->Vertex.UV;
		}
#else
		// ASM harcoded for 56
		nlctassert(sizeof(CRawVertexNormalSkin3)==56);


		/*  226 cycles / loop typical
			192 cycles / loop in theory (no memory problem)
			148 optimal
		*/
		__asm
		{
			mov		ecx, nBlockInf
			mov		esi, src
			mov		edi, destVertexPtr
		theLoop:
			// Vertex.
			// **** boneMat3x4[ src->MatrixId[0] ].mulSetPoint( src->Vertex.Pos, *(CVector*)(destVertexPtr) );

			// eax= matrix0
			mov		eax, [esi+0]src.MatrixId			// uop: 0/1
			lea		eax, [eax*2+eax]
			shl		eax, 4
			add		eax, boneMat3x4						// uop: 1/0
			// ebx= matrix1
			mov		ebx, [esi+4]src.MatrixId			// uop: 0/1
			lea		ebx, [ebx*2+ebx]
			shl		ebx, 4
			add		ebx, boneMat3x4						// uop: 1/0
			// edx= matrix2
			mov		edx, [esi+8]src.MatrixId			// uop: 0/1
			lea		edx, [edx*2+edx]
			shl		edx, 4
			add		edx, boneMat3x4						// uop: 1/0

			// load x y z
			fld		[esi]src.Vertex.Pos.x					// uop: 0/1
			fld		[esi]src.Vertex.Pos.y					// uop: 0/1
			fld		[esi]src.Vertex.Pos.z					// uop: 0/1

			// **** vout.x= (a11*vin.x + a12*vin.y + a13*vin.z + a14);
			// 1st Matrix
			fld		[eax]CMatrix3x4.a11				// uop: 0/1
			fmul	st, st(3)							// uop: 1/0 (5)
			fld		[eax]CMatrix3x4.a12				// uop: 0/1
			fmul	st, st(3)							// uop: 1/0 (5)
			faddp	st(1), st							// uop: 1/0 (3)
			fld		[eax]CMatrix3x4.a13				// uop: 0/1
			fmul	st, st(2)							// uop: 1/0 (5)
			faddp	st(1), st							// uop: 1/0 (3)
			fld		[eax]CMatrix3x4.a14				// uop: 0/1
			faddp	st(1), st							// uop: 1/0 (3)
			// mul by scale
			fmul	[esi+0]src.Weights

			// 2nd matrix
			fld		[ebx]CMatrix3x4.a11
			fmul	st, st(4)
			fld		[ebx]CMatrix3x4.a12
			fmul	st, st(4)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a13
			fmul	st, st(3)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a14
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+4]src.Weights
			faddp	st(1), st

			// 3rd matrix
			fld		[edx]CMatrix3x4.a11
			fmul	st, st(4)
			fld		[edx]CMatrix3x4.a12
			fmul	st, st(4)
			faddp	st(1), st
			fld		[edx]CMatrix3x4.a13
			fmul	st, st(3)
			faddp	st(1), st
			fld		[edx]CMatrix3x4.a14
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+8]src.Weights
			faddp	st(1), st

			// store
			fstp	dword ptr[edi]						// uop: 0/0/1/1

			// **** vout.y= (a21*vin.x + a22*vin.y + a23*vin.z + a24);
			fld		[eax]CMatrix3x4.a21
			fmul	st, st(3)
			fld		[eax]CMatrix3x4.a22
			fmul	st, st(3)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a23
			fmul	st, st(2)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a24
			faddp	st(1), st
			// mul by scale
			fmul	[esi+0]src.Weights

			// 2nd matrix
			fld		[ebx]CMatrix3x4.a21
			fmul	st, st(4)
			fld		[ebx]CMatrix3x4.a22
			fmul	st, st(4)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a23
			fmul	st, st(3)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a24
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+4]src.Weights
			faddp	st(1), st

			// 3rd matrix
			fld		[edx]CMatrix3x4.a21
			fmul	st, st(4)
			fld		[edx]CMatrix3x4.a22
			fmul	st, st(4)
			faddp	st(1), st
			fld		[edx]CMatrix3x4.a23
			fmul	st, st(3)
			faddp	st(1), st
			fld		[edx]CMatrix3x4.a24
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+8]src.Weights
			faddp	st(1), st

			// store
			fstp	dword ptr[edi+4]

			// **** vout.z= (a31*vin.x + a32*vin.y + a33*vin.z + a34);
			fld		[eax]CMatrix3x4.a31
			fmul	st, st(3)
			fld		[eax]CMatrix3x4.a32
			fmul	st, st(3)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a33
			fmul	st, st(2)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a34
			faddp	st(1), st
			// mul by scale
			fmul	[esi+0]src.Weights

			// 2nd matrix
			fld		[ebx]CMatrix3x4.a31
			fmul	st, st(4)
			fld		[ebx]CMatrix3x4.a32
			fmul	st, st(4)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a33
			fmul	st, st(3)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a34
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+4]src.Weights
			faddp	st(1), st

			// 3rd matrix
			fld		[edx]CMatrix3x4.a31
			fmul	st, st(4)
			fld		[edx]CMatrix3x4.a32
			fmul	st, st(4)
			faddp	st(1), st
			fld		[edx]CMatrix3x4.a33
			fmul	st, st(3)
			faddp	st(1), st
			fld		[edx]CMatrix3x4.a34
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+8]src.Weights
			faddp	st(1), st

			// store
			fstp	dword ptr[edi+8]

			// free x y z
			fstp	st									// uop: 1/0
			fstp	st									// uop: 1/0
			fstp	st									// uop: 1/0


			// Normal
			// **** boneMat3x4[ src->MatrixId[0] ].mulSetVector( src->Vertex.Normal, *(CVector*)(destVertexPtr + NL3D_RAWSKIN_NORMAL_OFF) );

			// load x y z
			fld		[esi]src.Vertex.Normal.x
			fld		[esi]src.Vertex.Normal.y
			fld		[esi]src.Vertex.Normal.z
			// **** vout.x= (a11*vin.x + a12*vin.y + a13*vin.z + a14);
			fld		[eax]CMatrix3x4.a11				// uop: 0/1
			fmul	st, st(3)							// uop: 1/0 (5)
			fld		[eax]CMatrix3x4.a12				// uop: 0/1
			fmul	st, st(3)							// uop: 1/0 (5)
			faddp	st(1), st							// uop: 1/0 (3)
			fld		[eax]CMatrix3x4.a13				// uop: 0/1
			fmul	st, st(2)							// uop: 1/0 (5)
			faddp	st(1), st							// uop: 1/0 (3)
			// mul by scale
			fmul	[esi+0]src.Weights

			// 2nd matrix
			fld		[ebx]CMatrix3x4.a11
			fmul	st, st(4)
			fld		[ebx]CMatrix3x4.a12
			fmul	st, st(4)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a13
			fmul	st, st(3)
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+4]src.Weights
			faddp	st(1), st

			// 3rd matrix
			fld		[edx]CMatrix3x4.a11
			fmul	st, st(4)
			fld		[edx]CMatrix3x4.a12
			fmul	st, st(4)
			faddp	st(1), st
			fld		[edx]CMatrix3x4.a13
			fmul	st, st(3)
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+8]src.Weights
			faddp	st(1), st

			// store
			fstp	dword ptr[edi+12]					// uop: 0/0/1/1

			// **** vout.y= (a21*vin.x + a22*vin.y + a23*vin.z + a24);
			fld		[eax]CMatrix3x4.a21
			fmul	st, st(3)
			fld		[eax]CMatrix3x4.a22
			fmul	st, st(3)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a23
			fmul	st, st(2)
			faddp	st(1), st
			// mul by scale
			fmul	[esi+0]src.Weights

			// 2nd matrix
			fld		[ebx]CMatrix3x4.a21
			fmul	st, st(4)
			fld		[ebx]CMatrix3x4.a22
			fmul	st, st(4)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a23
			fmul	st, st(3)
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+4]src.Weights
			faddp	st(1), st

			// 3rd matrix
			fld		[edx]CMatrix3x4.a21
			fmul	st, st(4)
			fld		[edx]CMatrix3x4.a22
			fmul	st, st(4)
			faddp	st(1), st
			fld		[edx]CMatrix3x4.a23
			fmul	st, st(3)
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+8]src.Weights
			faddp	st(1), st

			// store
			fstp	dword ptr[edi+16]

			// **** vout.z= (a31*vin.x + a32*vin.y + a33*vin.z + a34);
			fld		[eax]CMatrix3x4.a31
			fmul	st, st(3)
			fld		[eax]CMatrix3x4.a32
			fmul	st, st(3)
			faddp	st(1), st
			fld		[eax]CMatrix3x4.a33
			fmul	st, st(2)
			faddp	st(1), st
			// mul by scale
			fmul	[esi+0]src.Weights

			// 2nd matrix
			fld		[ebx]CMatrix3x4.a31
			fmul	st, st(4)
			fld		[ebx]CMatrix3x4.a32
			fmul	st, st(4)
			faddp	st(1), st
			fld		[ebx]CMatrix3x4.a33
			fmul	st, st(3)
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+4]src.Weights
			faddp	st(1), st

			// 3rd matrix
			fld		[edx]CMatrix3x4.a31
			fmul	st, st(4)
			fld		[edx]CMatrix3x4.a32
			fmul	st, st(4)
			faddp	st(1), st
			fld		[edx]CMatrix3x4.a33
			fmul	st, st(3)
			faddp	st(1), st
			// mul by scale, and append
			fmul	[esi+8]src.Weights
			faddp	st(1), st

			// store
			fstp	dword ptr[edi+20]

			// free x y z
			fstp	st
			fstp	st
			fstp	st


			// UV copy.
			// **** *(CUV*)(destVertexPtr + NL3D_RAWSKIN_UV_OFF)= src->Vertex.UV;
			mov		eax, [esi]src.Vertex.UV.U					// uop: 0/1
			mov		dword ptr[edi+24], eax				// uop: 0/0/1/1
			mov		eax, [esi]src.Vertex.UV.V					// uop: 0/1
			mov		dword ptr[edi+28], eax				// uop: 0/0/1/1


			// **** next
			add		esi, 56								// uop: 1/0
			add		edi, NL3D_RAWSKIN_VERTEX_SIZE		// uop: 1/0
			dec		ecx									// uop: 1/0
			jnz		theLoop								// uop: 1/1 (p1)

			mov		nBlockInf, ecx
			mov		src, esi
			mov		destVertexPtr, edi
		}
#endif

	}
}

// ***************************************************************************
void		CMeshMRMGeom::applyArrayRawSkinNormal4(CRawVertexNormalSkin4 *src, uint8 *destVertexPtr,
	CMatrix3x4 *boneMat3x4, uint nInf)
{
	// must write contigously in AGP, and ASM is hardcoded...
	nlctassert(NL3D_RAWSKIN_NORMAL_OFF==12);
	nlctassert(NL3D_RAWSKIN_UV_OFF==24);

	/*extern	uint TESTYOYO_NumRawSkinVertices4;
	TESTYOYO_NumRawSkinVertices4+= nInf;
	H_AUTO( TestYoyo_RawSkin4 );*/

	// Since VertexPtr may be a AGP Ram, MUST NOT read into it! (mulAdd*() do it!)
	CVector	tmpVert;

#ifdef	NL3D_RAWSKIN_PRECACHE
	for(;nInf>0;)
	{
		// number of vertices to process for this block.
		uint	nBlockInf= min(NumCacheVertexNormal4, nInf);
		// next block.
		nInf-= nBlockInf;

		// cache the data in L1 cache.
		CFastMem::precache(src, nBlockInf * sizeof(CRawVertexNormalSkin4));
#else
	{
		uint	nBlockInf= nInf;
#endif

		//  for all InfluencedVertices only.
		for(;nBlockInf>0;nBlockInf--, src++, destVertexPtr+=NL3D_RAWSKIN_VERTEX_SIZE)
		{
			// Vertex.
			boneMat3x4[ src->MatrixId[0] ].mulSetPoint( src->Vertex.Pos, src->Weights[0], tmpVert);
			boneMat3x4[ src->MatrixId[1] ].mulAddPoint( src->Vertex.Pos, src->Weights[1], tmpVert);
			boneMat3x4[ src->MatrixId[2] ].mulAddPoint( src->Vertex.Pos, src->Weights[2], tmpVert);
			boneMat3x4[ src->MatrixId[3] ].mulAddPoint( src->Vertex.Pos, src->Weights[3], tmpVert);
			*(CVector*)(destVertexPtr)= tmpVert;
			// Normal.
			boneMat3x4[ src->MatrixId[0] ].mulSetVector( src->Vertex.Normal, src->Weights[0], tmpVert);
			boneMat3x4[ src->MatrixId[1] ].mulAddVector( src->Vertex.Normal, src->Weights[1], tmpVert);
			boneMat3x4[ src->MatrixId[2] ].mulAddVector( src->Vertex.Normal, src->Weights[2], tmpVert);
			boneMat3x4[ src->MatrixId[3] ].mulAddVector( src->Vertex.Normal, src->Weights[3], tmpVert);
			*(CVector*)(destVertexPtr + NL3D_RAWSKIN_NORMAL_OFF)= tmpVert;
			// UV copy.
			*(CUV*)(destVertexPtr + NL3D_RAWSKIN_UV_OFF)= src->Vertex.UV;
		}

		// NB: ASM not done for 4 vertices, cause very rare and negligeable ...
	}
}


// ***************************************************************************
void	CMeshMRMGeom::applyRawSkinWithNormal(CLod &lod, CRawSkinNormalCache &rawSkinLod, const CSkeletonModel *skeleton, uint8 *vbHard, float alphaLod)
{
	nlassert(_Skinned);
	if(_SkinWeights.empty())
		return;

	// Some assert
	//===========================
	// must have XYZ, Normal and UV only
	nlassert( _VBufferFinal.getVertexFormat() == (CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag | CVertexBuffer::TexCoord0Flag) );
	nlassert( _VBufferFinal.getValueType(CVertexBuffer::TexCoord0) == CVertexBuffer::Float2 );
	nlassert( _VBufferFinal.getVertexSize() ==NL3D_RAWSKIN_VERTEX_SIZE);

	// HardCoded for normalOff==12 (see applyArrayRawSkinNormal*)
	nlassert( _VBufferFinal.getNormalOff()==NL3D_RAWSKIN_NORMAL_OFF );
	nlassert( _VBufferFinal.getTexCoordOff()==NL3D_RAWSKIN_UV_OFF );
	// assert, code below is written especially for 4 per vertex.
	nlassert( NL3D_MESH_SKINNING_MAX_MATRIX==4 );


	// Compute useful Matrix for this lod.
	//===========================
	// Those arrays map the array of bones in skeleton.
	static	vector<CMatrix3x4>			boneMat3x4;
	computeBoneMatrixes3x4(boneMat3x4, lod.MatrixInfluences, skeleton);


	// TestYoyo
	/*extern	uint TESTYOYO_NumRawSkinVertices;
	TESTYOYO_NumRawSkinVertices+= rawSkinLod.Vertices1.size();
	TESTYOYO_NumRawSkinVertices+= rawSkinLod.Vertices2.size();
	TESTYOYO_NumRawSkinVertices+= rawSkinLod.Vertices3.size();
	TESTYOYO_NumRawSkinVertices+= rawSkinLod.Vertices4.size();*/


	uint	nInf;

	// Manage "SoftVertices"
	if(rawSkinLod.TotalSoftVertices)
	{
		// apply skinning into Temp RAM for vertices that are Src of Geomorph
		//===========================
		static	vector<uint8>	tempSkin;
		uint	tempVbSize= rawSkinLod.TotalSoftVertices*NL3D_RAWSKIN_VERTEX_SIZE;
		if(tempSkin.size() < tempVbSize)
			tempSkin.resize(tempVbSize);
		uint8		*destVertexPtr= &tempSkin[0];

		// 1 Matrix
		nInf= rawSkinLod.SoftVertices[0];
		if(nInf>0)
		{
			applyArrayRawSkinNormal1(&rawSkinLod.Vertices1[0], destVertexPtr, &boneMat3x4[0], nInf);
			destVertexPtr+= nInf * NL3D_RAWSKIN_VERTEX_SIZE;
		}
		// 2 Matrix
		nInf= rawSkinLod.SoftVertices[1];
		if(nInf>0)
		{
			applyArrayRawSkinNormal2(&rawSkinLod.Vertices2[0], destVertexPtr, &boneMat3x4[0], nInf);
			destVertexPtr+= nInf * NL3D_RAWSKIN_VERTEX_SIZE;
		}
		// 3 Matrix
		nInf= rawSkinLod.SoftVertices[2];
		if(nInf>0)
		{
			applyArrayRawSkinNormal3(&rawSkinLod.Vertices3[0], destVertexPtr, &boneMat3x4[0], nInf);
			destVertexPtr+= nInf * NL3D_RAWSKIN_VERTEX_SIZE;
		}
		// 4 Matrix
		nInf= rawSkinLod.SoftVertices[3];
		if(nInf>0)
		{
			applyArrayRawSkinNormal4(&rawSkinLod.Vertices4[0], destVertexPtr, &boneMat3x4[0], nInf);
			destVertexPtr+= nInf * NL3D_RAWSKIN_VERTEX_SIZE;
		}

		// Fast Copy this into AGP Ram. NB: done before Geomorphs, because ensure some precaching this way!!
		//===========================
		// Skin geomorphs.
		uint8	*vbHardStart= vbHard + rawSkinLod.Geomorphs.size()*NL3D_RAWSKIN_VERTEX_SIZE;

		// fast copy
		CFastMem::memcpy(vbHardStart, &tempSkin[0], tempVbSize);

		// Geomorphs directly into AGP Ram
		//===========================
		clamp(alphaLod, 0.f, 1.f);
		float		a= alphaLod;
		float		a1= 1 - alphaLod;

		// Fast Geomorph
		applyGeomorphPosNormalUV0(rawSkinLod.Geomorphs, &tempSkin[0], vbHard, NL3D_RAWSKIN_VERTEX_SIZE, a, a1);
	}

	// Manage HardVertices
	if(rawSkinLod.TotalHardVertices)
	{
		// apply skinning directly into AGP RAM for vertices that are not Src of Geomorph
		//===========================
		uint	startId;

		// Skip Geomorphs and SoftVertices.
		uint8		*destVertexPtr= vbHard + (rawSkinLod.Geomorphs.size()+rawSkinLod.TotalSoftVertices)*NL3D_RAWSKIN_VERTEX_SIZE;

		// 1 Matrix
		nInf= rawSkinLod.HardVertices[0];
		startId= rawSkinLod.SoftVertices[0];
		if(nInf>0)
		{
			applyArrayRawSkinNormal1(&rawSkinLod.Vertices1[startId], destVertexPtr, &boneMat3x4[0], nInf);
			destVertexPtr+= nInf * NL3D_RAWSKIN_VERTEX_SIZE;
		}
		// 2 Matrix
		nInf= rawSkinLod.HardVertices[1];
		startId= rawSkinLod.SoftVertices[1];
		if(nInf>0)
		{
			applyArrayRawSkinNormal2(&rawSkinLod.Vertices2[startId], destVertexPtr, &boneMat3x4[0], nInf);
			destVertexPtr+= nInf * NL3D_RAWSKIN_VERTEX_SIZE;
		}
		// 3 Matrix
		nInf= rawSkinLod.HardVertices[2];
		startId= rawSkinLod.SoftVertices[2];
		if(nInf>0)
		{
			applyArrayRawSkinNormal3(&rawSkinLod.Vertices3[startId], destVertexPtr, &boneMat3x4[0], nInf);
			destVertexPtr+= nInf * NL3D_RAWSKIN_VERTEX_SIZE;
		}
		// 4 Matrix
		nInf= rawSkinLod.HardVertices[3];
		startId= rawSkinLod.SoftVertices[3];
		if(nInf>0)
		{
			applyArrayRawSkinNormal4(&rawSkinLod.Vertices4[startId], destVertexPtr, &boneMat3x4[0], nInf);
			destVertexPtr+= nInf * NL3D_RAWSKIN_VERTEX_SIZE;
		}
	}
}

#endif // ADD_MESH_MRM_SKIN_TEMPLATE
