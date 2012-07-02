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

#include "nel/3d/mesh.h"
#include "nel/3d/mesh_instance.h"
#include "nel/3d/scene.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/mesh_morpher.h"
#include "nel/misc/bsphere.h"
#include "nel/3d/stripifier.h"
#include "nel/misc/fast_floor.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/mesh_blender.h"
#include "nel/3d/matrix_3x4.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/visual_collision_mesh.h"
#include "nel/3d/meshvp_wind_tree.h"

using namespace std;
using namespace NLMISC;




namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// MeshGeom Tools.
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
sint	CMeshGeom::CCornerTmp::Flags=0;


// ***************************************************************************
bool	CMeshGeom::CCornerTmp::operator<(const CCornerTmp &c) const
{
	sint	i;

	// Vert first.
	if(Vertex!=c.Vertex)
		return Vertex<c.Vertex;

	// Order: normal, uvs, color0, color1, skinning.
	if((CCornerTmp::Flags & CVertexBuffer::NormalFlag) && Normal!=c.Normal)
		return Normal<c.Normal;
	for(i=0; i<CVertexBuffer::MaxStage; i++)
	{
		if((CCornerTmp::Flags & (CVertexBuffer::TexCoord0Flag<<i)) && Uvws[i]!=c.Uvws[i])
			return Uvws[i]<c.Uvws[i];
	}
	if((CCornerTmp::Flags & CVertexBuffer::PrimaryColorFlag) && Color!=c.Color)
		return Color<c.Color;
	if((CCornerTmp::Flags & CVertexBuffer::SecondaryColorFlag) && Specular!=c.Specular)
		return Specular<c.Specular;

	if ((CCornerTmp::Flags & CVertexBuffer::PaletteSkinFlag)==CVertexBuffer::PaletteSkinFlag)
	{
		for(i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
		{
			if(Palette.MatrixId[i] != c.Palette.MatrixId[i])
				return Palette.MatrixId[i] < c.Palette.MatrixId[i];
			if(Weights[i] != c.Weights[i])
				return Weights[i] < c.Weights[i];
		}
	}


	// All are equal!!
	return false;
}


// ***************************************************************************
// ***************************************************************************
// CMeshGeom.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CMeshGeom::CMeshGeom()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	_Skinned= false;
	_OriginalSkinRestored= true;
	_MeshMorpher = new CMeshMorpher;
	_BoneIdComputed = false;
	_BoneIdExtended= false;
	_PreciseClipping= false;
}


// ***************************************************************************
CMeshGeom::~CMeshGeom()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	delete _MeshMorpher;
}


// ***************************************************************************
void	CMeshGeom::optimizeTriangleOrder()
{
	CStripifier		stripifier;

	// for all rdrpass of all matrix blocks.
	for(uint mb= 0;mb<_MatrixBlocks.size();mb++)
	{
		for(uint  rp=0; rp<_MatrixBlocks[mb].RdrPass.size(); rp++ )
		{
			// stripify list of triangles of this pass.
			CRdrPass	&pass= _MatrixBlocks[mb].RdrPass[rp];
			stripifier.optimizeTriangles(pass.PBlock, pass.PBlock);
		}
	}

}


// ***************************************************************************
void	CMeshGeom::build (CMesh::CMeshBuild &m, uint numMaxMaterial)
{
	sint	i;

	// Empty geometry?
	if(m.Vertices.size()==0 || m.Faces.size()==0)
	{
		_VBuffer.setNumVertices(0);
		_VBuffer.setName("CMeshGeom");
		_VBuffer.reserve(0);
		_MatrixBlocks.clear();
		_BBox.setCenter(CVector::Null);
		_BBox.setSize(CVector::Null);
		return;
	}
	nlassert(numMaxMaterial>0);

	// Copy the UV routing table
	for (i=0; i<CVertexBuffer::MaxStage; i++)
		_VBuffer.setUVRouting (i, m.UVRouting[i]);

	/// 0. First, make bbox.
	//======================
	_BBox= makeBBox(m.Vertices);


	/// 1. If skinning, group by matrix Block the vertices.
	//================================================

	// First, copy Face array.
	vector<CFaceTmp>	tmpFaces;
	tmpFaces.resize(m.Faces.size());
	for(i=0;i<(sint)tmpFaces.size();i++)
		tmpFaces[i]= m.Faces[i];

	_Skinned= ((m.VertexFlags & CVertexBuffer::PaletteSkinFlag)==CVertexBuffer::PaletteSkinFlag);
	// Skinning is OK only if SkinWeights are of same size as vertices.
	_Skinned= _Skinned && (m.Vertices.size()==m.SkinWeights.size());

	// If skinning is KO, remove the Skin option.
	uint	vbFlags= m.VertexFlags;
	if(!_Skinned)
		vbFlags&= ~CVertexBuffer::PaletteSkinFlag;
	// Force presence of vertex.
	vbFlags|= CVertexBuffer::PositionFlag;


	// If the mesh is not skinned, we have just 1 _MatrixBlocks.
	if(!_Skinned)
	{
		_MatrixBlocks.resize(1);
		// For each faces, assign it to the matrix block 0.
		for(i=0;i<(sint)tmpFaces.size();i++)
			tmpFaces[i].MatrixBlockId= 0;
	}
	// Else We must group/compute the matrixs blocks.
	else
	{
		// reset matrix blocks.
		_MatrixBlocks.clear();
		// build matrix blocks, and link faces to good matrix blocks.
		buildSkin(m, tmpFaces);
	}

	/// 2. Then, for all faces, resolve continuities, building VBuffer.
	//================================================
	// Setup VB.
	_VBuffer.setNumVertices(0);
	_VBuffer.reserve(0);

	bool useFormatExt = false;
	/** If all texture coordinates are of dimension 2, we can setup the flags as before.
	  * If this isn't the case, we must setup a custom format
	  */
	for (uint k = 0; k < CVertexBuffer::MaxStage; ++k)
	{
		if (
			(vbFlags & (CVertexBuffer::TexCoord0Flag << k))
			&& m.NumCoords[k] != 2)
		{
			useFormatExt = true;
			break;
		}
	}

	if (!useFormatExt)
	{
		// setup standard format
		_VBuffer.setVertexFormat(vbFlags);
	}
	else // setup extended format
	{
		_VBuffer.clearValueEx();
		if (vbFlags & CVertexBuffer::PositionFlag) _VBuffer.addValueEx(CVertexBuffer::Position, CVertexBuffer::Float3);
		if (vbFlags & CVertexBuffer::NormalFlag) _VBuffer.addValueEx(CVertexBuffer::Normal, CVertexBuffer::Float3);
		if (vbFlags & CVertexBuffer::PrimaryColorFlag) _VBuffer.addValueEx(CVertexBuffer::PrimaryColor, CVertexBuffer::UChar4);
		if (vbFlags & CVertexBuffer::SecondaryColorFlag) _VBuffer.addValueEx(CVertexBuffer::SecondaryColor, CVertexBuffer::UChar4);
		if (vbFlags & CVertexBuffer::WeightFlag) _VBuffer.addValueEx(CVertexBuffer::Weight, CVertexBuffer::Float4);
		if (vbFlags & CVertexBuffer::PaletteSkinFlag) _VBuffer.addValueEx(CVertexBuffer::PaletteSkin, CVertexBuffer::UChar4);
		if (vbFlags & CVertexBuffer::FogFlag) _VBuffer.addValueEx(CVertexBuffer::Fog, CVertexBuffer::Float1);

		for (uint k = 0; k < CVertexBuffer::MaxStage; ++k)
		{
			if (vbFlags & (CVertexBuffer::TexCoord0Flag << k))
			{
				switch(m.NumCoords[k])
				{
					case 2:
						_VBuffer.addValueEx((CVertexBuffer::TValue) (CVertexBuffer::TexCoord0 + k), CVertexBuffer::Float2);
					break;
					case 3:
						_VBuffer.addValueEx((CVertexBuffer::TValue) (CVertexBuffer::TexCoord0 + k), CVertexBuffer::Float3);
					break;
					default:
						nlassert(0);
					break;
				}
			}
		}
		_VBuffer.initEx();
	}

	// Set local flags for corner comparison.
	CCornerTmp::Flags= vbFlags;
	// Setup locals.
	TCornerSet	corners;
	const CFaceTmp		*pFace= &(*tmpFaces.begin());
	uint32		nFaceMB = 0;
	sint		N= (sint)tmpFaces.size();
	sint		currentVBIndex=0;

	m.VertLink.clear ();

	// process each face, building up the VB.
	for(;N>0;N--, pFace++)
	{
		sint	v0= pFace->Corner[0].Vertex;
		sint	v1= pFace->Corner[1].Vertex;
		sint	v2= pFace->Corner[2].Vertex;
		findVBId(corners, &pFace->Corner[0], currentVBIndex, m.Vertices[v0], m);
		findVBId(corners, &pFace->Corner[1], currentVBIndex, m.Vertices[v1], m);
		findVBId(corners, &pFace->Corner[2], currentVBIndex, m.Vertices[v2], m);
		CMesh::CVertLink vl1(nFaceMB, 0, pFace->Corner[0].VBId);
		CMesh::CVertLink vl2(nFaceMB, 1, pFace->Corner[1].VBId);
		CMesh::CVertLink vl3(nFaceMB, 2, pFace->Corner[2].VBId);
		m.VertLink.push_back(vl1);
		m.VertLink.push_back(vl2);
		m.VertLink.push_back(vl3);
		++nFaceMB;
	}


	/// 3. build the RdrPass material.
	//================================
	uint	mb;

	// For each _MatrixBlocks, point on those materials.
	for(mb= 0;mb<_MatrixBlocks.size();mb++)
	{
		// Build RdrPass ids.
		_MatrixBlocks[mb].RdrPass.resize (numMaxMaterial);

		for(i=0;i<(sint)_MatrixBlocks[mb].RdrPass.size(); i++)
		{
			_MatrixBlocks[mb].RdrPass[i].MaterialId= i;
			// for build, force 32 bit indices
			_MatrixBlocks[mb].RdrPass[i].PBlock.setFormat(CIndexBuffer::Indices32);
		}
	}


	/// 4. Then, for all faces, build the RdrPass PBlock.
	//===================================================
	pFace= &(*tmpFaces.begin());
	N= (sint)tmpFaces.size();
	for(;N>0;N--, pFace++)
	{
		sint	mbId= pFace->MatrixBlockId;
		nlassert(mbId>=0 && mbId<(sint)_MatrixBlocks.size());
		// Insert the face in good MatrixBlock/RdrPass.
		CIndexBuffer &ib = _MatrixBlocks[mbId].RdrPass[pFace->MaterialId].PBlock;
		uint index = ib.getNumIndexes();
		ib.setNumIndexes (index+3);
		CIndexBufferReadWrite iba;
		ib.lock (iba);
		iba.setTri(index, pFace->Corner[0].VBId, pFace->Corner[1].VBId, pFace->Corner[2].VBId);
	}


	/// 5. Remove empty RdrPasses.
	//============================
	for(mb= 0;mb<_MatrixBlocks.size();mb++)
	{
		// NB: slow process (erase from a vector). Doens't matter since made at build.
		vector<CRdrPass>::iterator	itPass;
		for( itPass=_MatrixBlocks[mb].RdrPass.begin(); itPass!=_MatrixBlocks[mb].RdrPass.end(); )
		{
			// If this pass is empty, remove it.
			if( itPass->PBlock.getNumIndexes()==0 )
				itPass= _MatrixBlocks[mb].RdrPass.erase(itPass);
			else
				itPass++;
		}
	}

	/// 6. Misc.
	//============================
	// BShapes
	this->_MeshMorpher->BlendShapes = m.BlendShapes;

	// sort triangles for better cache use.
	optimizeTriangleOrder();

	// SmartPtr Copy VertexProgram effect.
	this->_MeshVertexProgram= m.MeshVertexProgram;

	/// 7. Compact bones id and build bones name array.
	//=================================================

	// If skinned
	if(_Skinned)
	{
		// Reserve some space
		_BonesName.reserve (m.BonesNames.size ());

		// Current local bone
		uint currentBone = 0;

		// For each matrix block
		uint matrixBlock;
		for (matrixBlock=0; matrixBlock<_MatrixBlocks.size(); matrixBlock++)
		{
			// Ref on the matrix block
			CMatrixBlock &mb = _MatrixBlocks[matrixBlock];

			// Remap the skeleton index in model index
			std::map<uint, uint> remap;

			// For each matrix
			uint matrix;
			for (matrix=0; matrix<mb.NumMatrix; matrix++)
			{
				// Get bone id in the skeleton
				std::map<uint, uint>::iterator ite = remap.find (mb.MatrixId[matrix]);

				// Not found
				if (ite == remap.end())
				{
					// Insert it
					remap.insert (std::map<uint, uint>::value_type (mb.MatrixId[matrix], currentBone));

					// Check the matrix id
					nlassert (mb.MatrixId[matrix] < m.BonesNames.size());

					// Set the bone name
					_BonesName.push_back (m.BonesNames[mb.MatrixId[matrix]]);

					// Set the id in local
					mb.MatrixId[matrix] = currentBone++;
				}
				else
				{
					// Set the id in local
					mb.MatrixId[matrix] = ite->second;
				}
			}
		}

		// Bone id in local
		_BoneIdComputed = false;
		_BoneIdExtended = false;
	}

	// Set the vertex buffer preferred memory
	bool avoidVBHard= _Skinned || ( _MeshMorpher && _MeshMorpher->BlendShapes.size()>0 );
	_VBuffer.setPreferredMemory (avoidVBHard?CVertexBuffer::RAMPreferred:CVertexBuffer::StaticPreferred, false);

	// End!!
	// Some runtime not serialized compilation
	compileRunTime();
}

