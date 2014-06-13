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
#include "nel/misc/system_info.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/fast_mem.h"
#include "nel/3d/mesh_mrm_skinned.h"
#include "nel/3d/mrm_builder.h"
#include "nel/3d/mrm_parameters.h"
#include "nel/3d/mesh_mrm_skinned_instance.h"
#include "nel/3d/scene.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/stripifier.h"
#include "nel/3d/mesh_blender.h"
#include "nel/3d/render_trav.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/raw_skinned.h"
#include "nel/3d/shifted_triangle_cache.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/matrix_3x4.h"


using namespace NLMISC;
using namespace std;


namespace NL3D
{


H_AUTO_DECL( NL3D_MeshMRMGeom_RenderShadow )

// ***************************************************************************
// ***************************************************************************
// CMeshMRMSkinnedGeom::CLod
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CMeshMRMSkinnedGeom::CLod::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base vdrsion.
	*/

	f.serialVersion(0);
	uint	i;

	f.serial(NWedges);
	f.serialCont(RdrPass);
	f.serialCont(Geomorphs);
	f.serialCont(MatrixInfluences);

	// Serial array of InfluencedVertices. NB: code written so far for NL3D_MESH_SKINNING_MAX_MATRIX==4 only.
	nlassert(NL3D_MESH_SKINNING_MAX_MATRIX==4);
	for(i= 0; i<NL3D_MESH_SKINNING_MAX_MATRIX; i++)
	{
		f.serialCont(InfluencedVertices[i]);
	}
}


// ***************************************************************************
void		CMeshMRMSkinnedGeom::CLod::optimizeTriangleOrder()
{
	CStripifier		stripifier;

	// for all rdrpass
	for(uint  rp=0; rp<RdrPass.size(); rp++ )
	{
		// stripify list of triangles of this pass.
		CIndexBuffer block;
		getRdrPassPrimitiveBlock(rp, block);

		stripifier.optimizeTriangles(block, block);

		buildPrimitiveBlock(rp, block);
	}
}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::CLod::getRdrPassPrimitiveBlock (uint renderPass, CIndexBuffer &block) const
{
	const CRdrPass &rd = RdrPass[renderPass];
	const uint count = rd.getNumTriangle();
	block.setFormat(NL_SKINNED_MESH_MRM_INDEX_FORMAT);
	block.setNumIndexes (count*3);

	CIndexBufferReadWrite ibaWrite;
	block.lock (ibaWrite);

	uint i;
	for (i=0; i<count; i++)
	{
		ibaWrite.setTri (i*3, rd.PBlock[i*3+0], rd.PBlock[i*3+1], rd.PBlock[i*3+2]);
	}
}

// ***************************************************************************

void	CMeshMRMSkinnedGeom::CLod::buildPrimitiveBlock(uint renderPass, const CIndexBuffer &block)
{
	CRdrPass &rd = RdrPass[renderPass];
	const uint count = block.getNumIndexes()/3;
	rd.PBlock.resize (3*count);
	uint i;
	CIndexBufferRead ibaRead;
	block.lock (ibaRead);
	if (ibaRead.getFormat() == CIndexBuffer::Indices32)
	{
		const uint32 *triPtr = (const uint32 *) ibaRead.getPtr ();
		for (i=0; i<count; i++)
		{
			rd.PBlock[i*3+0] = (uint16) (triPtr[3*i+0]);
			rd.PBlock[i*3+1] = (uint16) (triPtr[3*i+1]);
			rd.PBlock[i*3+2] = (uint16) (triPtr[3*i+2]);
		}
	}
	else
	{
		const uint16 *triPtr = (const uint16 *) ibaRead.getPtr ();
		for (i=0; i<count; i++)
		{
			rd.PBlock[i*3+0] = (triPtr[3*i+0]);
			rd.PBlock[i*3+1] = (triPtr[3*i+1]);
			rd.PBlock[i*3+2] = (triPtr[3*i+2]);
		}
	}
}

// ***************************************************************************
// ***************************************************************************
// CMeshMRMSkinnedGeom.
// ***************************************************************************
// ***************************************************************************




// ***************************************************************************
static	NLMISC::CAABBoxExt	makeBBox(const std::vector<CVector>	&Vertices)
{
	NLMISC::CAABBox		ret;
	nlassert(Vertices.size());
	ret.setCenter(Vertices[0]);
	for(sint i=0;i<(sint)Vertices.size();i++)
	{
		ret.extend(Vertices[i]);
	}

	return ret;
}


// ***************************************************************************
CMeshMRMSkinnedGeom::CMeshMRMSkinnedGeom()
{
	_BoneIdComputed = false;
	_BoneIdExtended = false;
	_PreciseClipping= false;
	_MeshDataId= 0;
	_SupportShadowSkinGrouping= false;
}


// ***************************************************************************
CMeshMRMSkinnedGeom::~CMeshMRMSkinnedGeom()
{
}


// ***************************************************************************
void			CMeshMRMSkinnedGeom::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	// check input.
	if(distanceFinest<0)	return;
	if(distanceMiddle<=distanceFinest)	return;
	if(distanceCoarsest<=distanceMiddle)	return;

	// Change.
	_LevelDetail.DistanceFinest= distanceFinest;
	_LevelDetail.DistanceMiddle= distanceMiddle;
	_LevelDetail.DistanceCoarsest= distanceCoarsest;

	// compile
	_LevelDetail.compileDistanceSetup();
}

// ***************************************************************************

void			CMeshMRMSkinnedGeom::build(CMesh::CMeshBuild &m,
									uint numMaxMaterial, const CMRMParameters &params)
{
	// Empty geometry?
	if(m.Vertices.size()==0 || m.Faces.size()==0)
	{
		_VBufferFinal.clear();
		_Lods.clear();
		_BBox.setCenter(CVector::Null);
		_BBox.setSize(CVector::Null);
		return;
	}
	nlassert(numMaxMaterial>0);


	/// 0. First, make bbox.
	//======================
	// NB: this is equivalent as building BBox from MRM VBuffer, because CMRMBuilder create new vertices
	// which are just interpolation of original vertices.
	_BBox= makeBBox(m.Vertices);


	/// 1. Launch the MRM build process.
	//================================================
	CMRMBuilder			mrmBuilder;
	CMeshBuildMRM		meshBuildMRM;
	std::vector<CMesh::CMeshBuild*> bsList;

	mrmBuilder.compileMRM(m, bsList, params, meshBuildMRM, numMaxMaterial);
	nlassert (meshBuildMRM.Skinned);

	// Then build the packed vertex buffer
	//================================================
	_VBufferFinal.build (meshBuildMRM.VBuffer, meshBuildMRM.SkinWeights);
	_Lods= meshBuildMRM.Lods;

	// Compute degradation control.
	//================================================
	_LevelDetail.DistanceFinest= meshBuildMRM.DistanceFinest;
	_LevelDetail.DistanceMiddle= meshBuildMRM.DistanceMiddle;
	_LevelDetail.DistanceCoarsest= meshBuildMRM.DistanceCoarsest;
	nlassert(_LevelDetail.DistanceFinest>=0);
	nlassert(_LevelDetail.DistanceMiddle > _LevelDetail.DistanceFinest);
	nlassert(_LevelDetail.DistanceCoarsest > _LevelDetail.DistanceMiddle);
	// Compute OODistDelta and DistancePow
	_LevelDetail.compileDistanceSetup();


	// For load balancing.
	//================================================
	// compute Max Face Used
	_LevelDetail.MaxFaceUsed= 0;
	_LevelDetail.MinFaceUsed= 0;
	// Count of primitive block
	if(_Lods.size()>0)
	{
		uint	pb;
		// Compute MinFaces.
		CLod	&firstLod= _Lods[0];
		for (pb=0; pb<firstLod.RdrPass.size(); pb++)
		{
			CRdrPass &pass= firstLod.RdrPass[pb];
			// Sum tri
			_LevelDetail.MinFaceUsed+= pass.getNumTriangle ();
		}
		// Compute MaxFaces.
		CLod	&lastLod= _Lods[_Lods.size()-1];
		for (pb=0; pb<lastLod.RdrPass.size(); pb++)
		{
			CRdrPass &pass= lastLod.RdrPass[pb];
			// Sum tri
			_LevelDetail.MaxFaceUsed+= pass.getNumTriangle ();
		}
	}


	// For skinning.
	//================================================

	// Inform that the mesh data has changed
	dirtMeshDataId();


	// For AGP SKinning optim, and for Render optim
	//================================================
	uint i;
	for(i=0;i<_Lods.size();i++)
	{
		// sort triangles for better cache use.
		_Lods[i].optimizeTriangleOrder();
	}

	// No Blend Shapes
	//================================================
	nlassert (meshBuildMRM.BlendShapes.size() == 0);

	// Compact bone id and build a bone id names
	//================================================

	// Remap
	std::map<uint, uint> remap;

	// Current bone
	uint currentBone = 0;

	// Reserve memory
	_BonesName.reserve (m.BonesNames.size());

	// For each vertices
	uint vert;
	CPackedVertexBuffer::CPackedVertex *vertices = _VBufferFinal.getPackedVertices();
	for (vert=0; vert<_VBufferFinal.getNumVertices(); vert++)
	{
		// Found one ?
		bool found=false;

		// For each weight
		uint weight;
		for (weight=0; weight<NL3D_MESH_MRM_SKINNED_MAX_MATRIX; weight++)
		{
			// Active ?
			if ((vertices[vert].Weights[weight]>0)||(weight==0))
			{
				// Look for it
				std::map<uint, uint>::iterator ite = remap.find (vertices[vert].Matrices[weight]);

				// Find ?
				if (ite == remap.end())
				{
					// Insert it
					remap.insert (std::map<uint, uint>::value_type (vertices[vert].Matrices[weight], currentBone));

					// Check the id
					nlassert (vertices[vert].Matrices[weight]<m.BonesNames.size());

					// Set the bone name
					_BonesName.push_back (m.BonesNames[vertices[vert].Matrices[weight]]);

					// Set the local bone id
					vertices[vert].Matrices[weight] = currentBone++;
				}
				else
				{
					// Set the local bone id
					vertices[vert].Matrices[weight] = ite->second;
				}

				// Found one
				found = true;
			}
		}

		// Found one ?
		nlassert (found);
	}

	// Remap the vertex influence by lods
	uint lod;
	for (lod=0; lod<_Lods.size(); lod++)
	{
		// For each matrix used
		uint matrix;
		for (matrix=0; matrix<_Lods[lod].MatrixInfluences.size(); matrix++)
		{
			// Remap
			std::map<uint, uint>::iterator ite = remap.find (_Lods[lod].MatrixInfluences[matrix]);

			// Not find ?
			if (ite == remap.end())
			{
				// Remove it
				_Lods[lod].MatrixInfluences.erase (_Lods[lod].MatrixInfluences.begin()+matrix);
				matrix--;
				continue;
			}

			// Remap
			_Lods[lod].MatrixInfluences[matrix] = ite->second;
		}
	}

	// Misc.
	//===================
	// Some runtime not serialized compilation
	compileRunTime();

}

