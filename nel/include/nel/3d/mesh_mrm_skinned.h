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

#ifndef NL_MESH_MRM_SKINNED_H
#define NL_MESH_MRM_SKINNED_H

#include "nel/misc/types_nl.h"
#include "nel/3d/shape.h"
#include "nel/3d/driver.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/uv.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/animated_material.h"
#include "nel/3d/mesh_base.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mrm_mesh.h"
#include "nel/3d/mrm_parameters.h"
#include "nel/3d/bone.h"
#include "nel/3d/mesh_geom.h"
#include "nel/3d/mrm_level_detail.h"
#include "nel/3d/shadow_skin.h"
#include <set>
#include <vector>


namespace NL3D
{


using	NLMISC::CVector;
using	NLMISC::CPlane;
using	NLMISC::CMatrix;
class	CMRMBuilder;
// Fast matrix in mesh_mrm_skin.cpp
class	CMatrix3x4;
class	CRawVertexNormalSkinned1;
class	CRawVertexNormalSkinned2;
class	CRawVertexNormalSkinned3;
class	CRawVertexNormalSkinned4;
class	CRawSkinnedNormalCache;
class	CMeshMRMSkinnedInstance;
class	CSkinSpecularRdrPass;

#define NL3D_MESH_MRM_SKINNED_WEIGHT_FACTOR		(255.f)
#define NL3D_MESH_MRM_SKINNED_UV_FACTOR			(8192.f)
#define NL3D_MESH_MRM_SKINNED_NORMAL_FACTOR		(32767.f)
#define NL3D_MESH_MRM_SKINNED_DEFAULT_POS_SCALE (8.f/32767.f)
#define NL3D_MESH_MRM_SKINNED_MAX_MATRIX		4
#define NL3D_MESH_MRM_SKINNED_VERTEX_FORMAT		(CVertexBuffer::PositionFlag|CVertexBuffer::NormalFlag|CVertexBuffer::TexCoord0Flag|CVertexBuffer::PaletteSkinFlag)

// ***************************************************************************
/**
 * An MRM mesh geometry, with no materials information.
 *
 * To build a CMeshMRMSkinnedGeom, you should:
 *	- build a CMesh::CMeshBuild   meshBuild (see CMesh)
 *	- call MeshMRMGeom.build(MeshBuild);
 *
 * NB: internally, build() use CMRMBuilder, a builder of MRM.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CMeshMRMSkinnedGeom : public IMeshGeom
{
public:
	/// Constructor
	CMeshMRMSkinnedGeom();
	~CMeshMRMSkinnedGeom();

	/** Build a mesh, replacing old.
	 * this is much slower than CMeshGeom::build(), because it computes the MRM.
	 * \param params parameters of the MRM build process.
	 */
	void			build(CMesh::CMeshBuild &m, uint numMaxMaterial, const CMRMParameters &params= CMRMParameters());

	/// change materials Ids (called from CMesh::optimizeMaterialUsage())
	void			applyMaterialRemap(const std::vector<sint> &remap);


