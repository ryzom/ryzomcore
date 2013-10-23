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

#ifndef NL_MESH_H
#define NL_MESH_H

#include "nel/misc/types_nl.h"
#include "nel/3d/shape.h"
#include "nel/3d/driver.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/uv.h"
#include "nel/misc/bit_set.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/animated_material.h"
#include "nel/3d/mesh_base.h"
#include "nel/3d/mesh_geom.h"
#include "nel/3d/mesh_morpher.h"
#include "nel/3d/mesh_vertex_program.h"
#include "nel/3d/shadow_skin.h"
#include <set>
#include <vector>


namespace NL3D
{


using	NLMISC::CVector;
using	NLMISC::CPlane;
using	NLMISC::CMatrix;


class CMeshGeom;
class CSkeletonModel;
class CMatrix3x4;

// ***************************************************************************
// Should be 4.
#define		NL3D_MESH_SKINNING_MAX_MATRIX		4

// Above this distance, Mesh with a bigger Radius will use BBox clipping
#define		NL3D_MESH_PRECISE_CLIP_THRESHOLD	5.0f


// ***************************************************************************
/**
 * An instanciable mesh.
 * Skinning support: support only palette skinning.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CMesh : public CMeshBase
{
public:

	/// \name Structures for building a mesh.
	//@{

	/// A corner of a face.
	struct	CCorner
	{
		sint32			Vertex;		/// The vertex Id.
		CVector			Normal;
		NLMISC::CUVW	Uvws[CVertexBuffer::MaxStage];
		CRGBA			Color;
		CRGBA			Specular;

		// Setup all to 0, but Color (to white)... Important for good corner comparison.
		// This is slow but doesn't matter since used at mesh building....
		CCorner();

		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	};

	/// A Triangle face.
	struct	CFace
	{
		CCorner		Corner[3];
		sint32		MaterialId;
		sint32      SmoothGroup;
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	};


	/** Skinning: A skin weight for a vertex.
	 * NB: if you don't use all matrix for this vertex, use at least the 0th matrix, and simply set 0 on Weights you don't use.
	 */
	struct	CSkinWeight
	{
		/// What matrix of the skeleton shape this vertex use.
		uint32			MatrixId[NL3D_MESH_SKINNING_MAX_MATRIX];
		/// weight of this matrix (sum of 4 must be 1).
		float			Weights[NL3D_MESH_SKINNING_MAX_MATRIX];

		/// ctor.
		CSkinWeight()
		{
			for(uint i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
			{
				MatrixId[i]=0;
				Weights[i]=0;
			}
		}

		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	};

	struct CVertLink
	{
		uint32 nFace, nCorner;
		uint32 VertVB;

		CVertLink (uint32 face, uint32 corner, uint32 iVB)
		{
			nFace = face;
			nCorner = corner;
			VertVB = iVB;
		}
	};

	/** Mesh Interface System for MRM
	 */
	struct	CInterfaceVertex
	{
		CVector					Pos;
		CVector					Normal;
	};
	struct	CInterface
	{
		// The polygon describing the interface between 2 meshs.
		std::vector<CInterfaceVertex>	Vertices;
	};
	/// For each vertex
	struct	CInterfaceLink
	{
		// to which interface this vertex is welded. -1 if none
		sint	InterfaceId;
		// to which vertex of the interface this vertex is welded
		uint	InterfaceVertexId;

		CInterfaceLink()
		{
			InterfaceId= -1;
		}
	};

	/// A mesh information.
	struct	CMeshBuild
	{
		/** the IDRV_VF* flags which tells what vertices data are used. See IDriver::setVertexFormat() for
		 * more information. NB: IDRV_VF_XYZ is always considered to true..
		 * Note that is some stage use 2 textures coordinates instead of 3, then the extended vertex format must be used isntead
		 */
		sint32						VertexFlags;
		uint8						NumCoords[CVertexBuffer::MaxStage]; // tells for each uvw if is uses 2 or 3 coords
		uint8						UVRouting[CVertexBuffer::MaxStage]; // gives the uv routing table. Each final UV channel can be routed to any vertex uv