// ***************************************************************************
void	CMeshMRMSkinnedGeom::applyMaterialRemap(const std::vector<sint> &remap)
{
	for(uint lod=0;lod<getNbLod();lod++)
	{
		for(uint rp=0;rp<getNbRdrPass(lod);rp++)
		{
			// remap
			uint32	&matId= _Lods[lod].RdrPass[rp].MaterialId;
			nlassert(remap[matId]>=0);
			matId= remap[matId];
		}
	}
}

// ***************************************************************************
void	CMeshMRMSkinnedGeom::applyGeomorph(std::vector<CMRMWedgeGeom>  &geoms, float alphaLod)
{
	applyGeomorphWithVBHardPtr(geoms, alphaLod);
}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::applyGeomorphWithVBHardPtr(std::vector<CMRMWedgeGeom>  &geoms, float alphaLod)
{
	// no geomorphs? quit.
	if(geoms.size()==0)
		return;

	clamp(alphaLod, 0.f, 1.f);
	sint		a= (uint)floor((256.f*alphaLod)+0.5f);
	sint		a1= 256 - a;


	// info from VBuffer.
	uint8		*vertexPtr= (uint8*)_VBufferFinal.getPackedVertices();
	sint32		vertexSize= sizeof(CPackedVertexBuffer::CPackedVertex);

	// use a faster method
	applyGeomorphPosNormalUV0Int(geoms, vertexPtr, vertexPtr, vertexSize, a, a1);
}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::applyGeomorphPosNormalUV0(std::vector<CMRMWedgeGeom>  &geoms, uint8 *vertexPtr, uint8 *vertexDestPtr, sint32 vertexSize, float a, float a1)
{
	nlassert(vertexSize==32);


	// For all geomorphs.
	uint			nGeoms= (uint)geoms.size();
	CMRMWedgeGeom	*ptrGeom= &(geoms[0]);
	uint8			*destPtr= vertexDestPtr;
	for(; nGeoms>0; nGeoms--, ptrGeom++, destPtr+= vertexSize )
	{
		// Consider the Pos/Normal/UV as an array of 8 float to interpolate.
		float			*start=	(float*)(vertexPtr + (ptrGeom->Start<<5));
		float			*end=	(float*)(vertexPtr + (ptrGeom->End<<5));
		float			*dst=	(float*)(destPtr);

		// unrolled
		dst[0]= start[0] * a + end[0]* a1;
		dst[1]= start[1] * a + end[1]* a1;
		dst[2]= start[2] * a + end[2]* a1;
		dst[3]= start[3] * a + end[3]* a1;
		dst[4]= start[4] * a + end[4]* a1;
		dst[5]= start[5] * a + end[5]* a1;
		dst[6]= start[6] * a + end[6]* a1;
		dst[7]= start[7] * a + end[7]* a1;
	}
}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::applyGeomorphPosNormalUV0Int(std::vector<CMRMWedgeGeom>  &geoms, uint8 *vertexPtr, uint8 *vertexDestPtr, sint32 vertexSize, sint a, sint a1)
{
	// For all geomorphs.
	uint			nGeoms= (uint)geoms.size();
	CMRMWedgeGeom	*ptrGeom= &(geoms[0]);
	uint8			*destPtr= vertexDestPtr;
	for(; nGeoms>0; nGeoms--, ptrGeom++, destPtr+= vertexSize )
	{
		// Consider the Pos/Normal/UV as an array of 8 float to interpolate.
		sint16			*start=	(sint16*)(vertexPtr + (ptrGeom->Start*vertexSize));
		sint16			*end=	(sint16*)(vertexPtr + (ptrGeom->End*vertexSize));
		sint16			*dst=	(sint16*)(destPtr);

		/* Hulud
		 * This is slow but, we don't care because this method is called for debug purpose only (watch skin without skinning)
		 */

		// unrolled
		dst[0]= (sint16)(((sint)(start[0]) * a + (sint)(end[0]) * a1)>>8);
		dst[1]= (sint16)(((sint)(start[1]) * a + (sint)(end[1]) * a1)>>8);
		dst[2]= (sint16)(((sint)(start[2]) * a + (sint)(end[2]) * a1)>>8);
		dst[3]= (sint16)(((sint)(start[3]) * a + (sint)(end[3]) * a1)>>8);
		dst[4]= (sint16)(((sint)(start[4]) * a + (sint)(end[4]) * a1)>>8);
		dst[5]= (sint16)(((sint)(start[5]) * a + (sint)(end[5]) * a1)>>8);
		dst[6]= (sint16)(((sint)(start[6]) * a + (sint)(end[6]) * a1)>>8);
		dst[7]= (sint16)(((sint)(start[7]) * a + (sint)(end[7]) * a1)>>8);
	}
}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::initInstance(CMeshBaseInstance *mbi)
{
}


// ***************************************************************************
bool	CMeshMRMSkinnedGeom::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	// Speed Clip: clip just the sphere.
	CBSphere	localSphere(_BBox.getCenter(), _BBox.getRadius());
	CBSphere	worldSphere;

	// transform the sphere in WorldMatrix (with nearly good scale info).
	localSphere.applyTransform(worldMatrix, worldSphere);

	// if out of only plane, entirely out.
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// We are sure that pyramid has normalized plane normals.
		// if SpherMax OUT return false.
		float	d= pyramid[i]*worldSphere.Center;
		if(d>worldSphere.Radius)
			return false;
	}

	// test if must do a precise clip, according to mesh size.
	if( _PreciseClipping )
	{
		CPlane	localPlane;

		// if out of only plane, entirely out.
		for(sint i=0;i<(sint)pyramid.size();i++)
		{
			// Transform the pyramid in Object space.
			localPlane= pyramid[i]*worldMatrix;
			// localPlane must be normalized, because worldMatrix mya have a scale.
			localPlane.normalize();
			// if the box is not partially inside the plane, quit
			if( !_BBox.clipBack(localPlane) )
				return false;
		}
	}

	return true;
}