	/** Change MRM Distance setup.
	 *	NB: no-op if distanceFinest<0, distanceMiddle<=distanceFinest or if distanceCoarsest<=distanceMiddle.
	 *	\param distanceFinest The MRM has its max faces when dist<=distanceFinest.
	 *	\param distanceMiddle The MRM has 50% of its faces at dist==distanceMiddle.
	 *	\param distanceCoarsest The MRM has faces/Divisor (ie near 0) when dist>=distanceCoarsest.
	 */
	void			changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);


	/// \name From IMeshGeom
	// @{

	/// Init instance info.
	virtual	void	initInstance(CMeshBaseInstance *mbi);

	/// clip this mesh in a driver. true if visible.
	virtual bool	clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix) ;

	/// render() this mesh in a driver, given an instance and his materials.
	virtual void	render(IDriver *drv, CTransformShape *trans, float polygonCount, uint32 rdrFlags, float globalAlpha);

	/// render() this mesh as a skin
	virtual void	renderSkin(CTransformShape *trans, float alphaMRM);

	/// get an approximation of the number of triangles this instance will render for a fixed distance.
	virtual float	getNumTriangles (float distance);

	/// serial this meshGeom.
	virtual void	serial(NLMISC::IStream &f);
	NLMISC_DECLARE_CLASS(CMeshMRMSkinnedGeom);

	/// Scene profile
	void	profileSceneRender(CRenderTrav *rdrTrav, CTransformShape *trans, float polygonCount, uint32 rdrFlags);

	// @}


	/// \name Geometry accessors
	// @{

	/// get the extended axis aligned bounding box of the mesh
	const NLMISC::CAABBoxExt& getBoundingBox() const
	{
		return _BBox;
	}

	/// get the vertex buffer used by the mrm mesh. NB: this VB store all Vertices used by All LODs.
	void getVertexBuffer(CVertexBuffer &output) const;

	/// get the skin weight buffer
	void getSkinWeights (std::vector<CMesh::CSkinWeight> &skinWeights) const;


	/// get the bone names of the meshMRM.
	const std::vector<std::string>			&getBonesName() const {return _BonesName;}

	/** get the number of LOD.
	 */
	uint getNbLod() const { return (uint)_Lods.size() ; }


	/** get the number of rendering pass of a LOD.
	 *	\param lodId the id of the LOD.
	 */
	uint getNbRdrPass(uint lodId) const { return (uint)_Lods[lodId].RdrPass.size() ; }


	/** get the primitive block associated with a rendering pass of a LOD.
	 *	\param lodId the id of the LOD.
	 *  \param renderingPassIndex the index of the rendering pass
	 */
	void getRdrPassPrimitiveBlock(uint lodId, uint renderingPassIndex, CIndexBuffer &block) const
	{
		_Lods[lodId].getRdrPassPrimitiveBlock(renderingPassIndex, block);
	}


	/** get the material ID associated with a rendering pass of a LOD.
	 *	\param lodId the id of the LOD.
	 *	\param renderingPassIndex the index of the rendering pass in the matrix block
	 */
	uint32 getRdrPassMaterial(uint lodId, uint renderingPassIndex) const
	{
		return _Lods[lodId].RdrPass[renderingPassIndex].MaterialId ;
	}


	/// Advanced. get the geomorphs for a special lod.
	const std::vector<CMRMWedgeGeom>	&getGeomorphs(uint lodId) const
	{
		return _Lods[lodId].Geomorphs;
	}

	// @}


	/// \name Skinning Behavior
	// @{

	/// Compute skinning id
	void			computeBonesId (CSkeletonModel *skeleton);

	/// update Skeleton Usage. increment or decrement. computeBonesId must has been called before.
	void			updateSkeletonUsage(CSkeletonModel *sm, bool increment);

	/// return array of bones used by the skin. computeBonesId must has been called before.
	const std::vector<sint32>			&getSkinBoneUsage() const {return _BonesId;}

	/// see CTransform::getSkinBoneSphere() doc for the meaning of this value. computeBonesId must has been called before.
	const std::vector<NLMISC::CBSphere>	&getSkinBoneSphere() const {return _BonesSphere;}

	/// Called for edition purpose (slow  O(NVertex) ). computeBonesId must has been called before.
	bool			getSkinBoneBBox(CSkeletonModel *skeleton, NLMISC::CAABBox &bbox, uint boneId) const;

	// @}


	/// \name Mesh Block Render Implementation
	// @{

	/** true if this meshGeom support meshBlock rendering.
	 *	return false if skinned/meshMorphed.
	 */
	virtual bool	supportMeshBlockRendering () const;

	virtual bool	sortPerMaterial() const { return false; }
	virtual uint	getNumRdrPassesForMesh() const { return 0; }
	virtual uint	getNumRdrPassesForInstance(CMeshBaseInstance * /* inst */) const { return 0; }
	virtual	void	beginMesh(CMeshGeomRenderContext &/* rdrCtx */) {}
	virtual	void	activeInstance(CMeshGeomRenderContext &/* rdrCtx */, CMeshBaseInstance * /* inst */, float /* polygonCount */, void * /* vbDst */) {}
	virtual	void	renderPass(CMeshGeomRenderContext &/* rdrCtx */, CMeshBaseInstance * /* inst */, float /* polygonCount */, uint /* rdrPass */) {}
	virtual	void	endMesh(CMeshGeomRenderContext &/* rdrCtx */) {}

	// @}


	/// get the MRM level detail information
	const	CMRMLevelDetail		&getLevelDetail() const {return _LevelDetail;}


	/// \name Special SkinGrouping Rendering
	// @{
	bool			supportSkinGrouping() const;
	sint			renderSkinGroupGeom(CMeshMRMSkinnedInstance	*mi, float alphaMRM, uint remainingVertices, uint8 *vbDest);
	void			renderSkinGroupPrimitives(CMeshMRMSkinnedInstance	*mi, uint baseVertex, std::vector<CSkinSpecularRdrPass> &specularRdrPasses, uint skinIndex);
	void			renderSkinGroupSpecularRdrPass(CMeshMRMSkinnedInstance	*mi, uint rdrPassId);
	// @}

	// Is this mesh Geom has a VertexProgram bound?
	virtual bool	hasMeshVertexProgram() const {return false;}

	/// \name ShadowMap Skin rendering
	// @{
	/// Setup the ShadowMesh
	void			setShadowMesh(const std::vector<CShadowVertex> &shadowVertices, const std::vector<uint32> &triangles);

	/// Get the num of shadow skin vertices
	uint			getNumShadowSkinVertices() const;

	/// Render the ShadowSkin (SkinGroup like)
	bool			supportShadowSkinGrouping() const {return _SupportShadowSkinGrouping;}
	sint			renderShadowSkinGeom(CMeshMRMSkinnedInstance	*mi, uint remainingVertices, uint8 *vbDest);
	void			renderShadowSkinPrimitives(CMeshMRMSkinnedInstance	*mi, CMaterial &castMat, IDriver *drv, uint baseVertex);

	/** Special use of skinning to compute intersection of a ray with it.
	 *	Internaly Use same system than ShadowSkinning, see CShadowSkin::getRayIntersection()
 	 */
	bool			supportIntersectSkin() const {return supportShadowSkinGrouping();}
	bool			intersectSkin(CMeshMRMSkinnedInstance	*mi, const CMatrix &toRaySpace, float &dist2D, float &distZ, bool computeDist2D);

	// @}

