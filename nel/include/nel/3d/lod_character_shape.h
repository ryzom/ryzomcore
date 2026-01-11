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

#ifndef NL_LOD_CHARACTER_SHAPE_H
#define NL_LOD_CHARACTER_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/3d/mesh.h"


namespace NL3D
{


// ***************************************************************************
/**
 * A build structure information for building a CLodCharacterShape
 *	This is the structure exported from the 3D editor
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CLodCharacterShapeBuild
{
public:
	class	CPixelInfo
	{
	public:
		// The associated Pos/Normal of this pixel.
		CVector		Pos;
		CVector		Normal;

		// An id for an empty pixel.
		static const CPixelInfo		EmptyPixel;

	public:
		CPixelInfo() {}
		CPixelInfo(const CVector &pos, const CVector &normal) : Pos(pos), Normal(normal) {}
		bool		operator==(const CPixelInfo &o) const
		{
			return Pos==o.Pos && Normal==o.Normal;
		}

		void		serial(NLMISC::IStream &f)
		{
			f.serial(Pos, Normal);
		}
	};

public:
	CLodCharacterShapeBuild();

	// The Vertices of the shapes
	std::vector<CVector>			Vertices;

	// Palette Skinning Vertices array (same size as Vertices).
	std::vector<CMesh::CSkinWeight>	SkinWeights;

	// UVs (same size as Vertices).
	std::vector<CUV>				UVs;

	// Normals (same size as Vertices).
	std::vector<CVector>			Normals;

	// Bones name. Each matrix id used in SkinWeights must have a corresponding string in the bone name array.
	std::vector<std::string>		BonesNames;

	// Faces array
	std::vector<uint32>				TriangleIndices;


public:

	/** compile the lod: compute Texture Information.
	 *	\param triangleSelection. If not same size as triangles, not used. Else texture info is filled only with
	 *	triangles whose their triangleSelection[triId]==true
	 *	\param textureOverSample is rounded to the best square (4,9,...). Prefer a srq(oddVal): 1,9,25,49 etc...
	 *	NB: overSamples are not averaged, but the nearest sample to the texel center is taken.
	 */
	void				compile(const std::vector<bool> &triangleSelection, uint textureOverSample=25);

	/// serial
	void				serial(NLMISC::IStream &f);

	/// get TextureInfo
	const CPixelInfo	*getTextureInfoPtr();
	uint				getTextureInfoWidth() const {return _Width;}
	uint				getTextureInfoHeight() const {return _Height;}

// ****************
private:
	// For each pixel of the texture (32*32), give what vertex/normal use it.
	std::vector<CPixelInfo>		_TextureInfo;
	uint32						_Width, _Height;

};