// ***************************************************************************
inline sint	CMeshMRMSkinnedGeom::chooseLod(float alphaMRM, float &alphaLod)
{
	// Choose what Lod to draw.
	alphaMRM*= _Lods.size()-1;
	sint	numLod= (sint)ceil(alphaMRM);
	if(numLod==0)
	{
		numLod= 0;
		alphaLod= 0;
	}
	else
	{
		// Lerp beetween lod i-1 and lod i.
		alphaLod= alphaMRM-(numLod-1);
	}

	/// Ensure numLod is correct
	if(numLod>=(sint)_Lods.size())
	{
		numLod= (sint)_Lods.size()-1;
		alphaLod= 1;
	}

	return numLod;
}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::render(IDriver *drv, CTransformShape *trans, float polygonCount, uint32 rdrFlags, float globalAlpha)
{
	nlassert(drv);
	if(_Lods.size()==0)
		return;


	// get the meshMRM instance.
	CMeshBaseInstance	*mi= safe_cast<CMeshBaseInstance*>(trans);


	// get the result of the Load Balancing.
	float	alphaMRM= _LevelDetail.getLevelDetailFromPolyCount(polygonCount);

	// choose the lod.
	float	alphaLod;
	sint	numLod= chooseLod(alphaMRM, alphaLod);


	// Render the choosen Lod.
	CLod	&lod= _Lods[numLod];
	if(lod.RdrPass.size()==0)
		return;


	// get the skeleton model to which I am binded (else NULL).
	CSkeletonModel *skeleton;
	skeleton = mi->getSkeletonModel();
	// The mesh must not be skinned for render()
	nlassert(!(mi->isSkinned() && skeleton));


	// Profiling
	//===========
	H_AUTO( NL3D_MeshMRMGeom_RenderNormal );

	// Skinning.
	//===========

	// set the instance worldmatrix.
	drv->setupModelMatrix(trans->getWorldMatrix());


	// Geomorph.
	//===========
	// Geomorph the choosen Lod (if not the coarser mesh).
	if(numLod>0)
	{
		applyGeomorph(lod.Geomorphs, alphaLod);
	}


	// force normalisation of normals..
	bool	bkupNorm= drv->isForceNormalize();
	drv->forceNormalize(true);


	// Setup meshVertexProgram
	//===========

	// Render the lod.
	//===========
	// active VB.

	/* Hulud
	 * This is slow but, we don't care because this method is called for debug purpose only (watch skin without skinning)
	 */
	CVertexBuffer tmp;
	getVertexBuffer (tmp);

	drv->activeVertexBuffer(tmp);


	// Global alpha used ?
	uint32	globalAlphaUsed= rdrFlags & IMeshGeom::RenderGlobalAlpha;
	uint8	globalAlphaInt=(uint8)NLMISC::OptFastFloor(globalAlpha*255);

	// Render all pass.
	if (globalAlphaUsed)
	{
		bool	gaDisableZWrite= (rdrFlags & IMeshGeom::RenderGADisableZWrite)?true:false;

		// for all passes
		for(uint i=0;i<lod.RdrPass.size();i++)
		{
			CRdrPass	&rdrPass= lod.RdrPass[i];

			if ( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (rdrFlags & IMeshGeom::RenderOpaqueMaterial) ) ||
				 ( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (rdrFlags & IMeshGeom::RenderTransparentMaterial) ) )
			{
				// CMaterial Ref
				CMaterial &material=mi->Materials[rdrPass.MaterialId];

				// Use a MeshBlender to modify material and driver.
				CMeshBlender	blender;
				blender.prepareRenderForGlobalAlpha(material, drv, globalAlpha, globalAlphaInt, gaDisableZWrite);

				/* Hulud
				 * This is slow but, we don't care because this method is called for debug purpose only (watch skin without skinning)
				 */
				CIndexBuffer block;
				lod.getRdrPassPrimitiveBlock (i, block);

				// Render
				drv->activeIndexBuffer(block);
				drv->renderTriangles(material, 0, block.getNumIndexes()/3);

				// Resetup material/driver
				blender.restoreRender(material, drv, gaDisableZWrite);
			}
		}
	}
	else
	{
		for(uint i=0;i<lod.RdrPass.size();i++)
		{
			CRdrPass	&rdrPass= lod.RdrPass[i];

			if ( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (rdrFlags & IMeshGeom::RenderOpaqueMaterial) ) ||
				 ( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (rdrFlags & IMeshGeom::RenderTransparentMaterial) ) )
			{
				// CMaterial Ref
				CMaterial &material=mi->Materials[rdrPass.MaterialId];

				/* Hulud
				 * This is slow but, we don't care because this method is called for debug purpose only (watch skin without skinning)
				 */
				CIndexBuffer block;
				lod.getRdrPassPrimitiveBlock (i, block);

				// Render with the Materials of the MeshInstance.
				drv->activeIndexBuffer(block);
				drv->renderTriangles(material, 0, block.getNumIndexes()/3);
			}
		}
	}

	// bkup force normalisation.
	drv->forceNormalize(bkupNorm);

}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::renderSkin(CTransformShape *trans, float alphaMRM)
{
}


// ***************************************************************************
bool	CMeshMRMSkinnedGeom::supportSkinGrouping() const
{
	return true;
}

// ***************************************************************************
sint	CMeshMRMSkinnedGeom::renderSkinGroupGeom(CMeshMRMSkinnedInstance	*mi, float alphaMRM, uint remainingVertices, uint8 *vbDest)
{
	H_AUTO( NL3D_MeshMRMGeom_rdrSkinGrpGeom )

	// since not tested in supportSkinGrouping(), must test _Lods.empty(): no lod, no draw
	if(_Lods.empty())
		return 0;

	// get a ptr on scene
	CScene				*ownerScene= mi->getOwnerScene();
	// get a ptr on renderTrav
	CRenderTrav			*renderTrav= &ownerScene->getRenderTrav();
	// get a ptr on the driver
	IDriver				*drv= renderTrav->getDriver();
	nlassert(drv);


	// choose the lod.
	float	alphaLod;
	sint	numLod= chooseLod(alphaMRM, alphaLod);
	_LastLodComputed= numLod;


	// Render the choosen Lod.
	CLod	&lod= _Lods[numLod];
	if(lod.RdrPass.size()==0)
		// return no vertices added
		return 0;

	// If the Lod is too big to render in the VBufferHard
	if(lod.NWedges>remainingVertices)
		// return Failure
		return -1;

	// get the skeleton model to which I am skinned
	CSkeletonModel *skeleton;
	skeleton = mi->getSkeletonModel();
	// must be skinned for renderSkin()
	nlassert(mi->isSkinned() && skeleton);


	// Profiling
	//===========
	H_AUTO( NL3D_MeshMRMGeom_rdrSkinGrpGeom_go );


	// Skinning.
	//===========

	// Use RawSkin if possible: only if no morph, and only Vertex/Normal
	updateRawSkinNormal(true, mi, numLod);

	// NB: the skeleton matrix has already been setuped by CSkeletonModel
	// NB: the normalize flag has already been setuped by CSkeletonModel

	// applySkin with RawSkin.
	//--------
	nlassert(mi->_RawSkinCache);

	H_AUTO( NL3D_RawSkinning );

	// RawSkin do all the job in optimized way: Skinning, copy to VBHard and Geomorph.

	// skinning with normal, but no tangent space
	applyRawSkinWithNormal (lod, *(mi->_RawSkinCache), skeleton, vbDest, alphaLod);

	// Vertices are packed in RawSkin mode (ie no holes due to MRM!)
	return	(sint)mi->_RawSkinCache->Geomorphs.size() +
			mi->_RawSkinCache->TotalSoftVertices +
			mi->_RawSkinCache->TotalHardVertices;
}