// ************************
private:
	friend class	CMRMBuilder;

	/// \name Structures for building a MRM mesh.
	//@{

	/// A block of primitives, sorted by material use.
	class	CRdrPass
	{
	public:
		// The id of this material.
		uint32				MaterialId;
		// The list of primitives.
		std::vector<uint16>	PBlock;

		// Serialize a rdrpass.
		void	serial(NLMISC::IStream &f)
		{
			(void)f.serialVersion(0);

			f.serial(MaterialId);
			f.serialCont(PBlock);
		}

		// Get num triangle
		uint getNumTriangle () const
		{
			return (uint)PBlock.size()/3;
		}
	};


	/// A block of vertices descriptor.
	struct	CVertexBlock
	{
		// The index of the start vertex.
		uint32	VertexStart;
		// Number of vertices.
		uint32	NVertices;

		void	serial(NLMISC::IStream &f)
		{
			f.serial(VertexStart, NVertices);
		}
	};



	/// A LOD of the MRM.
	class	CLod
	{
	public:
		/// The number of vertex in The VB this Lod needs.
		uint32						NWedges;
		/// List of geomorph, for this LOD.
		std::vector<CMRMWedgeGeom>	Geomorphs;
		/// List of rdr pass, for this LOD.
		std::vector<CRdrPass>		RdrPass;

		/** Skinning: list of influenced vertices to compute, for this lod only. There is 4 array, 0th
		 *	is for vertices which have only one matrix. 1st if for vertices which have only 2 matrix ....
		 */
		std::vector<uint32>				InfluencedVertices[NL3D_MESH_SKINNING_MAX_MATRIX];
		/// Skinning: list of Matrix which influence this Lod. So we know what matrix to compute.
		std::vector<uint32>				MatrixInfluences;



		CLod()
		{
		}

		// Serialize a Lod.
		void		serial(NLMISC::IStream &f);

		// Used in CMeshMRMSkinnedGeom::build().
		void		optimizeTriangleOrder();

		// Get the primitive block
		void getRdrPassPrimitiveBlock (uint renderPass, CIndexBuffer &block) const;

		// Build the primitive block
		void buildPrimitiveBlock(uint renderPass, const CIndexBuffer &block);
	};

	friend class	CLod;

	/** A mesh information. NB: private. unlike CMesh::CMeshBuild, do not herit from CMeshBase::CMeshBuild, because
	 * computed internally with CMRMBuilder, and only geometry is of interest.
	 */
	struct	CMeshBuildMRM
	{
		// This tells if the mesh is correctly skinned.
		bool								Skinned;

		// This is the array of SkinWeights, same size as the VB.
		std::vector<CMesh::CSkinWeight>		SkinWeights;

		// This VB is computed with CMRMBuilder and is ready to used
		CVertexBuffer			VBuffer;

		// Lod array, computed with CMRMBuilder and ready to used
		std::vector<CLod>		Lods;

		// The blend shapes
		std::vector<CBlendShape>	BlendShapes;

		/// \Degradation control.
		// @{
		/// The MRM has its max faces when dist<=DistanceFinest.
		float					DistanceFinest;
		/// The MRM has 50% of its faces at dist==DistanceMiddle.
		float					DistanceMiddle;
		/// The MRM has faces/Divisor when dist>=DistanceCoarsest.
		float					DistanceCoarsest;
		// @}

	};
	//@}