// ***************************************************************************
void CMeshGeom::setBlendShapes(std::vector<CBlendShape>&bs)
{
	_MeshMorpher->BlendShapes = bs;
	// must update some RunTime parameters
	compileRunTime();
}


// ***************************************************************************
void	CMeshGeom::applyMaterialRemap(const std::vector<sint> &remap)
{
	for(uint mb=0;mb<getNbMatrixBlock();mb++)
	{
		for(uint rp=0;rp<getNbRdrPass(mb);rp++)
		{
			// remap
			uint32	&matId= _MatrixBlocks[mb].RdrPass[rp].MaterialId;
			nlassert(remap[matId]>=0);
			matId= remap[matId];
		}
	}
}


// ***************************************************************************
void	CMeshGeom::initInstance(CMeshBaseInstance *mbi)
{
	// init the instance with _MeshVertexProgram infos
	if(_MeshVertexProgram)
		_MeshVertexProgram->initInstance(mbi);
}

// ***************************************************************************
bool	CMeshGeom::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
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
void	CMeshGeom::render(IDriver *drv, CTransformShape *trans, float polygonCount, uint32 rdrFlags, float globalAlpha)
{
	nlassert(drv);
	// get the mesh instance.
	CMeshBaseInstance	*mi= safe_cast<CMeshBaseInstance*>(trans);
	// get a ptr on scene
	CScene			*ownerScene= mi->getOwnerScene();
	// get a ptr on renderTrav
	CRenderTrav		*renderTrav= &ownerScene->getRenderTrav();

	// Soft vb if not supported by the driver
	if (drv->slowUnlockVertexBufferHard())
		_VBuffer.setPreferredMemory (CVertexBuffer::RAMPreferred, false);

	// get the skeleton model to which I am binded (else NULL).
	CSkeletonModel		*skeleton;
	skeleton= mi->getSkeletonModel();
	// The mesh must not be skinned for render()
	nlassert(!(_Skinned && mi->isSkinned() && skeleton));
	bool bMorphApplied = _MeshMorpher->BlendShapes.size() > 0;
	bool useTangentSpace = _MeshVertexProgram && _MeshVertexProgram->needTangentSpace();


	// Profiling
	//===========
	H_AUTO( NL3D_MeshGeom_RenderNormal );


	// Morphing
	// ========
	if (bMorphApplied)
	{
		// If _Skinned (NB: the skin is not applied) and if lod.OriginalSkinRestored, then restoreOriginalSkinPart is
		// not called but mush morpher write changed vertices into VBHard so its ok. The unchanged vertices
		// are written in the preceding call to restoreOriginalSkinPart.
		if (_Skinned)
		{
			_MeshMorpher->initSkinned(&_VBufferOri,
								 &_VBuffer,
								 useTangentSpace,
								 &_OriginalSkinVertices,
								 &_OriginalSkinNormals,
								 useTangentSpace ? &_OriginalTGSpace : NULL,
								 false );
			_MeshMorpher->updateSkinned (mi->getBlendShapeFactors());
		}
		else // Not even skinned so we have to do all the stuff
		{
			_MeshMorpher->init(&_VBufferOri,
								 &_VBuffer,
								 useTangentSpace);
			_MeshMorpher->update (mi->getBlendShapeFactors());
		}
	}


	// Skinning
	// ========

	// else setup instance matrix
	drv->setupModelMatrix(trans->getWorldMatrix());


	// since instance skin is invalid but mesh is skinned , we must copy vertices/normals from original vertices.
	if (_Skinned)
	{
		// do it for this Lod only, and if cache say it is necessary.
		if (!_OriginalSkinRestored)
			restoreOriginalSkinVertices();
	}


	// Setup meshVertexProgram
	//===========

	// use MeshVertexProgram effect?
	bool	useMeshVP= _MeshVertexProgram != NULL;
	if( useMeshVP )
	{
		CMatrix		invertedObjectMatrix;
		invertedObjectMatrix = trans->getWorldMatrix().inverted();
		// really ok if success to begin VP
		useMeshVP= _MeshVertexProgram->begin(drv, ownerScene, mi, invertedObjectMatrix, renderTrav->CamPos);
		if (!useMeshVP && !mi->_VPWindTreeFixed)
		{
			if (dynamic_cast<CMeshVPWindTree *>(&(*_MeshVertexProgram)))
			{
				// fix for mesh tree v.p : all material should be lighted
				for(uint mb=0;mb<_MatrixBlocks.size();mb++)
				{
					CMatrixBlock	&mBlock= _MatrixBlocks[mb];
					for(uint i=0;i<mBlock.RdrPass.size();i++)
					{
						CMaterial &mat=mi->Materials[mBlock.RdrPass[i].MaterialId];
						mat.setLighting(true, mat.getEmissive(), mat.getAmbient(), mat.getDiffuse(), mat.getSpecular());
					}
				}
			}
			mi->_VPWindTreeFixed = true;
		}
	}



	// Render the mesh.
	//===========
	// active VB.
	drv->activeVertexBuffer(_VBuffer);


	// Global alpha used ?
	uint32	globalAlphaUsed= rdrFlags & IMeshGeom::RenderGlobalAlpha;
	uint8	globalAlphaInt=(uint8)NLMISC::OptFastFloor(globalAlpha*255);


	// For all _MatrixBlocks
	for(uint mb=0;mb<_MatrixBlocks.size();mb++)
	{
		CMatrixBlock	&mBlock= _MatrixBlocks[mb];
		if(mBlock.RdrPass.size()==0)
			continue;

		// Global alpha ?
		if (globalAlphaUsed)
		{
			bool	gaDisableZWrite= (rdrFlags & IMeshGeom::RenderGADisableZWrite)?true:false;

			// Render all pass.
			for (uint i=0;i<mBlock.RdrPass.size();i++)
			{
				CRdrPass	&rdrPass= mBlock.RdrPass[i];
				// Render with the Materials of the MeshInstance.
				if ( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (rdrFlags & IMeshGeom::RenderOpaqueMaterial) ) ||
					 ( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (rdrFlags & IMeshGeom::RenderTransparentMaterial) ) )
				{
					// CMaterial Ref
					CMaterial &material=mi->Materials[rdrPass.MaterialId];

					// Use a MeshBlender to modify material and driver.
					CMeshBlender	blender;
					blender.prepareRenderForGlobalAlpha(material, drv, globalAlpha, globalAlphaInt, gaDisableZWrite);

					// Setup VP material
					if (useMeshVP)
					{
						_MeshVertexProgram->setupForMaterial(material, drv, ownerScene, &_VBuffer);
					}

					// Render
					drv->activeIndexBuffer(rdrPass.PBlock);
					drv->renderTriangles(material, 0, rdrPass.PBlock.getNumIndexes()/3);

					// Resetup material/driver
					blender.restoreRender(material, drv, gaDisableZWrite);
				}
			}
		}
		else
		{
			// Render all pass.
			for(uint i=0;i<mBlock.RdrPass.size();i++)
			{
				CRdrPass	&rdrPass= mBlock.RdrPass[i];
				// Render with the Materials of the MeshInstance.
				if( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (rdrFlags & IMeshGeom::RenderOpaqueMaterial) ) ||
					( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (rdrFlags & IMeshGeom::RenderTransparentMaterial) )		)
				{
					// CMaterial Ref
					CMaterial &material=mi->Materials[rdrPass.MaterialId];

					// Setup VP material
					if (useMeshVP)
					{
						_MeshVertexProgram->setupForMaterial(material, drv, ownerScene, &_VBuffer);
					}

					// render primitives
					drv->activeIndexBuffer(rdrPass.PBlock);
					drv->renderTriangles(material, 0, rdrPass.PBlock.getNumIndexes()/3);
				}
			}
		}
	}

	// End VertexProgram effect
	if(useMeshVP)
	{
		// Apply it.
		_MeshVertexProgram->end(drv);
	}
}