// ***************************************************************************
void	CMeshMRMSkinnedGeom::renderSkinGroupPrimitives(CMeshMRMSkinnedInstance	*mi, uint baseVertex, std::vector<CSkinSpecularRdrPass> &specularRdrPasses, uint skinIndex)
{
	H_AUTO( NL3D_MeshMRMGeom_rdrSkinGrpPrimitives );

	// get a ptr on scene
	CScene				*ownerScene= mi->getOwnerScene();
	// get a ptr on renderTrav
	CRenderTrav			*renderTrav= &ownerScene->getRenderTrav();
	// get a ptr on the driver
	IDriver				*drv= renderTrav->getDriver();
	nlassert(drv);

	// Get the lod choosen in renderSkinGroupGeom()
	CLod	&lod= _Lods[_LastLodComputed];


	// must update primitive cache
	updateShiftedTriangleCache(mi, _LastLodComputed, baseVertex);
	nlassert(mi->_ShiftedTriangleCache);


	// Render Triangles with cache
	//===========
	for(uint i=0;i<lod.RdrPass.size();i++)
	{
		CRdrPass	&rdrPass= lod.RdrPass[i];

		// CMaterial Ref
		CMaterial &material=mi->Materials[rdrPass.MaterialId];

		// TestYoyo. Material Speed Test
		/*if( material.getDiffuse()!=CRGBA(250, 251, 252) )
		{
			material.setDiffuse(CRGBA(250, 251, 252));
			// Set all texture the same.
			static CSmartPtr<ITexture>	pTexFile= new CTextureFile("fy_hom_visage_c1_fy_e1.tga");
			material.setTexture(0, pTexFile );
			// Remove Specular.
			if(material.getShader()==CMaterial::Specular)
			{
				CSmartPtr<ITexture>	tex= material.getTexture(0);
				material.setShader(CMaterial::Normal);
				material.setTexture(0, tex );
			}
			// Remove MakeUp
			material.setTexture(1, NULL);
		}*/

		// If the material is a specular material, don't render it now!
		if(material.getShader()==CMaterial::Specular)
		{
			// Add it to the rdrPass to sort!
			CSkinSpecularRdrPass	specRdrPass;
			specRdrPass.SkinIndex= skinIndex;
			specRdrPass.RdrPassIndex= i;
			// Get the handle of the specular Map as the sort Key
			ITexture	*specTex= material.getTexture(1);
			if(!specTex)
				specRdrPass.SpecId= 0;
			else
				specRdrPass.SpecId= drv->getTextureHandle( *specTex );
			// Append it to the list
			specularRdrPasses.push_back(specRdrPass);
		}
		else
		{
			// Get the shifted triangles.
			CShiftedTriangleCache::CRdrPass		&shiftedRdrPass= mi->_ShiftedTriangleCache->RdrPass[i];

			// Render with the Materials of the MeshInstance.
			drv->activeIndexBuffer(mi->_ShiftedTriangleCache->RawIndices);
			drv->renderTriangles(material, shiftedRdrPass.Triangles, shiftedRdrPass.NumTriangles);
		}
	}
}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::renderSkinGroupSpecularRdrPass(CMeshMRMSkinnedInstance	*mi, uint rdrPassId)
{
	H_AUTO( NL3D_MeshMRMGeom_rdrSkinGrpSpecularRdrPass );

	// get a ptr on scene
	CScene				*ownerScene= mi->getOwnerScene();
	// get a ptr on renderTrav
	CRenderTrav			*renderTrav= &ownerScene->getRenderTrav();
	// get a ptr on the driver
	IDriver				*drv= renderTrav->getDriver();
	nlassert(drv);

	// Get the lod choosen in renderSkinGroupGeom()
	CLod	&lod= _Lods[_LastLodComputed];


	// _ShiftedTriangleCache must have been computed in renderSkinGroupPrimitives
	nlassert(mi->_ShiftedTriangleCache);


	// Render Triangles with cache
	//===========
	CRdrPass	&rdrPass= lod.RdrPass[rdrPassId];

	// CMaterial Ref
	CMaterial &material=mi->Materials[rdrPass.MaterialId];

	// Get the shifted triangles.
	CShiftedTriangleCache::CRdrPass		&shiftedRdrPass= mi->_ShiftedTriangleCache->RdrPass[rdrPassId];

	// Render with the Materials of the MeshInstance.
	drv->activeIndexBuffer(mi->_ShiftedTriangleCache->RawIndices);
	drv->renderTriangles(material, shiftedRdrPass.Triangles, shiftedRdrPass.NumTriangles);
}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::updateShiftedTriangleCache(CMeshMRMSkinnedInstance *mi, sint curLodId, uint baseVertex)
{
	// if the instance has a cache, but not sync to us, delete it.
	if( mi->_ShiftedTriangleCache && (
		mi->_ShiftedTriangleCache->MeshDataId != _MeshDataId ||
		mi->_ShiftedTriangleCache->LodId != curLodId ||
		mi->_ShiftedTriangleCache->BaseVertex != baseVertex) )
	{
		mi->clearShiftedTriangleCache();
	}

	// If the instance has not a valid cache, must create it.
	if( !mi->_ShiftedTriangleCache )
	{
		mi->_ShiftedTriangleCache= new CShiftedTriangleCache;
		// Fill the cache Key.
		mi->_ShiftedTriangleCache->MeshDataId= _MeshDataId;
		mi->_ShiftedTriangleCache->LodId= curLodId;
		mi->_ShiftedTriangleCache->BaseVertex= baseVertex;

		// Build list of PBlock. From Lod, or from RawSkin cache.
		static	vector<CIndexBuffer*>	pbList;
		pbList.clear();
		nlassert(mi->_RawSkinCache);
		pbList.resize(mi->_RawSkinCache->RdrPass.size());
		for(uint i=0;i<pbList.size();i++)
		{
			pbList[i]= &mi->_RawSkinCache->RdrPass[i];
		}

		// Build RdrPass
		mi->_ShiftedTriangleCache->RdrPass.resize((uint32)pbList.size());

		// First pass, count number of triangles, and fill header info
		uint	totalTri= 0;
		uint	i;
		for(i=0;i<pbList.size();i++)
		{
			mi->_ShiftedTriangleCache->RdrPass[i].NumTriangles= pbList[i]->getNumIndexes()/3;
			totalTri+= pbList[i]->getNumIndexes()/3;
		}

		// Allocate triangles indices.
		mi->_ShiftedTriangleCache->RawIndices.setFormat(NL_SKINNED_MESH_MRM_INDEX_FORMAT);
		mi->_ShiftedTriangleCache->RawIndices.setNumIndexes(totalTri*3);

		CIndexBufferReadWrite iba;
		mi->_ShiftedTriangleCache->RawIndices.lock(iba);

		// Second pass, fill ptrs, and fill Arrays
		uint	indexTri= 0;
		for(i=0;i<pbList.size();i++)
		{
			CShiftedTriangleCache::CRdrPass	&dstRdrPass= mi->_ShiftedTriangleCache->RdrPass[i];
			dstRdrPass.Triangles= indexTri*3;

			// Fill the array
			uint	numTris= pbList[i]->getNumIndexes()/3;
			if(numTris)
			{
				uint	nIds= numTris*3;
				// index, and fill
				CIndexBufferRead ibaRead;
				pbList[i]->lock (ibaRead);
				#ifndef NL_SKINNED_MESH_MRM_INDEX16
					nlassert(iba.getFormat() == CIndexBuffer::Indices32);
					const uint32	*pSrcTri= (const uint32 *) ibaRead.getPtr();
					uint32	*pDstTri= (uint32 *) iba.getPtr() + dstRdrPass.Triangles;
					for(;nIds>0;nIds--,pSrcTri++,pDstTri++)
						*pDstTri= *pSrcTri + baseVertex;
				#else
					nlassert(iba.getFormat() == CIndexBuffer::Indices16);
					const uint16	*pSrcTri= (const uint16 *) ibaRead.getPtr();
					uint16	*pDstTri= (uint16 *) iba.getPtr() + dstRdrPass.Triangles;
					for(;nIds>0;nIds--,pSrcTri++,pDstTri++)
						*pDstTri= *pSrcTri + baseVertex;
				#endif
			}

			// Next
			indexTri+= dstRdrPass.NumTriangles;
		}
	}
}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::serial(NLMISC::IStream &f)
{
	// because of complexity, serial is separated in save / load.

	/*
	Version 0:
		- base version.
	*/
	f.serialVersion(0);


	// serial bones names
	f.serialCont (_BonesName);

	if (f.isReading())
	{
		// Bones index are in skeleton model id list
		_BoneIdComputed = false;

		// Must always recompute usage of parents of bones used.
		_BoneIdExtended = false;
	}

	// serial Basic info.
	// ==================
	f.serial(_BBox);
	f.serial(_LevelDetail.MaxFaceUsed);
	f.serial(_LevelDetail.MinFaceUsed);
	f.serial(_LevelDetail.DistanceFinest);
	f.serial(_LevelDetail.DistanceMiddle);
	f.serial(_LevelDetail.DistanceCoarsest);
	f.serial(_LevelDetail.OODistanceDelta);
	f.serial(_LevelDetail.DistancePow);

	// Prepare the VBuffer.
	if (f.isReading())
		_VBufferFinal.contReset();
	_VBufferFinal.serial(f);

	// serial Shadow Skin Information
	f.serialCont (_ShadowSkin.Vertices);
	f.serialCont (_ShadowSkin.Triangles);

	// resest the Lod arrays. NB: each Lod is empty, and ready to receive Lod data.
	// ==================
	if (f.isReading())
	{
		contReset(_Lods);
	}

	f.serialCont (_Lods);

	if (f.isReading())
	{
		// Inform that the mesh data has changed
		dirtMeshDataId();

		// Some runtime not serialized compilation
		compileRunTime();
	}
}

// ***************************************************************************

float CMeshMRMSkinnedGeom::getNumTriangles (float distance)
{
	// NB: this is an approximation, but this is continious.
	return _LevelDetail.getNumTriangles(distance);
}

// ***************************************************************************
void	CMeshMRMSkinnedGeom::computeBonesId (CSkeletonModel *skeleton)
{
	// Already computed ?
	if (!_BoneIdComputed)
	{
		// Get a pointer on the skeleton
		nlassert (skeleton);
		if (skeleton)
		{
			// **** For each bones, compute remap
			std::vector<uint> remap;
			skeleton->remapSkinBones(_BonesName, _BonesId, remap);


			// **** Remap the vertices, and compute Bone Spheres.

			// Find the Geomorph space: to process only real vertices, not geomorphed ones.
			uint	nGeomSpace= 0;
			uint	lod;
			for (lod=0; lod<_Lods.size(); lod++)
			{
				nGeomSpace= max(nGeomSpace, (uint)_Lods[lod].Geomorphs.size());
			}

			// Prepare Sphere compute
			static std::vector<CAABBox>		boneBBoxes;
			static std::vector<bool>		boneBBEmpty;
			boneBBoxes.clear();
			boneBBEmpty.clear();
			boneBBoxes.resize(_BonesId.size());
			boneBBEmpty.resize(_BonesId.size(), true);


			// Remap the vertex, and compute the bone spheres. see CTransform::getSkinBoneSphere() doc.
			// for true vertices
			uint vert;
			const uint vertexCount = _VBufferFinal.getNumVertices();
			CPackedVertexBuffer::CPackedVertex *vertices = _VBufferFinal.getPackedVertices();
			for (vert=nGeomSpace; vert<vertexCount; vert++)
			{
				// get the vertex position.
				CVector	vertex;
				_VBufferFinal.getPos (vertex, vertices[vert]);

				// For each weight
				uint weight;
				for (weight=0; weight<NL3D_MESH_SKINNING_MAX_MATRIX; weight++)
				{
					// Active ?
					if ((vertices[vert].Weights[weight]>0)||(weight==0))
					{
						// Check id
						uint	srcId= vertices[vert].Matrices[weight];
						nlassert (srcId < remap.size());
						// remap
						vertices[vert].Matrices[weight] = remap[srcId];

						// if the boneId is valid (ie found)
						if(_BonesId[srcId]>=0)
						{
							// transform the vertex pos in BoneSpace
							CVector		p= skeleton->Bones[_BonesId[srcId]].getBoneBase().InvBindPos * vertex;
							// extend the bone bbox.
							if(boneBBEmpty[srcId])
							{
								boneBBoxes[srcId].setCenter(p);
								boneBBEmpty[srcId]= false;
							}
							else
							{
								boneBBoxes[srcId].extend(p);
							}
						}
					}
					else
						break;
				}
			}

			// Compile spheres
			_BonesSphere.resize(_BonesId.size());
			for(uint bone=0;bone<_BonesSphere.size();bone++)
			{
				// If the bone is empty, mark with -1 in the radius.
				if(boneBBEmpty[bone])
				{
					_BonesSphere[bone].Radius= -1;
				}
				else
				{
					_BonesSphere[bone].Center= boneBBoxes[bone].getCenter();
					_BonesSphere[bone].Radius= boneBBoxes[bone].getRadius();
				}
			}

			// **** Remap the vertex influence by lods
			for (lod=0; lod<_Lods.size(); lod++)
			{
				// For each matrix used
				uint matrix;
				for (matrix=0; matrix<_Lods[lod].MatrixInfluences.size(); matrix++)
				{
					// Check
					nlassert (_Lods[lod].MatrixInfluences[matrix]<remap.size());

					// Remap
					_Lods[lod].MatrixInfluences[matrix] = remap[_Lods[lod].MatrixInfluences[matrix]];
				}
			}

			// **** Remap Shadow Vertices.
			for(vert=0;vert<_ShadowSkin.Vertices.size();vert++)
			{
				CShadowVertex	&v= _ShadowSkin.Vertices[vert];
				// Check id
				nlassert (v.MatrixId < remap.size());
				v.MatrixId= remap[v.MatrixId];
			}

			// Computed
			_BoneIdComputed = true;
		}
	}

	// Already extended ?
	if (!_BoneIdExtended)
	{
		nlassert (skeleton);
		if (skeleton)
		{
			// the total bone Usage of the mesh.
			vector<bool>	boneUsage;
			boneUsage.resize(skeleton->Bones.size(), false);

			// for all Bones marked as valid.
			uint	i;
			for(i=0; i<_BonesId.size(); i++)
			{
				// if not a valid boneId, skip it.
				if(_BonesId[i]<0)
					continue;

				// mark him and his father in boneUsage.
				skeleton->flagBoneAndParents(_BonesId[i], boneUsage);
			}

			// fill _BonesIdExt with bones of _BonesId and their parents.
			_BonesIdExt.clear();
			for(i=0; i<boneUsage.size();i++)
			{
				// if the bone is used by the mesh, add it to BoneIdExt.
				if(boneUsage[i])
					_BonesIdExt.push_back(i);
			}

		}

		// Extended
		_BoneIdExtended= true;
	}

}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::updateSkeletonUsage(CSkeletonModel *sm, bool increment)
{
	// For all Bones used.
	for(uint i=0; i<_BonesIdExt.size();i++)
	{
		uint	boneId= _BonesIdExt[i];
		// Some explicit Error.
		if(boneId>=sm->Bones.size())
			nlerror(" Skin is incompatible with Skeleton: tries to use bone %d", boneId);
		// increment or decrement not Forced, because CMeshGeom use getActiveBoneSkinMatrix().
		if(increment)
			sm->incBoneUsage(boneId, CSkeletonModel::UsageNormal);
		else
			sm->decBoneUsage(boneId, CSkeletonModel::UsageNormal);
	}
}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::compileRunTime()
{
	_PreciseClipping= _BBox.getRadius() >= NL3D_MESH_PRECISE_CLIP_THRESHOLD;

	// The Mesh must follow those restrictions, to support group skinning
	nlassert (_VBufferFinal.getNumVertices() < NL3D_MESH_SKIN_MANAGER_MAXVERTICES);

	// Support Shadow SkinGrouping if Shadow setuped, and if not too many vertices.
	_SupportShadowSkinGrouping= !_ShadowSkin.Vertices.empty() &&
		NL3D_SHADOW_MESH_SKIN_MANAGER_VERTEXFORMAT==CVertexBuffer::PositionFlag &&
		_ShadowSkin.Vertices.size() <= NL3D_SHADOW_MESH_SKIN_MANAGER_MAXVERTICES;
}