		// Vertices array
		std::vector<CVector>		Vertices;

		// Palette Skinning Vertices array (same size as Vertices). NULL if no skinning.
		std::vector<CSkinWeight>	SkinWeights;

		// Bones name. Each matrix id used in SkinWeights must have a corresponding string in the bone name array.
		std::vector<std::string>	BonesNames;

		// Faces array
		std::vector<CFace>			Faces;

		// Blend shapes if some
		std::vector<CBlendShape>	BlendShapes;

		// Link between VB and max vertex indices
		std::vector<CVertLink>		VertLink; // Filled when called build

		// MeshVertexProgram to copy to meshGeom.
		NLMISC::CSmartPtr<IMeshVertexProgram>	MeshVertexProgram;

		// Mesh Interface System for MRM building
		std::vector<CInterface>		Interfaces;
		// must be same size as Vertices, else Mesh Interface system disabled
		std::vector<CInterfaceLink>	InterfaceLinks;

		NLMISC::CBitSet             InterfaceVertexFlag; // each bit indicate if the vertex belongs to an interface


		CMeshBuild();

		// Serialization
		//void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	};
	//@}


public:
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/// Constructor
	CMesh();
	/// dtor
	~CMesh();
	CMesh(const CMesh &mesh);
	CMesh	&operator=(const CMesh &mesh);


	/// Build a mesh, replacing old. WARNING: This has a side effect of deleting AnimatedMaterials.
	void			build(CMeshBase::CMeshBaseBuild &mbase, CMeshBuild &mbuild);

	/// Build a mesh from material info, and a builded MeshGeom. WARNING: This has a side effect of deleting AnimatedMaterials.
	void			build(CMeshBase::CMeshBaseBuild &mbuild, CMeshGeom &meshGeom);

	/** Optimize material use. If a material in CMeshBase is not used by any renderPasses, it is removed, and ids are updated.
	 *	WARNING: This has a side effect of deleting AnimatedMaterials.
	 *	\param remap a remap material Id: newId= remap[oldId]. -1 means "no more used"
	 */
	void			optimizeMaterialUsage(std::vector<sint> &remap);


	void			setBlendShapes(std::vector<CBlendShape>&bs);

	/// Compute skinning id
	void			computeBonesId (CSkeletonModel *skeleton);

	/// update Skeleton Usage. increment or decrement.
	void			updateSkeletonUsage(CSkeletonModel *sm, bool increment);

	/// \name From IShape
	// @{

	/// Create a CMeshInstance, which contains materials.
	virtual	CTransformShape		*createInstance(CScene &scene);

	/// clip this mesh in a driver.
	virtual bool	clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix) ;

	/// render() this mesh in a driver.
	virtual void	render(IDriver *drv, CTransformShape *trans, bool opaquePass);

	/// serial this mesh.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CMesh);

	/// get trinagle count.
	virtual float	getNumTriangles (float distance);

	/// Get bbox.
	virtual void	getAABBox(NLMISC::CAABBox &bbox) const {bbox= getBoundingBox().getAABBox();}

	/// profiling
	virtual void	profileSceneRender(CRenderTrav *rdrTrav, CTransformShape *trans, bool opaquePass);

	/// System Mem Geometry Copy, built at load time
	virtual void	buildSystemGeometry();

	// @}

	/// \name Geometry accessors
	// @{

	/// get the extended axis aligned bounding box of the mesh
	const NLMISC::CAABBoxExt& getBoundingBox() const;

	/// get the vertex buffer used by the mesh
	const CVertexBuffer &getVertexBuffer() const;

	/// get the number of matrix block
	uint getNbMatrixBlock() const;

	/** get the number of rendering pass for a given matrix block
	 *  \param matrixBlockIndex the index of the matrix block the rendering passes belong to
	 */
	uint getNbRdrPass(uint matrixBlockIndex) const;