// ***************************************************************************
void	CMeshGeom::renderSkin(CTransformShape *trans, float alphaMRM)
{
	// get the mesh instance.
	CMeshBaseInstance	*mi= safe_cast<CMeshBaseInstance*>(trans);
	// get a ptr on scene
	CScene			*ownerScene= mi->getOwnerScene();
	// get a ptr on renderTrav
	CRenderTrav		*renderTrav= &ownerScene->getRenderTrav();
	// get a ptr on the driver
	IDriver			*drv= renderTrav->getDriver();
	nlassert(drv);

	// get the skeleton model to which I am binded (else NULL).
	CSkeletonModel		*skeleton;
	skeleton= mi->getSkeletonModel();
	// must be skinned for renderSkin()
	nlassert(_Skinned && mi->isSkinned() && skeleton);
	bool bMorphApplied = _MeshMorpher->BlendShapes.size() > 0;
	bool useTangentSpace = _MeshVertexProgram && _MeshVertexProgram->needTangentSpace();


	// Profiling
	//===========
	H_AUTO( NL3D_MeshGeom_RenderSkinned );


	// Morphing
	// ========
	if (bMorphApplied)
	{
		// Since Skinned we must update original skin vertices and normals because skinning use it
		_MeshMorpher->initSkinned(&_VBufferOri,
							 &_VBuffer,
							 useTangentSpace,
							 &_OriginalSkinVertices,
							 &_OriginalSkinNormals,
							 useTangentSpace ? &_OriginalTGSpace : NULL,
							 true );
		_MeshMorpher->updateSkinned (mi->getBlendShapeFactors());
	}


	// Skinning
	// ========

	// NB: the skeleton matrix has already been setuped by CSkeletonModel
	// NB: the normalize flag has already been setuped by CSkeletonModel


	// apply the skinning: _VBuffer is modified.
	applySkin(skeleton);


	// Setup meshVertexProgram
	//===========

	// use MeshVertexProgram effect?
	bool	useMeshVP= _MeshVertexProgram != NULL;
	if( useMeshVP )
	{
		CMatrix		invertedObjectMatrix;
		invertedObjectMatrix = skeleton->getWorldMatrix().inverted();
		// really ok if success to begin VP
		useMeshVP= _MeshVertexProgram->begin(drv, ownerScene, mi, invertedObjectMatrix, renderTrav->CamPos);
	}


	// Render the mesh.
	//===========
	// active VB.
	drv->activeVertexBuffer(_VBuffer);


	// For all _MatrixBlocks
	for(uint mb=0;mb<_MatrixBlocks.size();mb++)
	{
		CMatrixBlock	&mBlock= _MatrixBlocks[mb];
		if(mBlock.RdrPass.size()==0)
			continue;

		// Render all pass.
		for(uint i=0;i<mBlock.RdrPass.size();i++)
		{
			CRdrPass	&rdrPass= mBlock.RdrPass[i];

			// CMaterial Ref
			CMaterial &material=mi->Materials[rdrPass.MaterialId];

			// Setup VP material
			if (useMeshVP)
			{
				_MeshVertexProgram->setupForMaterial(material, drv, ownerScene, &_VBuffer);
			}

			// render primitives
			drv->activeIndexBuffer(rdrPass.PBlock);
			drv->renderTriangles(material, 0, rdrPass.PBlock.getNumIndexes()/3);
		}
	}

	// End VertexProgram effect
	if(useMeshVP)
	{
		// Apply it.
		_MeshVertexProgram->end(drv);
	}

}


// ***************************************************************************
void	CMeshGeom::renderSimpleWithMaterial(IDriver *drv, const CMatrix &worldMatrix, CMaterial &mat)
{
	H_AUTO( NL3D_MeshGeom_RenderSimpleWithMaterial );

	nlassert(drv);

	// setup matrix
	drv->setupModelMatrix(worldMatrix);

	// Active simple VB.
	drv->activeVertexBuffer(_VBuffer);

	// For all _MatrixBlocks
	for(uint mb=0;mb<_MatrixBlocks.size();mb++)
	{
		CMatrixBlock	&mBlock= _MatrixBlocks[mb];
		if(mBlock.RdrPass.size()==0)
			continue;

		// Render all pass.
		for(uint i=0;i<mBlock.RdrPass.size();i++)
		{
			CRdrPass	&rdrPass= mBlock.RdrPass[i];

			// render primitives
			drv->activeIndexBuffer(rdrPass.PBlock);
			drv->renderTriangles(mat, 0, rdrPass.PBlock.getNumIndexes()/3);
		}
	}

}


// ***************************************************************************
void	CMeshGeom::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/*
	Version 5:
		- Preferred memory.
	Version 4:
		- BonesName.
	Version 3:
		- MeshVertexProgram.
	Version 2:
		- precompute of triangle order. (nothing more to load).
	Version 1:
		- added blend shapes
	Version 0:
		- separate serialisation CMesh / CMeshGeom.
	*/
	sint ver = f.serialVersion (4);


	// must have good original Skinned Vertex before writing.
	if( !f.isReading() && _Skinned && !_OriginalSkinRestored )
	{
		restoreOriginalSkinVertices();
	}


	// Version 4+: Array of bone name
	if (ver >= 4)
	{
		f.serialCont (_BonesName);
	}

	if (f.isReading())
	{
		// Version3-: Bones index are in skeleton model id list
		_BoneIdComputed = (ver < 4);
		// In all case, must recompute usage of parents.
		_BoneIdExtended= false;
	}
	else
	{
		// Warning, if you have skinned this shape, you can't write it anymore because skinning id have been changed!
		nlassert (_BoneIdComputed==false);
	}

	// Version3+: MeshVertexProgram.
	if (ver >= 3)
	{
		IMeshVertexProgram	*mvp= NULL;
		if(f.isReading())
		{
			f.serialPolyPtr(mvp);
			_MeshVertexProgram= mvp;
		}
		else
		{
			mvp= _MeshVertexProgram;
			f.serialPolyPtr(mvp);
		}
	}
	else if(f.isReading())
	{
		// release vp
		_MeshVertexProgram= NULL;
	}

	// TestYoyo
	//_MeshVertexProgram= NULL;

	// Version1+: _MeshMorpher.
	if (ver >= 1)
		f.serial (*_MeshMorpher);

	// serial geometry.
	f.serial (_VBuffer);

	f.serialCont (_MatrixBlocks);
	f.serial (_BBox);
	f.serial (_Skinned);


	// If _VertexBuffer changed, flag the VertexBufferHard.
	if(f.isReading())
	{
		// if >= version 2, reorder of triangles is precomputed, else compute it now.
		if(ver < 2 )
		{
			optimizeTriangleOrder();
		}
	}

	// Skinning: If Version < 4, _BonesName are not present, must compute _BonesId from localId
	// Else it is computed at first computeBonesId().
	if(ver < 4)
		buildBoneUsageVer3();

	// TestYoyo
	//_MeshVertexProgram= NULL;
	/*{
		uint numTris= 0;
		for(uint i=0;i<_MatrixBlocks.size();i++)
		{
			for(uint j=0;j<_MatrixBlocks[i].RdrPass.size();j++)
				numTris+= _MatrixBlocks[i].RdrPass[j].PBlock.getNumTri();
		}
		nlinfo("YOYO: %d Vertices. %d Triangles.", _VBuffer.getNumVertices(), numTris);
	}*/

	// Some runtime not serialized compilation
	if(f.isReading())
		compileRunTime();
}


// ***************************************************************************
void	CMeshGeom::compileRunTime()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// if skinned, prepare skinning
	if(_Skinned)
	{
		// bkup vertices
		bkupOriginalSkinVertices();
		// build the shadow skin
		buildShadowSkin();
	}

	// Do precise clipping for big object??
	_PreciseClipping= _BBox.getRadius() >= NL3D_MESH_PRECISE_CLIP_THRESHOLD;

	// Support MeshBlockRendering only if not skinned/meshMorphed.
	bool	supportMeshBlockRendering= !_Skinned && _MeshMorpher->BlendShapes.size()==0;

	// true only if one matrix block, and at least one rdrPass.
	supportMeshBlockRendering= supportMeshBlockRendering && _MatrixBlocks.size()==1 && _MatrixBlocks[0].RdrPass.size()>0;
	if (supportMeshBlockRendering && _MeshVertexProgram)
	{
		supportMeshBlockRendering = supportMeshBlockRendering && _MeshVertexProgram->supportMeshBlockRendering();
	}

	// TestYoyo
	//supportMeshBlockRendering= false;

	// support MeshVertexProgram, but no material sorting...
	bool	supportMBRPerMaterial= supportMeshBlockRendering && _MeshVertexProgram==NULL;


	// setup flags
	_SupportMBRFlags= 0;
	if(supportMeshBlockRendering)
		_SupportMBRFlags|= MBROk;
	if(supportMBRPerMaterial)
		_SupportMBRFlags|= MBRSortPerMaterial;

	bool avoidVBHard= _Skinned || ( _MeshMorpher && _MeshMorpher->BlendShapes.size()>0 );
	_VBuffer.setPreferredMemory (avoidVBHard?CVertexBuffer::RAMPreferred:CVertexBuffer::StaticPreferred, false);
}

// ***************************************************************************
bool	CMeshGeom::retrieveVertices(std::vector<NLMISC::CVector> &vertices) const
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	uint	i;

	// if resident, fails!!! cannot read!
	const CVertexBuffer	&vb= getVertexBuffer();
	if(vb.isResident())
		return false;

	vertices.clear();
	vertices.resize(vb.getNumVertices());
	{
		CVertexBufferRead vba;
		vb.lock (vba);
		const uint8	*pVert= (const uint8*)vba.getVertexCoordPointer(0);
		uint		vSize= vb.getVertexSize();
		for(i=0;i<vertices.size();i++)
		{
			vertices[i]= *(const CVector*)pVert;
			pVert+= vSize;
		}
	}

	return true;
}