// ***************************************************************************
void	CMeshMRMSkinnedGeom::profileSceneRender(CRenderTrav *rdrTrav, CTransformShape *trans, float polygonCount, uint32 rdrFlags)
{
	// if no _Lods, no draw
	if(_Lods.empty())
		return;

	// get the result of the Load Balancing.
	float	alphaMRM= _LevelDetail.getLevelDetailFromPolyCount(polygonCount);

	// choose the lod.
	float	alphaLod;
	sint	numLod= chooseLod(alphaMRM, alphaLod);

	// Render the choosen Lod.
	CLod	&lod= _Lods[numLod];

	// get the mesh instance.
	CMeshBaseInstance	*mi= safe_cast<CMeshBaseInstance*>(trans);

	// Profile all pass.
	uint	triCount= 0;
	for (uint i=0;i<lod.RdrPass.size();i++)
	{
		CRdrPass	&rdrPass= lod.RdrPass[i];
		// Profile with the Materials of the MeshInstance.
		if ( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (rdrFlags & IMeshGeom::RenderOpaqueMaterial) ) ||
			 ( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (rdrFlags & IMeshGeom::RenderTransparentMaterial) ) )
		{
			triCount+= rdrPass.getNumTriangle();
		}
	}

	// Profile
	if(triCount)
	{
		// tri per VBFormat
		rdrTrav->Scene->incrementProfileTriVBFormat(rdrTrav->Scene->BenchRes.MeshMRMProfileTriVBFormat,
			NL3D_MESH_SKIN_MANAGER_VERTEXFORMAT, triCount);

		rdrTrav->Scene->BenchRes.NumMeshMRMVBufferStd++;
		rdrTrav->Scene->BenchRes.NumMeshMRMRdrNormal++;
		rdrTrav->Scene->BenchRes.NumMeshMRMTriRdrNormal+= triCount;
	}
}


// ***************************************************************************
bool	CMeshMRMSkinnedGeom::getSkinBoneBBox(CSkeletonModel *skeleton, NLMISC::CAABBox &bbox, uint boneId) const
{
	bbox.setCenter(CVector::Null);
	bbox.setHalfSize(CVector::Null);

	if(!skeleton)
		return false;

	// get the bindpos of the wanted bone
	nlassert(boneId<skeleton->Bones.size());
	const CMatrix		&invBindPos= skeleton->Bones[boneId].getBoneBase().InvBindPos;


	// Find the Geomorph space: to process only real vertices, not geomorphed ones.
	uint	nGeomSpace= 0;
	uint	lod;
	for (lod=0; lod<_Lods.size(); lod++)
	{
		nGeomSpace= max(nGeomSpace, (uint)_Lods[lod].Geomorphs.size());
	}

	// Prepare BBox compute
	bool	bbEmpty= true;

	// Remap the vertex, and compute the wanted bone bbox
	// for true vertices
	const uint vertexCount = _VBufferFinal.getNumVertices();
	const CPackedVertexBuffer::CPackedVertex *vertices = _VBufferFinal.getPackedVertices();
	for (uint vert=nGeomSpace; vert<vertexCount; vert++)
	{
		// get the vertex position.
		CVector	vertex;
		_VBufferFinal.getPos (vertex, vertices[vert]);

		// For each weight
		uint weight;
		for (weight=0; weight<NL3D_MESH_SKINNING_MAX_MATRIX; weight++)
		{
			// Active ?
			if ((vertices[vert].Weights[weight]>0)||(weight==0))
			{
				// Check id is the wanted one
				if(vertices[vert].Matrices[weight]==boneId)
				{
					// transform the vertex pos in BoneSpace
					CVector		p= invBindPos * vertex;
					// extend the bone bbox.
					if(bbEmpty)
					{
						bbox.setCenter(p);
						bbEmpty= false;
					}
					else
					{
						bbox.extend(p);
					}
				}
			}
			else
				break;
		}
	}

	// return true if some influence found
	return !bbEmpty;
}



// ***************************************************************************
// ***************************************************************************
// Mesh Block Render Interface
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CMeshMRMSkinnedGeom::supportMeshBlockRendering () const
{
	/*
		Yoyo: Don't Support It for MRM because too Slow!!
		The problem is that lock() unlock() on each instance, on the same VBHeap IS AS SLOWER AS
		VB switching.

		TODO_OPTIMIZE: find a way to optimize MRM.
	*/
	return false;
}

// ***************************************************************************
// ***************************************************************************
// CMeshMRMSkinned.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
CMeshMRMSkinned::CMeshMRMSkinned()
{
}

// ***************************************************************************

bool			CMeshMRMSkinned::isCompatible(const CMesh::CMeshBuild &m)
{
	/* Optimised shape for skinned object with MRM, 1 UV coordinates, 1 to 4 skinning weight and 256 matrices
	Tangeant space, vertex program, mesh block rendering and vertex buffer hard are not available. */

	// Vertex format
	if (m.VertexFlags != NL3D_MESH_MRM_SKINNED_VERTEX_FORMAT)
		return false;

	// No W
	if (m.NumCoords[0] != 2)
		return false;

	// No blend shape
	if (!m.BlendShapes.empty())
		return false;

	// Check number of vertices
	if (m.Vertices.size() > NL3D_MESH_SKIN_MANAGER_MAXVERTICES)
		return false;

	// Ok
	return true;
}

// ***************************************************************************
void			CMeshMRMSkinned::build (CMeshBase::CMeshBaseBuild &mBase, CMesh::CMeshBuild &m,
								 const CMRMParameters &params)
{
	nlassert (isCompatible(m));

	/// copy MeshBase info: materials ....
	CMeshBase::buildMeshBase (mBase);

	// Then build the geom.
	_MeshMRMGeom.build (m, (uint)mBase.Materials.size(), params);
}
// ***************************************************************************
void			CMeshMRMSkinned::build (CMeshBase::CMeshBaseBuild &m, const CMeshMRMSkinnedGeom &mgeom)
{
	/// copy MeshBase info: materials ....
	CMeshBase::buildMeshBase(m);

	// Then copy the geom.
	_MeshMRMGeom= mgeom;
}


// ***************************************************************************
void			CMeshMRMSkinned::optimizeMaterialUsage(std::vector<sint> &remap)
{
	// For each material, count usage.
	vector<bool>	materialUsed;
	materialUsed.resize(CMeshBase::_Materials.size(), false);
	for(uint lod=0;lod<getNbLod();lod++)
	{
		for(uint rp=0;rp<getNbRdrPass(lod);rp++)
		{
			uint	matId= getRdrPassMaterial(lod, rp);
			// flag as used.
			materialUsed[matId]= true;
		}
	}

	// Apply it to meshBase
	CMeshBase::applyMaterialUsageOptim(materialUsed, remap);

	// Apply lut to meshGeom.
	_MeshMRMGeom.applyMaterialRemap(remap);
}


// ***************************************************************************
CTransformShape		*CMeshMRMSkinned::createInstance(CScene &scene)
{
	// Create a CMeshMRMSkinnedInstance, an instance of a mesh.
	//===============================================
	CMeshMRMSkinnedInstance		*mi= (CMeshMRMSkinnedInstance*)scene.createModel(NL3D::MeshMRMSkinnedInstanceId);
	mi->Shape= this;

	// instanciate the material part of the MeshMRM, ie the CMeshBase.
	CMeshBase::instanciateMeshBase(mi, &scene);


	// do some instance init for MeshGeom
	_MeshMRMGeom.initInstance(mi);

	// init the FilterType
	mi->initRenderFilterType();

	return mi;
}