// ***************************************************************************
/**
 * A very Small Shape with anims encoded as Key Meshes. Used for Lod of skinned meshes
 *	NB: normals are not skinned (for anim size consideration).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CLodCharacterShape
{
public:

	/// A lod Animation Build information
	struct	CAnimBuild
	{
		/// Name of the Anim.
		std::string		Name;
		/// The number of Keys.
		uint			NumKeys;
		/// The effective time of the animation
		float			AnimLength;

		/// List of Keys * NumVertices
		std::vector<CVector>	Keys;
	};

	/// A compressed vector information
	struct	CVector3s
	{
		sint16	x,y,z;

		void		serial(NLMISC::IStream &f)
		{
			f.serial(x,y,z);
		}
	};

public:

	/// Constructor
	CLodCharacterShape();

	/** build the Mesh base information
	 *	NB: SkinWeights array tells for each vertex what bone to use (for alpha test removing)
	 */
	void			buildMesh(const std::string &name, const CLodCharacterShapeBuild &lodBuild);

	/** Add an animation. many nlAssert to verify array size etc... return false, if same AnimName exist.
	 *	NB: the entire animation is compressed to CVector3s internally.
	 */
	bool			addAnim(const CAnimBuild &animBuild);

	/// serial this shape
	void			serial(NLMISC::IStream &f);

	/// Get name of this lod
	const std::string &		getName() const {return _Name;}

	/// get the animId from a name. -1 if not found
	sint			getAnimIdByName(const std::string &name) const;

	/// get the number of vertices of this mesh
	uint			getNumVertices() const {return _NumVertices;}

	/// get the number of triangles of this mesh
	uint			getNumTriangles() const {return _NumTriangles;}

	/// get the number of bones
	uint			getNumBones() const {return (uint)_Bones.size();}

	/// get a bone id, according to its name. -1 if not found
	sint			getBoneIdByName(const std::string &name) const;

	/// get a ptr to the triangles indices
	const TLodCharacterIndexType *getTriangleArray() const;

	/// get the vector of triangles indices
	const std::vector<TLodCharacterIndexType>	&getTriangleIndices() const {return _TriangleIndices;}

	/// get a ptr on the UVs.
	const CUV		*getUVs() const;

	/// get a ptr to the triangles indices
	const CVector	*getNormals() const;


	/** \name Vertex per Bone AlphaTesting
	 *	This system is used for example to remove weapon of a Lod, using AlphaTesting (ie the polygons
	 *	of the weapons are still drawn, but with alpha==0 => invisible)
	 */
	// @{

	/// init the process by resize-ing a tmp uint8 vector of getNumVertices() size, and reset to 0
	void			startBoneAlpha(std::vector<uint8>	&tmpAlphas) const;

	/// Add a bone alpha influence to tmpAlpha.
	void			addBoneAlpha(uint boneId, std::vector<uint8> &tmpAlphas) const;

	// @}

	/** get a ptr to the vertices of the key according to animId and time.
	 *	NB: the anim Loop if wrapMode is true
	 *	\param unPackScaleFactor return value is the scale factor which to multiply
	 *	\return NULL if animId is not valid
	 */
	const CVector3s	*getAnimKey(uint animId, TGlobalAnimationTime time, bool wrapMode, CVector &unPackScaleFactor) const;


// *******************************
private:

	/// A lod Animation
	struct	CAnim
	{
		/// Name of the Anim.
		std::string		Name;
		/// The number of Keys.
		uint32			NumKeys;
		/// The effective time of the animation
		float			AnimLength;
		float			OOAnimLength;
		/// The Scale factor to be multiplied to transform CVector3s to CVector
		CVector			UnPackScaleFactor;

		/// List of Keys * NumVertices
		std::vector<CVector3s>	Keys;

		void			serial(NLMISC::IStream &f);
	};

	struct	CVertexInf
	{
		// Id of the vertex that the bone influence.
		uint32		VertexId;
		// weight of influence
		float		Influence;

		void		serial(NLMISC::IStream &f)
		{
			f.serial(VertexId);
			f.serial(Influence);
		}
	};

	/// A Bone influence: list of all vertices to influence.
	struct	CBoneInfluence
	{
		// Name of the bone.
		std::string				Name;

		// list of vertex this bone influence.
		std::vector<CVertexInf>	InfVertices;

		void		serial(NLMISC::IStream &f);
	};

	/// Map name To Id.
	typedef	std::map<std::string, uint32>	TStrIdMap;
	typedef	TStrIdMap::iterator				ItStrIdMap;
	typedef	TStrIdMap::const_iterator		CstItStrIdMap;

private:
	std::string				_Name;
	uint32					_NumVertices;
	uint32					_NumTriangles;
	// UVs and Normals
	std::vector<CUV>		_UVs;
	std::vector<CVector>	_Normals;
	/// List of bones and vertices they influence
	std::vector<CBoneInfluence>		_Bones;

	// The map of bone.
	TStrIdMap				_BoneMap;

	/// numTriangles * 3.
	std::vector<TLodCharacterIndexType>		_TriangleIndices;

	/// List of animation.
	std::vector<CAnim>		_Anims;

	// The map of animation.
	TStrIdMap				_AnimMap;

};


} // NL3D


#endif // NL_LOD_CHARACTER_SHAPE_H

/* End of lod_character_shape.h */