// ***************************************************************************
bool	CMeshGeom::retrieveTriangles(std::vector<uint32> &indices) const
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	uint	i;

	indices.clear();

	// count numTris
	uint	numTris= 0;
	for(i=0;i<getNbMatrixBlock();i++)
	{
		for(uint rp=0;rp<getNbRdrPass(i);rp++)
		{
			// if resident, fails!!! cannot read!
			const CIndexBuffer	&pb= getRdrPassPrimitiveBlock(i, rp);
			if(pb.isResident())
				return false;
			numTris+= getRdrPassPrimitiveBlock(i, rp).getNumIndexes()/3;
		}
	}
	indices.resize(numTris*3);

	// build indices
	uint	triIdx= 0;
	for(i=0;i<getNbMatrixBlock();i++)
	{
		for(uint rp=0;rp<getNbRdrPass(i);rp++)
		{
			const CIndexBuffer	&pb= getRdrPassPrimitiveBlock(i, rp);
			CIndexBufferRead iba;
			pb.lock (iba);
			// copy
			if (pb.getFormat() == CIndexBuffer::Indices32)
			{
				memcpy(&indices[triIdx*3], iba.getPtr(), pb.getNumIndexes()*sizeof(uint32));
			}
			else
			{
				// std::copy will convert from 16 bits index to 32 bit index
				std::copy((uint16 *) iba.getPtr(), ((uint16 *) iba.getPtr()) +  pb.getNumIndexes(), &indices[triIdx*3]);
			}
			// next
			triIdx+= pb.getNumIndexes()/3;
		}
	}

	return true;
}


// ***************************************************************************
// ***************************************************************************
// Skinning.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CMeshGeom::buildSkin(CMesh::CMeshBuild &m, std::vector<CFaceTmp>	&tmpFaces)
{
	sint	i,j,k;
	TBoneMap		remainingBones;
	list<uint>		remainingFaces;


	// 0. normalize SkinWeights: for all weights at 0, copy the matrixId from 0th matrix => no random/bad use of matrix.
	//================================
	for(i=0;i<(sint)m.SkinWeights.size();i++)
	{
		CMesh::CSkinWeight	&sw= m.SkinWeights[i];

		// 0th weight must not be 0.
		nlassert(sw.Weights[0]!=0);

		// Begin at 1, tests all other weights.
		for(j=1;j<NL3D_MESH_SKINNING_MAX_MATRIX;j++)
		{
			// We don't use this entry??
			if(sw.Weights[j]==0)
			{
				// Setup MatrixId so that this vertex do no use more matrix than it really wants.
				sw.MatrixId[j]= sw.MatrixId[0];
			}
		}
	}


	// 1. build the list of used/remaining bones, in ascending order. (so we use the depth-first topolgy of hierarchy).
	//================================
	for(i=0;i<(sint)tmpFaces.size();i++)
	{
		CFaceTmp	&face= tmpFaces[i];

		for(j=0;j<3;j++)
		{
			CMesh::CSkinWeight	&sw= m.SkinWeights[face.Corner[j].Vertex];
			for(k=0;k<NL3D_MESH_SKINNING_MAX_MATRIX;k++)
			{
				// insert (if not already here) the used bone in the set.
				// and insert his refcount. (NB: ctor() init it to 0 :) ).
				remainingBones[sw.MatrixId[k]].RefCount++;
			}
		}
	}


	// 2. Create the list of un-inserted faces.
	//================================
	for(i=0;i<(sint)tmpFaces.size();i++)
	{
		remainingFaces.push_back(i);
	}



	// 3. Create as many Blocks as necessary.
	//================================
	// Which bones a face use (up to 12).
	vector<uint>	boneUse;
	boneUse.reserve(NL3D_MESH_SKINNING_MAX_MATRIX*3);

	// While still exist faces.
	while(!remainingFaces.empty())
	{
		// create a new matrix block.
		_MatrixBlocks.push_back(CMatrixBlock());
		CMatrixBlock	&matrixBlock= _MatrixBlocks[_MatrixBlocks.size()-1];
		matrixBlock.NumMatrix=0;

		// a. reset remainingBones as not inserted in the current matrixBlock.
		//============================
		ItBoneMap	itBone;
		for(itBone= remainingBones.begin();itBone!=remainingBones.end();itBone++)
		{
			itBone->second.Inserted= false;
		}


		// b. while still exist bones, try to insert faces which use them in matrixBlock.
		//============================
		while(!remainingBones.empty())
		{
			// get the first bone from the map. (remind: depth-first order).
			uint		currentBoneId= remainingBones.begin()->first;

			// If no more faces in the remainingFace list use this bone, remove it, and continue.
			if(remainingBones.begin()->second.RefCount==0)
			{
				remainingBones.erase(remainingBones.begin());
				continue;
			}

			// this is a marker, to know if a face insertion will occurs.
			bool		faceAdded= false;

			// traverse all faces, trying to insert them in current MatrixBlock processed.
			list<uint>::iterator	itFace;
			for(itFace= remainingFaces.begin(); itFace!=remainingFaces.end();)
			{
				bool	useCurrentBoneId;
				uint	newBoneAdded;

				// i/ Get info on current face.
				//-----------------------------

				// build which bones this face use.
				tmpFaces[*itFace].buildBoneUse(boneUse, m.SkinWeights);

				// test if this face use the currentBoneId.
				useCurrentBoneId= false;
				for(i=0;i<(sint)boneUse.size();i++)
				{
					// if this face use the currentBoneId
					if(boneUse[i]==currentBoneId)
					{
						useCurrentBoneId= true;
						break;
					}
				}
				// compute how many bones that are not in the current matrixblock this face use.
				newBoneAdded=0;
				for(i=0;i<(sint)boneUse.size();i++)
				{
					// if this bone is not inserted in the current matrix block, inform it.
					if(!remainingBones[boneUse[i]].Inserted)
						newBoneAdded++;
				}


				// ii/ insert/reject face.
				//------------------------

				// If this face do not add any more bone, we can insert it into the current matrixblock.
				// If it use the currentBoneId, and do not explode max count, we allow insert it too in the current matrixblock.
				if( newBoneAdded==0 ||
					(useCurrentBoneId && newBoneAdded+matrixBlock.NumMatrix < IDriver::MaxModelMatrix) )
				{
					// Insert this face in the current matrix block

					CFaceTmp	&face= tmpFaces[*itFace];

					// for all vertices of this face.
					for(j=0;j<3;j++)
					{
						CMesh::CSkinWeight	&sw= m.SkinWeights[face.Corner[j].Vertex];

						// for each corner weight (4)
						for(k=0;k<NL3D_MESH_SKINNING_MAX_MATRIX;k++)
						{
							// get the global boneId this corner weight use.
							uint		boneId= sw.MatrixId[k];
							// get the CBoneTmp this corner weight use.
							CBoneTmp	&bone= remainingBones[boneId];

							// decRef the bone .
							bone.RefCount--;

							// Is this bone already inserted in the MatrixBlock ?
							if( !bone.Inserted )
							{
								// No, insert it.
								bone.Inserted= true;
								// link it to the MatrixId in the current matrixBlock.
								bone.MatrixIdInMB= matrixBlock.NumMatrix;

								// modify the matrixBlock
								matrixBlock.MatrixId[matrixBlock.NumMatrix]= boneId;
								// increment the number of matrix in the matrixBlock.
								matrixBlock.NumMatrix++;
							}

							// Copy Weight info for this Corner.
							// Set what matrix in the current matrix block this corner use.
							face.Corner[j].Palette.MatrixId[k]= bone.MatrixIdInMB;
							// Set weight.
							face.Corner[j].Weights[k]= sw.Weights[k];
						}
					}

					// to Which matrixblock this face is inserted.
					face.MatrixBlockId= (sint)_MatrixBlocks.size()-1;

					// remove the face from remain face list.
					itFace= remainingFaces.erase(itFace);

					// inform the algorithm that a face has been added.
					faceAdded= true;
				}
				else
				{
					// do not append this face to the current matrix block, skip to the next
					itFace++;
				}
			}

			// If no faces have been added during this pass, we are blocked, and either the MatrixBlock may be full,
			// or there is no more face. So quit this block and process a new one.
			if(!faceAdded)
				break;
		}

	}
	// NB: at the end of this loop, remainingBones may not be empty(), but all remainingBones should have RefCount==0.



	// 4. Re-order matrix use in MatrixBlocks, for minimum matrix change between MatrixBlocks.
	//================================
	vector<CMatrixBlockRemap>	blockRemaps;
	blockRemaps.resize(_MatrixBlocks.size());


	// For all MatrixBlocks > first, try to "mirror" bones from previous.
	for(i=1;i<(sint)_MatrixBlocks.size();i++)
	{
		CMatrixBlock		&mBlock= _MatrixBlocks[i];
		CMatrixBlock		&mPrevBlock= _MatrixBlocks[i-1];
		CMatrixBlockRemap	&remap= blockRemaps[i];

		// First bkup the bone ids in remap table.
		for(j=0;j<(sint)mBlock.NumMatrix;j++)
		{
			remap.Remap[j]= mBlock.MatrixId[j];
		}

		// For all ids of this blocks, try to mirror them.
		for(j=0;j<(sint)mBlock.NumMatrix;j++)
		{
			// get the location of this bone in the prev bone.
			sint	idLoc= mPrevBlock.getMatrixIdLocation(mBlock.MatrixId[j]);
			// If not found, or if bigger than current array, fails (cant be mirrored).
			// Or if already mirrored.
			if(idLoc==-1 || idLoc>=(sint)mBlock.NumMatrix || idLoc==j)
			{
				// next id.
				j++;
			}
			else
			{
				// puts me on my mirrored location. and swap with the current one at this mirrored location.
				swap(mBlock.MatrixId[j], mBlock.MatrixId[idLoc]);
				// mBlock.MatrixId[j] is now a candidate for mirror.
			}
		}

		// Then build the Remap table, to re-order faces matrixId which use this matrix block.
		for(j=0;j<(sint)mBlock.NumMatrix;j++)
		{
			// get the boneid which was at this position j before.
			uint	boneId= remap.Remap[j];
			// search his new position, and store the result in the remap table.
			remap.Remap[j]= mBlock.getMatrixIdLocation(boneId);
		}

		// NB: this matrixBlock is re-ordered. next matrixBlock use this state.
	}


	// For all faces/corners/weights, remap MatrixIds.
	for(i=0;i<(sint)tmpFaces.size();i++)
	{
		CFaceTmp	&face= tmpFaces[i];
		// do it but for matrixblock0.
		if(face.MatrixBlockId!=0)
		{
			CMatrixBlockRemap	&remap= blockRemaps[face.MatrixBlockId];
			// For all corners.
			for(j=0;j<3;j++)
			{
				for(k=0;k<NL3D_MESH_SKINNING_MAX_MATRIX;k++)
				{
					uint	oldId= face.Corner[j].Palette.MatrixId[k];
					face.Corner[j].Palette.MatrixId[k]= (uint8)remap.Remap[oldId];
				}
			}
		}
	}

}