// ***************************************************************************
bool	CMeshMRMSkinned::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	return _MeshMRMGeom.clip(pyramid, worldMatrix);
}


// ***************************************************************************
void	CMeshMRMSkinned::render(IDriver *drv, CTransformShape *trans, bool passOpaque)
{
	// 0 or 0xFFFFFFFF
	uint32	mask= (0-(uint32)passOpaque);
	uint32	rdrFlags;
	// select rdrFlags, without ifs.
	rdrFlags=	mask & (IMeshGeom::RenderOpaqueMaterial | IMeshGeom::RenderPassOpaque);
	rdrFlags|=	~mask & (IMeshGeom::RenderTransparentMaterial);
	// render the mesh
	_MeshMRMGeom.render(drv, trans, trans->getNumTrianglesAfterLoadBalancing(), rdrFlags, 1);
}


// ***************************************************************************
void	CMeshMRMSkinned::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	*/
	(void)f.serialVersion(0);

	// serial Materials infos contained in CMeshBase.
	CMeshBase::serialMeshBase(f);


	// serial the geometry.
	_MeshMRMGeom.serial(f);
}


// ***************************************************************************
float	CMeshMRMSkinned::getNumTriangles (float distance)
{
	return _MeshMRMGeom.getNumTriangles (distance);
}


// ***************************************************************************
const CMeshMRMSkinnedGeom& CMeshMRMSkinned::getMeshGeom () const
{
	return _MeshMRMGeom;
}


// ***************************************************************************
void	CMeshMRMSkinned::computeBonesId (CSkeletonModel *skeleton)
{
	_MeshMRMGeom.computeBonesId (skeleton);
}

// ***************************************************************************
void	CMeshMRMSkinned::updateSkeletonUsage (CSkeletonModel *skeleton, bool increment)
{
	_MeshMRMGeom.updateSkeletonUsage(skeleton, increment);
}

// ***************************************************************************
void	CMeshMRMSkinned::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	_MeshMRMGeom.changeMRMDistanceSetup(distanceFinest, distanceMiddle, distanceCoarsest);
}

// ***************************************************************************
IMeshGeom	*CMeshMRMSkinned::supportMeshBlockRendering (CTransformShape *trans, float &polygonCount ) const
{
	return NULL;
}

// ***************************************************************************
void	CMeshMRMSkinned::profileSceneRender(CRenderTrav *rdrTrav, CTransformShape *trans, bool passOpaque)
{
	// 0 or 0xFFFFFFFF
	uint32	mask= (0-(uint32)passOpaque);
	uint32	rdrFlags;
	// select rdrFlags, without ifs.
	rdrFlags=	mask & (IMeshGeom::RenderOpaqueMaterial | IMeshGeom::RenderPassOpaque);
	rdrFlags|=	~mask & (IMeshGeom::RenderTransparentMaterial);
	// profile the mesh
	_MeshMRMGeom.profileSceneRender(rdrTrav, trans, trans->getNumTrianglesAfterLoadBalancing(), rdrFlags);
}



// ***************************************************************************
// ***************************************************************************
// CMeshMRMSkinnedGeom RawSkin optimisation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CMeshMRMSkinnedGeom::dirtMeshDataId()
{
	// see updateRawSkinNormal()
	_MeshDataId++;
}


// ***************************************************************************
void		CMeshMRMSkinnedGeom::updateRawSkinNormal(bool enabled, CMeshMRMSkinnedInstance *mi, sint curLodId)
{
	if(!enabled)
	{
		// if the instance cache is not cleared, must clear.
		mi->clearRawSkinCache();
	}
	else
	{
		// If the instance has no RawSkin, or has a too old RawSkin cache, must delete it, and recreate
		if ((mi->_RawSkinCache == NULL) || (mi->_RawSkinCache->MeshDataId!=_MeshDataId))
		{
			// first delete if too old.
			mi->clearRawSkinCache();

			// Then recreate, and use _MeshDataId to verify that the instance works with same data.
			mi->_RawSkinCache= new CRawSkinnedNormalCache;
			mi->_RawSkinCache->MeshDataId= _MeshDataId;
			mi->_RawSkinCache->LodId= -1;
		}


		/* If the instance rawSkin has a different Lod (or if -1), then must recreate it.
			NB: The lod may change each frame per instance, but suppose not so many change, so we can cache those data.
		*/
		if( mi->_RawSkinCache->LodId != curLodId )
		{
			H_AUTO( NL3D_CMeshMRMGeom_updateRawSkinNormal );

			CRawSkinnedNormalCache	&skinLod= *mi->_RawSkinCache;
			CLod				&lod= _Lods[curLodId];
			uint				i;
			sint				rawIdx;

			// Clear the raw skin mesh.
			skinLod.clearArrays();

			// Cache this lod
			mi->_RawSkinCache->LodId= curLodId;

			// For each matrix influence.
			nlassert(NL3D_MESH_SKINNING_MAX_MATRIX==4);

			// For each vertex, acknowledge if it is a src for geomorph.
			static	vector<uint8>	softVertices;
			softVertices.clear();
			softVertices.resize( _VBufferFinal.getNumVertices(), 0 );
			for(i=0;i<lod.Geomorphs.size();i++)
			{
				softVertices[lod.Geomorphs[i].Start]= 1;
				softVertices[lod.Geomorphs[i].End]= 1;
			}

			// The remap from old index in _VBufferFinal to RawSkin vertices (without Geomorphs).
			static	vector<uint32>	vertexRemap;
			vertexRemap.resize( _VBufferFinal.getNumVertices() );
			sint	softSize[4];
			sint	hardSize[4];
			sint	softStart[4];
			sint	hardStart[4];
			// count vertices
			skinLod.TotalSoftVertices= 0;
			skinLod.TotalHardVertices= 0;
			for(i=0;i<4;i++)
			{
				softSize[i]= 0;
				hardSize[i]= 0;
				// Count.
				for(uint j=0;j<lod.InfluencedVertices[i].size();j++)
				{
					uint	vid= lod.InfluencedVertices[i][j];
					if(softVertices[vid])
						softSize[i]++;
					else
						hardSize[i]++;
				}
				skinLod.TotalSoftVertices+= softSize[i];
				skinLod.TotalHardVertices+= hardSize[i];
				skinLod.SoftVertices[i]= softSize[i];
				skinLod.HardVertices[i]= hardSize[i];
			}
			// compute offsets
			softStart[0]= 0;
			hardStart[0]= skinLod.TotalSoftVertices;
			for(i=1;i<4;i++)
			{
				softStart[i]= softStart[i-1]+softSize[i-1];
				hardStart[i]= hardStart[i-1]+hardSize[i-1];
			}
			// compute remap
			for(i=0;i<4;i++)
			{
				uint	softIdx= softStart[i];
				uint	hardIdx= hardStart[i];
				for(uint j=0;j<lod.InfluencedVertices[i].size();j++)
				{
					uint	vid= lod.InfluencedVertices[i][j];
					if(softVertices[vid])
						vertexRemap[vid]= softIdx++;
					else
						vertexRemap[vid]= hardIdx++;
				}
			}


			// Resize the dest array.
			skinLod.Vertices1.resize((uint32)lod.InfluencedVertices[0].size());
			skinLod.Vertices2.resize((uint32)lod.InfluencedVertices[1].size());
			skinLod.Vertices3.resize((uint32)lod.InfluencedVertices[2].size());
			skinLod.Vertices4.resize((uint32)lod.InfluencedVertices[3].size());

			// Vertex buffer pointers
			const CPackedVertexBuffer::CPackedVertex *vertices = _VBufferFinal.getPackedVertices();

			// 1 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices1.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[0][i];
				// where to store?
				rawIdx= vertexRemap[vid];
				if(softVertices[vid])
					rawIdx-= softStart[0];
				else
					rawIdx+= softSize[0]-hardStart[0];

				// fill raw struct
				const CPackedVertexBuffer::CPackedVertex &vertex = vertices[vid];
				skinLod.Vertices1[rawIdx].MatrixId[0]= vertex.Matrices[0];
				_VBufferFinal.getPos (skinLod.Vertices1[rawIdx].Vertex, vertex);
				vertex.getNormal (skinLod.Vertices1[rawIdx].Normal);
				vertex.getU (skinLod.Vertices1[rawIdx].UV.U);
				vertex.getV (skinLod.Vertices1[rawIdx].UV.V);
			}

			// 2 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices2.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[1][i];
				// where to store?
				rawIdx= vertexRemap[vid];
				if(softVertices[vid])
					rawIdx-= softStart[1];
				else
					rawIdx+= softSize[1]-hardStart[1];
				// fill raw struct
				const CPackedVertexBuffer::CPackedVertex &vertex = vertices[vid];
				skinLod.Vertices2[rawIdx].MatrixId[0]= vertex.Matrices[0];
				skinLod.Vertices2[rawIdx].MatrixId[1]= vertex.Matrices[1];
				_VBufferFinal.getPos (skinLod.Vertices2[rawIdx].Vertex, vertex);
				vertex.getWeight (skinLod.Vertices2[rawIdx].Weights[0], 0);
				vertex.getWeight (skinLod.Vertices2[rawIdx].Weights[1], 1);
				vertex.getNormal (skinLod.Vertices2[rawIdx].Normal);
				vertex.getU (skinLod.Vertices2[rawIdx].UV.U);
				vertex.getV (skinLod.Vertices2[rawIdx].UV.V);
			}

			// 3 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices3.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[2][i];
				// where to store?
				rawIdx= vertexRemap[vid];
				if(softVertices[vid])
					rawIdx-= softStart[2];
				else
					rawIdx+= softSize[2]-hardStart[2];
				// fill raw struct
				const CPackedVertexBuffer::CPackedVertex &vertex = vertices[vid];
				skinLod.Vertices3[rawIdx].MatrixId[0]= vertex.Matrices[0];
				skinLod.Vertices3[rawIdx].MatrixId[1]= vertex.Matrices[1];
				skinLod.Vertices3[rawIdx].MatrixId[2]= vertex.Matrices[2];
				_VBufferFinal.getPos (skinLod.Vertices3[rawIdx].Vertex, vertex);
				vertex.getWeight (skinLod.Vertices3[rawIdx].Weights[0], 0);
				vertex.getWeight (skinLod.Vertices3[rawIdx].Weights[1], 1);
				vertex.getWeight (skinLod.Vertices3[rawIdx].Weights[2], 2);
				vertex.getNormal (skinLod.Vertices3[rawIdx].Normal);
				vertex.getU (skinLod.Vertices3[rawIdx].UV.U);
				vertex.getV (skinLod.Vertices3[rawIdx].UV.V);
			}

			// 4 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices4.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[3][i];
				// where to store?
				rawIdx= vertexRemap[vid];
				if(softVertices[vid])
					rawIdx-= softStart[3];
				else
					rawIdx+= softSize[3]-hardStart[3];
				// fill raw struct
				const CPackedVertexBuffer::CPackedVertex &vertex = vertices[vid];
				skinLod.Vertices4[rawIdx].MatrixId[0]= vertex.Matrices[0];
				skinLod.Vertices4[rawIdx].MatrixId[1]= vertex.Matrices[1];
				skinLod.Vertices4[rawIdx].MatrixId[2]= vertex.Matrices[2];
				skinLod.Vertices4[rawIdx].MatrixId[3]= vertex.Matrices[3];
				_VBufferFinal.getPos (skinLod.Vertices4[rawIdx].Vertex, vertex);
				vertex.getWeight (skinLod.Vertices4[rawIdx].Weights[0], 0);
				vertex.getWeight (skinLod.Vertices4[rawIdx].Weights[1], 1);
				vertex.getWeight (skinLod.Vertices4[rawIdx].Weights[2], 2);
				vertex.getWeight (skinLod.Vertices4[rawIdx].Weights[3], 3);
				vertex.getNormal (skinLod.Vertices4[rawIdx].Normal);
				vertex.getU (skinLod.Vertices4[rawIdx].UV.U);
				vertex.getV (skinLod.Vertices4[rawIdx].UV.V);
			}

			// Remap Geomorphs.
			//========
			uint	numGeoms= (uint)lod.Geomorphs.size();
			skinLod.Geomorphs.resize( numGeoms );
			for(i=0;i<numGeoms;i++)
			{
				// NB: don't add "numGeoms" to the index because RawSkin look in a TempArray in RAM, which start at 0...
				skinLod.Geomorphs[i].Start= vertexRemap[lod.Geomorphs[i].Start];
				skinLod.Geomorphs[i].End= vertexRemap[lod.Geomorphs[i].End];
			}

			// Remap RdrPass.
			//========
			skinLod.RdrPass.resize(lod.RdrPass.size());
			for(i=0;i<skinLod.RdrPass.size();i++)
			{
				// remap tris.
				skinLod.RdrPass[i].setFormat(NL_SKINNED_MESH_MRM_INDEX_FORMAT);
				skinLod.RdrPass[i].setNumIndexes(lod.RdrPass[i].getNumTriangle()*3);
				const uint16	*srcTriPtr= &(lod.RdrPass[i].PBlock[0]);
				CIndexBufferReadWrite ibaWrite;
				skinLod.RdrPass[i].lock (ibaWrite);
				#ifndef NL_SKINNED_MESH_MRM_INDEX16
					nlassert(ibaWrite.getFormat() == CIndexBuffer::Indices32);
					uint32	*dstTriPtr= (uint32	*) ibaWrite.getPtr();
					uint32	numIndices= lod.RdrPass[i].PBlock.size();
					for(uint j=0;j<numIndices;j++, srcTriPtr++, dstTriPtr++)
					{
						uint	vid= (uint)*srcTriPtr;
						// If this index refers to a Geomorphed vertex, don't modify!
						if(vid<numGeoms)
							*dstTriPtr= vid;
						else
							*dstTriPtr= vertexRemap[vid] + numGeoms;
					}
				#else
					nlassert(ibaWrite.getFormat() == CIndexBuffer::Indices16);
					uint16	*dstTriPtr= (uint16 *) ibaWrite.getPtr();
					uint32	numIndices= (uint32)lod.RdrPass[i].PBlock.size();
					for(uint j=0;j<numIndices;j++, srcTriPtr++, dstTriPtr++)
					{
						uint	vid= (uint)*srcTriPtr;
						// If this index refers to a Geomorphed vertex, don't modify!
						if(vid<numGeoms)
							*dstTriPtr= vid;
						else
							*dstTriPtr= (uint16) (vertexRemap[vid] + numGeoms);
					}
				#endif
			}
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// CMeshMRMSkinnedGeom Shadow Skin Rendering
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CMeshMRMSkinnedGeom::setShadowMesh(const std::vector<CShadowVertex> &shadowVertices, const std::vector<uint32> &triangles)
{
	_ShadowSkin.Vertices= shadowVertices;
	_ShadowSkin.Triangles= triangles;
	// update flag. Support Shadow SkinGrouping if Shadow setuped, and if not too many vertices.
	_SupportShadowSkinGrouping= !_ShadowSkin.Vertices.empty() &&
		NL3D_SHADOW_MESH_SKIN_MANAGER_VERTEXFORMAT==CVertexBuffer::PositionFlag &&
		_ShadowSkin.Vertices.size() <= NL3D_SHADOW_MESH_SKIN_MANAGER_MAXVERTICES;
}