	/** get the primitive block associated with a rendering pass of a matrix block
	 *  \param matrixBlockIndex the index of the matrix block the renderin pass belong to
	 *  \param renderingPassIndex the index of the rendering pass in the matrix block
	 */
	const CIndexBuffer &getRdrPassPrimitiveBlock(uint matrixBlockIndex, uint renderingPassIndex) const;

	/** get the material ID associated with a rendering pass of a matrix block
	 *  \param matrixBlockIndex the index of the matrix block the renderin pass belong to
	 *  \param renderingPassIndex the index of the rendering pass in the matrix block
	 */
	uint32 getRdrPassMaterial(uint matrixBlockIndex, uint renderingPassIndex) const;

	/// Get the geom mesh
	const CMeshGeom &getMeshGeom () const;

	// @}


	/// \name Mesh Block Render Interface
	// @{
	virtual IMeshGeom	*supportMeshBlockRendering (CTransformShape *trans, float &polygonCount ) const;
	// @}

private:

	// The geometry.
	CMeshGeom		*_MeshGeom;

	// Called at load or build time, all that is not serialized
	void	compileRunTime();
};




// ***************************************************************************
/**
 * A mesh geometry.
 * Skinning support: support only palette skinning.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CMeshGeom: public IMeshGeom
{
public:

	/// Constructor
	CMeshGeom();
	virtual ~CMeshGeom();

	/// Build a meshGeom
	void			build(CMesh::CMeshBuild &mbuild, uint numMaxMaterial);

	void			setBlendShapes(std::vector<CBlendShape>&bs);

	/// change materials Ids (called from CMesh::optimizeMaterialUsage())
	void			applyMaterialRemap(const std::vector<sint> &remap);


	/// \name From IMeshGeom
	// @{

	/// Init instance info.
	virtual	void	initInstance(CMeshBaseInstance *mbi);

	/// clip this mesh
	virtual bool	clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix) ;

	/// render() this mesh in a driver.
	virtual void	render(IDriver *drv, CTransformShape *trans, float polygonCount, uint32 rdrFlags, float globalAlpha);

	/// render() this mesh as a skin
	virtual void	renderSkin(CTransformShape *trans, float alphaMRM);

	// get an approximation of the number of triangles this instance will render for a fixed distance.
	virtual float	getNumTriangles (float distance);

	/// serial this mesh.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CMeshGeom);

	// profile
	virtual void	profileSceneRender(CRenderTrav *rdrTrav, CTransformShape *trans, float polygonCount, uint32 rdrFlags);

	// @}


	/// \name Geometry accessors
	// @{

	/// get the extended axis aligned bounding box of the mesh
	const NLMISC::CAABBoxExt& getBoundingBox() const
	{
		return _BBox;
	}

	/// get the vertex buffer used by the mesh
	const CVertexBuffer &getVertexBuffer() const { return _VBuffer ; }

	/// get the number of matrix block
	uint getNbMatrixBlock() const { return (uint)_MatrixBlocks.size() ; }

	/** get the number of rendering pass for a given matrix block
	 *  \param matrixBlockIndex the index of the matrix block the rendering passes belong to
	 */
	uint getNbRdrPass(uint matrixBlockIndex) const { return (uint)_MatrixBlocks[matrixBlockIndex].RdrPass.size() ; }

	/** get the primitive block associated with a rendering pass of a matrix block
	 *  \param matrixBlockIndex the index of the matrix block the renderin pass belong to
	 *  \param renderingPassIndex the index of the rendering pass in the matrix block
	 */
	const CIndexBuffer &getRdrPassPrimitiveBlock(uint matrixBlockIndex, uint renderingPassIndex) const
	{
		return _MatrixBlocks[matrixBlockIndex].RdrPass[renderingPassIndex].PBlock ;
	}