// ***************************************************************************
void	CMeshGeom::CFaceTmp::buildBoneUse(vector<uint>	&boneUse, vector<CMesh::CSkinWeight> &skinWeights)
{
	boneUse.clear();

	// For the 3 corners of the face.
	for(sint i=0;i<3;i++)
	{
		// get the CSkinWeight of this vertex.
		CMesh::CSkinWeight	&sw= skinWeights[Corner[i].Vertex];

		// For all skin weights of this vertex,
		for(sint j=0;j<NL3D_MESH_SKINNING_MAX_MATRIX;j++)
		{
			uint	boneId= sw.MatrixId[j];
			// insert (if not in the array) this bone.
			if( find(boneUse.begin(), boneUse.end(), boneId)==boneUse.end() )
				boneUse.push_back(boneId);
		}
	}


}



// ***************************************************************************
sint	CMeshGeom::CMatrixBlock::getMatrixIdLocation(uint32 boneId) const
{
	for(uint i=0;i<NumMatrix;i++)
	{
		if(MatrixId[i]==boneId)
			return i;
	}

	// not found.
	return -1;
}


// ***************************************************************************
float	CMeshGeom::getNumTriangles (float distance)
{
	// Sum of triangles
	uint32 triCount=0;

	// For each matrix block
	uint mbCount=(uint)_MatrixBlocks.size();
	for (uint mb=0; mb<mbCount; mb++)
	{
		CMatrixBlock &block=_MatrixBlocks[mb];

		// Count of primitive block
		uint pCount=(uint)block.RdrPass.size();
		for (uint pb=0; pb<pCount; pb++)
		{
			// Ref on the primitive block
			CRdrPass &pass=block.RdrPass[pb];

			// Sum tri
			triCount+=pass.PBlock.getNumIndexes ()/3;
		}
	}
	return (float)triCount;
}