public:

	/// The packed vertex buffer
	class CPackedVertexBuffer
	{
	public:
		/// The Final VBuffer
		struct CPackedVertex
		{
			sint16	X, Y, Z;
			sint16	Nx, Ny, Nz;
			sint16	U, V;
			uint8	Matrices[NL3D_MESH_MRM_SKINNED_MAX_MATRIX];
			uint8	Weights[NL3D_MESH_MRM_SKINNED_MAX_MATRIX];

			// Decompact it
			inline void getPos (CVector &dest, float factor) const
			{
				dest.x = (float)X * factor;
				dest.y = (float)Y * factor;
				dest.z = (float)Z * factor;
			}
			inline void getNormal (CVector &dest) const
			{
				dest.x = (float)Nx * (1.f/NL3D_MESH_MRM_SKINNED_NORMAL_FACTOR);
				dest.y = (float)Ny * (1.f/NL3D_MESH_MRM_SKINNED_NORMAL_FACTOR);
				dest.z = (float)Nz * (1.f/NL3D_MESH_MRM_SKINNED_NORMAL_FACTOR);
			}
			inline void getU (float &_u) const
			{
				_u = (float)U * (1.f/NL3D_MESH_MRM_SKINNED_UV_FACTOR);
			}
			inline void getV (float &_v) const
			{
				_v = (float)V * (1.f/NL3D_MESH_MRM_SKINNED_UV_FACTOR);
			}
			inline void getWeight (float &dest, uint index) const
			{
				dest = (float)Weights[index] * (1.f/NL3D_MESH_MRM_SKINNED_WEIGHT_FACTOR);
			}

			// Compact it
			void setNormal (const CVector &pos);
			void setUV (float _u, float _v);
			void setWeight (uint weightId, float weight);
			void setPos (const CVector &pos, float scaleFactor);

			// Serial it
			void serial(NLMISC::IStream &f);
		};

		// Build it
		void build (const CVertexBuffer &buffer, const std::vector<CMesh::CSkinWeight> &skinWeight);

		// Clear it
		void clear ()
		{
			_PackedBuffer.clear();
		}

		// Reset
		void contReset()
		{
			NLMISC::contReset (_PackedBuffer);
		}

		// Access it
		const CPackedVertex	*getPackedVertices() const
		{
			if (_PackedBuffer.empty())
				return NULL;
			else
				return &(_PackedBuffer[0]);
		}

		// Access it
		CPackedVertex	*getPackedVertices()
		{
			if (_PackedBuffer.empty())
				return NULL;
			else
				return &(_PackedBuffer[0]);
		}

		// Acces it
		uint getNumVertices() const
		{
			return (uint)_PackedBuffer.size();
		}

		// Decompact position
		inline void getPos (CVector &dest, const CPackedVertex &src) const
		{
			src.getPos (dest, _DecompactScale);
		}

		// Serialize it
		void serial(NLMISC::IStream &f);

	private:
		/// Decompact position values
		float						_DecompactScale;

		//
		std::vector<CPackedVertex>	_PackedBuffer;
	};