	/** get the material ID associated with a rendering pass of a matrix block
	 *  \param matrixBlockIndex the index of the matrix block the renderin pass belong to
	 *  \param renderingPassIndex the index of the rendering pass in the matrix block
	 */
	uint32 getRdrPassMaterial(uint matrixBlockIndex, uint renderingPassIndex) const
	{
		return _MatrixBlocks[matrixBlockIndex].RdrPass[renderingPassIndex].MaterialId ;
	}

	/// get the number of BlendShapes
	uint getNbBlendShapes() const { if(_MeshMorpher) return (uint)_MeshMorpher->BlendShapes.size(); return 0; }

	/** Tool function to retrieve vector geometry only of the mesh.
	 *	return false if the vbuffer cannot be read (resident)
	 */
	bool	retrieveVertices(std::vector<NLMISC::CVector> &vertices) const;

	/** Tool function to retrieve triangles geometry only of the mesh (of all rdrpass).
	 *	return false if the index buffer cannot be read (resident)
	 */
	bool	retrieveTriangles(std::vector<uint32> &indices) const;

	// @}


	/// \name Skinning Behavior
	// @{

	/// Return true if the mesh is skinned, else return false.
	bool isSkinned () const
	{
		return _Skinned;
	}

	/// Compute skinning id
	void			computeBonesId (CSkeletonModel *skeleton);

	/// update Skeleton Usage. increment or decrement. computeBonesId must has been called before.
	void			updateSkeletonUsage(CSkeletonModel *sm, bool increment);

	/// return array of bones used by the skin. computeBonesId must has been called before.
	const std::vector<sint32>	&getSkinBoneUsage() const {return _BonesId;}

	/// see CTransform::getSkinBoneSphere() doc for the meaning of this value. computeBonesId must has been called before.
	const std::vector<NLMISC::CBSphere>	&getSkinBoneSphere() const {return _BonesSphere;}

	/// Skin intersection
	bool			supportIntersectSkin() const {return _Skinned;}
	bool			intersectSkin(CTransformShape	*mi, const CMatrix &toRaySpace, float &dist2D, float &distZ, bool computeDist2D);

	// @}


	/** render the mesh geometry with a single material. Render is said "Simple" because no special features are used:
	 *		- mesh is rendered without VertexProgram (if it has one).
	 *		- mesh is rendered without Skinning.
	 *		- mesh is rendered without use of VertexBufferHard.
	 *		- mesh is rendered without MeshMorpher.
	 *		- .....
	 */
	void			renderSimpleWithMaterial(IDriver *drv, const CMatrix &worldMatrix, CMaterial &mat);



	/// \name Mesh Block Render Implementation
	// @{

	/** true if this meshGeom support meshBlock rendering.
	 *	return false if skinned/meshMorphed.
	 */
	virtual bool	supportMeshBlockRendering () const;

	virtual bool	sortPerMaterial() const;
	virtual uint	getNumRdrPassesForMesh() const ;
	virtual uint	getNumRdrPassesForInstance(CMeshBaseInstance *inst) const ;
	virtual	void	beginMesh(CMeshGeomRenderContext &rdrCtx) ;
	virtual	void	activeInstance(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *inst, float polygonCount, void *vbDst) ;
	virtual	void	renderPass(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *inst, float polygonCount, uint rdrPass) ;
	virtual	void	endMesh(CMeshGeomRenderContext &rdrCtx) ;

	virtual	bool	getVBHeapInfo(uint &vertexFormat, uint &numVertices);
	virtual	void	computeMeshVBHeap(void *dst, uint indexStart);

	// @}

	// Is this mesh Geom has a VertexProgram bound?
	virtual bool	hasMeshVertexProgram() const {return _MeshVertexProgram!=NULL;}

	// get the Mesh VertexProgram
	IMeshVertexProgram	*getMeshVertexProgram() const {return _MeshVertexProgram;}

// ************************
private:

	/// A block of primitives, sorted by material used.
	class	CRdrPass
	{
	public:
		// The id of this material.
		uint32				MaterialId;
		// The list of primitives.
		CIndexBuffer		PBlock;

		// The same, shifted for VBHeap rendering.
		CIndexBuffer		VBHeapPBlock;


		// Serialize a rdrpass.
		void	serial(NLMISC::IStream &f)
		{
			(void)f.serialVersion(0);

			f.serial(MaterialId);
			f.serial(PBlock);
		}
		CRdrPass()
		{
			NL_SET_IB_NAME(PBlock, "CMesh::CRdrPass::PBlock");
			NL_SET_IB_NAME(VBHeapPBlock, "CMesh::CRdrPass::VBHeapPBlock");
			PBlock.setFormat(NL_MESH_INDEX_FORMAT);
		}
	};


	/// A block of RdrPasses, sorted by matrix use.
	class	CMatrixBlock
	{
	public:
		// ctor
		CMatrixBlock() : NumMatrix(0)
		{
			std::fill(MatrixId, MatrixId + IDriver::MaxModelMatrix, 0);
		}
		/// Which matrix we use for this block.
		uint32					MatrixId[IDriver::MaxModelMatrix];
		/// Number of matrix actually used.
		uint32					NumMatrix;
		/// List of rdr pass, for this matrix block.
		std::vector<CRdrPass>	RdrPass;

		void	serial(NLMISC::IStream &f)
		{
			(void)f.serialVersion(0);

			// Code written for IDriver::MaxModelMatrix==16 matrixs.
			nlctassert(IDriver::MaxModelMatrix == 16);
			for(uint i=0;i<IDriver::MaxModelMatrix;i++)
			{
				f.serial(MatrixId[i]);
			}
			f.serial(NumMatrix);
			f.serialCont(RdrPass);
		}


		/// return the idx of this bone, in MatrixId. -1 if not found.
		sint	getMatrixIdLocation(uint32 boneId) const;
	};


private:
	/**  Just for build process.
	 * NB: we must store palette info by corner (not by vertex) because Matrix Block grouping may insert vertex
	 * discontinuities. eg: a vertex use Matrix18. After Matrix grouping (16matrix max), Matrix18 could be Matrix2 for a group
	 * of face, but Matrix13 for another!!
	 */
	struct	CCornerTmp : public CMesh::CCorner
	{
		CPaletteSkin	Palette;
		float			Weights[NL3D_MESH_SKINNING_MAX_MATRIX];

		// The comparison.
		bool		operator<(const CCornerTmp &c) const;
		// The result of the compression.
		mutable sint	VBId;
		// The flags to know what to compare.
		static	sint	Flags;

		// Setup all to 0, but Color (to white)... Important for good corner comparison.
		// This is slow but doesn't matter since used at mesh building....
		CCornerTmp()
		{
			VBId= 0;
			for(sint i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
			{
				Palette.MatrixId[i]=0;
				Weights[i]=0;
			}
		}

		// copy from corner.
		CCornerTmp &operator=(const CMesh::CCorner &o)
		{
			Vertex= o.Vertex;
			Normal= o.Normal;
			for(sint i=0;i<CVertexBuffer::MaxStage;i++)
			{
				Uvws[i]= o.Uvws[i];
			}
			Color= o.Color;
			Specular= o.Specular;

			return *this;
		}

	};


	/** Just for build process. A Bone.
	 */
	struct	CBoneTmp
	{
		// How many ref points on it? (NB: a corner may have multiple (up to 4) on it).
		uint	RefCount;
		// Am i inserted into the current matrixblock?
		bool	Inserted;
		// If I am inserted into the current matrixblock, to which local bone (0-15) I am linked?
		uint	MatrixIdInMB;

		CBoneTmp()
		{
			RefCount= 0;
			Inserted=false;
		}
	};


	/** Just for build process. A map of Bone.
	 */
	typedef	std::map<uint, CBoneTmp>	TBoneMap;
	typedef	TBoneMap::iterator			ItBoneMap;