// ***************************************************************************
void	CMeshGeom::computeBonesId (CSkeletonModel *skeleton)
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


			// **** Remap matrix blocks
			for (uint matrixBlock=0; matrixBlock<_MatrixBlocks.size(); matrixBlock++)
			{
				// Ref on the matrix block
				CMatrixBlock &mb = _MatrixBlocks[matrixBlock];

				// For each matrix
				uint matrix;
				for (matrix=0; matrix<mb.NumMatrix; matrix++)
				{
					// Get bone id in the skeleton
					nlassert (mb.MatrixId[matrix]<remap.size());
					mb.MatrixId[matrix] = remap[mb.MatrixId[matrix]];
				}
			}

			// **** Remap ShadowSkin, and compute Bone Sphere
			// Prepare Sphere compute
			static std::vector<CAABBox>		boneBBoxes;
			static std::vector<bool>		boneBBEmpty;
			boneBBoxes.clear();
			boneBBEmpty.clear();
			boneBBoxes.resize(_BonesId.size());
			boneBBEmpty.resize(_BonesId.size(), true);

			// For simplicity, use the shadow skin info only, to compute bone sphere
			for(uint vert=0;vert<_ShadowSkin.Vertices.size();vert++)
			{
				CShadowVertex	&v= _ShadowSkin.Vertices[vert];
				// Check id
				uint	srcId= v.MatrixId;
				nlassert ( srcId < remap.size());
				// remap
				v.MatrixId= remap[srcId];

				// if the boneId is valid (ie found)
				if(_BonesId[srcId]>=0)
				{
					// transform the vertex pos in BoneSpace
					CVector		p= skeleton->Bones[_BonesId[srcId]].getBoneBase().InvBindPos * v.Vertex;
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
void	CMeshGeom::buildBoneUsageVer3 ()
{
	if(_Skinned)
	{
		// parse all matrixBlocks, couting MaxBoneId used.
		uint32	maxBoneId= 0;
		// For each matrix block
		uint matrixBlock;
		for (matrixBlock=0; matrixBlock<_MatrixBlocks.size(); matrixBlock++)
		{
			CMatrixBlock &mb = _MatrixBlocks[matrixBlock];
			// For each matrix
			for (uint matrix=0; matrix<mb.NumMatrix; matrix++)
			{
				maxBoneId= max(mb.MatrixId[matrix], maxBoneId);
			}
		}

		// alloc an array of maxBoneId+1, reset to 0.
		std::vector<uint8>		boneUsage;
		boneUsage.resize(maxBoneId+1, 0);

		// reparse all matrixBlocks, counting usage for each bone.
		for (matrixBlock=0; matrixBlock<_MatrixBlocks.size(); matrixBlock++)
		{
			CMatrixBlock &mb = _MatrixBlocks[matrixBlock];
			// For each matrix
			for (uint matrix=0; matrix<mb.NumMatrix; matrix++)
			{
				// mark this bone as used.
				boneUsage[mb.MatrixId[matrix]]= 1;
			}
		}

		// For each bone used
		_BonesId.clear();
		for(uint i=0; i<boneUsage.size();i++)
		{
			// if the bone is used by the mesh, add it to BoneId.
			if(boneUsage[i])
				_BonesId.push_back(i);
		}

		// Must also compute _BonesSphere
		// Cannot do it easily, deprecated data format => suppose radius 0 sphere
		CBSphere	sphere(CVector::Null, 0.f);
		_BonesSphere.clear();
		_BonesSphere.resize(_BonesId.size(), sphere);

	}
}


// ***************************************************************************
void	CMeshGeom::updateSkeletonUsage(CSkeletonModel *sm, bool increment)
{
	// For all Bones used by this mesh.
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
void	CMeshGeom::bkupOriginalSkinVertices()
{
	nlassert(_Skinned);

	// reset
	contReset(_OriginalSkinVertices);
	contReset(_OriginalSkinNormals);
	contReset(_OriginalTGSpace);

	// get num of vertices
	uint	numVertices= _VBuffer.getNumVertices();

	CVertexBufferRead vba;
	_VBuffer.lock (vba);

	// Copy VBuffer content into Original vertices normals.
	if(_VBuffer.getVertexFormat() & CVertexBuffer::PositionFlag)
	{
		// copy vertices from VBuffer. (NB: useless geomorphed vertices are still copied, but doesn't matter).
		_OriginalSkinVertices.resize(numVertices);
		for(uint i=0; i<numVertices;i++)
		{
			_OriginalSkinVertices[i]= *vba.getVertexCoordPointer(i);
		}
	}
	if(_VBuffer.getVertexFormat() & CVertexBuffer::NormalFlag)
	{
		// copy normals from VBuffer. (NB: useless geomorphed normals are still copied, but doesn't matter).
		_OriginalSkinNormals.resize(numVertices);
		for(uint i=0; i<numVertices;i++)
		{
			_OriginalSkinNormals[i]= *vba.getNormalCoordPointer(i);
		}
	}

	// is there tangent space added ?
	if (_MeshVertexProgram && _MeshVertexProgram->needTangentSpace())
	{
		// yes, backup it
		nlassert(_VBuffer.getNumTexCoordUsed() > 0);
		uint tgSpaceStage = _VBuffer.getNumTexCoordUsed() - 1;
		_OriginalTGSpace.resize(numVertices);
		for(uint i=0; i<numVertices;i++)
		{
			_OriginalTGSpace[i]= *(CVector*)vba.getTexCoordPointer(i, tgSpaceStage);
		}
	}
}


// ***************************************************************************
void	CMeshGeom::restoreOriginalSkinVertices()
{
	nlassert(_Skinned);

	// get num of vertices
	uint	numVertices= _VBuffer.getNumVertices();

	CVertexBufferReadWrite vba;
	_VBuffer.lock (vba);

	// Copy VBuffer content into Original vertices normals.
	if(_VBuffer.getVertexFormat() & CVertexBuffer::PositionFlag)
	{
		// copy vertices from VBuffer. (NB: useless geomorphed vertices are still copied, but doesn't matter).
		for(uint i=0; i<numVertices;i++)
		{
			*vba.getVertexCoordPointer(i)= _OriginalSkinVertices[i];
		}
	}
	if(_VBuffer.getVertexFormat() & CVertexBuffer::NormalFlag)
	{
		// copy normals from VBuffer. (NB: useless geomorphed normals are still copied, but doesn't matter).
		for(uint i=0; i<numVertices;i++)
		{
			*vba.getNormalCoordPointer(i)= _OriginalSkinNormals[i];
		}
	}
	if (_MeshVertexProgram && _MeshVertexProgram->needTangentSpace())
	{
		uint numTexCoords = _VBuffer.getNumTexCoordUsed();
		nlassert(numTexCoords >= 2);
		nlassert(_OriginalTGSpace.size() == numVertices);
		// copy tangent space vectors
		for(uint i = 0; i < numVertices; ++i)
		{
			*(CVector*)vba.getTexCoordPointer(i, numTexCoords - 1)= _OriginalTGSpace[i];
		}
	}

	// cleared
	_OriginalSkinRestored= true;
}


// ***************************************************************************
// Flags for software vertex skinning.
#define	NL3D_SOFTSKIN_VNEEDCOMPUTE	3
#define	NL3D_SOFTSKIN_VMUSTCOMPUTE	1
#define	NL3D_SOFTSKIN_VCOMPUTED		0
// 3 means "vertex may need compute".
// 1 means "Primitive say vertex must be computed".
// 0 means "vertex is computed".


// ***************************************************************************
void	CMeshGeom::applySkin(CSkeletonModel *skeleton)
{
	// init.
	//===================
	if(_OriginalSkinVertices.empty())
		return;

	// Use correct skinning
	TSkinType	skinType;
	if( _OriginalSkinNormals.empty() )
		skinType= SkinPosOnly;
	else if( _OriginalTGSpace.empty() )
		skinType= SkinWithNormal;
	else
		skinType= SkinWithTgSpace;

	// Get VB src/dst info/ptrs.
	uint	numVertices= (uint)_OriginalSkinVertices.size();
	uint	dstStride= _VBuffer.getVertexSize();
	// Get dst TgSpace.
	uint	tgSpaceStage = 0;
	if( skinType>= SkinWithTgSpace)
	{
		nlassert(_VBuffer.getNumTexCoordUsed() > 0);
		tgSpaceStage= _VBuffer.getNumTexCoordUsed() - 1;
	}

	// Mark all vertices flag to not computed.
	static	vector<uint8>	skinFlags;
	skinFlags.resize(numVertices);
	// reset all flags
	memset(&skinFlags[0], NL3D_SOFTSKIN_VNEEDCOMPUTE, numVertices );

	CVertexBufferRead vba;
	_VBuffer.lock (vba);

	// For all MatrixBlocks
	//===================
	for(uint mb= 0; mb<_MatrixBlocks.size();mb++)
	{
		// compute matrixes for this block.
		static	CMatrix3x4	matrixes[IDriver::MaxModelMatrix];
		computeSkinMatrixes(skeleton, matrixes, mb==0?NULL:&_MatrixBlocks[mb-1], _MatrixBlocks[mb]);

		// check what vertex to skin for this PB.
		flagSkinVerticesForMatrixBlock(&skinFlags[0], _MatrixBlocks[mb]);

		// Get VB src/dst ptrs.
		uint8		*pFlag= &skinFlags[0];
		CVector		*srcVector= &_OriginalSkinVertices[0];
		uint8		*srcPal= (uint8*)vba.getPaletteSkinPointer(0);
		uint8		*srcWgt= (uint8*)vba.getWeightPointer(0);
		uint8		*dstVector= (uint8*)vba.getVertexCoordPointer(0);
		// Normal.
		CVector		*srcNormal= NULL;
		uint8		*dstNormal= NULL;
		if(skinType>=SkinWithNormal)
		{
			srcNormal= &_OriginalSkinNormals[0];
			dstNormal= (uint8*)vba.getNormalCoordPointer(0);
		}
		// TgSpace.
		CVector		*srcTgSpace= NULL;
		uint8		*dstTgSpace= NULL;
		if(skinType>=SkinWithTgSpace)
		{
			srcTgSpace= &_OriginalTGSpace[0];
			dstTgSpace= (uint8*)vba.getTexCoordPointer(0, tgSpaceStage);
		}


		// For all vertices that need to be computed.
		uint		size= numVertices;
		for(;size>0;size--)
		{
			// If we must compute this vertex.
			if(*pFlag==NL3D_SOFTSKIN_VMUSTCOMPUTE)
			{
				// Flag this vertex as computed.
				*pFlag=NL3D_SOFTSKIN_VCOMPUTED;

				CPaletteSkin	*psPal= (CPaletteSkin*)srcPal;

				// checks indices.
				nlassert(psPal->MatrixId[0]<IDriver::MaxModelMatrix);
				nlassert(psPal->MatrixId[1]<IDriver::MaxModelMatrix);
				nlassert(psPal->MatrixId[2]<IDriver::MaxModelMatrix);
				nlassert(psPal->MatrixId[3]<IDriver::MaxModelMatrix);

				// compute vertex part.
				computeSoftwarePointSkinning(matrixes, srcVector, psPal, (float*)srcWgt, (CVector*)dstVector);

				// compute normal part.
				if(skinType>=SkinWithNormal)
					computeSoftwareVectorSkinning(matrixes, srcNormal, psPal, (float*)srcWgt, (CVector*)dstNormal);

				// compute tg part.
				if(skinType>=SkinWithTgSpace)
					computeSoftwareVectorSkinning(matrixes, srcTgSpace, psPal, (float*)srcWgt, (CVector*)dstTgSpace);
			}

			// inc flags.
			pFlag++;
			// inc src (all whatever skin type used...)
			srcVector++;
			srcNormal++;
			srcTgSpace++;
			// inc paletteSkin and dst  (all whatever skin type used...)
			srcPal+= dstStride;
			srcWgt+= dstStride;
			dstVector+= dstStride;
			dstNormal+= dstStride;
			dstTgSpace+= dstStride;
		}
	}


	// dirt
	_OriginalSkinRestored= false;
}


// ***************************************************************************
void	CMeshGeom::flagSkinVerticesForMatrixBlock(uint8 *skinFlags, CMatrixBlock &mb)
{
	for(uint i=0; i<mb.RdrPass.size(); i++)
	{
		CIndexBuffer	&PB= mb.RdrPass[i].PBlock;

		uint	nIndex;

		// This may be better to flags in 2 pass (first traverse primitives, then test vertices).
		// Better sol for BTB..., because number of tests are divided by 6 (for triangles).

		// for all prims, indicate which vertex we must compute.
		// nothing if not already computed (ie 0), because 0&1==0.
		// Lines.
		// Tris.
		CIndexBufferRead iba;
		PB.lock (iba);
		nIndex= PB.getNumIndexes();
		if (iba.getFormat() == CIndexBuffer::Indices32)
		{
			uint32 *pIndex= (uint32*)iba.getPtr();
			for(;nIndex>0;nIndex--, pIndex++)
				skinFlags[*pIndex]&= NL3D_SOFTSKIN_VMUSTCOMPUTE;
		}
		else
		{
			uint16 *pIndex= (uint16*)iba.getPtr();
			for(;nIndex>0;nIndex--, pIndex++)
				skinFlags[*pIndex]&= NL3D_SOFTSKIN_VMUSTCOMPUTE;
		}
	}
}


// ***************************************************************************
void	CMeshGeom::computeSoftwarePointSkinning(CMatrix3x4 *matrixes, CVector *srcVec, CPaletteSkin *srcPal, float *srcWgt, CVector *pDst)
{
	CMatrix3x4		*pMat;

	// 0th matrix influence.
	pMat= matrixes + srcPal->MatrixId[0];
	pMat->mulSetPoint(*srcVec, srcWgt[0], *pDst);
	// 1th matrix influence.
	pMat= matrixes + srcPal->MatrixId[1];
	pMat->mulAddPoint(*srcVec, srcWgt[1], *pDst);
	// 2th matrix influence.
	pMat= matrixes + srcPal->MatrixId[2];
	pMat->mulAddPoint(*srcVec, srcWgt[2], *pDst);
	// 3th matrix influence.
	pMat= matrixes + srcPal->MatrixId[3];
	pMat->mulAddPoint(*srcVec, srcWgt[3], *pDst);
}


// ***************************************************************************
void	CMeshGeom::computeSoftwareVectorSkinning(CMatrix3x4 *matrixes, CVector *srcVec, CPaletteSkin *srcPal, float *srcWgt, CVector *pDst)
{
	CMatrix3x4		*pMat;

	// 0th matrix influence.
	pMat= matrixes + srcPal->MatrixId[0];
	pMat->mulSetVector(*srcVec, srcWgt[0], *pDst);
	// 1th matrix influence.
	pMat= matrixes + srcPal->MatrixId[1];
	pMat->mulAddVector(*srcVec, srcWgt[1], *pDst);
	// 2th matrix influence.
	pMat= matrixes + srcPal->MatrixId[2];
	pMat->mulAddVector(*srcVec, srcWgt[2], *pDst);
	// 3th matrix influence.
	pMat= matrixes + srcPal->MatrixId[3];
	pMat->mulAddVector(*srcVec, srcWgt[3], *pDst);
}


// ***************************************************************************
void	CMeshGeom::computeSkinMatrixes(CSkeletonModel *skeleton, CMatrix3x4 *matrixes, CMatrixBlock  *prevBlock, CMatrixBlock  &mBlock)
{
	// For all matrix of this mBlock.
	for(uint idMat=0;idMat<mBlock.NumMatrix;idMat++)
	{
		uint	curBoneId= mBlock.MatrixId[idMat];

		// If same matrix binded as previous block, no need to bind!!
		if(prevBlock && idMat<prevBlock->NumMatrix && prevBlock->MatrixId[idMat]== curBoneId)
			continue;

		// Else, we must setup the matrix
		matrixes[idMat].set(skeleton->getActiveBoneSkinMatrix(curBoneId));
	}
}


// ***************************************************************************
void	CMeshGeom::profileSceneRender(CRenderTrav *rdrTrav, CTransformShape *trans, float polygonCount, uint32 rdrFlags)
{
	// get the mesh instance.
	CMeshBaseInstance	*mi= safe_cast<CMeshBaseInstance*>(trans);

	// For all _MatrixBlocks
	uint	triCount= 0;
	for(uint mb=0;mb<_MatrixBlocks.size();mb++)
	{
		CMatrixBlock	&mBlock= _MatrixBlocks[mb];

		// Profile all pass.
		for (uint i=0;i<mBlock.RdrPass.size();i++)
		{
			CRdrPass	&rdrPass= mBlock.RdrPass[i];
			// Profile with the Materials of the MeshInstance.
			if ( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (rdrFlags & IMeshGeom::RenderOpaqueMaterial) ) ||
				 ( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (rdrFlags & IMeshGeom::RenderTransparentMaterial) ) )
			{
				triCount+= rdrPass.PBlock.getNumIndexes()/3;
			}
		}
	}

	// Profile
	if(triCount)
	{
		// tri per VBFormat
		rdrTrav->Scene->incrementProfileTriVBFormat(rdrTrav->Scene->BenchRes.MeshProfileTriVBFormat,
			_VBuffer.getVertexFormat(), triCount);

		// VBHard
		if(_VBuffer.getPreferredMemory()!=CVertexBuffer::RAMPreferred)
			rdrTrav->Scene->BenchRes.NumMeshVBufferHard++;
		else
			rdrTrav->Scene->BenchRes.NumMeshVBufferStd++;

		// rendered in BlockRendering, only if not transparent pass (known it if RenderTransparentMaterial is set)
		if(supportMeshBlockRendering() && (rdrFlags & IMeshGeom::RenderTransparentMaterial)==0 )
		{
			if(isMeshInVBHeap())
			{
				rdrTrav->Scene->BenchRes.NumMeshRdrBlockWithVBHeap++;
				rdrTrav->Scene->BenchRes.NumMeshTriRdrBlockWithVBHeap+= triCount;
			}
			else
			{
				rdrTrav->Scene->BenchRes.NumMeshRdrBlock++;
				rdrTrav->Scene->BenchRes.NumMeshTriRdrBlock+= triCount;
			}
		}
		else
		{
			rdrTrav->Scene->BenchRes.NumMeshRdrNormal++;
			rdrTrav->Scene->BenchRes.NumMeshTriRdrNormal+= triCount;
		}
	}
}

// ***************************************************************************
bool	CMeshGeom::intersectSkin(CTransformShape	*mi, const CMatrix &toRaySpace, float &dist2D, float &distZ, bool computeDist2D)
{
	// for Mesh, Use the Shadow Skinning (simple version).

	// get skeleton
	if(!mi || _OriginalSkinVertices.empty())
		return false;
	CSkeletonModel	*skeleton= mi->getSkeletonModel();
	if(!skeleton)
		return false;

	// Compute skinning with all matrix this Mesh use. (the shadow geometry cannot use other Matrix than the mesh use).
	static std::vector<uint32>	matInfs;
	matInfs.resize(_BonesId.size());
	for(uint i=0;i<matInfs.size();i++)
	{
		// treat any "missing bone" as the root one.
		matInfs[i]= max(_BonesId[i], (sint32)0);
	}
	return _ShadowSkin.getRayIntersection(toRaySpace, *skeleton, matInfs, dist2D, distZ, computeDist2D);
}

// ***************************************************************************
void	CMeshGeom::buildShadowSkin()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// reset
	contReset(_ShadowSkin.Vertices);
	contReset(_ShadowSkin.Triangles);

	nlassert(_Skinned && (_VBuffer.getVertexFormat() & CVertexBuffer::PaletteSkinFlag)
		&& (_VBuffer.getVertexFormat() & CVertexBuffer::PositionFlag) );


	// *** Copy VBuffer content
	{
		// get num of vertices
		uint	numVertices= _VBuffer.getNumVertices();

		// lock VB
		CVertexBufferRead vba;
		_VBuffer.lock (vba);
		uint8		*srcPal= (uint8*)vba.getPaletteSkinPointer(0);
		uint8		*srcVert= (uint8*)vba.getVertexCoordPointer(0);
		uint32		srcVertSize= _VBuffer.getVertexSize();

		// copy vertices from VBuffer
		_ShadowSkin.Vertices.resize(numVertices);
		for(uint i=0; i<numVertices;i++)
		{
			// Copy Vertex
			_ShadowSkin.Vertices[i].Vertex= *((CVector*)srcVert);
			// Suppose the 0 matrix inf is the highest (we are at least sure it is not 0)
			// And SkinWeight Export show the 0th is the highest one...
			_ShadowSkin.Vertices[i].MatrixId= ((CPaletteSkin*)srcPal)->MatrixId[0];

			// Next
			srcVert+= srcVertSize;
			srcPal+= srcVertSize;
		}
	}

	// But _ShadowSkin.Vertices[i].MatrixId is incorrect, since < IDriver::MaxModelMatrix


	// *** Count number of triangles, and get start index of each matrix block in the final Tri list
	uint	numIndices= 0;
	uint	mb;
	// can't be static cause of ThreadSafe
	vector<pair<uint32,uint32> >		mbIndexRange;
	mbIndexRange.resize(_MatrixBlocks.size());
	for(mb=0;mb<_MatrixBlocks.size();mb++)
	{
		// this matrix block start here
		mbIndexRange[mb].first= numIndices;

		// count tris rendered for this matrix block
		const CMatrixBlock	&mBlock= _MatrixBlocks[mb];
		for(uint rp=0;rp<mBlock.RdrPass.size();rp++)
		{
			const CRdrPass	&rPass= mBlock.RdrPass[rp];
			numIndices+= rPass.PBlock.getNumIndexes();
		}

		// this matrix block end here
		mbIndexRange[mb].second= numIndices;
	}


	// *** Fill Triangles
	nlverify(retrieveTriangles(_ShadowSkin.Triangles));
	nlassert(numIndices==_ShadowSkin.Triangles.size());


	// *** Reindex correctly MatrixId, (ie unpack matrix blocks)
	// can't be static cause of ThreadSafe
	vector<bool>	vertReIndexed;
	vertReIndexed.resize(_ShadowSkin.Vertices.size(), false);
	// for all matrix blocks
	for(mb=0;mb<mbIndexRange.size();mb++)
	{
		const CMatrixBlock	&mBlock= _MatrixBlocks[mb];

		uint	iStart= mbIndexRange[mb].first;
		uint	iEnd= mbIndexRange[mb].second;
		nlassert(iStart <= iEnd && iEnd<=_ShadowSkin.Triangles.size());

		// For all indices in this range
		uint32	*pIndex= &_ShadowSkin.Triangles[iStart];
		uint	nbIndex= iEnd - iStart;
		for(;nbIndex>0;--nbIndex, pIndex++)
		{
			uint	index= *pIndex;
			// if not already reindexed
			if(!vertReIndexed[index])
			{
				vertReIndexed[index]= true;
				// reindex
				uint	matId= _ShadowSkin.Vertices[index].MatrixId;
				nlassert(matId<mBlock.NumMatrix);
				_ShadowSkin.Vertices[index].MatrixId= mBlock.MatrixId[matId];
			}
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// Mesh Block Render Interface
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CMeshGeom::supportMeshBlockRendering () const
{
	return _SupportMBRFlags!=0;
}

// ***************************************************************************
bool	CMeshGeom::sortPerMaterial() const
{
	return (_SupportMBRFlags & MBRSortPerMaterial)!=0;
}
// ***************************************************************************
uint	CMeshGeom::getNumRdrPassesForMesh() const
{
	return (uint)_MatrixBlocks[0].RdrPass.size();
}
// ***************************************************************************
uint	CMeshGeom::getNumRdrPassesForInstance(CMeshBaseInstance *inst) const
{
	return (uint)_MatrixBlocks[0].RdrPass.size();
}
// ***************************************************************************
void	CMeshGeom::beginMesh(CMeshGeomRenderContext &rdrCtx)
{
	if(rdrCtx.RenderThroughVBHeap)
	{
		// Don't setup VB in this case, since use the VBHeap setuped one.
		// NB: no VertexProgram test since VBHeap not possible with it...
		nlassert( (_SupportMBRFlags & MBRCurrentUseVP)==0 );
	}
	else
	{
		// use MeshVertexProgram effect?
		if( _MeshVertexProgram != NULL && _MeshVertexProgram->isMBRVpOk(rdrCtx.Driver) )
		{
			// Ok will use it.
			_SupportMBRFlags|= MBRCurrentUseVP;
			// Before VB activation
			_MeshVertexProgram->beginMBRMesh(rdrCtx.Driver, rdrCtx.Scene );
		}

		// active VB. SoftwareSkinning: reset flags for skinning.
		rdrCtx.Driver->activeVertexBuffer(_VBuffer);
	}
}
// ***************************************************************************
void	CMeshGeom::activeInstance(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *inst, float polygonCount, void *vbDst)
{
	// setup instance matrix
	rdrCtx.Driver->setupModelMatrix(inst->getWorldMatrix());

	// setupLighting.
	inst->changeLightSetup(rdrCtx.RenderTrav);

	// MeshVertexProgram ?
	if( _SupportMBRFlags & MBRCurrentUseVP )
	{
		CMatrix		invertedObjectMatrix;
		invertedObjectMatrix = inst->getWorldMatrix().inverted();
		_MeshVertexProgram->beginMBRInstance(rdrCtx.Driver, rdrCtx.Scene, inst, invertedObjectMatrix);
	}
}
// ***************************************************************************
void	CMeshGeom::renderPass(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *mi, float polygonCount, uint rdrPassId)
{
	CMatrixBlock	&mBlock= _MatrixBlocks[0];

	CRdrPass		&rdrPass= mBlock.RdrPass[rdrPassId];
	// Render with the Materials of the MeshInstance, only if not blended.
	if( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) ) )
	{
		CMaterial	&material= mi->Materials[rdrPass.MaterialId];

		// MeshVertexProgram ?
		if( _SupportMBRFlags & MBRCurrentUseVP )
		{
			rdrCtx.RenderTrav->changeVPLightSetupMaterial(material, false);
		}

		if(rdrCtx.RenderThroughVBHeap)
		{
			// render shifted primitives
			rdrCtx.Driver->activeIndexBuffer(rdrPass.VBHeapPBlock);
			rdrCtx.Driver->renderTriangles(material, 0, rdrPass.VBHeapPBlock.getNumIndexes()/3);
		}
		else
		{
			// render primitives
			rdrCtx.Driver->activeIndexBuffer(rdrPass.PBlock);
			rdrCtx.Driver->renderTriangles(material, 0, rdrPass.PBlock.getNumIndexes()/3);
		}
	}
}
// ***************************************************************************
void	CMeshGeom::endMesh(CMeshGeomRenderContext &rdrCtx)
{
	// MeshVertexProgram ?
	if( _SupportMBRFlags & MBRCurrentUseVP )
	{
		// End Mesh
		_MeshVertexProgram->endMBRMesh( rdrCtx.Driver );

		// and remove Current Flag.
		_SupportMBRFlags&= ~MBRCurrentUseVP;
	}
}

// ***************************************************************************
bool	CMeshGeom::getVBHeapInfo(uint &vertexFormat, uint &numVertices)
{
	// CMeshGeom support VBHeap rendering, assuming supportMeshBlockRendering is true.
	if( _SupportMBRFlags )
	/* Yoyo: If VertexProgram, DON'T SUPPORT!! because VB need to be activated AFTER meshVP activation
		NB: still possible with complex code to do it (sort per VP type (with or not)...), but tests in Ryzom
		shows that VBHeap is not really important (not so much different shapes...)
	*/
	if( _MeshVertexProgram==NULL )
	{
		vertexFormat= _VBuffer.getVertexFormat();
		numVertices= _VBuffer.getNumVertices();
		return true;
	}

	return false;
}

// ***************************************************************************
void	CMeshGeom::computeMeshVBHeap(void *dst, uint indexStart)
{
	// Fill dst with Buffer content.
	CVertexBufferRead vba;
	_VBuffer.lock (vba);
	memcpy(dst, vba.getVertexCoordPointer(), _VBuffer.getNumVertices()*_VBuffer.getVertexSize() );

	// NB: only 1 MB is possible ...
	nlassert(_MatrixBlocks.size()==1);
	CMatrixBlock	&mBlock= _MatrixBlocks[0];
	// For all rdrPass.
	for(uint i=0;i<mBlock.RdrPass.size();i++)
	{
		// shift the PB
		CIndexBuffer	&srcPb= mBlock.RdrPass[i].PBlock;
		CIndexBuffer	&dstPb= mBlock.RdrPass[i].VBHeapPBlock;
		uint j;

		// Tris.
		dstPb.setNumIndexes(srcPb.getNumIndexes());
		CIndexBufferRead ibaRead;
		srcPb.lock (ibaRead);
		CIndexBufferReadWrite ibaWrite;
		dstPb.lock (ibaWrite);
		// nico : apparently not used, so don't manage 16 bits index here
		nlassert(ibaRead.getFormat() == CIndexBuffer::Indices32);
		nlassert(ibaWrite.getFormat() == CIndexBuffer::Indices32);
		const uint32	*srcTriPtr= (const uint32 *) ibaRead.getPtr();
		uint32			*dstTriPtr= (uint32 *) ibaWrite.getPtr();
		for(j=0; j<dstPb.getNumIndexes();j++)
		{
			dstTriPtr[j]= srcTriPtr[j]+indexStart;
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// CMeshBuild components.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
CMesh::CCorner::CCorner()
{
	sint	i;
	Vertex= 0;
	Normal= CVector::Null;
	for(i=0;i<CVertexBuffer::MaxStage;i++)
	{
		Uvws[i]= CUVW(0, 0, 0);
	}
	Color.set(255,255,255,255);
	Specular.set(0,0,0,0);
}


// ***************************************************************************
void CMesh::CCorner::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	nlassert(0); // not used
	f.serial(Vertex);
	f.serial(Normal);
	for(int i=0;i<CVertexBuffer::MaxStage;++i) f.serial(Uvws[i]);
	f.serial(Color);
	f.serial(Specular);
}

// ***************************************************************************
void CMesh::CFace::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	for(int i=0;i<3;++i)
		f.serial(Corner[i]);
	f.serial(MaterialId);
	f.serial(SmoothGroup);
}

// ***************************************************************************
void CMesh::CSkinWeight::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	for(int i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;++i)
	{
		f.serial(MatrixId[i]);
		f.serial(Weights[i]);
	}
}

// ***************************************************************************
/* Serialization is not used.
void CMesh::CMeshBuild::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint	ver= f.serialVersion(0);

	// Serial mesh base (material info).
	CMeshBaseBuild::serial(f);

	// Serial Geometry.
	f.serial( VertexFlags );
	f.serialCont( Vertices );
	f.serialCont( SkinWeights );
	f.serialCont( Faces );

}*/


// ************************************
CMesh::CMeshBuild::CMeshBuild()
{
	for (uint k = 0; k < CVertexBuffer::MaxStage; ++k)
	{
		NumCoords[k] = 2;
		UVRouting[k] = k;
	}
}


// ***************************************************************************
// ***************************************************************************
// CMesh.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
CMesh::CMesh()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// create the MeshGeom
	_MeshGeom= new CMeshGeom;
}
// ***************************************************************************
CMesh::~CMesh()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// delete the MeshGeom
	delete _MeshGeom;
}