// ***************************************************************************
uint			CMeshMRMSkinnedGeom::getNumShadowSkinVertices() const
{
	return (uint)_ShadowSkin.Vertices.size();
}

// ***************************************************************************
sint			CMeshMRMSkinnedGeom::renderShadowSkinGeom(CMeshMRMSkinnedInstance	*mi, uint remainingVertices, uint8 *vbDest)
{
	uint	numVerts= (uint)_ShadowSkin.Vertices.size();

	// if no verts, no draw
	if(numVerts==0)
		return 0;

	// if no lods, there should be no verts, but still ensure no bug in skinning
	if(_Lods.empty())
		return 0;

	// If the Lod is too big to render in the VBufferHard
	if(numVerts>remainingVertices)
		// return Failure
		return -1;

	// get the skeleton model to which I am skinned
	CSkeletonModel *skeleton;
	skeleton = mi->getSkeletonModel();
	// must be skinned for renderSkin()
	nlassert(skeleton);


	// Profiling
	//===========
	H_AUTO_USE( NL3D_MeshMRMGeom_RenderShadow );


	// Skinning.
	//===========

	// For all matrix this Mesh use. (the shadow geometry cannot use other Matrix than the mesh use).
	// NB: take the best lod since the lower lods cannot use other Matrix than the higher one.
	static	vector<CMatrix3x4>		boneMat3x4;
	CLod	&lod= _Lods[_Lods.size()-1];
	computeBoneMatrixes3x4(boneMat3x4, lod.MatrixInfluences, skeleton);

	_ShadowSkin.applySkin((CVector*)vbDest, boneMat3x4);


	// How many vertices are added to the VBuffer ???
	return numVerts;
}

// ***************************************************************************
void			CMeshMRMSkinnedGeom::renderShadowSkinPrimitives(CMeshMRMSkinnedInstance	*mi, CMaterial &castMat, IDriver *drv, uint baseVertex)
{
	nlassert(drv);

	if(_ShadowSkin.Triangles.empty())
		return;

	// Profiling
	//===========
	H_AUTO_USE( NL3D_MeshMRMGeom_RenderShadow );

	// NB: the skeleton matrix has already been setuped by CSkeletonModel
	// NB: the normalize flag has already been setuped by CSkeletonModel

	// TODO_SHADOW: optim: Special triangle cache for shadow!
	static	CIndexBuffer		shiftedTris;
	shiftedTris.setPreferredMemory(CIndexBuffer::RAMVolatile, false);
	if (shiftedTris.getName().empty()) NL_SET_IB_NAME(shiftedTris, "CMeshMRMSkinnedGeom::renderShadowSkinPrimitives::shiftedTris");
	//if(shiftedTris.getNumIndexes()<_ShadowSkin.Triangles.size())
	//{
		shiftedTris.setFormat(NL_SKINNED_MESH_MRM_INDEX_FORMAT);
		shiftedTris.setNumIndexes((uint32)_ShadowSkin.Triangles.size());
	//}
	{
		CIndexBufferReadWrite iba;
		shiftedTris.lock(iba);
		const uint32	*src= &_ShadowSkin.Triangles[0];
		TSkinnedMeshMRMIndexType	*dst= (TSkinnedMeshMRMIndexType*) iba.getPtr();
		for(uint n= (uint)_ShadowSkin.Triangles.size();n>0;n--, src++, dst++)
		{
			*dst= (TSkinnedMeshMRMIndexType)(*src + baseVertex);
		}
	}

	// Render Triangles with cache
	//===========

	uint	numTris= (uint)_ShadowSkin.Triangles.size()/3;

	// Render with the Materials of the MeshInstance.
	drv->activeIndexBuffer(shiftedTris);
	drv->renderTriangles(castMat, 0, numTris);
}

