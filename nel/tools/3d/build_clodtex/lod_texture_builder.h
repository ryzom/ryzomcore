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

#ifndef NL_LOD_TEXTURE_BUILDER_H
#define NL_LOD_TEXTURE_BUILDER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/lod_character_texture.h"
#include "nel/3d/lod_character_shape.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/mesh_mrm_skinned.h"
#include <set>


// ***************************************************************************
/**
 * Build a ClodCharacterTexture with a Clod and a Shape.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CLodTextureBuilder
{
public:
	/// Constructor
	CLodTextureBuilder();

	/// Set the overSample threshold. Default to 0.05f.
	void			setOverSampleDistance(float sam) {_OverSampleDistance= sam;}

	/// Set the Clod shape build reference
	void			setLod(const NL3D::CLodCharacterShapeBuild &lod);

	/// compute the LodCharacterTexture For a CMesh
	bool			computeTexture(const NL3D::CMesh &mesh, NL3D::CLodCharacterTexture &text);

	/// compute the LodCharacterTexture For a CMeshMRM
	bool			computeTexture(const NL3D::CMeshMRM &meshMRM, NL3D::CLodCharacterTexture &text);

	/// compute the LodCharacterTexture For a CMeshMRMSkinned
	bool			computeTexture(const NL3D::CMeshMRMSkinned &meshMRM, NL3D::CLodCharacterTexture &text);


// ****************
private:
	typedef NL3D::CLodCharacterShapeBuild::CPixelInfo		CPixelInfo;

	NL3D::CLodCharacterShapeBuild	_CLod;
	float							_MaxDistanceQuality;
	float							_OverSampleDistance;

	// compute a distance, according to Position and normals.
	float			computeQualityPixel(const CPixelInfo &p0, const CPixelInfo &p1) const;

	// Samples of the current mesh.
	struct	CSample
	{
		// Pos/Normal
		CPixelInfo		P;
		// From which texture (materialId) does this sample come from?
		uint			MaterialId;
		// The UV of this sample
		NLMISC::CUV		UV;
	};
	std::vector<CSample>			_Samples;

	typedef std::pair<uint,uint>		TEdge;
	typedef std::set<TEdge>				TEdgeSet;
	typedef TEdgeSet::iterator			ItEdgeSet;

	// samples the triangles into _Samples (16 & 32 bits versions)
	void			addSampleTris(const uint8 *srcPos, const uint8 *srcNormal, const uint8 *srcUV, uint vertexSize, 
		const uint32 *triPointer, uint numTris, uint materialId, TEdgeSet &edgeSet);
	void			addSampleTris(const uint8 *srcPos, const uint8 *srcNormal, const uint8 *srcUV, uint vertexSize, 
		const uint16 *triPointer, uint numTris, uint materialId, TEdgeSet &edgeSet);
	

	/// build text with _Samples.
	bool			computeTextureFromSamples(NL3D::CLodCharacterTexture &text);



};


#endif // NL_LOD_TEXTURE_BUILDER_H

/* End of lod_texture_builder.h */