	/** Just for build process. A Triangle face.
	 */
	struct	CFaceTmp
	{
		CCornerTmp		Corner[3];
		uint			MaterialId;
		// which matrixblock own this face. -1 <=> Not owned.
		sint			MatrixBlockId;

		CFaceTmp()
		{
			MatrixBlockId= -1;
		}
		CFaceTmp	&operator=(const CMesh::CFace& o)
		{
			Corner[0]= o.Corner[0];
			Corner[1]= o.Corner[1];
			Corner[2]= o.Corner[2];
			MaterialId= o.MaterialId;

			return *this;
		}


		void	buildBoneUse(std::vector<uint>	&boneUse, std::vector<CMesh::CSkinWeight> &skinWeights);

	};


	/** Just for build process. A MatrixBlock remap.
	 */
	class	CMatrixBlockRemap
	{
	public:
		uint32					Remap[IDriver::MaxModelMatrix];
	};


private:
	/** Skinning: this is the list of vertices (mirror of VBuffer), at the bind Pos.
	 *	Potentially modified by the mesh morpher
	 */
	std::vector<CVector>		_OriginalSkinVertices;
	std::vector<CVector>		_OriginalSkinNormals;
	std::vector<CVector>		_OriginalTGSpace;

	/// VBuffer of the mesh (potentially modified by the mesh morpher and skinning)
	CVertexBuffer				_VBuffer;
	/// The original VBuffer of the mesh used only if there are blend shapes.
	CVertexBuffer				_VBufferOri;
	/// The matrix blocks.
	std::vector<CMatrixBlock>	_MatrixBlocks;
	/// For clipping.
	NLMISC::CAABBoxExt			_BBox;
	/// This tells if the mesh is correctly skinned.
	bool						_Skinned;
	/// This tells if the mesh VBuffer has coorect BindPos vertices
	bool						_OriginalSkinRestored;

	/// This boolean is true if the bones id have been passed in the skeleton
	bool						_BoneIdComputed;
	/// true if the _BonesIdExt have been computed (for bone Usage).
	bool						_BoneIdExtended;
	/// see CTransform::getSkinBoneSphere() doc for the meaning of this value
	std::vector<NLMISC::CBSphere>	_BonesSphere;


	/// This array give the name of the local bones used.
	std::vector<std::string>	_BonesName;
	/// This array give the index in the skeleton of the local bones used. computed at first computeBoneId()
	std::vector<sint32>			_BonesId;
	/// Same as _BonesId but with parent of bones added. (used for bone usage)
	std::vector<sint32>			_BonesIdExt;

	/// \name Mesh Block Render Implementation
	// @{
	/// setuped at compileRunTime.
	enum TMBRSupport
	{
		MBROk= 1,
		MBRSortPerMaterial= 2,
		MBRCurrentUseVP= 4,
	};
	// Of if don't support MBR at all
	uint8							_SupportMBRFlags;
	// @}

	/// Estimate if we must do a Precise clipping (ie with bboxes)
	bool						_PreciseClipping;

	// The Mesh Morpher
	CMeshMorpher	*_MeshMorpher;


	// Possible MeshVertexProgram to apply at render()
	NLMISC::CSmartPtr<IMeshVertexProgram>	_MeshVertexProgram;


private:
	// Locals, for build.
	class	CCornerPred
	{
	public:
		bool operator()(const CCornerTmp *x, const CCornerTmp *y) const
		{
			return (*x<*y);
		}
	};
	typedef		std::set<CCornerTmp*, CCornerPred>	TCornerSet;
	typedef		TCornerSet::iterator ItCornerSet;