// ***************************************************************************
bool		CMeshMRMSkinnedGeom::intersectSkin(CMeshMRMSkinnedInstance *mi, const CMatrix &toRaySpace, float &dist2D, float &distZ, bool computeDist2D)
{
	// no inst/verts/lod => no intersection
	if(!mi || _ShadowSkin.Vertices.empty() || _Lods.empty())
		return false;
	CSkeletonModel	*skeleton= mi->getSkeletonModel();
	if(!skeleton)
		return false;

	// Compute skinning with all matrix this Mesh use. (the shadow geometry cannot use other Matrix than the mesh use).
	// NB: take the best lod (_Lods.back()) since the lower lods cannot use other Matrix than the higher one.
	return _ShadowSkin.getRayIntersection(toRaySpace, *skeleton, _Lods.back().MatrixInfluences, dist2D, distZ, computeDist2D);
}


// ***************************************************************************
// ***************************************************************************
// CMeshMRMSkinnedGeom::CPackedVertexBuffer
// ***************************************************************************
// ***************************************************************************

void CMeshMRMSkinnedGeom::CPackedVertexBuffer::serial(NLMISC::IStream &f)
{
	// Version
	f.serialVersion(0);

	f.serialCont (_PackedBuffer);
	f.serial (_DecompactScale);
}

// ***************************************************************************

void CMeshMRMSkinnedGeom::CPackedVertexBuffer::CPackedVertex::serial(NLMISC::IStream &f)
{
	// Version
	f.serialVersion(0);

	f.serial (X);
	f.serial (Y);
	f.serial (Z);
	f.serial (Nx);
	f.serial (Ny);
	f.serial (Nz);
	f.serial (U);
	f.serial (V);
	uint i;
	for (i=0; i<NL3D_MESH_MRM_SKINNED_MAX_MATRIX; i++)
	{
		f.serial (Matrices[i]);
		f.serial (Weights[i]);
	}
}

// ***************************************************************************

void CMeshMRMSkinnedGeom::CPackedVertexBuffer::CPackedVertex::setNormal (const CVector &src)
{
	CVector pos = src;
	pos *= NL3D_MESH_MRM_SKINNED_NORMAL_FACTOR;
	pos.x = (float)floor(pos.x+0.5f);
	pos.y = (float)floor(pos.y+0.5f);
	pos.z = (float)floor(pos.z+0.5f);
	clamp (pos.x, -32768.f, 32767.f);
	clamp (pos.y, -32768.f, 32767.f);
	clamp (pos.z, -32768.f, 32767.f);
	Nx = (sint16)pos.x;
	Ny = (sint16)pos.y;
	Nz = (sint16)pos.z;
}

// ***************************************************************************

void CMeshMRMSkinnedGeom::CPackedVertexBuffer::CPackedVertex::setPos (const NLMISC::CVector &pos, float scaleFactor)
{
	CVector _pos = pos;
	_pos /= scaleFactor;
	_pos.x = (float)floor(_pos.x+0.5f);
	_pos.y = (float)floor(_pos.y+0.5f);
	_pos.z = (float)floor(_pos.z+0.5f);
	clamp (_pos.x, -32768.f, 32767.f);
	clamp (_pos.y, -32768.f, 32767.f);
	clamp (_pos.z, -32768.f, 32767.f);
	X = (sint16)_pos.x;
	Y = (sint16)_pos.y;
	Z = (sint16)_pos.z;
}

// ***************************************************************************

void CMeshMRMSkinnedGeom::CPackedVertexBuffer::CPackedVertex::setUV (float _u, float _v)
{
	float u = _u * NL3D_MESH_MRM_SKINNED_UV_FACTOR;
	float v = _v * NL3D_MESH_MRM_SKINNED_UV_FACTOR;
	u = (float)floor(u+0.5f);
	v = (float)floor(v+0.5f);
	clamp (u, -32768.f, 32767.f);
	clamp (v, -32768.f, 32767.f);
	U = (sint16)u;
	V = (sint16)v;
}

// ***************************************************************************

void CMeshMRMSkinnedGeom::CPackedVertexBuffer::CPackedVertex::setWeight (uint weightId, float weight)
{
	weight = weight * NL3D_MESH_MRM_SKINNED_WEIGHT_FACTOR;
	weight = (float)floor(weight+0.5f);
	clamp (weight, 0.f, 255.f);
	Weights[weightId] = (uint8)weight;
}

// ***************************************************************************

void CMeshMRMSkinnedGeom::CPackedVertexBuffer::build (const CVertexBuffer &buffer, const std::vector<CMesh::CSkinWeight> &skinWeight)
{
	const uint numVertices = buffer.getNumVertices();
	nlassert (numVertices == skinWeight.size());
 	nlassert ((buffer.getVertexFormat() & (CVertexBuffer::PositionFlag|CVertexBuffer::NormalFlag|CVertexBuffer::TexCoord0Flag)) == (CVertexBuffer::PositionFlag|CVertexBuffer::NormalFlag|CVertexBuffer::TexCoord0Flag));

	_PackedBuffer.resize (numVertices);

	// default scale
	_DecompactScale = NL3D_MESH_MRM_SKINNED_DEFAULT_POS_SCALE;

	CVertexBufferRead vba;
	buffer.lock (vba);

	if (numVertices)
	{
		// Get the min max of the bbox
		CVector _min = *vba.getVertexCoordPointer(0);
		CVector _max = _min;

		// For each vertex
		uint i;
		for (i=1; i<numVertices; i++)
		{
			// Update min max
			const CVector &vect = *vba.getVertexCoordPointer(i);
			_min.minof(_min, vect);
			_max.maxof(_max, vect);
		}

		// Scale enough ?
		float max_width = std::max((float)fabs(_min.x), std::max((float)fabs(_min.y), (float)fabs(_min.z)));
		max_width = std::max(max_width, std::max((float)fabs(_max.x), std::max((float)fabs(_max.y), (float)fabs(_max.z))));
		_DecompactScale = std::max(_DecompactScale, max_width / 32767.f);

		// Pack
		for (i=0; i<numVertices; i++)
		{
			CPackedVertex &vertex = _PackedBuffer[i];

			// Position
			vertex.setPos (*vba.getVertexCoordPointer(i), _DecompactScale);

			// Normal
			vertex.setNormal (*vba.getNormalCoordPointer(i));

			// UV
			const float *uv = (const float*)vba.getTexCoordPointer(i, 0);
			vertex.setUV (uv[0], uv[1]);

			// Matrices
			uint j;
			sint weightSum = 0;
			for (j=0; j<NL3D_MESH_MRM_SKINNED_MAX_MATRIX; j++)
			{
				vertex.Matrices[j] = (uint8)skinWeight[i].MatrixId[j];
				vertex.setWeight (j, skinWeight[i].Weights[j]);
				weightSum += (sint)vertex.Weights[j];
			}

			// Weight error
			weightSum = 255 - weightSum;

			// Add error to the first weight
			nlassert ((sint)vertex.Weights[0] + weightSum <= 255);
			nlassert ((sint)vertex.Weights[0] + weightSum > 0);
			vertex.Weights[0] += weightSum;
		}
	}
}

// ***************************************************************************

void CMeshMRMSkinnedGeom::getVertexBuffer(CVertexBuffer &output) const
{
	output.setVertexFormat (CVertexBuffer::PositionFlag|CVertexBuffer::NormalFlag|CVertexBuffer::TexCoord0Flag);
	const uint numVertices = _VBufferFinal.getNumVertices();
	output.setNumVertices (numVertices);

	// Set all UV routing on 0
	uint i;
	for (i=0; i<CVertexBuffer::MaxStage; i++)
		output.setUVRouting (i, 0);

	CVertexBufferReadWrite vba;
	output.lock (vba);
	const CPackedVertexBuffer::CPackedVertex	*vertex = _VBufferFinal.getPackedVertices();
	for (i=0; i<numVertices; i++)
	{
		_VBufferFinal.getPos(*vba.getVertexCoordPointer(i), vertex[i]);
		vertex[i].getNormal(*vba.getNormalCoordPointer(i));
		float *texCoord = (float*)vba.getTexCoordPointer(i,0);
		vertex[i].getU(texCoord[0]);
		vertex[i].getV(texCoord[1]);
	}
}

// ***************************************************************************

void CMeshMRMSkinnedGeom::getSkinWeights (std::vector<CMesh::CSkinWeight> &skinWeights) const
{
	const uint vertexCount = _VBufferFinal.getNumVertices();
	skinWeights.resize (vertexCount);
	const CPackedVertexBuffer::CPackedVertex *vertices = _VBufferFinal.getPackedVertices();
	uint i;
	for (i=0; i<vertexCount; i++)
	{
		const CPackedVertexBuffer::CPackedVertex &vertex = vertices[i];

		uint j;
		// Matrices
		for (j=0; j<NL3D_MESH_MRM_SKINNED_MAX_MATRIX; j++)
		{
			skinWeights[i].MatrixId[j] = vertex.Matrices[j];
			vertex.getWeight (skinWeights[i].Weights[j], j);
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
uint	CMeshMRMSkinnedGeom::NumCacheVertexNormal1= NL_BlockByteL1 / sizeof(CRawVertexNormalSkinned1);
// Number of vertices per block to process with 2 matrix.
uint	CMeshMRMSkinnedGeom::NumCacheVertexNormal2= NL_BlockByteL1 / sizeof(CRawVertexNormalSkinned2);
// Number of vertices per block to process with 3 matrix.
uint	CMeshMRMSkinnedGeom::NumCacheVertexNormal3= NL_BlockByteL1 / sizeof(CRawVertexNormalSkinned3);
// Number of vertices per block to process with 4 matrix.
uint	CMeshMRMSkinnedGeom::NumCacheVertexNormal4= NL_BlockByteL1 / sizeof(CRawVertexNormalSkinned4);


/* Old School template: include the same file with define switching,
	Was used before to reuse same code for and without SSE.
	Useless now because SSE removed, but keep it for possible future work on it.
*/
#define ADD_MESH_MRM_SKINNED_TEMPLATE
#include "mesh_mrm_skinned_template.cpp"


} // NL3D






