// ***************************************************************************
CMesh::CMesh(const CMesh &mesh) : CMeshBase()
{
	// create the MeshGeom
	_MeshGeom= new CMeshGeom(*mesh._MeshGeom);
}


// ***************************************************************************
CMesh	&CMesh::operator=(const CMesh &mesh)
{
	// Copy CMeshBase part
	(CMeshBase&)*this= (CMeshBase&)mesh;

	// copy content of meshGeom.
	*_MeshGeom= *mesh._MeshGeom;


	return *this;
}



// ***************************************************************************
void	CMesh::build (CMeshBase::CMeshBaseBuild &mbase, CMeshBuild &m)
{
	/// copy MeshBase info: materials ....
	CMeshBase::buildMeshBase (mbase);

	// build the geometry.
	_MeshGeom->build (m, (uint)mbase.Materials.size());

	// compile some stuff
	compileRunTime();
}


// ***************************************************************************
void	CMesh::optimizeMaterialUsage(std::vector<sint> &remap)
{
	// For each material, count usage.
	vector<bool>	materialUsed;
	materialUsed.resize(CMeshBase::_Materials.size(), false);
	for(uint mb=0;mb<getNbMatrixBlock();mb++)
	{
		for(uint rp=0;rp<getNbRdrPass(mb);rp++)
		{
			uint	matId= getRdrPassMaterial(mb, rp);
			// flag as used.
			materialUsed[matId]= true;
		}
	}

	// Apply it to meshBase
	CMeshBase::applyMaterialUsageOptim(materialUsed, remap);

	// Apply lut to meshGeom.
	_MeshGeom->applyMaterialRemap(remap);
}