private:

	// The packed vertex buffer
	CPackedVertexBuffer			_VBufferFinal;

	/// This boolean is true if the bones id have been passed in the skeleton
	bool						_BoneIdComputed;
	/// true if the _BonesIdExt have been computed (for bone Usage).
	bool						_BoneIdExtended;

	/// Last lod rendered. used with renderSkinGroup*() only
	uint8						_LastLodComputed;

	/// This array give the name of the local bones
	std::vector<std::string>	_BonesName;
	/// This array give the index in the skeleton of the local bones used. computed at first computeBoneId()
	std::vector<sint32>			_BonesId;
	/// Same as _BonesId but with parent of bones added. (used for bone usage)
	std::vector<sint32>			_BonesIdExt;
	/// see CTransform::getSkinBoneSphere() doc for the meaning of this value
	std::vector<NLMISC::CBSphere>	_BonesSphere;

	/// List of Lods.
	std::vector<CLod>			_Lods;
	/// For clipping. this is the BB of all vertices of all Lods.
	NLMISC::CAABBoxExt			_BBox;


	/// \Degradation control.
	// @{
	CMRMLevelDetail				_LevelDetail;
	// @}


	/// \name Hard VB
	// @{

	/// NB: HERE FOR PACKING ONLY. For clipping. Estimate if we must do a Precise clipping (ie with bboxes)
	bool						_PreciseClipping;

	// @}

	/// \name ShadowMap Skin rendering
	// @{
	CShadowSkin						_ShadowSkin;
	bool							_SupportShadowSkinGrouping;
	// @}

private:

	/// choose the lod according to the alphaMRM [0,1] given.
	sint	chooseLod(float alphaMRM, float &alphaLod);

	/// Apply the geomorph to the _VBuffer, or the VBhard, if exist/used
	void	applyGeomorph(std::vector<CMRMWedgeGeom>  &geoms, float alphaLod);

	/// Apply the geomorph to the VBhard ptr, if not NULL
	void	applyGeomorphWithVBHardPtr(std::vector<CMRMWedgeGeom>  &geoms, float alphaLod);

	/// Faster, but common geomorph apply
	void	applyGeomorphPosNormalUV0Int(std::vector<CMRMWedgeGeom>  &geoms, uint8 *vertexPtr, uint8 *vertexDestPtr, sint32 vertexSize, sint a, sint a1);

	/// Faster, but common geomorph apply
	void	applyGeomorphPosNormalUV0(std::vector<CMRMWedgeGeom>  &geoms, uint8 *vertexPtr, uint8 *vertexDestPtr, sint32 vertexSize, float a, float a1);

	/** The same as apply skin, but with normal modified. Normal is not normalized.
	  *	4 versions from slower to faster.
	  */
	void	applyRawSkinWithNormal(CLod &lod, CRawSkinnedNormalCache &rawSkinLod, const CSkeletonModel *skeleton, uint8 *vbHard, float alphaLod);

	// Some runtime not serialized compilation
	void		compileRunTime();

	// SkinGroup
	void		updateShiftedTriangleCache(CMeshMRMSkinnedInstance *mi, sint curLodId, uint baseVertex);