	// Find and fill the VBuffer.
	void	findVBId(TCornerSet  &corners, const CCornerTmp *corn, sint &currentVBIndex, const CVector &vert, const CMesh::CMeshBuild &mb)
	{
		ItCornerSet  it= corners.find(const_cast<CCornerTmp *>(corn));
		if(it!=corners.end())
			corn->VBId= (*it)->VBId;
		else
		{
			// Add corner to the set to not insert same corner two times.
			corners.insert (const_cast<CCornerTmp *>(corn));
			sint	i;
			corn->VBId= currentVBIndex++;
			// Fill the VBuffer.
			_VBuffer.setNumVertices(currentVBIndex);
			sint	id= currentVBIndex-1;

			CVertexBufferReadWrite vba;
			_VBuffer.lock (vba);

			// XYZ.
			vba.setVertexCoord(id, vert);
			// Normal
			if(CCornerTmp::Flags & CVertexBuffer::NormalFlag)
				vba.setNormalCoord(id, corn->Normal);
			// Uvws.
			for(i=0;i<CVertexBuffer::MaxStage;i++)
			{
				if(CCornerTmp::Flags & (CVertexBuffer::TexCoord0Flag<<i))
				{
					switch(mb.NumCoords[i])
					{
						case 2:
							vba.setTexCoord(id, uint8(i), corn->Uvws[i].U, corn->Uvws[i].V);
						break;
						case 3:
							vba.setValueFloat3Ex((CVertexBuffer::TValue) (CVertexBuffer::TexCoord0 + i), id, corn->Uvws[i].U, corn->Uvws[i].V, corn->Uvws[i].W);
						break;
						default: // not supported
							nlassert(0);
						break;
					}
				}
			}
			// Color.
			if(CCornerTmp::Flags & CVertexBuffer::PrimaryColorFlag)
				vba.setColor(id, corn->Color);
			// Specular.
			if(CCornerTmp::Flags & CVertexBuffer::SecondaryColorFlag)
				vba.setSpecular(id, corn->Specular);

			// setup palette skinning.
			if ((CCornerTmp::Flags & CVertexBuffer::PaletteSkinFlag)==CVertexBuffer::PaletteSkinFlag)
			{
				vba.setPaletteSkin(id, corn->Palette);
				for(i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
					vba.setWeight(id, uint8(i), corn->Weights[i]);
			}
		}
	}


	// optimize triangles order of all render pass.
	void	optimizeTriangleOrder();

	// Some runtime not serialized compilation
	void	compileRunTime();


	/// \name Skinning
	// @{

	enum	TSkinType {SkinPosOnly=0, SkinWithNormal, SkinWithTgSpace};

	// build skinning.
	void	buildSkin(CMesh::CMeshBuild &m, std::vector<CFaceTmp>	&tmpFaces);

	// Build bone Usage information for serialized mesh <= version 3.
	void	buildBoneUsageVer3 ();

	// bkup from VBuffer into _OriginalSkin*
	void	bkupOriginalSkinVertices();
	// restore from _OriginalSkin* to VBuffer. set _OriginalSkinRestored to true
	void	restoreOriginalSkinVertices();

	// apply Skin to all vertices from _OriginalSkin* to _VBuffer.
	void	applySkin(CSkeletonModel *skeleton);


	void	flagSkinVerticesForMatrixBlock(uint8 *skinFlags, CMatrixBlock &mb);
	void	computeSkinMatrixes(CSkeletonModel *skeleton, CMatrix3x4 *matrixes, CMatrixBlock  *prevBlock, CMatrixBlock &curBlock);
	void	computeSoftwarePointSkinning(CMatrix3x4 *matrixes, CVector *srcVector, CPaletteSkin *srcPal, float *srcWgt, CVector *dstVector);
	void	computeSoftwareVectorSkinning(CMatrix3x4 *matrixes, CVector *srcVector, CPaletteSkin *srcPal, float *srcWgt, CVector *dstVector);

	// Shadow mapping and CMesh. NB: not serialized, but created at each load
	CShadowSkin				_ShadowSkin;
	// build the shadow skin, from the VertexBuffer/IndexBuffer
	void	buildShadowSkin();

	// @}

};




} // NL3D


#endif // NL_MESH_H

/* End of mesh.h */