// ***************************************************************************
void CMesh::setBlendShapes(std::vector<CBlendShape>&bs)
{
	_MeshGeom->setBlendShapes (bs);
}

// ***************************************************************************
void	CMesh::build(CMeshBase::CMeshBaseBuild &mbuild, CMeshGeom &meshGeom)
{
	/// copy MeshBase info: materials ....
	CMeshBase::buildMeshBase(mbuild);

	// build the geometry.
	*_MeshGeom= meshGeom;

	// compile some stuff
	compileRunTime();
}


// ***************************************************************************
CTransformShape		*CMesh::createInstance(CScene &scene)
{
	// Create a CMeshInstance, an instance of a mesh.
	//===============================================
	CMeshInstance		*mi= (CMeshInstance*)scene.createModel(NL3D::MeshInstanceId);
	mi->Shape= this;

	// instanciate the material part of the Mesh, ie the CMeshBase.
	CMeshBase::instanciateMeshBase(mi, &scene);


	// do some instance init for MeshGeom
	_MeshGeom->initInstance(mi);

	// init render Filter
	mi->initRenderFilterType();

	return mi;
}


// ***************************************************************************
bool	CMesh::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	return _MeshGeom->clip(pyramid, worldMatrix);
}


// ***************************************************************************
void	CMesh::render(IDriver *drv, CTransformShape *trans, bool passOpaque)
{
	// 0 or 0xFFFFFFFF
	uint32	mask= (0-(uint32)passOpaque);
	uint32	rdrFlags;
	// select rdrFlags, without ifs.
	rdrFlags=	mask & (IMeshGeom::RenderOpaqueMaterial | IMeshGeom::RenderPassOpaque);
	rdrFlags|=	~mask & (IMeshGeom::RenderTransparentMaterial);
	// render the mesh
	_MeshGeom->render(drv, trans, 0, rdrFlags, 1);
}


// ***************************************************************************
void	CMesh::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/*
	Version 6:
		- cut in serialisation, because of:
			- bad ITexture serialisation (with no version....) => must cut.  (see CMeshBase serial).
			- because of this and to simplify, make a cut too in CMesh serialisation.
	NB : all old version code is dropped.
	*/
	sint	ver= f.serialVersion(6);


	if(ver<6)
		throw NLMISC::EStream(f, "Mesh in Stream is too old (Mesh version < 6)");


	// serial Materials infos contained in CMeshBase.
	CMeshBase::serialMeshBase(f);


	// serial geometry.
	_MeshGeom->serial(f);

	// if reading, compile some stuff
	if(f.isReading())
		compileRunTime();
}


// ***************************************************************************
const NLMISC::CAABBoxExt& CMesh::getBoundingBox() const
{
	return _MeshGeom->getBoundingBox();
}
// ***************************************************************************
const CVertexBuffer &CMesh::getVertexBuffer() const
{
	return _MeshGeom->getVertexBuffer() ;
}
// ***************************************************************************
uint CMesh::getNbMatrixBlock() const
{
	return _MeshGeom->getNbMatrixBlock();
}
// ***************************************************************************
uint CMesh::getNbRdrPass(uint matrixBlockIndex) const
{
	return _MeshGeom->getNbRdrPass(matrixBlockIndex) ;
}
// ***************************************************************************
const CIndexBuffer &CMesh::getRdrPassPrimitiveBlock(uint matrixBlockIndex, uint renderingPassIndex) const
{
	return _MeshGeom->getRdrPassPrimitiveBlock(matrixBlockIndex, renderingPassIndex) ;
}
// ***************************************************************************
uint32 CMesh::getRdrPassMaterial(uint matrixBlockIndex, uint renderingPassIndex) const
{
	return _MeshGeom->getRdrPassMaterial(matrixBlockIndex, renderingPassIndex) ;
}
// ***************************************************************************
float	CMesh::getNumTriangles (float distance)
{
	// A CMesh do not degrad himself, so return 0, to not be taken into account.
	return 0;
}
// ***************************************************************************
const	CMeshGeom& CMesh::getMeshGeom () const
{
	return *_MeshGeom;
}
// ***************************************************************************
void	CMesh::computeBonesId (CSkeletonModel *skeleton)
{
	nlassert (_MeshGeom);
	_MeshGeom->computeBonesId (skeleton);
}


// ***************************************************************************
void	CMesh::updateSkeletonUsage(CSkeletonModel *sm, bool increment)
{
	nlassert (_MeshGeom);
	_MeshGeom->updateSkeletonUsage(sm, increment);
}

// ***************************************************************************
IMeshGeom	*CMesh::supportMeshBlockRendering (CTransformShape *trans, float &polygonCount ) const
{
	// Ok if meshGeom is ok.
	if(_MeshGeom->supportMeshBlockRendering())
	{
		polygonCount= 0;
		return _MeshGeom;
	}
	else
		return NULL;
}


// ***************************************************************************
void	CMesh::profileSceneRender(CRenderTrav *rdrTrav, CTransformShape *trans, bool passOpaque)
{
	// 0 or 0xFFFFFFFF
	uint32	mask= (0-(uint32)passOpaque);
	uint32	rdrFlags;
	// select rdrFlags, without ifs.
	rdrFlags=	mask & (IMeshGeom::RenderOpaqueMaterial | IMeshGeom::RenderPassOpaque);
	rdrFlags|=	~mask & (IMeshGeom::RenderTransparentMaterial);
	// render the mesh
	_MeshGeom->profileSceneRender(rdrTrav, trans, 0, rdrFlags);
}

// ***************************************************************************
void	CMesh::compileRunTime()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// **** try to build a Visual Collision Mesh
	// clear first
	if(_VisualCollisionMesh)
	{
		delete _VisualCollisionMesh;
		_VisualCollisionMesh= NULL;
	}
	// build only if wanted
	if( (_CollisionMeshGeneration==AutoCameraCol && !_LightInfos.empty()) ||
		_CollisionMeshGeneration==ForceCameraCol )
	{

		vector<CVector>		vertices;
		vector<uint32>		indices;
		if(_MeshGeom->retrieveVertices(vertices) && _MeshGeom->retrieveTriangles(indices))
		{
			// ok, can build!
			_VisualCollisionMesh= new CVisualCollisionMesh;
			// if fails to build cause of too many vertices/indices for instance
			if( !_VisualCollisionMesh->build(vertices, indices,const_cast<CVertexBuffer&>(_MeshGeom->getVertexBuffer())) )
			{
				// delete
				delete _VisualCollisionMesh;
				_VisualCollisionMesh= NULL;
			}
		}
	}
}

// ***************************************************************************
void	CMesh::buildSystemGeometry()
{
	// clear any
	_SystemGeometry.clear();

	// don't build a system copy if skinned. In this case, ray intersection is done through CSkeletonModel
	// and intersectSkin() scheme
	if(_MeshGeom->isSkinned())
		return;

	// retrieve geometry (if VB/IB not resident)
	if( !_MeshGeom->retrieveVertices(_SystemGeometry.Vertices) ||
		!_MeshGeom->retrieveTriangles(_SystemGeometry.Triangles))
	{
		_SystemGeometry.clear();
	}

	// TestYoyo
	/*static uint32	totalMem= 0;
	totalMem+= _SystemGeometry.Vertices.size()*sizeof(CVector);
	totalMem+= _SystemGeometry.Triangles.size()*sizeof(uint32);
	nlinfo("CMesh: TotalMem: %d", totalMem);*/
}


} // NL3D