private:

	/// \name RawSkin optimisation.
	// @{

	/// Each time the mesh is loaded/built, this increment
	uint		_MeshDataId;

	/// compute RawSkin info in the MRMInstance according to current skin setup.
	void		updateRawSkinNormal(bool enabled, CMeshMRMSkinnedInstance *mi, sint curLodId);
	/// Increment the refCount, so instances RawSkins are no longer valid
	void		dirtMeshDataId();

	// ApplySkin method
	void		applyArrayRawSkinNormal1(CRawVertexNormalSkinned1 *src, uint8 *destVertexPtr,
		CMatrix3x4 *boneMat3x4, uint nInf);
	void		applyArrayRawSkinNormal2(CRawVertexNormalSkinned2 *src, uint8 *destVertexPtr,
		CMatrix3x4 *boneMat3x4, uint nInf);
	void		applyArrayRawSkinNormal3(CRawVertexNormalSkinned3 *src, uint8 *destVertexPtr,
		CMatrix3x4 *boneMat3x4, uint nInf);
	void		applyArrayRawSkinNormal4(CRawVertexNormalSkinned4 *src, uint8 *destVertexPtr,
		CMatrix3x4 *boneMat3x4, uint nInf);


public:
	static  uint	NumCacheVertexNormal1;
	static  uint	NumCacheVertexNormal2;
	static  uint	NumCacheVertexNormal3;
	static  uint	NumCacheVertexNormal4;

	// @}
};



// ***************************************************************************
/**
 * An instanciable MRM mesh.
 *
 * To build a CMeshMRMSkinned, you should:
 *	- build a CMesh::CMeshBuild   meshBuild (see CMesh)
 *	- call MeshMRM.build(MeshBuild);
 *	- call if you want setAnimatedMaterial() etc...
 *
 * NB: internally, build() use CMRMBuilder, a builder of MRM.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMeshMRMSkinned : public CMeshBase
{
public:
	/// Constructor
	CMeshMRMSkinned();

	/** Return true if the CMesh::CMeshBuild is compatible with the CMeshMRMSkinned.
	  * If not, build a CMeshMRM. You must test this before call build().
	  */
	static bool		isCompatible(const CMesh::CMeshBuild &m);

	/** Build a mesh, replacing old. WARNING: This has a side effect of deleting AnimatedMaterials.
	 * Call isCompatible() before this method.
	 * this is much slower than CMesh::build(), because it computes the MRM.
	 * \param params parameters of the MRM build process.
	 */
	void			build ( CMeshBase::CMeshBaseBuild &mBase, CMesh::CMeshBuild &m,
							const CMRMParameters &params= CMRMParameters() );

	/** Build a mesh, replacing old. build from a CMeshBaseBuild (materials info) and a previously builded CMeshMRMSkinnedGeom.
	 *	WARNING: This has a side effect of deleting AnimatedMaterials.
	 *	this is much slower than CMesh::build(), because it computes the MRM.
	 * \param params parameters of the MRM build process.
	 */
	void			build (CMeshBase::CMeshBaseBuild &m, const CMeshMRMSkinnedGeom &mgeom);

	/** Optimize material use. If a material in CMeshBase is not used by any renderPasses, it is removed, and ids are updated.
	 *	WARNING: This has a side effect of deleting AnimatedMaterials.
	 *	\param remap a remap material Id: newId= remap[oldId]. -1 means "no more used"
	 */
	void			optimizeMaterialUsage(std::vector<sint> &remap);


	/// Compute skinning id
	void			computeBonesId (CSkeletonModel *skeleton);

	/// update Skeleton Usage. increment or decrement.
	void			updateSkeletonUsage(CSkeletonModel *sm, bool increment);

	/** Change MRM Distance setup.
	 *	NB: no-op if distanceFinest<0, distanceMiddle<=distanceFinest or if distanceCoarsest<=distanceMiddle.
	 *	\param distanceFinest The MRM has its max faces when dist<=distanceFinest.
	 *	\param distanceMiddle The MRM has 50% of its faces at dist==distanceMiddle.
	 *	\param distanceCoarsest The MRM has faces/Divisor (ie near 0) when dist>=distanceCoarsest.
	 */
	void			changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);

	/// \name From IShape
	// @{

	/// Create a CMeshInstance, which contains materials.
	virtual	CTransformShape		*createInstance(CScene &scene);

	/// clip this mesh in a driver.
	virtual bool	clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix) ;

	/// render() this mesh in a driver.
	virtual void	render(IDriver *drv, CTransformShape *trans, bool passOpaque);

	/// get an approximation of the number of triangles this instance will render for a fixed distance.
	virtual float	getNumTriangles (float distance);

	/// serial this mesh.
	virtual void	serial(NLMISC::IStream &f);
	NLMISC_DECLARE_CLASS(CMeshMRMSkinned);

	/// Get bbox.
	virtual void	getAABBox(NLMISC::CAABBox &bbox) const {bbox= getBoundingBox().getAABBox();}

	/// profiling
	virtual void	profileSceneRender(CRenderTrav *rdrTrav, CTransformShape *trans, bool opaquePass);

	// @}


	/// \name Geometry accessors
	// @{

	/// get the extended axis aligned bounding box of the mesh
	const NLMISC::CAABBoxExt& getBoundingBox() const
	{
		return _MeshMRMGeom.getBoundingBox();
	}

	/// get the vertex buffer used by the mrm mesh. NB: this VB store all Vertices used by All LODs.
	void getVertexBuffer(CVertexBuffer &output) const { _MeshMRMGeom.getVertexBuffer(output); }


	/** get the number of LOD.
	 */
	uint getNbLod() const { return _MeshMRMGeom.getNbLod()  ; }


	/** get the number of rendering pass of a LOD.
	 *	\param lodId the id of the LOD.
	 */
	uint getNbRdrPass(uint lodId) const { return _MeshMRMGeom.getNbRdrPass(lodId) ; }


	/** get the primitive block associated with a rendering pass of a LOD.
	 *	\param lodId the id of the LOD.
	 *  \param renderingPassIndex the index of the rendering pass
	 */
	void getRdrPassPrimitiveBlock(uint lodId, uint renderingPassIndex, CIndexBuffer &block) const
	{
		_MeshMRMGeom.getRdrPassPrimitiveBlock(lodId, renderingPassIndex, block);
	}


	/** get the material ID associated with a rendering pass of a LOD.
	 *	\param lodId the id of the LOD.
	 *	\param renderingPassIndex the index of the rendering pass in the matrix block
	 */
	uint32 getRdrPassMaterial(uint lodId, uint renderingPassIndex) const
	{
		return _MeshMRMGeom.getRdrPassMaterial(lodId, renderingPassIndex) ;
	}

	/// Get the mesh geom
	const CMeshMRMSkinnedGeom& getMeshGeom () const;

	// @}


	/// \name Mesh Block Render Interface
	// @{
	virtual IMeshGeom	*supportMeshBlockRendering (CTransformShape *trans, float &polygonCount ) const;
	// @}


// ************************
private:

	CMeshMRMSkinnedGeom		_MeshMRMGeom;


};


} // NL3D


#endif // NL_MESH_MRM_SKINNED_H

/* End of mesh_mrm_skinned.h */
